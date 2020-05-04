/*
 * Copyright (c) 2009-2020 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Driver for OpenGL graphics on Windows. This is a multiple display
 * driver; one context is created for each Agar window.
 */

#include <agar/config/ag_threads.h>

#include <agar/core/core.h>
#include <agar/core/win32.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/text.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>

/* Print out events in WndProc */
/* #define DEBUG_WGL */

static int  nDrivers = 0;			/* Drivers open */
static int  wndClassCount = 1;			/* Window class counter */
static AG_DriverEventQ wglEventQ;		/* Event queue */
#ifdef AG_THREADS
static _Nonnull_Mutex AG_Mutex wglClassLock;		/* Lock on wndClassCount */
static _Nonnull_Mutex AG_Mutex wglEventLock;		/* Lock on wglEventQ */
#endif
static _Nullable HKL wglKbdLayout = NULL;		/* Keyboard layout */
static AG_EventSink *_Nullable wglEventSpinner = NULL;	/* Standard event sink */
static AG_EventSink *_Nullable wglEventEpilogue = NULL;	/* Standard event epilogue */

/* Driver instance data */
typedef struct ag_driver_wgl {
	struct ag_driver_mw _inherit;
	HWND        hwnd;		/* Window handle */
	HDC         hdc;		/* Device context */
	HGLRC       hglrc;		/* Rendering context */
	WNDCLASSEX  wndclass;		/* Window class */
	AG_GL_Context gl;		/* Common OpenGL context data */
	LRESULT     nchittest;		/* Result of last WM_NCHITTEST */
} AG_DriverWGL;

typedef struct ag_cursor_wgl {
	struct ag_cursor _inherit;
	int      shared;		/* Shared cursor */
	COLORREF black;
	COLORREF white;
	HCURSOR  cursor;
} AG_CursorWGL;

AG_DriverMwClass agDriverWGL;

#define AGDRIVER_IS_WGL(drv) \
	(AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverWGL)

struct ag_windows_key_mapping {
	int kcode;			/* Windows VK_ virtual key */
	AG_KeySym key;			/* Agar keysym */
};
#include <agar/gui/drv_wgl_keymaps.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void      WGL_InitDefaultCursor(AG_DriverWGL *_Nonnull);
static void      WGL_PostResizeCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static int       WGL_RaiseWindow(AG_Window *_Nonnull);
static int       WGL_SetInputFocus(AG_Window *_Nonnull);
static void      WGL_PostMoveCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static int       WGL_GetNextEvent(void *_Nullable, AG_DriverEvent *_Nonnull);
static int       WGL_ProcessEvent(void *_Nullable, AG_DriverEvent *_Nonnull);
static int       WGL_GetDisplaySize(Uint *_Nonnull, Uint *_Nonnull);
static int       WGL_PendingEvents(void *_Nonnull drvCaller);
static int       WGL_SetCursor(void *_Nonnull, AG_Cursor *_Nonnull);

static void
WGL_SetWindowsError(char *_Nonnull errorMessage, DWORD errorCode)
{
	char lpBuffer[65536];

	if (FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpBuffer,
		0, NULL)) {
		AG_SetError("WGL Driver: %s! (%s)", errorMessage, lpBuffer);
	} else {
		AG_SetError("WGL Driver: %s!", errorMessage);
	}
}

/* Return the Agar window corresponding to a Windows window handle */
static AG_Window *_Nullable
LookupWindowByID(HWND hwnd)
{
	AG_Window *win;
	AG_DriverWGL *wgl;

	/* XXX can we set a user pointer to avoid this traversal? */
	AGOBJECT_FOREACH_CHILD(wgl, &agDrivers, ag_driver_wgl) {
		if (!AGDRIVER_IS_WGL(wgl) ||
		    wgl->hwnd != hwnd) {
			continue;
		}
		win = AGDRIVER_MW(wgl)->win;
		if (!(AGDRIVER_MW(wgl)->flags & AG_DRIVER_MW_OPEN) ||
		    (win->flags & AG_WINDOW_DETACHING) ||
		    WIDGET(win)->drv == NULL) {
			return (NULL);
		}
		return (win);
	}
	return (NULL);
}

/*
 * Standard AG_EventLoop() event sink.
 */
static int
WGL_EventSink(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	AG_DriverEvent dev;

	/* expand WGL_PendingEvents(): */
	if ((TAILQ_EMPTY(&wglEventQ) &&
	     GetQueueStatus(QS_ALLINPUT) == 0)) {
		return (0);
	}
	if (WGL_GetNextEvent(NULL, &dev) == 1) {
		return WGL_ProcessEvent(NULL, &dev);
	}
	return (0);
}
static int
WGL_EventEpilogue(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	return (0);
}

static int
WGL_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverWGL *wgl = obj;
	
	/* Register Mouse and keyboard */
	if ((drv->mouse = AG_MouseNew(wgl, "Windows mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(wgl, "Windows keyboard")) == NULL)
		goto fail;
	
	/* Driver manages rendering of window background. */
	drv->flags |= AG_DRIVER_WINDOW_BG;
	
	if (nDrivers == 0) {
		char kbdLayoutName[KL_NAMELENGTH];
		
		/* Set up event filters for standard AG_EventLoop(). */
		if ((wglEventSpinner = AG_AddEventSpinner(WGL_EventSink, NULL)) == NULL ||
		    (wglEventEpilogue = AG_AddEventEpilogue(WGL_EventEpilogue, NULL)) == NULL)
			goto fail;

		/* Initialize globals */
		TAILQ_INIT(&wglEventQ);
		AG_MutexInitRecursive(&wglClassLock);
		AG_MutexInitRecursive(&wglEventLock);

		/* Set up keyboard layout */
		GetKeyboardLayoutName(kbdLayoutName);
		if ((wglKbdLayout = LoadKeyboardLayout("00000409",
		    KLF_NOTELLSHELL)) == NULL) {
			wglKbdLayout = GetKeyboardLayout(0);
		}
		LoadKeyboardLayout(kbdLayoutName, KLF_ACTIVATE);
		InitKeymaps();
	}
	nDrivers++;
	return (0);
fail:
	if (wglEventSpinner)  { AG_DelEventSpinner(wglEventSpinner);   wglEventSpinner = NULL; }
	if (wglEventEpilogue) { AG_DelEventEpilogue(wglEventEpilogue); wglEventEpilogue = NULL; }
	if (drv->kbd)   { AG_ObjectDelete(drv->kbd);   drv->kbd = NULL; }
	if (drv->mouse) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
WGL_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
/*	AG_DriverEvent *dev, *devNext; */

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0) {
		AG_DelEventSink(wglEventSpinner); wglEventSpinner = NULL;
		AG_DelEventEpilogue(wglEventEpilogue); wglEventEpilogue = NULL;
#if 0
		for (dev = TAILQ_FIRST(&wglEventQ);
		     dev != TAILQ_LAST(&wglEventQ, ag_driver_eventq);
		     dev = devNext) {
			devNext = TAILQ_NEXT(dev, events);
			free(dev);
		}
#endif
		TAILQ_INIT(&wglEventQ);

		AG_MutexDestroy(&wglClassLock);
		AG_MutexDestroy(&wglEventLock);
	}

	AG_ObjectDelete(drv->mouse);	drv->mouse = NULL;
	AG_ObjectDelete(drv->kbd);	drv->kbd = NULL;
}

/* Return suitable window style from Agar window flags. */
static void
WGL_GetWndStyle(AG_Window *_Nonnull win, DWORD *_Nonnull wndStyle,
    DWORD *_Nonnull wndStyleEx)
{
	if (win->wmType == AG_WINDOW_WM_NORMAL) {
		*wndStyle = WS_OVERLAPPEDWINDOW;
	} else {
		*wndStyle = WS_POPUP;
	}
	*wndStyleEx = 0;
	
	if (win->flags & AG_WINDOW_NOTITLE)
		(*wndStyle) &= ~(WS_CAPTION);
	if (win->flags & AG_WINDOW_NOMINIMIZE)
		(*wndStyle) &= ~(WS_MINIMIZEBOX);
	if (win->flags & AG_WINDOW_NOMAXIMIZE)
		(*wndStyle) &= ~(WS_MAXIMIZEBOX);
	if (win->flags & AG_WINDOW_NORESIZE)
		(*wndStyle) &= ~(WS_THICKFRAME);
#if defined(WINVER) && (WINVER >= 0x0400)
	if (win->wmType == AG_WINDOW_WM_TOOLBAR ||
	    win->wmType == AG_WINDOW_WM_MENU ||
	    win->wmType == AG_WINDOW_WM_DROPDOWN_MENU ||
	    win->wmType == AG_WINDOW_WM_POPUP_MENU)
		(*wndStyleEx) |= WS_EX_TOOLWINDOW;
#endif
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
	if (win->flags & AG_WINDOW_KEEPBELOW)
		(*wndStyleEx) |= WS_EX_NOACTIVATE;
#endif
#if defined(WS_EX_TOPMOST)
	if (win->flags & AG_WINDOW_KEEPABOVE)
		(*wndStyleEx) |= WS_EX_TOPMOST;
#endif
}

/* Return window rectangle adjusted for titlebar/border style. */
static void
WGL_GetWndRect(AG_Window *_Nonnull win, AG_Rect *_Nonnull r)
{
	DWORD wndStyle, wndStyleEx;
	RECT wndRect = {
	    r->x,
	    r->y,
	    r->x + r->w,
	    r->y + r->h
	};

	WGL_GetWndStyle(win, &wndStyle, &wndStyleEx);
	AdjustWindowRectEx(&wndRect, wndStyle, 0, wndStyleEx);

	r->x = wndRect.left;
	r->y = wndRect.top;
	r->w = wndRect.right - wndRect.left;
	r->h = wndRect.bottom - wndRect.top;
}

static int
WGL_OpenWindow(AG_Window *_Nonnull win, const AG_Rect *_Nonnull r, int depthReq,
    Uint mwFlags)
{
	char wndClassName[32]; 
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	GLuint pixelFormat;	
	WNDCLASSEX wndClass;
	DWORD wndStyle, wndStyleEx;
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		16,                      /* Color depth */
		0, 0, 0, 0, 0, 0, 0, 0,  /* Ignore Color bits */
		0, 0, 0, 0, 0,           /* No Accumulation buffer */
		0,                       /* No Z-Buffer */
		0, 0,                    /* No Stencil + AUX-Buffer */
		0, 0, 0, 0, 0            /* All other attributes are not used */
	};
	RECT wndRect;
	AG_Rect rVP;
	int x,y;

	if (agStereo)
		pixelFormatDescriptor.dwFlags |= PFD_STEREO;

	/* Register Window Class */
	AG_MutexLock(&wglClassLock);
	sprintf(wndClassName, "agar-wgl-windowclass-%d", wndClassCount);
	wndClassCount++;
	AG_MutexUnlock(&wglClassLock);

	memset(&wndClass, 0, sizeof(WNDCLASSEX));
	wndClass.cbSize        = sizeof(WNDCLASSEX);
	wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc   = WndProc;
	wndClass.hInstance     = GetModuleHandle(NULL);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hCursor       = NULL;
	wndClass.lpszClassName = wndClassName;

	if (!RegisterClassEx(&wndClass)) {
		WGL_SetWindowsError("Cannot register WGL window class", 
		    GetLastError());
		return (-1);
	}

	/* Translate Agar window flags to window style. */
	WGL_GetWndStyle(win, &wndStyle, &wndStyleEx);

	x = r->x;
	y = r->y;

	/*
	 * XXX TODO it would be best to pass CW_USEDEFAULT here, but
	 * I could not find a way to retrieve the final allocated
	 * coordinates; GetWindowRect() for instance does not seem
	 * to work as expected. For now we'll just center the window.
	 */
	if (mwFlags & AG_DRIVER_MW_ANYPOS) {
		Uint wDisp, hDisp;

		if (WGL_GetDisplaySize(&wDisp, &hDisp) == 0) {
			x = wDisp/2 - r->w/2;
			y = hDisp/2 - r->h/2;
		}
	}
	
	/* Adjust window with account for window borders, if any */
	wndRect.left   = x;
	wndRect.top    = y;
	wndRect.right  = x + r->w;
	wndRect.bottom = y + r->h;
	AdjustWindowRectEx(&wndRect, wndStyle, 0, wndStyleEx);

	/* Create OpenGL Window */
	wgl->hwnd = CreateWindowEx(
		wndStyleEx,
		wndClassName,
		win->caption,
		wndStyle,
		wndRect.left,
		wndRect.top,
		(wndRect.right - wndRect.left),
		(wndRect.bottom - wndRect.top),
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);
	if (!wgl->hwnd) {
		WGL_SetWindowsError("Cannot create window", GetLastError());
		return (-1);
	}

	wgl->nchittest = 0;

	/* Initialize device & rendering context */
	if (!(wgl->hdc = GetDC(wgl->hwnd))) {
		DestroyWindow(wgl->hwnd);
		WGL_SetWindowsError("GetDC failed", GetLastError());
		return (-1);
	}
	if (!(pixelFormat = ChoosePixelFormat(wgl->hdc, &pixelFormatDescriptor))) {
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		WGL_SetWindowsError("ChoosePixelFormat failed", GetLastError());
		return (-1);
	}
	if (!(SetPixelFormat(wgl->hdc, pixelFormat, &pixelFormatDescriptor))) {
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		WGL_SetWindowsError("SetPixelFormat failed", GetLastError());
		return (-1);
	}
	if (!(wgl->hglrc = wglCreateContext(wgl->hdc))) {
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		WGL_SetWindowsError("wglCreateContext failed", GetLastError());
		return (-1);
	}
	if (!(wglMakeCurrent(wgl->hdc, wgl->hglrc))) {
		wglDeleteContext(wgl->hglrc);
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		WGL_SetWindowsError("wglMakeCurrent failed", GetLastError());
		return (-1);
	}
	AG_GL_InitContext(wgl, &wgl->gl);

	rVP.x = 0;
	rVP.y = 0;
	rVP.w = WIDTH(win);
	rVP.h = HEIGHT(win);
	AG_GL_SetViewport(&wgl->gl, &rVP);
	
	/* Show the window */
	ShowWindow(wgl->hwnd, SW_SHOW);
	if (!(win->flags & AG_WINDOW_KEEPBELOW)) {
		SetForegroundWindow(wgl->hwnd);
		/* SetCapture(wgl->hwnd); */
	}
	
	/* Set the pixel format */
	if ((drv->videoFmt = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		goto fail;
	}
#if AG_MODEL == AG_LARGE
	if (depthReq == 48) {				/* Deep color */
# if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, 48,
			0xffff000000000000,
			0x0000ffff00000000,
			0x00000000ffff0000);
# else
		AG_PixelFormatRGB(drv->videoFmt, 48,
			0x000000000000ffff,
			0x00000000ffff0000,
			0x0000ffff00000000);
# endif
	} else
#endif /* AG_LARGE */
	{						/* True Color */
#if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, 32,
			0xff000000,
			0x00ff0000,
			0x0000ff00);
#else
		AG_PixelFormatRGB(drv->videoFmt, 32,
			0x000000ff,
			0x0000ff00,
			0x00ff0000);
#endif
	}

	/* Create the built-in cursors */
	WGL_InitDefaultCursor(wgl);
	AG_InitStockCursors(drv);

	/* Focus the window. */
	if (!(win->flags & AG_WINDOW_DENYFOCUS)) {
		SetFocus(wgl->hwnd);
		agWindowFocused = win;
		AG_PostEvent(win, "window-gainfocus", NULL);
	}
	return (0);
fail:
	wglDeleteContext(wgl->hglrc);
	DestroyWindow(wgl->hwnd);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static void
WGL_CloseWindow(AG_Window *_Nonnull win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverWGL *wgl = (AG_DriverWGL *)drv;

	/* Release allocated cursors. */
	AG_FreeCursors(drv);

	/* Destroy our OpenGL context. */
	wglMakeCurrent(wgl->hdc, wgl->hglrc);
	AG_GL_DestroyContext(wgl);
	wglDeleteContext(wgl->hglrc);

	/* Close the window. */
	DestroyWindow(wgl->hwnd);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
}

static int
WGL_GetDisplaySize(Uint *_Nonnull w, Uint *_Nonnull h)
{
	RECT r;
	HWND desktop = GetDesktopWindow();

	GetWindowRect(desktop, &r);
	
	*w = r.right  - r.left;
	*h = r.bottom - r.top;

	return (0);
}

#ifdef DEBUG_WGL
# include "drv_wgl_prwincmd.inc"
#endif

#undef IN_KEYPAD
#define IN_KEYPAD(scan) (scan & 0x100)

/* Convert scancode to a proper index into agWindowsKeymap[]. */
static int
ScanToVirtualKey(int scan, Uint vKey)
{
	int vk = MapVirtualKeyEx(scan & 0xff, 1, wglKbdLayout);

	switch (vKey) {
	case VK_DIVIDE:
	case VK_MULTIPLY:
	case VK_SUBTRACT:
	case VK_ADD:
	case VK_LWIN:
	case VK_RWIN:
	case VK_APPS:
	case VK_LCONTROL:
	case VK_RCONTROL:
	case VK_LSHIFT:
	case VK_RSHIFT:
	case VK_LMENU:
	case VK_RMENU:
	case VK_SNAPSHOT:
	case VK_PAUSE:
		return (vKey);
	}
	switch (vk) {
	case VK_INSERT:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD0;
	case VK_END:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD1;
	case VK_DOWN:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD2;
	case VK_NEXT:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD3;
	case VK_LEFT:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD4;
	case VK_CLEAR:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD5;
	case VK_RIGHT:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD6;
	case VK_HOME:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD7;
	case VK_UP:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD8;
	case VK_PRIOR:		return IN_KEYPAD(scan) ? vk : VK_NUMPAD9;
	case VK_DELETE:		return IN_KEYPAD(scan) ? vk : VK_DECIMAL;
	}
	return (vk != 0) ? vk : vKey;
}
	
static __inline__ AG_DriverEvent *_Nullable /* _Malloc_Like_Attribute */
NewEvent(AG_Window *_Nonnull win, enum ag_driver_event_type type)
{
	AG_DriverEvent *dev;

	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) == NULL) {
		return (NULL);
	}
	dev->win = win;
	dev->type = type;
	return (dev);
}

/* 
 * Window procedure. Most events are just translated to AG_DriverEvent form
 * and queued for later retrieval by getNextEvent().
 */
LRESULT CALLBACK
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AG_Driver *drv;
	AG_DriverWGL *wgl;
	AG_Window *win;
	AG_DriverEvent *dev = NULL;
	int x, y;
	LRESULT rv = 1;
	
	AG_LockVFS(&agDrivers);
	
	if ((win = LookupWindowByID(hWnd)) == NULL) {
		goto fallback;
	}
	drv = WIDGET(win)->drv;
	wgl = (AG_DriverWGL *)drv;
#ifdef DEBUG_WGL
	WGL_Print_WinMsg(win, uMsg, wParam, lParam);
#endif
	switch (uMsg) {
	case WM_MOUSEMOVE:
		if ((dev = NewEvent(win, AG_DRIVER_MOUSE_MOTION)) == NULL) {
			goto fallback;
		}
		x = (int)LOWORD(lParam);
		y = (int)HIWORD(lParam);
		dev->data.motion.x = x;
		dev->data.motion.y = y;
		AG_MouseMotionUpdate(drv->mouse, 
		    dev->data.motion.x, dev->data.motion.y);
		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if ((dev = NewEvent(win, AG_DRIVER_MOUSE_BUTTON_DOWN)) == NULL) {
			goto fallback;
		}
		dev->data.button.which =
		    (wParam & MK_LBUTTON) ? AG_MOUSE_LEFT :
		    (wParam & MK_MBUTTON) ? AG_MOUSE_MIDDLE :
		    (wParam & MK_RBUTTON) ? AG_MOUSE_RIGHT : 0;
		x = (int)LOWORD(lParam);
		y = (int)HIWORD(lParam);
		dev->data.button.x = AGDRIVER_BOUNDED_WIDTH(win, x);
		dev->data.button.y = AGDRIVER_BOUNDED_HEIGHT(win, y);
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED, 
		    dev->data.button.which);
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		if ((dev = NewEvent(win, AG_DRIVER_MOUSE_BUTTON_UP)) == NULL) {
			goto fallback;
		}
		dev->data.button.which =
		    (uMsg == WM_LBUTTONUP) ? AG_MOUSE_LEFT :
		    (uMsg == WM_MBUTTONUP) ? AG_MOUSE_MIDDLE :
		    (uMsg == WM_RBUTTONUP) ? AG_MOUSE_RIGHT : 0;
		x = (int)LOWORD(lParam);
		y = (int)HIWORD(lParam);
		dev->data.button.x = AGDRIVER_BOUNDED_WIDTH(win, x);
		dev->data.button.y = AGDRIVER_BOUNDED_HEIGHT(win, y);
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED, 
		    dev->data.button.which);
		break;
	case 0x020a:				/* WM_MOUSEWHEEL (missing define) */
		{
			int move = (short)HIWORD(wParam);

			if (move == 0) {
				goto fallback;
			}
			if ((dev = NewEvent(win, AG_DRIVER_MOUSE_BUTTON_DOWN)) == NULL) {
				goto fallback;
			}
			dev->data.button.which =
			    (move > 0) ? AG_MOUSE_WHEELUP : AG_MOUSE_WHEELDOWN;
			x = (int)LOWORD(lParam) - WIDGET(win)->x;
			y = (int)HIWORD(lParam) - WIDGET(win)->y;
			dev->data.button.x = AGDRIVER_BOUNDED_WIDTH(win, x);
			dev->data.button.y = AGDRIVER_BOUNDED_HEIGHT(win, y);
			
			AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
			    dev->data.button.which);
		}
		break;
	case WM_KEYUP:
	case WM_KEYDOWN:
		{
			AG_KeyboardAction ka;
			int scan = HIWORD(lParam);
			Uint vKey = wParam;
			Uint8 keyState[256];
			Uint16 wc[2];
		
			if ((dev = NewEvent(win, (uMsg == WM_KEYDOWN) ?
			    AG_DRIVER_KEY_DOWN : AG_DRIVER_KEY_UP)) == NULL) {
				goto fallback;
			}
			dev->data.key.ucs = 0;
			ka = (uMsg == WM_KEYDOWN) ? AG_KEY_PRESSED :
			                            AG_KEY_RELEASED;

			switch (vKey) {
			case VK_CONTROL:
				vKey = (lParam & (1<<24)) ? VK_RCONTROL :
				                            VK_LCONTROL;
				break;
			case VK_RETURN:
				if (IN_KEYPAD(scan)) {
					AG_KeyboardUpdate(drv->kbd, ka,
					    AG_KEY_KP_ENTER);
					dev->data.key.ks = AG_KEY_KP_ENTER;
					goto out;
				}
				break;
			}

			dev->data.key.ks =
			    agWindowsKeymap[ScanToVirtualKey(scan,vKey)];

			GetKeyboardState(keyState);
			if ((keyState[VK_NUMLOCK] & 1) &&
			    vKey >= VK_NUMPAD0 && vKey <= VK_NUMPAD9) {
				dev->data.key.ucs = vKey - '0'+VK_NUMPAD0;
			} else if (ToUnicode((UINT)vKey, scan, keyState, wc,2, 0) > 0) {
				dev->data.key.ucs = wc[0];
			}
			AG_KeyboardUpdate(drv->kbd, ka, dev->data.key.ks);
		}
		break;
	case WM_SETFOCUS:
		if (win->visible) {
			dev = NewEvent(win, AG_DRIVER_FOCUS_IN);
			rv = 0;
			goto out;
		} else {
			goto fallback;
		}
	case WM_KILLFOCUS:
		if (win->visible) {
			dev = NewEvent(win, AG_DRIVER_FOCUS_OUT);
			rv = 0;
			goto out;
		} else {
			goto fallback;
		}
	case WM_SIZE:
		if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
			TAILQ_FOREACH(dev, &wglEventQ, events) {
				if (dev->type == AG_DRIVER_VIDEORESIZE)
					break;
			}
			if (dev) {
				dev->data.videoresize.w = LOWORD(lParam);
				dev->data.videoresize.h = HIWORD(lParam);
				dev = NULL;
				rv = 0;
				goto out;
			}
			if ((dev = NewEvent(win, AG_DRIVER_VIDEORESIZE)) == NULL) {
				goto fallback;
			}
			dev->data.videoresize.x = -1;
			dev->data.videoresize.y = -1;
			dev->data.videoresize.w = LOWORD(lParam);
			dev->data.videoresize.h = HIWORD(lParam);
		} else {
			goto fallback;
		}
		rv = 0;
		goto out;
	case WM_MOVE:
		WIDGET(win)->x = (int)(short)LOWORD(lParam);
		WIDGET(win)->y = (int)(short)HIWORD(lParam);
		rv = 0;
		goto out;
#if 0
	/*
	 * XXX TODO: use TrackMouseEvent(), translate WM_MOUSEHOVER
	 * events to AG_DRIVER_MOUSE_ENTER.
	 */
	case WM_MOUSEHOVER:
		break;
#endif
	case WM_MOUSELEAVE:
		dev = NewEvent(win, AG_DRIVER_MOUSE_LEAVE);
		break;
	case WM_ERASEBKGND:
		dev = NewEvent(win, AG_DRIVER_EXPOSE);
		rv = 0;
		goto out;
	case WM_CLOSE:
		dev = NewEvent(win, AG_DRIVER_CLOSE);
		rv = 0;
		goto out;
	case WM_SETCURSOR:
		AG_MouseGetState(drv->mouse, &x, &y);
		AG_MouseCursorUpdate(win, x, y);
		break;
	case WM_NCHITTEST:
		rv = DefWindowProc(hWnd, uMsg, wParam, lParam);
		wgl->nchittest = rv;
		goto out;
	default:
		goto fallback;
	}
out:
	if (dev) {
		TAILQ_INSERT_TAIL(&wglEventQ, dev, events);
	}
	AG_UnlockVFS(&agDrivers);
	return (rv);
fallback:
	AG_UnlockVFS(&agDrivers);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static int
WGL_PendingEvents(void *_Nonnull drvCaller)
{
	return (!TAILQ_EMPTY(&wglEventQ) ||
	        GetQueueStatus(QS_ALLINPUT) != 0);
}

static int
WGL_GetNextEvent(void *_Nonnull drvCaller, AG_DriverEvent *_Nonnull dev)
{
	AG_DriverEvent *devFirst;
	MSG msg;

	if (!TAILQ_EMPTY(&wglEventQ)) {
		goto get_event;
	}
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (!TAILQ_EMPTY(&wglEventQ))
			goto get_event;
	}
	return (0);
get_event:
	devFirst = TAILQ_FIRST(&wglEventQ);
	memcpy(dev, devFirst, sizeof(AG_DriverEvent));
	TAILQ_REMOVE(&wglEventQ, devFirst, events);
	free(devFirst);
	return (1);
}

static int
WGL_ProcessEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	AG_SizeAlloc a;
	AG_Driver *drv;
	AG_Window *win;
	int rv=1, useText;

	if ((win = dev->win) == NULL ||
	    win->flags & AG_WINDOW_DETACHING)
		return (0);

	AG_LockVFS(&agDrivers);
	drv = WIDGET(win)->drv;

	if ((useText = (win->flags & AG_WINDOW_USE_TEXT))) {
		AG_PushTextState();
		AG_TextFont(WIDGET(win)->font);
		AG_TextColor(&WIDGET(win)->pal.c[WIDGET(win)->state]
		                                [AG_TEXT_COLOR]);
	}
	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		AG_ProcessMouseMotion(win,
		    dev->data.motion.x, dev->data.motion.y,
		    drv->mouse->xRel, drv->mouse->yRel,
		    drv->mouse->btnState);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		AG_ProcessMouseButtonDown(win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		AG_ProcessMouseButtonUp(win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_KEY_UP:
		AG_ProcessKey(drv->kbd, win, AG_KEY_RELEASED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_KEY_DOWN:
		AG_ProcessKey(drv->kbd, win, AG_KEY_PRESSED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_MOUSE_ENTER:
		AG_PostEvent(win, "window-enter", NULL);
		break;
	case AG_DRIVER_MOUSE_LEAVE:
		AG_PostEvent(win, "window-leave", NULL);
		break;
	case AG_DRIVER_FOCUS_IN:
		if (win != agWindowFocused) {
			agWindowFocused = win;
			AG_PostEvent(win, "window-gainfocus", NULL);
		}
		break;
	case AG_DRIVER_FOCUS_OUT:
		if (win == agWindowFocused) {
			AG_PostEvent(win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		break;
	case AG_DRIVER_VIDEORESIZE:
		a.x = dev->data.videoresize.x;
		a.y = dev->data.videoresize.y;
		a.w = dev->data.videoresize.w;
		a.h = dev->data.videoresize.h;
		if (a.w != WIDTH(win) || a.h != HEIGHT(win)) {
			WGL_PostResizeCallback(win, &a);
		} else {
			WGL_PostMoveCallback(win, &a);
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(win, "window-close", NULL);
		break;
	case AG_DRIVER_EXPOSE:
		win->dirty = 1;
		break;
	default:
		rv = 0;
		break;
	}
	if (useText) {
		AG_PopTextState();
	}
	AG_UnlockVFS(&agDrivers);
	return (rv);
}

static void
WGL_BeginRendering(void *_Nonnull obj)
{
	AG_DriverWGL *wgl = obj;

	wglMakeCurrent(wgl->hdc, wgl->hglrc);
}

static void
WGL_RenderWindow(AG_Window *_Nonnull win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_GL_Context *gl = &wgl->gl;
	const AG_Color *cBg = &WCOLOR(win, BG_COLOR);

	AG_PushClipRect(win, &WIDGET(win)->r);
	
	glClearColor((float)cBg->r / AG_COLOR_LASTF,
	             (float)cBg->g / AG_COLOR_LASTF,
		     (float)cBg->b / AG_COLOR_LASTF, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	AG_WidgetDraw(win);

	AG_PopClipRect(win);
}

static void
WGL_EndRendering(void *_Nonnull obj)
{
	AG_DriverWGL *wgl = obj;
	AG_GL_Context *gl = &wgl->gl;
	Uint i;
	
	SwapBuffers(wgl->hdc);

	/* Remove textures and display lists queued for deletion */
	glDeleteTextures(gl->nTextureGC, gl->textureGC);
	for (i = 0; i < gl->nListGC; i++) {
		glDeleteLists(gl->listGC[i], 1);
	}
	gl->nTextureGC = 0;
	gl->nListGC = 0;
}

static int
WGL_MapWindow(AG_Window *_Nonnull win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;

	ShowWindow(wgl->hwnd, SW_SHOW);
	return (0);
}

static int
WGL_UnmapWindow(AG_Window *_Nonnull win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;

	ShowWindow(wgl->hwnd, SW_HIDE);
	return (0);
}

static int
WGL_RaiseWindow(AG_Window *_Nonnull win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;

	SetWindowPos(wgl->hwnd, HWND_TOP, 0,0, 0,0, SWP_NOMOVE | SWP_NOSIZE);
	return (0);
}

static int
WGL_LowerWindow(AG_Window *_Nonnull win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;

	SetWindowPos(wgl->hwnd, HWND_BOTTOM, 0,0, 0,0, SWP_NOMOVE | SWP_NOSIZE);
	return (0);
}

static int
WGL_ReparentWindow(AG_Window *_Nonnull win, AG_Window *_Nonnull winParent,
    int x, int y)
{
	AG_DriverWGL *wglWin = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_DriverWGL *wglParentWin = (AG_DriverWGL *)WIDGET(winParent)->drv;

	SetParent(wglWin->hwnd, wglParentWin->hwnd);
	return (0);
}

static int
WGL_GetInputFocus(AG_Window **rv)
{
	AG_DriverWGL *wgl = NULL;
	HWND hwnd;

	hwnd = GetFocus();

	AGOBJECT_FOREACH_CHILD(wgl, &agDrivers, ag_driver_wgl) {
		if (!AGDRIVER_IS_WGL(wgl)) {
			continue;
		}
		if (wgl->hwnd == hwnd)
			break;
	}
	if (wgl == NULL) {
		AG_SetError("Input focus is external to this application");
		return (-1);
	}
	*rv = AGDRIVER_MW(wgl)->win;
	return (0);
}

static int
WGL_SetInputFocus(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;

	SetFocus(wgl->hwnd);
	return (0);
}

static int
WGL_MoveWindow(AG_Window *win, int x, int y)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_SizeAlloc a;

	SetWindowPos(wgl->hwnd, NULL, x,y, 0,0, SWP_NOZORDER | SWP_NOSIZE);
	a.x = x;
	a.y = y;
	a.w = WIDTH(win);
	a.h = HEIGHT(win);
	WGL_PostMoveCallback(win, &a);
	return (0);
}

static int
WGL_ResizeWindow(AG_Window *win, Uint w, Uint h)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_SizeAlloc a;
	AG_Rect r;

	r.x = WIDGET(win)->x;
	r.y = WIDGET(win)->y;
	r.w = w;
	r.h = h;
	WGL_GetWndRect(win, &r);
	SetWindowPos(wgl->hwnd, NULL, 0,0, r.w,r.h, SWP_NOZORDER | SWP_NOMOVE);	

	a.x = WIDGET(win)->x;
	a.y = WIDGET(win)->y;
	a.w = w;
	a.h = h;
	WGL_PostResizeCallback(win, &a);
	return (0);
}

static int
WGL_MoveResizeWindow(AG_Window *win, AG_SizeAlloc *a)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_Rect r;

	r.x = a->x;
	r.y = a->y;
	r.w = a->w;
	r.h = a->h;
	WGL_GetWndRect(win, &r);
	SetWindowPos(wgl->hwnd, NULL, r.x, r.y, r.w, r.h,
	    SWP_NOZORDER);

	WGL_PostResizeCallback(win, a);
	return (0);
}

static int
WGL_SetWindowCaption(AG_Window *win, const char *s)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowText(wgl->hwnd, s);
	return (0);
}

static void
WGL_SetTransientFor(AG_Window *win, AG_Window *winParent)
{
	/* Nothing to do */
}

static void
WGL_TweakAlignment(AG_Window *win, AG_SizeAlloc *a, Uint wMax, Uint hMax)
{
	/* XXX TODO */
	switch (win->alignment) {
	case AG_WINDOW_TL:
	case AG_WINDOW_TC:
	case AG_WINDOW_TR:
		a->y += 50;
		if (a->y+a->h > hMax) { a->y = 0; }
		break;
	case AG_WINDOW_BL:
	case AG_WINDOW_BC:
	case AG_WINDOW_BR:
		a->y -= 100;
		if (a->y < 0) { a->y = 0; }
		break;
	}
}

/*
 * Cursor operations
 */

/* Initialize the default cursor. */
static void
WGL_InitDefaultCursor(AG_DriverWGL *wgl)
{
	AG_Driver *drv = AGDRIVER(wgl);
	const int nStockCursors = 1; /* TODO map */
	int i;

	for (i = 0; i < nStockCursors; i++) {
		AG_CursorWGL *acWGL;
	
		acWGL = Malloc(sizeof(AG_CursorWGL));
		acWGL->shared = 1;
		acWGL->cursor = LoadCursor(NULL, IDC_ARROW);
		AG_CursorInit(AGCURSOR(acWGL));
		TAILQ_INSERT_HEAD(&drv->cursors, AGCURSOR(acWGL), cursors);
		drv->nCursors++;
	}
}

static AG_Cursor *
WGL_CreateCursor(void *_Nonnull obj, Uint w, Uint h, const Uint8 *_Nonnull data,
    const Uint8 *_Nonnull mask, int xHot, int yHot)
{
	AG_Cursor *ac;
	AG_CursorWGL *acWGL;
	int dataSize, i;
	BYTE *xorMask, *andMask;
	Uint size = w*h;

	/*
	 * Initialize generic Agar cursor part.
	 */
	if ((acWGL = TryMalloc(sizeof(AG_CursorWGL))) == NULL) {
		return (NULL);
	}
	ac = (AG_Cursor *)acWGL;

	if ((ac->data = TryMalloc(size)) == NULL) {
		goto fail;
	}
	if ((ac->mask = TryMalloc(size)) == NULL) {
		free(ac->data);
		goto fail;
	}
	memcpy(ac->data, data, size);
	memcpy(ac->mask, mask, size);
	ac->w = w;
	ac->h = h;
	ac->xHot = xHot;
	ac->yHot = yHot;

	/*
	 * Initialize Windows-specific part.
	 */
	acWGL->shared = 0;
	acWGL->black = RGB(0, 0, 0);
	acWGL->white = RGB(0xFF, 0xFF, 0xFF);
	
	dataSize = (ac->w / 8) * ac->h;
	if ((xorMask = TryMalloc(dataSize)) == NULL) {
		free(ac->data);
		goto fail;
	}
	if ((andMask = TryMalloc(dataSize)) == NULL) {
		free(xorMask);
		free(ac->data);
		goto fail;
	}
	for (i = 0; i < dataSize; i++) {
		andMask[i] = ~ac->mask[i];
		xorMask[i] = ~ac->data[i] ^ ~ac->mask[i];
	}

	acWGL->cursor = CreateCursor(GetModuleHandle(NULL), ac->xHot, ac->yHot,
	    ac->w, ac->h, andMask, xorMask);
	if (!acWGL->cursor) {
		WGL_SetWindowsError("CreateCursor", GetLastError());
		goto fail;
	}
	return (ac);
fail:
	free(ac);
	return (NULL);
}

static void
WGL_FreeCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;
	AG_CursorWGL *acWGL = (AG_CursorWGL *)ac;

	if (ac == drv->activeCursor) {
		drv->activeCursor = NULL;
	}
	if (!acWGL->shared) {
		DestroyCursor(acWGL->cursor);
	}
	free(ac->data);
	free(ac->mask);
	free(ac);
}

static int
WGL_SetCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;
	AG_CursorWGL *acWGL = (AG_CursorWGL *)ac;

	if (drv->activeCursor == ac) {
		return (0);
	}
	SetCursor(acWGL->cursor);
	drv->activeCursor = ac;
	return (0);
}

static void
WGL_UnsetCursor(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	AG_DriverWGL *wgl = (AG_DriverWGL *)drv;
	
	switch (wgl->nchittest) {
	case HTBOTTOM:		SetCursor(LoadCursor(NULL, IDC_SIZENS));	break;
	case HTBOTTOMLEFT:	SetCursor(LoadCursor(NULL, IDC_SIZENESW));	break;
	case HTBOTTOMRIGHT:	SetCursor(LoadCursor(NULL, IDC_SIZENWSE));	break;
	case HTLEFT:
	case HTRIGHT:		SetCursor(LoadCursor(NULL, IDC_SIZEWE));	break;
	default:		SetCursor(LoadCursor(NULL, IDC_ARROW));		break;
	}
	drv->activeCursor = TAILQ_FIRST(&drv->cursors);
}

static int
WGL_GetCursorVisibility(void *_Nonnull obj)
{
	/* XXX TODO */
	return (1);
}

static void
WGL_SetCursorVisibility(void *_Nonnull obj, int flag)
{
	/* XXX TODO */
}

static void
WGL_PreResizeCallback(AG_Window *_Nonnull win)
{
#if 0
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;

	/*
	 * Backup all GL resources since it is not portable to assume that a
	 * display resize will not cause a change in GL contexts
	 * (XXX TODO test for platforms where this is unnecessary)
	 */
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	FreeWidgetResources(WIDGET(win));
	AG_ClearGlyphCache();
#endif
}

static void
WGL_PostResizeCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverWGL *wgl = (AG_DriverWGL *)drv;
	AG_SizeAlloc wa;
	AG_Rect rVP;
	const int x = (a->x == -1) ? WIDGET(win)->x : a->x;
	const int y = (a->y == -1) ? WIDGET(win)->y : a->y;

	wa.x = 0;
	wa.y = 0;
	wa.w = a->w;
	wa.h = a->h;
	AG_WidgetSizeAlloc(win, &wa);
	AG_WidgetUpdateCoords(win, 0,0);
	WIDGET(win)->x = x;
	WIDGET(win)->y = y;

	win->dirty = 1;

	wglMakeCurrent(wgl->hdc, wgl->hglrc);
	rVP.x = 0;
	rVP.y = 0;
	rVP.w = WIDTH(win);
	rVP.h = HEIGHT(win);
	AG_GL_SetViewport(&wgl->gl, &rVP);
}

static void
WGL_PostMoveCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_SizeAlloc wa;
	
	wa.x = 0;
	wa.y = 0;
	wa.w = a->w;
	wa.h = a->h;
	AG_WidgetSizeAlloc(win, &wa);
	AG_WidgetUpdateCoords(win, 0,0);
	WIDGET(win)->x = a->x;
	WIDGET(win)->y = a->y;

	win->dirty = 1;

	if (agWindowPinnedCount > 0)
		AG_WindowMovePinned(win, a->x - WIDGET(win)->x,
		                         a->y - WIDGET(win)->y);
}

AG_DriverMwClass agDriverWGL = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverWGL",
			sizeof(AG_DriverWGL),
			{ 1,6 },
			NULL,		/* init */
			NULL,		/* reset */
			NULL,		/* destroy */
			NULL,		/* load */
			NULL,		/* save */
			NULL,		/* edit */
		},
		"wgl",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES,
		WGL_Open,
		WGL_Close,
		WGL_GetDisplaySize,
		NULL,			/* beginEventProcessing */
		WGL_PendingEvents,
		WGL_GetNextEvent,
		WGL_ProcessEvent,
		NULL,			/* genericEventLoop */
		NULL,			/* endEventProcessing */
		NULL,			/* terminate */
		WGL_BeginRendering,
		WGL_RenderWindow,
		WGL_EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_StdUploadTexture,
		AG_GL_StdUpdateTexture,
		AG_GL_StdDeleteTexture,
		NULL,			/* setRefreshRate */
		AG_GL_StdPushClipRect,
		AG_GL_StdPopClipRect,
		AG_GL_StdPushBlendingMode,
		AG_GL_StdPopBlendingMode,
		WGL_CreateCursor,
		WGL_FreeCursor,
		WGL_SetCursor,
		WGL_UnsetCursor,
		WGL_GetCursorVisibility,
		WGL_SetCursorVisibility,
		AG_GL_BlitSurface,
		AG_GL_BlitSurfaceFrom,
#ifdef HAVE_OPENGL
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
#endif
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		AG_GL_PutPixel64,
		AG_GL_PutPixelRGB16,
#endif
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawLineW,
		AG_GL_DrawLineW_Sti16,
		AG_GL_DrawTriangle,
		AG_GL_DrawPolygon,
		AG_GL_DrawPolygon_Sti32,
		AG_GL_DrawArrow,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircleFilled,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		AG_GL_StdDeleteList,
		NULL,				/* getClipboardText */
		NULL				/* setClipboardText */
	},
	WGL_OpenWindow,
	WGL_CloseWindow,
	WGL_MapWindow,
	WGL_UnmapWindow,
	WGL_RaiseWindow,
	WGL_LowerWindow,
	WGL_ReparentWindow,
	WGL_GetInputFocus,
	WGL_SetInputFocus,
	WGL_MoveWindow,
	WGL_ResizeWindow,
	WGL_MoveResizeWindow,
	WGL_PreResizeCallback,
	WGL_PostResizeCallback,
	NULL,				/* setBorderWidth */
	WGL_SetWindowCaption,
	WGL_SetTransientFor,
	NULL,				/* setOpacity (TODO) */
	WGL_TweakAlignment
};

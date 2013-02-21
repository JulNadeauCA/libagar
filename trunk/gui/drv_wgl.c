/*
 * Copyright (c) 2009-2013 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/ag_threads.h>

#include <core/core.h>
#include <core/win32.h>

#include "gui.h"
#include "window.h"
#include "gui_math.h"
#include "text.h"
#include "cursors.h"
#include "opengl.h"

static int  nDrivers = 0;			/* Drivers open */
static int  wndClassCount = 1;			/* Window class counter */
static AG_DriverEventQ wglEventQ;		/* Event queue */
#ifdef AG_THREADS
static AG_Mutex wglClassLock;			/* Lock on wndClassCount */
static AG_Mutex wglEventLock;			/* Lock on wglEventQ */
#endif
static int  agExitWGL = 0;			/* Exit event loop */
static HKL wglKbdLayout = NULL;			/* Keyboard layout */
static AG_EventSink *wglEventSpinner = NULL;	/* Standard event sink */
static AG_EventSink *wglEventEpilogue = NULL;	/* Standard event epilogue */

/* Driver instance data */
typedef struct ag_driver_wgl {
	struct ag_driver_mw _inherit;
	HWND        hwnd;		/* Window handle */
	HDC         hdc;		/* Device context */
	HGLRC       hglrc;		/* Rendering context */
	WNDCLASSEX  wndclass;		/* Window class */
	AG_GL_Context gl;		/* Common OpenGL context data */
} AG_DriverWGL;

typedef struct ag_cursor_wgl {
	COLORREF black;
	COLORREF white;
	HCURSOR  cursor;
	int      visible;
} AG_CursorWGL;

AG_DriverMwClass agDriverWGL;

#define AGDRIVER_IS_WGL(drv) \
	(AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverWGL)

struct ag_windows_key_mapping {
	int kcode;			/* Windows VK_ virtual key */
	AG_KeySym key;			/* Agar keysym */
};
#include "drv_wgl_keymaps.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static int       InitDefaultCursor(AG_DriverWGL *);
static void      WGL_PostResizeCallback(AG_Window *, AG_SizeAlloc *);
static int       WGL_RaiseWindow(AG_Window *);
static int       WGL_SetInputFocus(AG_Window *);
static void      WGL_PostMoveCallback(AG_Window *, AG_SizeAlloc *);
static int       WGL_GetNextEvent(void *, AG_DriverEvent *);
static int       WGL_ProcessEvent(void *, AG_DriverEvent *);
static int       WGL_GetDisplaySize(Uint *, Uint *);
static int       WGL_PendingEvents(void *drvCaller);

static void
WGL_SetWindowsError(char* errorMessage, DWORD errorCode)
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
static AG_Window *
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
WGL_EventSink(AG_EventSink *es, AG_Event *event)
{
	AG_DriverEvent dev;

	if (!WGL_PendingEvents(NULL)) {
		return (0);
	}
	if (WGL_GetNextEvent(NULL, &dev) == 1) {
		return WGL_ProcessEvent(NULL, &dev);
	}
	return (0);
}
static int
WGL_EventEpilogue(AG_EventSink *es, AG_Event *event)
{
	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	return (0);
}

static int
WGL_Open(void *obj, const char *spec)
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
	if (wglEventSpinner != NULL) { AG_DelEventSpinner(wglEventSpinner); wglEventSpinner = NULL; }
	if (wglEventEpilogue != NULL) { AG_DelEventEpilogue(wglEventEpilogue); wglEventEpilogue = NULL; }
	if (drv->kbd != NULL) { AG_ObjectDelete(drv->kbd); drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
WGL_Close(void *obj)
{
	AG_Driver *drv = obj;
	AG_DriverEvent *dev, *devNext;
	AG_DriverWGL *wgl = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0) {
		AG_DelEventSink(wglEventSpinner); wglEventSpinner = NULL;
		AG_DelEventEpilogue(wglEventEpilogue); wglEventEpilogue = NULL;

		for (dev = TAILQ_FIRST(&wglEventQ);
		     dev != TAILQ_LAST(&wglEventQ, ag_driver_eventq);
		     dev = devNext) {
			devNext = TAILQ_NEXT(dev, events);
			Free(dev);
		}
		TAILQ_INIT(&wglEventQ);

		AG_MutexDestroy(&wglClassLock);
		AG_MutexDestroy(&wglEventLock);
	}

	AG_ObjectDelete(drv->mouse);	drv->mouse = NULL;
	AG_ObjectDelete(drv->kbd);	drv->kbd = NULL;
}

/* Return suitable window style from Agar window flags. */
static void
WGL_GetWndStyle(AG_Window *win, DWORD *wndStyle, DWORD *wndStyleEx)
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
WGL_GetWndRect(AG_Window *win, AG_Rect *r)
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
WGL_OpenWindow(AG_Window *win, AG_Rect r, int depthReq, Uint mwFlags)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	char wndClassName[64]; 
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
	AG_SizeAlloc a;

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
	
	/*
	 * XXX TODO it would be best to pass CW_USEDEFAULT here, but
	 * I could not find a way to retrieve the final allocated
	 * coordinates; GetWindowRect() for instance does not seem
	 * to work as expected. For now we'll just center the window.
	 */
	if (mwFlags & AG_DRIVER_MW_ANYPOS) {
		Uint wDisp, hDisp;

		if (WGL_GetDisplaySize(&wDisp, &hDisp) == 0) {
			r.x = wDisp/2 - r.w/2;
			r.y = hDisp/2 - r.h/2;
		}
	}
	
	/* Adjust window with account for window borders, if any */
	wndRect.left = r.x;
	wndRect.top = r.y;
	wndRect.right = r.x + r.w;
	wndRect.bottom = r.y + r.h;
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
	if (AG_GL_InitContext(wgl, &wgl->gl) == -1) {
		return (-1);
	}
	AG_GL_SetViewport(&wgl->gl, AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	/* Show the window */
	ShowWindow(wgl->hwnd, SW_SHOW);
	if (!(win->flags & AG_WINDOW_KEEPBELOW)) {
		SetForegroundWindow(wgl->hwnd);
		/* SetCapture(wgl->hwnd); */
	}
	
	/* Set the pixel format */
	drv->videoFmt = AG_PixelFormatRGB(16, 0x000000ff, 0x0000ff00, 0x00ff0000);
	if (drv->videoFmt == NULL)
		goto fail;

	/* Create the built-in cursors */
	if (InitDefaultCursor(wgl) == -1 || AG_InitStockCursors(drv) == -1)
		goto fail;

	/* Update agar's idea of the actual window coordinates. */
	a.x = r.x;
	a.y = r.y;
	a.w = r.w;
	a.h = r.h;
	AG_WidgetSizeAlloc(win, &a);
	AG_WidgetUpdateCoords(win, a.x, a.y);

	/* Focus the window. */
	if (!(win->flags & AG_WINDOW_DENYFOCUS)) {
		SetFocus(wgl->hwnd);
		agWindowFocused = win;
		AG_PostEvent(NULL, win, "window-gainfocus", NULL);
	}
	
	return (0);
fail:
	wglDeleteContext(wgl->hglrc);
	DestroyWindow(wgl->hwnd);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static void
WGL_CloseWindow(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverWGL *wgl = (AG_DriverWGL *)drv;

	/* Destroy our OpenGL context. */
	wglMakeCurrent(wgl->hdc, wgl->hglrc);
	AG_GL_DestroyContext(wgl);
	wglDeleteContext(wgl->hglrc);

	/* Close the window. */
	DestroyWindow(wgl->hwnd);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
}

static int
WGL_GetDisplaySize(Uint *w, Uint *h)
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

/* 
 * Window procedure. We only translate and queue events for later retrieval
 * by getNextEvent().
 */
LRESULT CALLBACK
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AG_Driver *drv;
	AG_Window *win;
	AG_DriverEvent *dev;
	int x, y;
	
	AG_LockVFS(&agDrivers);
	
	if ((win = LookupWindowByID(hWnd)) == NULL) {
		goto fallback;
	}
	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) == NULL) {
		goto fallback;
	}
	dev->win = win;
	drv = WIDGET(win)->drv;
#ifdef DEBUG_WGL
	WGL_Print_WinMsg(win, uMsg, wParam, lParam);
#endif
	switch (uMsg) {
	case WM_MOUSEMOVE:
		dev->type = AG_DRIVER_MOUSE_MOTION;
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
		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
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
		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
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
			dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
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

			if (uMsg == WM_KEYDOWN) {
				dev->type = AG_DRIVER_KEY_DOWN;
				ka = AG_KEY_PRESSED;
			} else {
				dev->type = AG_DRIVER_KEY_UP;
				ka = AG_KEY_RELEASED;
			}
			dev->data.key.ucs = 0;

			switch (vKey) {
			case VK_CONTROL:
				vKey = (lParam & (1<<24)) ? VK_RCONTROL :
				                            VK_LCONTROL;
				break;
			case VK_RETURN:
				if (IN_KEYPAD(scan)) {
					AG_KeyboardUpdate(drv->kbd, ka,
					    AG_KEY_KP_ENTER, 0);
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
			AG_KeyboardUpdate(drv->kbd, ka, dev->data.key.ks,
			    dev->data.key.ucs);
		}
		break;
	case WM_SETFOCUS:
		if (win->visible) {
			/* SetCapture(hWnd); */
			dev->type = AG_DRIVER_FOCUS_IN;
			goto ret0;
		} else {
			goto fallback;
		}
	case WM_KILLFOCUS:
		if (win->visible) {
			dev->type = AG_DRIVER_FOCUS_OUT;
			goto ret0;
		} else {
			goto fallback;
		}
	case WM_SIZE:
		if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
			dev->type = AG_DRIVER_VIDEORESIZE;
			dev->data.videoresize.x = -1;
			dev->data.videoresize.y = -1;
			dev->data.videoresize.w = LOWORD(lParam);
			dev->data.videoresize.h = HIWORD(lParam);
		} else {
			Free(dev);
			goto fallback;
		}
		goto ret0;
	case WM_MOVE:
		dev->type = AG_DRIVER_UNKNOWN;
		WIDGET(win)->x = (int)(short)LOWORD(lParam);
		WIDGET(win)->y = (int)(short)HIWORD(lParam);
		goto ret0;
#if 0
	/*
	 * XXX TODO: use TrackMouseEvent(), translate WM_MOUSEHOVER
	 * events to AG_DRIVER_MOUSE_ENTER.
	 */
	case WM_MOUSEHOVER:
		break;
#endif
	case WM_MOUSELEAVE:
		dev->type = AG_DRIVER_MOUSE_LEAVE;
		dev->win = win;
		break;
	case WM_ERASEBKGND:
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = win;
		goto ret0;
	case WM_CLOSE:
		dev->type = AG_DRIVER_CLOSE;
		dev->win = win;
		goto ret0;
	default:
		Free(dev);
		goto fallback;
	}
out:
	TAILQ_INSERT_TAIL(&wglEventQ, dev, events);
	AG_UnlockVFS(&agDrivers);
	return (1);
ret0:
	TAILQ_INSERT_TAIL(&wglEventQ, dev, events);
	AG_UnlockVFS(&agDrivers);
	return (0);
fallback:
	AG_UnlockVFS(&agDrivers);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static __inline__ int
WGL_PendingEvents(void *drvCaller)
{
	return (!TAILQ_EMPTY(&wglEventQ) ||
	        GetQueueStatus(QS_ALLINPUT) != 0);
}

static int
WGL_GetNextEvent(void *drvCaller, AG_DriverEvent *dev)
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
	TAILQ_REMOVE(&wglEventQ, devFirst, events);
	memcpy(dev, devFirst, sizeof(AG_DriverEvent));
	Free(devFirst);
	return (1);
}

static int
WGL_ProcessEvent(void *drvCaller, AG_DriverEvent *dev)
{
	AG_Driver *drv;
	AG_SizeAlloc a;
	int rv = 1;

	if (dev->win == NULL)
		return (0);

	AG_LockVFS(&agDrivers);
	drv = WIDGET(dev->win)->drv;

	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		AG_ProcessMouseMotion(dev->win,
		    dev->data.motion.x, dev->data.motion.y,
		    drv->mouse->xRel, drv->mouse->yRel,
		    drv->mouse->btnState);
		AG_MouseCursorUpdate(dev->win,
		     dev->data.motion.x, dev->data.motion.y);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		AG_ProcessMouseButtonDown(dev->win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		AG_ProcessMouseButtonUp(dev->win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_KEY_UP:
		AG_ProcessKey(drv->kbd, dev->win, AG_KEY_RELEASED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_KEY_DOWN:
		AG_ProcessKey(drv->kbd, dev->win, AG_KEY_PRESSED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_MOUSE_ENTER:
		AG_PostEvent(NULL, dev->win, "window-enter", NULL);
		break;
	case AG_DRIVER_MOUSE_LEAVE:
		AG_PostEvent(NULL, dev->win, "window-leave", NULL);
		break;
	case AG_DRIVER_FOCUS_IN:
		agWindowFocused = dev->win;
		AG_PostEvent(NULL, dev->win, "window-gainfocus", NULL);
		break;
	case AG_DRIVER_FOCUS_OUT:
		if (dev->win == agWindowFocused) {
			AG_PostEvent(NULL, dev->win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		break;
	case AG_DRIVER_VIDEORESIZE:
		a.x = dev->data.videoresize.x;
		a.y = dev->data.videoresize.y;
		a.w = dev->data.videoresize.w;
		a.h = dev->data.videoresize.h;
		if (a.w != WIDTH(dev->win) || a.h != HEIGHT(dev->win)) {
			WGL_PostResizeCallback(dev->win, &a);
		} else {
			WGL_PostMoveCallback(dev->win, &a);
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(NULL, dev->win, "window-close", NULL);
		break;
	case AG_DRIVER_EXPOSE:
		dev->win->dirty = 1;
		break;
	default:
		rv = 0;
		break;
	}
	AG_UnlockVFS(&agDrivers);
	return (rv);
}

static void
WGL_BeginRendering(void *obj)
{
	AG_DriverWGL *wgl = obj;

	wglMakeCurrent(wgl->hdc, wgl->hglrc);
}

static void
WGL_RenderWindow(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_GL_Context *gl = &wgl->gl;
	AG_Color c;

	gl->clipStates[0] = glIsEnabled(GL_CLIP_PLANE0); glEnable(GL_CLIP_PLANE0);
	gl->clipStates[1] = glIsEnabled(GL_CLIP_PLANE1); glEnable(GL_CLIP_PLANE1);
	gl->clipStates[2] = glIsEnabled(GL_CLIP_PLANE2); glEnable(GL_CLIP_PLANE2);
	gl->clipStates[3] = glIsEnabled(GL_CLIP_PLANE3); glEnable(GL_CLIP_PLANE3);
	
	c = WCOLOR(win,0);
	glClearColor(c.r/255.0,
	             c.g/255.0,
		     c.b/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	AG_WidgetDraw(win);
}

static void
WGL_EndRendering(void *obj)
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
WGL_MapWindow(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	ShowWindow(wgl->hwnd, SW_SHOW);
	return (0);
}

static int
WGL_UnmapWindow(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	ShowWindow(wgl->hwnd, SW_HIDE);
	return (0);
}

static int
WGL_RaiseWindow(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowPos(wgl->hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return (0);
}

static int
WGL_LowerWindow(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowPos(wgl->hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return (0);
}

static int
WGL_ReparentWindow(AG_Window *win, AG_Window *winParent, int x, int y)
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

	SetWindowPos(wgl->hwnd, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
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
	SetWindowPos(wgl->hwnd, NULL, 0, 0, r.w, r.h,
	    SWP_NOZORDER|SWP_NOMOVE);	

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
WGL_SetBorderWidth(AG_Window *win, Uint width)
{
	/* There is no border width in win32! */
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
static int
InitDefaultCursor(AG_DriverWGL *wgl)
{
	AG_Driver *drv = AGDRIVER(wgl);
	AG_Cursor *ac;
	struct ag_cursor_wgl *cg;
	
	if ((cg = TryMalloc(sizeof(struct ag_cursor_wgl))) == NULL)
		return (-1);
	if ((drv->cursors = TryMalloc(sizeof(AG_Cursor))) == NULL) {
		Free(cg);
		return (-1);
	}

	ac = &drv->cursors[0];
	drv->nCursors = 1;
	AG_CursorInit(ac);
	cg->cursor = LoadCursor(NULL, IDC_ARROW);
	cg->visible = 1;
	ac->p = cg;
	return (0);
}

static int
WGL_CreateCursor(void *obj, AG_Cursor *ac)
{
	AG_CursorWGL *cg;
	int          size, i;
	BYTE         *xorMask, *andMask;

	if ((cg = TryMalloc(sizeof(AG_CursorWGL))) == NULL) {
		return (-1);
	}
	cg->black = RGB(0, 0, 0);
	cg->white = RGB(0xFF, 0xFF, 0xFF);
	
	/* Calc size for cursor data */
	size = (ac->w / 8) * ac->h;

	/* Allocate memory for xorMask (which represents the cursor data) */
	if ((xorMask = TryMalloc(size)) == NULL) {
		Free(cg);
		return (-1);
	}

	/* Allocate memory for andMask (which represents the transparence) */
	if ((andMask = TryMalloc(size)) == NULL) {
		Free(xorMask);
		Free(cg);
		return (-1);
	}

	/* Copy cursor data into buffers for use with CreateCursor */
	for (i = 0; i < size; i++) {
		andMask[i] = ~ac->mask[i];
		xorMask[i] = ~ac->data[i] ^ ~ac->mask[i];
	}

	/* Create cursor */
	if ((cg->cursor = CreateCursor(GetModuleHandle(NULL), 
	    ac->xHot, ac->yHot, ac->w, ac->h, andMask, xorMask))) {
		cg->visible = 0;
		ac->p = cg;
		return (0);
	}
	
	WGL_SetWindowsError("CreateCursor failed!", GetLastError());
	return (-1);
}

static void
WGL_FreeCursor(void *obj, AG_Cursor *ac)
{
	AG_CursorWGL *cg = ac->p;
	
	DestroyCursor(cg->cursor);
	Free(cg);
	ac->p = NULL;
}

static int
WGL_SetCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;
	AG_CursorWGL *cg = ac->p;

	if (drv->activeCursor == ac) {
		return (0);
	}
	if (ac == &drv->cursors[0]) {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	} else {
		SetCursor(cg->cursor);
	}
	drv->activeCursor = ac;
	cg->visible = 1;
	return (0);
}

static void
WGL_UnsetCursor(void *obj)
{
	AG_Driver *drv = obj;
	
	if (drv->activeCursor == &drv->cursors[0]) {
		return;
	}
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	drv->activeCursor = &drv->cursors[0];
}

static int
WGL_GetCursorVisibility(void *obj)
{
	/* XXX TODO */
	return (1);
}

static void
WGL_SetCursorVisibility(void *obj, int flag)
{
	/* XXX TODO */
}

static void
WGL_PreResizeCallback(AG_Window *win)
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
WGL_PostResizeCallback(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverWGL *wgl = (AG_DriverWGL *)drv;
	int x = (a->x == -1) ? WIDGET(win)->x : a->x;
	int y = (a->y == -1) ? WIDGET(win)->y : a->y;
	
	/* Update per-widget coordinate information */
	a->x = 0;
	a->y = 0;
	AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Viewport dimensions have changed */
	wglMakeCurrent(wgl->hdc, wgl->hglrc);
	AG_GL_SetViewport(&wgl->gl, AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
}

static void
WGL_PostMoveCallback(AG_Window *win, AG_SizeAlloc *a)
{
	AG_SizeAlloc aNew;
	int xRel, yRel;
	
	xRel = a->x - WIDGET(win)->x;
	yRel = a->y - WIDGET(win)->y;

	/* Update the window coordinates. */
	aNew.x = 0;
	aNew.y = 0;
	aNew.w = a->w;
	aNew.h = a->h;
	AG_WidgetSizeAlloc(win, &aNew);
	AG_WidgetUpdateCoords(win, 0, 0);
	WIDGET(win)->x = a->x;
	WIDGET(win)->y = a->y;
	win->dirty = 1;

	/* Move other windows pinned to this one. */
	AG_WindowMovePinned(win, xRel, yRel);
}

AG_DriverMwClass agDriverWGL = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverWGL",
			sizeof(AG_DriverWGL),
			{ 1,5 },
			NULL,	/* init */
			NULL,	/* reinit */
			NULL,	/* destroy */
			NULL,	/* load */
			NULL,	/* save */
			NULL,	/* edit */
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
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB,
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawArrowUp,
		AG_GL_DrawArrowDown,
		AG_GL_DrawArrowLeft,
		AG_GL_DrawArrowRight,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircleFilled,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		AG_GL_StdDeleteList
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
	NULL,				/* captureWindow */
	WGL_SetBorderWidth,
	WGL_SetWindowCaption,
	WGL_SetTransientFor,
	NULL,				/* setOpacity (TODO) */
	WGL_TweakAlignment
};

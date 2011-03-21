/*
 * Copyright (c) 2009-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/have_wgl.h>
#include <config/ag_threads.h>
#ifdef HAVE_WGL

#include <core/core.h>
#include <core/config.h>

#include <core/win32.h>

#include "gui.h"
#include "window.h"
#include "perfmon.h"
#include "gui_math.h"
#include "text.h"
#include "cursors.h"
#include "opengl.h"

static int  nDrivers = 0;		/* Drivers open */
static Uint rNom = 16;			/* Nominal refresh rate (ms) */
static int  rCur = 0;			/* Effective refresh rate (ms) */
static int  wndClassCount = 1;		/* Window class counter */
static AG_DriverEventQ wglEventQ;	/* Event queue */
#ifdef AG_THREADS
static AG_Mutex wglClassLock;		/* Lock on wndClassCount */
static AG_Mutex wglEventLock;		/* Lock on wglEventQ */
#endif
static int  agExitWGL = 0;		/* Exit event loop */

/* Driver instance data */
typedef struct ag_driver_wgl {
	struct ag_driver_mw _inherit;
	HWND        hwnd;		/* Window handle */
	HDC         hdc;		/* Device context */
	HGLRC       hglrc;		/* Rendering context */
	WNDCLASSEX  wndclass;		/* Window class */

	int          clipStates[4];	/* Clipping GL state */
	AG_ClipRect *clipRects;		/* Clipping rectangles */
	Uint        nClipRects;
	Uint        *textureGC;		/* Textures queued for deletion */
	Uint        nTextureGC;
	Uint        *listGC;		/* Display lists queued for deletion */
	Uint        nListGC;
	AG_GL_BlendState bs[1];		/* Saved blending states */
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

struct ag_key_mapping {			/* Keymap translation table entry */
	int kcode;			/* Scancode */
	int kclass;			/* X keysym class (e.g., 0xff) */
	AG_KeySym key;			/* Corresponding Agar keysym */
};
#include "drv_wgl_keymaps.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static int       InitClipRects(AG_DriverWGL *, int, int);
static void      InitClipRect0(AG_DriverWGL *, AG_Window *);
static int       InitDefaultCursor(AG_DriverWGL *);
static void      WGL_PostResizeCallback(AG_Window *, AG_SizeAlloc *);
static int       WGL_RaiseWindow(AG_Window *);
static int       WGL_SetInputFocus(AG_Window *);
static void      WGL_PostMoveCallback(AG_Window *, AG_SizeAlloc *);
static int       WGL_GetNextEvent(void *, AG_DriverEvent *);
static int       WGL_ProcessEvent(void *, AG_DriverEvent *);
static int       WGL_GetDisplaySize(Uint *, Uint *);

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

	/* XXX TODO portable to optimize based on numerical XIDs? */
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

static void
WGL_Init(void *obj)
{
	AG_DriverWGL *wgl = obj;
	
	wgl->clipRects = NULL;
	wgl->nClipRects = 0;
	wgl->textureGC = NULL;
	wgl->nTextureGC = 0;
	wgl->listGC = NULL;
	wgl->nListGC = 0;

	memset(wgl->clipStates, 0, sizeof(wgl->clipStates));
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
	
	if (nDrivers == 0) {
		InitKeymaps();
		TAILQ_INIT(&wglEventQ);
		AG_MutexInitRecursive(&wglClassLock);
		AG_MutexInitRecursive(&wglEventLock);
	}
	nDrivers++;
	return (0);
fail:
	if (drv->kbd != NULL) {
		AG_ObjectDetach(drv->kbd);
		AG_ObjectDestroy(drv->kbd);
		drv->kbd = NULL;
	}
	if (drv->mouse != NULL) {
		AG_ObjectDetach(drv->mouse);
		AG_ObjectDestroy(drv->mouse);
		drv->mouse = NULL;
	}
	return (-1);
}

static void
WGL_Close(void *obj)
{
	AG_Driver *drv = obj;
	AG_DriverEvent *dev, *devNext;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0) {
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

	AG_ObjectDetach(drv->mouse);
	AG_ObjectDestroy(drv->mouse);
	AG_ObjectDetach(drv->kbd);
	AG_ObjectDestroy(drv->kbd);
	
	drv->mouse = NULL;
	drv->kbd = NULL;
}

static void
WGL_Destroy(void *obj)
{
	AG_DriverWGL *wgl = obj;
	
	Free(wgl->clipRects);
	Free(wgl->textureGC);
	Free(wgl->listGC);
}

/* Return suitable window style from Agar window flags. */
static void
WGL_GetWndStyle(AG_Window *win, DWORD *wndStyle, DWORD *wndStyleEx)
{
	*wndStyle = ((win->flags & AG_WINDOW_POPUP) ||
	             (win->flags & AG_WINDOW_DIALOG) ||
	             (win->flags & AG_WINDOW_NOTITLE)) ?
		     WS_POPUP : WS_OVERLAPPEDWINDOW;
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
	if ((win->flags & AG_WINDOW_NOTITLE) || (win->flags & AG_WINDOW_POPUP))
		(*wndStyleEx) |= WS_EX_TOOLWINDOW;
#endif
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
	if (win->flags & AG_WINDOW_KEEPBELOW)
		(*wndStyleEx) |= WS_EX_NOACTIVATE;
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
	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	/* Show the window */
	ShowWindow(wgl->hwnd, SW_SHOW);
	if (!(win->flags & AG_WINDOW_KEEPBELOW)) {
		SetForegroundWindow(wgl->hwnd);
	}
	AGDRIVER_MW(wgl)->flags |= AG_DRIVER_MW_OPEN;

	/* Set the pixel format */
	drv->videoFmt = AG_PixelFormatRGB(16, 0x000000ff, 0x0000ff00, 0x00ff0000);
	if (drv->videoFmt == NULL)
		goto fail;

	/* Initialize the clipping rectangle stack */
	if (InitClipRects(wgl, r.w, r.h) == -1)
		goto fail;
	
	/* Create the built-in cursors */
	if (InitDefaultCursor(wgl) == -1 || AG_InitStockCursors(drv) == -1)
		goto fail;

	/* Update agar's idea of the actual window coordinates. */
	a.x = r.x;
	a.y = r.y;
	a.w = r.w;
	a.h = r.h;
	if (AG_WidgetSizeAlloc(win, &a) == 0)
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
	AGDRIVER_MW(wgl)->flags &= (~AG_DRIVER_MW_OPEN);
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
	AG_Glyph *gl;
	int i;

	wglMakeCurrent(wgl->hdc, wgl->hglrc);

	/* Invalidate cached glyph textures. */
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		SLIST_FOREACH(gl, &drv->glyphCache[i].glyphs, glyphs) {
			if (gl->texture != 0) {
				glDeleteTextures(1, (GLuint *)&gl->texture);
				gl->texture = 0;
			}
		}
	}

	wglDeleteContext(wgl->hglrc);
	DestroyWindow(wgl->hwnd);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	AGDRIVER_MW(wgl)->flags &= ~(AG_DRIVER_MW_OPEN);
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
	AG_KeyboardAction ka;
	HKL kLayout;
	unsigned char kState[256];
	unsigned short kResult = 0;
	int x, y, ret;
	
	if ((win = LookupWindowByID(hWnd)) == NULL) {
		goto fallback;
	}
	if ((dev = TryMalloc(sizeof(AG_DriverEvent))) == NULL) {
		goto fallback;
	}
	dev->win = win;
	drv = WIDGET(win)->drv;

	switch (uMsg) {
	case WM_MOUSEMOVE:
		dev->type = AG_DRIVER_MOUSE_MOTION;
		x = (int)LOWORD(lParam);
		y = (int)HIWORD(lParam);
		dev->data.motion.x = AGDRIVER_BOUNDED_WIDTH(win, x);
		dev->data.motion.y = AGDRIVER_BOUNDED_HEIGHT(win, y);
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
/*#if (_WIN32_WINNT >= 0x400) || (_WIN32_WINDOWS > 0x400) */
#ifdef WM_MOUSEWHEEL
	case WM_MOUSEWHEEL:
		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->data.button.which =
		    (short)HIWORD(wParam) > 0 ?
		    AG_MOUSE_WHEELUP :
		    AG_MOUSE_WHEELDOWN;
		x = (int)LOWORD(lParam);
		y = (int)HIWORD(lParam);
		dev->data.button.x = AGDRIVER_BOUNDED_WIDTH(win, x);
		dev->data.button.y = AGDRIVER_BOUNDED_HEIGHT(win, y);
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED, 
		    dev->data.button.which);
		break;
#endif
	case WM_KEYUP:
	case WM_KEYDOWN:
		if (uMsg == WM_KEYDOWN) {
			dev->type = AG_DRIVER_KEY_DOWN;
			ka = AG_KEY_PRESSED;
		} else {
			dev->type = AG_DRIVER_KEY_UP;
			ka = AG_KEY_RELEASED;
		}
		kLayout = GetKeyboardLayout(0);
		GetKeyboardState(kState);
		ret = ToAsciiEx(wParam, HIWORD(lParam) & 0xFF, kState, 
		    &kResult, 0, kLayout);
		if (ret == 1) {
			/* XXX @todo Do we have to take notice of ucs here? */
			AG_KeyboardUpdate(drv->kbd, ka, kResult, kResult);
			dev->data.key.ks = kResult;
			dev->data.key.ucs = (Uint32)kResult;
		} else {
			/* Special keycode, search it in map */
			AG_KeySym sym = agKeymapMisc[wParam&0xff];
			AG_KeyboardUpdate(drv->kbd, ka, sym, sym);
			dev->data.key.ks = sym;
			dev->data.key.ucs = (Uint32)sym;
		}
		break;
	case WM_SETFOCUS:
		if (win->visible) {
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
	TAILQ_INSERT_TAIL(&wglEventQ, dev, events);
	return (1);
ret0:
	TAILQ_INSERT_TAIL(&wglEventQ, dev, events);
	return (0);
fallback:
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
WGL_PostEventCallback(void *drvCaller)
{
	AG_Window *win;
	AG_Driver *drv;
	
	AG_LockVFS(&agDrivers);

	if (!TAILQ_EMPTY(&agWindowDetachQ))
		AG_FreeDetachedWindows();
	
	/*
	 * Exit when no more visible windows exist.
	 * XXX TODO make this behavior configurable
	 */
	if (TAILQ_EMPTY(&OBJECT(&agDrivers)->children)) {
		goto nowindows;
	}
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(win, drv) {
			if (win->visible)
				break;
		}
		if (win != NULL)
			break;
	}
	if (win == NULL)
		goto nowindows;

	if (agWindowToFocus != NULL) {
		AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(agWindowToFocus)->drv;

		if (wgl != NULL && AGDRIVER_IS_WGL(wgl)) {
			WGL_RaiseWindow(agWindowToFocus);
			WGL_SetInputFocus(agWindowToFocus);
		}
		agWindowToFocus = NULL;
	}
	AG_UnlockVFS(&agDrivers);
	return (1);
nowindows:
	AG_SetError("No more windows exist");
	agTerminating = 1;
	AG_UnlockVFS(&agDrivers);
	return (-1);
}

static int
WGL_ProcessEvent(void *drvCaller, AG_DriverEvent *dev)
{
	AG_Driver *drv;
	AG_SizeAlloc a;

	if (dev->win == NULL) {
		return (0);
	}
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (WIDGET(dev->win)->drv == drv)
			break;
	}
	if (drv == NULL) {
		return (0);
	}
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
		return (0);
	}
	return WGL_PostEventCallback(drvCaller);
}

static void
WGL_GenericEventLoop(void *obj)
{
	AG_Driver *drv;
	AG_Window *win;
	Uint32 t1, t2;

#ifdef AG_DEBUG
	AG_PerfMonInit();
#endif
	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();
		if (agExitWGL) {
			break;
		} else if (t2 - t1 >= rNom) {
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (!AGDRIVER_IS_WGL(drv)) {
					continue;
				}
				win = AGDRIVER_MW(drv)->win;
				if (!win->visible || !win->dirty) {
					continue;
				}
				AG_BeginRendering(drv);
				AG_ObjectLock(win);
				AG_WindowDraw(win);
				AG_ObjectUnlock(win);
				AG_EndRendering(drv);
			}
			t1 = AG_GetTicks();
			rCur = rNom - (t1-t2);
			if (rCur < 1) { rCur = 1; }
#ifdef AG_DEBUG
			if (agPerfWindow->visible)
				AG_PerfMonUpdate(rCur);
#endif
		} else if (WGL_PendingEvents(NULL)) {
			AG_DriverEvent dev;

			if (WGL_GetNextEvent(NULL, &dev) == 1 &&
			    WGL_ProcessEvent(NULL, &dev) == -1) {
				return;
			}
#ifdef AG_DEBUG
			agEventAvg++;
#endif
		} else if (AG_TIMEOUTS_QUEUED()) {		/* Safe */
			AG_ProcessTimeouts(t2);
		} else {
			AG_Delay(1);
		}
	}
}

static void
WGL_Terminate(void)
{
	/* XXX TODO */
	agExitWGL = 1;
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

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	wgl->clipStates[0] = glIsEnabled(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE0);
	wgl->clipStates[1] = glIsEnabled(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE1);
	wgl->clipStates[2] = glIsEnabled(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE2);
	wgl->clipStates[3] = glIsEnabled(GL_CLIP_PLANE3);
	glEnable(GL_CLIP_PLANE3);

	AG_WidgetDraw(win);
}

static void
WGL_EndRendering(void *obj)
{
	AG_DriverWGL *wgl = obj;
	Uint i;
	
	SwapBuffers(wgl->hdc);

	/* Remove textures and display lists queued for deletion */
	glDeleteTextures(wgl->nTextureGC, wgl->textureGC);
	for (i = 0; i < wgl->nListGC; i++) {
		glDeleteLists(wgl->listGC[i], 1);
	}
	wgl->nTextureGC = 0;
	wgl->nListGC = 0;
}

static void
WGL_DeleteTexture(void *drv, Uint texture)
{
	AG_DriverWGL *wgl = drv;

	wgl->textureGC = Realloc(wgl->textureGC, (wgl->nTextureGC+1)*sizeof(Uint));
	wgl->textureGC[wgl->nTextureGC++] = texture;
}

static void
WGL_DeleteList(void *drv, Uint list)
{
	AG_DriverWGL *wgl = drv;

	wgl->listGC = Realloc(wgl->listGC, (wgl->nListGC+1)*sizeof(Uint));
	wgl->listGC[wgl->nListGC++] = list;
}

/*
 * Clipping and blending control (rendering context)
 */

static void
WGL_PushClipRect(void *obj, AG_Rect r)
{
	AG_DriverWGL *wgl = obj;
	AG_ClipRect *cr, *crPrev;

	wgl->clipRects = Realloc(wgl->clipRects, (wgl->nClipRects+1)*
	                                         sizeof(AG_ClipRect));
	crPrev = &wgl->clipRects[wgl->nClipRects-1];
	cr = &wgl->clipRects[wgl->nClipRects++];

	cr->eqns[0][0] = 1.0;
	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;
	cr->eqns[0][3] = MIN(crPrev->eqns[0][3], -(double)(r.x));
	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	
	cr->eqns[1][0] = 0.0;
	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;
	cr->eqns[1][3] = MIN(crPrev->eqns[1][3], -(double)(r.y));
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
		
	cr->eqns[2][0] = -1.0;
	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;
	cr->eqns[2][3] = MIN(crPrev->eqns[2][3], (double)(r.x+r.w));
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
		
	cr->eqns[3][0] = 0.0;
	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;
	cr->eqns[3][3] = MIN(crPrev->eqns[3][3], (double)(r.y+r.h));
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
}

static void
WGL_PopClipRect(void *obj)
{
	AG_DriverWGL *wgl = obj;
	AG_ClipRect *cr;
	
#ifdef AG_DEBUG
	if (wgl->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &wgl->clipRects[wgl->nClipRects-2];
	wgl->nClipRects--;

	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
}


/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverWGL *wgl, int w, int h)
{
	AG_ClipRect *cr;
	int i;

	for (i = 0; i < 4; i++)
		wgl->clipStates[i] = 0;

	/* Rectangle 0 always covers the whole view. */
	if ((wgl->clipRects = TryMalloc(sizeof(AG_ClipRect))) == NULL) {
		return (-1);
	}
	wgl->nClipRects = 1;

	cr = &wgl->clipRects[0];
	cr->r = AG_RECT(0,0, w,h);

	cr->eqns[0][0] = 1.0;	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;	cr->eqns[0][3] = 0.0;
	cr->eqns[1][0] = 0.0;	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;	cr->eqns[1][3] = 0.0;
	cr->eqns[2][0] = -1.0;	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;	cr->eqns[2][3] = (double)w;
	cr->eqns[3][0] = 0.0;	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;	cr->eqns[3][3] = (double)h;

	return (0);
}

/* Initialize clipping rectangle 0 for the current window geometry. */
static void
InitClipRect0(AG_DriverWGL *wgl, AG_Window *win)
{
	AG_ClipRect *cr;

	cr = &wgl->clipRects[0];
	cr->r.w = WIDTH(win);
	cr->r.h = HEIGHT(win);
	cr->eqns[2][3] = (double)WIDTH(win);
	cr->eqns[3][3] = (double)HEIGHT(win);
}

static void
WGL_PushBlendingMode(void *obj, AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	AG_DriverWGL *wgl = obj;

	/* XXX TODO: stack */
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
	    &wgl->bs[0].texEnvMode);
	glGetBooleanv(GL_BLEND, &wgl->bs[0].enabled);
	glGetIntegerv(GL_BLEND_SRC, &wgl->bs[0].srcFactor);
	glGetIntegerv(GL_BLEND_DST, &wgl->bs[0].dstFactor);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(AG_GL_GetBlendingFunc(fnSrc), AG_GL_GetBlendingFunc(fnDst));
}

static void
WGL_PopBlendingMode(void *obj)
{
	AG_DriverWGL *wgl = obj;

	/* XXX TODO: stack */
	if (wgl->bs[0].enabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
	glBlendFunc(wgl->bs[0].srcFactor, wgl->bs[0].dstFactor);
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
	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Update clipping rectangle 0 */
	InitClipRect0(wgl, win);
	
	/* Update WGL context */
	wglMakeCurrent(wgl->hdc, wgl->hglrc);
	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
}

static void
WGL_PostMoveCallback(AG_Window *win, AG_SizeAlloc *a)
{
	int x = a->x;
	int y = a->y;

	/* Update per-widget coordinate information. */
	a->x = 0;
	a->y = 0;
	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
}

static int
WGL_SetRefreshRate(void *obj, int fps)
{
	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	rNom = 1000/fps;
	rCur = 0;
	return (0);
}


AG_DriverMwClass agDriverWGL = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverWGL",
			sizeof(AG_DriverWGL),
			{ 1,4 },
			WGL_Init,
			NULL,	/* reinit */
			WGL_Destroy,
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
		WGL_GenericEventLoop,
		NULL,			/* endEventProcessing */
		WGL_Terminate,
		WGL_BeginRendering,
		WGL_RenderWindow,
		WGL_EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_UploadTexture,
		AG_GL_UpdateTexture,
		WGL_DeleteTexture,
		WGL_SetRefreshRate,
		WGL_PushClipRect,
		WGL_PopClipRect,
		WGL_PushBlendingMode,
		WGL_PopBlendingMode,
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
		AG_GL_DrawCircle2,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		WGL_DeleteList
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
	WGL_SetTransientFor
};

#endif /* HAVE_WGL */

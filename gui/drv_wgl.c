/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
#ifdef HAVE_WGL

// XXX
#include <windows.h>
#undef SLIST_ENTRY // @todo core_close?

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "window.h"
#include "perfmon.h"
#include "gui_math.h"
#include "text.h"
#include "cursors.h"

#include "drv_gl_common.h"

static int  inited = 0;
static int  nDrivers = 0;	   /* Drivers open */
static Uint rNom = 16;		   /* Nominal refresh rate (ms) */
static int  rCur = 0;		   /* Effective refresh rate (ms) */
static int  wndClassCount = 1; // Window class counter

// Driver instance data
typedef struct ag_driver_wgl {
	struct ag_driver_mw _inherit;
	HWND                hwnd;          // Window handle
	HDC                 hdc;           // Device context
	HGLRC               hglrc;         // Rendering context
	WNDCLASSEX          wndclass;      // Window class
	int                 clipStates[4]; // Clipping GL state 
	AG_ClipRect         *clipRects;	   // Clipping rectangles
	Uint                nClipRects;
	Uint                *textureGC;	   // Textures queued for deletion
	Uint                nTextureGC;
	AG_GL_BlendState    bs[1];		   // Saved blending states
	AG_Cursor           *cursorToSet;  // Set cursor at end of event cycle 
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int       InitClipRects(AG_DriverWGL *wgl, int w, int h);
static void      ChangeCursor(AG_DriverWGL *wgl);
static int       InitDefaultCursor(AG_DriverWGL *wgl);
static void      WGL_PostResizeCallback(AG_Window *win, AG_SizeAlloc *a);

void
_SetWindowsError(char* errorMessage, DWORD errorCode)
{
	char lpBuffer[65536];

	if(FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpBuffer,
		0, NULL 
	)) {
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
		if (!AGDRIVER_IS_WGL(wgl)) {
			continue;
		}
		if (wgl->hwnd == hwnd) {
			win = AGDRIVER_MW(wgl)->win;
			if (WIDGET(win)->drv == NULL) {	/* Being detached */
				return (NULL);
			}
			return (win);
		}
	}
	return (NULL);
}

static void
WGL_Init(void *obj)
{
	AG_DriverWGL *wgl = obj;
	
#ifdef _DEBUG
	agVerbose = 1;
	AG_Verbose("WGL_Init\n");
#endif
	
	if(!inited) {
		InitKeymaps();

		inited = 1;
	}

	wgl->clipRects = NULL;
	wgl->nClipRects = 0;
	wgl->textureGC = NULL;
	wgl->nTextureGC = 0;
	wgl->cursorToSet = NULL;

	memset(wgl->clipStates, 0, sizeof(wgl->clipStates));
}

static int
WGL_Open(void *obj, const char *spec)
{
	AG_DriverWGL *wgl = obj;
	
	// Register Mouse and keyboard
	AGDRIVER(wgl)->mouse = AG_MouseNew(wgl, "Windows mouse");
	AGDRIVER(wgl)->kbd = AG_KeyboardNew(wgl, "Windows keyboard");

	nDrivers++;
	
	return 0;
}

static void
WGL_Close(void *obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
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
	
#ifdef _DEBUG
	AG_Verbose("WGL_Destroy\n");
#endif
	
	Free(wgl->clipRects);
	Free(wgl->textureGC);
}

static int
WGL_OpenWindow(AG_Window *win, AG_Rect r, int depthReq, Uint mwFlags)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;

	// WGL-specific variables
	GLuint     pixelFormat;	
	WNDCLASSEX wndClass;
	DWORD      wndStyle   = WS_OVERLAPPEDWINDOW;
	DWORD      wndStyleEx = 0;

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		16,                      // Color depth
		0, 0, 0, 0, 0, 0, 0, 0,  // Ignore Color bits
		0, 0, 0, 0, 0,           // No Accumulation buffer
		0,                       // No Z-Buffer
		0, 0,                    // No Stencil + AUX-Buffer
		0, 0, 0, 0, 0            // All other attributes are not used
	};

	int  left, top;
	RECT wndRect       = {r.x, r.y, r.x + r.w, r.y + r.h};
	char wndClassName[64]; 

#ifdef _DEBUG
	AG_Verbose("WGL_OpenWindow\n");
#endif

	// Generate window class atom name
	sprintf(wndClassName, "agar-wgl-windowclass-%d", wndClassCount);
	wndClassCount++;

	// Register Window Class	
	memset(&wndClass, 0, sizeof(WNDCLASSEX));
	wndClass.cbSize        = sizeof(WNDCLASSEX);
	wndClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc   = WndProc; // We handle the messages on our own!
	wndClass.hInstance     = GetModuleHandle(NULL);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hCursor       = NULL;
	wndClass.lpszClassName = wndClassName;
	if(!RegisterClassEx(&wndClass)) {
		_SetWindowsError("Cannot register WGL window class", GetLastError());
		return -1;
	}
	
#ifdef _DEBUG
	AG_Verbose("WGL_OpenWindow: Registered window class\n");
#endif

	// Adjust window with account for window borders
	AdjustWindowRectEx (&wndRect, wndStyle, 0, wndStyleEx);

	// Calc window position
	left = wndRect.left;
	top  = wndRect.top;
	if(mwFlags & AG_DRIVER_MW_ANYPOS) {
		left = CW_USEDEFAULT;
		top  = CW_USEDEFAULT;
	}

	// Create OpenGL Window
	wgl->hwnd = CreateWindowEx(
		wndStyleEx,
		wndClassName,
		"AGAR Window",
		wndStyle,
		CW_USEDEFAULT, //wndRect.left,
		CW_USEDEFAULT, //wndRect.top,
		wndRect.right  - wndRect.left,
		wndRect.bottom - wndRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	if(!wgl->hwnd) {
		_SetWindowsError("Cannot create window", GetLastError());
		return -1;
	}
	
#ifdef _DEBUG
	AG_Verbose("WGL_OpenWindow: Created window\n");
#endif

	// Initialize device & rendering contxt
	if(!(wgl->hdc = GetDC(wgl->hwnd))) {
		DestroyWindow(wgl->hwnd);
		_SetWindowsError("GetDC failed", GetLastError());
		return -1;
	}
	if(!(pixelFormat = ChoosePixelFormat(wgl->hdc, &pixelFormatDescriptor))) {
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		_SetWindowsError("ChoosePixelFormat failed", GetLastError());
		return -1;
	}
	if(!(SetPixelFormat(wgl->hdc, pixelFormat, &pixelFormatDescriptor))) {
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		_SetWindowsError("SetPixelFormat failed", GetLastError());
		return -1;
	}
	if(!(wgl->hglrc = wglCreateContext(wgl->hdc))) {
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		_SetWindowsError("wglCreateContext failed", GetLastError());
		return -1;
	}
	if(!(wglMakeCurrent(wgl->hdc, wgl->hglrc))) {
		wglDeleteContext(wgl->hglrc);
		ReleaseDC(wgl->hwnd, wgl->hdc);
		DestroyWindow(wgl->hwnd);
		_SetWindowsError("wglMakeCurrent failed", GetLastError());
		return -1;
	}
	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
#ifdef _DEBUG
	AG_Verbose("WGL_OpenWindow: Initialized GL\n");
#endif

	// Show the window
	ShowWindow(wgl->hwnd, SW_SHOW);
	SetForegroundWindow(wgl->hwnd);
	SetFocus(wgl->hwnd);
	
	AGDRIVER_MW(wgl)->flags |= AG_DRIVER_MW_OPEN;

	// Set the pixel format
	drv->videoFmt = AG_PixelFormatRGB(16, 0x000000ff, 0x0000ff00, 0x00ff0000);
	if (drv->videoFmt == NULL)
		goto fail;

	// Initialize the clipping rectangle stack
	if (InitClipRects(wgl, r.w, r.h) == -1)
		goto fail;
	
	/* Create the built-in cursors. */
	if (InitDefaultCursor(wgl) == -1 || AG_InitStockCursors(drv) == -1)
		goto fail;
	

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

#ifdef _DEBUG
	AG_Verbose("WGL_CloseWindow\n");
#endif

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

	return 0;
}

LRESULT CALLBACK
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AG_DriverWGL      *wgl;
	AG_Driver         *drv;
	AG_Window         *win;
	int               x, y;
	AG_KeyboardAction ka;

	switch(uMsg) {
		case WM_MOUSEMOVE:
			if((win = LookupWindowByID(hWnd))) {
				drv = WIDGET(win)->drv;
				wgl = (AG_DriverWGL *)drv;
				x   = AGDRIVER_BOUNDED_WIDTH(win,  LOWORD(lParam));
				y   = AGDRIVER_BOUNDED_HEIGHT(win, HIWORD(lParam));
				AG_MouseMotionUpdate(drv->mouse, x, y);
				AG_ProcessMouseMotion(
					win, x, y,
					drv->mouse->xRel,
					drv->mouse->yRel,
					drv->mouse->btnState
				);

				if (wgl->cursorToSet != NULL) {
					ChangeCursor(wgl);
				}

				AG_Verbose("WGL_ProcessEvents: MouseMove (%d, %d)\n", x, y);
			} else {
				Verbose("WGL_ProcessEvents: WM_MOUSEMOVE on unknown window!\n");
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if ((win = LookupWindowByID(hWnd))) {
				int button = 
					(wParam & MK_LBUTTON) ? AG_MOUSE_LMASK :
					(wParam & MK_MBUTTON) ? AG_MOUSE_MMASK :
					(wParam & MK_RBUTTON) ? AG_MOUSE_RMASK : 0;

				drv = WIDGET(win)->drv;
				x   = AGDRIVER_BOUNDED_WIDTH (win, LOWORD(lParam));
				y   = AGDRIVER_BOUNDED_HEIGHT(win, HIWORD(lParam));
				AG_MouseButtonUpdate(drv->mouse,
				    AG_BUTTON_PRESSED,
				    button);
				AG_ProcessMouseButtonDown(win, x, y,
				   button);
				AG_Verbose("WGL_ProcessEvents: MouseButtonDown (%d, %d, %d)\n", x, y, button);
			} else {
				Verbose("WGL: WM_L|M|RBUTTONDOWN on unknown window\n");
			}
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			if ((win = LookupWindowByID(hWnd))) {
				int button = 
					(uMsg == WM_LBUTTONUP) ? AG_MOUSE_LMASK :
					(uMsg == WM_MBUTTONUP) ? AG_MOUSE_MMASK :
					(uMsg == WM_RBUTTONUP) ? AG_MOUSE_RMASK : 0;

				drv = WIDGET(win)->drv;
				x   = AGDRIVER_BOUNDED_WIDTH (win, LOWORD(lParam));
				y   = AGDRIVER_BOUNDED_HEIGHT(win, HIWORD(lParam));
				AG_MouseButtonUpdate(drv->mouse,
				    AG_BUTTON_RELEASED,
				    button);
				AG_ProcessMouseButtonUp(win, x, y,
				   button);
				AG_Verbose("WGL_ProcessEvents: MouseButtonUp (%d, %d, %d)\n", x, y, button);
			} else {
				AG_Verbose("WGL_ProcessEvents: WM_L|M|RBUTTONUP on unknown window\n");
			}
			break;
		case WM_KEYUP:
		case WM_KEYDOWN:			
			if ((win = LookupWindowByID(hWnd))) {	
				HKL            kLayout;
				unsigned char  kState[256];
				unsigned short kResult = 0;
				int            ret;
				
				ka  = (uMsg == WM_KEYDOWN) ? AG_KEY_PRESSED : AG_KEY_RELEASED;
				drv = WIDGET(win)->drv;

				kLayout = GetKeyboardLayout(0);
				GetKeyboardState(kState);
				ret = ToAsciiEx(wParam, HIWORD(lParam) & 0xFF, kState, &kResult, 0, kLayout);
				if(ret == 1) {
					// @todo Do we have to take notice of ucs here?
					AG_KeyboardUpdate(drv->kbd, ka, kResult, kResult);
					AG_ProcessKey(drv->kbd, win, ka, kResult, kResult);
				} else {
					// Entered char is a special keycode, search it in map
					AG_KeySym sym = agKeymapMisc[wParam&0xff];
					AG_KeyboardUpdate(drv->kbd, ka, sym, sym);
					AG_ProcessKey(drv->kbd, win, ka, sym, sym);					
				}
			}
			break;
		case WM_SETFOCUS:
			if ((win = LookupWindowByID(hWnd))) {
				AG_Verbose("WGL_ProcessEvents: %08X has focus now!\n", hWnd);
				agWindowFocused = win;
				AG_PostEvent(NULL, win, "window-gainfocus", NULL);
			}
			break;
		case WM_KILLFOCUS:
			if ((win = LookupWindowByID(hWnd)) && agWindowFocused == win) {
				AG_Verbose("WGL_ProcessEvents: %08X lost focus!\n", hWnd);
				AG_PostEvent(NULL, win, "window-lostfocus", NULL);
				agWindowFocused = NULL;
			}
			break;
		case WM_SIZE:
			if (win = LookupWindowByID(hWnd)) {
				drv = WIDGET(win)->drv;

				if(drv->flags & AG_DRIVER_MW_OPEN) {
					AG_SizeAlloc a;

					a.x = 0;
					a.y = 0;
					a.w = LOWORD(lParam);
					a.h = HIWORD(lParam);
					
					AG_Verbose("WGL_ProcessEvents: Set window size to %d,%d,%d,%d\n", a.x, a.y, a.w, a.h);

					WGL_PostResizeCallback(win, &a);
				}
			}
			break;
	}
	
	// We just forward all window messages, which haven't been handled
	// by ProcessEvents, to the system-internal window procedure
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static int
WGL_ProcessEvents(void *drvCaller)
{
	MSG msg;
	int nProcessed = 0;
	
	AG_LockVFS(&agDrivers);

	// @todo : window handle NULL must be replaced by a real one?
	if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if(LookupWindowByID(msg.hwnd)) {
			// Dispatch message
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			nProcessed++;	
		}
	}

	AG_UnlockVFS(&agDrivers);
	
	return nProcessed;
}


static void
WGL_GenericEventLoop(void *obj)
{
	AG_Driver *drv;
	AG_Window *win;
	Uint32 t1, t2;

#ifdef _DEBUG
	AG_Verbose("WGL_GenericEventLoop\n");
#endif

	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();
		if (t2 - t1 >= rNom) {
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (!AGDRIVER_IS_WGL(drv)) {
					continue;
				}
				win = AGDRIVER_MW(drv)->win;
				if (win->visible) {
					AG_BeginRendering(drv);
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
					AG_EndRendering(drv);
				}
			}
			t1 = AG_GetTicks();
			rCur = rNom - (t1-t2);
			if (rCur < 1) { rCur = 1; }
		} else if (GetQueueStatus(QS_ALLEVENTS) != 0) {
			if (WGL_ProcessEvents(NULL) == -1)
				return;
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

	// Remove textures queued for deletion.
	for (i = 0; i < wgl->nTextureGC; i++) {
		glDeleteTextures(1, (GLuint *)&wgl->textureGC[i]);
	}
	wgl->nTextureGC = 0;
}

static void
WGL_DeleteTexture(void *drv, Uint texture)
{
	AG_DriverWGL *wgl = drv;

	wgl->textureGC = Realloc(wgl->textureGC, (wgl->nTextureGC+1)*sizeof(Uint));
	wgl->textureGC[wgl->nTextureGC++] = texture;
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
	if ((wgl->clipRects = AG_TryMalloc(sizeof(AG_ClipRect))) == NULL) {
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
		return -1;
	}
	*rv = AGDRIVER_MW(wgl)->win;
	return 0;
}

static int
WGL_SetInputFocus(AG_Window *win)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetFocus(wgl->hwnd);
	return 0;
}

static int
WGL_MoveWindow(AG_Window *win, int x, int y)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowPos(wgl->hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);	
	return 0;
}

static int
WGL_ResizeWindow(AG_Window *win, Uint w, Uint h)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowPos(wgl->hwnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE);	
	return 0;
}

static int
WGL_MoveResizeWindow(AG_Window *win, AG_SizeAlloc *a)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowPos(wgl->hwnd, NULL, a->x, a->y, a->w, a->h, SWP_NOZORDER);	
	return 0;
}

static int
WGL_SetBorderWidth(AG_Window *win, Uint width)
{
	// @todo There is no border width in win32!
	return 0;
}

static int
WGL_SetWindowCaption(AG_Window *win, const char *s)
{
	AG_DriverWGL *wgl = (AG_DriverWGL *)WIDGET(win)->drv;
	SetWindowText(wgl->hwnd, s);
	return 0;
}

static void
WGL_SetTransientFor(AG_Window *win, AG_Window *winParent)
{
	// @todo Therse is no need for transient windows in win32?
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
	
	if ((cg = AG_TryMalloc(sizeof(struct ag_cursor_wgl))) == NULL)
		return (-1);
	if ((drv->cursors = AG_TryMalloc(sizeof(AG_Cursor))) == NULL) {
		free(cg);
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

static void
ChangeCursor(AG_DriverWGL *wgl)
{
	AG_Cursor *ac = wgl->cursorToSet;
	struct ag_cursor_wgl *cg = ac->p;

	if (wgl->cursorToSet != AGDRIVER(wgl)->activeCursor) {
		if (wgl->cursorToSet == &AGDRIVER(wgl)->cursors[0]) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		} else {
			SetCursor(cg->cursor);
		}
		AGDRIVER(wgl)->activeCursor = ac;
		cg->visible = 1;
	}
	wgl->cursorToSet = NULL;
}

static int
WGL_CreateCursor(void *obj, AG_Cursor *ac)
{
	AG_CursorWGL *cg;
	int          size, i;
	BYTE         *xorMask, *andMask;

	AG_Verbose("WGL_CreateCursor");

	// Initialize cursor struct
	if((cg = AG_TryMalloc(sizeof(AG_CursorWGL))) == NULL) {
		return -1;
	}
	cg->black = RGB(0, 0, 0);
	cg->white = RGB(0xFF, 0xFF, 0xFF);
	
	// Calc size for cursor data
	size = (ac->w / 8) * ac->h;

	// Allocate memory for xorMask (which represents the cursor data)
	if ((xorMask = AG_TryMalloc(size)) == NULL) {
		Free(cg);
		return -1;
	}

	// Allocate memory for andMask (which represents the transparence)
	if ((andMask = AG_TryMalloc(size)) == NULL) {
		Free(xorMask);
		Free(cg);
		return -1;
	}

	// Copy cursor data into buffers for use with CreateCursor
	for (i = 0; i < size; i++) {
		andMask[i] = ~ac->mask[i];
		xorMask[i] = ~ac->data[i] ^ ~ac->mask[i];
	}

	// Create cursor
	if(cg->cursor = CreateCursor(GetModuleHandle(NULL), ac->xHot, ac->yHot, ac->w, ac->h, andMask, xorMask)) {
		cg->visible = 0;
		ac->p = cg;

		AG_Verbose("WGL_CreateCursor: Created.");

		return 0;
	}
	
	_SetWindowsError("CreateCursor failed!", GetLastError());
	
	return -1;
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
WGL_PushCursor(void *obj, AG_Cursor *ac)
{
	AG_DriverWGL *wgl = obj;

	wgl->cursorToSet = ac;

	return (0);
}

static void
WGL_PopCursor(void *obj)
{
	AG_DriverWGL *wgl = obj;
	
	wgl->cursorToSet = &AGDRIVER(wgl)->cursors[0];
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
WGL_PostResizeCallback(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver    *drv = WIDGET(win)->drv;
	AG_DriverWGL *wgl = (AG_DriverWGL *)drv;
	AG_ClipRect *cr;

	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	// Update clipping rectangle 0
	cr = &wgl->clipRects[0];
	cr->r.w = WIDTH(win);
	cr->r.h = HEIGHT(win);
	cr->eqns[2][3] = (double)WIDTH(win);
	cr->eqns[3][3] = (double)HEIGHT(win);
	
	// Update WGL context.
	wglMakeCurrent(wgl->hdc, wgl->hglrc);

	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
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
		WGL_ProcessEvents,
		WGL_GenericEventLoop,
		NULL,			/* endEventProcessing */
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
		WGL_PushCursor,
		WGL_PopCursor,
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
		AG_GL_DrawFrame,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph
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
	NULL, // PreResizeCallback,
	WGL_PostResizeCallback,
	NULL,				/* captureWindow */
	WGL_SetBorderWidth,
	WGL_SetWindowCaption,
	WGL_SetTransientFor
};


#endif /* HAVE_WGL */
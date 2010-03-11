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
 * Driver for OpenGL graphics via X Windows System. This is a multiple display
 * driver; one context is created for each Agar window.
 */

#include <config/have_glx.h>
#ifdef HAVE_GLX

#include <core/core.h>
#include <core/config.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include "gui.h"
#include "window.h"
#include "perfmon.h"
#include "gui_math.h"
#include "text.h"
#include "cursors.h"

#include "drv_gl_common.h"

#define DEBUG_XSYNC

static int      nDrivers = 0;		/* Drivers open */
static Display *agDisplay = NULL;	/* X display (shared) */
static int      agScreen = 0;		/* Default screen (shared) */
static Uint     rNom = 16;		/* Nominal refresh rate (ms) */
static int      rCur = 0;		/* Effective refresh rate (ms) */
static AG_Mutex agDisplayLock;		/* Lock on agDisplay */

static char           xkbBuf[64];	/* For Unicode key translation */
static XComposeStatus xkbCompStatus;	/* For Unicode key translation */
static Atom           wmDeleteWindow;	/* WM_DELETE_WINDOW atom */

/* Driver instance data */
typedef struct ag_driver_glx {
	struct ag_driver_mw _inherit;
	Window           w;		/* X window */
	GLXContext       glxCtx;	/* GLX context */
	int              clipStates[4];	/* Clipping GL state */
	AG_ClipRect     *clipRects;	/* Clipping rectangles */
	Uint            nClipRects;
	Uint            *textureGC;	/* Textures queued for deletion */
	Uint            nTextureGC;
	AG_GL_BlendState bs[1];		/* Saved blending states */
	AG_Mutex         lock;		/* Protect Xlib calls */
} AG_DriverGLX;

static int modMasksInited = 0;		/* For modifier key translation */
struct {
	Uint Lmeta, Rmeta;
	Uint Lalt, Ralt;
	Uint num, mode;
} modMasks;
struct ag_key_mapping {			/* Keymap translation table entry */
	int kcode;			/* Scancode */
	int kclass;			/* X keysym class (e.g., 0xff) */
	AG_KeySym key;			/* Corresponding Agar keysym */
};
#include "drv_glx_keymaps.h"

typedef struct ag_cursor_glx {
	XColor black;
	XColor white;
	Cursor xc;
	int visible;
} AG_CursorGLX;

AG_DriverMwClass agDriverGLX;
#if 0
#define AGDRIVER_IS_GLX(drv) \
	AG_OfClass((drv),"AG_Driver:AG_DriverMw:AG_DriverGLX")
#endif
#define AGDRIVER_IS_GLX(drv) \
	(AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverGLX)

static void PostResizeCallback(AG_Window *, AG_SizeAlloc *);
static void PostMoveCallback(AG_Window *, AG_SizeAlloc *);
static int  RaiseWindow(AG_Window *);
static int  SetInputFocus(AG_Window *);
static void SetTransientFor(AG_Window *, AG_Window *);

static void
Init(void *obj)
{
	AG_DriverGLX *glx = obj;

	glx->clipRects = NULL;
	glx->nClipRects = 0;
	memset(glx->clipStates, 0, sizeof(glx->clipStates));
	glx->textureGC = NULL;
	glx->nTextureGC = 0;
	AG_MutexInitRecursive(&glx->lock);
}

static void
Destroy(void *obj)
{
	AG_DriverGLX *glx = obj;

	AG_MutexDestroy(&glx->lock);
	Free(glx->clipRects);
	Free(glx->textureGC);
}

/*
 * Driver initialization
 */

static void
InitGlobals(void)
{
	AG_MutexInitRecursive(&agDisplayLock);
	agScreen = DefaultScreen(agDisplay);
	InitKeymaps();
	memset(xkbBuf, '\0', sizeof(xkbBuf));
	memset(&xkbCompStatus, 0, sizeof(xkbCompStatus));
	wmDeleteWindow = XInternAtom(agDisplay, "WM_DELETE_WINDOW", False);
}

#ifdef DEBUG_XSYNC
static int
HandleErrorX11(Display *disp, XErrorEvent *xev)
{
	char buf[128];

	XGetErrorText(disp, xev->error_code, buf, sizeof(buf));
	fprintf(stderr, "Caught X error: %s\n", buf);
	abort();
}
#endif /* DEBUG_XSYNC */

static int
Open(void *obj, const char *spec)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	int err, ev;

	if (agDisplay == NULL) {
		/* Initialize globals */
		if ((agDisplay = XOpenDisplay(NULL)) == NULL) {
			AG_SetError("Cannot open X display");
			goto fail;
		}
		if (!glXQueryExtension(agDisplay, &err, &ev)) {
			AG_SetError("GLX extension is not available");
			XCloseDisplay(agDisplay);
			goto fail;
		}
		InitGlobals();
	}
	
	AG_MutexLock(&agDisplayLock);

	/* Register the core X mouse and keyboard */
	if ((drv->mouse = AG_MouseNew(glx, "X mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(glx, "X keyboard")) == NULL)
		goto fail;

#ifdef DEBUG_XSYNC
	XSynchronize(agDisplay, True);
	XSetErrorHandler(HandleErrorX11);
#endif /* DEBUG_XSYNC */

	nDrivers++;
	AG_MutexUnlock(&agDisplayLock);
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
	agDisplay = NULL;
	agScreen = 0;
	AG_MutexUnlock(&agDisplayLock);
	return (-1);
}

static void
Close(void *obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	AG_MutexLock(&agDisplayLock);
	if (--nDrivers == 0) {
		XCloseDisplay(agDisplay);
		agDisplay = NULL;
		agScreen = 0;
	}
	AG_MutexUnlock(&agDisplayLock);

	AG_ObjectDetach(drv->mouse);
	AG_ObjectDestroy(drv->mouse);
	AG_ObjectDetach(drv->kbd);
	AG_ObjectDestroy(drv->kbd);
	
	drv->mouse = NULL;
	drv->kbd = NULL;
}

static int
GetDisplaySize(Uint *w, Uint *h)
{
	AG_MutexLock(&agDisplayLock);
	*w = (Uint)DisplayWidth(agDisplay, agScreen);
	*h = (Uint)DisplayHeight(agDisplay, agScreen);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

/* Get Agar keycode corresponding to an X KeyCode */
static int
LookupKeyCode(KeyCode keycode, AG_KeySym *rv)
{
	KeySym ks;
	int ret = 0;
	
	AG_MutexLock(&agDisplayLock);

	if ((ks = XKeycodeToKeysym(agDisplay, keycode, 0)) == 0) {
		*rv = AG_KEY_NONE;
		goto out;
	}
	switch (ks>>8) {
#ifdef XK_MISCELLANY
	case 0xff:
		*rv = agKeymapMisc[ks&0xff];
		ret = 1;
		goto out;
#endif
#ifdef XK_XKB_KEYS
	case 0xfe:
		*rv = agKeymapXKB[ks&0xff];
		ret = 1;
		goto out;
#endif
	default:
		*rv = (AG_KeySym)(ks&0xff);
		ret = 1;
		goto out;
	}
out:
	AG_MutexUnlock(&agDisplayLock);
	return (ret);
}

/* Return the number of pending events in the event queue. */
static int
PendingEvents(void)
{
	struct timeval tv;
	fd_set fdset;
	int fd, ret = 0;

	AG_MutexLock(&agDisplayLock);

	if (agDisplay == NULL)
		goto out;

	XFlush(agDisplay);

	if (XEventsQueued(agDisplay, QueuedAlready)) {
		ret = 1;
		goto out;
	}

	/* Block on the X connection fd */
	fd = ConnectionNumber(agDisplay);
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (select(fd+1, &fdset, NULL, NULL, &tv) == 1) {
		ret = XPending(agDisplay);
	}
out:
	AG_MutexUnlock(&agDisplayLock);
	return (ret);
}

/* Return the Agar window corresponding to a X Window ID */
static __inline__ AG_Window *
LookupWindowByID(Window xw)
{
	AG_Window *win;
	AG_DriverGLX *glx;

	/* XXX TODO portable to optimize based on numerical XIDs? */
	AGOBJECT_FOREACH_CHILD(glx, &agDrivers, ag_driver_glx) {
		if (!AGDRIVER_IS_GLX(glx)) {
			continue;
		}
		if (glx->w == xw) {
			win = AGDRIVER_MW(glx)->win;
			if (WIDGET(win)->drv == NULL) {	/* Being detached */
				return (NULL);
			}
			return (win);
		}
	}
	return (NULL);
}

static void
InitModifierMasks(void)
{
	XModifierKeymap *xmk;
	int i, j;
	Uint n;

	if (modMasksInited) {
		return;
	}
	modMasksInited = 1;

	xmk = XGetModifierMapping(agDisplay);
	n = xmk->max_keypermod;
	for (i = 3; i < 8; i++) {
		for (j = 0; j < n; j++) {
			KeyCode kc = xmk->modifiermap[i*n + j];
			KeySym ks = XKeycodeToKeysym(agDisplay, kc, 0);
			Uint mask = 1<<i;

			switch (ks) {
			case XK_Num_Lock:	modMasks.num = mask;	break;
			case XK_Alt_L:		modMasks.Lalt = mask;	break;
			case XK_Alt_R:		modMasks.Ralt = mask;	break;
			case XK_Meta_L:		modMasks.Lmeta = mask;	break;
			case XK_Meta_R:		modMasks.Rmeta = mask;	break;
			case XK_Mode_switch:	modMasks.mode = mask;	break;
			}
		}
	}
	XFreeModifiermap(xmk);
}

/* Refresh the internal keyboard state. */
static void
UpdateKeyboard(AG_Keyboard *kbd, char *kv)
{
	int i, j;
	Uint ms = 0;
	AG_KeySym key;
	Window dummy;
	int x, y;
	Uint mask;
	
	AG_MutexLock(&agDisplayLock);

	/* Get the keyboard modifier state. XXX */
	InitModifierMasks();
	if (XQueryPointer(agDisplay, DefaultRootWindow(agDisplay),
	    &dummy, &dummy, &x, &y, &x, &y, &mask)) {
		if (mask & LockMask) { ms |= AG_KEYMOD_CAPSLOCK; }
		if (mask & modMasks.mode) { ms |= AG_KEYMOD_MODE; }
		if (mask & modMasks.num) { ms |= AG_KEYMOD_NUMLOCK; }
	}
	memset(kbd->keyState, 0, kbd->keyCount);
	for (i = 0; i < 32; i++) {
		if (kv[i] == 0) {
			continue;
		}
		for (j = 0; j < 8; j++) {
			if ((kv[i] & (1<<j)) == 0) {
				continue;
			}
			if (!LookupKeyCode(i<<3|j, &key)) {
				continue;
			}
			kbd->keyState[key] = AG_KEY_PRESSED;

			switch (key) {
			case AG_KEY_LSHIFT: ms |= AG_KEYMOD_LSHIFT; break;
			case AG_KEY_RSHIFT: ms |= AG_KEYMOD_RSHIFT; break;
			case AG_KEY_LCTRL:  ms |= AG_KEYMOD_LCTRL;  break;
			case AG_KEY_RCTRL:  ms |= AG_KEYMOD_RCTRL;  break;
			case AG_KEY_LALT:   ms |= AG_KEYMOD_LALT;   break;
			case AG_KEY_RALT:   ms |= AG_KEYMOD_RALT;   break;
			case AG_KEY_LMETA:  ms |= AG_KEYMOD_LMETA;  break;
			case AG_KEY_RMETA:  ms |= AG_KEYMOD_RMETA;  break;
			default:
				break;
			}
		}
	}

	kbd->keyState[AG_KEY_CAPSLOCK] = (ms & AG_KEYMOD_CAPSLOCK) ?
	    AG_KEY_PRESSED : AG_KEY_RELEASED;
	kbd->keyState[AG_KEY_NUMLOCK] = (ms & AG_KEYMOD_NUMLOCK) ?
	    AG_KEY_PRESSED : AG_KEY_RELEASED;

	/* Set the final modifier state */
	AG_SetModState(kbd, ms);
	
	AG_MutexUnlock(&agDisplayLock);
}

static int
ProcessEvents(void *drvCaller)
{
	AG_DriverGLX *glx;
	AG_Driver *drv;
	AG_Window *win;
	XEvent xev;
	AG_KeySym ks;
	AG_KeyboardAction ka;
	Uint32 ucs;
	int x, y;
	int nProcessed = 0;
	AG_CursorArea *ca;

	while (PendingEvents()) {
		AG_LockVFS(&agDrivers);
		AG_MutexLock(&agDisplayLock);
		XNextEvent(agDisplay, &xev);
		switch (xev.type) {
		case MotionNotify:
			if ((win = LookupWindowByID(xev.xmotion.window))) {
				drv = WIDGET(win)->drv;
				glx = (AG_DriverGLX *)drv;
				AG_MutexLock(&glx->lock);

				x = AGDRIVER_BOUNDED_WIDTH(win, xev.xmotion.x);
				y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xmotion.y);
				AG_MouseMotionUpdate(drv->mouse, x,y);
				AG_ProcessMouseMotion(win, x, y,
				    drv->mouse->xRel,
				    drv->mouse->yRel,
				    drv->mouse->btnState);

				/* Change the cursor if necessary. */
				TAILQ_FOREACH(ca, &win->cursorAreas,
				    cursorAreas) {
					if (AG_RectInside(&ca->r, x,y))
						break;
				}
				if (ca == NULL) {
					if (drv->activeCursor != &drv->cursors[0])
						AGDRIVER_CLASS(drv)->unsetCursor(drv);
				} else if (ca->c != drv->activeCursor) {
					AGDRIVER_CLASS(drv)->setCursor(drv, ca->c);
				}
				AG_MutexUnlock(&glx->lock);
			} else {
				Verbose("MotionNotify on unknown window\n");
			}
			break;
		case ButtonPress:
			if ((win = LookupWindowByID(xev.xbutton.window))) {
				drv = WIDGET(win)->drv;
				glx = (AG_DriverGLX *)drv;

				AG_MutexLock(&glx->lock);
				x = AGDRIVER_BOUNDED_WIDTH(win, xev.xbutton.x);
				y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xbutton.y);
				AG_MouseButtonUpdate(drv->mouse,
				    AG_BUTTON_PRESSED,
				    xev.xbutton.button);
				AG_ProcessMouseButtonDown(win, x, y,
				    xev.xbutton.button);
				AG_MutexUnlock(&glx->lock);
			} else {
				Verbose("ButtonPress on unknown window\n");
			}
			break;
		case ButtonRelease:
			if ((win = LookupWindowByID(xev.xbutton.window))) {
				drv = WIDGET(win)->drv;
				glx = (AG_DriverGLX *)drv;

				AG_MutexLock(&glx->lock);
				x = AGDRIVER_BOUNDED_WIDTH(win, xev.xbutton.x);
				y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xbutton.y);
				AG_MouseButtonUpdate(drv->mouse,
				    AG_BUTTON_RELEASED,
				    xev.xbutton.button);
				AG_ProcessMouseButtonUp(win, x, y,
				    xev.xbutton.button);
				AG_MutexUnlock(&glx->lock);
			} else {
				Verbose("ButtonRelease on unknown window\n");
			}
			break;
		case KeyRelease:
			if (IsKeyRepeat(&xev)) {
				/* We implement key repeat internally */
				goto next_event;
			}
			/* FALLTHROUGH */
		case KeyPress:
			if (XLookupString(&xev.xkey, xkbBuf, sizeof(xkbBuf),
			    NULL, &xkbCompStatus) >= 1) {
				ucs = (Uint8)xkbBuf[0];	/* XXX */
			} else {
				ucs = 0;
			}
			ka = (xev.type == KeyPress) ? AG_KEY_PRESSED :
			                              AG_KEY_RELEASED;
			if (LookupKeyCode(xev.xkey.keycode, &ks)) {
				if ((win = LookupWindowByID(xev.xkey.window))) {
					drv = WIDGET(win)->drv;
					glx = (AG_DriverGLX *)drv;

					AG_MutexLock(&glx->lock);
					AG_KeyboardUpdate(drv->kbd, ka, ks, ucs);
					AG_ProcessKey(drv->kbd, win, ka, ks, ucs);
					AG_MutexUnlock(&glx->lock);
				}
			} else {
				Verbose("Unknown keycode: %d\n",
				    (int)xev.xkey.keycode);
			}
			break;
#if 0
		case EnterNotify:
			if ((xev.xcrossing.mode != NotifyGrab) &&
			    (xev.xcrossing.mode != NotifyUngrab)) {
				/* TODO: AG_AppFocusEvent() */
				AG_MouseMotionUpdate(drv->mouse,
				    xev->xcrossing.x,
				    xev->xcrossing.y);
				InputEvent(&xev);
			}
			break;
		case LeaveNotify:
			if ((xev.xcrossing.mode != NotifyGrab) &&
			    (xev.xcrossing.mode != NotifyUngrab) &&
			    (xev.xcrossing.detail != NotifyInferior)) {
				/* TODO: AG_AppFocusEvent() */
				AG_MouseMotionUpdate(drv->mouse,
				    xev.xcrossing.x,
				    xev.xcrossing.y);
				InputEvent(&xev);
			}
			break;
#endif
		case EnterNotify:
			if ((win = LookupWindowByID(xev.xcrossing.window))) {
				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				AG_MutexLock(&glx->lock);
				AG_PostEvent(NULL, win, "window-enter", NULL);
				AG_MutexUnlock(&glx->lock);
			}
			break;
		case LeaveNotify:
			if ((win = LookupWindowByID(xev.xcrossing.window))) {
				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				AG_MutexLock(&glx->lock);
				AG_PostEvent(NULL, win, "window-leave", NULL);
				AG_MutexUnlock(&glx->lock);
			}
			break;
		case FocusIn:
			if ((win = LookupWindowByID(xev.xfocus.window))) {
				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				AG_MutexLock(&glx->lock);
				agWindowFocused = win;
				AG_PostEvent(NULL, win, "window-gainfocus", NULL);
				AG_MutexUnlock(&glx->lock);
			}
			break;
		case FocusOut:
			if ((win = LookupWindowByID(xev.xfocus.window)) &&
			    agWindowFocused == win) {
				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				AG_MutexLock(&glx->lock);
				AG_PostEvent(NULL, win, "window-lostfocus", NULL);
				agWindowFocused = NULL;
				AG_MutexUnlock(&glx->lock);
			}
			break;
		case MapNotify:
			break;
		case UnmapNotify:
			break;
		case DestroyNotify:
			break;
		case KeymapNotify:
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (!AGDRIVER_IS_GLX(drv)) {
					continue;
				}
				glx = (AG_DriverGLX *)drv;
				AG_MutexLock(&glx->lock);
				UpdateKeyboard(drv->kbd, xev.xkeymap.key_vector);
				AG_MutexUnlock(&glx->lock);
			}
			break;
		case MappingNotify:
			XRefreshKeyboardMapping(&xev.xmapping);
			break;
		case ConfigureNotify:
			if ((win = LookupWindowByID(xev.xconfigure.window))) {
				AG_SizeAlloc a;
				Window ignore;

				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				AG_MutexLock(&glx->lock);

				XTranslateCoordinates(agDisplay,
				    xev.xconfigure.window,
				    DefaultRootWindow(agDisplay),
				    0, 0,
				    &a.x, &a.y,
				    &ignore);
				a.w = xev.xconfigure.width;
				a.h = xev.xconfigure.height;
				if (a.w != WIDTH(win) || a.h != HEIGHT(win)) {
					PostResizeCallback(win, &a);
				} else {
					PostMoveCallback(win, &a);
				}

				AG_MutexUnlock(&glx->lock);
			}
			break;
		case ReparentNotify:
			/* printf("ReparentNotify\n"); */
			break;
		case ClientMessage:
			if ((xev.xclient.format == 32) &&
			    (xev.xclient.data.l[0] == wmDeleteWindow) &&
			    (win = LookupWindowByID(xev.xclient.window))) {
				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				AG_MutexLock(&glx->lock);
				AG_PostEvent(NULL, win, "window-close", NULL);
				AG_MutexUnlock(&glx->lock);
			}
			break;
#if 0
		case Expose:
			if ((win = LookupWindowByID(xev.xexpose.window))) {
				AG_Driver *drv = WIDGET(win)->drv;
				AG_BeginRendering(drv);
				AG_ObjectLock(win);
				AG_WindowDraw(win);
				AG_ObjectUnlock(win);
				AG_EndRendering(drv);
			}
			break;
#endif
		default:
			printf("Unknown X event %d\n", xev.type);
			break;
		}
next_event:
		nProcessed++;
		AG_MutexUnlock(&agDisplayLock);
		AG_UnlockVFS(&agDrivers);

		if (!TAILQ_EMPTY(&agWindowDetachQ)) {
			AG_FreeDetachedWindows();
		}
		/*
		 * Exit when no more windows exist.
		 * XXX TODO make this behavior configurable
		 */
		if (TAILQ_EMPTY(&OBJECT(&agDrivers)->children))
			goto quit;

		AG_MutexLock(&agDisplayLock);
		AG_LockVFS(&agDrivers);
		if (agWindowToFocus != NULL) {
			glx = (AG_DriverGLX *)WIDGET(agWindowToFocus)->drv;
			if (glx != NULL && AGDRIVER_IS_GLX(glx)) {
				AG_MutexLock(&glx->lock);
				RaiseWindow(agWindowToFocus);
				SetInputFocus(agWindowToFocus);
				AG_MutexUnlock(&glx->lock);
			}
			agWindowToFocus = NULL;
		}
		AG_UnlockVFS(&agDrivers);
		AG_MutexUnlock(&agDisplayLock);
	}
	return (nProcessed);
quit:
	agTerminating = 1;
	return (-1);
}

static void
GenericEventLoop(void *obj)
{
	AG_Driver *drv;
	AG_DriverGLX *glx;
	AG_Window *win;
	Uint32 t1, t2;

	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();
		if (t2 - t1 >= rNom) {
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (!AGDRIVER_IS_GLX(drv)) {
					continue;
				}
				glx = (AG_DriverGLX *)drv;
				AG_MutexLock(&glx->lock);
				win = AGDRIVER_MW(drv)->win;
				if (win->visible) {
					AG_BeginRendering(drv);
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
					AG_EndRendering(drv);
				}
				AG_MutexUnlock(&glx->lock);
			}
			t1 = AG_GetTicks();
			rCur = rNom - (t1-t2);
			if (rCur < 1) { rCur = 1; }
		} else if (PendingEvents() != 0) {
			if (ProcessEvents(NULL) == -1)
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
Terminate(void)
{
	AG_DriverGLX *glx;
	XClientMessageEvent xe;
	
	AG_MutexLock(&agDisplayLock);
	AGOBJECT_FOREACH_CHILD(glx, &agDrivers, ag_driver_glx) {
		if (!AGDRIVER_IS_GLX(glx)) {
			continue;
		}
		AG_MutexLock(&glx->lock);
		xe.format = 32;
		xe.data.l[0] = glx->w;
		XSendEvent(agDisplay, glx->w, False, NoEventMask, (XEvent *)&xe);
		AG_MutexUnlock(&glx->lock);
	}
	XSync(agDisplay, False);
	AG_MutexUnlock(&agDisplayLock);
	exit(0);
}

static void
BeginRendering(void *obj)
{
	AG_DriverGLX *glx = obj;

	AG_MutexLock(&agDisplayLock);
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	AG_MutexUnlock(&agDisplayLock);
}

static void
RenderWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glx->clipStates[0] = glIsEnabled(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE0);
	glx->clipStates[1] = glIsEnabled(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE1);
	glx->clipStates[2] = glIsEnabled(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE2);
	glx->clipStates[3] = glIsEnabled(GL_CLIP_PLANE3);
	glEnable(GL_CLIP_PLANE3);

	AG_WidgetDraw(win);
}

static void
EndRendering(void *obj)
{
	AG_DriverGLX *glx = obj;
	Uint i;
	
	AG_MutexLock(&agDisplayLock);
	
/*	glFlush(); */
	glXSwapBuffers(agDisplay, glx->w);

	/* Remove textures queued for deletion. */
	for (i = 0; i < glx->nTextureGC; i++) {
		glDeleteTextures(1, (GLuint *)&glx->textureGC[i]);
	}
	glx->nTextureGC = 0;
/*	glXMakeCurrent(agDisplay, None, NULL); */
	
	AG_MutexUnlock(&agDisplayLock);
}

static void
DeleteTexture(void *drv, Uint texture)
{
	AG_DriverGLX *glx = drv;

	AG_MutexLock(&agDisplayLock);
	glx->textureGC = Realloc(glx->textureGC,
	    (glx->nTextureGC+1)*sizeof(Uint));
	glx->textureGC[glx->nTextureGC++] = texture;
	AG_MutexUnlock(&agDisplayLock);
}

static int
SetRefreshRate(void *obj, int fps)
{
	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	AG_MutexLock(&agDisplayLock);
	rNom = 1000/fps;
	rCur = 0;
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

/*
 * Clipping and blending control (rendering context)
 */

static void
PushClipRect(void *obj, AG_Rect r)
{
	AG_DriverGLX *glx = obj;
	AG_ClipRect *cr, *crPrev;

	AG_MutexLock(&glx->lock);

	glx->clipRects = Realloc(glx->clipRects, (glx->nClipRects+1)*
	                                         sizeof(AG_ClipRect));
	crPrev = &glx->clipRects[glx->nClipRects-1];
	cr = &glx->clipRects[glx->nClipRects++];

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
	
	AG_MutexUnlock(&glx->lock);
}

static void
PopClipRect(void *obj)
{
	AG_DriverGLX *glx = obj;
	AG_ClipRect *cr;
	
	AG_MutexLock(&glx->lock);
#ifdef AG_DEBUG
	if (glx->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &glx->clipRects[glx->nClipRects-2];
	glx->nClipRects--;

	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
	
	AG_MutexUnlock(&glx->lock);
}

static void
PushBlendingMode(void *obj, AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	AG_DriverGLX *glx = obj;
	
	AG_MutexLock(&glx->lock);

	/* XXX TODO: stack */
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
	    &glx->bs[0].texEnvMode);
	glGetBooleanv(GL_BLEND, &glx->bs[0].enabled);
	glGetIntegerv(GL_BLEND_SRC, &glx->bs[0].srcFactor);
	glGetIntegerv(GL_BLEND_DST, &glx->bs[0].dstFactor);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(AG_GL_GetBlendingFunc(fnSrc), AG_GL_GetBlendingFunc(fnDst));
	
	AG_MutexUnlock(&glx->lock);
}

static void
PopBlendingMode(void *obj)
{
	AG_DriverGLX *glx = obj;
	
	AG_MutexLock(&glx->lock);

	/* XXX TODO: stack */
	if (glx->bs[0].enabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
	glBlendFunc(glx->bs[0].srcFactor, glx->bs[0].dstFactor);
	
	AG_MutexUnlock(&glx->lock);
}

/*
 * Cursor operations
 */

static int
CreateCursor(void *obj, AG_Cursor *ac)
{
	AG_DriverGLX *glx = obj;
	AG_CursorGLX *cg;
	int i, size;
	char *xData, *xMask;
	XGCValues gcVals;
	GC gc;
	XImage *dataImg, *maskImg;
	Pixmap dataPixmap, maskPixmap;

	if ((cg = TryMalloc(sizeof(AG_CursorGLX))) == NULL) {
		return (-1);
	}
	memset(&cg->black, 0, sizeof(cg->black));
	cg->white.pixel = 0xffff;
	cg->white.red = 0xffff;
	cg->white.green = 0xffff;
	cg->white.blue = 0xffff;

	size = (ac->w/8)*ac->h;
	if ((xData = TryMalloc(size)) == NULL) {
		Free(cg);
		return (-1);
	}
	if ((xMask = TryMalloc(size)) == NULL) {
		Free(xData);
		Free(cg);
		return (-1);
	}
	for (i = 0; i < size; i++) {
		xMask[i] = ac->data[i] | ac->mask[i];
		xData[i] = ac->data[i];
	}
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	/* Create the data image */
	dataImg = XCreateImage(agDisplay,
	    DefaultVisual(agDisplay,agScreen),
	    1, XYBitmap, 0, xData, ac->w,ac->h, 8, ac->w/8);
	dataImg->byte_order = MSBFirst;
	dataImg->bitmap_bit_order = MSBFirst;
	dataPixmap = XCreatePixmap(agDisplay,
	    RootWindow(agDisplay,agScreen),
	    ac->w,ac->h, 1);

	/* Create the mask image */
	maskImg = XCreateImage(agDisplay,
	    DefaultVisual(agDisplay,agScreen),
	    1, XYBitmap, 0, xMask, ac->w,ac->h, 8, ac->w/8);
	maskImg->byte_order = MSBFirst;
	maskImg->bitmap_bit_order = MSBFirst;
	maskPixmap = XCreatePixmap(agDisplay,
	    RootWindow(agDisplay,agScreen),
	    ac->w,ac->h,1);

	/* Blit the data and mask to the pixmaps */
	gcVals.function = GXcopy;
	gcVals.foreground = ~0;
	gcVals.background =  0;
	gcVals.plane_mask = AllPlanes;
	gc = XCreateGC(agDisplay, dataPixmap,
	    GCFunction|GCForeground|GCBackground|GCPlaneMask,
	    &gcVals);
	XPutImage(agDisplay, dataPixmap, gc, dataImg, 0,0, 0,0, ac->w,ac->h);
	XPutImage(agDisplay, maskPixmap, gc, maskImg, 0,0, 0,0, ac->w,ac->h);
	XFreeGC(agDisplay, gc);
	XDestroyImage(dataImg);
	XDestroyImage(maskImg);

	/* Create the X cursor */
	cg->xc = XCreatePixmapCursor(agDisplay, dataPixmap, maskPixmap,
	    &cg->black, &cg->white, ac->xHot, ac->yHot);
	cg->visible = 0;

	XFreePixmap(agDisplay, dataPixmap);
	XFreePixmap(agDisplay, maskPixmap);
	XSync(agDisplay, False);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	ac->p = cg;
	return (0);
}

static void
FreeCursor(void *obj, AG_Cursor *ac)
{
	AG_DriverGLX *glx = obj;
	AG_CursorGLX *cg = ac->p;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);
	
	XFreeCursor(agDisplay, cg->xc);
	XSync(agDisplay, False);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	Free(cg);
	ac->p = NULL;
}

static int
SetCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	AG_CursorGLX *cg = ac->p;

	if (drv->activeCursor == ac) {
		return (0);
	}

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	if (ac == &drv->cursors[0]) {
		XUndefineCursor(agDisplay, glx->w);
	} else {
		XDefineCursor(agDisplay, glx->w, cg->xc);
	}

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	drv->activeCursor = ac;
	cg->visible = 1;
	return (0);
}

static void
UnsetCursor(void *obj)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	
	if (drv->activeCursor == &drv->cursors[0])
		return;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XUndefineCursor(agDisplay, glx->w);
	drv->activeCursor = &drv->cursors[0];
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
}

static int
GetCursorVisibility(void *obj)
{
	/* XXX TODO */
	return (1);
}

static void
SetCursorVisibility(void *obj, int flag)
{
	/* XXX TODO */
}

/*
 * Window operations
 */

/* Wait on X events sent to the given window. */
static Bool
WaitConfigureNotify(Display *d, XEvent *xe, char *arg) {
	return (xe->type == ConfigureNotify) && (xe->xmap.window == (Window)arg);
}
static Bool
WaitMapNotify(Display *d, XEvent *xe, char *arg) {
	return (xe->type == MapNotify) && (xe->xmap.window == (Window)arg);
}
static Bool
WaitUnmapNotify(Display *d, XEvent *xe, char *arg) {
	return (xe->type == UnmapNotify) && (xe->xmap.window == (Window)arg);
}

/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverGLX *glx, int w, int h)
{
	AG_ClipRect *cr;
	int i;

	for (i = 0; i < 4; i++)
		glx->clipStates[i] = 0;

	/* Rectangle 0 always covers the whole view. */
	if ((glx->clipRects = TryMalloc(sizeof(AG_ClipRect))) == NULL) {
		return (-1);
	}
	glx->nClipRects = 1;

	cr = &glx->clipRects[0];
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
InitClipRect0(AG_DriverGLX *glx, AG_Window *win)
{
	AG_ClipRect *cr;

	cr = &glx->clipRects[0];
	cr->r.w = WIDTH(win);
	cr->r.h = HEIGHT(win);
	cr->eqns[2][3] = (double)WIDTH(win);
	cr->eqns[3][3] = (double)HEIGHT(win);
}

/* Initialize the default cursor. */
static int
InitDefaultCursor(AG_DriverGLX *glx)
{
	AG_Driver *drv = AGDRIVER(glx);
	AG_Cursor *ac;
	AG_CursorGLX *cg;
	
	if ((cg = TryMalloc(sizeof(AG_CursorGLX))) == NULL)
		return (-1);
	if ((drv->cursors = TryMalloc(sizeof(AG_Cursor))) == NULL) {
		Free(cg);
		return (-1);
	}

	ac = &drv->cursors[0];
	drv->nCursors = 1;
	AG_CursorInit(ac);
	cg->xc = XCreateFontCursor(agDisplay, XC_left_ptr);
	cg->visible = 1;
	ac->p = cg;
	return (0);
}

/* Set the WM_NORMAL_HINTS property. */
static void
SetWmNormalHints(AG_Window *win, const AG_Rect *r, Uint mwFlags)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XSizeHints *h;

	if ((h = XAllocSizeHints()) == NULL)  {
		return;
	}
	h->flags = PMinSize|PMaxSize;

	if (!(mwFlags & AG_DRIVER_MW_ANYPOS))
		h->flags |= PPosition;

	if (!(win->flags & AG_WINDOW_NORESIZE)) {
		h->min_width = 32;
		h->min_height = 32;
		h->max_width = 4096;
		h->max_height = 4096;
	} else {
		h->min_width = h->max_width = r->w;
		h->min_height = h->max_height = r->h;
	}
	XSetWMNormalHints(agDisplay, glx->w, h);
	XFree(h);
}

/* Try to disable WM titlebars and decorations. */
static void
SetNoDecorationHints(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	Atom wmHints;

	/* For Motif-compliant window managers. */
	if ((wmHints = XInternAtom(agDisplay, "_MOTIF_WM_HINTS", True))
	    != None) {
		/* Hints used by Motif compliant window managers */
		struct {
			unsigned long flags;
			unsigned long functions;
			unsigned long decorations;
			long input_mode;
			unsigned long status;
		} MWMHints = { (1L << 1), 0, 0, 0, 0 };

		XChangeProperty(agDisplay, glx->w,
		    wmHints, wmHints, 32,
		    PropModeReplace,
		    (unsigned char *)&MWMHints,
		    sizeof(MWMHints)/sizeof(long));
	}

	/* For KWM */
	if ((wmHints = XInternAtom(agDisplay, "KWM_WIN_DECORATION", True))
	    != None) {
		long KWMHints = 0;

		XChangeProperty(agDisplay, glx->w,
		    wmHints, wmHints, 32,
		    PropModeReplace,
		    (unsigned char *)&KWMHints,
		    sizeof(KWMHints)/sizeof(long));
	}

	/* For GNOME */
	if ((wmHints = XInternAtom(agDisplay, "_WIN_HINTS", True)) != None) {
		long GNOMEHints = 0;

		XChangeProperty(agDisplay, glx->w,
		    wmHints, wmHints, 32,
		    PropModeReplace,
		    (unsigned char *)&GNOMEHints,
		    sizeof(GNOMEHints)/sizeof(long));
	}
}

static int
OpenWindow(AG_Window *win, AG_Rect r, int depthReq, Uint mwFlags)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	XSetWindowAttributes xwAttrs;
	XVisualInfo *xvi;
	int glxAttrs[16];
	int i = 0;
	Ulong valuemask;
	int depth;
	
	glxAttrs[i++] = GLX_RGBA;
	glxAttrs[i++] = GLX_RED_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_GREEN_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_BLUE_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_DEPTH_SIZE;	glxAttrs[i++] = 16;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	/* Try to obtain a double-buffered visual, fallback to single. */
	glxAttrs[i++] = GLX_DOUBLEBUFFER;
	glxAttrs[i++] = None;
	if ((xvi = glXChooseVisual(agDisplay, agScreen, glxAttrs)) == NULL) {
		glxAttrs[i-=2] = None;
		if ((xvi = glXChooseVisual(agDisplay, agScreen, glxAttrs))
		    == NULL) {
			AG_SetError("Cannot find an acceptable GLX visual");
			goto fail_unlock;
		}
	}

	xwAttrs.colormap = XCreateColormap(agDisplay,
	    RootWindow(agDisplay,xvi->screen),
	    xvi->visual, AllocNone);
	xwAttrs.background_pixmap = None;
	xwAttrs.border_pixel = (xvi->visual == DefaultVisual(agDisplay,agScreen)) ?
	                       BlackPixel(agDisplay,agScreen) : 0;
	xwAttrs.event_mask = KeyPressMask|KeyReleaseMask|
	                     ButtonPressMask|ButtonReleaseMask|
			     EnterWindowMask|LeaveWindowMask|
			     PointerMotionMask|ButtonMotionMask|
			     KeymapStateMask|
			     StructureNotifyMask|
			     FocusChangeMask;
	valuemask = CWColormap | CWBackPixmap | CWBorderPixel | CWEventMask;

	/* Create a new window. */
	depth = (depthReq >= 1) ? depthReq : xvi->depth;
	glx->w = XCreateWindow(agDisplay,
	    RootWindow(agDisplay,agScreen),
	    r.x, r.y,
	    r.w, r.h, 0, depth,
	    InputOutput,
	    xvi->visual,
	    valuemask,
	    &xwAttrs);
	if (glx->w == 0) {
		AG_SetError("XCreateWindow failed");
		goto fail_unlock;
	}
	AGDRIVER_MW(glx)->flags |= AG_DRIVER_MW_OPEN;

	if (win->parent != NULL) {
		SetTransientFor(win, win->parent);
	}
	SetWmNormalHints(win, &r, mwFlags);

	if (win->flags & (AG_WINDOW_NOTITLE|AG_WINDOW_NORESIZE))
		SetNoDecorationHints(win);

	if (!(win->flags & AG_WINDOW_NOCLOSE))
		XSetWMProtocols(agDisplay, glx->w, &wmDeleteWindow, 1);
	
	/* Create the GLX rendering context. */
	glx->glxCtx = glXCreateContext(agDisplay, xvi, 0, GL_FALSE);
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);

	/* Set the pixel formats. */
	drv->videoFmt = AG_PixelFormatRGB(depth,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000, 0x00ff0000, 0x0000ff00
#else
		0x000000ff, 0x0000ff00, 0x00ff0000
#endif
	);
	if (drv->videoFmt == NULL)
		goto fail;

	/* Initialize the clipping rectangle stack. */
	if (InitClipRects(glx, r.w, r.h) == -1)
		goto fail;
	
	/* Create the built-in cursors. */
	if (InitDefaultCursor(glx) == -1 ||
	    AG_InitStockCursors(drv) == -1) {
		goto fail;
	}
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
fail:
	glXDestroyContext(agDisplay, glx->glxCtx);
	XDestroyWindow(agDisplay, glx->w);
	AGDRIVER_MW(glx)->flags &= ~(AG_DRIVER_MW_OPEN);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
fail_unlock:
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (-1);
}

static void
CloseWindow(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverGLX *glx = (AG_DriverGLX *)drv;
/*	AG_Glyph *gl; */
/*	int i; */

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
#if 0
	/* Invalidate cached glyph textures. */
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		SLIST_FOREACH(gl, &drv->glyphCache[i].glyphs, glyphs) {
			if (gl->texture != 0) {
				glDeleteTextures(1, (GLuint *)&gl->texture);
				gl->texture = 0;
			}
		}
	}
#endif
	glXDestroyContext(agDisplay, glx->glxCtx);
	XDestroyWindow(agDisplay, glx->w);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	AGDRIVER_MW(glx)->flags &= ~(AG_DRIVER_MW_OPEN);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
}

static int
MapWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	AG_SizeAlloc a;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XMapWindow(agDisplay, glx->w);
	XIfEvent(agDisplay, &xev, WaitMapNotify, (char *)glx->w);
	a.x = WIDGET(win)->x;
	a.y = WIDGET(win)->y;
	a.w = WIDTH(win);
	a.h = HEIGHT(win);
	PostResizeCallback(win, &a);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	return (0);
}

static int
UnmapWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XUnmapWindow(agDisplay, glx->w);
	XIfEvent(agDisplay, &xev, WaitUnmapNotify, (char *)glx->w);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
RaiseWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XRaiseWindow(agDisplay, glx->w);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
LowerWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XLowerWindow(agDisplay, glx->w);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
ReparentWindow(AG_Window *win, AG_Window *winParent, int x, int y)
{
	AG_DriverGLX *glxWin = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_DriverGLX *glxParentWin = (AG_DriverGLX *)WIDGET(winParent)->drv;
/*	XEvent xev; */
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glxWin->lock);
	AG_MutexLock(&glxParentWin->lock);

	XReparentWindow(agDisplay, glxWin->w, glxParentWin->w, x,y);

	AG_MutexUnlock(&glxParentWin->lock);
	AG_MutexUnlock(&glxWin->lock);
	AG_MutexUnlock(&agDisplayLock);

/*	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w); */
	return (0);
}

static int
GetInputFocus(AG_Window **rv)
{
	AG_DriverGLX *glx = NULL;
	Window wRet;
	int revertToRet;

	AG_MutexLock(&agDisplayLock);
	XGetInputFocus(agDisplay, &wRet, &revertToRet);
	AG_MutexUnlock(&agDisplayLock);

	AGOBJECT_FOREACH_CHILD(glx, &agDrivers, ag_driver_glx) {
		if (!AGDRIVER_IS_GLX(glx)) {
			continue;
		}
		if (glx->w == wRet)
			break;
	}
	if (glx == NULL) {
		AG_SetError("Input focus is external to this application");
		return (-1);
	}
	*rv = AGDRIVER_MW(glx)->win;
	return (0);
}

static int
SetInputFocus(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XSetInputFocus(agDisplay, glx->w, RevertToParent, CurrentTime);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static void
PreResizeCallback(AG_Window *win)
{
#if 0
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;

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
PostResizeCallback(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverGLX *glx = (AG_DriverGLX *)drv;
	char kv[32];
	int x = a->x;
	int y = a->y;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	/* Update per-widget coordinate information. */
	a->x = 0;
	a->y = 0;
	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Update clipping rectangle 0 */
	InitClipRect0(glx, win);
	
	/* Update the keyboard state. XXX */
	XQueryKeymap(agDisplay, kv);
	UpdateKeyboard(drv->kbd, kv);

	/* Update GLX context. */
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	AG_GL_InitContext(AG_RECT(0, 0, WIDTH(win), HEIGHT(win)));
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	/* Save the new effective window position. */
	WIDGET(win)->x = a->x = x;
	WIDGET(win)->y = a->y = y;
}

static void
PostMoveCallback(AG_Window *win, AG_SizeAlloc *a)
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
MoveWindow(AG_Window *win, int x, int y)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	/* printf("XMoveWindow(%d,%d)\n", x, y); */
	XMoveWindow(agDisplay, glx->w, x, y);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	return (0);
}

#if 0
/* Save/restore associated widget GL resources (for GL context changes). */
static void
FreeWidgetResources(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		FreeWidgetResources(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}
static void
RegenWidgetResources(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		RegenWidgetResources(chld);
	}
	AG_WidgetRegenResourcesGL(wid);
}
#endif

static int
ResizeWindow(AG_Window *win, Uint w, Uint h)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);
	
	/* Resize the window and wait for notification from the server. */
	XResizeWindow(agDisplay, glx->w, w, h);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
MoveResizeWindow(AG_Window *win, AG_SizeAlloc *a)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	/* printf("XMoveResizeWindow: %d,%d (%dx%d)\n",
	    a->x, a->y, a->w, a->h); */
	XMoveResizeWindow(agDisplay, glx->w,
	    a->x, a->y,
	    a->w, a->h);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	return (0);
}

static int
SetBorderWidth(AG_Window *win, Uint width)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XSetWindowBorderWidth(agDisplay, glx->w, width);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
SetWindowCaption(AG_Window *win, const char *s)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XTextProperty xtp;
	Status rv;

	if (s[0] == '\0') {
		return (0);
	}

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

#ifdef X_HAVE_UTF8_STRING
	/* XFree86 "utf8" and "UTF8" functions are available */
	rv = Xutf8TextListToTextProperty(agDisplay,
	    (char **)&s, 1, XUTF8StringStyle,
	    &xtp);
#else
	rv = XStringListToTextProperty(
	    (char **)&s, 1,
	    &xtp);
#endif
	if (rv != Success) {
		AG_SetError("Cannot convert string to X property");
		goto fail;
	}
	XSetTextProperty(agDisplay, glx->w, &xtp, XA_WM_NAME);
	XFree(xtp.value);
	XSync(agDisplay, False);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
fail:
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (-1);
}

static void
SetTransientFor(AG_Window *win, AG_Window *winParent)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_DriverGLX *glxParent;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);
	
	if (winParent != NULL &&
	    (glxParent = (AG_DriverGLX *)WIDGET(winParent)->drv) != NULL &&
	    AGDRIVER_IS_GLX(glxParent) &&
	    (AGDRIVER_MW(glxParent)->flags & AG_DRIVER_MW_OPEN)) {
		XSetTransientForHint(agDisplay, glx->w, glxParent->w);
	} else {
		XSetTransientForHint(agDisplay, glx->w, None);
	}
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
}

AG_DriverMwClass agDriverGLX = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverGLX",
			sizeof(AG_DriverGLX),
			{ 1,4 },
			Init,
			NULL,	/* reinit */
			Destroy,
			NULL,	/* load */
			NULL,	/* save */
			NULL,	/* edit */
		},
		"glx",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL|AG_DRIVER_TEXTURES,
		Open,
		Close,
		GetDisplaySize,
		NULL,			/* beginEventProcessing */
		ProcessEvents,
		GenericEventLoop,
		NULL,			/* endEventProcessing */
		Terminate,
		BeginRendering,
		RenderWindow,
		EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_UploadTexture,
		AG_GL_UpdateTexture,
		DeleteTexture,
		SetRefreshRate,
		PushClipRect,
		PopClipRect,
		PushBlendingMode,
		PopBlendingMode,
		CreateCursor,
		FreeCursor,
		SetCursor,
		UnsetCursor,
		GetCursorVisibility,
		SetCursorVisibility,
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
	OpenWindow,
	CloseWindow,
	MapWindow,
	UnmapWindow,
	RaiseWindow,
	LowerWindow,
	ReparentWindow,
	GetInputFocus,
	SetInputFocus,
	MoveWindow,
	ResizeWindow,
	MoveResizeWindow,
	PreResizeCallback,
	PostResizeCallback,
	NULL,				/* captureWindow */
	SetBorderWidth,
	SetWindowCaption,
	SetTransientFor
};

#endif /* HAVE_GLX */

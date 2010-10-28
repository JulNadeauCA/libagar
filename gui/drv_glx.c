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
#include "opengl.h"

#define DEBUG_XSYNC

static int      nDrivers = 0;		/* Drivers open */
static Display *agDisplay = NULL;	/* X display (shared) */
static int      agScreen = 0;		/* Default screen (shared) */
static Uint     rNom = 20;		/* Nominal refresh rate (ms) */
static int      rCur = 0;		/* Effective refresh rate (ms) */
static AG_Mutex agDisplayLock;		/* Lock on agDisplay */
static int      agExitGLX = 0;		/* Exit event loop */

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
	Uint            *listGC;	/* Display lists queued for deletion */
	Uint            nListGC;
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

static void GLX_PostResizeCallback(AG_Window *, AG_SizeAlloc *);
static void GLX_PostMoveCallback(AG_Window *, AG_SizeAlloc *);
static int  GLX_RaiseWindow(AG_Window *);
static int  GLX_SetInputFocus(AG_Window *);
static void GLX_SetTransientFor(AG_Window *, AG_Window *);

static void
Init(void *obj)
{
	AG_DriverGLX *glx = obj;

	glx->clipRects = NULL;
	glx->nClipRects = 0;
	memset(glx->clipStates, 0, sizeof(glx->clipStates));
	glx->textureGC = NULL;
	glx->nTextureGC = 0;
	glx->listGC = NULL;
	glx->nListGC = 0;
	glx->w = 0;
	AG_MutexInitRecursive(&glx->lock);
}

static void
Destroy(void *obj)
{
	AG_DriverGLX *glx = obj;

	AG_MutexDestroy(&glx->lock);
	Free(glx->clipRects);
	Free(glx->textureGC);
	Free(glx->listGC);
}

/*
 * Driver initialization
 */

static int
InitGlobals(void)
{
	int err, ev;

	if ((agDisplay = XOpenDisplay(NULL)) == NULL) {
		AG_SetError("Cannot open X display");
		return (-1);
	}
	if (!glXQueryExtension(agDisplay, &err, &ev)) {
		AG_SetError("GLX extension is not available");
		XCloseDisplay(agDisplay);
		agDisplay = NULL;
		return (-1);
	}

	AG_MutexInitRecursive(&agDisplayLock);
	agScreen = DefaultScreen(agDisplay);
	InitKeymaps();
	memset(xkbBuf, '\0', sizeof(xkbBuf));
	memset(&xkbCompStatus, 0, sizeof(xkbCompStatus));
	wmDeleteWindow = XInternAtom(agDisplay, "WM_DELETE_WINDOW", False);
	return (0);
}

static void
DestroyGlobals(void)
{
	AG_MutexLock(&agDisplayLock);
	XCloseDisplay(agDisplay);
	agDisplay = NULL;
	agScreen = 0;
	AG_MutexUnlock(&agDisplayLock);
	AG_MutexDestroy(&agDisplayLock);
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
GLX_Open(void *obj, const char *spec)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	int initedGlobals = 0;

	if (agDisplay == NULL) {
		if (InitGlobals() == -1) {
			goto fail;
		}
		initedGlobals = 1;
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
	if (initedGlobals) {
		DestroyGlobals();
	}
	AG_MutexUnlock(&agDisplayLock);
	return (-1);
}

static void
GLX_Close(void *obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0)
		DestroyGlobals();

	AG_ObjectDetach(drv->mouse);
	AG_ObjectDestroy(drv->mouse);
	AG_ObjectDetach(drv->kbd);
	AG_ObjectDestroy(drv->kbd);
	
	drv->mouse = NULL;
	drv->kbd = NULL;
}

static int
GLX_GetDisplaySize(Uint *w, Uint *h)
{
	AG_MutexLock(&agDisplayLock);
	*w = (Uint)DisplayWidth(agDisplay, agScreen);
	*h = (Uint)DisplayHeight(agDisplay, agScreen);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

/*
 * Get Agar keycode corresponding to an X KeyCode.
 * agDisplayLock must be held.
 */
static int
LookupKeyCode(KeyCode keycode, AG_KeySym *rv)
{
	KeySym ks;
	
	if ((ks = XKeycodeToKeysym(agDisplay, keycode, 0)) == 0) {
		*rv = AG_KEY_NONE;
		return (0);
	}
	switch (ks>>8) {
#ifdef XK_MISCELLANY
	case 0xff:
		*rv = agKeymapMisc[ks&0xff];
		return (1);
#endif
#ifdef XK_XKB_KEYS
	case 0xfe:
		*rv = agKeymapXKB[ks&0xff];
		return (1);
#endif
	default:
		*rv = (AG_KeySym)(ks&0xff);
		return (1);
	}
	return (0);
}

/* Return the Agar window corresponding to a X Window ID */
static __inline__ AG_Window *
LookupWindowByID(Window xw)
{
	AG_Window *win;
	AG_DriverGLX *glx;

	/* XXX TODO portable to optimize based on numerical XIDs? */
	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(glx, &agDrivers, ag_driver_glx) {
		if (!AGDRIVER_IS_GLX(glx)) {
			continue;
		}
		if (glx->w == xw) {
			win = AGDRIVER_MW(glx)->win;
			if (WIDGET(win)->drv == NULL) {	/* Being detached */
				goto fail;
			}
			AG_UnlockVFS(&agDrivers);
			return (win);
		}
	}
fail:
	AG_UnlockVFS(&agDrivers);
	AG_SetError("X event from unknown window");
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

#ifdef XK_MISCELLANY
			switch (ks) {
			case XK_Num_Lock:	modMasks.num = mask;	break;
			case XK_Alt_L:		modMasks.Lalt = mask;	break;
			case XK_Alt_R:		modMasks.Ralt = mask;	break;
			case XK_Meta_L:		modMasks.Lmeta = mask;	break;
			case XK_Meta_R:		modMasks.Rmeta = mask;	break;
			case XK_Mode_switch:	modMasks.mode = mask;	break;
			}
#endif /* XK_MISCELLANY */
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

/* Refresh the internal keyboard state for all glx driver instances. */
static void
UpdateKeyboardAll(char *kv)
{
	AG_Driver *drv;

	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (!AGDRIVER_IS_GLX(drv)) {
			continue;
		}
		UpdateKeyboard(drv->kbd, kv);
	}
	AG_UnlockVFS(&agDrivers);
}

static __inline__ int
GLX_PendingEvents(void *drvCaller)
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

static int
PostEventCallback(void *drvCaller)
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
		AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(agWindowToFocus)->drv;

		if (glx != NULL && AGDRIVER_IS_GLX(glx)) {
			AG_MutexLock(&agDisplayLock);
			AG_MutexLock(&glx->lock);
			GLX_RaiseWindow(agWindowToFocus);
			GLX_SetInputFocus(agWindowToFocus);
			AG_MutexUnlock(&glx->lock);
			AG_MutexUnlock(&agDisplayLock);
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
GLX_GetNextEvent(void *drvCaller, AG_DriverEvent *dev)
{
	XEvent xev;
	int x, y;
	AG_KeySym ks;
	AG_Window *win;
	Uint32 ucs;
	
	AG_MutexLock(&agDisplayLock);
	XNextEvent(agDisplay, &xev);
	AG_MutexUnlock(&agDisplayLock);

	switch (xev.type) {
	case MotionNotify:
		if ((win = LookupWindowByID(xev.xmotion.window)) == NULL) {
			return (-1);
		}
		x = AGDRIVER_BOUNDED_WIDTH(win, xev.xmotion.x);
		y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xmotion.y);
		AG_MouseMotionUpdate(WIDGET(win)->drv->mouse, x,y);
		
		dev->type = AG_DRIVER_MOUSE_MOTION;
		dev->win = win;
		dev->data.motion.x = x;
		dev->data.motion.y = y;
		break;
	case ButtonPress:
		if ((win = LookupWindowByID(xev.xbutton.window)) == NULL) {
			return (-1);
		}
		x = AGDRIVER_BOUNDED_WIDTH(win, xev.xbutton.x);
		y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xbutton.y);
		AG_MouseButtonUpdate(WIDGET(win)->drv->mouse,
		    AG_BUTTON_PRESSED, xev.xbutton.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->win = win;
		dev->data.button.which = (AG_MouseButton)xev.xbutton.button;
		dev->data.button.x = xev.xbutton.x;
		dev->data.button.y = xev.xbutton.y;
		break;
	case ButtonRelease:
		if ((win = LookupWindowByID(xev.xbutton.window)) == NULL) {
			return (-1);
		}
		x = AGDRIVER_BOUNDED_WIDTH(win, xev.xbutton.x);
		y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xbutton.y);
		AG_MouseButtonUpdate(WIDGET(win)->drv->mouse,
		    AG_BUTTON_RELEASED, xev.xbutton.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
		dev->win = win;
		dev->data.button.which = (AG_MouseButton)xev.xbutton.button;
		dev->data.button.x = xev.xbutton.x;
		dev->data.button.y = xev.xbutton.y;
		break;
	case KeyRelease:
		AG_MutexLock(&agDisplayLock);
		if (IsKeyRepeat(&xev)) {
			/* We implement key repeat internally */
			AG_MutexUnlock(&agDisplayLock);
			return (0);
		}
		AG_MutexUnlock(&agDisplayLock);
		/* FALLTHROUGH */
	case KeyPress:
		AG_MutexLock(&agDisplayLock);
		if (XLookupString(&xev.xkey, xkbBuf, sizeof(xkbBuf),
		    NULL, &xkbCompStatus) >= 1) {
			ucs = (Uint8)xkbBuf[0];	/* XXX */
		} else {
			ucs = 0;
		}
		if (!LookupKeyCode(xev.xkey.keycode, &ks)) {
			AG_SetError("Keyboard event: Unknown keycode: %d",
			    (int)xev.xkey.keycode);
			AG_MutexUnlock(&agDisplayLock);
			return (-1);
		}
		AG_MutexUnlock(&agDisplayLock);

		if ((win = LookupWindowByID(xev.xkey.window)) == NULL) {
			return (-1);
		}
		AG_KeyboardUpdate(WIDGET(win)->drv->kbd,
		    (xev.type == KeyPress) ? AG_KEY_PRESSED : AG_KEY_RELEASED,
		    ks, ucs);

		dev->type = (xev.type == KeyPress) ? AG_DRIVER_KEY_DOWN :
		                                     AG_DRIVER_KEY_UP;
		dev->win = win;
		dev->data.key.ks = ks;
		dev->data.key.ucs = ucs;
		break;
	case EnterNotify:
		if ((win = LookupWindowByID(xev.xcrossing.window)) == NULL) {
			return (-1);
		}
		dev->type = AG_DRIVER_MOUSE_ENTER;
		dev->win = win;
		break;
	case LeaveNotify:
		if ((win = LookupWindowByID(xev.xcrossing.window)) == NULL) {
			return (-1);
		}
		dev->type = AG_DRIVER_MOUSE_LEAVE;
		dev->win = win;
		break;
	case FocusIn:
		if ((win = LookupWindowByID(xev.xfocus.window)) == NULL) {
			return (-1);
		}
		agWindowFocused = win;
		AG_PostEvent(NULL, win, "window-gainfocus", NULL);
		dev->type = AG_DRIVER_FOCUS_IN;
		dev->win = win;
		break;
	case FocusOut:
		if ((win = LookupWindowByID(xev.xfocus.window)) == NULL) {
			return (-1);
		}
		if (agWindowFocused == win) {
			AG_PostEvent(NULL, win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		dev->type = AG_DRIVER_FOCUS_OUT;
		dev->win = win;
		break;
	case KeymapNotify:					/* Internal */
		UpdateKeyboardAll(xev.xkeymap.key_vector);
		return (0);
	case MappingNotify:					/* Internal */
		AG_MutexLock(&agDisplayLock);
		XRefreshKeyboardMapping(&xev.xmapping);
		AG_MutexUnlock(&agDisplayLock);
		return (0);
	case ConfigureNotify:					/* Internal */
		if ((win = LookupWindowByID(xev.xconfigure.window))) {
			Window ignore;

			AG_MutexLock(&agDisplayLock);
			XTranslateCoordinates(agDisplay,
			    xev.xconfigure.window,
			    DefaultRootWindow(agDisplay),
			    0, 0,
			    &x, &y,
			    &ignore);
			AG_MutexUnlock(&agDisplayLock);

			dev->type = AG_DRIVER_VIDEORESIZE;
			dev->win = win;
			dev->data.videoresize.x = x;
			dev->data.videoresize.y = y;
			dev->data.videoresize.w = xev.xconfigure.width;
			dev->data.videoresize.h = xev.xconfigure.height;
			break;
		} else {
			return (-1);
		}
	case Expose:
		if ((win = LookupWindowByID(xev.xexpose.window)) == NULL) {
			return (-1);
		}
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = win;
		break;
	case ClientMessage:
		if ((xev.xclient.format == 32) &&
		    (xev.xclient.data.l[0] == wmDeleteWindow) &&
		    (win = LookupWindowByID(xev.xclient.window))) {
			dev->type = AG_DRIVER_CLOSE;
			dev->win = win;
			break;
		}
		return (0);
	case MapNotify:
	case UnmapNotify:
	case DestroyNotify:
	case ReparentNotify:
		return (0);
	default:
		AG_SetError("Unknown X event %d\n", xev.type);
		return (-1);
	}
	return (1);
}

static int
GLX_ProcessEvent(void *drvCaller, AG_DriverEvent *dev)
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
			GLX_PostResizeCallback(dev->win, &a);
		} else {
			GLX_PostMoveCallback(dev->win, &a);
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(NULL, dev->win, "window-close", NULL);
		break;
	case AG_DRIVER_EXPOSE:
		dev->win->dirty = 1;
		break;
	default:
		break;
	}
	return PostEventCallback(drvCaller);
}

static void
GLX_GenericEventLoop(void *obj)
{
	AG_Driver *drv;
	AG_DriverGLX *glx;
	AG_DriverEvent dev;
	AG_Window *win;
	Uint32 t1, t2;

#ifdef AG_DEBUG
	AG_PerfMonInit();
#endif
	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();
		if (agExitGLX) {
			break;
		} else if (t2 - t1 >= rNom) {
			AG_LockVFS(&agDrivers);
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (!AGDRIVER_IS_GLX(drv)) {
					continue;
				}
				glx = (AG_DriverGLX *)drv;
				AG_MutexLock(&glx->lock);
				win = AGDRIVER_MW(drv)->win;
				if (win->visible && win->dirty) {
					AG_BeginRendering(drv);
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
					AG_EndRendering(drv);
				}
				AG_MutexUnlock(&glx->lock);
			}
			AG_UnlockVFS(&agDrivers);
			t1 = AG_GetTicks();
			rCur = rNom - (t1-t2);
			if (rCur < 1) { rCur = 1; }
#ifdef AG_DEBUG
			if (agPerfWindow->visible)
				AG_PerfMonUpdate(rCur);
#endif
		} else if (GLX_PendingEvents(NULL) != 0) {
			if (GLX_GetNextEvent(NULL, &dev) == 1 &&
			    GLX_ProcessEvent(NULL, &dev) == -1)
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
GLX_Terminate(void)
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
	agExitGLX = 1;
	AG_MutexUnlock(&agDisplayLock);
}

static void
GLX_BeginRendering(void *obj)
{
	AG_DriverGLX *glx = obj;

	AG_MutexLock(&agDisplayLock);
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	AG_MutexUnlock(&agDisplayLock);
}

static void
GLX_RenderWindow(AG_Window *win)
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
GLX_EndRendering(void *obj)
{
	AG_DriverGLX *glx = obj;
	Uint i;
	
	AG_MutexLock(&agDisplayLock);

	/* Exchange front and back buffers. */
	glXSwapBuffers(agDisplay, glx->w);

	/* Remove textures and display lists queued for deletion. */
	glDeleteTextures(glx->nTextureGC, (const GLuint *)glx->textureGC);
	for (i = 0; i < glx->nListGC; i++) {
		glDeleteLists(glx->listGC[i], 1);
	}
	glx->nTextureGC = 0;
	glx->nListGC = 0;

	AG_MutexUnlock(&agDisplayLock);
}

static void
GLX_DeleteTexture(void *drv, Uint texture)
{
	AG_DriverGLX *glx = drv;

	AG_MutexLock(&agDisplayLock);
	glx->textureGC = Realloc(glx->textureGC, (glx->nTextureGC+1)*sizeof(Uint));
	glx->textureGC[glx->nTextureGC++] = texture;
	AG_MutexUnlock(&agDisplayLock);
}

static void
GLX_DeleteList(void *drv, Uint list)
{
	AG_DriverGLX *glx = drv;

	AG_MutexLock(&agDisplayLock);
	glx->listGC = Realloc(glx->listGC, (glx->nListGC+1)*sizeof(Uint));
	glx->listGC[glx->nListGC++] = list;
	AG_MutexUnlock(&agDisplayLock);
}

static int
GLX_SetRefreshRate(void *obj, int fps)
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
GLX_PushClipRect(void *obj, AG_Rect r)
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
GLX_PopClipRect(void *obj)
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
GLX_PushBlendingMode(void *obj, AG_BlendFn fnSrc, AG_BlendFn fnDst)
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
GLX_PopBlendingMode(void *obj)
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
GLX_CreateCursor(void *obj, AG_Cursor *ac)
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
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	
	XSync(agDisplay, False);

	ac->p = cg;
	return (0);
}

static void
GLX_FreeCursor(void *obj, AG_Cursor *ac)
{
	AG_DriverGLX *glx = obj;
	AG_CursorGLX *cg = ac->p;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);
	
	XFreeCursor(agDisplay, cg->xc);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	
	XSync(agDisplay, False);

	Free(cg);
	ac->p = NULL;
}

static int
GLX_SetCursor(void *obj, AG_Cursor *ac)
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
	
	XSync(agDisplay, False);

	drv->activeCursor = ac;
	cg->visible = 1;
	return (0);
}

static void
GLX_UnsetCursor(void *obj)
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
	
	XSync(agDisplay, False);
}

static int
GLX_GetCursorVisibility(void *obj)
{
	/* XXX TODO */
	return (1);
}

static void
GLX_SetCursorVisibility(void *obj, int flag)
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
GLX_OpenWindow(AG_Window *win, AG_Rect r, int depthReq, Uint mwFlags)
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
			     FocusChangeMask|
			     ExposureMask;
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
		GLX_SetTransientFor(win, win->parent);
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
GLX_CloseWindow(AG_Window *win)
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
GLX_MapWindow(AG_Window *win)
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
	GLX_PostResizeCallback(win, &a);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);

	return (0);
}

static int
GLX_UnmapWindow(AG_Window *win)
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
GLX_RaiseWindow(AG_Window *win)
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
GLX_LowerWindow(AG_Window *win)
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
GLX_ReparentWindow(AG_Window *win, AG_Window *winParent, int x, int y)
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
GLX_GetInputFocus(AG_Window **rv)
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
GLX_SetInputFocus(AG_Window *win)
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
GLX_PreResizeCallback(AG_Window *win)
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
GLX_PostResizeCallback(AG_Window *win, AG_SizeAlloc *a)
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
GLX_PostMoveCallback(AG_Window *win, AG_SizeAlloc *a)
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
GLX_MoveWindow(AG_Window *win, int x, int y)
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
GLX_ResizeWindow(AG_Window *win, Uint w, Uint h)
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
GLX_MoveResizeWindow(AG_Window *win, AG_SizeAlloc *a)
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
GLX_SetBorderWidth(AG_Window *win, Uint width)
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
GLX_SetWindowCaption(AG_Window *win, const char *s)
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
GLX_SetTransientFor(AG_Window *win, AG_Window *winParent)
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
		GLX_Open,
		GLX_Close,
		GLX_GetDisplaySize,
		NULL,			/* beginEventProcessing */
		GLX_PendingEvents,
		GLX_GetNextEvent,
		GLX_ProcessEvent,
		GLX_GenericEventLoop,
		NULL,			/* endEventProcessing */
		GLX_Terminate,
		GLX_BeginRendering,
		GLX_RenderWindow,
		GLX_EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_UploadTexture,
		AG_GL_UpdateTexture,
		GLX_DeleteTexture,
		GLX_SetRefreshRate,
		GLX_PushClipRect,
		GLX_PopClipRect,
		GLX_PushBlendingMode,
		GLX_PopBlendingMode,
		GLX_CreateCursor,
		GLX_FreeCursor,
		GLX_SetCursor,
		GLX_UnsetCursor,
		GLX_GetCursorVisibility,
		GLX_SetCursorVisibility,
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
		GLX_DeleteList
	},
	GLX_OpenWindow,
	GLX_CloseWindow,
	GLX_MapWindow,
	GLX_UnmapWindow,
	GLX_RaiseWindow,
	GLX_LowerWindow,
	GLX_ReparentWindow,
	GLX_GetInputFocus,
	GLX_SetInputFocus,
	GLX_MoveWindow,
	GLX_ResizeWindow,
	GLX_MoveResizeWindow,
	GLX_PreResizeCallback,
	GLX_PostResizeCallback,
	NULL,				/* captureWindow */
	GLX_SetBorderWidth,
	GLX_SetWindowCaption,
	GLX_SetTransientFor
};

#endif /* HAVE_GLX */

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

/* The GLX_{Get,Set}ClipboardText() routines are based on code from SDL2 : */
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Driver for OpenGL graphics via X Windows System. 
 * One GL context is created for each Agar window.
 */

#include <agar/config/have_xkb.h>
#include <agar/config/have_xf86misc.h>
#include <agar/config/have_xinerama.h>

#include <agar/core/core.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#ifdef HAVE_XKB
#include <X11/XKBlib.h>
#endif
#ifdef HAVE_XF86MISC
#include <X11/extensions/xf86misc.h>
#endif
#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>

/* #define DEBUG_XEVENTS */
/* #define DEBUG_MOTION_XEVENTS */
#define DEBUG_CLIPBOARD

static int nDrivers = 0;                         /* Drivers open */
static int agScreen = 0;                         /* X screen (shared) */

static char           xkbBuf[64];                /* For Unicode translation */
static XComposeStatus xkbCompStatus;             /* For key composition */

static _Nonnull_Mutex AG_Mutex agDisplayLock;    /* Lock on agDisplay */
static Display *_Nullable      agDisplay = NULL; /* X display handle */

static Window agClipboardWindow;                 /* For managing SelectionRequest */
static int    agClipboardSelectionWaiting = 0;   /* Waiting on clipboard event */

AG_EventSink *_Nullable glxEventSink = NULL;     /* Process X events */
AG_EventSink *_Nullable glxEventEpilogue = NULL; /* Event sink epilogue */
AG_EventSink *_Nullable glxEventSpinner = NULL;  /* For agTimeOps_renderer */

/* Driver instance data */
typedef struct ag_driver_glx {
	struct ag_driver_mw _inherit;
	Window w;                    /* X window */
	GLXContext glxCtx;           /* GLX context */
	AG_GL_Context gl;            /* Common OpenGL context data */
	AG_Mutex lock;               /* Protect Xlib calls */
	int wmHintsSet;              /* WM hints have been set */
	int ptrIsGrabbed;            /* Active grab successful */
	AG_Timer toInitExpose;       /* Initial Expose */
} AG_DriverGLX;

static int modMasksInited = 0;		/* For modifier key translation */
struct {
	Uint Lmeta, Rmeta;
	Uint Lalt, Ralt;
	Uint num, mode;
} modMasks;
struct ag_glx_key_mapping {		/* Keymap translation table entry */
	int kcode;			/* Scancode */
	int kclass;			/* X keysym class (e.g., 0xff) */
	AG_KeySym key;			/* Corresponding Agar keysym */
};
#include <agar/gui/drv_glx_keymaps.h>

typedef struct ag_cursor_glx {
	struct ag_cursor _inherit;
	XColor black;
	XColor white;
	Cursor xc;
	int visible;
	int shared;
} AG_CursorGLX;

AG_DriverMwClass agDriverGLX;
#if 0
#define AGDRIVER_IS_GLX(drv) AG_OfClass((drv),"AG_Driver:AG_DriverMw:AG_DriverGLX")
#else
#define AGDRIVER_IS_GLX(drv) (AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverGLX)
#endif

#ifdef AG_WM_HINTS
static Atom wmDeleteWindow, wmNetWmWindowOpacity, wmProtocols, wmTakeFocus,
            wmMotifWmHints, wmKwmWinDecoration, wmWinHints, wmNetWmState,
            wmNetWmStateModal, wmNetWmStateSkipTaskbar, wmNetWmStateAbove,
            wmNetWmStateBelow, wmNetWmWindowType, wmNetWmWindowOpacity;
#endif

static int  GLX_InitGlobals(void);
static void GLX_PostResizeCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static void GLX_PostMoveCallback(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);
static void GLX_SetTransientFor(AG_Window *, AG_Window *_Nullable);
static int  GLX_SetClipboardText(void *_Nonnull, const char *_Nonnull);

#ifdef AG_WM_HINTS
static void SetHints(AG_Window *_Nonnull);
static void SetMotifWmHints(AG_Window *_Nonnull);
static void SetKwmWmHints(AG_Window *_Nonnull);
static void SetGnomeWmHints(AG_Window *_Nonnull);
#endif

static void
Init(void *_Nonnull obj)
{
	AG_DriverGLX *glx = obj;
	AG_DriverMw *dmw = obj;

	dmw->flags |= AG_DRIVER_MW_ANYPOS_AVAIL;
	glx->w = 0;
	glx->wmHintsSet = 0;
	glx->ptrIsGrabbed = 0;
	AG_MutexInitRecursive(&glx->lock);
}

static void
Destroy(void *_Nonnull obj)
{
	AG_DriverGLX *glx = obj;

	AG_MutexDestroy(&glx->lock);
}

/*
 * Process events on Xlib internal connections. This is used, notably,
 * for connections to IM servers with the X Input Method Protocol.
 */
static int
GLX_EventSinkInternal(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	Display *dpy = AG_PTR(1);

	XProcessInternalConnection(dpy, es->ident);
	return (1);
}
static void
GLX_ConnectionWatchProc(Display *dpy, XPointer clntData, int fd,
    Bool opening, XPointer *watchData)
{
#ifdef DEBUG_XEVENTS
	Debug(NULL, "Xlib %s fd %d\n", opening ? "opened" : "closed", fd);
#endif
	if (opening) {
		AG_AddEventSink(AG_SINK_READ, fd, 0,
		    GLX_EventSinkInternal, "%p", dpy);
	} else {
		AG_DelEventSinksByIdent(AG_SINK_READ, fd, 0);
	}
}

static void
GLX_DestroyGlobals(void)
{
	if (nDrivers > 0)
		return;

	if (agDisplay) {
		int *intFds, nIntFds, i;
		
		AG_DelEventSink(glxEventSink); glxEventSink = NULL;
		AG_DelEventEpilogue(glxEventEpilogue); glxEventEpilogue = NULL;
		
		XRemoveConnectionWatch(agDisplay, GLX_ConnectionWatchProc, NULL);
		if (XInternalConnectionNumbers(agDisplay, &intFds, &nIntFds)) {
			for (i = 0; i < nIntFds; i++) {
				AG_DelEventSinksByIdent(AG_SINK_READ, intFds[i], 0);
			}
			XFree(intFds);
		}
		(void)XConnectionNumber(agDisplay);
		XCloseDisplay(agDisplay);
		agDisplay = NULL;
	}
	if (glxEventSpinner != NULL) {
		AG_DelEventSink(glxEventSpinner);
		glxEventSpinner = NULL;
	}
	agScreen = 0;
	AG_MutexDestroy(&agDisplayLock);
}

static int
HandleErrorX11(Display *disp, XErrorEvent *xev)
{
	char buf[128];

	XGetErrorText(disp, xev->error_code, buf, sizeof(buf));
	fprintf(stderr, "Caught X error:\n");
	fprintf(stderr, "%s\n", buf);
	abort();
}

#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
static int
GLX_EventSpin(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	AG_Delay(1);
	return (0);
}
#endif

static int
GLX_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;

	if (GLX_InitGlobals() == -1)
		return (-1);

	AG_MutexLock(&agDisplayLock);
	nDrivers++;

	/* Register the core X mouse and keyboard */
	if ((drv->mouse = AG_MouseNew(glx, "X mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(glx, "X keyboard")) == NULL)
		goto fail;
	
	/* Driver manages rendering of window background. */
	drv->flags |= AG_DRIVER_WINDOW_BG;
	
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
	if (agTimeOps == &agTimeOps_renderer)
		glxEventSpinner = AG_AddEventSpinner(GLX_EventSpin, "%p", drv);
#endif
	if (agXsync) {
		Debug(glx, "Enabling synchronous X events\n");
		XSynchronize(agDisplay, True);
		XSetErrorHandler(HandleErrorX11);
	}
	AG_MutexUnlock(&agDisplayLock);
	return (0);
fail:
	if (drv->kbd != NULL) { AG_ObjectDelete(drv->kbd); drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	AG_MutexUnlock(&agDisplayLock);
	if (--nDrivers == 0) {
		GLX_DestroyGlobals();
	}
	return (-1);
}

static void
GLX_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	AG_MutexLock(&agDisplayLock);
	if (drv->kbd) {
		AG_ObjectDetach(drv->kbd);
		AG_ObjectDestroy(drv->kbd);
		drv->kbd = NULL;
	}
	if (drv->mouse) {
		AG_ObjectDetach(drv->mouse);
		AG_ObjectDestroy(drv->mouse);
		drv->mouse = NULL;
	}
	AG_MutexUnlock(&agDisplayLock);

	if (--nDrivers == 0)
		GLX_DestroyGlobals();
}

static int
GLX_GetDisplaySize(Uint *_Nonnull w, Uint *_Nonnull h)
{
	AG_MutexLock(&agDisplayLock);
#ifdef HAVE_XINERAMA
	{
		int event, error, nScreens;
		XineramaScreenInfo *xs;

		if (XineramaQueryExtension(agDisplay, &event, &error) &&
		    (xs = XineramaQueryScreens(agDisplay, &nScreens)) != NULL) {
			*w = (Uint)xs[0].width;
			*h = (Uint)xs[0].height;
			XFree(xs);
			goto out;
		}
	}
#endif /* HAVE_XINERAMA */
	*w = (Uint)DisplayWidth(agDisplay, agScreen);
	*h = (Uint)DisplayHeight(agDisplay, agScreen);
out:
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

/*
 * Get Agar keycode corresponding to an X KeyCode.
 * agDisplayLock must be held.
 */
static int
LookupKeyCode(KeyCode keycode, AG_KeySym *_Nonnull rv)
{
	KeySym ks;

#ifdef HAVE_XKB
	if ((ks = XkbKeycodeToKeysym(agDisplay, keycode, 0, 0)) == 0) {
		*rv = AG_KEY_NONE;
		return (0);
	}
#else
	if ((ks = XKeycodeToKeysym(agDisplay, keycode, 0)) == 0) {
		*rv = AG_KEY_NONE;
		return (0);
	}
#endif

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
static __inline__ AG_Window *_Nullable
LookupWindowByID(Window xw)
{
	AG_Window *win;
	AG_DriverGLX *glx;

	/* XXX TODO we can optimize based on numerical XIDs? */
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
	AG_SetErrorS("X event from unknown window");
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
#ifdef HAVE_XKB
			KeySym ks = XkbKeycodeToKeysym(agDisplay, kc, 0, 0);
#else
			KeySym ks = XKeycodeToKeysym(agDisplay, kc, 0);
#endif
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
UpdateKeyboard(AG_Keyboard *_Nonnull kbd)
{
	char kv[32];
	int i, j;
	Uint ms = 0;
	AG_KeySym key;
	Window dummy;
	int x, y;
	Uint mask;
	
	AG_MutexLock(&agDisplayLock);
	
	/* Get the keyboard modifier state. */
	InitModifierMasks();
	if (XQueryPointer(agDisplay, DefaultRootWindow(agDisplay),
	    &dummy, &dummy, &x, &y, &x, &y, &mask)) {
		if (mask & LockMask) { ms |= AG_KEYMOD_CAPSLOCK; }
		if (mask & modMasks.mode) { ms |= AG_KEYMOD_MODE; }
		if (mask & modMasks.num) { ms |= AG_KEYMOD_NUMLOCK; }
	}
	memset(kbd->keyState, 0, kbd->keyCount);
	XQueryKeymap(agDisplay, kv);
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
			default:				    break;
			}
		}
	}

	kbd->keyState[AG_KEY_CAPSLOCK] = (ms & AG_KEYMOD_CAPSLOCK) ?
	    AG_KEY_PRESSED : AG_KEY_RELEASED;
	kbd->keyState[AG_KEY_NUMLOCK] = (ms & AG_KEYMOD_NUMLOCK) ?
	    AG_KEY_PRESSED : AG_KEY_RELEASED;

	/* Set the final modifier state */
	kbd->modState = ms;
	
	AG_MutexUnlock(&agDisplayLock);
}

/* Refresh the internal keyboard state for all glx driver instances. */
static void
UpdateKeyboardAll(void)
{
	AG_Driver *drv;

	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (!AGDRIVER_IS_GLX(drv)) {
			continue;
		}
		UpdateKeyboard(drv->kbd);
	}
	AG_UnlockVFS(&agDrivers);
}

static __inline__ int
GLX_PendingEvents(void *_Nonnull drvCaller)
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

	/* Poll the X connection fd */
	fd = XConnectionNumber(agDisplay);
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
GLX_GetNextEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	XEvent xev;
	int x, y;
	AG_KeySym ks;
	AG_Window *win;
	AG_Char ch;

	AG_MutexLock(&agDisplayLock);
	XNextEvent(agDisplay, &xev);
	AG_MutexUnlock(&agDisplayLock);

	switch (xev.type) {
	case MotionNotify:
		if ((win = LookupWindowByID(xev.xmotion.window)) == NULL) {
			return (-1);
		}
#ifdef DEBUG_MOTION_XEVENTS
		Debug(win, "MotionNotify(%d,%d)\n", xev.xmotion.x, xev.xmotion.y);
#endif
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
#ifdef DEBUG_XEVENTS
		Debug(win, "ButtonPress(%d,%d, %d)\n", xev.xmotion.x, xev.xmotion.y, xev.xbutton.button);
#endif
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
#ifdef DEBUG_XEVENTS
		Debug(win, "ButtonRelease(%d,%d, %d)\n", xev.xmotion.x, xev.xmotion.y, xev.xbutton.button);
#endif
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
			ch = (Uint8)xkbBuf[0];	/* XXX */
		} else {
			ch = 0;
		}
		if (!LookupKeyCode(xev.xkey.keycode, &ks)) {
			AG_SetError(_("Keyboard event: Unknown keycode: %d"),
			    (int)xev.xkey.keycode);
			AG_MutexUnlock(&agDisplayLock);
			return (-1);
		}
		AG_MutexUnlock(&agDisplayLock);

		if ((win = LookupWindowByID(xev.xkey.window)) == NULL) {
			return (-1);
		}
#ifdef DEBUG_XEVENTS
		Debug(win, "Key%s(%u)\n", (xev.type == KeyPress) ? "Press" : "Release", (Uint)xev.xkey.keycode);
#endif
		AG_KeyboardUpdate(WIDGET(win)->drv->kbd,
		    (xev.type == KeyPress) ? AG_KEY_PRESSED : AG_KEY_RELEASED,
		    ks);

		dev->type = (xev.type == KeyPress) ? AG_DRIVER_KEY_DOWN :
		                                     AG_DRIVER_KEY_UP;
		dev->win = win;
		dev->data.key.ks = ks;
		dev->data.key.ucs = ch;
		break;
	case EnterNotify:
		if ((win = LookupWindowByID(xev.xcrossing.window)) == NULL) {
			return (-1);
		}
#ifdef DEBUG_XEVENTS
		Debug(win, "EnterNotify\n");
#endif
		dev->type = AG_DRIVER_MOUSE_ENTER;
		dev->win = win;
		break;
	case LeaveNotify:
		if ((win = LookupWindowByID(xev.xcrossing.window)) == NULL) {
			return (-1);
		}
#ifdef DEBUG_XEVENTS
		Debug(win, "LeaveNotify\n");
#endif
		dev->type = AG_DRIVER_MOUSE_LEAVE;
		dev->win = win;
		break;
	case FocusIn:
		if ((win = LookupWindowByID(xev.xfocus.window)) == NULL) {
			return (-1);
		}
#ifdef DEBUG_XEVENTS
		Debug(win, "FocusIn\n");
#endif
		dev->type = AG_DRIVER_FOCUS_IN;
		dev->win = win;
		break;
	case FocusOut:
		if ((win = LookupWindowByID(xev.xfocus.window)) == NULL) {
			return (-1);
		}
#ifdef DEBUG_XEVENTS
		Debug(win, "FocusOut\n");
#endif
		dev->type = AG_DRIVER_FOCUS_OUT;
		dev->win = win;
		break;
	case KeymapNotify:					/* Internal */
		UpdateKeyboardAll();
		return (0);
	case MappingNotify:					/* Internal */
#ifdef DEBUG_XEVENTS
		Debug(NULL, "MappingNotify\n");
#endif
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
#ifdef DEBUG_XEVENTS
			Debug(win, "ConfigureNotify(%d,%d, %d,%d)\n", x,y, xev.xconfigure.width, xev.xconfigure.height);
#endif
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
#ifdef DEBUG_XEVENTS
		Debug(win, "Expose\n");
#endif
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = win;
		break;
	case ClientMessage:
#ifdef AG_WM_HINTS
		if ((xev.xclient.format == 32) &&
		    (xev.xclient.data.l[0] == wmDeleteWindow) &&
		    (win = LookupWindowByID(xev.xclient.window))) {
# ifdef DEBUG_XEVENTS
			Debug(win, "ClientMessage(WM_DELETE_WINDOW)\n");
# endif
			dev->type = AG_DRIVER_CLOSE;
			dev->win = win;
			break;
		}
#endif /* AG_WM_HINTS */
		return (0);
	case MapNotify:
	case UnmapNotify:
	case DestroyNotify:
	case ReparentNotify:
		return (0);
	case SelectionRequest:
		if (agClipboardWindow != None &&
		    agClipboardWindow == xev.xany.window) {
			const XSelectionRequestEvent *req = &xev.xselectionrequest;
			XEvent sev;
			Uchar *selData;
			Ulong nBytes, overflow;
			int selFormat;
#ifdef DEBUG_CLIPBOARD
			Debug(NULL, "SelectionRequest (0x%lx -> 0x%lx)\n",
			    req->requestor, req->target);
#endif
			memset(&sev, 0, sizeof(XEvent));
			sev.xany.type = SelectionNotify;
			sev.xselection.selection = req->selection;
			sev.xselection.target = None;
			sev.xselection.property = None;
			sev.xselection.requestor = req->requestor;
			sev.xselection.time = req->time;

			if (XGetWindowProperty(agDisplay, DefaultRootWindow(agDisplay),
			    XInternAtom(agDisplay,"AGAR_CUTBUFFER",False),
			    0, INT_MAX/4, False, req->target,
			    &sev.xselection.target, &selFormat, &nBytes,
			    &overflow, &selData) == Success) {
				Atom xaTargets = XInternAtom(agDisplay, "TARGETS", 0);

				if (sev.xselection.target == req->target) {
					XChangeProperty(agDisplay,
					    req->requestor,
					    req->property, sev.xselection.target,
					    selFormat,
					    PropModeReplace, selData, nBytes);
				} else if (xaTargets == req->target) {
					Atom suppFormats[] = {
					    xaTargets,
					    sev.xselection.target
					};
					XChangeProperty(agDisplay,
					    req->requestor, req->property,
					    XA_ATOM, 32, PropModeReplace,
					    (Uchar *)suppFormats,
					    sizeof(suppFormats)/sizeof(suppFormats[0]));

					sev.xselection.property = req->property;
					sev.xselection.target = xaTargets;
				}
				XFree(selData);
			}
			XSendEvent(agDisplay, req->requestor, False, 0, &sev);
			XSync(agDisplay, False);
		}
		return (0);
	case SelectionNotify:
#ifdef DEBUG_CLIPBOARD
		Debug(NULL, "SelectionNotify\n");
#endif
		if (agClipboardWindow != None &&
		    agClipboardWindow == xev.xany.window) {
			agClipboardSelectionWaiting = 0;
		}
		return (0);
	case SelectionClear:
#ifdef DEBUG_CLIPBOARD
		Debug(NULL, "SelectionClear\n");
#endif
#if 0
		if (agClipboardWindow != None &&
		    agClipboardWindow == xev.xany.window) {
			/* TODO Generate an event? */
		}
#endif
		return (0);
	default:
		AG_SetError(_("Unknown X event %d"), xev.type);
		return (-1);
	}
	return (1);
}

static int
GLX_ProcessEvent(void *_Nullable drvCaller, AG_DriverEvent *_Nonnull dev)
{
	AG_Window *win;
	AG_Driver *drv;
	AG_SizeAlloc a;
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
		    drv->mouse->xRel, drv->mouse->yRel, drv->mouse->btnState);
		AG_MouseCursorUpdate(win,
		     dev->data.motion.x, dev->data.motion.y);
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
		if (agWindowFocused != win) {
			agWindowFocused = win;
			AG_PostEvent(win, "window-gainfocus", NULL);
		}
		break;
	case AG_DRIVER_FOCUS_OUT:
		if (agWindowFocused == win) {
			AG_PostEvent(win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		break;
	case AG_DRIVER_VIDEORESIZE:
		{
			AG_DriverGLX *glx = (AG_DriverGLX *)drv;

			AG_MutexLock(&glx->lock);

			a.x = dev->data.videoresize.x;
			a.y = dev->data.videoresize.y;
			a.w = dev->data.videoresize.w;
			a.h = dev->data.videoresize.h;

			if (a.w != WIDTH(win) || a.h != HEIGHT(win))
				GLX_PostResizeCallback(win, &a);

			if (a.x != WIDGET(win)->x || a.y != WIDGET(win)->y)
				GLX_PostMoveCallback(win, &a);

			AG_MutexUnlock(&glx->lock);
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
GLX_BeginRendering(void *_Nonnull obj)
{
	AG_DriverGLX *glx = obj;

	AG_MutexLock(&glx->lock);
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
}

static void
GLX_RenderWindow(AG_Window *_Nonnull win)
{
	const AG_Color *cBg = &WCOLOR(win, BG_COLOR);

	AG_PushClipRect(win, &WIDGET(win)->r);

	glClearColor((float)cBg->r / AG_COLOR_LASTF,
	             (float)cBg->g / AG_COLOR_LASTF,
		     (float)cBg->b / AG_COLOR_LASTF, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	AG_WidgetDraw(win);

	AG_PopClipRect(win);
}

static void
GLX_EndRendering(void *_Nonnull obj)
{
	AG_DriverGLX *glx = obj;
	AG_GL_Context *gl = &glx->gl;
	Uint i;
	
	/* Exchange front and back buffers. */
	glXSwapBuffers(agDisplay, glx->w);

	/* Remove textures and display lists queued for deletion. */
	glDeleteTextures(gl->nTextureGC, (const GLuint *)gl->textureGC);
	for (i = 0; i < gl->nListGC; i++) {
		glDeleteLists(gl->listGC[i], 1);
	}
	gl->nTextureGC = 0;
	gl->nListGC = 0;

	AG_MutexUnlock(&glx->lock);
}

/*
 * Clipping and blending control (rendering context)
 */

/*
 * Cursor operations
 */

static AG_Cursor *
GLX_CreateCursor(void *_Nonnull obj, Uint w, Uint h, const Uint8 *_Nonnull data,
    const Uint8 *_Nonnull mask, int xHot, int yHot)
{
	AG_DriverGLX *glx = obj;
	AG_Cursor *ac;
	AG_CursorGLX *acGLX;
	Uint size = w*h, dataSize;
	int i;
	char *xData, *xMask;
	XGCValues gcVals;
	GC gc;
	XImage *dataImg, *maskImg;
	Pixmap dataPixmap, maskPixmap;

	if ((acGLX = TryMalloc(sizeof(AG_CursorGLX))) == NULL) {
		return (NULL);
	}
	ac = (AG_Cursor *)acGLX;

	if ((ac->data = TryMalloc(size)) == NULL) {
		free(ac);
		return (NULL);
	}
	if ((ac->mask = TryMalloc(size)) == NULL) {
		free(ac->data);
		free(ac);
		return (NULL);
	}
	memcpy(ac->data, data, size);
	memcpy(ac->mask, mask, size);
	ac->w = w;
	ac->h = h;
	ac->xHot = xHot;
	ac->yHot = yHot;

	/*
	 * Initialize X11-specific part.
	 */
	memset(&acGLX->black, 0, sizeof(acGLX->black));
	acGLX->white.pixel = 0xffff;
	acGLX->white.red = 0xffff;
	acGLX->white.green = 0xffff;
	acGLX->white.blue = 0xffff;

	dataSize = (w >> 3) * h;
	if ((xData = TryMalloc(dataSize)) == NULL) {
		goto fail;
	}
	if ((xMask = TryMalloc(dataSize)) == NULL) {
		free(xData);
		goto fail;
	}
	for (i = 0; i < dataSize; i++) {
		xMask[i] = ac->data[i] | ac->mask[i];
		xData[i] = ac->data[i];
	}
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	/* Create the data image */
	dataImg = XCreateImage(agDisplay, DefaultVisual(agDisplay,agScreen),
	    1, XYBitmap, 0, xData, w, h, 8, (w >> 3));
	dataImg->byte_order = MSBFirst;
	dataImg->bitmap_bit_order = MSBFirst;
	dataPixmap = XCreatePixmap(agDisplay, RootWindow(agDisplay,agScreen),
	    w, h, 1);

	/* Create the mask image */
	maskImg = XCreateImage(agDisplay, DefaultVisual(agDisplay,agScreen),
	    1, XYBitmap, 0, xMask, w, h, 8, (w >> 3));
	maskImg->byte_order = MSBFirst;
	maskImg->bitmap_bit_order = MSBFirst;
	maskPixmap = XCreatePixmap(agDisplay, RootWindow(agDisplay,agScreen),
	    w, h, 1);

	/* Blit the data and mask to the pixmaps */
	gcVals.function = GXcopy;
	gcVals.foreground = ~0;
	gcVals.background =  0;
	gcVals.plane_mask = AllPlanes;
	gc = XCreateGC(agDisplay, dataPixmap,
	    GCFunction | GCForeground | GCBackground | GCPlaneMask,
	    &gcVals);
	XPutImage(agDisplay, dataPixmap, gc, dataImg, 0,0, 0,0, w,h);
	XPutImage(agDisplay, maskPixmap, gc, maskImg, 0,0, 0,0, w,h);
	XFreeGC(agDisplay, gc);
	XDestroyImage(dataImg);
	XDestroyImage(maskImg);

	/* Create the X cursor */
	acGLX->xc = XCreatePixmapCursor(agDisplay, dataPixmap, maskPixmap,
	    &acGLX->black, &acGLX->white, ac->xHot, ac->yHot);
	acGLX->visible = 0;
	acGLX->shared = 0;

	XFreePixmap(agDisplay, dataPixmap);
	XFreePixmap(agDisplay, maskPixmap);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (ac);
fail:
	free(ac->data);
	free(ac->mask);
	free(ac);
	return (NULL);
}

static void
GLX_FreeCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	AG_CursorGLX *acGLX = (AG_CursorGLX *)ac;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);
	
	if (ac == drv->activeCursor)
		drv->activeCursor = NULL;

	if (!acGLX->shared)
		XFreeCursor(agDisplay, acGLX->xc);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	
	free(ac->data);
	free(ac->mask);
	free(ac);
}

static int
GLX_SetCursor(void *_Nonnull obj, AG_Cursor *_Nonnull ac)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	AG_CursorGLX *acGLX = (AG_CursorGLX *)ac;

	if (drv->activeCursor == ac)
		return (0);

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	if (ac == TAILQ_FIRST(&drv->cursors)) {
		XUndefineCursor(agDisplay, glx->w);
	} else {
		XDefineCursor(agDisplay, glx->w, acGLX->xc);
	}
	XSync(agDisplay, False);

	drv->activeCursor = ac;
	acGLX->visible = 1;

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static void
GLX_UnsetCursor(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	AG_DriverGLX *glx = obj;
	
	if (drv->activeCursor == TAILQ_FIRST(&drv->cursors))
		return;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XUndefineCursor(agDisplay, glx->w);
	XSync(agDisplay, False);
	drv->activeCursor = TAILQ_FIRST(&drv->cursors);
	
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
}

static int
GLX_GetCursorVisibility(void *_Nonnull obj)
{
	/* XXX TODO */
	return (1);
}

static void
GLX_SetCursorVisibility(void *_Nonnull obj, int flag)
{
	/* XXX TODO */
}

static int
CreateClipboardWindow(void)
{
	XSetWindowAttributes xwAttrs;

	agClipboardWindow = XCreateWindow(agDisplay,
	    RootWindow(agDisplay, DefaultScreen(agDisplay)),
	    -10, -10, 1, 1, 0,
	    CopyFromParent, InputOnly, CopyFromParent, 0, &xwAttrs);
	if (agClipboardWindow == 0) {
		AG_SetErrorS("Clipboard window could not be created");
		return (-1);
	}
	XFlush(agDisplay);
#ifdef DEBUG_CLIPBOARD
	Debug(NULL, "Created clipboard window\n");
#endif
	return (0);
}

static char *_Nullable
GLX_GetClipboardText(void *_Nonnull obj)
{
	AG_DriverGLX *glx = obj;
	char *text = NULL;
	Window owner;
	Atom xaClipboard, sFormat, selection, selType;
	Ulong nBytes, overflow;
	Uchar *src;
	int selFormat;

#ifdef DEBUG_CLIPBOARD
	Debug(glx, "GetClipboardText()\n");
#endif
	AG_MutexLock(&agDisplayLock);

	if (agClipboardWindow == None &&
	    CreateClipboardWindow() == -1) {
		goto out;
	}
	if ((xaClipboard = XInternAtom(agDisplay, "CLIPBOARD", 0)) == None) {
		text = NULL;
		goto out;
	}
#ifdef X_HAVE_UTF8_STRING
	sFormat = XInternAtom(agDisplay,"UTF8_STRING",False);
#else
	sFormat = XA_STRING;
#endif
	owner = XGetSelectionOwner(agDisplay, xaClipboard);
	if (owner == None) {                           /* Legacy (no UTF8) */
		owner = DefaultRootWindow(agDisplay);
		selection = XA_CUT_BUFFER0;
		sFormat = XA_STRING;
	} else if (owner == agClipboardWindow) {
		owner = DefaultRootWindow(agDisplay);
		selection = XInternAtom(agDisplay, "AGAR_CUTBUFFER", False);
	} else {
		Uint32 waitStart, waitElapsed;

		owner = agClipboardWindow;
		selection = XInternAtom(agDisplay, "AGAR_SELECTION", False);
		XConvertSelection(agDisplay, xaClipboard, sFormat, selection,
		    owner, CurrentTime);

		/*
		 * When using synergy on Linux and when data has been put in
		 * the clipboard on the remote (Windows anyway) machine then
		 * agClipboardSelectionWaiting may never be set to 0.
		 * Time out after a while.
		 */
		waitStart = AG_GetTicks();
		agClipboardSelectionWaiting = 1;
		while (agClipboardSelectionWaiting) {
			AG_DriverEvent dev;

			/* Pump events */
			while (XPending(agDisplay)) {
				if (GLX_GetNextEvent(NULL, &dev) == 1)
					GLX_ProcessEvent(NULL, &dev);
			}

			waitElapsed = AG_GetTicks() - waitStart;
			/*
			 * Wait one second for a clipboard response.
			 */
			if (waitElapsed > 1000) {
				agClipboardSelectionWaiting = 0;
				/*
				 * We need to set the clipboard text so that
				 * next time we won't timeout, otherwise we
				 * will hang on every call to this function.
				 */
				GLX_SetClipboardText(glx, "");
				AG_SetError(_("Clipboard timeout"));
				return Strdup("");
			}
		}
	}

	if (XGetWindowProperty(agDisplay, owner, selection, 0, INT_MAX/4, False,
	    sFormat, &selType, &selFormat, &nBytes, &overflow, &src) == Success) {
#ifdef DEBUG_CLIPBOARD
		Debug(NULL, "XGetWindowProperty: selType=%lx (sFormat=%lx), nBytes=%ld\n",
		    selType, sFormat, (long)nBytes);
#endif
		if (selType == sFormat) {
			if ((text = TryMalloc(nBytes+1)) != NULL) {
				memcpy(text, src, nBytes);
				text[nBytes] = '\0';
			}
		}
		XFree(src);
	}
out:
	AG_MutexUnlock(&agDisplayLock);
	return (text != NULL) ? text : Strdup("");
}

static int
GLX_SetClipboardText(void *_Nonnull obj, const char *_Nonnull text)
{
	Atom xaClipboard;

	AG_MutexLock(&agDisplayLock);

	if (agClipboardWindow == None &&
	    CreateClipboardWindow() == -1)
		goto fail;

#ifdef DEBUG_CLIPBOARD
	Debug(obj, "Change AGAR_CUTBUFFER -> \"%s\" (len %ld)\n", text,
	    (long)strlen(text));
#endif
	/* Save the selection on the root window */
	XChangeProperty(agDisplay, DefaultRootWindow(agDisplay),
	    XInternAtom(agDisplay,"AGAR_CUTBUFFER",False),
#ifdef X_HAVE_UTF8_STRING
	    XInternAtom(agDisplay,"UTF8_STRING",False),
#else
	    XA_STRING,
#endif
	    8, PropModeReplace, (const Uchar *)text, strlen(text));

	xaClipboard = XInternAtom(agDisplay, "CLIPBOARD", 0);
	if (xaClipboard != None &&
	    (XGetSelectionOwner(agDisplay, xaClipboard) != agClipboardWindow))
		XSetSelectionOwner(agDisplay, xaClipboard,
		    agClipboardWindow, CurrentTime);

	if (XGetSelectionOwner(agDisplay, XA_PRIMARY) != agClipboardWindow)
		XSetSelectionOwner(agDisplay, XA_PRIMARY,
		    agClipboardWindow, CurrentTime);

	AG_MutexUnlock(&agDisplayLock);
	return (0);
fail:
	AG_MutexUnlock(&agDisplayLock);
	return (-1);
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

static Uint32
InitExposeTimeout(AG_Timer *_Nonnull timer, AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	win->dirty = 1;
	return (0);
}

static int
GLX_OpenWindow(AG_Window *_Nonnull win, const AG_Rect *_Nonnull r, int depthReq,
    Uint mwFlags)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	int glxAttrs[16];
	XSetWindowAttributes xwAttrs;
	XVisualInfo *xvi;
	XSizeHints *h;
	int i=0, depth;
	
	glxAttrs[i++] = GLX_RGBA;
#if AG_MODEL == AG_LARGE
	glxAttrs[i++] = GLX_RED_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_GREEN_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_BLUE_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_DEPTH_SIZE;	glxAttrs[i++] = 16;
#else
	glxAttrs[i++] = GLX_RED_SIZE;	glxAttrs[i++] = 1;
	glxAttrs[i++] = GLX_GREEN_SIZE;	glxAttrs[i++] = 1;
	glxAttrs[i++] = GLX_BLUE_SIZE;	glxAttrs[i++] = 1;
	glxAttrs[i++] = GLX_DEPTH_SIZE;	glxAttrs[i++] = 8;
#endif
	glxAttrs[i++] = GLX_DOUBLEBUFFER;
	if (agStereo) { glxAttrs[i++] = GLX_STEREO; }
	glxAttrs[i++] = None;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	if ((xvi = glXChooseVisual(agDisplay, agScreen, glxAttrs)) == NULL) {
#if AG_MODEL == AG_LARGE
		glxAttrs[2] = 1;
		glxAttrs[4] = 1;
		glxAttrs[6] = 1;
		glxAttrs[8] = 8;
#else
		glxAttrs[i-=2] = None;
#endif
		if ((xvi = glXChooseVisual(agDisplay, agScreen, glxAttrs))
		    == NULL) {
			AG_SetErrorS(_("Cannot find an acceptable GLX visual"));
			goto fail_unlock;
		}
	}

	xwAttrs.colormap = XCreateColormap(agDisplay,
	    RootWindow(agDisplay,xvi->screen),
	    xvi->visual, AllocNone);
	xwAttrs.background_pixmap = None;
	xwAttrs.border_pixel = (xvi->visual == DefaultVisual(agDisplay,agScreen)) ?
	                       BlackPixel(agDisplay,agScreen) : 0;

	xwAttrs.event_mask = KeyPressMask | KeyReleaseMask |
	                     ButtonPressMask | ButtonReleaseMask |
			     EnterWindowMask | LeaveWindowMask |
			     PointerMotionMask | ButtonMotionMask |
			     KeymapStateMask | StructureNotifyMask |
			     FocusChangeMask | ExposureMask;

	/* Create a new window. */
	depth = (depthReq >= 1) ? depthReq : xvi->depth;
	Debug(glx, "Creating %ux%u %d-bpp window at %d,%d\n",
	    r->w, r->h, depth,
	    r->x, r->y);
	glx->w = XCreateWindow(agDisplay,
	    RootWindow(agDisplay,agScreen),
	    r->x, r->y,
	    r->w, r->h, 0, depth,
	    InputOutput,
	    xvi->visual,
	    CWColormap | CWBackPixmap | CWBorderPixel | CWEventMask,
	    &xwAttrs);
	if (glx->w == 0) {
		AG_SetErrorS("XCreateWindow failed");
		goto fail_unlock;
	}

	/* Honor AG_WindowMakeTransient() */
	if (win->transientFor != NULL)
		GLX_SetTransientFor(win, win->transientFor);
	
	/* Set WM_NORMAL_HINTS. The other WM hints are set in MapWindow(). */
	if ((h = XAllocSizeHints()) != NULL)  {
		h->flags = 0;
		if (!(mwFlags & AG_DRIVER_MW_ANYPOS))
			h->flags |= PPosition;
	
		h->flags |= PMinSize | PMaxSize;
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

	/* Create the GLX rendering context. */
	glx->glxCtx = glXCreateContext(agDisplay, xvi, 0, GL_TRUE);
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	AG_GL_InitContext(glx, &glx->gl);
	
	/* Set the pixel formats. */
	if ((drv->videoFmt = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		goto fail_ctx;
	}
#if AG_MODEL == AG_LARGE
	if (depth == 48) {				/* Deep color */
# if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, depth,
			0xffff000000000000,
			0x0000ffff00000000,
			0x00000000ffff0000);
# else
		AG_PixelFormatRGB(drv->videoFmt, depth,
			0x000000000000ffff,
			0x00000000ffff0000,
			0x0000ffff00000000);
# endif
	} else
#endif /* AG_LARGE */
	{						/* True Color */
#if AG_BYTEORDER == AG_BIG_ENDIAN
		AG_PixelFormatRGB(drv->videoFmt, depth,
			0xff000000,
			0x00ff0000,
			0x0000ff00);
#else
		AG_PixelFormatRGB(drv->videoFmt, depth,
			0x000000ff,
			0x0000ff00,
			0x00ff0000);
#endif
	}

	/* Create the built-in cursors. */
	{
		AG_CursorGLX *acGLX = Malloc(sizeof(AG_CursorGLX));
		AG_Cursor *ac = AGCURSOR(acGLX);

		AG_CursorInit(ac);
		acGLX->xc = XCreateFontCursor(agDisplay, XC_left_ptr);
		acGLX->visible = 1;
		acGLX->shared = 1;

		TAILQ_INSERT_HEAD(&drv->cursors, ac, cursors);
		drv->nCursors++;
	}

	AG_InitStockCursors(drv);

	AG_InitTimer(&glx->toInitExpose, "init", 0);
	AG_AddTimer(glx, &glx->toInitExpose, 1, InitExposeTimeout, "%p", win);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	
	XFree(xvi);
	return (0);
fail_ctx:
	AG_GL_DestroyContext(glx);
	glXMakeCurrent(agDisplay, None, NULL);
	glXDestroyContext(agDisplay, glx->glxCtx);
	XDestroyWindow(agDisplay, glx->w);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
fail_unlock:
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	if (xvi)
		XFree(xvi);
	return (-1);
}

static int
GLX_MapWindow(AG_Window *_Nonnull win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	AG_Rect rVP;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

#ifdef AG_WM_HINTS
	if (!glx->wmHintsSet)
		SetHints(win);                  /* Set window manager hints */
#endif
	XMapWindow(agDisplay, glx->w);
	XIfEvent(agDisplay, &xev, WaitMapNotify, (char *)glx->w);

	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);  /* Set GL viewport */
	rVP.x = 0;
	rVP.y = 0;
	rVP.w = WIDTH(win);
	rVP.h = HEIGHT(win);
	AG_GL_SetViewport(&glx->gl, &rVP);

	if (win->flags & AG_WINDOW_MODAL) {
		glx->ptrIsGrabbed = (XGrabPointer(agDisplay, glx->w, True,
		    PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, /* pointer_mode */
		    GrabModeAsync, /* keyboard_mode */
		    None,          /* confine_to */
		    None,          /* cursor */
		    CurrentTime) == 0 ? 1 : 0);
	}

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

#ifdef AG_WM_HINTS
static void
SetHints(AG_Window *_Nonnull win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	Atom wmprot[2], atom;
	int nwmprot;
		
	glx->wmHintsSet = 1;
	
	/* Set EWMH-compliant window type */
	if (wmNetWmWindowType != None) {
		atom = XInternAtom(agDisplay, agWindowWmTypeNames[win->wmType],
		                   True);
		if (atom != None) {
			XChangeProperty(agDisplay, glx->w, wmNetWmWindowType,
			    XA_ATOM, 32, PropModeReplace, (Uchar *)&atom, 1);
		}
		if (win->wmType == AG_WINDOW_WM_DROPDOWN_MENU ||
		    win->wmType == AG_WINDOW_WM_POPUP_MENU) {
			atom = XInternAtom(agDisplay, "_NET_WM_WINDOW_TYPE_MENU",
			                   True);
			if (atom != None)
				XChangeProperty(agDisplay, glx->w,
				    wmNetWmWindowType, XA_ATOM, 32,
				    PropModeAppend, (Uchar *)&atom, 1);
		}
		if (win->wmType != AG_WINDOW_WM_NORMAL) {
			atom = XInternAtom(agDisplay, "_NET_WM_WINDOW_TYPE_DOCK",
			                   True);
			if (atom != None)
				XChangeProperty(agDisplay, glx->w,
				    wmNetWmWindowType, XA_ATOM, 32,
				    PropModeAppend, (Uchar *)&atom, 1);
		}
	}
	if (win->wmType != AG_WINDOW_WM_NORMAL &&
	    wmNetWmState != None &&
	    wmNetWmStateSkipTaskbar != None)
		XChangeProperty(agDisplay, glx->w, wmNetWmState, XA_ATOM, 32,
		    PropModeAppend, (Uchar *)&wmNetWmStateSkipTaskbar, 1);
	
	/* Set window manager hints. */
	if (wmMotifWmHints != None)
		SetMotifWmHints(win);
	if (wmKwmWinDecoration != None)
		SetKwmWmHints(win);
	if (wmWinHints != None)
		SetGnomeWmHints(win);

	/* Set application-modal window hints. */
	if (win->flags & AG_WINDOW_MODAL) {
		if (wmNetWmState != None && wmNetWmStateModal != None)
			XChangeProperty(agDisplay, glx->w,
			    wmNetWmState, XA_ATOM, 32,
			    PropModeAppend,
			    (Uchar *)&wmNetWmStateModal, 1);
	}
	
	/* Honor KEEPABOVE and KEEPBELOW */
	if (win->flags & AG_WINDOW_KEEPABOVE) {
		if (wmNetWmState != None && wmNetWmStateAbove != None) {
			XChangeProperty(agDisplay, glx->w,
			    wmNetWmState, XA_ATOM, 32,
			    PropModeAppend, (Uchar *)&wmNetWmStateAbove, 1);
		}
	} else if (win->flags & AG_WINDOW_KEEPBELOW) {
		if (wmNetWmState != None && wmNetWmStateBelow != None) {
			XChangeProperty(agDisplay, glx->w,
			    wmNetWmState, XA_ATOM, 32,
			    PropModeAppend, (Uchar *)&wmNetWmStateBelow, 1);
		}
	}

	/* Honor NOCLOSE and DENYFOCUS */
	nwmprot = 0;
	if (!(win->flags & AG_WINDOW_NOCLOSE)) {
		wmprot[nwmprot++] = wmDeleteWindow;
	}
	if (win->flags & AG_WINDOW_DENYFOCUS) {
		XWMHints h;
		h.flags = InputHint;
		h.input = False;
		XSetWMHints(agDisplay, glx->w, &h);
		wmprot[nwmprot++] = wmTakeFocus;
	}
	XChangeProperty(agDisplay, glx->w, wmProtocols, XA_ATOM, 32,
	    PropModeReplace,
	    (Uchar *)wmprot, nwmprot);
}

/*
 * Set the hints for Motif-compliant window managers.
 */
struct ag_motif_wm_hints {
	unsigned long flags;
	unsigned long fns;
	unsigned long dec;
	long input_mode;
	unsigned long status;
};
#define AG_MWM_HINTS_FUNCTIONS   (1L << 0)
#define AG_MWM_HINTS_DECORATIONS (1L << 1)
#define AG_MWM_HINTS_INPUT_MODE  (1L << 2)
#define AG_MWM_FN_ALL       (1L << 0)
#define AG_MWM_FN_RESIZE    (1L << 1)
#define AG_MWM_FN_MOVE      (1L << 2)
#define AG_MWM_FN_MINIMIZE  (1L << 3)
#define AG_MWM_FN_MAXIMIZE  (1L << 4)
#define AG_MWM_FN_CLOSE     (1L << 5)
#define AG_MWM_DEC_ALL      (1L << 0)
#define AG_MWM_DEC_BORDER   (1L << 1)
#define AG_MWM_DEC_RESIZEH  (1L << 2)
#define AG_MWM_DEC_TITLE    (1L << 3)
#define AG_MWM_DEC_MENU     (1L << 4)
#define AG_MWM_DEC_MINIMIZE (1L << 5)
#define AG_MWM_DEC_MAXIMIZE (1L << 6)
#define AG_MWM_INPUT_MODELESS                  0L	/* Document modal */
#define AG_MWM_INPUT_PRIMARY_APPLICATION_MODAL 1L	/* Document modal */
#define AG_MWM_INPUT_FULL_APPLICATION_MODAL    3L	/* Application modal */
static void
SetMotifWmHints(AG_Window *_Nonnull win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	struct ag_motif_wm_hints hNew, *hints = NULL;
	Atom type = None;
	Ulong nItems, bytesAfter;
	Uchar *data;
	int format, rv;

	hNew.flags = AG_MWM_HINTS_FUNCTIONS | AG_MWM_HINTS_DECORATIONS |
	             AG_MWM_HINTS_INPUT_MODE;
	hNew.fns = 0UL;
	hNew.dec = 0UL;
	hNew.input_mode = (win->flags & AG_WINDOW_MODAL) ?
	                  AG_MWM_INPUT_FULL_APPLICATION_MODAL :
			  AG_MWM_INPUT_MODELESS;
	hNew.status = 0UL;

	if (!(win->flags & AG_WINDOW_NOMINIMIZE)) {
		hNew.fns |= AG_MWM_FN_MINIMIZE;
		hNew.dec |= AG_MWM_DEC_MINIMIZE;
	}
	if (!(win->flags & AG_WINDOW_NOMAXIMIZE)) {
		hNew.fns |= AG_MWM_FN_MAXIMIZE;
		hNew.dec |= AG_MWM_DEC_MAXIMIZE;
	}
	if (!(win->flags & AG_WINDOW_NORESIZE))  hNew.fns |= AG_MWM_FN_RESIZE;
	if (!(win->flags & AG_WINDOW_NOMOVE))    hNew.fns |= AG_MWM_FN_MOVE;
	if (!(win->flags & AG_WINDOW_NOCLOSE))   hNew.fns |= AG_MWM_FN_CLOSE;
	if (!(win->flags & AG_WINDOW_NOBORDERS)) hNew.dec |= AG_MWM_DEC_BORDER;
	if (!(win->flags & AG_WINDOW_NOHRESIZE)) hNew.dec |= AG_MWM_DEC_RESIZEH;
	if (!(win->flags & AG_WINDOW_NOTITLE))   hNew.dec |= AG_MWM_DEC_TITLE;

	rv = XGetWindowProperty(agDisplay, glx->w, wmMotifWmHints, 0,
	    sizeof(struct ag_motif_wm_hints)/sizeof(long), False,
	    AnyPropertyType, &type, &format, &nItems, &bytesAfter, &data);
	if (rv != Success)
		return;

	if (data == NULL || type != wmMotifWmHints) {
		hints = &hNew;
	} else {
		hints = (struct ag_motif_wm_hints *)data;
		hints->flags = hNew.flags;
		hints->fns = hNew.fns;
		hints->dec = hNew.dec;
		hints->input_mode = hNew.input_mode;
	}
	XChangeProperty(agDisplay, glx->w,
	    wmMotifWmHints, wmMotifWmHints, 32,
	    PropModeReplace, (Uchar *)hints,
	    sizeof(struct ag_motif_wm_hints)/sizeof(long));
}

/* Set hints for KWM */
static void
SetKwmWmHints(AG_Window *_Nonnull win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;

	if (win->flags & AG_WINDOW_NOTITLE ||
	    win->flags & AG_WINDOW_NOBORDERS) {
		long KWMHints = 0;

		XChangeProperty(agDisplay, glx->w,
		    wmKwmWinDecoration, wmKwmWinDecoration, 32,
		    PropModeReplace, (Uchar *)&KWMHints,
		    sizeof(KWMHints)/sizeof(long));
	}
}

/* Set hints for GNOME */
static void
SetGnomeWmHints(AG_Window *_Nonnull win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;

	if (win->flags & AG_WINDOW_NOTITLE ||
	    win->flags & AG_WINDOW_NOBORDERS) {
		long GNOMEHints = 0;

		XChangeProperty(agDisplay, glx->w,
		    wmWinHints, wmWinHints, 32,
		    PropModeReplace, (Uchar *)&GNOMEHints,
		    sizeof(GNOMEHints)/sizeof(long));
	}
}
#endif /* AG_WM_HINTS */

static void
GLX_CloseWindow(AG_Window *_Nonnull win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverGLX *glx = (AG_DriverGLX *)drv;

	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);
	
	/* Release allocated cursors. */
	AG_FreeCursors(drv);

	/* Destroy our OpenGL context. */
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	AG_GL_DestroyContext(glx);
	glXMakeCurrent(agDisplay, None, NULL);
	glXDestroyContext(agDisplay, glx->glxCtx);

	/* Destroy the window. */
	XDestroyWindow(agDisplay, glx->w);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
}

static int
GLX_UnmapWindow(AG_Window *_Nonnull win)
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
GLX_RaiseWindow(AG_Window *_Nonnull win)
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
GLX_LowerWindow(AG_Window *_Nonnull win)
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
GLX_ReparentWindow(AG_Window *_Nonnull win, AG_Window *_Nonnull winParent,
    int x, int y)
{
	AG_DriverGLX *glxWin = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_DriverGLX *glxParentWin = (AG_DriverGLX *)WIDGET(winParent)->drv;
/*	XEvent xev; */
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glxWin->lock);
	AG_MutexLock(&glxParentWin->lock);

	XReparentWindow(agDisplay, glxWin->w, glxParentWin->w, x,y);
	/* XXX wait? */

	AG_MutexUnlock(&glxParentWin->lock);
	AG_MutexUnlock(&glxWin->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
GLX_GetInputFocus(AG_Window *_Nonnull *_Nonnull rv)
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
		if (glx->w && glx->w == wRet)
			break;
	}
	if (glx == NULL) {
		AG_SetErrorS(_("Input focus is external to this application"));
		return (-1);
	}
	*rv = AGDRIVER_MW(glx)->win;
	return (0);
}

static int
GLX_SetInputFocus(AG_Window *_Nonnull win)
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
GLX_PreResizeCallback(AG_Window *_Nonnull win)
{
#if 0
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;

	/*
	 * Backup GL resources since it is not portable to assume that a
	 * display resize will not cause a change in GL contexts
	 */
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	GLX_FreeWidgetResources(WIDGET(win));
	AG_TextClearGlyphCache(glx);
#endif
}

static void
GLX_PostResizeCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverGLX *glx = (AG_DriverGLX *)drv;
	AG_SizeAlloc wa;
	AG_Rect rVP;

	wa.x = 0;
	wa.y = 0;
	wa.w = a->w;
	wa.h = a->h;
	AG_WidgetSizeAlloc(win, &wa);
	AG_WidgetUpdateCoords(win, 0,0);
	WIDGET(win)->x = a->x;
	WIDGET(win)->y = a->y;

	win->dirty = 1;

	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	rVP.x = 0;
	rVP.y = 0;
	rVP.w = WIDTH(win);
	rVP.h = HEIGHT(win);
	AG_GL_SetViewport(&glx->gl, &rVP);
}

static void
GLX_PostMoveCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
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

static int
GLX_MoveWindow(AG_Window *_Nonnull win, int dx, int dy)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XMoveWindow(agDisplay, glx->w, dx,dy);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

#if 0
static void
GLX_FreeWidgetResources(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		GLX_FreeWidgetResources(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}

static void
RegenWidgetResources(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		RegenWidgetResources(chld);
	}
	AG_WidgetRegenResourcesGL(wid);
}
#endif

static int
GLX_ResizeWindow(AG_Window *_Nonnull win, Uint w, Uint h)
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
GLX_MoveResizeWindow(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	XMoveResizeWindow(agDisplay, glx->w,
	    a->x, a->y,
	    a->w, a->h);

	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
	return (0);
}

static int
GLX_SetBorderWidth(AG_Window *_Nonnull win, Uint width)
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
GLX_SetWindowCaption(AG_Window *_Nonnull win, const char *_Nonnull s)
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
		AG_SetErrorS(_("Cannot convert string to X property"));
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
GLX_SetTransientFor(AG_Window *_Nonnull win, AG_Window *_Nullable forParent)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_DriverGLX *glxParent;
	
	AG_MutexLock(&agDisplayLock);
	AG_MutexLock(&glx->lock);

	if (glx->w != 0) {
		if (forParent != NULL &&
		    (glxParent = (AG_DriverGLX *)WIDGET(forParent)->drv) != NULL &&
		    AGDRIVER_IS_GLX(glxParent) &&
		    (AGDRIVER_MW(glxParent)->flags & AG_DRIVER_MW_OPEN)) {
			XSetTransientForHint(agDisplay, glx->w, glxParent->w);
		} else {
			XSetTransientForHint(agDisplay, glx->w, None);
		}
	}
	AG_MutexUnlock(&glx->lock);
	AG_MutexUnlock(&agDisplayLock);
}

static int
GLX_SetOpacity(AG_Window *_Nonnull win, float f)
{
#ifdef AG_WM_HINTS
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	Ulong opacity = (f > 0.99) ? (Ulong) 0xffffffff : (Ulong)(f*0xffffffff);

	if (wmNetWmWindowOpacity != None) {
		XChangeProperty(agDisplay, glx->w,
		    wmNetWmWindowOpacity, XA_CARDINAL, 32,
		    PropModeReplace,
		    (Uchar *)&opacity, 1);
		return (0);
	} else {
		AG_SetErrorS("No opacity");
		return (-1);
	}
#else
	AG_SetErrorS("No opacity (no WM hints)");
	return (-1;
#endif
}

static void
GLX_TweakAlignment(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a,
    Uint wMax, Uint hMax)
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
	default:
		break;
	}
}

/*
 * Standard AG_EventLoop() event sink.
 */
static int
GLX_EventSink(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	Display *dpy = AG_PTR(1);
	AG_DriverEvent dev;

	while (XPending(dpy)) {
		if (GLX_GetNextEvent(NULL, &dev) == 1)
			GLX_ProcessEvent(NULL, &dev);
	}
	return (1);
}
static int
GLX_EventEpilogue(AG_EventSink *_Nonnull es, AG_Event *_Nonnull event)
{
	Display *dpy = AG_PTR(1);
	AG_DriverEvent dev;

	/* Work around an apparent buffering problem in X/Xlib/XCB. */
	while (XPending(dpy)) {
		if (GLX_GetNextEvent(NULL, &dev) == 1)
			GLX_ProcessEvent(NULL, &dev);
	}

	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	
	/* Flush the output buffer before the next poll. */
	XFlush(dpy);

	return (0);
}

static int
GLX_InitGlobals(void)
{
	int err, ev, xfd;

	if (nDrivers > 0)
		return (0);

#ifdef AG_THREADS
	XInitThreads();
#endif
	if ((agDisplay = XOpenDisplay(NULL)) == NULL) {
		AG_SetErrorS(_("Cannot open X display"));
		return (-1);
	}
	xfd = XConnectionNumber(agDisplay);
	if (!glXQueryExtension(agDisplay, &err, &ev)) {
		AG_SetErrorS(_("GLX extension is not available"));
		goto fail;
	}

#if defined(HAVE_XKB)
	{
		int xkbmajor = XkbMajorVersion;
		int xkbminor = XkbMinorVersion;
		int xkbopcode, xkbevent, xkberror;

		if (XkbQueryExtension(agDisplay, &xkbopcode, &xkbevent,
		                      &xkberror, &xkbmajor, &xkbminor)) {
			XkbDescPtr xkb;

			if ((xkb = XkbAllocKeyboard()) == NULL) {
				goto fail;
			}
			XkbGetControls(agDisplay, XkbRepeatKeysMask, xkb);
			agKbdDelay = xkb->ctrls->repeat_delay;
			agKbdRepeat = xkb->ctrls->repeat_interval;
			XkbFreeKeyboard(xkb, 0, True);
		}
	}
#elif defined(HAVE_XF86MISC)
	{
		XF86MiscKbdSettings kbdinfo;
		int dummy;

		if (XF86MiscQueryExtension(agDisplay, &dummy, &dummy) &&
		    XF86MiscGetKbdSettings(agDisplay, &kbdinfo)) {
			agKbdDelay = kbdinfo.delay;
			agKbdRepeat = kbdinfo.rate;
		}
	}
#endif /* HAVE_XF86MISC */

	AG_MutexInitRecursive(&agDisplayLock);
	agScreen = DefaultScreen(agDisplay);
	InitKeymaps();
	memset(xkbBuf, '\0', sizeof(xkbBuf));
	memset(&xkbCompStatus, 0, sizeof(xkbCompStatus));
#ifdef AG_WM_HINTS
	wmProtocols = XInternAtom(agDisplay, "WM_PROTOCOLS", True);
	wmTakeFocus = XInternAtom(agDisplay, "WM_TAKE_FOCUS", True);
	wmDeleteWindow = XInternAtom(agDisplay, "WM_DELETE_WINDOW", True);
	wmMotifWmHints = XInternAtom(agDisplay, "_MOTIF_WM_HINTS", True);
	wmKwmWinDecoration = XInternAtom(agDisplay, "KWM_WIN_DECORATION", True);
	wmWinHints = XInternAtom(agDisplay, "_WIN_HINTS", True);
	wmNetWmState = XInternAtom(agDisplay, "_NET_WM_STATE", True);
	wmNetWmStateModal = XInternAtom(agDisplay, "_NET_WM_STATE_MODAL", True);
	wmNetWmStateSkipTaskbar = XInternAtom(agDisplay, "_NET_WM_STATE_SKIP_TASKBAR", True);
	wmNetWmStateAbove = XInternAtom(agDisplay, "_NET_WM_STATE_ABOVE", True);
	wmNetWmStateBelow = XInternAtom(agDisplay, "_NET_WM_STATE_BELOW", True);
	wmNetWmWindowType = XInternAtom(agDisplay, "_NET_WM_WINDOW_TYPE", True);
	wmNetWmWindowOpacity = XInternAtom(agDisplay, "_NET_WM_WINDOW_OPACITY", True);
#endif

	agClipboardWindow = None;

	/* Set up event filters for standard AG_EventLoop(). */
	if ((glxEventSink = AG_AddEventSink(AG_SINK_READ, xfd, 0, GLX_EventSink, "%p", agDisplay)) == NULL ||
	    (glxEventEpilogue = AG_AddEventEpilogue(GLX_EventEpilogue, "%p", agDisplay)) == NULL) {
		goto fail;
	}
	XAddConnectionWatch(agDisplay, GLX_ConnectionWatchProc, NULL);
	return (0);
fail:
	if (glxEventSink != NULL) { AG_DelEventSink(glxEventSink); glxEventSink = NULL; }
	if (glxEventEpilogue != NULL) { AG_DelEventEpilogue(glxEventEpilogue); glxEventEpilogue = NULL; }
	XCloseDisplay(agDisplay);
	agDisplay = NULL;
	return (-1);
}

AG_DriverMwClass agDriverGLX = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverGLX",
			sizeof(AG_DriverGLX),
			{ 1,6 },
			Init,
			NULL,		/* reset */
			Destroy,
			NULL,		/* load */
			NULL,		/* save */
			NULL,		/* edit */
		},
		"glx",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES,
		GLX_Open,
		GLX_Close,
		GLX_GetDisplaySize,
		NULL,			/* beginEventProcessing */
		GLX_PendingEvents,
		GLX_GetNextEvent,
		GLX_ProcessEvent,
		NULL,			/* genericEventLoop */
		NULL,			/* endEventProcessing */
		NULL,			/* terminate */
		GLX_BeginRendering,
		GLX_RenderWindow,
		GLX_EndRendering,
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
		GLX_CreateCursor,
		GLX_FreeCursor,
		GLX_SetCursor,
		GLX_UnsetCursor,
		GLX_GetCursorVisibility,
		GLX_SetCursorVisibility,
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
		GLX_GetClipboardText,
		GLX_SetClipboardText
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
	GLX_SetBorderWidth,
	GLX_SetWindowCaption,
	GLX_SetTransientFor,
	GLX_SetOpacity,
	GLX_TweakAlignment
};

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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#ifdef __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glx.h>
#else
# include <GL/gl.h>
# include <GL/glx.h>
#endif

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "window.h"
#include "perfmon.h"
#include "gui_math.h"
#include "text.h"
#include "cursors.h"

#define DEBUG_XSYNC

static int      nDrivers = 0;		/* Drivers open */
static Display *agDisplay = NULL;	/* X display (shared) */
static int      agScreen = 0;		/* Default screen (shared) */
static Uint     rNom = 16;		/* Nominal refresh rate (ms) */
static int      rCur = 0;		/* Effective refresh rate (ms) */

static char           xkbBuf[64];	/* For Unicode key translation */
static XComposeStatus xkbCompStatus;	/* For Unicode key translation */
static Atom           wmDeleteWindowAtom; /* WM_DELETE_WINDOW atom */

/* Saved blending state */
struct blending_state {
	GLboolean enabled;		/* GL_BLEND enable bit */
	GLint srcFactor;		/* GL_BLEND_SRC mode */
	GLint dstFactor;		/* GL_BLEND_DST mode */
	GLfloat texEnvMode;		/* GL_TEXTURE_ENV mode */
};

/* Driver instance data */
typedef struct ag_driver_glx {
	struct ag_driver_mw _inherit;
	Window         w;		/* X window */
	GLXContext     glxCtx;		/* GLX context */
	int            clipStates[4];	/* Clipping GL state */
	AG_ClipRect   *clipRects;	/* Clipping rectangles */
	Uint          nClipRects;
	Uint          *textureGC;	/* Textures queued for deletion */
	Uint          nTextureGC;
	struct blending_state bs[1];	/* Saved blending states */
	AG_Cursor     *cursorToSet;	/* Set cursor at end of event cycle */
	GLubyte        disabledStipple[128]; /* "Disabled" stipple pattern */
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

struct ag_cursor_glx {
	XColor black;
	XColor white;
	Cursor xc;
	int visible;
};

#define AGDRIVER_IS_GLX(drv) \
	AG_OfClass((drv),"AG_Driver:AG_DriverMw:AG_DriverGLX")

static void DrawRectFilled(void *, AG_Rect, AG_Color);
static void PostResizeCallback(AG_Window *, AG_SizeAlloc *);
static void FreeWidgetResources(AG_Widget *);
static void RegenWidgetResources(AG_Widget *);
static int  RaiseWindow(AG_Window *);
static int  SetInputFocus(AG_Window *);

static void
Init(void *obj)
{
	AG_DriverGLX *glx = obj;

	glx->clipRects = NULL;
	glx->nClipRects = 0;
	memset(glx->clipStates, 0, sizeof(glx->clipStates));
	glx->textureGC = NULL;
	glx->nTextureGC = 0;
	glx->cursorToSet = NULL;
	memset(glx->disabledStipple, 0xaa, sizeof(glx->disabledStipple));
}

static void
Destroy(void *obj)
{
	AG_DriverGLX *glx = obj;

	Free(glx->clipRects);
	Free(glx->textureGC);
}

/*
 * Driver initialization
 */

static void
InitGlobals(void)
{
	agScreen = DefaultScreen(agDisplay);
	InitKeymaps();
	memset(xkbBuf, '\0', sizeof(xkbBuf));
	memset(&xkbCompStatus, 0, sizeof(xkbCompStatus));
	wmDeleteWindowAtom = XInternAtom(agDisplay, "WM_DELETE_WINDOW", False);
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
		agMouse = AG_MouseNew(glx, "X mouse");
		agKeyboard = AG_KeyboardNew(glx, "X keyboard");
	} else {
		AGDRIVER(glx)->mouse = agMouse;
		AGDRIVER(glx)->kbd = agKeyboard;
	}

#ifdef DEBUG_XSYNC
	XSynchronize(agDisplay, True);
	XSetErrorHandler(HandleErrorX11);
#endif /* DEBUG_XSYNC */

	nDrivers++;
	return (0);
fail:
	agDisplay = NULL;
	agScreen = 0;
	return (-1);
}

static void
Close(void *obj)
{
#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (--nDrivers == 0) {
		XCloseDisplay(agDisplay);
		agDisplay = NULL;
		agScreen = 0;

		AG_ObjectDetach(agMouse);
		AG_ObjectDestroy(agMouse);
		agMouse = NULL;
		AG_ObjectDetach(agKeyboard);
		AG_ObjectDestroy(agKeyboard);
		agKeyboard = NULL;
	}
}

static int
GetDisplaySize(Uint *w, Uint *h)
{
	*w = (Uint)DisplayWidth(agDisplay, agScreen);
	*h = (Uint)DisplayHeight(agDisplay, agScreen);
	return (0);
}

/* Get Agar keycode corresponding to an X KeyCode */
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
}

/* Return the number of pending events in the event queue. */
static int
PendingEvents(void)
{
	struct timeval tv;
	fd_set fdset;
	int fd;

	XFlush(agDisplay);
	if (XEventsQueued(agDisplay, QueuedAlready))
		return (1);

	/* Block on the X connection fd */
	fd = ConnectionNumber(agDisplay);
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (select(fd+1, &fdset, NULL, NULL, &tv) == 1)
		return XPending(agDisplay);

	return (0);
}

/* Return the Agar window corresponding to a X Window ID */
static __inline__ AG_Window *
LookupWindowByID(Window xw)
{
	AG_Driver *drv;

	/* XXX TODO portable to optimize based on numerical XIDs? */
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (AGDRIVER_MULTIPLE(drv)) {
			AG_DriverGLX *glx = (AG_DriverGLX *)drv;

			if (glx->w == xw)
				return (AGDRIVER_MW(drv)->win);
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
	AG_SetModState(agKeyboard, ms);
}

static void
ChangeCursor(AG_DriverGLX *glx)
{
	AG_Cursor *ac = glx->cursorToSet;
	struct ag_cursor_glx *cg = ac->p;

	if (glx->cursorToSet != AGDRIVER(glx)->activeCursor) {
		if (glx->cursorToSet == &AGDRIVER(glx)->cursors[0]) {
			XUndefineCursor(agDisplay, glx->w);
		} else {
			XDefineCursor(agDisplay, glx->w, cg->xc);
		}
		AGDRIVER(glx)->activeCursor = ac;
		cg->visible = 1;
	}
	glx->cursorToSet = NULL;
}

static int
ProcessEvents(void *drvCaller)
{
	AG_DriverGLX *glx;
	AG_Window *win;
	XEvent xev;
	AG_KeySym ks;
	AG_KeyboardAction ka;
	Uint32 ucs;
	int x, y;
	int nProcessed = 0;

	while (PendingEvents()) {
		AG_LockVFS(&agDrivers);
		XNextEvent(agDisplay, &xev);
		switch (xev.type) {
		case MotionNotify:
			if ((win = LookupWindowByID(xev.xmotion.window))) {
				glx = (AG_DriverGLX *)WIDGET(win)->drv;
				x = AGDRIVER_BOUNDED_WIDTH(win, xev.xmotion.x);
				y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xmotion.y);
				AG_MouseMotionUpdate(agMouse, x,y);
				AG_ProcessMouseMotion(win, x, y,
				    agMouse->xRel,
				    agMouse->yRel,
				    agMouse->btnState);
				
				if (glx->cursorToSet != NULL)
					ChangeCursor(glx);
			} else {
				Verbose("MotionNotify on unknown window\n");
			}
			break;
		case ButtonPress:
			if ((win = LookupWindowByID(xev.xbutton.window))) {
				x = AGDRIVER_BOUNDED_WIDTH(win, xev.xbutton.x);
				y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xbutton.y);
				AG_MouseButtonUpdate(agMouse, AG_BUTTON_PRESSED,
				    xev.xbutton.button);
				AG_ProcessMouseButtonDown(win, x, y,
				    xev.xbutton.button);
			} else {
				Verbose("ButtonPress on unknown window\n");
			}
			break;
		case ButtonRelease:
			if ((win = LookupWindowByID(xev.xbutton.window))) {
				x = AGDRIVER_BOUNDED_WIDTH(win, xev.xbutton.x);
				y = AGDRIVER_BOUNDED_HEIGHT(win, xev.xbutton.y);
				AG_MouseButtonUpdate(agMouse, AG_BUTTON_RELEASED,
				    xev.xbutton.button);
				AG_ProcessMouseButtonUp(win, x, y,
				    xev.xbutton.button);
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
#if 0
				printf("KeyPress ks=0x%x (ucs=%c)\n",
				    (Uint)ks, (char)ucs);
#endif
				AG_KeyboardUpdate(agKeyboard, ka, ks, ucs);
				if ((win = LookupWindowByID(xev.xkey.window))
				    != NULL) {
					AG_ProcessKey(agKeyboard, win, ka, ks,
					    ucs);
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
				AG_MouseMotionUpdate(agMouse,
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
				AG_MouseMotionUpdate(agMouse,
				    xev.xcrossing.x,
				    xev.xcrossing.y);
				InputEvent(&xev);
			}
			break;
#endif
		case EnterNotify:
		case LeaveNotify:
			/* printf(">{Enter,Leave}Notify\n"); */
			break;
		case FocusIn:
			if ((win = LookupWindowByID(xev.xfocus.window))) {
				agWindowFocused = win;
				AG_PostEvent(NULL, win, "window-gainfocus", NULL);
			}
			break;
		case FocusOut:
			if ((win = LookupWindowByID(xev.xfocus.window)) &&
			    agWindowFocused == win) {
				AG_PostEvent(NULL, win, "window-lostfocus", NULL);
				agWindowFocused = NULL;
			}
			break;
		case MapNotify:
		case UnmapNotify:
			/* printf(">{Map,Unmap}Notify\n"); */
			break;
		case KeymapNotify:
			UpdateKeyboard(agKeyboard, xev.xkeymap.key_vector);
			break;
		case MappingNotify:
			XRefreshKeyboardMapping(&xev.xmapping);
			break;
		case ConfigureNotify:
			if ((win = LookupWindowByID(xev.xconfigure.window))
			    != NULL) {
				AG_SizeAlloc a;

				a.x = 0;
				a.y = 0;
				a.w = xev.xconfigure.width;
				a.h = xev.xconfigure.height;
				PostResizeCallback(win, &a);
			}
			break;
		case ReparentNotify:
			/* printf("ReparentNotify\n"); */
			break;
		case ClientMessage:
			if ((xev.xclient.format == 32) &&
			    (xev.xclient.data.l[0] == wmDeleteWindowAtom)) {
				goto quit;
			}
			break;
#if 0
		case Expose:
			if ((win = LookupWindowByID(xev.xexpose.window)) != NULL) {
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
		if (!TAILQ_EMPTY(&agWindowDetachQ)) {
			AG_FreeDetachedWindows();
		}
		if (agWindowToFocus != NULL) {
			glx = (AG_DriverGLX *)WIDGET(agWindowToFocus)->drv;
			if (glx != NULL && AGDRIVER_IS_GLX(glx)) {
				RaiseWindow(agWindowToFocus);
				SetInputFocus(agWindowToFocus);
			}
			/* XXX TODO window-lostfocus to previous */
			AG_PostEvent(NULL, agWindowToFocus,
			    "window-gainfocus", NULL);
			agWindowToFocus = NULL;
		}
		AG_UnlockVFS(&agDrivers);
	}
	return (nProcessed);
quit:
	agTerminating = 1;
	AG_UnlockVFS(&agDrivers);
	return (-1);
}

static void
GenericEventLoop(void *obj)
{
	AG_Driver *drv;
	AG_Window *win;
	Uint32 t1, t2;

	t1 = AG_GetTicks();
	for (;;) {
		t2 = AG_GetTicks();
		if (t2 - t1 >= rNom) {
			/* XXX */
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
				if (AGDRIVER_CLASS(drv)->wm != AG_WM_MULTIPLE) {
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
		} else if (PendingEvents() != 0) {
			if (ProcessEvents(NULL) == -1) {
				printf("terminating\n");
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

/* Prepare our GL context for GUI rendering. */
static void
InitGLContext(AG_DriverGLX *glx)
{
	AG_Window *win = AGDRIVER_MW(glx)->win;

	glViewport(0, 0, WIDTH(win), HEIGHT(win));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIDTH(win), HEIGHT(win), 0, -1.0, 1.0);

	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
}

static void
BeginRendering(void *obj)
{
	AG_DriverGLX *glx = obj;

	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
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
	
/*	glFlush(); */
	glXSwapBuffers(agDisplay, glx->w);

	/* Remove textures queued for deletion. */
	for (i = 0; i < glx->nTextureGC; i++) {
		glDeleteTextures(1, (GLuint *)&glx->textureGC[i]);
	}
	glx->nTextureGC = 0;
/*	glXMakeCurrent(agDisplay, None, NULL); */
}
	
static void
FillRect(void *obj, AG_Rect r, AG_Color c)
{
	int x2 = r.x + r.w - 1;
	int y2 = r.y + r.h - 1;

	glBegin(GL_POLYGON);
	glColor3ub(c.r, c.g, c.b);
	glVertex2i(r.x, r.y);
	glVertex2i(x2, r.y);
	glVertex2i(x2, y2);
	glVertex2i(r.x, y2);
	glEnd();
}

static void
UpdateRegion(void *obj, AG_Rect r)
{
	/* No-op */
}

static void
CopyColorKeySurface(AG_Surface *suTex, AG_Surface *suSrc)
{
	Uint8 *pSrc;
	int x, y;

	pSrc = suSrc->pixels;
	for (y = 0; y < suSrc->h; y++) {
		for (x = 0; x < suSrc->w; x++) {
			Uint32 c = AG_GET_PIXEL(suSrc,pSrc);
			Uint8 r,g,b;

			if (c != suSrc->format->colorkey) {
				AG_GetRGB(c, suSrc->format, &r,&g,&b);
				AG_PUT_PIXEL2(suTex, x,y, 
				    AG_MapRGBA(suTex->format,
				    r,g,b,AG_ALPHA_OPAQUE));
			} else {
				AG_PUT_PIXEL2(suTex, x,y, 
				    AG_MapRGBA(suTex->format,
				    0,0,0,AG_ALPHA_TRANSPARENT));
			}
			pSrc += suSrc->format->BytesPerPixel;
		}
	}
}

static int
UploadTexture(Uint *rv, AG_Surface *suSrc, AG_TexCoord *tc)
{
	AG_Surface *suTex;
	int w = PowOf2i(suSrc->w);
	int h = PowOf2i(suSrc->h);
	GLuint texture;

	/* Convert to the GL_RGBA/GL_UNSIGNED_BYTE format. */
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)suSrc->w / w;
		tc->h = (float)suSrc->h / h;
	}
	suTex = AG_SurfaceRGBA(w,h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	    );
	if (suTex == NULL) {
		return (-1);
	}
	if (suSrc->flags & AG_SRCCOLORKEY) {
		CopyColorKeySurface(suTex, suSrc);
	} else {
		AG_SurfaceCopy(suTex, suSrc);
	}
	
	/* Upload as an OpenGL texture. */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
#if 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, suTex->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	AG_SurfaceFree(suTex);
	*rv = texture;
	return (0);
}

static int
UpdateTexture(Uint texture, AG_Surface *su)
{
	AG_Surface *suTex;
	int w, h;

	/*
	 * Convert to the GL_RGBA/GL_UNSIGNED_BYTE format and adjust for
	 * power-of-two dimensions.
	 * TODO check for GL_ARB_texture_non_power_of_two.
	 */
	w = PowOf2i(su->w);
	h = PowOf2i(su->h);
	suTex = AG_SurfaceRGBA(w,h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	);
	if (suTex == NULL) {
		return (-1);
	}
	AG_SurfaceCopy(suTex, su);

	/* Upload as an OpenGL texture. */
	glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, suTex->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	AG_SurfaceFree(suTex);
	return (0);
}

static void
DeleteTexture(void *drv, Uint texture)
{
	AG_DriverGLX *glx = drv;

	glx->textureGC = Realloc(glx->textureGC,
	    (glx->nTextureGC+1)*sizeof(Uint));
	glx->textureGC[glx->nTextureGC++] = texture;
}

static int
SetRefreshRate(void *obj, int fps)
{
	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	rNom = 1000/fps;
	rCur = 0;
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
}

static void
PopClipRect(void *obj)
{
	AG_DriverGLX *glx = obj;
	AG_ClipRect *cr;
	
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
}

static __inline__ GLenum
GetBlendingFunc(AG_BlendFn fn)
{
	switch (fn) {
	case AG_ALPHA_ONE:		return (GL_ONE);
	case AG_ALPHA_ZERO:		return (GL_ZERO);
	case AG_ALPHA_SRC:		return (GL_SRC_ALPHA);
	case AG_ALPHA_DST:		return (GL_DST_ALPHA);
	case AG_ALPHA_ONE_MINUS_DST:	return (GL_ONE_MINUS_DST_ALPHA);
	case AG_ALPHA_ONE_MINUS_SRC:	return (GL_ONE_MINUS_SRC_ALPHA);
	case AG_ALPHA_OVERLAY:		return (GL_ONE);	/* XXX */
	default:			return (GL_ONE);
	}
}

static void
PushBlendingMode(void *obj, AG_BlendFn srcFn, AG_BlendFn dstFn)
{
	AG_DriverGLX *glx = obj;

	/* XXX TODO: stack */
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
	    &glx->bs[0].texEnvMode);
	glGetBooleanv(GL_BLEND, &glx->bs[0].enabled);
	glGetIntegerv(GL_BLEND_SRC, &glx->bs[0].srcFactor);
	glGetIntegerv(GL_BLEND_DST, &glx->bs[0].dstFactor);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(GetBlendingFunc(srcFn), GetBlendingFunc(dstFn));
}

static void
PopBlendingMode(void *obj)
{
	AG_DriverGLX *glx = obj;

	/* XXX TODO: stack */
	if (glx->bs[0].enabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
	glBlendFunc(glx->bs[0].srcFactor, glx->bs[0].dstFactor);
}

/*
 * Cursor operations
 */

static int
CreateCursor(void *obj, AG_Cursor *ac)
{
	struct ag_cursor_glx *cg;
	int i, size;
	char *xData, *xMask;
	XGCValues gcVals;
	GC gc;
	XImage *dataImg, *maskImg;
	Pixmap dataPixmap, maskPixmap;

	if ((cg = AG_TryMalloc(sizeof(struct ag_cursor_glx))) == NULL) {
		return (-1);
	}
	memset(&cg->black, 0, sizeof(cg->black));
	cg->white.pixel = 0xffff;
	cg->white.red = 0xffff;
	cg->white.green = 0xffff;
	cg->white.blue = 0xffff;

	size = (ac->w/8)*ac->h;
	if ((xData = AG_TryMalloc(size)) == NULL) {
		free(cg);
		return (-1);
	}
	if ((xMask = AG_TryMalloc(size)) == NULL) {
		free(xData);
		free(cg);
		return (-1);
	}
	for (i = 0; i < size; i++) {
		xMask[i] = ac->data[i] | ac->mask[i];
		xData[i] = ac->data[i];
	}

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

	ac->p = cg;
	return (0);
}

static void
FreeCursor(void *obj, AG_Cursor *ac)
{
	struct ag_cursor_glx *cg = ac->p;
	
	XFreeCursor(agDisplay, cg->xc);
	XSync(agDisplay, False);
	free(cg);
	ac->p = NULL;
}

static int
PushCursor(void *obj, AG_Cursor *ac)
{
	AG_DriverGLX *glx = obj;

	glx->cursorToSet = ac;
	return (0);
}

static void
PopCursor(void *obj)
{
	AG_DriverGLX *glx = obj;
	
	glx->cursorToSet = &AGDRIVER(glx)->cursors[0];
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
 * Surface operations (rendering context)
 */

static __inline__ void
UpdateWidgetTexture(AG_Widget *wid, int s)
{
	if (wid->textures[s] == 0) {
		wid->textures[s] = AG_SurfaceTexture(wid->surfaces[s],
		    &wid->texcoords[s]);
	} else if (wid->surfaceFlags[s] & AG_WIDGET_SURFACE_REGEN) {
		wid->surfaceFlags[s] &= ~(AG_WIDGET_SURFACE_REGEN);
		UpdateTexture(wid->textures[s], wid->surfaces[s]);
	}
}

/* XXX inefficient */
static void
BlitSurface(void *obj, AG_Widget *wid, AG_Surface *s, int x, int y)
{
	AG_Driver *drv = obj;
	GLuint texture;
	AG_TexCoord tc;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

	texture = AG_SurfaceTexture(s, &tc);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc.x, tc.y);	glVertex2i(x,      y);
		glTexCoord2f(tc.w, tc.y);	glVertex2i(x+s->w, y);
		glTexCoord2f(tc.x, tc.h);	glVertex2i(x,      y+s->h);
		glTexCoord2f(tc.w, tc.h);	glVertex2i(x+s->w, y+s->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

static void
BlitSurfaceFrom(void *obj, AG_Widget *wid, AG_Widget *widSrc, int s, AG_Rect *r,
    int x, int y)
{
	AG_Driver *drv = obj;
	AG_Surface *su = widSrc->surfaces[s];
	AG_TexCoord tcTmp, *tc;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");
	AG_ASSERT_CLASS(widSrc, "AG_Widget:*");

	UpdateWidgetTexture(widSrc, s);

	if (r == NULL) {
		tc = &widSrc->texcoords[s];
	} else {
		tc = &tcTmp;
		tcTmp.x = (float)r->x/PowOf2i(r->x);
		tcTmp.y = (float)r->y/PowOf2i(r->y);
		tcTmp.w = (float)r->w/PowOf2i(r->w);
		tcTmp.h = (float)r->h/PowOf2i(r->h);
	}

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	glBindTexture(GL_TEXTURE_2D, widSrc->textures[s]);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2i(x,       y);
		glTexCoord2f(tc->w, tc->y);	glVertex2i(x+su->w, y);
		glTexCoord2f(tc->x, tc->h);	glVertex2i(x,       y+su->h);
		glTexCoord2f(tc->w, tc->h);	glVertex2i(x+su->w, y+su->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

static void
BlitSurfaceGL(void *obj, AG_Widget *wid, AG_Surface *s, float w, float h)
{
	AG_Driver *drv = obj;
	GLuint texture;
	AG_TexCoord tc;
	float w2 = w/2.0f;
	float h2 = h/2.0f;

	texture = AG_SurfaceTexture(s, &tc);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc.x, tc.y);	glVertex2f( w2,  h2);
		glTexCoord2f(tc.w, tc.y);	glVertex2f(-w2,  h2);
		glTexCoord2f(tc.x, tc.h);	glVertex2f( w2, -h2);
		glTexCoord2f(tc.w, tc.h);	glVertex2f(-w2, -h2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

static void
BlitSurfaceFromGL(void *obj, AG_Widget *wid, int s, float w, float h)
{
	AG_Driver *drv = obj;
	AG_TexCoord *tc;
	float w2 = w/2.0f;
	float h2 = h/2.0f;
	
	UpdateWidgetTexture(wid, s);
	tc = &wid->texcoords[s];

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, wid->textures[s]);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2f( w2,  h2);
		glTexCoord2f(tc->w, tc->y);	glVertex2f(-w2,  h2);
		glTexCoord2f(tc->x, tc->h);	glVertex2f( w2, -h2);
		glTexCoord2f(tc->w, tc->h);	glVertex2f(-w2, -h2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

static void
BlitSurfaceFlippedGL(void *obj, AG_Widget *wid, int s, float w, float h)
{
	AG_Driver *drv = obj;
	AG_TexCoord *tc;
	
	UpdateWidgetTexture(wid, s);
	tc = &wid->texcoords[s];

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[s]);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->w, tc->y);	glVertex2f(0.0, 0.0);
		glTexCoord2f(tc->x, tc->y);	glVertex2f(w,   0.0);
		glTexCoord2f(tc->w, tc->h);	glVertex2f(0.0, h);
		glTexCoord2f(tc->x, tc->h);	glVertex2f(w,   h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

static void
BackupSurfaces(void *obj, AG_Widget *wid)
{
	AG_Surface *su;
	GLint w, h;
	Uint i;

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++)  {
		if (wid->textures[i] == 0 ||
		    wid->surfaces[i] != NULL) {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[i]);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
		    &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,
		    &h);

		su = AG_SurfaceRGBA(w, h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
			0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
		);
		if (su == NULL) {
			AG_FatalError("Allocating texture: %s", AG_GetError());
		}
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		    su->pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		wid->surfaces[i] = su;
	}
	glDeleteTextures(wid->nsurfaces, (GLuint *)wid->textures);
	memset(wid->textures, 0, wid->nsurfaces*sizeof(Uint));
	AG_ObjectUnlock(wid);
}

static void
RestoreSurfaces(void *obj, AG_Widget *wid)
{
	Uint i;

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++)  {
		if (wid->surfaces[i] != NULL) {
			wid->textures[i] = AG_SurfaceTexture(wid->surfaces[i],
			    &wid->texcoords[i]);
		} else {
			wid->textures[i] = 0;
		}
	}
	AG_ObjectUnlock(wid);
}

static int
RenderToSurface(void *obj, AG_Widget *wid, AG_Surface **s)
{
	AG_Driver *drv = obj;
	Uint8 *pixels;
	AG_Surface *su;
	int visiblePrev;

	/* XXX TODO render to offscreen buffer instead of display! */
	AG_BeginRendering(drv);
	visiblePrev = wid->window->visible;
	wid->window->visible = 1;
	AG_WindowDraw(wid->window);
	wid->window->visible = visiblePrev;
	AG_EndRendering(drv);

	if ((pixels = AG_TryMalloc(wid->w*wid->h*4)) == NULL) {
		return (-1);
	}
	glReadPixels(
	    wid->rView.x1,
	    HEIGHT(AGDRIVER_MW(drv)->win) - wid->rView.y2,
	    wid->w,
	    wid->h,
	    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	AG_FlipSurface(pixels, wid->h, wid->w*4);
	su = AG_SurfaceFromPixelsRGBA(pixels, wid->w, wid->h, 32,
	    (wid->w*4), 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
	if (su == NULL) {
		free(pixels);
		return (-1);
	}
	return (0);
}

/*
 * Rendering operations (rendering context)
 */

static void
PutPixel(void *obj, int x, int y, AG_Color C)
{
	glBegin(GL_POINTS);
	glColor3ub(C.r, C.g, C.b);
	glVertex2i(x, y);
	glEnd();
}

static void
PutPixel32(void *obj, int x, int y, Uint32 c)
{
	AG_Driver *drv = obj;
	Uint8 r, g, b;

	AG_GetRGB(c, drv->videoFmt, &r,&g,&b);
	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2i(x, y);
	glEnd();
}
	
static void
PutPixelRGB(void *obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2i(x, y);
	glEnd();
}

static void
BlendPixel(void *obj, int x, int y, AG_Color C, AG_BlendFn fnSrc,
    AG_BlendFn fnDst)
{
	AG_Driver *drv = obj;

	/* XXX inefficient */

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv, fnSrc, fnDst);
	glBegin(GL_POINTS);
	glColor4ub(C.r, C.g, C.b, C.a);
	glVertex2i(x, y);
	glEnd();
	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

static void
DrawLine(void *obj, int x1, int y1, int x2, int y2, AG_Color C)
{
	glBegin(GL_LINES);
	glColor3ub(C.r, C.g, C.b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

static void
DrawLineH(void *obj, int x1, int x2, int y, AG_Color C)
{
	glBegin(GL_LINES);
	glColor3ub(C.r, C.g, C.b);
	glVertex2s(x1, y);
	glVertex2s(x2, y);
	glEnd();
}

static void
DrawLineV(void *obj, int x, int y1, int y2, AG_Color C)
{
	glBegin(GL_LINES);
	glColor3ub(C.r, C.g, C.b);
	glVertex2s(x, y1);
	glVertex2s(x, y2);
	glEnd();
}

static void
DrawLineBlended(void *obj, int x1, int y1, int x2, int y2, AG_Color C,
    AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	if (C.a < 255)
		PushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_LINES);
	glColor4ub(C.r, C.g, C.b, C.a);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
	
	if (C.a < 255)
		PopBlendingMode(obj);
}

static void
DrawArrowUp(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x,		y - h2);
		glVertex2i(x - h2,	y + h2);
		glVertex2i(x + h2,	y + h2);
	}
	glEnd();
}

static void
DrawArrowDown(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x - h2,	y - h2);
		glVertex2i(x + h2,	y - h2);
		glVertex2i(x,		y + h2);
	}
	glEnd();
}

static void
DrawArrowLeft(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x - h2,	y);
		glVertex2i(x + h2,	y + h2);
		glVertex2i(x + h2,	y - h2);
	}
	glEnd();
}

static void
DrawArrowRight(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x + h2,	y);
		glVertex2i(x - h2,	y + h2);
		glVertex2i(x - h2,	y - h2);
	}
	glEnd();
}

static void
DrawRectDithered(void *obj, AG_Rect r, AG_Color C)
{
	AG_DriverGLX *glx = obj;
	int stipplePrev;
	
	stipplePrev = glIsEnabled(GL_POLYGON_STIPPLE);
	glEnable(GL_POLYGON_STIPPLE);
	glPushAttrib(GL_POLYGON_STIPPLE_BIT);
	glPolygonStipple(glx->disabledStipple);
	DrawRectFilled(obj, r, C);
	glPopAttrib();
	if (!stipplePrev) { glDisable(GL_POLYGON_STIPPLE); }
}

/* Render a 3D-style box with rounded edges. */
static void
DrawBoxRounded(void *obj, AG_Rect r, int z, int rad, AG_Color C[3])
{
	glPushMatrix();
	glTranslatef((float)r.x, (float)r.y, 0.0);

	/* XXX TODO */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(0, r.h);
		glVertex2i(0, rad);
		glVertex2i(rad, 0);
		glVertex2i(r.w-rad, 0);
		glVertex2i(r.w, rad);
		glVertex2i(r.w, r.h);
	}
	glEnd();
	if (z >= 0) {
		glBegin(GL_LINE_STRIP);
		{
			glColor3ub(C[1].r, C[1].g, C[1].b);
			glVertex2i(0, r.h);
			glVertex2i(0, rad);
			glVertex2i(rad, 0);
		}
		glEnd();
		glBegin(GL_LINES);
		{
			glColor3ub(C[2].r, C[2].g, C[2].b);
			glVertex2i(r.w-1, r.h);
			glVertex2i(r.w-1, rad);
		}
		glEnd();
	}

	glPopMatrix();
}

/* Render a 3D-style box with rounded top edges. */
static void
DrawBoxRoundedTop(void *obj, AG_Rect r, int z, int rad, AG_Color C[3])
{
	glPushMatrix();
	glTranslatef((float)r.x, (float)r.y, 0.0);

	/* XXX TODO */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(0, r.h);
		glVertex2i(0, rad);
		glVertex2i(rad, 0);
		glVertex2i(r.w-rad, 0);
		glVertex2i(r.w, rad);
		glVertex2i(r.w, r.h);
	}
	glEnd();
	if (z >= 0) {
		glBegin(GL_LINE_STRIP);
		{
			glColor3ub(C[1].r, C[1].g, C[1].b);
			glVertex2i(0, r.h);
			glVertex2i(0, rad);
			glVertex2i(rad, 0);
		}
		glEnd();
		glBegin(GL_LINES);
		{
			glColor3ub(C[2].r, C[2].g, C[2].b);
			glVertex2i(r.w-1, r.h);
			glVertex2i(r.w-1, rad);
		}
		glEnd();
	}

	glPopMatrix();
}

static void
DrawCircle(void *obj, int x, int y, int radius, AG_Color C)
{
	int nEdges = radius*2;
	int i;
	
	glBegin(GL_LINE_LOOP);
	glColor3ub(C.r, C.g, C.b);
	for (i = 0; i < nEdges; i++) {
		glVertex2f(x + radius*Cos((2*AG_PI*i)/nEdges),
		           y + radius*Sin((2*AG_PI*i)/nEdges));
	}
	glEnd();
}

static void
DrawCircle2(void *obj, int x, int y, int radius, AG_Color C)
{
	int nEdges = radius*2;
	int i;
	
	glBegin(GL_LINE_LOOP);
	glColor3ub(C.r, C.g, C.b);
	for (i = 0; i < nEdges; i++) {
		glVertex2f(x + radius*Cos((2*AG_PI*i)/nEdges),
		           y + radius*Sin((2*AG_PI*i)/nEdges));
		glVertex2f(x + (radius+1)*Cos((2*AG_PI*i)/nEdges),
		           y + (radius+1)*Sin((2*AG_PI*i)/nEdges));
	}
	glEnd();
}

static void
DrawRectFilled(void *obj, AG_Rect r, AG_Color C)
{
	int x2 = r.x + r.w - 1;
	int y2 = r.y + r.h - 1;

	glBegin(GL_POLYGON);
	glColor3ub(C.r, C.g, C.b);
	glVertex2i(r.x, r.y);
	glVertex2i(x2, r.y);
	glVertex2i(x2, y2);
	glVertex2i(r.x, y2);
	glEnd();
}

static void
DrawRectBlended(void *obj, AG_Rect r, AG_Color C, AG_BlendFn fnSrc,
    AG_BlendFn fnDst)
{
	int x1 = r.x;
	int y1 = r.y;
	int x2 = x1+r.w;
	int y2 = y1+r.h;

	if (C.a < 255)
		PushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_POLYGON);
	glColor4ub(C.r, C.g, C.b, C.a);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
	
	if (C.a < 255)
		PopBlendingMode(obj);
}

static void
DrawFrame(void *obj, AG_Rect r, AG_Color C[2])
{
	DrawLineH(obj, r.x,		r.x+r.w-1,	r.y,		C[0]);
	DrawLineH(obj, r.x,		r.x+r.w,	r.y+r.h-1,	C[1]);
	DrawLineV(obj, r.x,		r.y,		r.y+r.h-1,	C[0]);
	DrawLineV(obj, r.x+r.w-1,	r.y,		r.y+r.h-1,	C[1]);
}

static void
DrawGlyph(void *obj, AG_Glyph *gl, int x, int y)
{
	AG_Driver *drv = obj;
	AG_Surface *su = gl->su;
	AG_TexCoord *tc;

	if (gl->nTextures < drv->id+1) {
		gl->textures = Realloc(gl->textures, (drv->id+1)*sizeof(Uint));
		gl->texcoords = Realloc(gl->texcoords, (drv->id+1)*sizeof(AG_TexCoord));
		gl->nTextures = drv->id+1;
		gl->textures[drv->id] = 0;
	}
	if (gl->textures[drv->id] == 0) {
		UploadTexture(&gl->textures[drv->id], su,
		    &gl->texcoords[drv->id]);
	}
	glBindTexture(GL_TEXTURE_2D, gl->textures[drv->id]);
	tc = &gl->texcoords[drv->id];
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2i(x,       y);
		glTexCoord2f(tc->w, tc->y);	glVertex2i(x+su->w, y);
		glTexCoord2f(tc->x, tc->h);	glVertex2i(x,       y+su->h);
		glTexCoord2f(tc->w, tc->h);	glVertex2i(x+su->w, y+su->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
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
	if ((glx->clipRects = AG_TryMalloc(sizeof(AG_ClipRect))) == NULL) {
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

/* Initialize the default cursor. */
static int
InitDefaultCursor(AG_DriverGLX *glx)
{
	AG_Driver *drv = AGDRIVER(glx);
	AG_Cursor *ac;
	struct ag_cursor_glx *cg;
	
	if ((cg = AG_TryMalloc(sizeof(struct ag_cursor_glx))) == NULL)
		return (-1);
	if ((drv->cursors = AG_TryMalloc(sizeof(AG_Cursor))) == NULL) {
		free(cg);
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

static int
OpenWindow(AG_Window *win, AG_Rect r, int depthReq, Uint flags)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv, *drvParent;
	XSetWindowAttributes xwAttrs;
	XVisualInfo *xvi;
	Window wParent;
	int glxAttrs[16];
	int i = 0;
	Ulong valuemask;
	int depth;
	
	glxAttrs[i++] = GLX_RGBA;
	glxAttrs[i++] = GLX_RED_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_GREEN_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_BLUE_SIZE;	glxAttrs[i++] = 2;
	glxAttrs[i++] = GLX_DEPTH_SIZE;	glxAttrs[i++] = 16;
	
	/* Try to obtain a double-buffered visual, fallback to single. */
	glxAttrs[i++] = GLX_DOUBLEBUFFER;
	glxAttrs[i++] = None;
	if ((xvi = glXChooseVisual(agDisplay, agScreen, glxAttrs)) == NULL) {
		glxAttrs[i-=2] = None;
		if ((xvi = glXChooseVisual(agDisplay, agScreen, glxAttrs))
		    == NULL) {
			AG_SetError("Cannot find an acceptable GLX visual");
			return (-1);
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

	if (win->flags & (AG_WINDOW_NOTITLE|AG_WINDOW_NOBORDERS)) {
		/* XXX */
//		valuemask |= CWOverrideRedirect;
//		xwAttrs.override_redirect = True;
	}
	if (win->parent != NULL &&
	    (drvParent = WIDGET(win->parent)->drv) &&
	    AGDRIVER_IS_GLX(drvParent) &&
	    (AGDRIVER_MW(drvParent)->flags & AG_DRIVER_MW_OPEN)) {
		AG_DriverGLX *glxParent = (AG_DriverGLX *)drvParent;
		wParent = glxParent->w;
	} else {
		wParent = RootWindow(agDisplay,agScreen);
	}

	/* Create an (initially unmapped) window. */
	depth = (depthReq >= 1) ? depthReq : xvi->depth;
	glx->w = XCreateWindow(agDisplay,
	    wParent,
	    r.x, r.y,
	    r.w, r.h, 0, depth,
	    InputOutput,
	    xvi->visual,
	    valuemask,
	    &xwAttrs);
	if (glx->w == 0) {
		AG_SetError("XCreateWindow failed");
		return (-1);
	}
	AGDRIVER_MW(glx)->flags |= AG_DRIVER_MW_OPEN;

	/* Create a GLX context and initialize state. */
	glx->glxCtx = glXCreateContext(agDisplay, xvi, 0, GL_FALSE);
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
	InitGLContext(glx);

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
	return (0);
fail:
	glXDestroyContext(agDisplay, glx->glxCtx);
	XDestroyWindow(agDisplay, glx->w);
	AGDRIVER_MW(glx)->flags &= ~(AG_DRIVER_MW_OPEN);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static void
CloseWindow(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverGLX *glx = (AG_DriverGLX *)drv;

	XDestroyWindow(agDisplay, glx->w);
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	AGDRIVER_MW(glx)->flags &= ~(AG_DRIVER_MW_OPEN);
}

static int
MapWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;

	XMapWindow(agDisplay, glx->w);
	XIfEvent(agDisplay, &xev, WaitMapNotify, (char *)glx->w);
	return (0);
}

static int
UnmapWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;

	XUnmapWindow(agDisplay, glx->w);
	XIfEvent(agDisplay, &xev, WaitUnmapNotify, (char *)glx->w);
	return (0);
}

static int
RaiseWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
/*	XEvent xev; */

	XRaiseWindow(agDisplay, glx->w);
/*	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w); */
	return (0);
}

static int
LowerWindow(AG_Window *win)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
/*	XEvent xev; */

	XLowerWindow(agDisplay, glx->w);
/*	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w); */
	return (0);
}

static int
ReparentWindow(AG_Window *win, AG_Window *winParent, int x, int y)
{
	AG_DriverGLX *glxWin = (AG_DriverGLX *)WIDGET(win)->drv;
	AG_DriverGLX *glxParentWin = (AG_DriverGLX *)WIDGET(winParent)->drv;
/*	XEvent xev; */

	XReparentWindow(agDisplay, glxWin->w, glxParentWin->w, x,y);
/*	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w); */
	return (0);
}

static int
GetInputFocus(AG_Window **rv)
{
	AG_DriverGLX *glx = NULL;
	Window wRet;
	int revertToRet;

	XGetInputFocus(agDisplay, &wRet, &revertToRet);

	AGOBJECT_FOREACH_CHILD(glx, &agDriver, ag_driver_glx) {
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
	
	XSetInputFocus(agDisplay, glx->w, RevertToParent, CurrentTime);
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
	AG_ClipRect *cr;
	char kv[32];

	(void)AG_WidgetSizeAlloc(win, a);
	AG_WidgetUpdateCoords(win, 0, 0);

	/* Update clipping rectangle 0 */
	cr = &glx->clipRects[0];
	cr->r.w = WIDTH(win);
	cr->r.h = HEIGHT(win);
	cr->eqns[2][3] = (double)WIDTH(win);
	cr->eqns[3][3] = (double)HEIGHT(win);
	
	/* Update the keyboard state. */
	XQueryKeymap(agDisplay, kv);
	UpdateKeyboard(agKeyboard, kv);

	/* Update GLX context. */
	glXMakeCurrent(agDisplay, glx->w, glx->glxCtx);
#if 0
	/* Restore our saved GL resources and reset GL state. */
	RegenWidgetResources(WIDGET(win));
#endif
	InitGLContext(glx);
}

static int
MoveWindow(AG_Window *win, int x, int y)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;

	/* printf("XMoveWindow(%d,%d)\n", x, y); */
	XMoveWindow(agDisplay, glx->w, x, y);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);
	return (0);
}

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

static int
ResizeWindow(AG_Window *win, Uint w, Uint h)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;
	
	/* Resize the window and wait for notification from the server. */
	XResizeWindow(agDisplay, glx->w, w, h);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);
	return (0);
}

static int
MoveResizeWindow(AG_Window *win, AG_SizeAlloc *a)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;

	/* printf("XMoveResizeWindow: %d,%d (%dx%d)\n",
	    a->x, a->y, a->w, a->h); */
	XMoveResizeWindow(agDisplay, glx->w,
	    a->x, a->y,
	    a->w, a->h);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);
	return (0);
}

static int
SetBorderWidth(AG_Window *win, Uint width)
{
	AG_DriverGLX *glx = (AG_DriverGLX *)WIDGET(win)->drv;
	XEvent xev;

	XSetWindowBorderWidth(agDisplay, glx->w, width);
	XIfEvent(agDisplay, &xev, WaitConfigureNotify, (char *)glx->w);
	return (0);
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
		BeginRendering,
		RenderWindow,
		EndRendering,
		FillRect,
		UpdateRegion,
		UploadTexture,
		UpdateTexture,
		DeleteTexture,
		SetRefreshRate,
		PushClipRect,
		PopClipRect,
		PushBlendingMode,
		PopBlendingMode,
		CreateCursor,
		FreeCursor,
		PushCursor,
		PopCursor,
		GetCursorVisibility,
		SetCursorVisibility,
		BlitSurface,
		BlitSurfaceFrom,
		BlitSurfaceGL,
		BlitSurfaceFromGL,
		BlitSurfaceFlippedGL,
		BackupSurfaces,
		RestoreSurfaces,
		RenderToSurface,
		PutPixel,
		PutPixel32,
		PutPixelRGB,
		BlendPixel,
		DrawLine,
		DrawLineH,
		DrawLineV,
		DrawLineBlended,
		DrawArrowUp,
		DrawArrowDown,
		DrawArrowLeft,
		DrawArrowRight,
		DrawBoxRounded,
		DrawBoxRoundedTop,
		DrawCircle,
		DrawCircle2,
		DrawRectFilled,
		DrawRectBlended,
		DrawRectDithered,
		DrawFrame,
		DrawGlyph
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
	SetBorderWidth
};

#endif /* HAVE_GLX */

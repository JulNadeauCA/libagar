/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Low-level interface between the GUI and video display.
 */

#include <config/have_jpeg.h>
#include <config/have_x11.h>

#include <core/core.h>
#include <core/config.h>
#include <core/dir.h>

#include "window.h"
#include "primitive.h"
#include "cursors.h"
#include "colors.h"
#include "menu.h"
#include "text.h"
#ifdef DEBUG
#include "label.h"
#include "fixed_plotter.h"
#endif
#include "gui_math.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifdef HAVE_JPEG
#undef HAVE_STDLIB_H		/* Work around SDL.h retardation */
#include <jpeglib.h>
#include <errno.h>
#endif

#include "opengl.h"

/*
 * Invert the Y-coordinate in OpenGL mode.
 */
/* #define OPENGL_INVERTED_Y */

AG_Display *agView = NULL;		/* Main view */
AG_PixelFormat *agVideoFmt = NULL;	/* Current format of display */
AG_PixelFormat *agSurfaceFmt = NULL;	/* Preferred format for surfaces */
const SDL_VideoInfo *agVideoInfo;	/* Display information */
int agBgPopupMenu = 0;			/* Background popup menu */
static int initedGlobals = 0;

const char *agBlendFuncNames[] = {
	N_("Alpha sum"),
	N_("Source alpha"),
	N_("Destination alpha"),
	N_("One minus destination alpha"),
	N_("One minus source alpha"),
	NULL
};

struct ag_global_key {
	SDLKey keysym;
	SDLMod keymod;
	void (*fn)(void);
	void (*fn_ev)(AG_Event *);
	SLIST_ENTRY(ag_global_key) gkeys;
};
static SLIST_HEAD(,ag_global_key) agGlobalKeys =
    SLIST_HEAD_INITIALIZER(&agGlobalKeys);
static AG_Mutex agGlobalKeysLock;

#ifdef DEBUG
int agEventAvg = 0;		/* Number of events in last frame */
int agIdleAvg = 0;		/* Measured SDL_Delay() granularity */
AG_Window *agPerfWindow;
static AG_FixedPlotter *agPerfGraph;
static AG_FixedPlotterItem *agPerfFPS, *agPerfEvnts, *agPerfIdle;
#endif

#ifdef HAVE_OPENGL
static void
InitGL(void)
{
	Uint8 bR, bG, bB;

	glViewport(0, 0, agView->w, agView->h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, agView->w, agView->h, 0, -1.0, 1.0);

	AG_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &bR, &bG, &bB);
	glClearColor(bR/255.0, bG/255.0, bB/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_FLAT);
		
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
}
#endif /* HAVE_OPENGL */

void
AG_ClearBackground(void)
{
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Uint8 r, g, b;

		AG_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &r, &g, &b);
		glClearColor(r/255.0, g/255.0, b/255.0, 1.0);
	} else
#endif
	{
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, agView->w, agView->h);
	}
}

static void
InitView(AG_Display *v)
{
	AG_ObjectInit(v, &agDisplayClass);
	AG_ObjectSetName(v, "_agView");
	v->winop = AG_WINOP_NONE;
	v->ndirty = 0;
	v->maxdirty = 4;
	v->dirty = Malloc(v->maxdirty*sizeof(SDL_Rect));
	v->opengl = 0;
	v->winModal = Malloc(sizeof(AG_Window *));
	v->nModal = 0;
	v->winSelected = NULL;
	v->winToFocus = NULL;
	v->rNom = 16;
	v->rCur = 0;
	TAILQ_INIT(&v->windows);
	TAILQ_INIT(&v->detach);
}

static void
InitGlobals(void)
{
	if (initedGlobals) {
		return;
	}
	initedGlobals = 1;

	AG_MutexInitRecursive(&agGlobalKeysLock);
	AG_RegisterClass(&agDisplayClass);
	agVideoInfo = SDL_GetVideoInfo();
}

/*
 * Initialize Agar with an existing SDL/OpenGL display surface. If the
 * surface has SDL_OPENGL or SDL_OPENGLBLIT set, we assume that OpenGL
 * primitives are to be used in rendering.
 */
int
AG_InitVideoSDL(SDL_Surface *display, Uint flags)
{
	InitGlobals();
	
	if (display->w < AG_GetUint16(agConfig,"view.min-w") ||
	    display->h < AG_GetUint16(agConfig,"view.min-h")) {
		AG_SetError(_("The resolution is too small."));
		return (-1);
	}

	agView = Malloc(sizeof(AG_Display));
	InitView(agView);
	agView->v = display;
	agView->w = display->w;
	agView->h = display->h;
	agView->depth = display->format->BitsPerPixel;
	agView->rNom = 1000/AG_GetUint(agConfig, "view.nominal-fps");

	AG_SetUint8(agConfig, "view.depth", agView->depth);
	AG_SetUint16(agConfig, "view.w", agView->w);
	AG_SetUint16(agConfig, "view.h", agView->h);
	AG_SetBool(agConfig, "view.full-screen", display->flags&SDL_FULLSCREEN);
	AG_SetBool(agConfig, "view.async-blits", display->flags&SDL_ASYNCBLIT);

	if (display->flags & (SDL_OPENGL|SDL_OPENGLBLIT)) {
		AG_SetBool(agConfig, "view.opengl", 1);
		agView->opengl = 1;
	} else {
		if (flags & AG_VIDEO_OPENGL) {
			AG_SetError("AG_VIDEO_OPENGL flag requested, but "
			            "display surface is missing SDL_OPENGL");
			goto fail;
		}
		AG_SetBool(agConfig, "view.opengl", 0);
		agView->opengl = 0;
	}

	agView->stmpl = AG_SurfaceRGBA(1,1, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
 	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	if (agView->stmpl == NULL) {
		return (-1);
	}
	agVideoFmt = agView->v->format;
	agSurfaceFmt = agView->stmpl->format;

#ifdef HAVE_OPENGL
	if (agView->opengl)
		InitGL();
#endif
	if (AG_InitGUI(0) == -1) {
		goto fail;
	}
	if (flags & AG_VIDEO_BGPOPUPMENU)
		agBgPopupMenu = 1;
	if (!(flags & AG_VIDEO_NOBGCLEAR))
		AG_ClearBackground();

	return (0);
fail:
	Free(agView);
	agView = NULL;
	return (-1);
}

/* Initialize Agar with a new video display. */
int
AG_InitVideo(int w, int h, int bpp, Uint flags)
{
	Uint32 screenflags = 0;
	int depth;

	InitGlobals();

	if (!SDL_WasInit(SDL_INIT_VIDEO) &&
	    SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		AG_SetError("SDL_INIT_VIDEO: %s", SDL_GetError());
		return (-1);
	}
	SDL_WM_SetCaption(agProgName, agProgName);

	if (flags & (AG_VIDEO_OPENGL|AG_VIDEO_OPENGL_OR_SDL)) {
#ifdef HAVE_OPENGL
		AG_SetBool(agConfig, "view.opengl", 1);
#else
		if ((flags & AG_VIDEO_OPENGL_OR_SDL) == 0)
			AG_FatalError("Agar OpenGL support is not compiled in");
#endif
	}
	
	if (flags & AG_VIDEO_HWSURFACE) {
		screenflags |= SDL_HWSURFACE;
	} else {
		screenflags |= SDL_SWSURFACE;
	}
	if (flags & AG_VIDEO_RESIZABLE) { screenflags |= SDL_RESIZABLE; }
	if (flags & AG_VIDEO_ASYNCBLIT) { screenflags |= SDL_ASYNCBLIT; }
	if (flags & AG_VIDEO_ANYFORMAT) { screenflags |= SDL_ANYFORMAT; }
	if (flags & AG_VIDEO_HWPALETTE) { screenflags |= SDL_HWPALETTE; }
	if (flags & AG_VIDEO_DOUBLEBUF) { screenflags |= SDL_DOUBLEBUF; }
	if (flags & AG_VIDEO_FULLSCREEN) { screenflags |= SDL_FULLSCREEN; }
	if (flags & AG_VIDEO_NOFRAME) { screenflags |= SDL_NOFRAME; }
	if (flags & AG_VIDEO_BGPOPUPMENU) { agBgPopupMenu = 1; }

	agView = Malloc(sizeof(AG_Display));
	InitView(agView);

	agView->rNom = 1000/AG_GetUint(agConfig,"view.nominal-fps");
	depth = bpp > 0 ? bpp : AG_GetUint8(agConfig,"view.depth");
	agView->w = w > 0 ? w : AG_GetUint16(agConfig,"view.w");
	agView->h = h > 0 ? h : AG_GetUint16(agConfig,"view.h");

	if (AG_GetBool(agConfig,"view.full-screen"))
		screenflags |= SDL_FULLSCREEN;
	if (AG_GetBool(agConfig,"view.async-blits"))
		screenflags |= SDL_HWSURFACE|SDL_ASYNCBLIT;

	agView->depth = SDL_VideoModeOK(agView->w, agView->h, depth,
	    screenflags);
	if (agView->depth == 8)
		screenflags |= SDL_HWPALETTE;
	
#ifdef HAVE_OPENGL
	if (AG_GetBool(agConfig,"view.opengl")) {
		screenflags |= SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		agView->opengl = 1;
	}
#endif

	if (agView->w < AG_GetUint16(agConfig,"view.min-w") ||
	    agView->h < AG_GetUint16(agConfig,"view.min-h")) {
		AG_SetError(_("The resolution is too small."));
		goto fail;
	}

	/* Set the video mode. */
	agView->v = SDL_SetVideoMode(agView->w, agView->h, agView->depth,
	    screenflags);
	if (agView->v == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s",
		    agView->w, agView->h, agView->depth, SDL_GetError());
		goto fail;
	}
	agView->stmpl = AG_SurfaceRGBA(1,1, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
 	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	if (agView->stmpl == NULL) {
		AG_FatalError(NULL);
	}
	agVideoFmt = agView->v->format;
	agSurfaceFmt = agView->stmpl->format;

	Verbose(_("Video display is %dbpp (%08x,%08x,%08x)\n"),
	     (int)agVideoFmt->BitsPerPixel, 
	     (Uint)agVideoFmt->Rmask, (Uint)agVideoFmt->Gmask,
	     (Uint)agVideoFmt->Bmask);
	Verbose(_("Reference surface is %dbpp (%08x,%08x,%08x,%08x)\n"),
	     (int)agSurfaceFmt->BitsPerPixel,
	     (Uint)agSurfaceFmt->Rmask,
	     (Uint)agSurfaceFmt->Gmask,
	     (Uint)agSurfaceFmt->Bmask,
	     (Uint)agSurfaceFmt->Amask);

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		if (agVerbose) {
			Verbose("\tGL Version: %s\n", glGetString(GL_VERSION));
			Verbose("\tGL Vendor: %s\n", glGetString(GL_VENDOR));
			Verbose("\tGL Renderer: %s\n",
			    glGetString(GL_RENDERER));
		}
		InitGL();
	} else
#endif /* HAVE_OPENGL */
	{
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, agView->v->w, agView->v->h);
	}

	AG_SetUint8(agConfig, "view.depth", agView->depth);
	AG_SetUint16(agConfig, "view.w", agView->w);
	AG_SetUint16(agConfig, "view.h", agView->h);

	if (AG_InitGUI(0) == -1)
		goto fail;

	if (!(flags & AG_VIDEO_NOBGCLEAR)) {
		AG_ClearBackground();
	}
	return (0);
fail:
	Free(agView);
	agView = NULL;
	return (-1);
}

void
AG_DestroyVideo(void)
{
	AG_Window *win, *nwin;

	if (agView == NULL)
		return;
	
	for (win = TAILQ_FIRST(&agView->detach);
	     win != TAILQ_END(&agView->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);
		AG_ObjectDestroy(win);
	}
	for (win = TAILQ_FIRST(&agView->windows);
	     win != TAILQ_END(&agView->windows);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, windows);
		AG_ObjectDestroy(win);
	}

	AG_TextDestroy();

	AG_SurfaceFree(agView->stmpl);
	Free(agView->dirty);
	Free(agView->winModal);
	Free(agView);
	
	AG_ColorsDestroy();
	AG_CursorsDestroy();

	AG_ClearGlobalKeys();
	AG_MutexDestroy(&agGlobalKeysLock);
	
	agView = NULL;
}

void
AG_BindGlobalKey(SDLKey keysym, SDLMod keymod, void (*fn)(void))
{
	struct ag_global_key *gk;

	gk = Malloc(sizeof(struct ag_global_key));
	gk->keysym = keysym;
	gk->keymod = keymod;
	gk->fn = fn;
	gk->fn_ev = NULL;

	AG_MutexLock(&agGlobalKeysLock);
	SLIST_INSERT_HEAD(&agGlobalKeys, gk, gkeys);
	AG_MutexUnlock(&agGlobalKeysLock);
}

void
AG_BindGlobalKeyEv(SDLKey keysym, SDLMod keymod, void (*fn_ev)(AG_Event *))
{
	struct ag_global_key *gk;

	gk = Malloc(sizeof(struct ag_global_key));
	gk->keysym = keysym;
	gk->keymod = keymod;
	gk->fn = NULL;
	gk->fn_ev = fn_ev;
	
	AG_MutexLock(&agGlobalKeysLock);
	SLIST_INSERT_HEAD(&agGlobalKeys, gk, gkeys);
	AG_MutexUnlock(&agGlobalKeysLock);
}

int
AG_UnbindGlobalKey(SDLKey keysym, SDLMod keymod)
{
	struct ag_global_key *gk;

	AG_MutexLock(&agGlobalKeysLock);
	SLIST_FOREACH(gk, &agGlobalKeys, gkeys) {
		if (gk->keysym == keysym && gk->keymod == keymod) {
			SLIST_REMOVE(&agGlobalKeys, gk, ag_global_key, gkeys);
			AG_MutexUnlock(&agGlobalKeysLock);
			Free(gk);
			return (0);
		}
	}
	AG_MutexUnlock(&agGlobalKeysLock);
	AG_SetError(_("No such key binding"));
	return (-1);
}

void
AG_ClearGlobalKeys(void)
{
	struct ag_global_key *gk, *gkNext;

	AG_MutexLock(&agGlobalKeysLock);
	for (gk = SLIST_FIRST(&agGlobalKeys);
	     gk != SLIST_END(&agGlobalKeys);
	     gk = gkNext) {
		gkNext = SLIST_NEXT(gk, gkeys);
		Free(gk);
	}
	SLIST_INIT(&agGlobalKeys);
	AG_MutexUnlock(&agGlobalKeysLock);
}

#ifdef HAVE_OPENGL
static void
FreeWidgetResourcesGL(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		FreeWidgetResourcesGL(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}

static void
RegenWidgetResourcesGL(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		RegenWidgetResourcesGL(chld);
	}
	AG_WidgetRegenResourcesGL(wid);
}
#endif /* HAVE_OPENGL */

/*
 * Respond to a VIDEORESIZE event. Must be called from event context and
 * agView VFS must be locked.
 */
int
AG_ResizeDisplay(int w, int h)
{
	Uint32 flags = agView->v->flags & (SDL_SWSURFACE|SDL_FULLSCREEN|
	                                   SDL_HWSURFACE|SDL_ASYNCBLIT|
					   SDL_HWPALETTE|SDL_RESIZABLE|
					   SDL_OPENGL);
	AG_Window *win;
	SDL_Surface *su;
	int ow, oh;

#ifdef HAVE_OPENGL
	/*
	 * Until SDL implements GL context saving in a portable way, we have
	 * to release and regenerate all resources tied to our GL context.
	 */
	if (agView->opengl) {
		TAILQ_FOREACH(win, &agView->windows, windows)
			FreeWidgetResourcesGL(WIDGET(win));
	}
	AG_ClearGlyphCache();
#endif

	/* XXX set a minimum! */
	if ((su = SDL_SetVideoMode(w, h, 0, flags)) == NULL) {
		AG_SetError("Cannot resize display to %ux%u: %s",
		    w, h, SDL_GetError());
		goto fail;
	}
	ow = agView->w;
	oh = agView->h;

	agView->v = su;
	agView->w = w;
	agView->h = h;
	AG_SetUint16(agConfig, "view.w", w);
	AG_SetUint16(agConfig, "view.h", h);

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		InitGL();
		TAILQ_FOREACH(win, &agView->windows, windows)
			RegenWidgetResourcesGL(WIDGET(win));
	} else
#endif
	{
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, w, h);
	}

	TAILQ_FOREACH(win, &agView->windows, windows) {
		AG_SizeAlloc a;

		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDGET(win)->w;
		a.h = WIDGET(win)->h;

		AG_ObjectLock(win);

		if (win->flags & AG_WINDOW_MAXIMIZED) {
			AG_WindowSetGeometryMax(win);
		} else {
			if (win->flags & AG_WINDOW_HMAXIMIZE) {
				a.x = 0;
				a.w = agView->w;
			} else {
				if (a.x+a.w > agView->w) {
					a.x = agView->w - a.w;
					if (a.x < 0) {
						a.x = 0;
						a.w = agView->w;
					}
				}
			}
			if (win->flags & AG_WINDOW_VMAXIMIZE) {
				a.y = 0;
				a.h = agView->h;
			} else {
				if (a.y+a.h > agView->h) {
					a.y = agView->h - a.h;
					if (a.y < 0) {
						a.y = 0;
						a.h = agView->w;
					}
				}
			}
			AG_WidgetSizeAlloc(win, &a);
			AG_WindowUpdate(win);
		}
		AG_ObjectUnlock(win);
	}
	return (0);
fail:
	return (-1);
}

/*
 * Respond to a VIDEOEXPOSE event. Must be called from the main event/rendering
 * context and the View must be locked.
 */
void
AG_ViewVideoExpose(void)
{
	AG_Window *win;

	TAILQ_FOREACH(win, &agView->windows, windows) {
		AG_ObjectLock(win);
		if (win->visible) {
			AG_WidgetDraw(win);
		}
		AG_ObjectUnlock(win);
	}
#ifdef HAVE_OPENGL
	if (agView->opengl)
		SDL_GL_SwapBuffers();
#endif
}

/* Return the named window or NULL if there is no such window. */
AG_Window *
AG_FindWindow(const char *name)
{
	AG_Window *win;

	AG_LockVFS(agView);
	TAILQ_FOREACH(win, &agView->windows, windows) {
		if (strcmp(OBJECT(win)->name, name) == 0)
			break;
	}
	AG_UnlockVFS(agView);
	return (win);
}

/* Attach a window to a view. */
void
AG_ViewAttach(void *child)
{
	AG_Window *win = child;
	
	AG_LockVFS(agView);
	AG_ObjectAttach(agView, win);
	
	if (win->flags & AG_WINDOW_KEEPBELOW) {
		TAILQ_INSERT_HEAD(&agView->windows, win, windows);
	} else {
		TAILQ_INSERT_TAIL(&agView->windows, win, windows);
	}
	if (win->flags & AG_WINDOW_FOCUSONATTACH) {
		AG_WindowFocus(win);
	}
	AG_SetStyle(win, agView->style);
	AG_UnlockVFS(agView);
}

/*
 * Hide a window (and its children) and place them on a queue, to be
 * detached at the end of the current event processing cycle.
 */
void
AG_ViewDetach(AG_Window *win)
{
	AG_Window *subwin, *nsubwin;
	AG_Window *owin;

	AG_LockVFS(agView);

	for (subwin = TAILQ_FIRST(&win->subwins);
	     subwin != TAILQ_END(&win->subwins);
	     subwin = nsubwin) {
		nsubwin = TAILQ_NEXT(subwin, swins);
		AG_ViewDetach(subwin);
	}
	TAILQ_INIT(&win->subwins);
	
	AG_WindowHide(win);
	AG_PostEvent(agView, win, "detached", NULL);
	TAILQ_REMOVE(&agView->windows, win, windows);
	TAILQ_INSERT_TAIL(&agView->detach, win, detach);

	TAILQ_FOREACH(owin, &agView->windows, windows) {
		TAILQ_FOREACH(subwin, &owin->subwins, swins) {
			if (subwin == win)
				break;
		}
		if (subwin != NULL)
			TAILQ_REMOVE(&owin->subwins, subwin, swins);
	}
	AG_UnlockVFS(agView);
}

/*
 * Release the windows on the detachment queue. Called at the end of the
 * current event processing cycle.
 */
static __inline__ void
FreeDetachedWindows(void)
{
	AG_Window *win, *nwin;

	for (win = TAILQ_FIRST(&agView->detach);
	     win != TAILQ_END(&agView->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);
		AG_ObjectDetach(win);
		AG_ObjectDestroy(win);
	}
	TAILQ_INIT(&agView->detach);
}

/* Return a newly allocated surface containing a copy of ss. */
AG_Surface *
AG_DupSurface(AG_Surface *ss)
{
	AG_Surface *rs;

	rs = (AG_Surface *)SDL_ConvertSurface(ss, ss->format, SDL_SWSURFACE |
	    (ss->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCEL)));
	if (rs == NULL) {
		AG_SetError("SDL_ConvertSurface: %s", SDL_GetError());
		return (NULL);
	}
	rs->format->alpha = ss->format->alpha;
	rs->format->colorkey = ss->format->colorkey;
	return (rs);
}

/*
 * Allocate a new surface containing a pixmap of ss scaled to wxh.
 * The source surface must not be locked by the calling thread.
 *
 * XXX very primitive and inefficient
 */
int
AG_ScaleSurface(AG_Surface *ss, Uint16 w, Uint16 h, AG_Surface **ds)
{
	Uint8 r1, g1, b1, a1;
	Uint8 *pDst;
	int x, y;
	int same_fmt;

	if (*ds == NULL) {
		*ds = AG_SurfaceNew(w, h, ss->format,
		    ss->flags & (AG_SWSURFACE|AG_SRCALPHA|AG_SRCCOLORKEY));
		if (*ds == NULL) {
			return (-1);
		}
		(*ds)->format->alpha = ss->format->alpha;
		(*ds)->format->colorkey = ss->format->colorkey;
		same_fmt = 1;
	} else {
		//same_fmt = AG_SamePixelFmt(*ds, ss);
		same_fmt = 0;
	}

	if (ss->w == w && ss->h == h) {
		AG_SurfaceCopy(*ds, ss);
		return (0);
	}

	AG_SurfaceLock(ss);
	AG_SurfaceLock(*ds);
	pDst = (Uint8 *)(*ds)->pixels;
	for (y = 0; y < (*ds)->h; y++) {
		for (x = 0; x < (*ds)->w; x++) {
			Uint8 *pSrc = (Uint8 *)ss->pixels +
			    (y*ss->h/(*ds)->h)*ss->pitch +
			    (x*ss->w/(*ds)->w)*ss->format->BytesPerPixel;
			Uint32 cSrc = AG_GET_PIXEL(ss, pSrc);
			Uint32 cDst;

			if (same_fmt) {
				cDst = cSrc;
			} else {
				AG_GetRGBA(cSrc, ss->format,
				    &r1, &g1, &b1, &a1);
				cDst = AG_MapRGBA((*ds)->format,
				    r1, g1, b1, a1);
			}
			AG_PUT_PIXEL((*ds), pDst, cDst);
			pDst += (*ds)->format->BytesPerPixel;
		}
	}
	AG_SurfaceUnlock(*ds);
	AG_SurfaceUnlock(ss);
	return (0);
}

/* Set the alpha value of all pixels in a surface where a != 0. */
void
AG_SetAlphaPixels(AG_Surface *su, Uint8 alpha)
{
	int x, y;

	AG_SurfaceLock(su);
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint8 *dst = (Uint8 *)su->pixels +
			    y*su->pitch +
			    x*su->format->BytesPerPixel;
			Uint8 r, g, b, a;

			AG_GetRGBA(*(Uint32 *)dst, su->format, &r, &g, &b, &a);

			if (a != 0)
				a = alpha;

			AG_PUT_PIXEL(su, dst,
			    AG_MapRGBA(su->format, r, g, b, a));
		}
	}
	AG_SurfaceUnlock(su);
}

int
AG_SetRefreshRate(int fps)
{
	AG_LockVFS(agView);

	if (fps == -1) {
		Uint fpsNom;

		if (AG_GetProp(agConfig, "view.nominal-fps", AG_PROP_UINT,
		    &fpsNom) == NULL) {
			goto fail;
		}
		agView->rNom = 1000/fpsNom;
		agView->rCur = 0;
		goto out;
	}
	AG_SetUint(agConfig, "view.nominal-fps", fps);
	agView->rNom = 1000/fps;
	agView->rCur = 0;
out:
	AG_UnlockVFS(agView);
	return (0);
fail:
	AG_UnlockVFS(agView);
	return (0);
}

#ifdef HAVE_OPENGL

/*
 * Update the contents of an existing OpenGL texture from a given
 * surface. Must be called from rendering context.
 */
void
AG_UpdateTexture(AG_Surface *sourcesu, int texture)
{
	AG_Surface *texsu;
	int w, h;

	/*
	 * Convert to the GL_RGBA/GL_UNSIGNED_BYTE format and adjust for
	 * power-of-two dimensions.
	 * TODO check for GL_ARB_texture_non_power_of_two.
	 */
	w = PowOf2i(sourcesu->w);
	h = PowOf2i(sourcesu->h);
	texsu = AG_SurfaceRGBA(w,h, 32, 0,
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
	if (texsu == NULL) {
		AG_FatalError(NULL);
	}
	AG_SurfaceCopy(texsu, sourcesu);

	/* Upload as an OpenGL texture. */
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, texsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	AG_SurfaceFree(texsu);
}

/*
 * Generate an OpenGL texture from an Agar surface.
 * Returns the texture handle and 4 coordinates into texcoord.
 *
 * Must be called from widget rendering context only.
 */
Uint
AG_SurfaceTexture(AG_Surface *sourcesu, float *texcoord)
{
	AG_Surface *texsu;
	int w = PowOf2i(sourcesu->w);
	int h = PowOf2i(sourcesu->h);
	GLuint texture;

	/* Convert to the GL_RGBA/GL_UNSIGNED_BYTE format. */
	if (texcoord != NULL) {
		texcoord[0] = 0.0f;
		texcoord[1] = 0.0f;
		texcoord[2] = (GLfloat)sourcesu->w / w;
		texcoord[3] = (GLfloat)sourcesu->h / h;
	}
	texsu = AG_SurfaceRGBA(w,h, 32, 0,
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
	if (texsu == NULL) {
		AG_FatalError(NULL);
	}
	AG_SurfaceCopy(texsu, sourcesu);
	
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
	    GL_UNSIGNED_BYTE, texsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	AG_SurfaceFree(texsu);
	return (texture);
}

/*
 * Capture the contents of the OpenGL display into a surface. Must be
 * invoked in rendering context.
 */
AG_Surface *
AG_CaptureGLView(void)
{
	Uint8 *pixels;

	pixels = Malloc(agView->w*agView->h*3);
	glReadPixels(0, 0, agView->w, agView->h, GL_RGB, GL_UNSIGNED_BYTE,
	    pixels);
	AG_FlipSurface(pixels, agView->h, agView->w*3);
	return AG_SurfaceFromPixelsRGBA(pixels, agView->w, agView->h, 0, 24,
	    agView->w*3, 0x000000ff, 0x0000ff00, 0x00ff0000);
}
#endif /* HAVE_OPENGL */

/*
 * Blend the specified components with the pixel at s:[x,y], using the
 * given alpha function.
 *
 * Clipping is not done; the destination surface must be locked.
 */
void
AG_BlendPixelRGBA(AG_Surface *s, Uint8 *pDst, Uint8 sR, Uint8 sG, Uint8 sB,
    Uint8 sA, AG_BlendFn func)
{
	Uint32 cDst;
	Uint8 dR, dG, dB, dA;
	int alpha = 0;

	cDst = AG_GET_PIXEL(s, pDst);
	if ((s->flags & AG_SRCCOLORKEY) && (cDst == s->format->colorkey)) {
	 	AG_PUT_PIXEL(s, pDst, AG_MapRGBA(s->format, sR,sG,sB,sA));
	} else {
		AG_GetRGBA(cDst, s->format, &dR, &dG, &dB, &dA);
		switch (func) {
		case AG_ALPHA_OVERLAY:	alpha = dA+sA; break;
		case AG_ALPHA_SRC:	alpha = sA; break;
		case AG_ALPHA_DST:	alpha = dA; break;
		case AG_ALPHA_ONE_MINUS_DST: alpha = 1-dA; break;
		case AG_ALPHA_ONE_MINUS_SRC: alpha = 1-sA; break;
		}
		alpha = (alpha < 0) ? 0 : (alpha > 255) ? 255 : alpha;
		AG_PUT_PIXEL(s, pDst, AG_MapRGBA(s->format,
		    (((sR - dR) * sA) >> 8) + dR,
		    (((sG - dG) * sA) >> 8) + dG,
		    (((sB - dB) * sA) >> 8) + dB,
		    (Uint8)alpha));
	}
}

void
AG_ViewCapture(void)
{
	char path[MAXPATHLEN];
	
	if (AG_DumpSurface(agView->v, path) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Screenshot saved to %s."),
		    path);
	} else {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
}

/* Dump a surface to a JPEG image. */
int
AG_DumpSurface(AG_Surface *pSu, char *path_save)
{
#ifdef HAVE_JPEG
	AG_Surface *su;
	char path[MAXPATHLEN];
	struct jpeg_error_mgr jerrmgr;
	struct jpeg_compress_struct jcomp;
	Uint8 *jcopybuf;
	FILE *fp;
	Uint seq = 0;
	int fd;
	JSAMPROW row[1];
	int x;

	AG_GetStringCopy(agConfig, "save-path", path, sizeof(path));
	Strlcat(path, AG_PATHSEP, sizeof(path));
	Strlcat(path, "screenshot", sizeof(path));
	if (AG_MkDir(path) == -1 && errno != EEXIST) {
		AG_SetError("mkdir %s: %s", path, strerror(errno));
		return (-1);
	}

	if (!agView->opengl || pSu != agView->v) {
		su = pSu;
	} else {
#ifdef HAVE_OPENGL
		su = AG_CaptureGLView();
#else
		su = NULL;
#endif
	}

	for (;;) {
		char file[MAXPATHLEN];

		Snprintf(file, sizeof(file), "%s/%s%u.jpg", path, agProgName,
		    seq++);
		if ((fd = open(file, O_WRONLY|O_CREAT|O_EXCL, 0600)) == -1) {
			if (errno == EEXIST) {
				continue;
			} else {
				AG_SetError("%s: %s", file, strerror(errno));
				goto out;
			}
		}
		break;
	}
	if (path_save != NULL)
		Strlcpy(path_save, path, MAXPATHLEN);

	if ((fp = fdopen(fd, "wb")) == NULL) {
		AG_SetError("fdopen: %s", strerror(errno));
		return (-1);
	}

	jcomp.err = jpeg_std_error(&jerrmgr);

	jpeg_create_compress(&jcomp);

	jcomp.image_width = su->w;
	jcomp.image_height = su->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, agScreenshotQuality, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(su->w*3);

	jpeg_start_compress(&jcomp, TRUE);
	while (jcomp.next_scanline < jcomp.image_height) {
		Uint8 *pSrc = (Uint8 *)su->pixels +
		    jcomp.next_scanline*su->pitch;
		Uint8 *pDst = jcopybuf;
		Uint8 r, g, b;

		for (x = agView->w; x > 0; x--) {
			AG_GetRGB(AG_GET_PIXEL(su,pSrc), su->format, &r,&g,&b);
			*pDst++ = r;
			*pDst++ = g;
			*pDst++ = b;
			pSrc += su->format->BytesPerPixel;
		}
		row[0] = jcopybuf;
		jpeg_write_scanlines(&jcomp, row, 1);
	}
	jpeg_finish_compress(&jcomp);
	jpeg_destroy_compress(&jcomp);

	fclose(fp);
	close(fd);
	Free(jcopybuf);
out:
#ifdef HAVE_OPENGL
	if (agView->opengl && su != pSu)
		AG_SurfaceFree(su);
#endif
	return (0);
#else
	AG_SetError(_("Screenshot feature requires libjpeg"));
	return (-1);
#endif
}

/* Flip the lines of a frame buffer; useful with glReadPixels(). */
void
AG_FlipSurface(Uint8 *src, int h, int pitch)
{
	Uint8 *tmp = Malloc(pitch);
	int h2 = h >> 1;
	Uint8 *p1 = &src[0];
	Uint8 *p2 = &src[(h-1)*pitch];
	int i;

	for (i = 0; i < h2; i++) {
		memcpy(tmp, p1, pitch);
		memcpy(p1, p2, pitch);
		memcpy(p2, tmp, pitch);

		p1 += pitch;
		p2 -= pitch;
	}
	Free(tmp);
}

/*
 * Obtain the hue/saturation/value of a given RGB triplet.
 * Note that the hue is lost as saturation approaches 0.
 */
void
AG_RGB2HSV(Uint8 r, Uint8 g, Uint8 b, float *h, float *s, float *v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/255.0F;
	vG = (float)g/255.0F;
	vB = (float)b/255.0F;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {
		/* This is a gray color (zero hue, no saturation). */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0F + deltaMax/2.0F) / deltaMax;
		deltaG = ((vMax - vG)/6.0F + deltaMax/2.0F) / deltaMax;
		deltaB = ((vMax - vB)/6.0F + deltaMax/2.0F) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0F;
		} else if (vG == vMax) {
			*h = 120.0F + (deltaR - deltaB)*360.0F;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0F + (deltaG - deltaR)*360.0F;	/* 2/3 */
		}

		if (*h < 0.0F)		(*h)++;
		if (*h > 360.0F)	(*h)--;
	}
}

/* Convert hue/saturation/value to RGB. */
void
AG_HSV2RGB(float h, float s, float v, Uint8 *r, Uint8 *g, Uint8 *b)
{
	float var[3];
	float vR, vG, vB, hv;
	int iv;

	if (s == 0.0) {
		*r = (Uint8)v*255;
		*g = (Uint8)v*255;
		*b = (Uint8)v*255;
		return;
	}
	
	hv = h/60.0F;
	iv = Floor(hv);
	var[0] = v * (1.0F - s);
	var[1] = v * (1.0F - s*(hv - iv));
	var[2] = v * (1.0F - s*(1.0F - (hv - iv)));

	switch (iv) {
	case 0:		vR = v;		vG = var[2];	vB = var[0];	break;
	case 1:		vR = var[1];	vG = v;		vB = var[0];	break;
	case 2:		vR = var[0];	vG = v;		vB = var[2];	break;
	case 3:		vR = var[0];	vG = var[1];	vB = v;		break;
	case 4:		vR = var[2];	vG = var[0];	vB = v;		break;
	default:	vR = v;		vG = var[0];	vB = var[1];	break;
	}
	
	*r = vR*255;
	*g = vG*255;
	*b = vB*255;
}

#ifdef DEBUG
/*
 * Update the performance counters.
 * XXX remove this once the graph widget implements polling.
 */
static __inline__ void
PerfMonitorUpdate(void)
{
	static int einc = 0;

	AG_FixedPlotterDatum(agPerfFPS, agView->rCur);
	AG_FixedPlotterDatum(agPerfEvnts, agEventAvg * 30 / 10);
	AG_FixedPlotterDatum(agPerfIdle, agIdleAvg);
	AG_FixedPlotterScroll(agPerfGraph, 1);

	if (++einc == 1) {
		agEventAvg = 0;
		einc = 0;
	}
}

AG_Window *
AG_EventShowPerfGraph(void)
{
	AG_WindowShow(agPerfWindow);
	return (agPerfWindow);
}

static void
PerfMonitorInit(void)
{
	AG_Label *lbl;

	agPerfWindow = AG_WindowNewNamed(0, "event-fps-counter");
	AG_WindowSetCaption(agPerfWindow, _("Performance counters"));
	AG_WindowSetPosition(agPerfWindow, AG_WINDOW_LOWER_CENTER, 0);
	lbl = AG_LabelNewPolled(agPerfWindow, AG_LABEL_HFILL,
	    "%dms (nom %dms), %d evnt, %dms idle",
	    &agView->rCur, &agView->rNom, &agEventAvg, &agIdleAvg);
	AG_LabelSizeHint(lbl, 1, "000ms (nom 000ms), 00 evnt, 000ms idle");
	agPerfGraph = AG_FixedPlotterNew(agPerfWindow, AG_FIXED_PLOTTER_LINES,
	                                               AG_FIXED_PLOTTER_XAXIS|
						       AG_FIXED_PLOTTER_EXPAND);
	agPerfFPS = AG_FixedPlotterCurve(agPerfGraph, "refresh", 0,160,0, 99);
	agPerfEvnts = AG_FixedPlotterCurve(agPerfGraph, "event", 0,0,180, 99);
	agPerfIdle = AG_FixedPlotterCurve(agPerfGraph, "idle", 180,180,180, 99);
}
#endif /* DEBUG */

/*
 * Try to ensure a fixed frame rate, and idle as much as possible.
 * TODO provide MD hooks for finer idling.
 */
void
AG_EventLoop_FixedFPS(void)
{
	SDL_Event ev;
	AG_Window *win;
	Uint32 Tr1, Tr2 = 0;

#ifdef DEBUG
	PerfMonitorInit();
#endif
	Tr1 = SDL_GetTicks();
	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) {
			AG_LockVFS(agView);
#ifdef HAVE_OPENGL
			if (agView->opengl)
				glClear(GL_COLOR_BUFFER_BIT|
				        GL_DEPTH_BUFFER_BIT);
#endif
			TAILQ_FOREACH(win, &agView->windows, windows) {
				AG_ObjectLock(win);
				if (!win->visible) {
					AG_ObjectUnlock(win);
					continue;
				}
				AG_WidgetDraw(win);
				if (!(win->flags & AG_WINDOW_NOUPDATERECT)) {
					AG_QueueVideoUpdate(
					    WIDGET(win)->x, WIDGET(win)->y,
					    WIDGET(win)->w, WIDGET(win)->h);
				}
				AG_ObjectUnlock(win);
			}
			if (agView->ndirty > 0) {
#ifdef HAVE_OPENGL
				if (agView->opengl) {
					SDL_GL_SwapBuffers();
				} else
#endif
				{
					SDL_UpdateRects(agView->v,
					    agView->ndirty,
					    agView->dirty);
				}
				agView->ndirty = 0;
			}
			AG_UnlockVFS(agView);

			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			agView->rCur = agView->rNom - (Tr1-Tr2);
#ifdef DEBUG
			if (agPerfWindow->visible)
				PerfMonitorUpdate();
#endif
			if (agView->rCur < 1) {
				agView->rCur = 1;
			}
		} else if (SDL_PollEvent(&ev) != 0) {
			AG_ProcessEvent(&ev);
#ifdef DEBUG
			agEventAvg++;
#endif
		} else if (AG_TIMEOUTS_QUEUED()) {		/* Safe */
			AG_ProcessTimeouts(Tr2);
		} else if (agView->rCur > agIdleThresh) {
			SDL_Delay(agView->rCur - agIdleThresh);
#ifdef DEBUG
			agIdleAvg = SDL_GetTicks() - Tr2;
		} else {
			agIdleAvg = 0;
		}
#else
		}
#endif
	}
}

static void
UnminimizeWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_WindowUnminimize(win);
}

void
AG_ProcessEvent(SDL_Event *ev)
{
	AG_LockVFS(agView);

	switch (ev->type) {
	case SDL_MOUSEMOTION:
#ifdef OPENGL_INVERTED_Y
		if (agView->opengl) {
			ev->motion.y = agView->h - ev->motion.y;
			ev->motion.yrel = -ev->motion.yrel;
		}
#endif
		AG_WindowEvent(ev);
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
#ifdef OPENGL_INVERTED_Y
		if (agView->opengl)
			ev->button.y = agView->h - ev->button.y;
#endif
		if (AG_WindowEvent(ev) == 0 &&
		    agBgPopupMenu && ev->type == SDL_MOUSEBUTTONDOWN &&
		    (ev->button.button == SDL_BUTTON_MIDDLE ||
		     ev->button.button == SDL_BUTTON_RIGHT)) {
			AG_Menu *me;
			AG_MenuItem *mi;
			AG_Window *win;
			int x, y;

			me = AG_MenuNew(NULL, 0);
			mi = me->itemSel = AG_MenuAddItem(me, NULL);

			TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq,
			    windows) {
				if (strcmp(win->caption, "win-popup")
				    == 0) {
					continue;
				}
				AG_MenuAction(mi, win->caption, NULL,
				    UnminimizeWindow, "%p", win);
			}
				
			AG_MouseGetState(&x, &y);
			AG_MenuExpand(me, mi, x+4, y+4);
		}
		break;
	case SDL_KEYDOWN:
#if 0
		fprintf(stderr, "[DOWN] Key=%d(%c), Mod=%d\n",
		    (int)ev->key.keysym.sym,
		    (char)ev->key.keysym.sym,
		    (int)ev->key.keysym.mod);
#endif
		{
			struct ag_global_key *gk;

			AG_MutexLock(&agGlobalKeysLock);
			SLIST_FOREACH(gk, &agGlobalKeys, gkeys) {
				if (gk->keysym == ev->key.keysym.sym &&
				    ((gk->keymod == KMOD_NONE &&
				      ev->key.keysym.mod == KMOD_NONE) ||
				      ev->key.keysym.mod & gk->keymod)) {
					if (gk->fn != NULL) {
						gk->fn();
					} else if (gk->fn_ev != NULL) {
						gk->fn_ev(NULL);
					}
				}
			}
			AG_MutexUnlock(&agGlobalKeysLock);
		}
		/* FALLTHROUGH */
	case SDL_KEYUP:
#if 0
		fprintf(stderr, "[  UP] Key=%d(%c), Mod=%d\n",
		    (int)ev->key.keysym.sym,
		    (char)ev->key.keysym.sym,
		    (int)ev->key.keysym.mod);
#endif
		AG_WindowEvent(ev);
		break;
	case SDL_JOYAXISMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		AG_WindowEvent(ev);
		break;
	case SDL_VIDEORESIZE:
		AG_ResizeDisplay(ev->resize.w, ev->resize.h);
		break;
	case SDL_VIDEOEXPOSE:
		AG_ViewVideoExpose();
		break;
	case SDL_QUIT:
#if 0
		if (!agTerminating &&
		    AG_FindEventHandler(agWorld, "quit") != NULL) {
			AG_PostEvent(NULL, agWorld, "quit", NULL);
			break;
		}
#endif
		/* FALLTHROUGH */
	case SDL_USEREVENT:
		AG_UnlockVFS(agView);
		agTerminating = 1;
		AG_Destroy();
		/* NOTREACHED */
		break;
	}
	FreeDetachedWindows();
	AG_UnlockVFS(agView);
}

Uint8
AG_MouseGetState(int *x, int *y)
{
	Uint8 rv;

	rv = SDL_GetMouseState(x, y);
#ifdef OPENGL_INVERTED_Y
	if (agView->opengl && y != NULL)
		*y = agView->h - *y;
#endif
	return (rv);
}

#define COMPUTE_SHIFTLOSS(mask, shift, loss) \
	shift = 0; \
	loss = 8; \
	if (mask != 0) { \
		for (m = mask ; (m & 0x01) == 0; m >>= 1) { \
			shift++; \
		} \
		while ((m & 0x01) != 0) { \
			loss--; \
			m >>= 1; \
		} \
	}

/* Specify a packed-pixel format from three 32-bit bitmasks. */
AG_PixelFormat *
AG_PixelFormatRGB(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	AG_PixelFormat *pf;
	Uint32 m;

	pf = Malloc(sizeof(AG_PixelFormat));
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->palette = NULL;
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = 0x00000000;
	COMPUTE_SHIFTLOSS(pf->Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTLOSS(pf->Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTLOSS(pf->Bmask, pf->Bshift, pf->Bloss);
	return (pf);
}

/* Specify a packed-pixel format from four 32-bit bitmasks. */
AG_PixelFormat *
AG_PixelFormatRGBA(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask,
    Uint32 Amask)
{
	AG_PixelFormat *pf;
	Uint32 m;

	pf = Malloc(sizeof(AG_PixelFormat));
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->palette = NULL;
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = Amask;
	COMPUTE_SHIFTLOSS(pf->Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTLOSS(pf->Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTLOSS(pf->Bmask, pf->Bshift, pf->Bloss);
	COMPUTE_SHIFTLOSS(pf->Amask, pf->Ashift, pf->Aloss);
	return (pf);
}

/*
 * Specify an indexed pixel format. If bpp=2, the palette is initialized to
 * [0 = white] and [1 = black], otherwise the palette is initialized to all
 * black.
 */
AG_PixelFormat *
AG_PixelFormatIndexed(int bpp)
{
	AG_PixelFormat *pf;
	AG_Palette *pal;

	pf = Malloc(sizeof(AG_PixelFormat));
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	
	pal = pf->palette = Malloc(sizeof(AG_Palette));
	pal->ncolors = 1<<bpp;
	pal->colors = Malloc(pal->ncolors*sizeof(AG_Color));
	
	if (bpp == 2) {
		pal->colors[0].r = 255;
		pal->colors[0].g = 255;
		pal->colors[0].b = 255;
		pal->colors[1].r = 0;
		pal->colors[1].g = 0;
		pal->colors[1].b = 0;
	} else {
		memset(pal->colors, 0, pal->ncolors*sizeof(AG_Color));
	}

	pf->Rmask = pf->Gmask = pf->Bmask = pf->Amask = 0;
	pf->Rloss = pf->Gloss = pf->Bloss = pf->Aloss = 8;
	pf->Rshift = pf->Gshift = pf->Bshift = pf->Ashift = 0;
	return (pf);
}

AG_PixelFormat *
AG_PixelFormatDup(const AG_PixelFormat *pf)
{
	AG_PixelFormat *pfd;

	pfd = Malloc(sizeof(AG_PixelFormat));
	if (pf->palette != NULL) {
		pfd->palette = Malloc(pf->palette->ncolors*sizeof(AG_Color));
		memcpy(pfd->palette->colors, pf->palette->colors,
		    pf->palette->ncolors*sizeof(AG_Color));
	} else {
		pfd->palette = NULL;
	}
	pfd->BitsPerPixel = pf->BitsPerPixel;
	pfd->BytesPerPixel = pf->BytesPerPixel;
	pfd->Rloss = pf->Rloss;
	pfd->Gloss = pf->Gloss;
	pfd->Bloss = pf->Bloss;
	pfd->Aloss = pf->Aloss;
	pfd->Rshift = pf->Rshift;
	pfd->Gshift = pf->Gshift;
	pfd->Bshift = pf->Bshift;
	pfd->Ashift = pf->Ashift;
	pfd->Rmask = pf->Rmask;
	pfd->Gmask = pf->Gmask;
	pfd->Bmask = pf->Bmask;
	pfd->Amask = pf->Amask;
	pfd->colorkey = pf->colorkey;
	pfd->alpha = pf->alpha;
	return (pfd);
}

void
AG_PixelFormatFree(AG_PixelFormat *fmt)
{
	Free(fmt->palette);
	free(fmt);
}

#undef COMPUTE_SHIFTLOSS

static __inline__ Uint32
SDLFlags(Uint flags)
{
	Uint32 sdlFlags = SDL_SWSURFACE;
	if (flags & AG_HWSURFACE) { sdlFlags |= SDL_HWSURFACE; }
	if (flags & AG_SRCCOLORKEY) { sdlFlags |= SDL_SRCCOLORKEY; }
	if (flags & AG_SRCALPHA) { sdlFlags |= SDL_SRCALPHA; }
	return (sdlFlags);
}

/* Create a new surface of the specified pixel format. */
AG_Surface *
AG_SurfaceNew(Uint w, Uint h, AG_PixelFormat *fmt, Uint flags)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w, h,
	    fmt->BitsPerPixel,
	    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h,
		    fmt->BitsPerPixel, SDL_GetError());
		return (NULL);
	}
	if (fmt->palette != NULL) {
		SDL_SetPalette((SDL_Surface *)s, SDL_LOGPAL,
		    (SDL_Color *)fmt->palette, 0, fmt->palette->ncolors);
	}
	return (s);
}

/* Create an empty surface. */
AG_Surface *
AG_SurfaceEmpty(void)
{
	return (AG_Surface *)SDL_CreateRGBSurface(SDL_SWSURFACE, 0,0,8,0,0,0,0);
}

/* Create a new surface of the specified pixel format. */
AG_Surface *
AG_SurfaceIndexed(Uint w, Uint h, int bpp, Uint flags)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w,h, bpp,
	    0,0,0,0);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h, bpp,
		    SDL_GetError());
		return (NULL);
	}
	return (s);
}

/* Create a new surface with the specified RGB pixel-packing format. */
AG_Surface *
AG_SurfaceRGB(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w,h, bpp,
	    Rmask,Gmask,Bmask,0);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h, bpp,
		    SDL_GetError());
		return (NULL);
	}
	return (s);
}

/* Create a new surface with the specified RGB pixel-packing format. */
AG_Surface *
AG_SurfaceRGBA(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask, Uint32 Amask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w,h, bpp,
	    Rmask,Gmask,Bmask,Amask);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h, bpp,
		    SDL_GetError());
		return (NULL);
	}
	return (s);
}

/* Create a new surface from pixel data in the specified packed RGB format. */
AG_Surface *
AG_SurfaceFromPixelsRGB(void *pixels, Uint w, Uint h, int bpp, int pitch,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurfaceFrom(pixels, (int)w, (int)h,
	    bpp, pitch, Rmask, Gmask, Bmask, 0);
	return (s);
}

/* Create a new surface from pixel data in the specified packed RGBA format. */
AG_Surface *
AG_SurfaceFromPixelsRGBA(void *pixels, Uint w, Uint h, int bpp, int pitch,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurfaceFrom(pixels, (int)w, (int)h,
	    bpp, pitch, Rmask, Gmask, Bmask, Amask);
	return (s);
}

/* Create a new surface from a .bmp file. */
AG_Surface *
AG_SurfaceFromBMP(const char *path)
{
	AG_Surface *s;

	if ((s = (AG_Surface *)SDL_LoadBMP(path)) == NULL) {
		AG_SetError("SDL_LoadBMP(%s): %s", path, SDL_GetError());
		return (NULL);
	}
	return (s);
}

AG_Surface *
AG_SurfaceFromSDL(AG_Surface *su)
{
	return AG_DupSurface(su);
}

SDL_Surface *
AG_SurfaceToSDL(AG_Surface *su)
{
	return (SDL_Surface *)AG_DupSurface(su);
}

AG_Surface *
AG_SurfaceFromSurface(AG_Surface *su, AG_PixelFormat *fmt, Uint flags)
{
	return (AG_Surface *)SDL_ConvertSurface(su, fmt, SDLFlags(flags));
}

void
AG_SurfaceCopy(AG_Surface *ds, AG_Surface *ss)
{
	SDL_Surface *dsSDL = (SDL_Surface *)ds;
	SDL_Surface *ssSDL = (SDL_Surface *)ss;
	Uint32 aflagsSave = ssSDL->flags & (AG_SRCALPHA|AG_RLEACCEL);
	Uint8 alphaSave = ssSDL->format->alpha;
	Uint32 cflagsSave = ssSDL->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
	Uint32 ckeySave = ssSDL->format->colorkey;

	SDL_SetAlpha(ssSDL, 0, 0);
	SDL_SetColorKey(ssSDL, 0, 0);
	SDL_BlitSurface(ssSDL, NULL, dsSDL, NULL);
	SDL_SetAlpha(ssSDL, aflagsSave, alphaSave);
	SDL_SetColorKey(ssSDL, cflagsSave, ckeySave);
}

/* Free the specified surface. */
void
AG_SurfaceFree(AG_Surface *s)
{
	SDL_FreeSurface((SDL_Surface *)s);
}

AG_ObjectClass agDisplayClass = {
	"AG_Display",
	sizeof(AG_Display),
	{ 0,0 },
	NULL,	/* init */
	NULL,	/* free */
	NULL,	/* destroy */
	NULL,	/* load */
	NULL,	/* save */
	NULL	/* edit */
};

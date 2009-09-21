/*
 * Copyright (c) 2001-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "opengl.h"

#include <core/core.h>
#include <core/config.h>

#include "window.h"
#include "primitive.h"
#include "cursors.h"
#include "colors.h"
#include "menu.h"
#include "text.h"
#include "gui_math.h"
#include "icons.h"

#ifdef AG_DEBUG
#include "perfmon.h"
#endif

/*
 * Invert the Y-coordinate in OpenGL mode.
 */
/* #define OPENGL_INVERTED_Y */

AG_Display          *agView = NULL;	/* Main view */
AG_PixelFormat      *agVideoFmt = NULL;	/* Current format of display */
AG_PixelFormat      *agSurfaceFmt = NULL; /* Preferred format for surfaces */

int agBgPopupMenu = 0;			/* Background popup menu */
int agRenderingContext = 0;		/* In rendering context (for debug) */

AG_ClipRect *agClipRects = NULL;	/* Clipping rectangle stack (first
					   entry always covers whole view) */
int          agClipStateGL[4];		/* Saved GL clipping plane states */
Uint        agClipRectCount = 0;

static void (*agVideoResizeCallback)(Uint w, Uint h) = NULL;

int agFullscreenMode = 0;
int agAsyncBlits = 0;

#ifdef HAVE_OPENGL
static void
InitGL(void)
{
	Uint8 bR, bG, bB;

	glViewport(0, 0, agView->w, agView->h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, agView->w, agView->h, 0, -1.0, 1.0);

	if (!agView->overlay) {
		AG_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &bR, &bG, &bB);
		glClearColor(bR/255.0, bG/255.0, bB/255.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
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

/* Allocate an Agar view (AG_View(3)) context.  */
static AG_Display *
AllocView(void)
{
	AG_Display *v;

	if ((v = AG_TryMalloc(sizeof(AG_Display))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(v, &agDisplayClass);
	AG_ObjectSetName(v, "_agView");
	v->winop = AG_WINOP_NONE;
	v->ndirty = 0;
	v->maxdirty = 4;
	v->opengl = 0;
	if ((v->dirty = AG_TryMalloc(v->maxdirty*sizeof(SDL_Rect))) == NULL) {
		goto fail;
	}
	if ((v->Lmodal = AG_ListNew()) == NULL) {
		free(v->dirty);
		goto fail;
	}
	v->winSelected = NULL;
	v->winToFocus = NULL;
	v->rNom = 16;
	v->rCur = 0;
	v->overlay = 0;
	TAILQ_INIT(&v->detach);
	return (v);
fail:
	free(v);
	return (NULL);
}

/*
 * Allocate the standard "reference" surfaces which Agar will use as
 * template when generating new surfaces.
 */
static int
AllocReferenceSurfaces(AG_Display *v)
{
	v->stmpl = AG_SurfaceRGBA(1,1, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
 	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	if (v->stmpl == NULL) {
		return (-1);
	}
	agVideoFmt = agView->v->format;
	agSurfaceFmt = agView->stmpl->format;
	return (0);
}

/* Destroy a View structure including all attached windows and widgets. */
static void
DestroyView(AG_Display *v)
{
	if (v->Lmodal != NULL)
		AG_ListDestroy(v->Lmodal);
	if (v->stmpl != NULL)
		AG_SurfaceFree(v->stmpl);
	if (v->dirty != NULL)
		Free(v->dirty);

	AG_ObjectDestroy(v);
}

/* Initialize the clipping rectangle stack. */
static int
InitClipRects(int wView, int hView)
{
	AG_ClipRect *cr;
	int i;

	for (i = 0; i < 4; i++)
		agClipStateGL[i] = 0;

	/* Rectangle 0 always covers the whole view. */
	if ((agClipRects = AG_TryMalloc(sizeof(AG_ClipRect))) == NULL)
		return (-1);

	cr = &agClipRects[0];
	cr->r = AG_RECT(0, 0, wView, hView);

	cr->eqns[0][0] = 1.0;
	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;
	cr->eqns[0][3] = 0.0;
	
	cr->eqns[1][0] = 0.0;
	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;
	cr->eqns[1][3] = 0.0;
	
	cr->eqns[2][0] = -1.0;
	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;
	cr->eqns[2][3] = (double)wView;
	
	cr->eqns[3][0] = 0.0;
	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;
	cr->eqns[3][3] = (double)hView;
	
	agClipRectCount = 1;
	return (0);
}

/*
 * Initialize Agar's graphics facilities. If no graphics driver is specified,
 * try to select the "best" option for the current platform.
 */
int
AG_InitGraphics(const char *driver)
{
	AG_InitGuiGlobals();

	if (AG_InitGUI(0) == -1)
		return (-1);
	
	return (0);
}

/*
 * Initialize Agar with an existing SDL/OpenGL display surface. If the
 * surface has SDL_OPENGL or SDL_OPENGLBLIT set, we assume that OpenGL
 * primitives are to be used in rendering.
 */
int
AG_InitVideoSDL(SDL_Surface *display, Uint flags)
{
	AG_InitGuiGlobals();

	agInitedSDL = 0;

	if (display->w < AG_GetUint16(agConfig,"view.min-w") ||
	    display->h < AG_GetUint16(agConfig,"view.min-h")) {
		AG_SetError(_("The resolution is too small."));
		return (-1);
	}

	/* Allocate the Agar view context. */
	if ((agView = AllocView()) == NULL) {
		return (-1);
	}
	agView->v = display;
	agView->w = display->w;
	agView->h = display->h;
	agView->depth = display->format->BitsPerPixel;
	agView->rNom = 1000/AG_GetUint(agConfig, "view.nominal-fps");

	/* Set the requested parameters; pull defaults from agConfig. */
	AG_SetUint8(agConfig, "view.depth", agView->depth);
	AG_SetUint16(agConfig, "view.w", agView->w);
	AG_SetUint16(agConfig, "view.h", agView->h);
	AG_SetBool(agConfig, "view.full-screen", display->flags&SDL_FULLSCREEN);
	AG_SetBool(agConfig, "view.async-blits", display->flags&SDL_ASYNCBLIT);
	if (flags & AG_VIDEO_OVERLAY) { agView->overlay = 1; }
	if (flags & AG_VIDEO_BGPOPUPMENU) { agBgPopupMenu = 1; }

	/* Enable OpenGL mode if the surface has SDL_OPENGL set. */
	if (display->flags & (SDL_OPENGL|SDL_OPENGLBLIT)) {
		if (flags & AG_VIDEO_SDL) {
			AG_SetError("AG_VIDEO_SDL flag requested, but "
			            "display surface has SDL_OPENGL set");
			goto fail;
		}
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

	/* Allocate the standard reference surfaces. */
	if (AllocReferenceSurfaces(agView) == -1)
		goto fail;
#ifdef HAVE_OPENGL
	if (agView->opengl)
		InitGL();
#endif
	if (AG_InitGUI(0) == -1 ||
	    InitClipRects(agView->w, agView->h) == -1) {
		goto fail;
	}
	if (!(flags & AG_VIDEO_NOBGCLEAR) && !agView->overlay) {
		AG_ClearBackground();
	}
	return (0);
fail:
	DestroyView(agView);
	agView = NULL;
	return (-1);
}

/* Initialize Agar with a new video display. */
int
AG_InitVideo(int w, int h, int bpp, Uint flags)
{
	Uint32 screenflags = 0;
	int depth;

	AG_InitGuiGlobals();

	if (SDL_Init(0) == -1) {
		AG_SetError("SDL_Init() failed: %s", SDL_GetError());
		return (-1);
	}
	agInitedSDL = 1;

	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			AG_SetError("SDL_INIT_VIDEO failed: %s", SDL_GetError());
			return (-1);
		}
		agInitedSDLVideo = 1;
	}

	SDL_WM_SetCaption(agProgName, agProgName);

	if (flags & (AG_VIDEO_OPENGL|AG_VIDEO_OPENGL_OR_SDL)) {
#ifdef HAVE_OPENGL
		AG_SetBool(agConfig, "view.opengl", 1);
#else
		if (!(flags & AG_VIDEO_OPENGL_OR_SDL)) {
			AG_SetError("AG_VIDEO_OPENGL requested but OpenGL "
			            "support is not compiled in");
			goto fail;
		}
#endif
	} else if (flags & AG_VIDEO_SDL) {
		AG_SetBool(agConfig, "view.opengl", 0);
	} else {
		AG_SetBool(agConfig, "view.opengl", 0);
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

	/* Allocate the Agar view context. */
	if ((agView = AllocView()) == NULL) {
		goto fail;
	}
	if (flags & AG_VIDEO_OVERLAY) { agView->overlay = 1; }
	if (flags & AG_VIDEO_BGPOPUPMENU) { agBgPopupMenu = 1; }

	/* Set the requested parameters; pull defaults from agConfig. */
	agView->rNom = 1000/AG_GetUint(agConfig,"view.nominal-fps");
	depth = bpp > 0 ? bpp : AG_GetUint8(agConfig,"view.depth");
	agView->w = w > 0 ? w : AG_GetUint16(agConfig,"view.w");
	agView->h = h > 0 ? h : AG_GetUint16(agConfig,"view.h");
	if (AG_GetBool(agConfig,"view.full-screen"))
		screenflags |= SDL_FULLSCREEN;
	if (AG_GetBool(agConfig,"view.async-blits"))
		screenflags |= SDL_HWSURFACE|SDL_ASYNCBLIT;

	/*
	 * Set the video mode.
	 */
	agView->depth = SDL_VideoModeOK(agView->w, agView->h, depth,
	    screenflags);
	if (agView->depth == 8) {
		Verbose(_("Enabling hardware palette"));
		screenflags |= SDL_HWPALETTE;
	}
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
	agView->v = SDL_SetVideoMode(agView->w, agView->h, agView->depth,
	    screenflags);
	if (agView->v == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s",
		    agView->w, agView->h, agView->depth, SDL_GetError());
		goto fail;
	}

	/* Allocate the standard reference surfaces. */
	if (AllocReferenceSurfaces(agView) == -1)
		goto fail;

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

	if (AG_InitGUI(0) == -1 ||
	    InitClipRects(w, h) == -1)
		goto fail;

	if (!(flags & AG_VIDEO_NOBGCLEAR) && !agView->overlay) {
		AG_ClearBackground();
	}
	return (0);
fail:
	if (agView != NULL) {
		DestroyView(agView);
		agView = NULL;
	}
	if (agInitedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		agInitedSDLVideo = 0;
	}
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
	TAILQ_INIT(&agView->detach);

	AG_TextDestroy();
	
	DestroyView(agView);
	agView = NULL;
	
	AG_ColorsDestroy();
	AG_CursorsDestroy();
	AG_ClearGlobalKeys();

	AG_DestroyGuiGlobals();
}

#ifdef HAVE_OPENGL
/* Save GL resources associated with a widget and its children. */
static void
FreeWidgetResourcesGL(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		FreeWidgetResourcesGL(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}

/* Restore GL resources associated with a widget and its children. */
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
	AG_ClipRect *cr0;

#ifdef HAVE_OPENGL
	/*
	 * Save all of our GL resources since it is not portable to assume
	 * that a display resize will not destroy them.
	 */
	if (agView->opengl) {
		VIEW_FOREACH_WINDOW(win, agView)
			FreeWidgetResourcesGL(WIDGET(win));
	}
	AG_ClearGlyphCache();
#endif

	/* Resize the display to the requested geometry. */
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

	/* Update clipping rectangle 0. */
	cr0 = &agClipRects[0];
	cr0->r.w = w;
	cr0->r.h = h;
	cr0->eqns[2][3] = (double)w;
	cr0->eqns[3][3] = (double)h;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		/* Restore our saved GL resources. */
		InitGL();
		VIEW_FOREACH_WINDOW(win, agView)
			RegenWidgetResourcesGL(WIDGET(win));
	} else
#endif
	{
		/* Clear the background. */
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, w, h);
	}

	/* Update the Agar window geometries. */
	VIEW_FOREACH_WINDOW(win, agView) {
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
	if (agVideoResizeCallback != NULL) {
		agVideoResizeCallback(w, h);
	}
	return (0);
fail:
	return (-1);
}

void
AG_SetVideoResizeCallback(void (*fn)(Uint w, Uint h))
{
	agVideoResizeCallback = fn;
}

/* Enter rendering context. */
void
AG_BeginRendering(void)
{
#ifdef AG_DEBUG
	agRenderingContext = 1;
#endif
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		if (!agView->overlay) {
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		} else {
			glPushAttrib(GL_VIEWPORT_BIT|GL_TRANSFORM_BIT|
			             GL_LIGHTING_BIT|GL_ENABLE_BIT);
			InitGL();
		}
		agClipStateGL[0] = glIsEnabled(GL_CLIP_PLANE0);
		glEnable(GL_CLIP_PLANE0);
		agClipStateGL[1] = glIsEnabled(GL_CLIP_PLANE1);
		glEnable(GL_CLIP_PLANE1);
		agClipStateGL[2] = glIsEnabled(GL_CLIP_PLANE2);
		glEnable(GL_CLIP_PLANE2);
		agClipStateGL[3] = glIsEnabled(GL_CLIP_PLANE3);
		glEnable(GL_CLIP_PLANE3);
	}
#endif
}

/* Leave rendering context. */
void
AG_EndRendering(void)
{
#ifdef AG_DEBUG
	if (agClipRectCount != 1)
		AG_FatalError("Inconsistent PushClipRect() / PopClipRect()");
#endif

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		if (!agView->overlay) {
			SDL_GL_SwapBuffers();
			if (agClipStateGL[0])	{ glEnable(GL_CLIP_PLANE0); }
			else			{ glDisable(GL_CLIP_PLANE0); }
			if (agClipStateGL[1])	{ glEnable(GL_CLIP_PLANE1); }
			else			{ glDisable(GL_CLIP_PLANE1); }
			if (agClipStateGL[2])	{ glEnable(GL_CLIP_PLANE2); }
			else			{ glDisable(GL_CLIP_PLANE2); }
			if (agClipStateGL[3])	{ glEnable(GL_CLIP_PLANE3); }
			else			{ glDisable(GL_CLIP_PLANE3); }
		} else {
			glPopAttrib();
		}
	} else
#endif
	if (agView->ndirty > 0) {
		SDL_UpdateRects(agView->v, agView->ndirty, agView->dirty);
		agView->ndirty = 0;
	}
#ifdef AG_DEBUG
	agRenderingContext = 0;
#endif
}

/* Respond to a VIDEOEXPOSE event by redrawing the windows. */
void
AG_ViewVideoExpose(void)
{
	AG_Window *win;

	AG_LockVFS(agView);
	AG_BeginRendering();
	VIEW_FOREACH_WINDOW(win, agView) {
		AG_ObjectLock(win);
		AG_WindowDraw(win);
		AG_ObjectUnlock(win);
	}
	AG_EndRendering();
	AG_UnlockVFS(agView);
}

/* Queue a video region for update (for FB graphics modes). */
void
AG_ViewUpdateFB(const AG_Rect2 *rp)
{
	AG_Rect2 r;
	int n;

	if (agView->opengl)
		return;

	/* XXX TODO use IntersectRect() against agView */
	r = *rp;
	if (r.x1 < 0) { r.x1 = 0; }
	if (r.y1 < 0) { r.y1 = 0; }
	if (r.x2 > agView->w) { r.x2 = agView->w; r.w = r.x2-r.x1; }
	if (r.y2 > agView->h) { r.y2 = agView->h; r.h = r.y2-r.y1; }
	if (r.w < 0) { r.x1 = 0; r.x2 = r.w = agView->w; }
	if (r.h < 0) { r.y1 = 0; r.y2 = r.h = agView->h; }

	n = agView->ndirty++;
	if (n+1 > agView->maxdirty) {
		agView->maxdirty *= 2;
		agView->dirty = AG_Realloc(agView->dirty, agView->maxdirty *
		                                          sizeof(SDL_Rect));
	}
	agView->dirty[n].x = r.x1;
	agView->dirty[n].y = r.y1;
	agView->dirty[n].w = r.w;
	agView->dirty[n].h = r.h;
}

/*
 * Release the windows on the detachment queue. Called at the end of the
 * current event processing cycle.
 */
static __inline__ void
FreeDetachedWindows(AG_Display *view)
{
	AG_Window *win, *nwin;

	for (win = TAILQ_FIRST(&view->detach);
	     win != TAILQ_END(&view->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);
		AG_ObjectSetDetachFn(win, NULL, NULL);	/* Actually detach */
		AG_ObjectDetach(win);
		AG_ObjectDestroy(win);
	}
	TAILQ_INIT(&view->detach);
}

int
AG_SetRefreshRate(int fps)
{
	AG_LockVFS(agView);

	if (fps == -1) {
		agView->rNom = 1000/AG_GetUint(agConfig,"view.nominal-fps");
		agView->rCur = 0;
		goto out;
	}
	AG_SetUint(agConfig, "view.nominal-fps", fps);
	agView->rNom = 1000/fps;
	agView->rCur = 0;
out:
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

/*
 * Generate an OpenGL texture from an Agar surface.
 * Returns the texture handle and 4 coordinates into texcoord.
 *
 * Must be called from widget rendering context only.
 */
Uint
AG_SurfaceTexture(AG_Surface *suSrc, float *texcoord)
{
	AG_Surface *suTex;
	int w = PowOf2i(suSrc->w);
	int h = PowOf2i(suSrc->h);
	GLuint texture;

	/* Convert to the GL_RGBA/GL_UNSIGNED_BYTE format. */
	if (texcoord != NULL) {
		texcoord[0] = 0.0f;
		texcoord[1] = 0.0f;
		texcoord[2] = (GLfloat)suSrc->w / w;
		texcoord[3] = (GLfloat)suSrc->h / h;
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
		AG_FatalError(NULL);
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
	return AG_SurfaceFromPixelsRGB(pixels, agView->w, agView->h, 24,
	    agView->w*3, 0x000000ff, 0x0000ff00, 0x00ff0000);
}
#endif /* HAVE_OPENGL */

void
AG_ViewCapture(void)
{
	char path[AG_PATHNAME_MAX];
	
	if (AG_DumpSurface(agView->v, path) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Screenshot saved to %s."),
		    path);
	} else {
		AG_TextMsgFromError();
	}
}

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

#ifdef AG_DEBUG
	AG_PerfMonInit();
#endif
	Tr1 = SDL_GetTicks();
	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) {
			AG_LockVFS(agView);
			AG_BeginRendering();
			VIEW_FOREACH_WINDOW(win, agView) {
				AG_ObjectLock(win);
				AG_WindowDraw(win);
				AG_ObjectUnlock(win);
			}
			AG_EndRendering();
			AG_UnlockVFS(agView);

			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			agView->rCur = agView->rNom - (Tr1-Tr2);
#ifdef AG_DEBUG
			if (agPerfWindow->visible)
				AG_PerfMonUpdate();
#endif
			if (agView->rCur < 1) {
				agView->rCur = 1;
			}
		} else if (SDL_PollEvent(&ev) != 0) {
			if (AG_ProcessEvent(&ev) == -1)
				return;
#ifdef AG_DEBUG
			agEventAvg++;
#endif
		} else if (AG_TIMEOUTS_QUEUED()) {		/* Safe */
			AG_ProcessTimeouts(Tr2);
		} else if (agView->rCur > agIdleThresh) {
			SDL_Delay(agView->rCur - agIdleThresh);
#ifdef AG_DEBUG
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

#ifdef AG_DEBUG
static void
OpenGuiDebugger(AG_Event *event)
{
	AG_Window *win;
	if ((win = AG_GuiDebugger()) != NULL)
		AG_WindowShow(win);
}
#endif /* AG_DEBUG */

static void
ExitApplication(AG_Event *event)
{
	AG_Quit();
}

/* Display the generic background popup menu. */
static void
BackgroundPopupMenu(void)
{
	AG_Menu *me;
	AG_MenuItem *mi;
	AG_Window *win;
	int x, y;
	int nWindows = 0;

	me = AG_MenuNew(NULL, 0);
	mi = me->itemSel = AG_MenuAddItem(me, NULL);

	VIEW_FOREACH_WINDOW_REVERSE(win, agView) {
		if (strcmp(win->caption, "win-popup") == 0) {
			continue;
		}
		AG_MenuAction(mi,
		    win->caption[0] != '\0' ? win->caption : _("Untitled"),
		    agIconWinMaximize.s,
		    UnminimizeWindow, "%p", win);
		nWindows++;
	}
	if (nWindows > 0) {
		AG_MenuSeparator(mi);
	}
#ifdef AG_DEBUG
	AG_MenuAction(mi, _("GUI debugger"), agIconMagnifier.s,
	    OpenGuiDebugger, NULL);
#endif
	AG_MenuAction(mi, _("Exit application"), agIconWinClose.s,
	    ExitApplication, NULL);
				
	AG_MouseGetState(&x, &y);
	AG_MenuExpand(me, mi, x+4, y+4);
}

/*
 * Process an SDL event. Returns 1 if the event was processed in some
 * way, -1 if application is exiting.
 */
int
AG_ProcessEvent(SDL_Event *ev)
{
	int rv = 0;

	AG_LockVFS(agView);

	switch (ev->type) {
	case SDL_MOUSEMOTION:
#ifdef OPENGL_INVERTED_Y
		if (agView->opengl) {
			ev->motion.y = agView->h - ev->motion.y;
			ev->motion.yrel = -ev->motion.yrel;
		}
#endif
		rv = AG_WindowEvent(ev);
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
			BackgroundPopupMenu();
			rv = 1;
		}
		break;
	case SDL_KEYDOWN:
		rv = AG_ExecGlobalKeys(ev->key.keysym.sym, ev->key.keysym.mod);
		/* FALLTHROUGH */
	case SDL_KEYUP:
		rv = AG_WindowEvent(ev);
		break;
	case SDL_JOYAXISMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		rv = AG_WindowEvent(ev);
		break;
	case SDL_VIDEORESIZE:
		AG_ResizeDisplay(ev->resize.w, ev->resize.h);
		rv = 1;
		break;
	case SDL_VIDEOEXPOSE:
		AG_ViewVideoExpose();
		rv = 1;
		break;
	case SDL_QUIT:
		/* FALLTHROUGH */
	case SDL_USEREVENT:
		AG_UnlockVFS(agView);
		agTerminating = 1;
		return (-1);
	}
	FreeDetachedWindows(agView);
	AG_UnlockVFS(agView);
	return (rv);
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

#ifdef AG_LEGACY
/* Pre-1.4 */
void
AG_ViewAttach(void *pWin)
{
	AG_ObjectAttach(agView, pWin);
}
void
AG_ViewDetach(AG_Window *win)
{
	AG_ObjectDetach(win);
}
#endif /* AG_LEGACY */

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

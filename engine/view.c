/*	$Csoft: view.c,v 1.81 2002/11/15 01:59:52 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include "engine.h"

#include "rootmap.h"
#include "map.h"
#include "physics.h"
#include "config.h"
#include "view.h"

#include "widget/widget.h"
#include "widget/window.h"

static const struct object_ops viewport_ops = {
	view_destroy,
	NULL,		/* load */
	NULL		/* save */
};

/* Read-only as long as the engine is running. */
struct viewport *view;

/* Scaled surface cache. */
struct cached_surface {
	SDL_Surface	*source_s;	/* Original surface */
	SDL_Surface	*scaled_s;	/* Scaled surface */
	size_t		 size;		/* Size of scaled surface in bytes */
	int		 nrefs;		/* Reference count */

	TAILQ_ENTRY(cached_surface) surfaces;
};
static TAILQ_HEAD(, cached_surface) cached_surfaces;
static pthread_mutex_t cached_surfaces_lock = PTHREAD_MUTEX_INITIALIZER;

static void	free_cached_surface(struct cached_surface *);

/* Initialize the graphic engine. */
void
view_init(gfx_engine_t ge)
{
	struct viewport *v;
	int screenflags = SDL_HWSURFACE;
	int bpp, w, h, mw, mh;

	bpp = prop_uint32(config, "view.bpp");
	w = prop_uint32(config, "view.w");
	h = prop_uint32(config, "view.h");
	mw = w / TILEW;
	mh = h / TILEH;

	if (prop_uint32(config, "flags") & CONFIG_FULLSCREEN)
		screenflags |= SDL_FULLSCREEN;
	if (prop_uint32(config, "flags") & CONFIG_ASYNCBLIT)
		screenflags |= SDL_ASYNCBLIT;

	v = emalloc(sizeof(struct viewport));
	object_init(&v->obj, "viewport", "main-view", NULL, 0, &viewport_ops);
	v->gfx_engine = ge;
	v->bpp = SDL_VideoModeOK(w, h, bpp, screenflags);
	v->w = prop_uint32(config, "view.w");
	v->h = prop_uint32(config, "view.h");
	if (v->gfx_engine == GFX_ENGINE_TILEBASED) {
		v->w -= v->w % TILEW;
		v->h -= v->h % TILEH;
	}
	if (v->w < 640 || v->h < 480) {
		fatal("minimum resolution is 640x480\n");
	}
	v->rootmap = NULL;
	v->winop = VIEW_WINOP_NONE;
	TAILQ_INIT(&v->windows);
	TAILQ_INIT(&v->detach);

	/* Dirty rectangle array */
	v->maxdirty = v->w/TILEW * v->h/TILEH;
	v->ndirty = 0;
	v->dirty = emalloc(v->maxdirty * sizeof(SDL_Rect *));

	pthread_mutexattr_init(&v->lockattr);
	pthread_mutexattr_settype(&v->lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&v->lock, &v->lockattr);

	switch (v->bpp) {
	case 8:
		dprintf("exclusive palette access\n");
		screenflags |= SDL_HWPALETTE;
		break;
	}

#if 0
	switch (v->gfx_engine) {
	case GFX_ENGINE_GUI:
		screenflags |= SDL_RESIZABLE;
		break;
	default:
	}
#endif

	v->v = SDL_SetVideoMode(v->w, v->h, 0, screenflags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", v->w, v->h, v->bpp,
		    SDL_GetError());
	}

	switch (v->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		v->rootmap = emalloc(sizeof(struct viewmap));
		v->rootmap->w = mw - 1;
		v->rootmap->h = mh - 1;

		v->rootmap->map = NULL;
		v->rootmap->x = 0;
		v->rootmap->y = 0;
		
		/*
		 * Precalculate node rectangles as well as an array
		 * able to hold all possible rectangles in a view,
		 * for optimization purposes.
		 */
		v->rootmap->maprects = rootmap_alloc_maprects(mw, mh);

		dprintf("precalculated %dx%d rectangles (%d Kb)\n",
		    mw, mh, (mw*mh * sizeof(SDL_Rect)) / 1024);
		break;
	case GFX_ENGINE_GUI:
		break;
	}
	view = v;
}

/*
 * Process all windows on the detach queue. This is executed after
 * window list traversal by the event loop.
 *
 * View must be locked. The detach queue must not be empty.
 */
void
view_detach_queued(void)
{
	struct window *win, *nwin;

	for (win = TAILQ_FIRST(&view->detach);
	     win != TAILQ_END(&view->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);

		TAILQ_REMOVE(&view->windows, win, windows);
		window_hide(win);
		event_post(win, "detached", "%p", view);
		object_destroy(win);
	}
	TAILQ_INIT(&view->detach);
}

void
view_destroy(void *p)
{
	struct viewport *v = p;
	struct window *win;
	
	pthread_mutex_lock(&v->lock);
	
	/* Free precalculated map rectangles. */
	if (v->rootmap != NULL) {
		rootmap_free_maprects(v);
		free(v->rootmap);
		v->rootmap = NULL;
	}

	/* Free the windows. */
	TAILQ_FOREACH(win, &v->windows, windows) {
		win->flags &= ~(WINDOW_MATERIALIZE|WINDOW_DEMATERIALIZE);
		view_detach(win);
	}
	if (!TAILQ_EMPTY(&view->detach)) {
		view_detach_queued();
	}

	/* Free the dirty rectangle array. */
	free(v->dirty);

	pthread_mutex_unlock(&v->lock);

	pthread_mutex_destroy(&v->lock);
	pthread_mutexattr_destroy(&v->lockattr);
}

/* Attach a window to a view. */
void
view_attach(void *child)
{
	struct window *win = child;
	
	pthread_mutex_lock(&view->lock);

	view->focus_win = NULL;

	OBJECT_ASSERT(child, "window");
	event_post(child, "attached", "%p", view);
	TAILQ_INSERT_TAIL(&view->windows, win, windows);
	
	pthread_mutex_unlock(&view->lock);
}

/* Detach a window from a view. */
void
view_detach(void *child)
{
	struct window *win = child;
	
	OBJECT_ASSERT(child, "window");

	/*
	 * Allow windows to detach themselves only after event processing
	 * is complete.
	 */
	pthread_mutex_lock(&view->lock);
	TAILQ_INSERT_HEAD(&view->detach, win, detach);
	pthread_mutex_unlock(&view->lock);
}

SDL_Surface *
view_surface(int flags, int w, int h)
{
	SDL_Surface *s;
	Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	s = SDL_CreateRGBSurface(flags, w, h, 32, rmask, gmask, bmask, amask);
	if (s == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
	}
	return (s);
}

/*
 * Allocate a new surface containing a bitmap of ss scaled to wxh.
 * The source surface must not be locked by the caller thread.
 */
SDL_Surface *
view_scale_surface(SDL_Surface *ss, Uint16 w, Uint16 h)
{
	SDL_Surface *ds;
	Uint32 col =0;
	Uint8 *src, *dst, r1, g1, b1, a1;
	struct cached_surface *cs;
	int x, y;

	pthread_mutex_lock(&cached_surfaces_lock);

	/* XXX use a hash table */
	TAILQ_FOREACH(cs, &cached_surfaces, surfaces) {
		if (cs->source_s == ss &&
		    cs->scaled_s->w == w && cs->scaled_s->h == h) {
			cs->nrefs++;
			pthread_mutex_unlock(&cached_surfaces_lock);
			return (cs->scaled_s);
		}
	}

	ds = view_surface(SDL_HWSURFACE, w, h);

	if (SDL_MUSTLOCK(ss))
		SDL_LockSurface(ss);
	if (SDL_MUSTLOCK(ds))
		SDL_LockSurface(ds);

	for (y = 0; y < ds->h; y++) {
		for (x = 0; x < ds->w; x++) {
			src = (Uint8 *)ss->pixels +
			    (y * ss->h / ds->h) * ss->pitch +
			    (x * ss->w / ds->w) * ss->format->BytesPerPixel;
			dst = (Uint8 *)ds->pixels +
			    y * ds->pitch +
			    x * ds->format->BytesPerPixel;

			switch (ss->format->BytesPerPixel) {
			case 1:
				SDL_GetRGBA(*src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 2:
				SDL_GetRGBA(*(Uint16 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 3:
				SDL_GetRGBA(*(Uint16 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 4:
				SDL_GetRGBA(*(Uint32 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			}

			/* Transparency hack for text surfaces. */
			if (r1 == 15 && g1 == 15 && b1 == 15) {
				a1 = 0;
			}

			col = SDL_MapRGBA(ds->format, r1, g1, b1, a1);
			switch (ds->format->BytesPerPixel) {
				_VIEW_PUTPIXEL_8(dst, col)
				_VIEW_PUTPIXEL_16(dst, col)
				_VIEW_PUTPIXEL_24(dst, col)
				_VIEW_PUTPIXEL_32(dst, col)
			}
		}
	}

	if (SDL_MUSTLOCK(ds))
		SDL_UnlockSurface(ds);
	if (SDL_MUSTLOCK(ss))
		SDL_UnlockSurface(ss);

	/* Read-allocate */
	cs = emalloc(sizeof(struct cached_surface));
	cs->source_s = ss;
	cs->scaled_s = ds;
	cs->size = (ds->w * ds->h) * ds->format->BytesPerPixel;
	cs->nrefs = 0;
	TAILQ_INSERT_HEAD(&cached_surfaces, cs, surfaces);
	
	pthread_mutex_unlock(&cached_surfaces_lock);
	return (ds);
}

void
view_unused_surface(SDL_Surface *scaled)
{
	struct cached_surface *cs;

	pthread_mutex_lock(&cached_surfaces_lock);
	TAILQ_FOREACH(cs, &cached_surfaces, surfaces) {
		if (cs->scaled_s == scaled) {
			cs->nrefs--;
		}
	}
	pthread_mutex_unlock(&cached_surfaces_lock);
}

void
view_invalidate_surface(SDL_Surface *scaled)
{
	struct cached_surface *cs;

	pthread_mutex_lock(&cached_surfaces_lock);
	TAILQ_FOREACH(cs, &cached_surfaces, surfaces) {
		if (cs->scaled_s == scaled) {
			/* Free the surface immediately. */
			dprintf("surface %p\n", cs->scaled_s);
			SDL_FreeSurface(cs->scaled_s);
			TAILQ_REMOVE(&cached_surfaces, cs, surfaces);
			free(cs);

			pthread_mutex_unlock(&cached_surfaces_lock);
			return;
		}
	}
	fatal("not in surface cache: %p\n", scaled);
}

/* Focus on a window. */
void
view_focus(struct window *win)
{
	/*
	 * The window at the tail of the list holds focus (drawn last,
	 * but receives events first).
	 */
	pthread_mutex_lock(&view->lock);
	TAILQ_REMOVE(&view->windows, win, windows);
	TAILQ_INSERT_TAIL(&view->windows, win, windows);
	pthread_mutex_unlock(&view->lock);
}


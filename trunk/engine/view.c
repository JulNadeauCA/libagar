/*	$Csoft: view.c,v 1.52 2002/07/06 23:49:43 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "engine.h"
#include "map.h"
#include "physics.h"
#include "config.h"

#include "widget/window.h"

static const struct object_ops viewport_ops = {
	view_destroy,
	NULL,		/* load */
	NULL		/* save */
};

/* Read-only as long as the engine is running. */
struct viewport *view;

static int	**view_allocmask(int, int);
static SDL_Rect	**view_allocmaprects(int, int);
static SDL_Rect	 *view_allocrects(int, int);
static void	  view_freemask(struct viewport *);
static void	  view_freemaprects(struct viewport *);

/* Allocate a mask of the given size. */
static int **
view_allocmask(int w, int h)
{
	int **mask, y, x = 0;

	mask = emalloc((w * h) * sizeof(int *));
	for (y = 0; y < h; y++) {
		*(mask + y) = emalloc(w * sizeof(int));
	}
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			mask[y][x] = 0;
		}
	}
	return (mask);
}

/* Precalculate rectangles. */
static SDL_Rect **
view_allocmaprects(int w, int h)
{
	SDL_Rect **rects;
	int x, y;

	rects = emalloc((w * h) * sizeof(SDL_Rect *));
	for (y = 0; y < h; y++) {
		*(rects + y) = emalloc(w * sizeof(SDL_Rect));
		for (x = 0; x < w; x++) {
			rects[y][x].x = x * TILEW;
			rects[y][x].y = y * TILEH;
			rects[y][x].w = TILEW;
			rects[y][x].h = TILEH;
		}
	}
	return (rects);
}

/*
 * Allocate an array able to hold wxh rectangles,
 * for optimization purposes.
 */
static SDL_Rect *
view_allocrects(int w, int h)
{
	SDL_Rect *rects;
	size_t len;
	
	len = (w * h) * sizeof(SDL_Rect *);
	rects = emalloc(len);
	memset(rects, (int)NULL, len);
	return (rects);
}

/*
 * Increment mask nodes matching rd.
 * View must be locked.
 */
void
view_maskfill(SDL_Rect *rd, int n)
{
	int x, y;

#ifdef DEBUG
	if (view->rootmap == NULL) {
		fatal("NULL rootmap\n");
	}
#endif

	for (y = rd->y; y < rd->y + rd->h; y++) {
		for (x = rd->x; x < rd->x + rd->w; x++) {
			view->rootmap->mask[y][x] += n;
		}
	}
}

/*
 * View must be locked.
 */
static void
view_freemask(struct viewport *v)
{
	int y;

	for (y = 0; y < v->rootmap->h; y++) {
		free(*(v->rootmap->mask + y));
	}
	free(v->rootmap->mask);
	v->rootmap->mask = NULL;
}

/*
 * Free a map rectangle array.
 * View must be locked.
 */
static void
view_freemaprects(struct viewport *v)
{
	int y;

	for (y = 0; y < v->rootmap->h; y++) {
		free(*(v->rootmap->maprects + y));
	}
	free(v->rootmap->maprects);
	v->rootmap->maprects = NULL;
}

void
view_init(gfx_engine_t ge)
{
	struct viewport *v;
	int screenflags = SDL_SWSURFACE;
	int bpp = config->view.bpp;
	int w = config->view.w, mw = w / TILEW;
	int h = config->view.h, mh = h / TILEH;

	if (config->flags & CONFIG_FULLSCREEN) {
		dprintf("full-screen mode\n");
		screenflags |= SDL_FULLSCREEN;
	}
	if (config->flags & CONFIG_ASYNCBLIT) {
		dprintf("async blit\n");
		screenflags |= SDL_ASYNCBLIT;
	}

	v = emalloc(sizeof(struct viewport));
	object_init(&v->obj, "viewport", "main-view", NULL, 0, &viewport_ops);
	v->gfx_engine = ge;
	v->bpp = SDL_VideoModeOK(w, h, bpp, screenflags);
	v->w = config->view.w;
	v->h = config->view.h;
	v->rootmap = NULL;
	v->winop = VIEW_WINOP_NONE;
	v->wop_mapx = 0;
	v->wop_mapy = 0;
	TAILQ_INIT(&v->windowsh);
	pthread_mutex_init(&v->lock, NULL);
	
	switch (v->bpp) {
	case 8:
		screenflags |= SDL_HWPALETTE;
		break;
	}

	v->v = SDL_SetVideoMode(v->w, v->h, v->bpp, screenflags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", v->w, v->h, v->bpp,
		    SDL_GetError());
	}

	switch (ge) {
	case GFX_ENGINE_TILEBASED:
		v->rootmap = emalloc(sizeof(struct viewmap));
		v->rootmap->w = mw - 1;
		v->rootmap->h = mh - 1;

		v->rootmap->map = NULL;
		v->rootmap->x = 0;
		v->rootmap->y = 0;
		
		dprintf("GFX_ENGINE_TILEBASED: %dx%d map view\n", mw, mh);
	
		/*
		 * Allocate view masks, precalculate node rectangles, and
		 * preallocate an array able to hold all possible rectangles
		 * in a view, for optimization purposes.
		 */
		v->rootmap->mask = view_allocmask(mw, mh);
		v->rootmap->maprects = view_allocmaprects(mw, mh);
		v->rootmap->rects = view_allocrects(mw, mh);
	
		SDL_WM_SetCaption("AGAR (tile-based)", "AGAR");
		SDL_ShowCursor(SDL_DISABLE);
		SDL_Delay(100);
		break;
	case GFX_ENGINE_GUI:
		dprintf("GFX_ENGINE_GUI\n");
		SDL_WM_SetCaption("AGAR (GUI)", "AGAR");
		SDL_ShowCursor(SDL_ENABLE);
		break;
	}

	view = v;
}

void
view_destroy(void *p)
{
	struct viewport *v = p;

	if (v->rootmap != NULL) {
		view_freemask(v);
		view_freemaprects(v);
		free(v->rootmap->rects);
	}
}

/*
 * Center the view. Only relevant to game mode.
 * Map must be locked.
 */
void
view_center(struct map *m, int mapx, int mapy)
{
	struct viewport *v = view;
	struct viewmap *rm = v->rootmap;
	int nx, ny;

	nx = mapx - (rm->w / 2);
	ny = mapy - (rm->h / 2);

	if (nx < 0)
		nx = 0;
	if (ny < 0)
		ny = 0;
	if (nx >= (m->mapw - rm->w))
		nx = (m->mapw - rm->w);
	if (ny >= (m->maph - rm->h))
		ny = (m->maph - rm->h);

	rm->x = nx;
	rm->y = ny;
}

/*
 * Scroll the view. Only relevant to game mode.
 * XXX later
 * View must be locked.
 */
void
view_scroll(struct map *m, int dir)
{
	struct viewmap *rm = view->rootmap;

	if (view->gfx_engine != GFX_ENGINE_TILEBASED) {
		dprintf("only relevant to tile mode\n");
		return;
	}

	switch (dir) {
	case DIR_UP:
		if (--rm->y < 0) {
			rm->y = 0;
		}
		break;
	case DIR_LEFT:
		if (--rm->x < 0) {
			rm->x = 0;
		}
		break;
	case DIR_DOWN:
		if (++rm->y > (m->maph - rm->h)) {
			rm->y = (m->maph - rm->h);
		}
		break;
	case DIR_RIGHT:
		if (++rm->x > (m->mapw - rm->w)) {
			rm->x = (m->mapw - rm->w);
		}
		break;
	}
	m->redraw++;
}

/*
 * Attach a window to a view.
 * View must be locked.
 */
void
view_attach(void *child)
{
	struct window *win = child;

	OBJECT_ASSERT(child, "window");
	event_post(child, "attached", "%p", view);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
}

/*
 * Detach a window from a view.
 * View muts be locked.
 */
void
view_detach(void *child)
{
	struct window *win = child;

	OBJECT_ASSERT(child, "window");
	event_post(child, "detached", "%p", view);
	TAILQ_REMOVE(&view->windowsh, win, windows);
}

/* Create a surface with native-endian masks. */
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
 * Focus on a window.
 * View and window must be locked.
 */
void
view_focus(struct window *win)
{
	dprintf("on %s\n", OBJECT(win)->name);

	TAILQ_REMOVE(&view->windowsh, win, windows);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
	
	win->redraw++;
}


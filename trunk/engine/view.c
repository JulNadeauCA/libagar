/*	$Csoft: view.c,v 1.42 2002/05/25 08:56:48 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/mapedit/mapedit.h>

#include <engine/widget/window.h>

static const struct object_ops viewport_ops = {
	view_destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* onattach */
	NULL,		/* ondetach */
	view_attach,
	view_detach
};

static int	**view_allocmask(int, int);
static SDL_Rect	**view_allocmaprects(struct map *, int, int);
static SDL_Rect	 *view_allocrects(struct map *, int, int);
static void	  view_freemask(struct viewport *);
static void	  view_freemaprects(struct viewport *);

/* Read-only as long as the engine is running. */
struct viewport *mainview;

/*
 * Start displaying the given map inside the given view.
 * Map/view must be locked.
 */
int
view_setmode(struct viewport *v, struct map *m, int mode, char *caption)
{
	if (mode > -1) {
		v->mode = mode;
	}

	v->fps = 30;	/* XXX pref */

	switch (v->mode) {
	case VIEW_MAPNAV:
		dprintf("map navigation mode\n");
		v->map = m;
		v->mapw = (v->w / m->tilew);
		v->maph = (v->h / m->tileh);
		v->mapxoffs = 0;
		v->mapyoffs = 0;
		v->vmapw = v->mapw - v->mapxoffs;
		v->vmaph = v->maph - v->mapyoffs;
		break;
	case VIEW_MAPEDIT:
		dprintf("map edition mode\n");
		v->map = m;
		v->mapw = (v->w / m->tilew);
		v->maph = (v->h / m->tileh);
		v->mapxoffs = 1;
		v->mapyoffs = 1;
		if (v->map->mapw < v->mapw)
			v->mapxoffs += (v->mapw - v->map->mapw) / 2;
		if (v->map->maph < v->maph)
			v->mapyoffs += (v->maph - v->map->maph) / 2;
		v->vmapw = v->mapw - v->mapxoffs - 1;	/* Tile list */
		v->vmaph = v->maph - v->mapyoffs;
		break;
	default:
		fatal("bad mode\n");
	}
	
	dprintf("view %dx%d, map view %dx%d at %d,%d\n", v->w, v->h,
	    v->mapxoffs, v->mapyoffs, v->mapw, v->maph);

	switch (v->depth) {
	case 8:
		v->flags |= SDL_HWPALETTE;
		break;
	}

	if (caption != NULL) {
		SDL_WM_SetCaption(caption, "AGAR");
	}
	v->v = SDL_SetVideoMode(v->w, v->h, v->depth, v->flags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", v->w, v->h, v->depth,
		    SDL_GetError());
		return (-1);
	}

	/*
	 * Allocate view masks, precalculate node rectangles, and
	 * preallocate an array able to hold all possible rectangles
	 * in a view, for optimization purposes.
	 */
	if (v->mapmask == NULL) {
		v->mapmask = view_allocmask(v->mapw, v->maph);
	}
	if (v->maprects == NULL) {
		v->maprects = view_allocmaprects(m, v->mapw, v->maph);
	}
	if (v->rects == NULL) {
		v->rects = view_allocrects(m, v->mapw, v->maph);
	}

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Delay(100);
	return (0);
}

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

/*
 * Precalculate rectangles.
 * Map/view must be locked.
 */
static SDL_Rect **
view_allocmaprects(struct map *m, int w, int h)
{
	SDL_Rect **rects;
	int x, y;

	rects = emalloc((w * h) * sizeof(SDL_Rect *));
	for (y = 0; y < h; y++) {
		*(rects + y) = emalloc(w * sizeof(SDL_Rect));
	}
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			rects[y][x].x = (x << m->shtilex);
			rects[y][x].y = (y << m->shtiley);
			rects[y][x].w = m->tilew;
			rects[y][x].h = m->tileh;
		}
	}
	
	return (rects);
}

/*
 * Allocate an array able to hold wxh rectangles,
 * for optimization purposes.
 */
static SDL_Rect *
view_allocrects(struct map *m, int w, int h)
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
 * Map/view must be locked.
 */
void
view_maskfill(struct viewport *v, SDL_Rect *rd, int n)
{
	int x, y;

	for (y = rd->y; y < rd->y + rd->h; y++) {
		for (x = rd->x; x < rd->x + rd->w; x++) {
			v->mapmask[y][x] += n;
		}
	}
}

/*
 * Free mask nodes.
 * Map/view must be locked.
 */
static void
view_freemask(struct viewport *v)
{
	int y;

	for (y = 0; y < v->maph; y++) {
		free(*(v->mapmask + y));
	}
	free(v->mapmask);
	v->mapmask = NULL;
}

/*
 * Free a map rectangle array.
 * View must be locked.
 */
static void
view_freemaprects(struct viewport *v)
{
	int y;

	for (y = 0; y < v->maph; y++) {
		free(*(v->maprects + y));
	}
	free(v->maprects);
	v->maprects = NULL;
}

struct viewport *
view_new(int w, int h, int depth, int flags)
{
	struct viewport *v;

	v = emalloc(sizeof(struct viewport));
	object_init(&v->obj, "viewport", "main-view", NULL, 0, &viewport_ops);
	v->fps = -1;
	v->w = w;
	v->h = h;
	v->flags = flags;
	v->depth = SDL_VideoModeOK(v->w, v->h, depth, flags);
	v->map = NULL;
	v->mapw = 0;
	v->maph = 0;
	v->mapx = 0;
	v->mapy = 0;
	v->mapxoffs = 0;
	v->mapyoffs = 0;
	v->vmapw = 0;
	v->vmaph = 0;
	v->mapmask = NULL;
	v->maprects = NULL;
	v->rects = NULL;
	TAILQ_INIT(&v->windowsh);
	pthread_mutex_init(&v->lock, NULL);

	return (v);
}

void
view_destroy(void *p)
{
	struct viewport *v = p;

	if (v->mapmask != NULL) {
		view_freemask(v);
	}
	if (v->maprects != NULL) {
		view_freemaprects(v);
	}
	if (v->rects != NULL) {
		free(v->rects);
		v->rects = NULL;
	}
}

/*
 * Switch to/from full screen mode.
 * Map/view must not be locked inside the calling thread.
 */
void
view_fullscreen(struct viewport *v, int full)
{
	SDL_Event nev;

	pthread_mutex_lock(&v->map->lock);

	if (full) {
		v->flags |= SDL_FULLSCREEN;
	} else {
		v->flags &= ~(SDL_FULLSCREEN);
	}

	/* Reset the mode. */
	view_setmode(v, v->map, -1, NULL);
	v->map->redraw++;

	pthread_mutex_unlock(&v->map->lock);

	/* Redraw everything. */
	nev.type = SDL_VIDEOEXPOSE;
	SDL_PushEvent(&nev);
}

/*
 * Center the view.
 * Map/view must be locked.
 */
void
view_center(struct viewport *view, int mapx, int mapy)
{
	int nx, ny;

	nx = mapx - (view->mapw / 2);
	ny = mapy - (view->maph / 2);

	if (nx <= 0)
		nx = 0;
	if (ny <= 0)
		ny = 0;
	if (nx > (view->map->mapw - view->mapw))
		nx = (view->map->mapw - view->mapw);
	if (ny > (view->map->maph - view->maph))
		ny = (view->map->maph - view->maph);

	view->mapx = nx;
	view->mapy = ny;
}

/* XXX later */
void
scroll(struct map *m, int dir)
{
	switch (dir) {
	case DIR_UP:
		if (--m->view->mapy < 0) {
			m->view->mapy = 0;
		}
		break;
	case DIR_DOWN:
		if (++m->view->mapy > (m->maph - m->view->maph)) {
			m->view->mapy = (m->maph - m->view->maph);
		}
		break;
	case DIR_LEFT:
		if (--m->view->mapx < 0) {
			m->view->mapx = 0;
		}
		break;
	case DIR_RIGHT:
		if (++m->view->mapx > (m->mapw - m->view->mapw)) {
			m->view->mapx = (m->mapw - m->view->mapw);
		}
		break;
	}
	m->redraw++;
}

/*
 * Redraw a view entirely.
 * The window list must not be locked by the caller thread.
 */
void
view_redraw(struct viewport *view)
{
	struct window *win;

	if (view->map != NULL) {		/* Async */
		view->map->redraw++;
	}
	if (curmapedit != NULL) {		/* Async */
		curmapedit->redraw++;
	}
	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH(win, &mainview->windowsh, windows) {
		pthread_mutex_lock(&win->lock);
		window_draw(win);
		pthread_mutex_unlock(&win->lock);
	}
	pthread_mutex_unlock(&view->lock);
}

#ifdef DEBUG
void
view_dumpmask(struct viewport *v)
{
	int x, y;

	for (y = 0; y < v->maph; y++) {
		for (x = 0; x < v->mapw; x++) {
			printf("(%d)", v->mapmask[y][x]);
		}
		printf("\n");
	}
}
#endif

/*
 * Attach a window to a view.
 * Will lock view.
 */
void
view_attach(void *parent, void *child)
{
	struct viewport *view = parent;
	struct window *win = child;

	OBJECT_ASSERT(parent, "viewport");
	OBJECT_ASSERT(child, "window");

	/* Notify the child being attached. */
	if (OBJECT_OPS(win)->onattach != NULL) {
		OBJECT_OPS(win)->onattach(view, win);
	}

	/* Attach and focus this window. */
	pthread_mutex_lock(&view->lock);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
	pthread_mutex_unlock(&view->lock);
}

/*
 * Detach a window from a view.
 * Window must be locked.
 */
void
view_detach(void *parent, void *child)
{
	struct viewport *view = parent;
	struct window *win = child;

	OBJECT_ASSERT(parent, "viewport");
	OBJECT_ASSERT(child, "window");

	/* Notify the child being detached. */
	if (OBJECT_OPS(win)->ondetach != NULL) {
		OBJECT_OPS(win)->ondetach(view, win);
	}

	pthread_mutex_lock(&view->lock);
	TAILQ_REMOVE(&view->windowsh, win, windows);
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
 * Focus on a window.
 * View and window must be locked.
 */
void
view_focus(struct viewport *view, struct window *win)
{
	dprintf("on %s\n", OBJECT(win)->name);

	TAILQ_REMOVE(&view->windowsh, win, windows);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
	
	win->redraw++;
}


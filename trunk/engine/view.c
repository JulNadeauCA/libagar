/*	$Csoft: view.c,v 1.48 2002/06/09 10:27:26 vedge Exp $	*/

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

#include "mapedit/mapedit.h"

#include "widget/window.h"

static const struct object_ops viewport_ops = {
	view_destroy,
	NULL,		/* load */
	NULL		/* save */
};

/* Read-only as long as the engine is running. */
struct viewport *mainview;

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
 * Map/view must be locked.
 */
void
view_maskfill(SDL_Rect *rd, int n)
{
	int x, y;

	for (y = rd->y; y < rd->y + rd->h; y++) {
		for (x = rd->x; x < rd->x + rd->w; x++) {
			mainview->mapmask[y][x] += n;
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

void
view_init(int mode, int w, int h, int depth, int flags)
{
	struct viewport *v;

	v = emalloc(sizeof(struct viewport));
	object_init(&v->obj, "viewport", "main-view", NULL, 0, &viewport_ops);
	v->mode = mode;
	v->w = w;
	v->h = h;
	v->depth = SDL_VideoModeOK(v->w, v->h, depth, flags);
	v->map = NULL;
	v->mapx = 0;
	v->mapy = 0;
	v->winop = VIEW_WINOP_NONE;
	v->wop_mapx = 0;
	v->wop_mapy = 0;
	TAILQ_INIT(&v->windowsh);
	pthread_mutex_init(&v->lock, NULL);
	
	switch (v->depth) {
	case 8:
		flags |= SDL_HWPALETTE;
		break;
	}

	v->v = SDL_SetVideoMode(w, h, v->depth, flags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", w, h, v->depth, SDL_GetError());
	}

	/* XXX should go away */
	switch (mode) {
	case VIEW_MAPNAV:
		dprintf("map navigation mode\n");
		v->mapw = (w / TILEW) - 1;
		v->maph = (h / TILEH);
		v->mapxoffs = 1;		/* XXX */
		v->mapyoffs = 1;
		v->vmapw = v->mapw - 1;
		v->vmaph = v->maph - 1;
		break;
	case VIEW_MAPEDIT:
		dprintf("map edition mode\n");
		v->mapw = (w / TILEW);
		v->maph = (h / TILEH);
		v->mapxoffs = 1;
		v->mapyoffs = 1;
		v->vmapw = v->mapw - v->mapxoffs - 1;	/* Tile list */
		v->vmaph = v->maph - v->mapyoffs;
#if 0
		if (v->map->mapw < v->mapw)
			v->mapxoffs += (v->mapw - v->map->mapw) / 2;
		if (v->map->maph < v->maph)
			v->mapyoffs += (v->maph - v->map->maph) / 2;
#endif
		break;
	default:
		fatal("bad mode\n");
	}
	
	/*
	 * Allocate view masks, precalculate node rectangles, and
	 * preallocate an array able to hold all possible rectangles
	 * in a view, for optimization purposes.
	 */
	dprintf("%dx%d map mask\n", v->mapw, v->maph);
	v->mapmask = view_allocmask(v->vmapw, v->vmaph);
	v->maprects = view_allocmaprects(v->mapw, v->maph);
	v->rects = view_allocrects(v->mapw, v->maph);

	SDL_WM_SetCaption("AGAR", "AGAR");
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Delay(100);

	mainview = v;
}

void
view_destroy(void *p)
{
	struct viewport *v = p;

	view_freemask(v);
	view_freemaprects(v);
	free(v->rects);
}

/*
 * Toggle full screen mode.
 *
 * View must be locked.
 */
void
view_fullscreen(int full)
{
	SDL_Event nev;
	struct viewport *v = mainview;

	if (full) {
		v->v->flags |= SDL_FULLSCREEN;
	} else {
		v->v->flags &= ~(SDL_FULLSCREEN);
	}

	/* XXX atomic: the v pointer is unprotected */
	v->v = SDL_SetVideoMode(v->w, v->h, v->depth, v->v->flags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", v->w, v->h, v->depth,
		    SDL_GetError());
	}

	/* Redraw everything. XXX */
	nev.type = SDL_VIDEOEXPOSE;
	SDL_PushEvent(&nev);
}

/*
 * Center the view.
 * Map/view must be locked.
 */
void
view_center(int mapx, int mapy)
{
	struct viewport *v = mainview;
	int nx, ny;

	nx = mapx - (v->mapw / 2);
	ny = mapy - (v->maph / 2);

	if (nx <= 0)
		nx = 0;
	if (ny <= 0)
		ny = 0;
	if (nx > (v->map->mapw - v->mapw))
		nx = (v->map->mapw - v->mapw);
	if (ny > (v->map->maph - v->maph))
		ny = (v->map->maph - v->maph);

	v->mapx = nx;
	v->mapy = ny;
}

/* XXX later */
void
scroll(struct map *m, int dir)
{
	switch (dir) {
	case DIR_UP:
		if (--mainview->mapy < 0) {
			mainview->mapy = 0;
		}
		break;
	case DIR_DOWN:
		if (++mainview->mapy > (m->maph - mainview->maph)) {
			mainview->mapy = (m->maph - mainview->maph);
		}
		break;
	case DIR_LEFT:
		if (--mainview->mapx < 0) {
			mainview->mapx = 0;
		}
		break;
	case DIR_RIGHT:
		if (++mainview->mapx > (m->mapw - mainview->mapw)) {
			mainview->mapx = (m->mapw - mainview->mapw);
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
view_redraw(void)
{
	struct window *win;
	struct viewport *v = mainview;

	if (v->map != NULL) {		/* Async */
		v->map->redraw++;
	}
	if (curmapedit != NULL) {	/* Async */
		curmapedit->redraw++;
	}
	pthread_mutex_lock(&v->lock);
	TAILQ_FOREACH(win, &v->windowsh, windows) {
		pthread_mutex_lock(&win->lock);
		if (win->flags & WINDOW_SHOW) {
			window_draw(win);
		}
		pthread_mutex_unlock(&win->lock);
	}
	pthread_mutex_unlock(&v->lock);
}

#ifdef DEBUG
void
view_dumpmask(void)
{
	struct viewport *v = mainview;
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
view_attach(void *child)
{
	struct window *win = child;

	OBJECT_ASSERT(child, "window");

	/* Notify the child being attached. */
	event_post(child, "attached", "%p", mainview);

	/* Attach and focus this window. */
	pthread_mutex_lock(&mainview->lock);
	TAILQ_INSERT_TAIL(&mainview->windowsh, win, windows);
	pthread_mutex_unlock(&mainview->lock);
}

/*
 * Detach a window from a view.
 * Window must be locked.
 */
void
view_detach(void *child)
{
	struct window *win = child;

	OBJECT_ASSERT(child, "window");

	/* Notify the child being detached. */
	event_post(child, "detached", "%p", mainview);

	pthread_mutex_lock(&mainview->lock);
	TAILQ_REMOVE(&mainview->windowsh, win, windows);
	pthread_mutex_unlock(&mainview->lock);
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

	TAILQ_REMOVE(&mainview->windowsh, win, windows);
	TAILQ_INSERT_TAIL(&mainview->windowsh, win, windows);
	
	win->redraw++;
}


/*	$Csoft: view.c,v 1.20 2002/03/05 17:02:24 vedge Exp $	*/

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
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
#include <engine/mapedit/mapedit.h>

static Uint32	**view_allocmask(Uint32, Uint32);
static SDL_Rect	**view_allocmaprects(struct map *, Uint32, Uint32);
static SDL_Rect	*view_allocrects(struct map *, Uint32, Uint32);
static void	view_freemask(struct viewport *);
static void	view_freemaprects(struct viewport *);
static void	view_freerects(struct viewport *);

struct viewport *mainview;

int
view_setmode(struct viewport *v, struct map *m, int mode, char *caption)
{
	pthread_mutex_lock(&v->lock);

	if (mode > -1) {
		v->mode = mode;
	}

	v->fps = 30;	/* XXX pref */

	switch (v->mode) {
	case VIEW_MAPNAV:
		dprintf("map navigation mode\n");
		v->map = m;
		v->mapw = (v->width / m->tilew);
		v->maph = (v->height / m->tileh);
		v->mapxoffs = 0;
		v->mapyoffs = 0;
		break;
	case VIEW_MAPEDIT:
		dprintf("map edition mode\n");
		v->map = m;
		v->mapw = (v->width / m->tilew) - 2;
		v->maph = (v->height / m->tileh) - 1;
		v->mapxoffs = 1;
		v->mapyoffs = 1;
		break;
	case VIEW_FIGHT:
		dprintf("off-map fight mode\n");
		v->map = NULL;
		v->mapw = -1;
		v->maph = -1;
		v->mapxoffs = -1;
		v->mapyoffs = -1;
		break;
	}

	switch (v->depth) {
	case 8:
		v->flags |= SDL_HWPALETTE;
		break;
	}

	if (caption != NULL) {
		SDL_WM_SetCaption(caption, "agar");
	}
	v->v = SDL_SetVideoMode(v->width + 64, v->height + 64, v->depth,
	    v->flags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", v->width, v->height, v->depth,
		    SDL_GetError());
		return (-1);
	}
	dprintf("fps %d flags %x geo %dx%dx%d on %s at %dx%d\n",
	    v->fps, v->flags, v->width, v->height, v->depth,
	    v->map->obj.name, v->mapx, v->mapy);

	if (v->mapmask != NULL)
		view_freemask(v);
	if (v->maprects != NULL)
		view_freemaprects(v);
	if (v->rects != NULL)
		view_freerects(v);
	v->mapmask = view_allocmask(v->mapw + 2, v->maph + 2);
	v->maprects = view_allocmaprects(m, v->mapw + 2, v->maph + 2);
	v->rects = view_allocrects(m, v->mapw + 2, v->maph + 2);

	if (curmapedit != NULL) {
		SDL_ShowCursor((v->flags & SDL_FULLSCREEN) ? 0 : 1);
	}
	
	pthread_mutex_unlock(&v->lock);
	return (0);
}

static Uint32 **
view_allocmask(Uint32 w, Uint32 h)
{
	Uint32 **mask;
	Uint32 x, y;

	mask = (Uint32 **)emalloc((w * h) * sizeof(Uint32));
	for (y = 0; y < h; y++) {
		*(mask + y) = (Uint32 *)emalloc(w * sizeof(Uint32));
	}
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			mask[y][x] = 0;
		}
	}
	
	return (mask);
}

static SDL_Rect **
view_allocmaprects(struct map *m, Uint32 w, Uint32 h)
{
	SDL_Rect **rects;
	Uint32 x, y;
	
	rects = (SDL_Rect **)emalloc((w * h) * sizeof(SDL_Rect));
	for (y = 0; y < h; y++) {
		*(rects + y) = (SDL_Rect *)emalloc(w * sizeof(SDL_Rect));
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

static SDL_Rect *
view_allocrects(struct map *m, Uint32 w, Uint32 h)
{
	SDL_Rect *rects;
	Uint32 len;
	
	len = (w * h) * sizeof(SDL_Rect *);
	rects = (SDL_Rect *)emalloc(len);
	memset(rects, NULL, len);
	
	return (rects);
}

void
view_maskfill(struct viewport *v, SDL_Rect *rd, Uint32 n)
{
	Uint32 x, y;
	
	pthread_mutex_lock(&v->lock);

	for (y = rd->y; y < rd->y + rd->h; y++) {
		for (x = rd->x; x < rd->x + rd->w; x++) {
			v->mapmask[y][x] += n;
		}
	}
	
	pthread_mutex_unlock(&v->lock);
}

static void
view_freemask(struct viewport *v)
{
	Uint32 y;
	
	pthread_mutex_lock(&v->lock);

	for (y = 0; y < v->maph; y++) {
		free(*(v->mapmask + y));
	}
	free(v->mapmask);
	v->mapmask = NULL;
	
	pthread_mutex_unlock(&v->lock);
}

static void
view_freemaprects(struct viewport *v)
{
	Uint32 y;

	pthread_mutex_lock(&v->lock);

	for (y = 0; y < v->maph; y++) {
		free(*(v->maprects + y));
	}
	free(v->maprects);
	v->maprects = NULL;
	
	pthread_mutex_unlock(&v->lock);
}

static void
view_freerects(struct viewport *v)
{
	free(v->rects);
	v->rects = NULL;
}

struct viewport *
view_create(Uint32 w, Uint32 h, Uint32 depth, Uint32 flags)
{
	struct viewport *v;

	v = emalloc(sizeof(struct viewport));
	v->fps = -1;
	v->width = w;
	v->height = h;
	v->flags = flags;
	v->depth = SDL_VideoModeOK(v->width, v->height, depth, flags);
	v->map = NULL;
	v->mapw = 0;
	v->maph = 0;
	v->mapx = 0;
	v->mapy = 0;
	v->mapxoffs = 0;
	v->mapyoffs = 0;
	v->mapmask = NULL;
	v->maprects = NULL;
	v->rects = NULL;

	if (pthread_mutex_init(&v->lock, NULL) != 0) {
		dperror("view");
		return (NULL);
	}

	return (v);
}

void
view_destroy(struct viewport *v)
{
	if (v->mapmask != NULL)
		view_freemask(v);
	if (v->maprects != NULL)
		view_freemaprects(v);
	if (v->rects != NULL)
		view_freerects(v);
	pthread_mutex_destroy(&v->lock);
	free(v);
}

int
view_fullscreen(struct viewport *v, int full)
{
	if (full) {
		v->flags |= SDL_FULLSCREEN;
	} else {
		v->flags &= ~(SDL_FULLSCREEN);
	}
	
	view_setmode(v, v->map, -1, NULL);
	v->map->redraw++;

	return (0);
}

void
view_center(struct viewport *view, int mapx, int mapy)
{
	int nx, ny;

	nx = mapx - (view->mapw / 2);
	ny = mapy - (view->maph / 2);

	if (nx <= 0)
		nx = 0;
	if (nx <= 0)
		nx = 0;
	if (nx > (view->map->mapw - view->mapw))
		nx = (view->map->mapw - view->mapw);
	if (ny > (view->map->maph - view->maph))
		ny = (view->map->maph - view->maph);

	view->mapx = nx;
	view->mapy = ny;
}

void
scroll(struct map *m, int dir)
{
	switch (dir) {
	case DIR_UP:
		decrease_uint32(&m->view->mapy, 1, 0);
		break;
	case DIR_DOWN:
		increase_uint32(&m->view->mapy, 1, m->maph - m->view->maph);
		break;
	case DIR_LEFT:
		decrease_uint32(&m->view->mapx, 1, 0);
		break;
	case DIR_RIGHT:
		increase_uint32(&m->view->mapx, 1, m->mapw - m->view->mapw);
		break;
	}
	m->redraw++;
}


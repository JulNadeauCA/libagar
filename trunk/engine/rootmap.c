/*	$Csoft: rootmap.c,v 1.35 2004/02/25 18:12:18 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>

#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/view.h>

void
rootmap_init(struct viewmap *rm, int mw, int mh)
{
	rm->w = mw - 1;
	rm->h = mh - 1;
	rm->map = NULL;
	rm->x = 0;
	rm->y = 0;
	rm->sx = 0;
	rm->sy = 0;
	
	/* Calculate the default tile coordinates. */
	rm->maprects = rootmap_alloc_maprects(mw, mh);
}

/*
 * Render all animations in the map view.
 * The view and the displayed map must be locked.
 */
void
rootmap_animate(void)
{
	struct map *m = view->rootmap->map;
	struct viewmap *rm = view->rootmap;
	struct node *node;
	struct noderef *nref;
	int mx, my, rx, ry;
	int layer = 0;

#ifdef DEBUG
	/* XXX */
	if (rm->x < 0 || rm->y < 0 ||
	    rm->y > m->maph - rm->h || rm->x > m->mapw - rm->w) {
		fatal("bad offs");
	}
#endif

draw_layer:
	if (!m->layers[layer].visible) {
		goto next_layer;
	}
	for (my = rm->y, ry = rm->sy - m->scale;		/* Downward */
	     (my - rm->y) < m->maph && my < m->maph;
	     my++, ry += m->scale) {

		for (mx = rm->x, rx = rm->sx - m->scale; 	/* Forward */
		     (mx - rm->x) < m->mapw && mx < m->mapw;
		     mx++, rx += m->scale) {
			node = &m->map[my][mx];
			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->type == NODEREF_ANIM &&
				    nref->layer == layer) {
					noderef_draw(m, nref, rx, ry);
				}
			}
		}
	}
next_layer:
	if (++layer < m->nlayers) 
		goto draw_layer;
}

/*
 * Draw static tiles.
 * The view and the displayed map must be locked.
 */
void
rootmap_draw(void)
{
	struct map *m = view->rootmap->map;
	struct viewmap *rm = view->rootmap;
	struct node *node;
	struct noderef *nref;
	int mx, my, rx, ry;
	int layer = 0;

draw_layer:
	if (!m->layers[layer].visible) {
		goto next_layer;
	}
	for (my = rm->y, ry = rm->sy - m->scale;		/* Downward */
	     (my - rm->y) < m->maph && my < m->maph;
	     my++, ry += m->scale) {

		for (mx = rm->x, rx = rm->sx - m->scale;	/* Forward */
		     (mx - rm->x) < m->mapw && mx < m->mapw;
		     mx++, rx += m->scale) {
			node = &m->map[my][mx];
			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->type == NODEREF_SPRITE &&
				    nref->layer == layer) {
					noderef_draw(m, nref, rx, ry);
				}
			}
		}
	}
next_layer:
	if (++layer < m->nlayers)
		goto draw_layer;
}

/* Display the given map. */
void
rootmap_focus(struct map *m)
{
	pthread_mutex_lock(&view->lock);
	dprintf("%s -> %s\n",
	    (view->rootmap == NULL || view->rootmap->map == NULL) ? "NULL" :
	    OBJECT(view->rootmap->map)->name, OBJECT(m)->name);
	view->rootmap->map = m;
	pthread_mutex_unlock(&view->lock);
}

void
rootmap_center(struct map *m, int mapx, int mapy)
{
	struct viewmap *rm = view->rootmap;
	int nx, ny;

	dprintf("centering on map `%s'\n", OBJECT(m)->name);

	pthread_mutex_lock(&m->lock);
	nx = mapx - rm->w/2;
	ny = mapy - rm->h/2;

	if (nx < 0)
		nx = 0;
	if (ny < 0)
		ny = 0;
	if (nx >= m->mapw - rm->w)
		nx = m->mapw - rm->w;
	if (ny >= m->maph - rm->h)
		ny = m->maph - rm->h;

	rm->x = nx;
	rm->y = ny;

	m->redraw++;
	pthread_mutex_unlock(&m->lock);
}

void
rootmap_scroll(struct map *m, int dir, int inc)
{
	struct viewmap *rm;
	
	pthread_mutex_lock(&view->lock);
	rm = view->rootmap;

	switch (dir) {
	case ROOTMAP_N:
		if ((rm->sy -= inc) <= -TILESZ) {
			if (--rm->y < 0) {
				rm->y = 0;
			}
			rm->sy = 0;
		}
		break;
	case ROOTMAP_S:
		if ((rm->sy += inc) >= TILESZ) {
			if (++rm->y > (m->maph - rm->h)) {
				rm->y = m->maph - rm->h;
			}
			rm->sy = 0;
		}
		break;
	case ROOTMAP_W:
		if ((rm->sx -= inc) <= -TILESZ) {
			if (--rm->x < 0) {
				rm->x = 0;
			}
			rm->sx = 0;
		}
		break;
	case ROOTMAP_E:
		if ((rm->sx += inc) >= TILESZ) {
			if (++rm->x > (m->mapw - rm->w)) {
				rm->x = m->mapw - rm->w;
			}
			rm->sx = 0;
		}
		break;
	}

	m->redraw++;
	pthread_mutex_unlock(&view->lock);
}

SDL_Rect **
rootmap_alloc_maprects(int w, int h)
{
	SDL_Rect **rects;
	int x, y;

	/* Calculate the default coordinates of visible rectangles. */
	rects = Malloc(h * sizeof(SDL_Rect *));
	for (y = 0; y < h; y++) {
		rects[y] = Malloc(w * sizeof(SDL_Rect));
		for (x = 0; x < w; x++) {
			rects[y][x].x = x*TILESZ;
			rects[y][x].y = y*TILESZ;
			rects[y][x].w = TILESZ;
			rects[y][x].h = TILESZ;
		}
	}
	return (rects);
}

void
rootmap_free_maprects(struct viewport *v)
{
	int y;

	pthread_mutex_lock(&v->lock);
	for (y = 0; y < v->rootmap->h; y++) {
		free(v->rootmap->maprects[y]);
	}
	free(v->rootmap->maprects);
	pthread_mutex_unlock(&v->lock);
}

void
rootmap_redraw(void)
{
	if (view->rootmap != NULL) {
		pthread_mutex_lock(&view->rootmap->map->lock);
		view->rootmap->map->redraw++;
		pthread_mutex_unlock(&view->rootmap->map->lock);
	}
}


/*	$Csoft: rootmap.c,v 1.5 2002/09/02 08:12:34 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "map.h"
#include "rootmap.h"
#include "physics.h"

#ifdef DEBUG
#define CHECK_RECTS(view, ri) {						\
	int _i;								\
	for (_i = 0; _i < (ri); _i++) {					\
		if ((view)->rootmap->rects[_i].x > (view)->w ||		\
		    (view)->rootmap->rects[_i].y > (view)->h ) {	\
			dprintrect("bogus",				\
			    &(view)->rootmap->rects[_i]);		\
			dprintf("bogus rectangle %d/%d\n", _i, (ri));	\
		}							\
	}								\
}
#define CHECK_MASK(x, y) {						\
	int _i;								\
									\
	_i = view->rootmap->mask[(y)][(x)];				\
	if (_i < 0 || _i > 128) {					\
		fatal("funny mask: %d at %d,%d\n", _i, (x), (y));	\
	}								\
	if (_i > 0) {							\
		continue;						\
	}								\
}
#else
#define CHECK_RECTS(view, ri)
#define CHECK_MASK(x, y)						\
	if (view->rootmap->mask[(y)][(x)] > 0) {			\
		continue;						\
	}
#endif

/*
 * Render a map node.
 * Inline since this is called from draw functions with many variables.
 * Must be called on a locked map.
 */
static void
rootmap_draw_node(struct map *m, struct node *node, Uint32 rx, Uint32 ry)
{
	SDL_Rect rd;
	SDL_Surface *src;
	struct noderef *nref;
	struct anim *anim;
	int i, j, frame;
	Uint32 t;

	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		if (nref->flags & MAPREF_SPRITE) {
			src = SPRITE(nref->pobj, nref->offs);
			rd.w = src->w;
			rd.h = src->h;
			rd.x = rx + nref->xoffs;
			rd.y = ry + nref->yoffs;
			SDL_BlitSurface(src, NULL, view->v, &rd);
		} else if (nref->flags & MAPREF_ANIM) {
			anim = ANIM(nref->pobj, nref->offs);
			frame = anim->frame;
#ifdef DEBUG
			if (frame > anim->nframes) {
				fatal("bad anim frame: %d > %d\n", frame,
				    anim->nframes);
			}
#endif

			if (nref->flags & MAPREF_ANIM_DELTA &&
			   (nref->flags & MAPREF_ANIM_STATIC) == 0) {
				t = SDL_GetTicks();
				if ((t - anim->delta) >= anim->delay) {
					anim->delta = t;
					if (++anim->frame >
					    anim->nframes - 1) {
						/* Loop */
						anim->frame = 0;
					}
				}
			} else if (nref->flags & MAPREF_ANIM_INDEPENDENT) {
				frame = nref->frame;

				if ((nref->flags & MAPREF_ANIM_STATIC) == 0) {
					if ((anim->delay < 1) ||
					    (++nref->fdelta > anim->delay+1)) {
						nref->fdelta = 0;
						if (++nref->frame >
						    anim->nframes - 1) {
							/* Loop */
							nref->frame = 0;
						}
					}
				}
			}

			for (i = 0, j = anim->nparts - 1;
			     i < anim->nparts;
			     i++, j--) {
				src = anim->frames[j][frame];
#ifdef DEBUG
				if (src->w < 0 || src->w > 4096 ||
				    src->h < 0 || src->h > 4096) {
					fatal("bad frame: j=%d i=%d [%d]\n",
					     j, i, frame);
				}
#endif
				rd.w = src->w;
				rd.h = src->h;
				rd.x = rx + nref->xoffs;
				rd.y = ry + nref->yoffs - (i * TILEH);
				SDL_BlitSurface(src, NULL, view->v, &rd);
			}
		}
	}
}

/*
 * Render all animations in the map view.
 *
 * Nodes with the NODE_ANIM flag set are rendered by the animator, even
 * if there is no animation on the node; nearby nodes with overlap > 0
 * are redrawn first.
 *
 * Map and view must be locked.
 */
void
rootmap_animate(void)
{
	struct map *m = view->rootmap->map;
	struct viewmap *rm = view->rootmap;
	struct node *nnode;
	int x, y, vx, vy, rx, ry, ox, oy;
	int ri = 0;

	return;

	for (y = rm->y, vy = 0;				/* Downward */
	     y < m->maph && vy <= rm->h;
	     y++, vy++) {

		ry = vy << m->shtiley;

		for (x = rm->x, vx = 0;			/* Forward */
		     x < m->mapw && vx <= rm->w;
		     x++, vx++) {
			struct node *node;

			/* Skip this node if it is masked. */
			CHECK_MASK(vx, vy);

			node = &m->map[y][x];
#ifdef DEBUG
			if (node->x != x || node->y != y) {
				fatal("node at %d,%d should be at %d,%d\n",
				    x, y, node->x, node->y);
			}
#endif
			if (node->overlap > 0) {
				/* Will later draw in a synchronized fashion. */
				continue;
			}

			rx = vx << m->shtilex;

			if (node->flags & NODE_ANIM) {
#if 1
				/*
				 * ooo
				 * ooo
				 * oSo
				 * ooo
				 */
				for (oy = -2; oy < 2; oy++) {
					for (ox = -1; ox < 2; ox++) {
						if (ox == 0 && oy == 0) {
							/* Origin */
							continue;
						}
						nnode = &m->map[y + oy][x + ox];
						if (nnode->overlap > 0 &&
						    (vx > 1 && vy > 1) &&
						    (vx < rm->w) &&
						    (vy < rm->h)) {
							rootmap_draw_node(m,
							    nnode,
							    rx + (TILEW*ox),
							    ry + (TILEH*oy));
							rm->rects[ri++] =
							    rm->maprects
							    [vy+oy][vx+ox];
						}
					}
				}

				/* Draw the node itself. */
				rootmap_draw_node(m, node, rx, ry);
				rm->rects[ri++] = rm->maprects[vy][vx];
#endif
			} else if (node->nanims > 0) {
				rootmap_draw_node(m, node, rx, ry);
				rm->rects[ri++] = rm->maprects[vy][vx];
			}
		}
	}
	if (ri > 0) {
		CHECK_RECTS(view, ri);
		SDL_UpdateRects(view->v, ri, rm->rects);
	}
}

/*
 * Draw static tiles.
 * Map and view must be locked.
 */
void
rootmap_draw(void)
{
	int x, y, vx, vy, rx, ry;
	struct map *m = view->rootmap->map;
	struct viewmap *rm = view->rootmap;
	struct node *node;
	struct noderef *nref;
	Uint32 nsprites;
	int ri = 0;

	dprintf("offset %d,%d\n", rm->x, rm->y);

	if (rm->y > m->maph - rm->h ||
	    rm->x > m->mapw - rm->w) {
		dprintf("exceeds map boundaries\n");
		return;
	}

	for (y = rm->y, vy = 0;				/* Downward */
	     y < m->maph && vy <= rm->h;
	     y++, vy++) {

		ry = vy << m->shtiley;

		for (x = rm->x, vx = 0;			/* Forward */
		     x < m->mapw && vx <= rm->w;
		     x++, vx++) {

			/* Skip this node if it is masked. */
			CHECK_MASK(vx, vy);

			rx = vx << m->shtilex;

			node = &m->map[y][x];
#ifdef DEBUG
			if (node->x != x || node->y != y) {
				fatal("node at %d,%d should be at %d,%d\n",
				    x, y, node->x, node->y);
			}
#endif

			if (node->nanims > 0 || ((node->flags & NODE_ANIM) ||
			    node->overlap > 0)) {
				/* rootmap_animate() shall handle this. */
				continue;
			}
		
			nsprites = 0;
			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref->flags & MAPREF_SPRITE) {
					SDL_Rect rd;

					rd.x = rx + nref->xoffs;
					rd.y = ry + nref->yoffs;
					rd.w = TILEW;
					rd.h = TILEH;
					SDL_BlitSurface(
					    SPRITE(nref->pobj, nref->offs),
					    NULL, view->v, &rd);
					nsprites++;
				}
			}
			rm->rects[ri++] = rm->maprects[vy][vx];
		}
	}

	if (ri > 0) {
		CHECK_RECTS(view, ri);
		SDL_UpdateRects(view->v, ri, rm->rects);
	}
}

/*
 * Change the map being displayed.
 * View must not be locked.
 */
void
rootmap_focus(struct map *m)
{
	dprintf("focusing %s\n", OBJECT(m)->name);

	pthread_mutex_lock(&view->lock);
#if 0
	dprintf("%s -> %s\n",
	    (view->rootmap == NULL) ? "NULL" : OBJECT(view->rootmap->map)->name,
	    OBJECT(m)->name);
#endif
	view->rootmap->map = m;
	pthread_mutex_unlock(&view->lock);
}

/*
 * Center the root map view around given coordinates.
 * Map must be locked.
 */
void
rootmap_center(struct map *m, int mapx, int mapy)
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
	m->redraw++;
}

/*
 * Scroll the root map view.
 * View must be locked.
 */
void
rootmap_scroll(struct map *m, int dir)
{
	struct viewmap *rm = view->rootmap;

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

/* Allocate a map mask of the given size. */
int **
rootmap_alloc_mask(int w, int h)
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

/* Precalculate rectangles. This helps saving a few cycles */
SDL_Rect **
rootmap_alloc_maprects(int w, int h)
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
SDL_Rect *
rootmap_alloc_rects(int w, int h)
{
	SDL_Rect *rects;
	size_t len;
	
	len = (w * h) * sizeof(SDL_Rect *);
	rects = emalloc(len);
	memset(rects, NULL, len);
	return (rects);
}

/*
 * Increment map mask nodes matching rd.
 * View must be locked.
 */
void
rootmap_maskfill(SDL_Rect *rd, int n)
{
	int x, y;

#ifdef DEBUG
	if (view->rootmap == NULL) {
		fatal("NULL rootmap\n");
	}
#endif

	for (y = rd->y; y < rd->y+rd->h; y++) {
		for (x = rd->x; x < rd->x+rd->w; x++) {
			view->rootmap->mask[y][x] += n;
		}
	}
}

/*
 * Free map mask nodes.
 * View must be locked.
 */
void
rootmap_free_mask(struct viewport *v)
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
void
rootmap_free_maprects(struct viewport *v)
{
	int y;

	for (y = 0; y < v->rootmap->h; y++) {
		free(*(v->rootmap->maprects + y));
	}
	free(v->rootmap->maprects);
	v->rootmap->maprects = NULL;
}


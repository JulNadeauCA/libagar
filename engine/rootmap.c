/*	$Csoft: rootmap.c,v 1.14 2002/11/28 07:19:24 vedge Exp $	*/

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

#include "engine.h"

#include "map.h"
#include "rootmap.h"
#include "physics.h"
#include "view.h"
#include "anim.h"

/* Update nodes individually. */
/* #define UPDATE_NODES */

#ifdef DEBUG
#define DEBUG_FOCUS	0x01
#define DEBUG_DIAG	0x02

int	rootmap_debug = DEBUG_DIAG|DEBUG_FOCUS;
#define	engine_debug rootmap_debug
#endif

/*
 * Render a map node.
 * Inline since this is called from draw functions with many variables.
 * The map must be locked.
 */
static __inline__ void
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
			rd.x = rx + nref->xoffs - view->rootmap->sx - TILEW;
			rd.y = ry + nref->yoffs - view->rootmap->sy - TILEH;
			rd.w = src->w;
			rd.h = src->h;
			SDL_BlitSurface(src, NULL, view->v, &rd);
		} else if (nref->flags & MAPREF_ANIM) {
			anim = ANIM(nref->pobj, nref->offs);
			frame = anim->frame;
#ifdef DEBUG
			if (frame < 0 || frame > anim->nframes) {
				fatal("bad animation frame#: %d > %d\n",
				    frame, anim->nframes);
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
					fatal("bad frame fragment: %d %d %d\n",
					    j, i, frame);
				}
#endif
				rd.x = rx + nref->xoffs -
				    view->rootmap->sx - TILEW;
				rd.y = ry + nref->yoffs - (i * TILEH) -
				    view->rootmap->sy - TILEH;
				rd.w = src->w;
				rd.h = src->h;
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
 * The view and the displayed map must be locked.
 */
void
rootmap_animate(void)
{
	struct map *m = view->rootmap->map;
	struct viewmap *rm = view->rootmap;
	struct node *nnode;
	int x, y, vx, vy, rx, ry, ox, oy;

	for (y = rm->y, vy = 0;				/* Downward */
	     y < m->maph && vy <= rm->h + 2;
	     y++, vy++) {

		ry = vy << m->shtiley;

		for (x = rm->x, vx = 0;			/* Forward */
		     x < m->mapw && vx <= rm->w + 2;
		     x++, vx++) {
			struct node *node;

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

							/* Render the node. */
							rootmap_draw_node(m,
							    nnode,
							    rx + (TILEW*ox),
							    ry + (TILEH*oy));
#ifdef UPDATE_NODES
							/* Queue video update */
							VIEW_UPDATE(
							    rm->maprects
							    [vy+oy][vx+ox]);
#endif
						}
					}
				}

				/* Render the node. */
				rootmap_draw_node(m, node, rx, ry);

#ifdef UPDATE_NODES
				/* Queue the video update. */
				VIEW_UPDATE(rm->maprects[vy][vx]);
#endif
			} else if (node->nanims > 0) {
#ifdef DEBUG
				if (node->x != vx+rm->x ||
				    node->y != vy+rm->y) {
					fatal("inconsistent node\n");
				}
#endif
				/* Render the node. */
				rootmap_draw_node(m, node, rx, ry);

#ifdef UPDATE_NODES
				/* Queue the video update. */
				VIEW_UPDATE(rm->maprects[vy][vx]);
#endif
			}
		}
	}
}

/*
 * Draw static tiles.
 * The view and the displayed map must be locked.
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

	if (rm->y > m->maph - rm->h ||
	    rm->x > m->mapw - rm->w) {
		debug(DEBUG_DIAG, "exceeds map boundaries\n");
		return;
	}

	for (y = rm->y, vy = 0;				/* Downward */
	     y < m->maph && vy <= rm->h + 2;
	     y++, vy++) {

		ry = vy << m->shtiley;

		for (x = rm->x, vx = 0;			/* Forward */
		     x < m->mapw && vx <= rm->w + 2;
		     x++, vx++) {
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

					rd.x = rx + nref->xoffs -
					    rm->sx - TILEW;
					rd.y = ry + nref->yoffs -
					    rm->sy - TILEH;
					rd.w = TILEW;
					rd.h = TILEH;
					SDL_BlitSurface(
					    SPRITE(nref->pobj, nref->offs),
					    NULL, view->v, &rd);
					nsprites++;
				}
			}
#ifdef UPDATE_NODES
			/* Queue the video update. */
			VIEW_UPDATE(rm->maprects[vy][vx]);
#endif
		}
	}
}

/* Change the map being displayed. */
void
rootmap_focus(struct map *m)
{
	pthread_mutex_lock(&view->lock);
	debug(DEBUG_FOCUS, "%s -> %s\n",
	    (view->rootmap == NULL || view->rootmap->map == NULL) ? "NULL" :
	    OBJECT(view->rootmap->map)->name, OBJECT(m)->name);
	view->rootmap->map = m;
	pthread_mutex_unlock(&view->lock);
}

/* Center the root map view around given coordinates. */
void
rootmap_center(struct map *m, int mapx, int mapy)
{
	struct viewmap *rm = view->rootmap;
	int nx, ny;

	debug(DEBUG_FOCUS, "centering on map `%s'\n", OBJECT(m)->name);

	pthread_mutex_lock(&m->lock);
	
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
	
	pthread_mutex_unlock(&m->lock);
}

/* Scroll the root map view. */
void
rootmap_scroll(struct map *m, int dir, int inc)
{
	struct viewmap *rm;
	
	pthread_mutex_lock(&view->lock);

	rm = view->rootmap;

	switch (dir) {
	case DIR_UP:
		if ((rm->sy -= inc) <= -TILEH) {
			if (--rm->y < 0) {
				rm->y = 0;
			}
			rm->sy = 0;
		}
		break;
	case DIR_DOWN:
		if ((rm->sy += inc) >= TILEH) {
			if (++rm->y > (m->maph - rm->h)) {
				rm->y = m->maph - rm->h;
			}
			rm->sy = 0;
		}
		break;
	case DIR_LEFT:
		if ((rm->sx -= inc) <= -TILEW) {
			if (--rm->x < 0) {
				rm->x = 0;
			}
			rm->sx = 0;
		}
		break;
	case DIR_RIGHT:
		if ((rm->sx += inc) >= TILEH) {
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

/* Calculate the default coordinates of visible rectangles. */
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

/* Free a map rectangle coordinate array. */
void
rootmap_free_maprects(struct viewport *v)
{
	int y;

	pthread_mutex_lock(&v->lock);
	for (y = 0; y < v->rootmap->h; y++) {
		free(*(v->rootmap->maprects + y));
	}
	pthread_mutex_unlock(&v->lock);
}


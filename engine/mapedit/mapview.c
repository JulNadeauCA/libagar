/*	$Csoft$	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/physics.h>
#include <engine/map.h>

#include <engine/widget/primitive.h>
#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "mapview.h"

static const struct widget_ops mapview_ops = {
	{
		mapview_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	mapview_draw,
	mapview_animate
};

static void	mapview_event(int, union evarg *);
static void	mapview_scaled(int, union evarg *);
static void	mapview_scroll(struct mapview *, int);

struct mapview *
mapview_new(struct region *reg, struct map *m, int flags, int rw, int rh)
{
	struct mapview *mv;

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, m, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, mv);
	pthread_mutex_unlock(&reg->win->lock);

	return (mv);
}

void
mapview_init(struct mapview *mv, struct map *m, int flags, int rw, int rh)
{
	widget_init(&mv->wid, "mapview", "widget", &mapview_ops, rw, rh);

	mv->flags = flags;
	mv->map = m;
	mv->mx = m->defx;
	mv->my = m->defy;
	mv->mw = 0;		/* Set on scale */
	mv->mh = 0;

	event_new(mv, "window-mousebuttonup", 0,
	    mapview_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(mv, "window-mousebuttondown", 0,
	    mapview_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(mv, "window-keyup", 0,
	    mapview_event, "%i", WINDOW_KEYUP);
	event_new(mv, "window-keydown", 0,
	    mapview_event, "%i", WINDOW_KEYDOWN);
	event_new(mv, "window-mousemotion", 0,
	    mapview_event, "%i", WINDOW_MOUSEMOTION);
	event_new(mv, "window-widget-scaled", 0,
	    mapview_scaled, NULL);
}

void
mapview_destroy(void *p)
{
	struct mapview *mv = p;

	OBJECT_ASSERT(mv, "widget");

	/* ... */
}

/* Map must be locked. XXX special case in event loop. */
void
mapview_draw(void *p)
{
	struct mapview *mv = p;
	struct map *m = mv->map;
	struct node *node;
	struct noderef *nref;
	int mx, my, vx, vy, rx, ry;
	Uint32 nsprites;

	OBJECT_ASSERT(mv, "widget");

	for (my = mv->my, vy = 0;
	     vy < mv->mh && my < m->maph;
	     my++, vy++) {

		ry = vy << m->shtiley;

		for (mx = mv->mx, vx = 0;
	     	     vx < mv->mw && mx < m->mapw;
		     mx++, vx++) {

			rx = vx << m->shtilex;

			node = &m->map[my][mx];

			if (node->nanims > 0 || ((node->flags & NODE_ANIM) ||
			    node->overlap > 0)) {
				/* mapview_animate() shall handle this. */
				continue;
			}
			
			nsprites = 0;
			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref->flags & MAPREF_SPRITE) {
					WIDGET_DRAW(mv,
					    SPRITE(nref->pobj, nref->offs),
					    rx, ry);
					nsprites++;
				}
			}

			if (nsprites > 0) {
			/*	MAPEDIT_POSTDRAW(m, node, vx, vy); */
			}
		}
	}
}

/*
 * Render an animated node.
 * Map must be locked.
 */
static __inline__ void
mapview_rendernode(struct map *m, struct node *node, Uint32 rx, Uint32 ry)
{
	SDL_Rect rd;
	SDL_Surface *src;
	struct noderef *nref;
	struct anim *anim;
	int i, j, frame;
	Uint32 t;

	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		anim = ANIM(nref->pobj, nref->offs);
		frame = anim->frame;
		
		if (nref->flags & MAPREF_ANIM_DELTA &&
		   (nref->flags & MAPREF_ANIM_STATIC) == 0) {
			t = SDL_GetTicks();
			if ((t - anim->delta) >= anim->delay) {
				anim->delta = t;
				if (++anim->frame > anim->nframes - 1) {
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
					if (++nref->frame > anim->nframes - 1) {
						/* Loop */
						nref->frame = 0;
					}
				}
			}
		}

		for (i = 0, j = anim->nparts - 1; i < anim->nparts; i++, j--) {
			src = anim->frames[j][frame];
#ifdef DEBUG
			if (src->w < 0 || src->w > 4096 ||
			    src->h < 0 || src->h > 4096) {
				fatal("bad frame: j=%d i=%d [%d]\n", j, i,
				    frame);
			}
#endif
			rd.w = src->w;
			rd.h = src->h;
			rd.x = rx + nref->xoffs;
			rd.y = ry + nref->yoffs - (i * TILEH);
			/* XXX blit */
			SDL_BlitSurface(src, NULL, view->v, &rd);
		}
	}
}

/* Update animations. */
void
mapview_animate(void *p)
{
	struct mapview *mv = p;
	struct map *m = mv->map;
	struct node *node, *nnode;
	int mx, my, vx, vy, rx, ry, ox, oy;

	OBJECT_ASSERT(mv, "widget");

	pthread_mutex_lock(&m->lock);
	for (my = mv->my, vy = 0;
	     vy < mv->mh && my < m->maph;
	     my++, vy++) {

		ry = WIDGET(mv)->win->y + (vy << m->shtiley);

		for (mx = mv->mx, vx = 0;
	     	     vx < mv->mw && mx < m->mapw;
		     mx++, vx++) {

			rx = WIDGET(mv)->win->x + (vx << m->shtilex);

			node = &m->map[my][mx];

#if 0
			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref->flags & MAPREF_SPRITE) {
					WIDGET_DRAW(mv,
					    SPRITE(nref->pobj, nref->offs),
					    rx, ry);
				}
			}
#endif
			
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
						nnode = &m->map[my+oy][mx+ox];
						
						/* XXX bounds, see map */
						if (nnode->overlap > 0) {
#if 0
							MAPEDIT_PREDRAW(m,
							    nnode,
							    vx+ox, vy+oy);
#endif
							mapview_rendernode(m,
							    nnode,
							    rx + (TILEW*ox),
							    ry + (TILEH*oy));
#if 0
							MAPEDIT_POSTDRAW(m,
							    nnode,
							    vx+ox, vy+oy);
							view->rects[ri++] =
							    view->maprects
							    [vy+oy][vx+ox];
#endif
						}
					}
				}
				mapview_rendernode(m, node, rx, ry);
			} else if (node->nanims > 0) {
//				MAPEDIT_PREDRAW(m, node, vx, vy);
				mapview_rendernode(m, node, rx, ry);
//				MAPEDIT_POSTDRAW(m, node, vx, vy);
//				view->rects[ri++] = view->maprects[vy][vx];
			}
		}
	}
	pthread_mutex_unlock(&m->lock);
}

static void
mapview_scroll(struct mapview *mv, int dir)
{
	switch (dir) {
	case DIR_LEFT:
		if (--mv->mx <= 0) {
			mv->mx = 0;
		}
		break;
	case DIR_UP:
		if (--mv->my <= 0) {
			mv->my = 0;
		}
		break;
	case DIR_RIGHT:
		if (++mv->mx >= (mv->map->mapw - mv->mw)) {
			mv->mx--;
		}
		break;
	case DIR_DOWN:
		if (++mv->my >= (mv->map->maph - mv->mh)) {
			mv->my--;
		}
		break;
	}
}

static void
mapview_event(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	struct map *m = mv->map;
	int type = argv[1].i;
	int keysym, keymod;
	int button, x, y;

	OBJECT_ASSERT(mv, "widget");

	switch (type) {
	case WINDOW_MOUSEMOTION:
		if (mv->mouse.move == 0) {
			return;
		}

		x = argv[2].i / TILEW;
		y = argv[3].i / TILEH;

		mapview_scroll(mv,
		    (mv->mouse.x < x) ? DIR_LEFT :
		    (mv->mouse.x > x) ? DIR_RIGHT :
		    (mv->mouse.y < y) ? DIR_UP :
		    (mv->mouse.y > y) ? DIR_DOWN : 0);
		
		mv->mouse.x = x;
		mv->mouse.y = y;
		break;
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		WIDGET_FOCUS(mv);
		if (button == 1) {
			mv->mouse.move++;
		}
		break;
	case WINDOW_MOUSEBUTTONUP:
		button = argv[2].i;
		if (button == 1) {
			mv->mouse.move = 0;
		}
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		keymod = argv[3].i;
		
		mapview_scroll(mv,
		    (keysym == SDLK_LEFT) ? DIR_LEFT :
		    (keysym == SDLK_RIGHT) ? DIR_RIGHT :
		    (keysym == SDLK_UP) ? DIR_UP :
		    (keysym == SDLK_DOWN) ? DIR_DOWN : 0);

		switch ((SDLKey)keysym) {
		case SDLK_LEFT:
			if (--mv->mx <= 0) {
				mv->mx = 0;
			}
			break;
		case SDLK_UP:
			if (--mv->my <= 0) {
				mv->my = 0;
			}
			break;
		case SDLK_RIGHT:
			if (++mv->mx >= (m->mapw - mv->mw)) {
				mv->mx--;
			}
			break;
		case SDLK_DOWN:
			if (++mv->my >= (m->maph - mv->mh)) {
				mv->my--;
			}
			break;
		default:
			break;
		}
		dprintf("move to %d,%d\n", mv->mx, mv->my);
		break;
	case WINDOW_KEYUP:
		break;
	}
}

static void
mapview_scaled(int argc, union evarg *argv)
{
	struct mapview *mv= argv[0].p;
	int maxw = argv[1].i, maxh = argv[2].i;
	int w, h;

	for (w = 0; w < WIDGET(mv)->w - 1 && w < maxw; w += TILEW) ;;
	for (h = 0; h < WIDGET(mv)->h - 1 && h < maxh; h += TILEH) ;;
	
	WIDGET(mv)->w = w;
	WIDGET(mv)->h = h;
	mv->mw = w / TILEW;
	mv->mh = h / TILEH;

	if (w == 0 || h == 0) {
		dprintf("%s: scaled to zero\n", OBJECT(mv)->name);
	}

	dprintf("scaled to %dx%d\n", w, h);
}


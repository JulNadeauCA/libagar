/*	$Csoft: mapview.c,v 1.1 2002/06/22 20:44:01 vedge Exp $	*/

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
#include "edcursor.h"

static const struct widget_ops mapview_ops = {
	{
		mapview_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	mapview_draw,
	NULL		/* animate */
};

static void	mapview_event(int, union evarg *);
static void	mapview_scaled(int, union evarg *);
static void	mapview_scroll(struct mapview *, int);

struct mapview *
mapview_new(struct region *reg, struct mapedit *med, struct map *m,
    int flags, int rw, int rh)
{
	struct mapview *mv;

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, m, flags, rw, rh);
	
	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, mv);
	pthread_mutex_unlock(&reg->win->lock);

	return (mv);
}

void
mapview_init(struct mapview *mv, struct mapedit *med, struct map *m,
    int flags, int rw, int rh)
{
	widget_init(&mv->wid, "mapview", "widget", &mapview_ops, rw, rh);

	mv->flags = flags;
	mv->med = med;
	mv->map = m;
	mv->mx = m->defx;
	mv->my = m->defy;
	mv->mw = 0;		/* Set on scale */
	mv->mh = 0;
	mv->cursor = edcursor_new(0, mv, m);

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

/*
 * Render an animated node.
 * Map must be locked.
 */
static __inline__ void
mapview_rendernode(struct mapview *mv, struct node *node, Uint32 rx, Uint32 ry)
{
	SDL_Surface *src;
	struct noderef *nref;
	struct anim *anim;
	struct map *m = mv->map;
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
			WIDGET_DRAW(mv, src,
			    rx + nref->xoffs,
			    ry + nref->yoffs - (i * TILEH));
		}
	}
}

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

			if (node->nanims > 0 || ((node->flags & NODE_ANIM))) {
				mapview_rendernode(mv, node, rx, ry);
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
		event_post(mv->cursor, "window-keydown",
		    "%i, %i", argv[2].i, argv[3].i);
		break;
	case WINDOW_KEYUP:
		event_post(mv->cursor, "window-keyup",
		    "%i, %i", argv[2].i, argv[3].i);
		break;
	}
}

static void
mapview_scaled(int argc, union evarg *argv)
{
	struct mapview *mv= argv[0].p;
	int maxw = argv[1].i, maxh = argv[2].i;
	int w, h;

	for (w = 0; w < WIDGET(mv)->w-1 && w < maxw; w += TILEW) ;;
	for (h = 0; h < WIDGET(mv)->h-1 && h < maxh; h += TILEH) ;;
	
	WIDGET(mv)->w = w;
	WIDGET(mv)->h = h;
	mv->mw = w/TILEW - 1;
	mv->mh = h/TILEH - 1;

	if (w == 0 || h == 0) {
		dprintf("%s: scaled to zero\n", OBJECT(mv)->name);
	}

	dprintf("scaled to %dx%d\n", w, h);
}

void
mapview_center(struct mapview *mv, Uint32 x, Uint32 y)
{
	struct map *m = mv->map;
	Uint32 nx, ny;

	nx = x - (mv->mw / 2);
	ny = y - (mv->mh / 2);
	
	if (nx <= 0)
		nx = 0;
	if (ny <= 0)
		ny = 0;
	if (nx >= m->mapw-mv->mw)
		nx = m->mapw - mv->mw;
	if (ny >= m->maph-mv->mh)
		ny = m->maph - mv->mh;

	mv->mx = nx;
	mv->my = ny;
}


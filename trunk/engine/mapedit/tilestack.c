/*	$Csoft: tilestack.c,v 1.16 2002/12/17 06:43:53 vedge Exp $	*/

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

#include <engine/engine.h>

#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/text.h>

#include "tilestack.h"
#include "mapview.h"
#include "mapedit.h"

static const struct widget_ops tilestack_ops = {
	{
		widget_destroy,
		NULL,	/* load */
		NULL	/* save */
	},
	tilestack_draw,
	NULL		/* update */
};

enum {
	SELECTION_COLOR,
	GRID_COLOR
};

static void	tilestack_scaled(int, union evarg *);

struct tilestack *
tilestack_new(struct region *reg, int flags, int rw, int rh, struct mapview *mv)
{
	struct tilestack *tilestack;

	tilestack = emalloc(sizeof(struct tilestack));
	tilestack_init(tilestack, flags, rw, rh, mv);

	region_attach(reg, tilestack);

	return (tilestack);
}

void
tilestack_init(struct tilestack *ts, int flags, int rw, int rh,
    struct mapview *mv)
{
	widget_init(&ts->wid, "tilestack", &tilestack_ops, rw, rh);
	widget_map_color(ts, SELECTION_COLOR, "tilestack-selection", 0, 200, 0);
	widget_map_color(ts, GRID_COLOR, "tilestack-grid", 208, 208, 208);

	ts->offs = 0;
	ts->flags = (flags != 0) ? flags : TILESTACK_VERT;
	ts->mv = mv;

	event_new(ts, "widget-scaled", tilestack_scaled, NULL);
}

static void
tilestack_scaled(int argc, union evarg *argv)
{
	struct tilestack *ts = argv[0].p;
	int maxw = argv[1].i, maxh = argv[2].i;
	int w, h;

	for (w = 0; w < WIDGET(ts)->w-1 && w < maxw; w += TILEW) ;;
	for (h = 0; h < WIDGET(ts)->h-1 && h < maxh; h += TILEH) ;;
	
	WIDGET(ts)->w = w;
	WIDGET(ts)->h = h;
}

void
tilestack_draw(void *p)
{
	struct tilestack *ts = p;
	struct mapview *mv = ts->mv;
	struct node *n;
	struct noderef *nref;
	struct art_anim *an;
	int nx, ny, y = 0;
	SDL_Surface *su;

	nx = mv->mx + mv->mouse.x;
	ny = mv->my + mv->mouse.y;

	if (nx >= mv->map->mapw - 1 || ny >= mv->map->maph ||
	    nx < 0 || ny < 0) {
		return;
	}

	n = &mv->map->map[ny][nx];

	TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
		switch (nref->type) {
		case NODEREF_SPRITE:
			su = view_scale_surface(SPRITE(nref->pobj, nref->offs),
			    WIDGET(ts)->w, WIDGET(ts)->w);
			WIDGET_DRAW(ts, su, 0, y);
			SDL_FreeSurface(su);
			break;
		case NODEREF_ANIM:
			an = ANIM(nref->pobj, nref->offs);
			su = view_scale_surface(an->frames[0],
			    WIDGET(ts)->w, WIDGET(ts)->w);
			WIDGET_DRAW(ts, su, 0, y);
			SDL_FreeSurface(su);

			if (nref->data.anim.flags & NODEREF_ANIM_AUTO) {
				WIDGET_DRAW(ts,
				    SPRITE(mv->med, MAPEDIT_ANIM_DELTA_TXT),
				    0, y);
			}
			WIDGET_DRAW(ts, SPRITE(mv->med, MAPEDIT_ANIM_TXT),
			    0, y);
			break;
		case NODEREF_WARP:
			su = text_render(NULL, -1,
			    SDL_MapRGB(view->v->format, 255, 255, 255),
			    nref->data.warp.map);
			WIDGET_DRAW(ts, su, 0, y);
			SDL_FreeSurface(su);
			break;
		}

		primitives.square(ts, 0, y, TILEW, TILEH,
		    WIDGET_COLOR(ts, GRID_COLOR));

		y += TILEH;
		if (y + TILEH > WIDGET(ts)->h) {
			break;
		}
	}
}


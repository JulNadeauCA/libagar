/*	$Csoft: objq.c,v 1.7 2002/07/23 23:49:36 vedge Exp $	*/

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include "mapedit.h"
#include "mapview.h"
#include "objq.h"

static const struct widget_ops objq_ops = {
	{
		objq_destroy,	/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	objq_draw,
	NULL		/* animate */
};

static void	 objq_scaled(int, union evarg *);
static void	 objq_event(int, union evarg *);

enum {
	SELECTION_COLOR,
	GRID_COLOR
};

enum {
	SELECT_BUTTON =		1,
	SCROLL_BUTTON_MASK =	(SDL_BUTTON_MMASK|SDL_BUTTON_RMASK),
	SCROLL_LEFT_KEY =	SDLK_LEFT,
	SCROLL_RIGHT_KEY =	SDLK_RIGHT
};

struct objq *
objq_new(struct region *reg, struct mapedit *med, int flags, int rw, int rh)
{
	struct objq *objq;

	objq = emalloc(sizeof(struct objq));
	objq_init(objq, med, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, objq);
	pthread_mutex_unlock(&reg->win->lock);

	return (objq);
}

void
objq_init(struct objq *oq, struct mapedit *med, int flags, int rw, int rh)
{
	widget_init(&oq->wid, "objq", "widget", &objq_ops, rw, rh);
	widget_map_color(oq, SELECTION_COLOR, "objq-selection", 0, 200, 0);
	widget_map_color(oq, GRID_COLOR, "objq-grid", 208, 208, 208);
	
	oq->med = med;
	oq->offs = 0;
	oq->flags = (flags != 0) ? flags : OBJQ_HORIZ;
	SLIST_INIT(&oq->tmaps);
	
	event_new(oq, "widget-scaled", 0,
	    objq_scaled, NULL);
	event_new(oq, "window-mousemotion", 0,
	    objq_event, "%i", WINDOW_MOUSEMOTION);
	event_new(oq, "window-mousebuttonup", 0,
	    objq_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(oq, "window-mousebuttondown", 0,
	    objq_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(oq, "window-keyup", 0,
	    objq_event, "%i", WINDOW_KEYUP);
	event_new(oq, "window-keydown", 0,
	    objq_event, "%i", WINDOW_KEYDOWN);
}

static void
objq_select(struct objq *oq, struct mapedit *med, struct editobj *eob)
{
	char caption[4096];
	struct object *ob = eob->pobj;
	struct objq_tmap *tm;
	struct window *win;
	struct region *reg;
	struct mapview *mv;

	SLIST_FOREACH(tm, &oq->tmaps, tmaps) {
		if (tm->ob == eob->pobj) {
			window_show(tm->win);
			pthread_mutex_lock(&tm->win->lock);
			view_focus(tm->win);
			pthread_mutex_unlock(&tm->win->lock);
			return;
		}
	}

	sprintf(caption, "%s", ob->name);
	win = emalloc(sizeof(struct window));
	window_init(win, caption, WINDOW_SOLID|WINDOW_CENTER,
	    0, 0, 19 + TILEW*3, 318, 19+TILEW, 45+TILEH);
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
	/* Map view */
	mv = mapview_new(reg, med, ob->art->map,
	    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_TILEMAP|MAPVIEW_GRID, 100, 100);
	win->focus = WIDGET(mv);

	tm = emalloc(sizeof(struct objq_tmap));
	tm->win = win;
	tm->ob = ob;
	SLIST_INSERT_HEAD(&oq->tmaps, tm, tmaps);

	pthread_mutex_lock(&win->lock);
	view_attach(win);
	window_show_locked(win);
	pthread_mutex_unlock(&win->lock);
}

static void
objq_event(int argc, union evarg *argv)
{
	struct objq *oq = argv[0].p;
	struct mapedit *med = oq->med;
	struct editobj *eob;
	int button, x, ox, ms;
	int curoffs;

	switch (argv[1].i) {
	case WINDOW_MOUSEMOTION:
		x = argv[2].i / TILEW;
		ox = oq->mouse.x;
		oq->mouse.x = x;

		ms = SDL_GetMouseState(NULL, NULL);
		if (ms & SCROLL_BUTTON_MASK) {
			if (oq->mouse.x > ox &&		/* Down */
			    --oq->offs < 0) {
				oq->offs = med->neobjs - 1;
			}
			if (oq->mouse.x < ox &&		/* Up */
			    ++oq->offs > med->neobjs-1) {
				oq->offs = 0;
			}
		}
		break;
	case WINDOW_MOUSEBUTTONUP:
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		x = argv[3].i;
		if (button != SELECT_BUTTON) {
			break;
		}
		curoffs = oq->offs + x/TILEW;
		if (curoffs < 0) {
			/* Wrap */
			curoffs += oq->offs + med->neobjs;
		} else if (med->curoffs >= med->neobjs) {
			curoffs -= med->neobjs;
		}
		while (curoffs > med->neobjs-1) {
			curoffs -= med->neobjs;
		}
		TAILQ_INDEX(eob, &med->eobjsh, eobjs, curoffs);
		objq_select(oq, med, eob);
		break;
	}
}

static void
objq_scaled(int argc, union evarg *argv)
{
	struct objq *oq = argv[0].p;
	int maxw = argv[1].i, maxh = argv[2].i;
	int w, h;

	for (w = 0; w < WIDGET(oq)->w-1 && w < maxw; w += TILEW) ;;
	for (h = 0; h < WIDGET(oq)->h-1 && h < maxh; h += TILEH) ;;
	
	WIDGET(oq)->w = w;
	WIDGET(oq)->h = h;
}

void
objq_destroy(void *p)
{
	struct objq *oq = p;
	struct objq_tmap *tm, *ntm;

	for (tm = SLIST_FIRST(&oq->tmaps);
	     tm != SLIST_END(&oq->tmaps);
	     tm = ntm) {
		ntm = SLIST_NEXT(tm, tmaps);
		free(tm);
	}
}

void
objq_draw(void *p)
{
	struct objq *oq = p;
	struct mapedit *med = oq->med;
	struct editobj *eob;
	int x = 0, i, sn;
	
	for (i = 0, sn = oq->offs;
	     i < (WIDGET(oq)->w / TILEW);
	     i++, x += TILEW) {
		if (sn > -1) {
			/* Absolute */
			TAILQ_INDEX(eob, &med->eobjsh, eobjs, sn);
		} else {
			/* Wrap */
			TAILQ_INDEX(eob, &med->eobjsh, eobjs,
			    sn + med->neobjs);
			if (eob == NULL) {
				goto nextref;
			}
		}

		WIDGET_DRAW(oq, SPRITE(eob->pobj, 0), x, 0);

		if (med->curobj == eob) {
			primitives.square(oq, x, 0, TILEW, TILEH,
			    WIDGET_COLOR(oq, SELECTION_COLOR));
			primitives.square(oq, x+1, 1, TILEW-1, TILEH-1,
			    WIDGET_COLOR(oq, SELECTION_COLOR));
		} else {
			primitives.square(oq, x, 0, TILEW, TILEH,
			    WIDGET_COLOR(oq, GRID_COLOR));
		}
nextref:
		if (++sn >= med->neobjs) {
			sn = 0;
		}
	}
}


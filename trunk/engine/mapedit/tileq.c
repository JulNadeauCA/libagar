/*	$Csoft: tileq.c,v 1.1 2002/06/24 18:41:11 vedge Exp $	*/

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

#include "mapedit.h"
#include "tileq.h"

static const struct widget_ops tileq_ops = {
	{
		NULL,	/* destroy */
		NULL,	/* load */
		NULL	/* save */
	},
	tileq_draw,
	NULL		/* animate */
};

static void	 tileq_scaled(int, union evarg *);
static void	 tileq_event(int, union evarg *);

#define TILEQ_ADJUST(tq, med) do {			\
	if ((tq)->offs > (med)->curobj->nrefs) {	\
		(tq)->offs = 0;				\
	}						\
} while (/*CONSTCOND*/0)

struct tileq *
tileq_new(struct region *reg, struct mapedit *med, int flags, int rw, int rh)
{
	struct tileq *tileq;

	tileq = emalloc(sizeof(struct tileq));
	tileq_init(tileq, med, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, tileq);
	pthread_mutex_unlock(&reg->win->lock);

	return (tileq);
}

void
tileq_init(struct tileq *tq, struct mapedit *med, int flags, int rw, int rh)
{
	widget_init(&tq->wid, "tileq", "widget", &tileq_ops, rw, rh);
	tq->med = med;
	tq->offs = 0;
	tq->flags = (flags != 0) ? flags : TILEQ_VERT;
	
	event_new(tq, "window-widget-scaled", 0,
	    tileq_scaled, NULL);
	event_new(tq, "window-mousemotion", 0,
	    tileq_event, "%i", WINDOW_MOUSEMOTION);
	event_new(tq, "window-mousebuttonup", 0,
	    tileq_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(tq, "window-mousebuttondown", 0,
	    tileq_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(tq, "window-keyup", 0,
	    tileq_event, "%i", WINDOW_KEYUP);
	event_new(tq, "window-keydown", 0,
	    tileq_event, "%i", WINDOW_KEYDOWN);
}

static void
tileq_event(int argc, union evarg *argv)
{
	struct tileq *tq = argv[0].p;
	struct mapedit *med = tq->med;
	int button, y, oy, ms;

	switch (argv[1].i) {
	case WINDOW_MOUSEMOTION:
		y = argv[3].i / TILEH;

		oy = tq->mouse.y;
		tq->mouse.y = y;

		ms = SDL_GetMouseState(NULL, NULL);
		if (ms & (SDL_BUTTON_LMASK|SDL_BUTTON_MMASK)) {
			pthread_mutex_lock(&med->lock);
			pthread_mutex_lock(&med->curobj->lock);
			TILEQ_ADJUST(tq, med);

			if (tq->mouse.y > oy &&		/* Down */
			    --tq->offs < 0) {
				tq->offs = med->curobj->nrefs - 1;
			}
			if (tq->mouse.y < oy &&		/* Up */
			    ++tq->offs > med->curobj->nrefs-1) {
				tq->offs = 0;
			}
			pthread_mutex_unlock(&med->curobj->lock);
			pthread_mutex_unlock(&med->lock);
		}
		break;
	case WINDOW_MOUSEBUTTONUP:
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		y = argv[4].i;
	
		if (button == 1) {
			break;
		}
		pthread_mutex_lock(&med->lock);
		pthread_mutex_lock(&med->curobj->lock);
		TILEQ_ADJUST(tq, med);

		med->curoffs = tq->offs + y/TILEH;
		if (med->curoffs < 0) {
			/* Wrap */
			med->curoffs += tq->offs + med->curobj->nrefs;
		} else if (med->curoffs >= med->curobj->nrefs) {
			med->curoffs -= med->curobj->nrefs;
		}
		while (med->curoffs > med->curobj->nrefs - 1) {
			med->curoffs -= med->curobj->nrefs;
		}
		pthread_mutex_unlock(&med->curobj->lock);
		pthread_mutex_unlock(&med->lock);
		break;
	case WINDOW_KEYDOWN:
		pthread_mutex_lock(&med->lock);
		pthread_mutex_lock(&med->curobj->lock);
		TILEQ_ADJUST(tq, med);

		/* XXX gendir */
		switch ((SDLKey)argv[2].i) {
		case SDLK_PAGEUP:
			if (--tq->offs < 0) {
				tq->offs = med->curobj->nrefs - 1;
			}
			break;
		case SDLK_PAGEDOWN:
			if (++tq->offs > med->curobj->nrefs-1) {
				tq->offs = 0;
			}
			break;
		default:
		}
		pthread_mutex_unlock(&med->curobj->lock);
		pthread_mutex_unlock(&med->lock);
		break;
	}
}

static void
tileq_scaled(int argc, union evarg *argv)
{
	struct tileq *tq = argv[0].p;
	int maxw = argv[1].i, maxh = argv[2].i;
	int w, h;

	for (w = 0; w < WIDGET(tq)->w-1 && w < maxw; w += TILEW) ;;
	for (h = 0; h < WIDGET(tq)->h-1 && h < maxh; h += TILEH) ;;
	
	WIDGET(tq)->w = w;
	WIDGET(tq)->h = h;
}

void
tileq_draw(void *p)
{
	struct tileq *tq = p;
	struct mapedit *med = tq->med;
	int y = 0, i, sn;
	
	pthread_mutex_lock(&med->lock);
	pthread_mutex_lock(&med->curobj->lock);

	TILEQ_ADJUST(tq, med);

	for (i = 0, sn = tq->offs;
	     i < (WIDGET(tq)->h / TILEH) - 1;
	     i++, y += TILEH) {
		struct editref *ref;
		struct anim *anim;

		if (sn > -1) {
			/* Absolute */
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs, sn);
		} else {
			/* Wrap */
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs,
			    sn + med->curobj->nrefs);
			if (ref == NULL) {
				/* XXX hack */
				goto nextref;
			}
		}

		if (ref == NULL) {
			goto nextref;
		}

		switch (ref->type) {
		case EDITREF_SPRITE:
			WIDGET_DRAW(tq, (SDL_Surface *)ref->p, 0, y);
			break;
		case EDITREF_ANIM:
			anim = (struct anim *)ref->p;
			WIDGET_DRAW(tq, anim->frames[0][0], 0, y);
			break;
		}

		if (med->curoffs == sn) {
			WIDGET_DRAW(tq, SPRITE(med, MAPEDIT_CIRQSEL), 0, y);
		} else {
			WIDGET_DRAW(tq, SPRITE(med, MAPEDIT_GRID), 0, y);
		}
nextref:
		if (++sn >= med->curobj->nrefs) {
			sn = 0;
		}
	}
	
	pthread_mutex_unlock(&med->curobj->lock);
	pthread_mutex_unlock(&med->lock);
}


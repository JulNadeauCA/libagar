/*	$Csoft: tlist.c,v 1.9 2002/08/21 23:52:14 vedge Exp $	*/

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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "tlist.h"

static struct widget_ops tlist_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	tlist_draw,
	NULL		/* animate */
};

enum {
	INSIDE_COLOR,
	OUTSIDE_COLOR,
	TEXT_COLOR
};

static void	tlist_event(int, union evarg *);

struct tlist *
tlist_new(struct region *reg, int rw, int rh, int flags)
{
	struct tlist *tl;

	tl = emalloc(sizeof(struct tlist));
	tlist_init(tl, rw, rh, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, tl);
	pthread_mutex_unlock(&reg->win->lock);

	return (tl);
}

void
tlist_init(struct tlist *tl, int rw, int rh, int flags)
{
	widget_init(&tl->wid, "tlist", "widget", &tlist_ops, rw, rh);

	widget_map_color(tl, INSIDE_COLOR, "tlist-inside", 250, 250, 250);
	widget_map_color(tl, OUTSIDE_COLOR, "tlist-outside", 150, 150, 200);
	widget_map_color(tl, TEXT_COLOR, "tlist-text", 240, 240, 240);

	tl->flags = flags;
	tl->xspacing = 6;
	tl->yspacing = 3;
	tl->ops.update = NULL;
	TAILQ_INIT(&tl->items);

	event_new(tl, "window-mousebuttondown", 0,
	    tlist_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(tl, "window-keydown", 0,
	    tlist_event, "%i", WINDOW_KEYDOWN);
}

void
tlist_draw(void *p)
{
	struct tlist *tl = p;
	int y, i;

	primitives.frame(tl, 0, 0, WIDGET(tl)->w, WIDGET(tl)->h,
	    WIDGET_FOCUSED(tl) ? 1 : 0);
	
	if (tl->ops.update != NULL) {
		tl->ops.update(tl);
	}

#if 0
	for (i = 0, y = 0;
	     i < tl->nitems;
	     i++, y += rad->item_h) {
		const char *s;
		SDL_Surface *ls;
	
		s = rad->items[i];

		/* Radio button */
		primitives.circle(rad, 0, y,
		    rad->tlist.w, rad->tlist.h, 6,
		    WIDGET_COLOR(rad, OUTSIDE_COLOR));
		if (rad->selitem == i) {
			primitives.circle(rad, 0, y,
			    rad->tlist.w, rad->tlist.h, 3,
			    WIDGET_COLOR(rad, INSIDE_COLOR));
			primitives.circle(rad, 0, y,
			    rad->tlist.w, rad->tlist.h, 2,
			    WIDGET_COLOR(rad, OUTSIDE_COLOR));
		}

		/* XXX cache */
		ls = text_render(NULL, -1,
		    WIDGET_COLOR(rad, TEXT_COLOR), (char *)s);
		WIDGET_DRAW(rad, ls, rad->tlist.w, y);
		SDL_FreeSurface(ls);
	}
#endif
}

static void
tlist_event(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int evtype = argv[1].i;

	switch (evtype) {
	case WINDOW_MOUSEBUTTONDOWN:
		break;
	case WINDOW_KEYDOWN:
		break;
	default:
		return;
	}

#if 0
	event_post(tl, "tlist-changed", "%c, %i", '*', sel);
#endif
}


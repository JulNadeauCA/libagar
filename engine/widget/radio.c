/*	$Csoft: radio.c,v 1.23 2003/03/02 04:13:15 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/view.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "radio.h"

static struct widget_ops radio_ops = {
	{
		radio_destroy,	/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	radio_draw,
	NULL		/* update */
};

enum {
	INSIDE_COLOR,
	OUTSIDE_COLOR,
	TEXT_COLOR
};

static void	radio_event(int, union evarg *);
static void	radio_scaled(int, union evarg *);

struct radio *
radio_new(struct region *reg, const char **items)
{
	struct radio *rad;

	rad = emalloc(sizeof(struct radio));
	radio_init(rad, items);

	region_attach(reg, rad);

	return (rad);
}

void
radio_init(struct radio *rad, const char **items)
{
	const char *s;
	int i, maxw;
	int fh;

	widget_init(&rad->wid, "radio", &radio_ops, -1, -1);
	WIDGET(rad)->flags |= WIDGET_CLIPPING;

	widget_map_color(rad, INSIDE_COLOR, "inside", 210, 210, 210);
	widget_map_color(rad, OUTSIDE_COLOR, "outside", 180, 180, 180);
	widget_map_color(rad, TEXT_COLOR, "text", 240, 240, 240);

	widget_bind(rad, "value", WIDGET_INT, NULL, &rad->def.value);
	rad->def.value = -1;

	rad->items = items;
	rad->xspacing = 2;
	rad->yspacing = 2;

	fh = text_font_height(font);
	rad->radius = (fh > 0 && fh < 256) ? fh : 10;
	rad->max_w = 0;
	
	for (rad->nitems = 0; (s = *items++) != NULL; rad->nitems++) ;;

	rad->labels = emalloc(sizeof(SDL_Surface *) * rad->nitems);

	for (i = 0; i < rad->nitems; i++) {
		SDL_Surface *su;

		su = text_render(NULL, -1, WIDGET_COLOR(rad, TEXT_COLOR),
		    (char *)rad->items[i]);
		rad->labels[i] = su;
		if (su->w > rad->max_w)
			rad->max_w = su->w;
	}

	event_new(rad, "window-mousebuttondown",
	    radio_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(rad, "window-keydown",
	    radio_event, "%i", WINDOW_KEYDOWN);
	event_new(rad, "widget-scaled", radio_scaled, NULL);
}

void
radio_draw(void *p)
{
	struct radio *rad = p;
	int y, i;

#if 0
	primitives.frame(rad, 0, 0, WIDGET(rad)->w, WIDGET(rad)->h,
	    WIDGET_FOCUSED(rad) ? 1 : 0);
#endif

	for (i = 0, y = 0; i < rad->nitems;
	     i++, y += (rad->radius + rad->yspacing)) {
		SDL_Surface *ls;
	
		/* Radio button */
		primitives.circle(rad, 0, y,
		    rad->radius, rad->radius, 6,
		    WIDGET_COLOR(rad, OUTSIDE_COLOR));
		if (widget_get_int(rad, "value") == i) {
			primitives.circle(rad, 0, y,
			    rad->radius, rad->radius, 3,
			    WIDGET_COLOR(rad, INSIDE_COLOR));
			primitives.circle(rad, 0, y,
			    rad->radius, rad->radius, 2,
			    WIDGET_COLOR(rad, OUTSIDE_COLOR));
		}
		widget_blit(rad, rad->labels[i], rad->radius, y);
	}
}

void
radio_destroy(void *p)
{
	struct radio *rad = p;
	int i;

	for (i = 0; i < rad->nitems; i++) {
		SDL_FreeSurface(rad->labels[i]);
	}

	widget_destroy(rad);
}

static void
radio_scaled(int argc, union evarg *argv)
{
	struct radio *rad = argv[0].p;
	int maxw = argv[1].i;
	int maxh = argv[1].i;

	if (WIDGET(rad)->rw == -1)
		WIDGET(rad)->w = rad->radius + rad->xspacing + rad->max_w;
	if (WIDGET(rad)->rh == -1)
		WIDGET(rad)->h = rad->nitems * (rad->yspacing + rad->radius);
}

static void
radio_event(int argc, union evarg *argv)
{
	struct radio *rad = argv[0].p;
	struct widget_binding *valueb;
	int type = argv[1].i;
	int button, keysym;
	int y;
	int *sel;

	valueb = widget_binding_get_locked(rad, "value", &sel);

	switch (type) {
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		y = argv[4].i;
	
		*sel = (y / (rad->radius + rad->yspacing/2));

		WIDGET_FOCUS(rad);
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		switch ((SDLKey)keysym) {
		case SDLK_DOWN:
			if (++(*sel) > rad->nitems) {
				*sel = 0;
			}
			break;
		case SDLK_UP:
			if (--(*sel) < 0) {
				*sel = 0;
			}
			break;
		default:
			break;
		}
		break;
	default:
		return;
	}

	if (*sel >= rad->nitems) {
		*sel = rad->nitems - 1;
	}

	event_post(rad, "radio-changed", "%i", *sel);

	widget_binding_unlock(valueb);
}


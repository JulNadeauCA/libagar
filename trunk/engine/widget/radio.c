/*	$Csoft: radio.c,v 1.15 2002/11/17 23:13:59 vedge Exp $	*/

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

#include <engine/view.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "radio.h"

static struct widget_ops radio_ops = {
	{
		widget_destroy,	/* destroy */
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

struct radio *
radio_new(struct region *reg, char **items, int selitem)
{
	struct radio *rad;

	rad = emalloc(sizeof(struct radio));
	radio_init(rad, items, selitem);

	region_attach(reg, rad);

	return (rad);
}

void
radio_init(struct radio *rad, char **items, int selitem)
{
	char *s;
	int maxw;

	widget_init(&rad->wid, "radio", "widget", &radio_ops, -1, -1);

	widget_map_color(rad, INSIDE_COLOR, "radio-inside", 250, 250, 250);
	widget_map_color(rad, OUTSIDE_COLOR, "radio-outside", 150, 150, 200);
	widget_map_color(rad, TEXT_COLOR, "radio-text", 240, 240, 240);

	rad->items = items;
	rad->selitem = 0;
	rad->xspacing = 6;
	rad->yspacing = 6;
	rad->radio.w = font_h;
	rad->radio.h = font_h;
	rad->justify = RADIO_LEFT;

	for (rad->nitems = 0, maxw = 0; (s = *items++) != NULL;) {
		SDL_Surface *su;

		su = text_render(NULL, -1, WIDGET_COLOR(rad, TEXT_COLOR), s);
		if (su->w > maxw) {
			maxw = su->w;
		}
		SDL_FreeSurface(su);
		rad->nitems++;
	}

#ifdef DEBUG
	if (selitem > rad->nitems) {
		fatal("item %d > %d items\n", selitem, rad->nitems);
	}
#endif

	WIDGET(rad)->h = rad->nitems * (rad->yspacing + rad->radio.h);
	WIDGET(rad)->w = rad->radio.w + rad->xspacing + maxw;

	event_new(rad, "window-mousebuttondown",
	    radio_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(rad, "window-keydown",
	    radio_event, "%i", WINDOW_KEYDOWN);
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
	     i++, y += rad->radio.h+rad->yspacing/2) {
		char *s;
		SDL_Surface *ls;
	
		s = rad->items[i];

		/* Radio button */
		primitives.circle(rad, 0, y,
		    rad->radio.w, rad->radio.h, 6,
		    WIDGET_COLOR(rad, OUTSIDE_COLOR));
		if (rad->selitem == i) {
			primitives.circle(rad, 0, y,
			    rad->radio.w, rad->radio.h, 3,
			    WIDGET_COLOR(rad, INSIDE_COLOR));
			primitives.circle(rad, 0, y,
			    rad->radio.w, rad->radio.h, 2,
			    WIDGET_COLOR(rad, OUTSIDE_COLOR));
		}

		/* XXX cache */
		ls = text_render(NULL, -1,
		    WIDGET_COLOR(rad, TEXT_COLOR), s);
		WIDGET_DRAW(rad, ls, rad->radio.w, y);
		SDL_FreeSurface(ls);
	}
}

static void
radio_event(int argc, union evarg *argv)
{
	struct radio *rad = argv[0].p;
	int type = argv[1].i;
	int button, keysym;
	int y, sel = rad->selitem;

	OBJECT_ASSERT(argv[0].p, "widget");

	switch (type) {
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		y = argv[4].i;
	
		sel = (y / (rad->radio.h + rad->yspacing/2));

		WIDGET_FOCUS(rad);
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		switch ((SDLKey)keysym) {
		case SDLK_DOWN:
			if (++sel > rad->nitems) {
				sel = 0;
			}
			break;
		case SDLK_UP:
			if (--sel < 0) {
				sel = 0;
			}
			break;
		default:
		}
		break;
	default:
		return;
	}

	if (sel >= rad->nitems) {
		sel = rad->nitems - 1;
	}

	rad->selitem = sel;
	event_post(rad, "radio-changed", "%c, %i", '*', sel);
}


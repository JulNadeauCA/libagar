/*	$Csoft: radio.c,v 1.36 2003/07/08 00:34:59 vedge Exp $	*/

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

#include "radio.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static struct widget_ops radio_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		radio_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	radio_draw,
	radio_scale
};

enum {
	XSPACING =	7,
	YSPACING =	2,
	XPADDING =	4,
	YPADDING =	4,
};

enum {
	INSIDE_COLOR,
	OUTSIDE_COLOR,
	TEXT_COLOR,
	FRAME_COLOR,
	MOUSEBUTTONDOWN_EVENT,
	KEYDOWN_EVENT
};

static void	radio_event(int, union evarg *);

struct radio *
radio_new(void *parent, const char **items)
{
	struct radio *rad;

	rad = Malloc(sizeof(struct radio));
	radio_init(rad, items);
	object_attach(parent, rad);
	return (rad);
}

void
radio_init(struct radio *rad, const char **items)
{
	const char *s, **itemsp = items;
	int i;

	widget_init(rad, "radio", &radio_ops, WIDGET_FOCUSABLE);
	widget_bind(rad, "value", WIDGET_INT, NULL, &rad->value);

	widget_map_color(rad, INSIDE_COLOR, "inside", 210, 210, 210, 255);
	widget_map_color(rad, OUTSIDE_COLOR, "outside", 180, 180, 180, 255);
	widget_map_color(rad, TEXT_COLOR, "text", 240, 240, 240, 255);
	widget_map_color(rad, FRAME_COLOR, "frame", 120, 120, 120, 255);

	rad->value = -1;
	rad->radius = text_font_height(font) / 2;
	rad->max_w = 0;

	for (rad->nitems = 0; (s = *itemsp++) != NULL; rad->nitems++)
		;;
	rad->labels = Malloc(sizeof(SDL_Surface *) * rad->nitems);
	for (i = 0; i < rad->nitems; i++) {
		rad->labels[i] = text_render(NULL, -1,
		    WIDGET_COLOR(rad, TEXT_COLOR), _(items[i]));
		if (rad->labels[i]->w > rad->max_w)
			rad->max_w = rad->labels[i]->w;
	}

	event_new(rad, "window-mousebuttondown", radio_event, "%i",
	    MOUSEBUTTONDOWN_EVENT);
	event_new(rad, "window-keydown", radio_event, "%i", KEYDOWN_EVENT);
}

void
radio_draw(void *p)
{
	struct radio *rad = p;
	int i, val;
	int x = XPADDING + rad->radius*2 + XSPACING;
	int y = YPADDING;

	primitives.frame(rad,
	    0,
	    0,
	    WIDGET(rad)->w,
	    WIDGET(rad)->h,
	    FRAME_COLOR);

	val = widget_get_int(rad, "value");

	for (i = 0;
	     i < rad->nitems;
	     i++, y += (rad->radius*2 + YSPACING)) {
		primitives.circle(rad,
		    XPADDING + rad->radius,
		    y + rad->radius,
		    rad->radius,
		    OUTSIDE_COLOR);

		if (i == val) {
			primitives.circle(rad,
			    XPADDING + rad->radius,
			    y + rad->radius,
			    rad->radius/2,
			    INSIDE_COLOR);
		}

		widget_blit(rad, rad->labels[i], x, y);
	}
}

void
radio_destroy(void *p)
{
	struct radio *rad = p;
	int i;

	for (i = 0; i < rad->nitems; i++)
		SDL_FreeSurface(rad->labels[i]);

	widget_destroy(rad);
}

void
radio_scale(void *p, int rw, int rh)
{
	struct radio *rad = p;

	if (rw == -1)
		WIDGET(rad)->w = XPADDING*2 + XSPACING + rad->radius*2 +
		    rad->max_w;

	if (rh == -1)
		WIDGET(rad)->h = rad->nitems * (YSPACING + rad->radius*2) + 
		    YPADDING*2;
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

	valueb = widget_get_binding(rad, "value", &sel);
	switch (type) {
	case MOUSEBUTTONDOWN_EVENT:
		button = argv[2].i;
		y = argv[4].i;
		*sel = (y/(rad->radius*2 + YSPACING/2));
		widget_focus(rad);
		break;
	case KEYDOWN_EVENT:
		keysym = argv[2].i;
		switch ((SDLKey)keysym) {
		case SDLK_DOWN:
			if (++(*sel) > rad->nitems)
				*sel = 0;
			break;
		case SDLK_UP:
			if (--(*sel) < 0)
				*sel = 0;
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


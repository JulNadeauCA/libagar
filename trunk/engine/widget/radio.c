/*	$Csoft: radio.c,v 1.47 2005/02/19 07:05:37 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
	XPADDING =	3,
	YPADDING =	4,
	RADIUS =	6,
	SEL_RADIUS =	3
};

enum {
	MOUSEBUTTONDOWN_EVENT,
	KEYDOWN_EVENT
};

static void radio_event(int, union evarg *);
static void mousemotion(int, union evarg *);

struct radio *
radio_new(void *parent, const char **items)
{
	struct radio *rad;

	rad = Malloc(sizeof(struct radio), M_OBJECT);
	radio_init(rad, items);
	object_attach(parent, rad);
	return (rad);
}

void
radio_init(struct radio *rad, const char **items)
{
	const char *s, **itemsp = items;
	int i;

	widget_init(rad, "radio", &radio_ops, WIDGET_FOCUSABLE|WIDGET_WFILL);
	widget_bind(rad, "value", WIDGET_INT, &rad->value);

	rad->value = -1;
	rad->max_w = 0;
	rad->oversel = -1;

	for (rad->nitems = 0; (s = *itemsp++) != NULL; rad->nitems++)
		;;
	rad->labels = Malloc(sizeof(SDL_Surface *) * rad->nitems, M_WIDGET);
	for (i = 0; i < rad->nitems; i++) {
		rad->labels[i] = text_render(NULL, -1, COLOR(RADIO_TXT_COLOR),
		    _(items[i]));
		if (rad->labels[i]->w > rad->max_w)
			rad->max_w = rad->labels[i]->w;
	}

	event_new(rad, "window-mousebuttondown", radio_event, "%i",
	    MOUSEBUTTONDOWN_EVENT);
	event_new(rad, "window-keydown", radio_event, "%i", KEYDOWN_EVENT);
	event_new(rad, "window-mousemotion", mousemotion, NULL);
}

static const int highlight[18] = {
	-5, +1,
	-5,  0,
	-5, -1,
	-4, -2,
	-4, -3,
	-3, -4,
	-2, -4,
	 0, -5,
	-1, -5
};

void
radio_draw(void *p)
{
	struct radio *rad = p;
	int i, val, j;
	int x = XPADDING + RADIUS*2 + XSPACING;
	int y = YPADDING;

	primitives.frame(rad,
	    0,
	    0,
	    WIDGET(rad)->w,
	    WIDGET(rad)->h,
	    COLOR(FRAME_COLOR));

	val = widget_get_int(rad, "value");

	for (i = 0;
	     i < rad->nitems;
	     i++, y += (RADIUS*2 + YSPACING)) {
		int xc = XPADDING + RADIUS;
		int yc = y + RADIUS;

		for (j = 0; j < 18; j+=2) {
			widget_put_pixel(rad,
			    xc + highlight[j],
			    yc + highlight[j+1],
			    COLOR(RADIO_HI_COLOR));
			widget_put_pixel(rad,
			    xc - highlight[j],
			    yc - highlight[j+1],
			    COLOR(RADIO_LO_COLOR));
		}

		if (i == val) {
			primitives.circle(rad,
			    XPADDING + RADIUS,
			    y + RADIUS,
			    SEL_RADIUS,
			    COLOR(RADIO_SEL_COLOR));
		} else if (i == rad->oversel) {
			primitives.circle(rad,
			    XPADDING + RADIUS,
			    y + RADIUS,
			    SEL_RADIUS,
			    COLOR(RADIO_OVER_COLOR));
		}
		widget_blit(rad, rad->labels[i], x, y);
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
	Free(rad->labels, M_WIDGET);

	widget_destroy(rad);
}

void
radio_scale(void *p, int rw, int rh)
{
	struct radio *rad = p;

	if (rw == -1)
		WIDGET(rad)->w = XPADDING*2 + XSPACING + RADIUS*2 + rad->max_w;
	if (rh == -1)
		WIDGET(rad)->h = rad->nitems*(YSPACING + RADIUS*2) + YPADDING*2;
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct radio *rad = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i - YPADDING;

	rad->oversel = (y/(RADIUS*2 + YSPACING));
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
		if (button == SDL_BUTTON_LEFT) {
			y = argv[4].i - YPADDING;
			*sel = (y/(RADIUS*2 + YSPACING));
			widget_focus(rad);
		}
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
	event_post(NULL, rad, "radio-changed", "%i", *sel);
	widget_binding_unlock(valueb);
}


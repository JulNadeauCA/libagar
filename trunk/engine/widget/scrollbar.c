/*	$Csoft: scrollbar.c,v 1.39 2004/03/28 07:41:12 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include "scrollbar.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

const struct widget_ops scrollbar_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		widget_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	scrollbar_draw,
	scrollbar_scale
};

enum {
	BG_COLOR,
	BUTTON_COLOR
};

enum button_which {
	BUTTON_NONE,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_SCROLL
};

static void	scrollbar_mousebuttonup(int, union evarg *);
static void	scrollbar_mousebuttondown(int, union evarg *);
static void	scrollbar_mousemotion(int, union evarg *);

struct scrollbar *
scrollbar_new(void *parent, enum scrollbar_type type)
{
	struct scrollbar *sb;

	sb = Malloc(sizeof(struct scrollbar), M_OBJECT);
	scrollbar_init(sb, type);
	object_attach(parent, sb);
	return (sb);
}

void
scrollbar_init(struct scrollbar *sb, enum scrollbar_type type)
{
	widget_init(sb, "scrollbar", &scrollbar_ops,
	    WIDGET_FOCUSABLE|WIDGET_UNFOCUSED_BUTTONUP);
	widget_bind(sb, "value", WIDGET_INT, &sb->value);
	widget_bind(sb, "min", WIDGET_INT, &sb->min);
	widget_bind(sb, "max", WIDGET_INT, &sb->max);

	widget_map_color(sb, BG_COLOR, "background", 120, 120, 120, 255);
	widget_map_color(sb, BUTTON_COLOR, "button", 140, 140, 140, 255);

	sb->value = 0;
	sb->min = 0;
	sb->max = 0;
	sb->type = type;
	sb->curbutton = BUTTON_NONE;
	sb->bar_size = 30;
	sb->button_size = text_font_height(NULL);

	event_new(sb, "window-mousebuttondown", scrollbar_mousebuttondown,
	    NULL);
	event_new(sb, "window-mousebuttonup", scrollbar_mousebuttonup, NULL);
	event_new(sb, "window-mousemotion", scrollbar_mousemotion, NULL);
}

static void
scrollbar_mousebuttonup(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;

	sb->curbutton = BUTTON_NONE;
}

/* Effect mouse scrolling or click on the scrollbar. */
static void
scrollbar_mouse_select(struct scrollbar *sb, int coord, int maxcoord)
{
	int ncoord = coord - sb->bar_size/2;
	int max;

	if (sb->curbutton != BUTTON_SCROLL)
		return;
	if (sb->bar_size == -1)
		return;

	if (ncoord < sb->button_size) {				/* Minimum */
		struct widget_binding *valueb;
		int *value;

		valueb = widget_get_binding(sb, "value", &value);
		*value = widget_get_int(sb, "min");
		widget_binding_unlock(valueb);
		goto changed;
	}

	max = widget_get_int(sb, "max");

	if (ncoord > maxcoord - sb->button_size - sb->bar_size/2) {
		widget_set_int(sb, "value", max);
		goto changed;
	}

	ncoord -= sb->button_size;
	widget_set_int(sb, "value", ncoord * max / maxcoord);
changed:
	event_post(NULL, sb, "scrollbar-changed", "%i",
	    widget_get_int(sb, "value"));
}

static void
scrollbar_mousebuttondown(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int coord = (sb->type == SCROLLBAR_HORIZ) ?
	    argv[2].i : argv[3].i;
	int maxcoord = (sb->type == SCROLLBAR_HORIZ) ?
	    WIDGET(sb)->w : WIDGET(sb)->h;

	widget_focus(sb);

	if (coord < sb->button_size) {
		sb->curbutton = BUTTON_UP;
	} else if (coord > maxcoord - sb->button_size) {
		sb->curbutton = BUTTON_DOWN;
	} else {
		sb->curbutton = BUTTON_SCROLL;
		scrollbar_mouse_select(sb, coord, maxcoord);
	}
}

static void
scrollbar_mousemotion(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int coord = (sb->type == SCROLLBAR_HORIZ) ?
	    argv[1].i : argv[2].i;
	int state = argv[5].i;
	int maxcoord = (sb->type == SCROLLBAR_HORIZ) ?
	    WIDGET(sb)->w : WIDGET(sb)->h;

	if (state & SDL_BUTTON_LMASK)
		scrollbar_mouse_select(sb, coord, maxcoord);
}

void
scrollbar_scale(void *p, int rw, int rh)
{
	struct scrollbar *sb = p;

	switch (sb->type) {
	case SCROLLBAR_HORIZ:
		if (rw == -1)
			WIDGET(sb)->w = sb->button_size*4;
		if (rh == -1) {
			WIDGET(sb)->h = sb->button_size;
		} else {
			sb->button_size = WIDGET(sb)->h;	/* Square */
		}
		break;
	case SCROLLBAR_VERT:
		if (rw == -1) {
			WIDGET(sb)->w = sb->button_size;
		} else {
			sb->button_size = WIDGET(sb)->w;	/* Square */
		}
		if (rh == -1)
			WIDGET(sb)->h = sb->button_size*4;
		break;
	}
}

void
scrollbar_draw(void *p)
{
	struct scrollbar *sb = p;
	int value, min, max;
	int w, h, x, y;
	int maxcoord;
	
	switch (sb->type) {
	case SCROLLBAR_HORIZ:
		if (WIDGET(sb)->w < sb->button_size*2 + 6) {
			return;
		}
		maxcoord = WIDGET(sb)->w;
		break;
	case SCROLLBAR_VERT:
		if (WIDGET(sb)->h < sb->button_size*2 + 6) {
			return;
		}
		maxcoord = WIDGET(sb)->h;
		break;
	}

	value = widget_get_int(sb, "value");
	min = widget_get_int(sb, "min");
	max = widget_get_int(sb, "max");

#ifdef DEBUG
	if (min < 0 || max < min || value < min || value > max) {
		dprintf("out of range: min=%d, max=%d, value=%d\n", min, max,
		    value);
		return;
	}
#endif

	primitives.box(sb, 0, 0, WIDGET(sb)->w, WIDGET(sb)->h, -1, BG_COLOR);

	switch (sb->type) {
	case SCROLLBAR_VERT:
		primitives.box(sb,
		    0,
		    0,
		    WIDGET(sb)->w,
		    sb->button_size,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    BUTTON_COLOR);
		primitives.box(sb,
		    0,
		    WIDGET(sb)->h - sb->button_size,
		    WIDGET(sb)->w,
		    sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    BUTTON_COLOR);

		if (max > 0) {
			if (sb->bar_size == -1) {		/* Full range */
				h = WIDGET(sb)->h - sb->button_size*2;
			} else {
				h = sb->bar_size;
			}
			y = value * (WIDGET(sb)->h - sb->button_size) /
			    max;
		} else {
			h = WIDGET(sb)->h - sb->button_size*2;
			y = 0;
		}
		if (h < 4)
			h = 4;
		if (sb->button_size + h + y > WIDGET(sb)->h - sb->button_size)
			y = WIDGET(sb)->h - sb->button_size*2 - h;

		primitives.box(sb,
		    0,
		    sb->button_size + y,
		    WIDGET(sb)->w,
		    h,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    BUTTON_COLOR);
		break;
	case SCROLLBAR_HORIZ:
		primitives.box(sb,
		    0,
		    0,
		    sb->button_size,
		    WIDGET(sb)->h,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    BUTTON_COLOR);
		primitives.box(sb,
		    WIDGET(sb)->w - sb->button_size, 0,
		    WIDGET(sb)->h,
		    sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    BUTTON_COLOR);

		if (max > 0) {
			if (sb->bar_size == -1) {		/* Full range */
				w = WIDGET(sb)->w - sb->button_size*2;
			} else {
				w = sb->bar_size;
			}
			x = value * (WIDGET(sb)->w - sb->button_size) / max;
		} else {
			w = WIDGET(sb)->w - sb->button_size*2;
			x = 0;
		}
		if (w < 4)
			w = 4;
		if (sb->button_size + w + x > WIDGET(sb)->w - sb->button_size)
			x = WIDGET(sb)->w - sb->button_size*2 - w;

		primitives.box(sb,
		    sb->button_size + x,
		    0,
		    w,
		    WIDGET(sb)->h,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    BUTTON_COLOR);
		break;
	}
}

void
scrollbar_set_bar_size(struct scrollbar *sb, int bsize)
{
	sb->bar_size = bsize;
}

void
scrollbar_get_bar_size(struct scrollbar *sb, int *bsize)
{
	*bsize = sb->bar_size;
}

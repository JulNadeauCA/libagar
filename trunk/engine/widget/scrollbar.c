/*	$Csoft: scrollbar.c,v 1.21 2003/04/17 02:29:48 vedge Exp $	*/

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

#include "scrollbar.h"

#include <engine/widget/primitive.h>
#include <engine/widget/region.h>
#include <engine/widget/window.h>

static const struct widget_ops scrollbar_ops = {
	{
		widget_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	scrollbar_draw,
	NULL		/* update */
};

enum {
	BACKGROUND_COLOR,
	SCROLL_BUTTON_COLOR,
	SCROLL_TRIANGLE_COLOR1,
	SCROLL_TRIANGLE_COLOR2
};

enum button {
	BUTTON_NONE,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_SCROLL
};

static void	scrollbar_mouse_buttonup(int, union evarg *);
static void	scrollbar_mouse_buttondown(int, union evarg *);
static void	scrollbar_mouse_motion(int, union evarg *);
static void	scrollbar_scaled(int, union evarg *);

struct scrollbar *
scrollbar_new(struct region *reg, int w, int h,
    enum scrollbar_orientation orient)
{
	struct scrollbar *sb;

	sb = Malloc(sizeof(struct scrollbar));
	scrollbar_init(sb, w, h, orient);
	region_attach(reg, sb);
	return (sb);
}

void
scrollbar_init(struct scrollbar *sb, int w, int h,
    enum scrollbar_orientation orient)
{
	widget_init(&sb->wid, "scrollbar", &scrollbar_ops, w, h);
	widget_map_color(sb, BACKGROUND_COLOR, "background", 120, 120, 120);
	widget_map_color(sb, SCROLL_BUTTON_COLOR, "button", 140, 140, 140);
	widget_map_color(sb, SCROLL_TRIANGLE_COLOR1, "triangle", 160, 160, 160);
	widget_map_color(sb, SCROLL_TRIANGLE_COLOR2, "triangle2", 80, 80, 80);

	widget_bind(sb, "value", WIDGET_INT, &sb->def.lock, &sb->def.value);
	widget_bind(sb, "min", WIDGET_INT, &sb->def.lock, &sb->def.min);
	widget_bind(sb, "max", WIDGET_INT, &sb->def.lock, &sb->def.max);

	sb->orientation = orient;
	sb->curbutton = BUTTON_NONE;
	sb->bar_size = 30;
	sb->button_size = 25;

	sb->def.value = 0;
	sb->def.min = 0;
	sb->def.max = 0;
	pthread_mutex_init(&sb->def.lock, &recursive_mutexattr);

	event_new(sb, "window-mousebuttondown",
	    scrollbar_mouse_buttondown, NULL);
	event_new(sb, "window-mousebuttonup",
	    scrollbar_mouse_buttonup, NULL);
	event_new(sb, "window-mousemotion",
	    scrollbar_mouse_motion, NULL);
	event_new(sb, "widget-scaled",
	    scrollbar_scaled, NULL);
}

static void
scrollbar_mouse_buttonup(int argc, union evarg *argv)
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

		valueb = widget_binding_get_locked(sb, "value", &value);
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
	event_post(sb, "scrollbar-changed", "%i", widget_get_int(sb, "value"));
}

static void
scrollbar_mouse_buttondown(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int coord = (sb->orientation == SCROLLBAR_HORIZ) ?
	    argv[2].i : argv[3].i;
	int maxcoord = (sb->orientation == SCROLLBAR_HORIZ) ?
	    WIDGET(sb)->w : WIDGET(sb)->h;

	WIDGET_FOCUS(sb);

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
scrollbar_mouse_motion(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int coord = (sb->orientation == SCROLLBAR_HORIZ) ?
	    argv[1].i : argv[2].i;
	int maxcoord = (sb->orientation == SCROLLBAR_HORIZ) ?
	    WIDGET(sb)->w : WIDGET(sb)->h;
	
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) {
		scrollbar_mouse_select(sb, coord, maxcoord);
	}
}

static void
scrollbar_scaled(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;

	switch (sb->orientation) {
	case SCROLLBAR_HORIZ:
		sb->button_size = WIDGET(sb)->h;	/* Square */
		break;
	case SCROLLBAR_VERT:
		sb->button_size = WIDGET(sb)->w;	/* Square */
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
	
	switch (sb->orientation) {
	case SCROLLBAR_HORIZ:
		if (WIDGET(sb)->w < sb->button_size*2 + 6)
			return;
		maxcoord = WIDGET(sb)->w;
		break;
	case SCROLLBAR_VERT:
		if (WIDGET(sb)->h < sb->button_size*2 + 6)
			return;
		maxcoord = WIDGET(sb)->h;
		break;
	}

	value = widget_get_int(sb, "value");
	min =	widget_get_int(sb, "min");
	max =	widget_get_int(sb, "max");

#ifdef DEBUG
	if (min < 0 || max < min || value < min || value > max)
		fatal("bad value/range: min=%d, max=%d, value=%d",
		    min, max, value);
#endif

	primitives.box(sb, 0, 0, WIDGET(sb)->w, WIDGET(sb)->h, -1,
	    WIDGET_COLOR(sb, BACKGROUND_COLOR));

	switch (sb->orientation) {
	case SCROLLBAR_VERT:
		/* Scrolling buttons */
		primitives.box(sb, 0, 0, WIDGET(sb)->w, sb->button_size,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		primitives.box(sb, 0, WIDGET(sb)->h - sb->button_size,
		    WIDGET(sb)->w, sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));

		/* Scrolling bar */
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
		    0, sb->button_size+y,
		    WIDGET(sb)->w, h,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		break;
	case SCROLLBAR_HORIZ:
		primitives.box(sb, 0, 0,
		    sb->button_size, WIDGET(sb)->h,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		primitives.box(sb,
		    WIDGET(sb)->w - sb->button_size, 0,
		    WIDGET(sb)->h, sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));

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
		    sb->button_size+x, 0,
		    w, WIDGET(sb)->h,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		break;
	}
}

__inline__ void
scrollbar_set_bar_size(struct scrollbar *sb, int bsize)
{
	sb->bar_size = bsize;
}

__inline__ void
scrollbar_get_bar_size(struct scrollbar *sb, int *bsize)
{
	*bsize = sb->bar_size;
}

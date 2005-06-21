/*	$Csoft: scrollbar.c,v 1.48 2005/06/16 15:58:34 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
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

#include <engine/widget/scrollbar.h>

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

enum button_which {
	BUTTON_NONE,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_SCROLL
};

static void scrollbar_mousebuttonup(int, union evarg *);
static void scrollbar_mousebuttondown(int, union evarg *);
static void scrollbar_mousemotion(int, union evarg *);

struct scrollbar *
scrollbar_new(void *parent, enum scrollbar_type type)
{
	struct scrollbar *sb;

	sb = Malloc(sizeof(struct scrollbar), M_WIDGET);
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

	sb->value = 0;
	sb->min = 0;
	sb->max = 0;
	sb->type = type;
	sb->curbutton = BUTTON_NONE;
	sb->bar_size = 30;
	sb->button_size = text_font_height;

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

/* Clicked or dragged mouse to coord, so adjust value */
static void
scrollbar_mouse_select(struct scrollbar *sb, int coord, int totalsize)
{
	const int scrolling_area = totalsize - (sb->button_size*2);
	int min, max;

	if (sb->curbutton != BUTTON_SCROLL)
		return;
	if (sb->bar_size == -1)
		return;
		
	min = widget_get_int(sb, "min");
	max = widget_get_int(sb, "max");
	
	if (max < min)
		return;
	
	/* mouse below min */
	if (coord <= sb->button_size) {
		widget_set_int(sb, "value", min);
	}
	/* mouse above max */
	else if (coord >= sb->button_size + scrolling_area) {
		widget_set_int(sb, "value", max);
	}
	/* mouse between */
	else {
		int ncoord = coord - sb->button_size;

		widget_set_int(sb, "value", ncoord*(max-min+1)/scrolling_area);
	}
	
	/* generate an event */
	event_post(NULL, sb, "scrollbar-changed", "%i",
		widget_get_int(sb, "value"));
}

static void
scrollbar_mousebuttondown(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int button = argv[1].i;
	int coord = (sb->type == SCROLLBAR_HORIZ) ?
		argv[2].i : argv[3].i;
	int totalsize = (sb->type == SCROLLBAR_HORIZ) ?
		WIDGET(sb)->w : WIDGET(sb)->h;
	int min, value, max, nvalue;

	if (button != SDL_BUTTON_LEFT)
		return;

	min = widget_get_int(sb, "min");
	max = widget_get_int(sb, "max");
	value = widget_get_int(sb, "value");

	if (max < min)
		return;
	
	widget_focus(sb);
	
	/* click on the up button */
	if (coord <= sb->button_size) {
		sb->curbutton = BUTTON_UP;
		if (value > min) 
			widget_set_int(sb, "value", value - 1);
	}
	/* click on the down button */
	else if (coord >= totalsize - sb->button_size) {
		sb->curbutton = BUTTON_DOWN;
		if (value < max)
			widget_set_int(sb, "value", value + 1);
	}
	/* click in between */
	else {
		sb->curbutton = BUTTON_SCROLL;
		scrollbar_mouse_select(sb, coord, totalsize);
	}
	
	/* generate an event if value changed */
	if (value != (nvalue = widget_get_int(sb, "value")))
		event_post(NULL, sb, "scrollbar-changed", "%i", nvalue);
}

static void
scrollbar_mousemotion(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int coord = (sb->type == SCROLLBAR_HORIZ) ?
		argv[1].i : argv[2].i;
	int state = argv[5].i;
	int totalsize = (sb->type == SCROLLBAR_HORIZ) ?
		WIDGET(sb)->w : WIDGET(sb)->h;

	if (state & SDL_BUTTON_LMASK)
		scrollbar_mouse_select(sb, coord, totalsize);
}

void
scrollbar_scale(void *p, int rw, int rh)
{
	struct scrollbar *sb = p;

	switch (sb->type) {
	case SCROLLBAR_HORIZ:
		if (rw == -1) {
			WIDGET(sb)->w = sb->button_size*4;
		}
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
		if (rh == -1) {
			WIDGET(sb)->h = sb->button_size*4;
		}
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

	if (WIDGET(sb)->w < sb->button_size ||
	    WIDGET(sb)->w < sb->button_size)
		return;

	value = widget_get_int(sb, "value");
	min = widget_get_int(sb, "min");
	max = widget_get_int(sb, "max");
	
	if (max < min || max == 0)
		return;

#ifdef DEBUG
	if (value < min || value > max) {
		dprintf("invalid value: min=%d, value=%d, max=%d\n", min,
		    value, max);
		return;
	}
#endif

	primitives.box(sb, 0, 0, WIDGET(sb)->w, WIDGET(sb)->h, -1,
	    COLOR(SCROLLBAR_COLOR));

	switch (sb->type) {
	case SCROLLBAR_VERT:
		if (WIDGET(sb)->h < sb->button_size*2 + 6) {
			return;
		}
		maxcoord = WIDGET(sb)->h - sb->button_size * 2 - sb->bar_size;
		
		/* Draw the up and down buttons. */
		primitives.box(sb,
		    0, 0,
		    sb->button_size, sb->button_size,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    COLOR(SCROLLBAR_BTN_COLOR));
		primitives.box(sb,
		    0, WIDGET(sb)->h - sb->button_size,
		    sb->button_size,
		    sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    COLOR(SCROLLBAR_BTN_COLOR));
		
		/* Calculate disabled bar */
		if (sb->bar_size == -1) {
			y = 0;
			h = WIDGET(sb)->h - sb->button_size*2;
		}
		/* Calculate active bar */
		else {
			y = value * maxcoord / (max-min);
			h = sb->bar_size;
			if (sb->button_size + y + h >
			    WIDGET(sb)->h - sb->button_size)
				y = WIDGET(sb)->h - sb->button_size*2 - h;
		}
		/* Draw bar */
		primitives.box(sb,
		    0, sb->button_size + y,
		    sb->button_size,
		    h,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    COLOR(SCROLLBAR_BTN_COLOR));
		break;
	case SCROLLBAR_HORIZ:
		if (WIDGET(sb)->w < sb->button_size*2 + 6) {
			return;
		}
		maxcoord = WIDGET(sb)->w - sb->button_size*2 - sb->bar_size;

		/* Draw the up and down buttons */
		primitives.box(sb,
		    0, 0,
		    sb->button_size, sb->button_size,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    COLOR(SCROLLBAR_BTN_COLOR));
		primitives.box(sb,
		    WIDGET(sb)->w - sb->button_size, 0,
		    sb->button_size, sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    COLOR(SCROLLBAR_BTN_COLOR));
		
		/* Calculate disabled bar */
		if (sb->bar_size == -1) {
			x = 0;
			w = WIDGET(sb)->w - sb->button_size*2;
		}
		/* Calculate active bar */
		else {
			x = value * maxcoord / (max-min);
			w = sb->bar_size;
			if (sb->button_size + x + w >
			    WIDGET(sb)->w - sb->button_size)
				x = WIDGET(sb)->w - sb->button_size*2 - w;
		}
		
		/* Draw bar */
		primitives.box(sb,
		    sb->button_size + x,
		    0,
		    w,
		    sb->button_size,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    COLOR(SCROLLBAR_BTN_COLOR));
		break;
	}
#if 0
	{
		SDL_Surface *txt;
		char label[32];

		snprintf(label, sizeof(label), "%d\n%d\n%d\n",
		    value, min, max);
		txt = text_render(NULL, -1, COLOR(TEXT_COLOR), label);
		widget_blit(sb, txt, 0, 0);
		SDL_FreeSurface(txt);
		    
	}
#endif
}

void
scrollbar_set_bar_size(struct scrollbar *sb, int bsize)
{
	sb->bar_size = (bsize > 10 || bsize == -1) ? bsize : 10;
}

void
scrollbar_get_bar_size(struct scrollbar *sb, int *bsize)
{
	*bsize = sb->bar_size;
}

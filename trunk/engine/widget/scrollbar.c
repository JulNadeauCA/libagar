/*	$Csoft: scrollbar.c,v 1.11 2002/12/17 06:45:39 vedge Exp $	*/

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

#include "widget.h"
#include "window.h"
#include "scrollbar.h"
#include "primitive.h"

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
static void	scrollbar_triangle(struct scrollbar *, int, int, int, Uint32);

struct scrollbar *
scrollbar_new(struct region *reg, int w, int h, int flags)
{
	struct scrollbar *sb;

	sb = emalloc(sizeof(struct scrollbar));
	scrollbar_init(sb, w, h, flags);

	region_attach(reg, sb);

	return (sb);
}

void
scrollbar_init(struct scrollbar *sb, int w, int h, int flags)
{
	widget_init(&sb->wid, "scrollbar", "widget", &scrollbar_ops, w, h);
	widget_map_color(sb, BACKGROUND_COLOR, "scrollbar-background",
	    120, 120, 120);
	widget_map_color(sb, SCROLL_BUTTON_COLOR, "scrollbar-button",
	    140, 140, 140);
	widget_map_color(sb, SCROLL_TRIANGLE_COLOR1, "scrollbar-triangle1",
	    160, 160, 160);
	widget_map_color(sb, SCROLL_TRIANGLE_COLOR2, "scrollbar-triangle2",
	    80, 80, 80);

	sb->flags = flags;
	sb->min = 0;
	sb->max = 0;
	sb->value = 0;
	sb->curbutton = BUTTON_NONE;
	sb->bar_size = 30;
	sb->button_size = 25;
	pthread_mutex_init(&sb->lock, NULL);

	event_new(sb, "window-mousebuttondown",
	    scrollbar_mouse_buttondown, NULL);
	event_new(sb, "window-mousebuttonup",
	    scrollbar_mouse_buttonup, NULL);
	event_new(sb, "window-mousemotion",
	    scrollbar_mouse_motion, NULL);
	event_new(sb, "widget-scaled",
	    scrollbar_scaled, NULL);
}

void
scrollbar_set_range(struct scrollbar *sb, int min, int max)
{
	pthread_mutex_lock(&sb->lock);
	sb->min = min;
	sb->max = max;
	pthread_mutex_unlock(&sb->lock);
}

void
scrollbar_get_range(struct scrollbar *sb, int *min, int *max)
{
	pthread_mutex_lock(&sb->lock);
	*min = sb->min;
	*max = sb->max;
	pthread_mutex_unlock(&sb->lock);
}

void
scrollbar_set_value(struct scrollbar *sb, int value)
{
	pthread_mutex_lock(&sb->lock);
	sb->value = value;
	pthread_mutex_unlock(&sb->lock);
}

void
scrollbar_get_value(struct scrollbar *sb, int *value)
{
	pthread_mutex_lock(&sb->lock);
	*value = sb->value;
	pthread_mutex_unlock(&sb->lock);
}

static void
scrollbar_mouse_buttonup(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;

	sb->curbutton = BUTTON_NONE;
}

static void
scrollbar_mouse_select(struct scrollbar *sb, int y)
{
	int ny = y - sb->bar_size/2;

	if (sb->curbutton != BUTTON_SCROLL) {		/* Not scrolling? */
		return;
	}
	if (sb->bar_size == -1) {			/* Full range? */
		return;
	}
	if (ny < sb->button_size) {			/* Up button? */
		scrollbar_set_value(sb, 0);
		return;
	}
	if (ny > WIDGET(sb)->h - sb->bar_size -		/* Down button? */
	    sb->button_size) {
		scrollbar_set_value(sb, sb->max);
		return;
	}

	pthread_mutex_lock(&sb->lock);
	ny -= sb->button_size;
	sb->value = ny * sb->max / WIDGET(sb)->h;
	pthread_mutex_unlock(&sb->lock);
	
	event_post(sb, "scrollbar-changed", "%i", sb->value);
}

static void
scrollbar_mouse_buttondown(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	if (y < sb->button_size) {
		sb->curbutton = BUTTON_UP;
	} else if (y > WIDGET(sb)->h - sb->button_size) {
		sb->curbutton = BUTTON_DOWN;
	} else {
		sb->curbutton = BUTTON_SCROLL;
		scrollbar_mouse_select(sb, y);
	}
}

static void
scrollbar_mouse_motion(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;

	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) {
		scrollbar_mouse_select(sb, y);
	}
}

static void
scrollbar_scaled(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;

	sb->button_size = WIDGET(sb)->w;		/* XXX square */
}

enum {
	TRIANGLE_LEFT,
	TRIANGLE_RIGHT,
	TRIANGLE_UP,
	TRIANGLE_DOWN
};

static void
scrollbar_triangle(struct scrollbar *sb, int xoffs, int yoffs, int orientation,
    Uint32 color)
{
	int bh = sb->button_size;

	switch (orientation) {
	case TRIANGLE_UP:
		primitives.line(sb,
		    xoffs + bh/2, yoffs + 2,
		    xoffs + bh - 2, yoffs + bh - bh/2,
		    color);
		primitives.line(sb,
		    xoffs + 2, yoffs + bh - bh/2,
		    xoffs + bh/2, yoffs + 2,
		    color);
		primitives.line(sb,
		    xoffs + 2, yoffs + bh - bh/2,
		    xoffs + bh - 2, yoffs + bh - bh/2,
		    color);
		break;
	case TRIANGLE_DOWN:
		primitives.line(sb,
		    xoffs + 2, yoffs + 2,
		    xoffs + bh - 2, yoffs + 2,
		    color);
		primitives.line(sb,
		    xoffs + 2, yoffs + 2,
		    xoffs + bh/2, yoffs + bh - bh/2,
		    color);
		primitives.line(sb,
		    xoffs + bh/2, yoffs + bh - bh/2,
		    xoffs + bh - 2, yoffs + 2,
		    color);
		break;
	}
}

void
scrollbar_draw(void *p)
{
	struct scrollbar *sb = p;
	int h, y;

	if (WIDGET(sb)->h < sb->button_size*2) {
		return;
	}

	primitives.box(sb, 0, 0, WIDGET(sb)->w, WIDGET(sb)->h, -1,
	    WIDGET_COLOR(sb, BACKGROUND_COLOR));

	if (sb->flags & SCROLLBAR_VERTICAL) {
		/* Scrolling buttons */
		primitives.box(sb, 0, 0, WIDGET(sb)->w, sb->button_size,
		    (sb->curbutton == BUTTON_UP) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		scrollbar_triangle(sb, -1, 2, TRIANGLE_UP,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR1));
		scrollbar_triangle(sb, 0, 3, TRIANGLE_UP,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR2));
		primitives.box(sb, 0, WIDGET(sb)->h - sb->button_size,
		    WIDGET(sb)->w, sb->button_size,
		    (sb->curbutton == BUTTON_DOWN) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));

		/* XXX centering */
		scrollbar_triangle(sb, -1, WIDGET(sb)->h - 17, TRIANGLE_DOWN,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR1));
		scrollbar_triangle(sb, 0, WIDGET(sb)->h - 16, TRIANGLE_DOWN,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR2));

		/* Scrolling bar */
		pthread_mutex_lock(&sb->lock);
		if (sb->max > 0) {
			if (sb->bar_size < 0) {
				h = WIDGET(sb)->h - sb->button_size*2;
			} else {
				h = sb->bar_size;
			}
			y = sb->value * (WIDGET(sb)->h - sb->button_size) /
			    sb->max;
		} else {
			h = WIDGET(sb)->h - sb->button_size*2;
			y = 0;
		}
		
		if (h < sb->button_size) {
			h = sb->button_size;
		}
		pthread_mutex_unlock(&sb->lock);

		if (sb->button_size + h + y > WIDGET(sb)->h - sb->button_size) {
			y = WIDGET(sb)->h - sb->button_size*2 - h;
		}

		primitives.box(sb,
		    0, sb->button_size+y,
		    WIDGET(sb)->w, h,
		    (sb->curbutton == BUTTON_SCROLL) ? -1 : 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
	}
}


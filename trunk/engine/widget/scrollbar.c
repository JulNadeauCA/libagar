/*	$Csoft: scrollbar.c,v 1.27 2002/09/06 01:28:47 vedge Exp $	*/

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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>

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
	MIN_SB_HEIGHT =	10
};

enum {
	BACKGROUND_COLOR,
	SCROLL_BUTTON_COLOR,
	SCROLL_TRIANGLE_COLOR1,
	SCROLL_TRIANGLE_COLOR2
};

static void	scrollbar_scaled(int, union evarg *);
static void	scrollbar_mouse_button(int, union evarg *);
static void	scrollbar_mouse_motion(int, union evarg *);
static void	scrollbar_triangle(struct scrollbar *, int, int, int, Uint32);

struct scrollbar *
scrollbar_new(struct region *reg, int w, int h, int item_size, int flags)
{
	struct scrollbar *sb;

	sb = emalloc(sizeof(struct scrollbar));
	scrollbar_init(sb, w, h, item_size, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, sb);
	pthread_mutex_unlock(&reg->win->lock);

	return (sb);
}

void
scrollbar_init(struct scrollbar *sb, int w, int h, int item_size, int flags)
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
	sb->item_size = item_size;
	sb->range.soft_start = 0;
	sb->range.start = 0;
	sb->range.max = 100;

	event_new(sb, "window-mousebuttondown", 0,
	    scrollbar_mouse_button, NULL);
	event_new(sb, "window-mousemotion", 0,
	    scrollbar_mouse_motion, NULL);
}

void
scrollbar_set_range(struct scrollbar *sb, int max)
{
	sb->range.max = max;
}

static void
scrollbar_mouse_button(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	WIDGET_FOCUS(sb);
}

static void
scrollbar_mouse_motion(int argc, union evarg *argv)
{
	struct scrollbar *sb = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;

	if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) == 0) {
		return;
	}
	
	dprintf("x, y: %d,%d; xrel, yrel: %d,%d\n", x, y, xrel, yrel);
	
	if (yrel < 0 && (sb->range.soft_start += yrel) < 0) {
		sb->range.soft_start = sb->item_size;
		if (++sb->range.start > sb->range.max) {
			sb->range.start = sb->range.max;
		}
	}
	if (yrel > 0 && sb->range.start > 0 &&
	    (sb->range.soft_start += yrel) > sb->item_size) {
		sb->range.soft_start = 0;
		if (--sb->range.start < 0) {
			sb->range.start = 0;
		}
	}

	event_post(sb, "scrollbar-changed", "%i", sb->range.start);
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
	switch (orientation) {
	case TRIANGLE_UP:
		primitives.line(sb,
		    xoffs + 20/2,	yoffs + 2,
		    xoffs + 20 - 2,	yoffs + 20 - 10,
		    color);
		primitives.line(sb,
		    xoffs + 2,		yoffs + 20 - 10,
		    xoffs + 20/2,	yoffs + 2,
		    color);
		primitives.line(sb,
		    xoffs + 2,		yoffs + 20 - 10,
		    xoffs + 20 - 2,	yoffs + 20 - 10,
		    color);
		break;
	case TRIANGLE_DOWN:
		primitives.line(sb,
		    xoffs + 2,		yoffs + 2,
		    xoffs + 20 - 2,	yoffs + 2,
		    color);
		primitives.line(sb,
		    xoffs + 2,		yoffs + 2,
		    xoffs + 20/2,	yoffs + 20 - 10,
		    color);
		primitives.line(sb,
		    xoffs + 20/2,	yoffs + 20 - 10,
		    xoffs + 20 - 2,	yoffs + 2,
		    color);
		break;
	}
}

void
scrollbar_draw(void *p)
{
	struct scrollbar *sb = p;
	int h, y;

	primitives.box(sb, 0, 0, WIDGET(sb)->w, WIDGET(sb)->h, -1,
	    WIDGET_COLOR(sb, BACKGROUND_COLOR));

	if (sb->flags & SCROLLBAR_VERTICAL) {
		/*
		 * Scrolling buttons
		 */
		primitives.box(sb, 0, 0, WIDGET(sb)->w, 20, 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		scrollbar_triangle(sb, -1, 2, TRIANGLE_UP,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR1));
		scrollbar_triangle(sb, 0, 3, TRIANGLE_UP,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR2));
		primitives.box(sb, 0, WIDGET(sb)->h - 20, WIDGET(sb)->w, 20, 1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
		scrollbar_triangle(sb, -1, WIDGET(sb)->h - 17, TRIANGLE_DOWN,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR1));
		scrollbar_triangle(sb, 0, WIDGET(sb)->h - 16, TRIANGLE_DOWN,
		    WIDGET_COLOR(sb, SCROLL_TRIANGLE_COLOR2));

		/*
		 * Scrolling bar
		 */

		/* Calculate height */
		h = sb->item_size*sb->range.max - (WIDGET(sb)->h - 40);
		if (h < 16) {
			h = WIDGET(sb)->h - 40;
		} else {
			h = WIDGET(sb)->h - h - 40;	/* XXX */
		}
		if (h < MIN_SB_HEIGHT) {
			h = MIN_SB_HEIGHT;
		}

		/* Calculate position */
		y = (sb->range.start * sb->item_size) *
		    (sb->range.max * sb->item_size) / (WIDGET(sb)->h + 40);

		if (20 + h + y > WIDGET(sb)->h - 20) {
			y = WIDGET(sb)->h - 40 - h;
		}

		primitives.box(sb,
		    0, 20+y,
		    WIDGET(sb)->w, h,
		    1,
		    WIDGET_COLOR(sb, SCROLL_BUTTON_COLOR));
	}
}


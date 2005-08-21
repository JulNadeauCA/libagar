/*	$Csoft: vpane.c,v 1.4 2005/06/15 03:36:51 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include "vpane.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static struct widget_ops vpane_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		box_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	vpane_draw,
	vpane_scale
};

struct vpane *
vpane_new(void *parent, int flags)
{
	struct vpane *pa;

	pa = Malloc(sizeof(struct vpane), M_OBJECT);
	vpane_init(pa, flags);
	object_attach(parent, pa);
	return (pa);
}

struct vpane_div *
vpane_add_div(struct vpane *pa, enum box_type b1type, int b1flags,
    enum box_type b2type, int b2flags)
{
	struct vpane_div *div;

	div = Malloc(sizeof(struct vpane_div), M_WIDGET);
	div->moving = 0;
	div->y = 0;
	div->box1 = box_new(pa, b1type, b1flags);
	div->box2 = box_new(pa, b2type, b2flags);

	box_set_padding(div->box1, 0);
	box_set_padding(div->box2, 0);
	box_set_spacing(div->box1, 0);
	box_set_spacing(div->box2, 0);

	TAILQ_INSERT_TAIL(&pa->divs, div, divs);
	return (div);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct vpane *pa = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct vpane_div *div;
	
	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (y > div->y-4 && y < div->y+4) {
			div->moving = 1;
			WIDGET(pa)->flags |= WIDGET_UNFOCUSED_MOTION;
		} else {
			div->moving = 0;
		}
	}
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct vpane *pa = argv[0].p;
	struct window *pwin;
	struct vpane_div *div;
	int rx = argv[3].i;
	int ry = argv[4].i;
	
	dprintf("mot %d,%d\n", rx, ry);

	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (div->moving) {
			struct widget *w1 = WIDGET(div->box1);
			struct widget *w2 = WIDGET(div->box2);

			div->y += ry;
			if (div->y < 8) {
				div->y = 8;
				break;
			} else if (div->y > WIDGET(pa)->h-8) {
				div->y = WIDGET(pa)->h-8;
				break;
			}

			w1->h += ry;
			WIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
	
			w2->y += ry;
			w2->h -= ry;
			WIDGET_OPS(w2)->scale(w2, w2->w, w2->h);

			if ((pwin = widget_parent_window(pa)) != NULL) {
				widget_update_coords(pwin, WIDGET(pwin)->x,
				    WIDGET(pwin)->y);
			}
			break;
		}
	}
	TAILQ_FOREACH(div, &pa->divs, divs)
		div->y = WIDGET(div->box2)->y - 5;
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	struct vpane *pa = argv[0].p;
	struct vpane_div *div;

	TAILQ_FOREACH(div, &pa->divs, divs)
		div->moving = 0;
	
	WIDGET(pa)->flags &= ~WIDGET_UNFOCUSED_MOTION;
}

void
vpane_init(struct vpane *pa, int flags)
{
	int boxflags = 0;

	if (flags & VPANE_WFILL) boxflags |= BOX_WFILL;
	if (flags & VPANE_HFILL) boxflags |= BOX_HFILL;

	box_init(&pa->box, BOX_VERT, boxflags);
	box_set_padding(&pa->box, 0);
	box_set_spacing(&pa->box, 0);
	WIDGET(pa)->flags |= WIDGET_UNFOCUSED_BUTTONUP;
	object_set_ops(pa, &vpane_ops);
	TAILQ_INIT(&pa->divs);

	event_new(pa, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(pa, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(pa, "window-mousemotion", mousemotion, NULL);
}

void
vpane_draw(void *p)
{
	struct vpane *pa = p;
	struct widget *wid;
	struct vpane_div *div;
	Uint32 c;
	int x = WIDGET(pa)->w >> 1;

	TAILQ_FOREACH(div, &pa->divs, divs) {
		primitives.box(pa, 0, div->y-3, WIDGET(pa)->w, 7,
		    div->moving ? -1 : 1, COLOR(PANE_COLOR));
		widget_put_pixel(pa, x, div->y, COLOR(PANE_CIRCLE_COLOR));
		widget_put_pixel(pa, x - 5, div->y, COLOR(PANE_CIRCLE_COLOR));
		widget_put_pixel(pa, x + 5, div->y, COLOR(PANE_CIRCLE_COLOR));
	}
}

void
vpane_scale(void *p, int w, int h)
{
	struct vpane *pa = p;
	struct vpane_div *div;
	struct widget *wid;

	OBJECT_FOREACH_CHILD(wid, pa, widget)
		WIDGET_OPS(wid)->scale(wid, w, h);

	box_scale(pa, w, h);

	TAILQ_FOREACH(div, &pa->divs, divs) {
		WIDGET(div->box1)->h -= 5;
		WIDGET(div->box2)->y += 5;
		WIDGET(div->box2)->h -= 5;
		WIDGET_OPS(div->box1)->scale(div->box1,
		    WIDGET(div->box1)->w,
		    WIDGET(div->box1)->h);
		WIDGET_OPS(div->box2)->scale(div->box2,
		    WIDGET(div->box2)->w,
		    WIDGET(div->box2)->h);

		div->y = WIDGET(div->box2)->y - 5;
	}
}

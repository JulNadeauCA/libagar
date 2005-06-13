/*	$Csoft: hpane.c,v 1.1 2005/06/10 02:02:47 vedge Exp $	*/

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

#include "hpane.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static struct widget_ops hpane_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		box_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	hpane_draw,
	hpane_scale
};

struct hpane *
hpane_new(void *parent, int flags)
{
	struct hpane *pa;

	pa = Malloc(sizeof(struct hpane), M_OBJECT);
	hpane_init(pa, flags);
	object_attach(parent, pa);
	return (pa);
}

struct hpane_div *
hpane_add_div(struct hpane *pa, enum box_type b1type, int b1flags,
    enum box_type b2type, int b2flags)
{
	struct hpane_div *div;

	div = Malloc(sizeof(struct hpane_div), M_WIDGET);
	div->moving = 0;
	div->x = 0;
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
	struct hpane *pa = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct hpane_div *div;
	
	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (x > div->x-2 && x < div->x+2) {
			div->moving = 1;
		} else {
			div->moving = 0;
		}
	}
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct hpane *pa = argv[0].p;
	struct window *pwin;
	struct hpane_div *div;
	int rx = argv[3].i;
	int ry = argv[4].i;

	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (div->moving) {
			struct widget *w1 = WIDGET(div->box1);
			struct widget *w2 = WIDGET(div->box2);

			div->x += rx;
			if (div->x < 8) {
				div->x = 8;
				break;
			} else if (div->x > WIDGET(pa)->w-8) {
				div->x = WIDGET(pa)->w-8;
				break;
			}

			w1->w += rx;
			WIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
	
			w2->x += rx;
			w2->w -= rx;
			WIDGET_OPS(w2)->scale(w2, w2->w, w2->h);

			if ((pwin = widget_parent_window(pa)) != NULL) {
				widget_update_coords(pwin, WIDGET(pwin)->x,
				    WIDGET(pwin)->y);
			}
			break;
		}
	}
	TAILQ_FOREACH(div, &pa->divs, divs)
		div->x = WIDGET(div->box2)->x - 5;
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	struct hpane *pa = argv[0].p;
	struct hpane_div *div;

	TAILQ_FOREACH(div, &pa->divs, divs)
		div->moving = 0;
}

void
hpane_init(struct hpane *pa, int flags)
{
	int boxflags = 0;

	if (flags & HPANE_WFILL) boxflags |= BOX_WFILL;
	if (flags & HPANE_HFILL) boxflags |= BOX_HFILL;

	box_init(&pa->box, BOX_HORIZ, boxflags);
	box_set_padding(&pa->box, 0);
	box_set_spacing(&pa->box, 0);
	WIDGET(pa)->flags |= WIDGET_UNFOCUSED_BUTTONUP;
	object_set_ops(pa, &hpane_ops);
	TAILQ_INIT(&pa->divs);

	event_new(pa, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(pa, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(pa, "window-mousemotion", mousemotion, NULL);
}

void
hpane_draw(void *p)
{
	struct hpane *pa = p;
	struct widget *wid;
	struct hpane_div *div;
	Uint32 c;

	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (div->moving) {
			c = COLOR(PANE_MOVING_COLOR);
		} else {
			c = COLOR(PANE_COLOR);
		}
		primitives.vline(pa, div->x-1, 0, WIDGET(pa)->h, c);
		primitives.vline(pa, div->x+0, 0, WIDGET(pa)->h, c);
		primitives.vline(pa, div->x+1, 0, WIDGET(pa)->h, c);
	}
}

void
hpane_scale(void *p, int w, int h)
{
	struct hpane *pa = p;
	struct hpane_div *div;
	struct widget *wid;

	OBJECT_FOREACH_CHILD(wid, pa, widget)
		WIDGET_OPS(wid)->scale(wid, w, h);

	box_scale(pa, w, h);

	TAILQ_FOREACH(div, &pa->divs, divs) {
		WIDGET(div->box1)->w -= 5;
		WIDGET(div->box2)->x += 5;
		WIDGET(div->box2)->w -= 5;
		WIDGET_OPS(div->box1)->scale(div->box1,
		    WIDGET(div->box1)->w,
		    WIDGET(div->box1)->h);
		WIDGET_OPS(div->box2)->scale(div->box2,
		    WIDGET(div->box2)->w,
		    WIDGET(div->box2)->h);

		div->x = WIDGET(div->box2)->x - 5;
	}
}

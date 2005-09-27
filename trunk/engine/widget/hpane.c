/*	$Csoft: hpane.c,v 1.7 2005/09/19 06:54:07 vedge Exp $	*/

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

static AG_WidgetOps hpane_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_HPaneDraw,
	AG_HPaneScale
};

AG_HPane *
AG_HPaneNew(void *parent, int flags)
{
	AG_HPane *pa;

	pa = Malloc(sizeof(AG_HPane), M_OBJECT);
	AG_HPaneInit(pa, flags);
	AG_ObjectAttach(parent, pa);
	return (pa);
}

AG_HPaneDiv *
AG_HPaneAddDiv(AG_HPane *pa, enum ag_box_type b1type, int b1flags,
    enum ag_box_type b2type, int b2flags)
{
	AG_HPaneDiv *div;

	div = Malloc(sizeof(AG_HPaneDiv), M_WIDGET);
	div->moving = 0;
	div->x = 0;
	div->box1 = AG_BoxNew(pa, b1type, b1flags);
	div->box2 = AG_BoxNew(pa, b2type, b2flags);

	AG_BoxSetPadding(div->box1, 0);
	AG_BoxSetPadding(div->box2, 0);
	AG_BoxSetSpacing(div->box1, 0);
	AG_BoxSetSpacing(div->box2, 0);

	TAILQ_INSERT_TAIL(&pa->divs, div, divs);
	return (div);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	AG_HPane *pa = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	AG_HPaneDiv *div;
	
	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (x > div->x-4 && x < div->x+4) {
			div->moving = 1;
			AGWIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_MOTION;
		} else {
			div->moving = 0;
		}
	}
}

static void
mousemotion(int argc, union evarg *argv)
{
	AG_HPane *pa = argv[0].p;
	AG_Window *pwin;
	AG_HPaneDiv *div;
	int rx = argv[3].i;
	int ry = argv[4].i;

	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (div->moving) {
			AG_Widget *w1 = AGWIDGET(div->box1);
			AG_Widget *w2 = AGWIDGET(div->box2);

			div->x += rx;
			if (div->x < 8) {
				div->x = 8;
				break;
			} else if (div->x > AGWIDGET(pa)->w-8) {
				div->x = AGWIDGET(pa)->w-8;
				break;
			}

			w1->w += rx;
			AGWIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
	
			w2->x += rx;
			w2->w -= rx;
			AGWIDGET_OPS(w2)->scale(w2, w2->w, w2->h);

			if ((pwin = AG_WidgetParentWindow(pa)) != NULL) {
				AG_WidgetUpdateCoords(pwin, AGWIDGET(pwin)->x,
				    AGWIDGET(pwin)->y);
			}
			break;
		}
	}
	TAILQ_FOREACH(div, &pa->divs, divs)
		div->x = AGWIDGET(div->box2)->x - 5;
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	AG_HPane *pa = argv[0].p;
	AG_HPaneDiv *div;

	TAILQ_FOREACH(div, &pa->divs, divs)
		div->moving = 0;
	
	AGWIDGET(pa)->flags &= ~AG_WIDGET_UNFOCUSED_MOTION;
}

void
AG_HPaneInit(AG_HPane *pa, int flags)
{
	int boxflags = 0;

	if (flags & AG_HPANE_WFILL) boxflags |= AG_BOX_WFILL;
	if (flags & AG_HPANE_HFILL) boxflags |= AG_BOX_HFILL;

	AG_BoxInit(&pa->box, AG_BOX_HORIZ, boxflags);
	AG_BoxSetPadding(&pa->box, 0);
	AG_BoxSetSpacing(&pa->box, 0);
	AGWIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;
	AG_ObjectSetOps(pa, &hpane_ops);
	TAILQ_INIT(&pa->divs);

	AG_SetEvent(pa, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(pa, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(pa, "window-mousemotion", mousemotion, NULL);
}

void
AG_HPaneDraw(void *p)
{
	AG_HPane *pa = p;
	AG_Widget *wid;
	AG_HPaneDiv *div;
	Uint32 c;
	int y = AGWIDGET(pa)->h >> 1;

	TAILQ_FOREACH(div, &pa->divs, divs) {
		agPrim.box(pa, div->x-3, 0, 7, AGWIDGET(pa)->h,
		    div->moving ? -1 : 1, AG_COLOR(PANE_COLOR));
		AG_WidgetPutPixel(pa, div->x, y, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, div->x, y-5, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, div->x, y+5, AG_COLOR(PANE_CIRCLE_COLOR));
	}
}

void
AG_HPaneScale(void *p, int w, int h)
{
	AG_HPane *pa = p;
	AG_HPaneDiv *div;
	AG_Widget *wid;

	AGOBJECT_FOREACH_CHILD(wid, pa, ag_widget)
		AGWIDGET_OPS(wid)->scale(wid, w, h);

	AG_BoxScale(pa, w, h);

	TAILQ_FOREACH(div, &pa->divs, divs) {
		AGWIDGET(div->box1)->w -= 5;
		AGWIDGET(div->box2)->x += 5;
		AGWIDGET(div->box2)->w -= 5;
		AGWIDGET_OPS(div->box1)->scale(div->box1,
		    AGWIDGET(div->box1)->w,
		    AGWIDGET(div->box1)->h);
		AGWIDGET_OPS(div->box2)->scale(div->box2,
		    AGWIDGET(div->box2)->w,
		    AGWIDGET(div->box2)->h);

		div->x = AGWIDGET(div->box2)->x - 5;
	}
}

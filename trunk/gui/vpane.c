/*	$Csoft: vpane.c,v 1.6 2005/10/07 01:53:12 vedge Exp $	*/

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
#include <engine/widget/cursors.h>

static AG_WidgetOps agVPaneOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_VPaneDraw,
	AG_VPaneScale
};

AG_VPane *
AG_VPaneNew(void *parent, int flags)
{
	AG_VPane *pa;

	pa = Malloc(sizeof(AG_VPane), M_OBJECT);
	AG_VPaneInit(pa, flags);
	AG_ObjectAttach(parent, pa);
	return (pa);
}

AG_VPaneDiv *
AG_VPaneAddDiv(AG_VPane *pa, enum ag_box_type b1type, int b1flags,
    enum ag_box_type b2type, int b2flags)
{
	AG_VPaneDiv *div;

	div = Malloc(sizeof(AG_VPaneDiv), M_WIDGET);
	div->moving = 0;
	div->y = 0;
	div->box1 = AG_BoxNew(pa, b1type, b1flags);
	div->box2 = AG_BoxNew(pa, b2type, b2flags);

	AG_BoxSetPadding(div->box1, 0);
	AG_BoxSetPadding(div->box2, 0);
	AG_BoxSetSpacing(div->box1, 0);
	AG_BoxSetSpacing(div->box2, 0);

	TAILQ_INSERT_TAIL(&pa->divs, div, divs);
	return (div);
}

static __inline__ int
OverDivControl(AG_VPaneDiv *div, int y)
{
	return (y >= (div->y - 4) && y <= (div->y + 4));
}

static void
mousebuttondown(AG_Event *event)
{
	AG_VPane *pa = AG_SELF();
	int button = AG_INT(1);
	int y = AG_INT(3);
	AG_VPaneDiv *div;
	
	TAILQ_FOREACH(div, &pa->divs, divs)
		div->moving = OverDivControl(div, y);
}

static void
mousemotion(AG_Event *event)
{
	AG_VPane *pa = AG_SELF();
	AG_Window *pwin;
	AG_VPaneDiv *div;
	int x = AG_INT(1);
	int y = AG_INT(2);
	int ry;
	
	if (x < 0 || x > AGWIDGET(pa)->w) {
		return;
	}
	ry = AG_INT(4);
	
	TAILQ_FOREACH(div, &pa->divs, divs) {
		if (div->moving) {
			AG_Widget *w1 = AGWIDGET(div->box1);
			AG_Widget *w2 = AGWIDGET(div->box2);

			div->y += ry;
			if (div->y < 8) {
				div->y = 8;
				break;
			} else if (div->y > AGWIDGET(pa)->h-8) {
				div->y = AGWIDGET(pa)->h-8;
				break;
			}

			w1->h += ry;
			AGWIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
	
			w2->y += ry;
			w2->h -= ry;
			AGWIDGET_OPS(w2)->scale(w2, w2->w, w2->h);

			if ((pwin = AG_WidgetParentWindow(pa)) != NULL) {
				AG_WidgetUpdateCoords(pwin, AGWIDGET(pwin)->x,
				    AGWIDGET(pwin)->y);
			}
			if (OverDivControl(div, y)) {
				AG_SetCursor(AG_VRESIZE_CURSOR);
			}
			break;
		} else if (OverDivControl(div, y)) {
			AG_SetCursor(AG_VRESIZE_CURSOR);
		}
	}
	TAILQ_FOREACH(div, &pa->divs, divs)
		div->y = AGWIDGET(div->box2)->y - 5;
}

static void
mousebuttonup(AG_Event *event)
{
	AG_VPane *pa = AG_SELF();
	AG_VPaneDiv *div;

	TAILQ_FOREACH(div, &pa->divs, divs)
		div->moving = 0;
}

void
AG_VPaneInit(AG_VPane *pa, int flags)
{
	int boxflags = 0;

	if (flags & AG_VPANE_WFILL) boxflags |= AG_BOX_WFILL;
	if (flags & AG_VPANE_HFILL) boxflags |= AG_BOX_HFILL;

	AG_BoxInit(&pa->box, AG_BOX_VERT, boxflags);
	AG_BoxSetPadding(&pa->box, 0);
	AG_BoxSetSpacing(&pa->box, 0);
	AGWIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
			       AG_WIDGET_UNFOCUSED_MOTION;
	AG_ObjectSetOps(pa, &agVPaneOps);
	TAILQ_INIT(&pa->divs);

	AG_SetEvent(pa, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(pa, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(pa, "window-mousemotion", mousemotion, NULL);
}

void
AG_VPaneDraw(void *p)
{
	AG_VPane *pa = p;
	AG_Widget *wid;
	AG_VPaneDiv *div;
	Uint32 c;
	int x = AGWIDGET(pa)->w >> 1;

	TAILQ_FOREACH(div, &pa->divs, divs) {
		agPrim.box(pa, 0, div->y-3, AGWIDGET(pa)->w, 7,
		    div->moving ? -1 : 1, AG_COLOR(PANE_COLOR));
		AG_WidgetPutPixel(pa, x, div->y, AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, x - 5, div->y,
		    AG_COLOR(PANE_CIRCLE_COLOR));
		AG_WidgetPutPixel(pa, x + 5, div->y,
		    AG_COLOR(PANE_CIRCLE_COLOR));
	}
}

void
AG_VPaneScale(void *p, int w, int h)
{
	AG_VPane *pa = p;
	AG_VPaneDiv *div;
	AG_Widget *wid;

	AGOBJECT_FOREACH_CHILD(wid, pa, ag_widget)
		AGWIDGET_OPS(wid)->scale(wid, w, h);

	AG_BoxScale(pa, w, h > 8 ? h-8 : h);

	TAILQ_FOREACH(div, &pa->divs, divs) {
		AGWIDGET(div->box1)->h -= 5;
		AGWIDGET(div->box2)->y += 5;
		AGWIDGET(div->box2)->h -= 5;
		AGWIDGET_OPS(div->box1)->scale(div->box1,
		    AGWIDGET(div->box1)->w,
		    AGWIDGET(div->box1)->h);
		AGWIDGET_OPS(div->box2)->scale(div->box2,
		    AGWIDGET(div->box2)->w,
		    AGWIDGET(div->box2)->h);

		div->y = AGWIDGET(div->box2)->y - 5;
	}
}

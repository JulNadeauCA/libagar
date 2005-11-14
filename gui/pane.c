/*	$Csoft: hpane.c,v 1.10 2005/10/07 01:53:12 vedge Exp $	*/

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

#include <core/core.h>
#include <core/view.h>

#include "pane.h"

#include <gui/window.h>
#include <gui/primitive.h>
#include <gui/cursors.h>

static AG_WidgetOps agPaneOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_PaneDraw,
	AG_PaneScale
};

AG_Pane *
AG_PaneNew(void *parent, enum ag_pane_type type, Uint flags)
{
	AG_Pane *pa;

	pa = Malloc(sizeof(AG_Pane), M_OBJECT);
	AG_PaneInit(pa, type, flags);
	AG_ObjectAttach(parent, pa);
	return (pa);
}

static __inline__ int
OverDivControl(AG_Pane *pa, int pos)
{
	return (pos >= (pa->dx - pa->dw) &&
	        pos <= (pa->dx + pa->dw));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();
	int button = AG_INT(1);

	if (button == SDL_BUTTON_LEFT) {
		pa->dmoving = OverDivControl(pa,
		    pa->type == AG_PANE_HORIZ ? AG_INT(2) : AG_INT(3));
	}
}

int
AG_PaneMoveDivider(AG_Pane *pa, int dx)
{
	AG_Widget *w1 = AGWIDGET(pa->div[0]);
	AG_Widget *w2 = AGWIDGET(pa->div[1]);
	AG_Window *pwin;

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (dx < pa->minw[0]) {
			pa->dx = pa->minw[0];
		} else if (dx > (AGWIDGET(pa)->w - pa->dw)) {
			pa->dx = AGWIDGET(pa)->w - pa->dw;
		} else if (dx > (AGWIDGET(pa)->w - pa->minw[1])) {
			pa->dx = AGWIDGET(pa)->w - pa->minw[1];
		} else {
			pa->dx = dx;
		}
		w1->w = pa->dx;
		AGWIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
		w2->x = pa->dx + pa->dw;
		w2->w = AGWIDGET(pa)->w - w1->w - pa->dw;
		AGWIDGET_OPS(w2)->scale(w2, w2->w, w2->h);
		break;
	case AG_PANE_VERT:
		if (dx < pa->minh[0]) {
			pa->dx = pa->minh[0];
		} else if (dx > (AGWIDGET(pa)->h - pa->dw)) {
			pa->dx = AGWIDGET(pa)->h - pa->dw;
		} else if (dx > (AGWIDGET(pa)->h - pa->minh[1])) {
			pa->dx = AGWIDGET(pa)->h - pa->minh[1];
		} else {
			pa->dx = dx;
		}
		w1->h = pa->dx;
		AGWIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
		w2->y = pa->dx + pa->dw;
		w2->h = AGWIDGET(pa)->h - w1->h - pa->dw;
		AGWIDGET_OPS(w2)->scale(w2, w2->w, w2->h);
		break;
	}
	if ((pwin = AG_WidgetParentWindow(pa)) != NULL) {
		AG_WidgetUpdateCoords(pwin, AGWIDGET(pwin)->x,
		                            AGWIDGET(pwin)->y);
	}
	return (pa->dx);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (y < 0 || y > AGWIDGET(pa)->h) {
			return;
		}
		if (pa->dmoving) {
			if (pa->rx < 0) { pa->rx = pa->dx; }
			pa->rx += AG_INT(3);
			pa->rx = AG_PaneMoveDivider(pa, pa->rx);
			if (OverDivControl(pa, x)) {
				AG_SetCursor(AG_HRESIZE_CURSOR);
			}
			break;
		} else if (OverDivControl(pa, x)) {
			AG_SetCursor(AG_HRESIZE_CURSOR);
		}
		break;
	case AG_PANE_VERT:
		if (x < 0 || x > AGWIDGET(pa)->w) {
			return;
		}
		if (pa->dmoving) {
			if (pa->rx < 0) { pa->rx = pa->dx; }
			pa->rx += AG_INT(4);
			pa->rx = AG_PaneMoveDivider(pa, pa->rx);
			if (OverDivControl(pa, y)) {
				AG_SetCursor(AG_VRESIZE_CURSOR);
			}
			break;
		} else if (OverDivControl(pa, y)) {
			AG_SetCursor(AG_VRESIZE_CURSOR);
		}
		break;
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();

	pa->dmoving = 0;
}

void
AG_PaneInit(AG_Pane *pa, enum ag_pane_type type, Uint flags)
{
	Uint boxflags = 0;
	Uint dflags1 = 0;
	Uint dflags2 = 0;

	if (flags & AG_PANE_HFILL) boxflags |= AG_BOX_HFILL;
	if (flags & AG_PANE_VFILL) boxflags |= AG_BOX_VFILL;

	switch (type) {
	case AG_PANE_VERT:
		AG_BoxInit(&pa->box, AG_BOX_VERT, boxflags);
		dflags1 = AG_BOX_HFILL;
		dflags2 = AG_BOX_HFILL;
		if (flags & AG_PANE_DIV1_FILL) { dflags1 |= AG_BOX_VFILL; }
		if (flags & AG_PANE_DIV2_FILL) { dflags2 |= AG_BOX_VFILL; }
		break;
	case AG_PANE_HORIZ:
		AG_BoxInit(&pa->box, AG_BOX_HORIZ, boxflags);
		dflags1 = AG_BOX_VFILL;
		dflags2 = AG_BOX_VFILL;
		if (flags & AG_PANE_DIV1_FILL) { dflags1 |= AG_BOX_HFILL; }
		if (flags & AG_PANE_DIV2_FILL) { dflags2 |= AG_BOX_HFILL; }
		break;
	}
	
	AGWIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
			       AG_WIDGET_UNFOCUSED_MOTION;
	AG_ObjectSetOps(pa, &agPaneOps);

	pa->type = type;
	pa->flags = flags;
	pa->div[0] = AG_BoxNew(pa, AG_BOX_VERT, dflags1);
	pa->div[1] = AG_BoxNew(pa, AG_BOX_VERT, dflags2);
	pa->dx = 0;
	pa->rx = -1;
	pa->dmoving = 0;
	pa->dw = 8;
	pa->minw[0] = -1; pa->minw[1] = -1;
	pa->minh[0] = -1; pa->minh[1] = -1;

	AG_BoxSetPadding(&pa->box, 0);
	AG_BoxSetSpacing(&pa->box, 0);
	AG_BoxSetPadding(pa->div[0], 0);
	AG_BoxSetPadding(pa->div[0], 0);
	AG_BoxSetSpacing(pa->div[1], 0);
	AG_BoxSetSpacing(pa->div[1], 0);

	AG_SetEvent(pa, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(pa, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(pa, "window-mousemotion", MouseMotion, NULL);
}

void
AG_PaneSetDividerWidth(AG_Pane *pa, int dw)
{
	pa->dw = dw;
}

void
AG_PaneSetDivisionMin(AG_Pane *pa, int which, int minw, int minh)
{
	pa->minw[which] = minw;
	pa->minh[which] = minh;
}

void
AG_PaneAttachBoxes(AG_Pane *pa, AG_Box *box1, AG_Box *box2)
{
	AG_ObjectDetach(pa->div[0]);
	AG_ObjectDetach(pa->div[1]);
	AG_ObjectDestroy(pa->div[0]);
	AG_ObjectDestroy(pa->div[1]);
	pa->div[0] = box1;
	pa->div[1] = box2;
	AG_ObjectAttach(pa, box1);
	AG_ObjectAttach(pa, box2);
}

void
AG_PaneDraw(void *p)
{
	AG_Pane *pa = p;
	AG_Widget *wid;
	int x, y;
	int z = pa->dmoving ? -1 : 1;

	switch (pa->type) {
	case AG_PANE_HORIZ:
		y = AGWIDGET(pa)->h >> 1;
		agPrim.box(pa, pa->dx+1, 0, pa->dw-2, AGWIDGET(pa)->h, z,
		    AG_COLOR(PANE_COLOR));
		if (!agView->opengl) {
			int cx = pa->dx + (pa->dw>>1);

			AG_WidgetPutPixel(pa, cx, y,
			    AG_COLOR(PANE_CIRCLE_COLOR));
			AG_WidgetPutPixel(pa, cx, y-5,
			    AG_COLOR(PANE_CIRCLE_COLOR));
			AG_WidgetPutPixel(pa, cx, y+5,
			    AG_COLOR(PANE_CIRCLE_COLOR));
		}
		break;
	case AG_PANE_VERT:
		x = AGWIDGET(pa)->w >> 1;
		agPrim.box(pa, 0, pa->dx+1, AGWIDGET(pa)->w, pa->dw-2, z,
		    AG_COLOR(PANE_COLOR));
		if (!agView->opengl) {
			int cx = pa->dx + (pa->dw>>1);
			
			AG_WidgetPutPixel(pa, x, cx,
			    AG_COLOR(PANE_CIRCLE_COLOR));
			AG_WidgetPutPixel(pa, x-5, cx,
			    AG_COLOR(PANE_CIRCLE_COLOR));
			AG_WidgetPutPixel(pa, x+5, cx,
			    AG_COLOR(PANE_CIRCLE_COLOR));
		}
		break;
	}
}

void
AG_PaneScale(void *p, int w, int h)
{
	AG_Pane *pa = p;
	AG_Widget *wid;

	if (w == -1 || h == -1) {
		AGWIDGET_OPS(pa->div[0])->scale(pa->div[0], -1, -1);
		AGWIDGET_OPS(pa->div[0])->scale(pa->div[1], -1, -1);
		if (pa->minw[0] < 0) { pa->minw[0] = AGWIDGET(pa->div[0])->w; }
		if (pa->minh[0] < 0) { pa->minh[0] = AGWIDGET(pa->div[0])->h; }
		if (pa->minw[1] < 0) { pa->minw[1] = AGWIDGET(pa->div[1])->w; }
		if (pa->minh[1] < 0) { pa->minh[1] = AGWIDGET(pa->div[1])->h; }
	}

	AGOBJECT_FOREACH_CHILD(wid, pa, ag_widget)
		AGWIDGET_OPS(wid)->scale(wid, w, h);

	AG_BoxScale(pa, w, h);

	switch (pa->type) {
	case AG_PANE_HORIZ:
		AGWIDGET(pa->div[1])->x += pa->dw;
		AGWIDGET(pa->div[1])->w -= pa->dw;
		AGWIDGET_OPS(pa->div[0])->scale(pa->div[0],
		    AGWIDGET(pa->div[0])->w,
		    AGWIDGET(pa->div[0])->h);
		AGWIDGET_OPS(pa->div[1])->scale(pa->div[1],
		    AGWIDGET(pa->div[1])->w,
		    AGWIDGET(pa->div[1])->h);
		pa->dx = AGWIDGET(pa->div[1])->x - pa->dw;
		break;
	case AG_PANE_VERT:
		AGWIDGET(pa->div[1])->y += pa->dw;
		AGWIDGET(pa->div[1])->h -= pa->dw;
		AGWIDGET_OPS(pa->div[0])->scale(pa->div[0],
		    AGWIDGET(pa->div[0])->w,
		    AGWIDGET(pa->div[0])->h);
		AGWIDGET_OPS(pa->div[1])->scale(pa->div[1],
		    AGWIDGET(pa->div[1])->w,
		    AGWIDGET(pa->div[1])->h);
		pa->dx = AGWIDGET(pa->div[1])->y - pa->dw;
		if (pa->dx > AGWIDGET(pa)->h - pa->dw) {
			pa->dx = AGWIDGET(pa)->h - pa->dw;
			AGWIDGET(pa->div[0])->h -= pa->dw;
			AGWIDGET_OPS(pa->div[0])->scale(pa->div[0],
			    AGWIDGET(pa->div[0])->w,
			    AGWIDGET(pa->div[0])->h);
		}
		break;
	}
	if (pa->rx >= 0) {
		AG_PaneMoveDivider(pa, pa->rx);
	}
}


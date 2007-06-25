/*
 * Copyright (c) 2005-2006 CubeSoft Communications, Inc.
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

#include "window.h"
#include "primitive.h"
#include "cursors.h"

static AG_WidgetOps agPaneOps = {
	{
		"AG_Widget:AG_Pane",
		sizeof(AG_Pane),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
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
	return (pos >= pa->dx &&
	        pos <= (pa->dx+pa->dw));
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

	pa->dx = dx;
	AG_PaneScale(pa, AGWIDGET(pa)->w, AGWIDGET(pa)->h);
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
	Uint wflags = 0;
	int i;

	if (flags & AG_PANE_HFILL) wflags |= AG_WIDGET_HFILL;
	if (flags & AG_PANE_VFILL) wflags |= AG_WIDGET_VFILL;

	AG_WidgetInit(pa, &agPaneOps, wflags);

	AGWIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
			       AG_WIDGET_UNFOCUSED_MOTION;
	AG_ObjectSetOps(pa, &agPaneOps);

	pa->type = type;
	pa->flags = flags;
	pa->div[0] = AG_BoxNew(pa, AG_BOX_VERT, AG_PANE_FRAME?AG_BOX_FRAME:0);
	pa->div[1] = AG_BoxNew(pa, AG_BOX_VERT, AG_PANE_FRAME?AG_BOX_FRAME:0);
	pa->dx = 0;
	pa->rx = -1;
	pa->dmoving = 0;
	pa->dw = 10;

	for (i = 0; i < 2; i++) {
		pa->minw[i] = -1;
		pa->minh[i] = -1;
		AG_BoxSetPadding(pa->div[i], 0);
		AG_BoxSetSpacing(pa->div[i], 0);
	}

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
AG_PaneAttachBox(AG_Pane *pa, int which, AG_Box *box)
{
	AG_ObjectAttach(pa->div[which], box);
	AGWIDGET(box)->flags |= AG_WIDGET_EXPAND;
}

void
AG_PaneAttachBoxes(AG_Pane *pa, AG_Box *box1, AG_Box *box2)
{
	AG_PaneAttachBox(pa, 0, box1);
	AG_PaneAttachBox(pa, 1, box2);
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
	AG_Widget *w1 = AGWIDGET(pa->div[0]);
	AG_Widget *w2 = AGWIDGET(pa->div[1]);

	if (w == -1 || h == -1) {
		int maxw = 0, maxh = 0;
		int dw, dh;
		int i;
		
		AGWIDGET(pa)->w = 0;
		AGWIDGET(pa)->h = 0;

		for (i = 0; i < 2; i++) {
			AG_Widget *div = AGWIDGET(pa->div[i]);

			AGWIDGET_OPS(div)->scale(div, -1, -1);
			if (pa->minw[i] == -1) pa->minw[i] = div->w;
			if (pa->minh[i] == -1) pa->minh[i] = div->h;
			if (div->w > maxw) { maxw = div->w; }
			if (div->h > maxh) { maxh = div->h; }
			switch (pa->type) {
			case AG_PANE_HORIZ:
				if ((dh = maxh) > AGWIDGET(pa)->h) {
					AGWIDGET(pa)->h = dh;
				}
				AGWIDGET(pa)->w += div->w;
				break;
			case AG_BOX_VERT:
				if ((dw = maxw) > AGWIDGET(pa)->w) {
					AGWIDGET(pa)->w = dw;
				}
				AGWIDGET(pa)->h += div->h;
				break;
			}
		}
		switch (pa->type) {
		case AG_PANE_HORIZ:
			AGWIDGET(pa)->w += pa->dw + 2;
			break;
		case AG_PANE_VERT:
			AGWIDGET(pa)->h += pa->dw + 2;
			break;
		}
		return;
	}
		
	AGWIDGET(pa)->w = w;
	AGWIDGET(pa)->h = h;
	w1->x = 0;
	w1->y = 0;
	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (pa->rx == -1) {
			if (pa->flags & AG_PANE_DIV) {
				pa->dx = AGWIDGET(pa)->w/2;
			} else if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = AGWIDGET(pa)->w - pa->minw[1];
			} else {
				pa->dx = pa->minw[0];
			}
			pa->rx = pa->dx;
		} else {
			if (pa->flags & AG_PANE_FORCE_DIV) {
				pa->dx = AGWIDGET(pa)->w/2;
			} else if (pa->flags & AG_PANE_FORCE_DIV1FILL) {
				pa->dx = AGWIDGET(pa)->w - pa->minw[1];
			} else if (pa->flags & AG_PANE_FORCE_DIV2FILL) {
				pa->dx = pa->minw[0];
			}
		}
		if (pa->dx < pa->minw[0]) {
			pa->rx = pa->dx = pa->minw[0];
		} else if (pa->dx > (AGWIDGET(pa)->w - pa->dw)) {
			pa->rx = pa->dx = AGWIDGET(pa)->w - pa->dw;
		} else if (pa->dx > (AGWIDGET(pa)->w - pa->minw[1])) {
			pa->rx = pa->dx = AGWIDGET(pa)->w - pa->minw[1];
		}
    		w1->w = pa->dx;
		w1->h = h;
		w2->x = pa->dx + pa->dw;
		w2->y = 0;
		w2->w = w - pa->dx - pa->dw;
		w2->h = h;
		AGWIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
		AGWIDGET_OPS(w2)->scale(w2, w2->w, w2->h);
		break;
	case AG_PANE_VERT:
		if (pa->rx == -1) {
			if (pa->flags & AG_PANE_DIV) {
				pa->dx = AGWIDGET(pa)->h/2;
			} else if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = AGWIDGET(pa)->h - pa->minh[1];
			} else {
				pa->dx = pa->minh[0];
			}
			pa->rx = pa->dx;
		} else {
			if (pa->flags & AG_PANE_FORCE_DIV) {
				pa->dx = AGWIDGET(pa)->h/2;
			} else if (pa->flags & AG_PANE_FORCE_DIV1FILL) {
				pa->dx = AGWIDGET(pa)->h - pa->minh[1];
			} else if (pa->flags & AG_PANE_FORCE_DIV2FILL) {
				pa->dx = pa->minh[0];
			}
		}
		if (pa->dx < pa->minh[0]) {
			pa->rx = pa->dx = pa->minh[0];
		} else if (pa->dx > (AGWIDGET(pa)->h - pa->dw)) {
			pa->rx = pa->dx = AGWIDGET(pa)->h - pa->dw;
		} else if (pa->dx > (AGWIDGET(pa)->h - pa->minh[1])) {
			pa->rx = pa->dx = AGWIDGET(pa)->h - pa->minh[1];
		}
		w1->w = w;
		w1->h = pa->dx;
		w2->x = 0;
		w2->y = pa->dx + pa->dw;
		w2->w = w;
		w2->h = h - pa->dx - pa->dw;
		AGWIDGET_OPS(w1)->scale(w1, w1->w, w1->h);
		AGWIDGET_OPS(w2)->scale(w2, w2->w, w2->h);
		break;
	}
}


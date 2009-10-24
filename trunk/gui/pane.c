/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "pane.h"

#include "window.h"
#include "primitive.h"
#include "cursors.h"

AG_Pane *
AG_PaneNew(void *parent, enum ag_pane_type type, Uint flags)
{
	AG_Pane *pa;

	pa = Malloc(sizeof(AG_Pane));
	AG_ObjectInit(pa, &agPaneClass);
	pa->type = type;
	pa->flags |= flags;

	if (flags & AG_PANE_HFILL) { AG_ExpandHoriz(pa); }
	if (flags & AG_PANE_VFILL) { AG_ExpandVert(pa); }

	AG_ObjectAttach(parent, pa);
	return (pa);
}

static __inline__ int
OverDivControl(AG_Pane *pa, int pos)
{
	return (pos >= pa->dx &&
	        pos <= (pa->dx + MIN(pa->wDiv,4)));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();
	int button = AG_INT(1);

	if (button == AG_MOUSE_LEFT &&
	    !(pa->flags & AG_PANE_UNMOVABLE)) {
		pa->dmoving = OverDivControl(pa,
		    pa->type == AG_PANE_HORIZ ? AG_INT(2) : AG_INT(3));
		if (pa->dmoving)
			WIDGET(pa)->flags |= AG_WIDGET_PRIO_MOTION;
	}
}

/* Move a divider to the given position. */
int
AG_PaneMoveDivider(AG_Pane *pa, int dx)
{
	AG_SizeAlloc a;
	int rv;

	AG_ObjectLock(pa);
	if (pa->rx == -1) {		/* Geometry not yet allocated */
		pa->rx = dx;
		pa->dx = dx;
		rv = dx;
	} else {
		pa->dx = dx;
		a.x = WIDGET(pa)->x;
		a.y = WIDGET(pa)->y;
		a.w = WIDTH(pa);
		a.h = HEIGHT(pa);
		AG_WidgetSizeAlloc(pa, &a);
		rv = pa->dx;
		AG_WidgetUpdate(pa);
		pa->rx = rv;
	}
	AG_ObjectUnlock(pa);
	return (rv);
}

/* Move a divider to the given position (%). */
int
AG_PaneMoveDividerPct(AG_Pane *pa, int pct)
{
	int rv;

	AG_ObjectLock(pa);
	if (pa->rx == -1) {		/* Geometry not yet allocated */
		pa->rxPct = pct;
		rv = 0;
	} else {
		int size = (pa->type == AG_PANE_HORIZ) ? WIDTH(pa) : HEIGHT(pa);
		rv = AG_PaneMoveDivider(pa, pct*size/100);
	}
	AG_ObjectUnlock(pa);
	return (rv);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();
	AG_Driver *drv = WIDGET(pa)->drv;
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dx = AG_INT(3);
	int dy = AG_INT(4);

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (y < 0 || y > HEIGHT(pa)) {
			return;
		}
		if (pa->dmoving) {
			if (pa->rx < 0) { pa->rx = pa->dx; }
			pa->rx += dx;
			if (pa->rx < 2) { pa->rx = 2; }
			AG_PaneMoveDivider(pa, pa->rx);
			if (OverDivControl(pa, x)) {
				AG_PushStockCursor(drv, AG_HRESIZE_CURSOR);
			}
			break;
		} else if (!(pa->flags & AG_PANE_UNMOVABLE) &&
		    OverDivControl(pa, x)) {
			AG_PushStockCursor(drv, AG_HRESIZE_CURSOR);
		} else {
			AG_PopCursor(drv);
		}
		break;
	case AG_PANE_VERT:
		if (x < 0 || x > WIDTH(pa)) {
			return;
		}
		if (pa->dmoving) {
			if (pa->rx < 0) { pa->rx = pa->dx; }
			pa->rx += dy;
			AG_PaneMoveDivider(pa, pa->rx);
			if (pa->rx < 2) { pa->rx = 2; }
			if (OverDivControl(pa, y)) {
				AG_PushStockCursor(drv, AG_VRESIZE_CURSOR);
			}
			break;
		} else if (!(pa->flags & AG_PANE_UNMOVABLE) &&
		    OverDivControl(pa, y)) {
			AG_PushStockCursor(drv, AG_VRESIZE_CURSOR);
		} else {
			AG_PopCursor(drv);
		}
		break;
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();

	if (pa->dmoving) {
		WIDGET(pa)->flags &= ~(AG_WIDGET_PRIO_MOTION);
		pa->dmoving = 0;
	}
}

static void
Init(void *obj)
{
	AG_Pane *pa = obj;
	int i;

	WIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
			     AG_WIDGET_UNFOCUSED_MOTION;

	pa->type = AG_PANE_VERT;
	pa->flags = 0;
	pa->div[0] = AG_BoxNew(pa, AG_BOX_VERT, AG_BOX_FRAME);
	pa->div[1] = AG_BoxNew(pa, AG_BOX_VERT, AG_BOX_FRAME);
	pa->dx = 0;
	pa->rx = -1;
	pa->rxPct = -1;
	pa->dmoving = 0;
	pa->wDiv = 8;

	for (i = 0; i < 2; i++) {
		pa->wMin[i] = 0;
		pa->hMin[i] = 0;
		pa->wReq[i] = -1;
		pa->hReq[i] = -1;
	}

	AG_SetEvent(pa, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(pa, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(pa, "mouse-motion", MouseMotion, NULL);

#ifdef AG_DEBUG
	AG_BindInt(pa, "flags", &pa->flags);
	AG_BindInt(pa, "wMin[0]", &pa->wMin[0]);
	AG_BindInt(pa, "hMin[0]", &pa->hMin[0]);
	AG_BindInt(pa, "wMin[1]", &pa->wMin[1]);
	AG_BindInt(pa, "hMin[1]", &pa->hMin[1]);
	AG_BindInt(pa, "wReq[0]", &pa->wReq[0]);
	AG_BindInt(pa, "hReq[0]", &pa->hReq[0]);
	AG_BindInt(pa, "wReq[1]", &pa->wReq[1]);
	AG_BindInt(pa, "hReq[1]", &pa->hReq[1]);
	AG_BindInt(pa, "dmoving", &pa->dmoving);
	AG_BindInt(pa, "dx", &pa->dx);
	AG_BindInt(pa, "rx", &pa->rx);
	AG_BindInt(pa, "wDiv", &pa->wDiv);
#endif
}

void
AG_PaneSetDividerWidth(AG_Pane *pa, int wDiv)
{
	AG_ObjectLock(pa);
	pa->wDiv = wDiv;
	AG_ObjectUnlock(pa);
}

void
AG_PaneSetDivisionPacking(AG_Pane *pa, int which, enum ag_box_type packing)
{
	AG_ObjectLock(pa);
	AG_BoxSetType(pa->div[which], packing);
	AG_ObjectUnlock(pa);
}

void
AG_PaneSetDivisionMin(AG_Pane *pa, int which, int wMin, int hMin)
{
	AG_ObjectLock(pa);
	pa->wMin[which] = wMin;
	pa->hMin[which] = hMin;
	AG_ObjectUnlock(pa);
}

void
AG_PaneAttachBox(AG_Pane *pa, int which, AG_Box *box)
{
	AG_ObjectLock(pa);
	AG_ObjectLock(box);

	/* XXX */
#if 0
	if (pa->div[which] != NULL) {

		AG_ObjectDetach(pa->div[which]);
		AG_ObjectDestroy(pa->div[which]);
	}
#endif
	AG_ObjectAttach(pa->div[which], box);
	WIDGET(box)->flags |= AG_WIDGET_EXPAND;
	
	AG_ObjectUnlock(box);
	AG_ObjectUnlock(pa);
}

void
AG_PaneAttachBoxes(AG_Pane *pa, AG_Box *box1, AG_Box *box2)
{
	AG_ObjectLock(pa);
	AG_PaneAttachBox(pa, 0, box1);
	AG_PaneAttachBox(pa, 1, box2);
	AG_ObjectUnlock(pa);
}

static void
Draw(void *obj)
{
	AG_Pane *pa = obj;
	
	AG_WidgetDraw(pa->div[0]);
	AG_WidgetDraw(pa->div[1]);

	if (pa->wDiv > 0) {
		switch (pa->type) {
		case AG_PANE_HORIZ:
			STYLE(pa)->PaneHorizDivider(pa, pa->dx, HEIGHT(pa)/2,
			    pa->wDiv, pa->dmoving);
			break;
		case AG_PANE_VERT:
			STYLE(pa)->PaneVertDivider(pa, WIDTH(pa)/2, pa->dx,
			    pa->wDiv, pa->dmoving);
			break;
		}
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Pane *pa = obj;
	AG_SizeReq rDiv;
	int wMax = 0, hMax = 0;
	int i;
		
	r->w = 0;
	r->h = 0;

	for (i = 0; i < 2; i++) {
		AG_Widget *div = WIDGET(pa->div[i]);

		AG_WidgetSizeReq(div, &rDiv);
		pa->wReq[i] = rDiv.w;
		pa->hReq[i] = rDiv.h;

		if (rDiv.w > wMax) { wMax = rDiv.w; }
		if (rDiv.h > hMax) { hMax = rDiv.h; }

		switch (pa->type) {
		case AG_PANE_HORIZ:
			r->h = MAX(r->h, hMax);
			r->w += rDiv.w;
			break;
		case AG_PANE_VERT:
			r->w = MAX(r->w, wMax);
			r->h += rDiv.h;
			break;
		}
	}
	switch (pa->type) {
	case AG_PANE_HORIZ:
		r->w += (pa->wDiv > 0) ? (pa->wDiv + 2) : 0;
		break;
	case AG_PANE_VERT:
		r->h += (pa->wDiv > 0) ? (pa->wDiv + 2) : 0;
		break;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Pane *pa = obj;
	AG_SizeReq r1, r2;
	AG_SizeAlloc a1, a2;

	a1.x = 0;
	a1.y = 0;

	AG_WidgetSizeReq(pa->div[0], &r1);
	AG_WidgetSizeReq(pa->div[1], &r2);

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (pa->dx == 0 && pa->rx == -1) {
			/* Set initial divider position. */
			if (pa->flags & AG_PANE_DIV) {
				pa->dx = a->w/2;
			} else if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = a->w - pa->wReq[1];
			} else {
				if (pa->rxPct != -1) {
					pa->dx = pa->rxPct*WIDTH(pa)/100;
				} else {
					pa->dx = pa->wReq[0];
				}
			}
			pa->rx = pa->dx;
		} else {
			/* Apply change in divider position. */
			if (pa->flags & AG_PANE_FORCE_DIV) {
				pa->dx = a->w/2;
			} else if (pa->flags & AG_PANE_FORCE_DIV1FILL) {
				pa->dx = a->w - pa->wReq[1];
			} else if (pa->flags & AG_PANE_FORCE_DIV2FILL) {
				pa->dx = pa->wReq[0];
			}
		}
		if (pa->dx < pa->wMin[0]) {
			pa->rx = pa->dx = pa->wMin[0];
		} else if (pa->dx > (a->w - pa->wDiv)) {
			pa->rx = pa->dx = a->w - pa->wDiv;
		} else if (pa->dx > (a->w - pa->wMin[1])) {
			pa->rx = pa->dx = a->w - pa->wMin[1];
		}
    		a1.w = pa->dx;
		a1.h = a->h;
		a2.x = pa->dx + pa->wDiv;
		a2.y = 0;
		a2.w = a->w - pa->dx - pa->wDiv;
		a2.h = a->h;
		break;
	case AG_PANE_VERT:
		if (pa->dx == 0 && pa->rx == -1) {
			/* Set initial divider position. */
			if (pa->flags & AG_PANE_DIV) {
				pa->dx = a->h/2;
			} else if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = a->h - pa->hReq[1];
			} else {
				if (pa->rxPct != -1) {
					pa->dx = pa->rxPct*HEIGHT(pa)/100;
				} else {
					pa->dx = pa->hReq[0];
				}
			}
			pa->rx = pa->dx;
		} else {
			/* Apply change in divider position. */
			if (pa->flags & AG_PANE_FORCE_DIV) {
				pa->dx = a->h/2;
			} else if (pa->flags & AG_PANE_FORCE_DIV1FILL) {
				pa->dx = a->h - pa->hReq[1];
			} else if (pa->flags & AG_PANE_FORCE_DIV2FILL) {
				pa->dx = pa->hReq[0];
			}
		}
		if (pa->dx < pa->hMin[0]) {
			pa->rx = pa->dx = pa->hMin[0];
		} else if (pa->dx > (a->h - pa->wDiv)) {
			pa->rx = pa->dx = a->h - pa->wDiv;
		} else if (pa->dx > (a->h - pa->hMin[1])) {
			pa->rx = pa->dx = a->h - pa->hMin[1];
		}
		a1.w = a->w;
		a1.h = pa->dx;
		a2.x = 0;
		a2.y = pa->dx + pa->wDiv;
		a2.w = a->w;
		a2.h = a->h - pa->dx - pa->wDiv;
		break;
	}
	AG_WidgetSizeAlloc(pa->div[0], &a1);
	AG_WidgetSizeAlloc(pa->div[1], &a2);
	return (0);
}

AG_WidgetClass agPaneClass = {
	{
		"Agar(Widget:Pane)",
		sizeof(AG_Pane),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

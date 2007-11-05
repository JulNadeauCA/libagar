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
	AG_PaneInit(pa, type, flags);
	AG_ObjectAttach(parent, pa);
	return (pa);
}

static __inline__ int
OverDivControl(AG_Pane *pa, int pos)
{
	return (pos >= pa->dx &&
	        pos <= (pa->dx + pa->wDiv));
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();
	int button = AG_INT(1);

	if (button == SDL_BUTTON_LEFT) {
		pa->dmoving = OverDivControl(pa,
		    pa->type == AG_PANE_HORIZ ? AG_INT(2) : AG_INT(3));
		if (pa->dmoving)
			WIDGET(pa)->flags |= AG_WIDGET_PRIO_MOTION;
	}
}

int
AG_PaneMoveDivider(AG_Pane *pa, int dx)
{
	AG_Window *pwin;
	AG_SizeAlloc a;

	pa->dx = dx;

	a.x = WIDGET(pa)->x;
	a.y = WIDGET(pa)->y;
	a.w = WIDGET(pa)->w;
	a.h = WIDGET(pa)->h;
	AG_WidgetSizeAlloc(pa, &a);
	
	if ((pwin = AG_WidgetParentWindow(pa)) != NULL) {
		AG_WidgetUpdateCoords(pwin, WIDGET(pwin)->x,
		                            WIDGET(pwin)->y);
	}
	return (pa->dx);
}

static void
mousemotion(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dx = AG_INT(3);
	int dy = AG_INT(4);

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (y < 0 || y > WIDGET(pa)->h) {
			return;
		}
		if (pa->dmoving) {
			if (pa->rx < 0) { pa->rx = pa->dx; }
			pa->rx += dx;
			if (pa->rx < 2) { pa->rx = 2; }
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
		if (x < 0 || x > WIDGET(pa)->w) {
			return;
		}
		if (pa->dmoving) {
			if (pa->rx < 0) { pa->rx = pa->dx; }
			pa->rx += dy;
			pa->rx = AG_PaneMoveDivider(pa, pa->rx);
			if (pa->rx < 2) { pa->rx = 2; }
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
mousebuttonup(AG_Event *event)
{
	AG_Pane *pa = AG_SELF();

	if (pa->dmoving) {
		WIDGET(pa)->flags &= ~(AG_WIDGET_PRIO_MOTION);
		pa->dmoving = 0;
	}
}

void
AG_PaneInit(AG_Pane *pa, enum ag_pane_type type, Uint flags)
{
	Uint wflags = 0;
	int i;

	if (flags & AG_PANE_HFILL) wflags |= AG_WIDGET_HFILL;
	if (flags & AG_PANE_VFILL) wflags |= AG_WIDGET_VFILL;

	AG_WidgetInit(pa, &agPaneOps, wflags);

	WIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
			     AG_WIDGET_UNFOCUSED_MOTION;
	AG_ObjectSetOps(pa, &agPaneOps);

	pa->type = type;
	pa->flags = flags|AG_PANE_INITSCALE;
	pa->div[0] = AG_BoxNew(pa, AG_BOX_VERT, AG_PANE_FRAME?AG_BOX_FRAME:0);
	pa->div[1] = AG_BoxNew(pa, AG_BOX_VERT, AG_PANE_FRAME?AG_BOX_FRAME:0);
	pa->dx = 0;
	pa->rx = -1;
	pa->dmoving = 0;
	pa->wDiv = 8;

	for (i = 0; i < 2; i++) {
		pa->minw[i] = -1;
		pa->minh[i] = -1;
#if 0
		AG_BoxSetPadding(pa->div[i], 0);
		AG_BoxSetSpacing(pa->div[i], 0);
#endif
	}

	AG_SetEvent(pa, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(pa, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(pa, "window-mousemotion", mousemotion, NULL);
}

void
AG_PaneSetDividerWidth(AG_Pane *pa, int wDiv)
{
	pa->wDiv = wDiv;
}

void
AG_PaneSetDivisionPacking(AG_Pane *pa, int which, enum ag_box_type packing)
{
	AG_BoxSetType(pa->div[which], packing);
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
	/* XXX */
#if 0
	if (pa->div[which] != NULL) {

		AG_ObjectDetach(pa->div[which]);
		AG_ObjectDestroy(pa->div[which]);
		Free(pa->div[which]);
	}
#endif
	AG_ObjectAttach(pa->div[which], box);
	WIDGET(box)->flags |= AG_WIDGET_EXPAND;
}

void
AG_PaneAttachBoxes(AG_Pane *pa, AG_Box *box1, AG_Box *box2)
{
	AG_PaneAttachBox(pa, 0, box1);
	AG_PaneAttachBox(pa, 1, box2);
}

static void
Draw(void *p)
{
	AG_Pane *pa = p;

	switch (pa->type) {
	case AG_PANE_HORIZ:
		STYLE(pa)->PaneHorizDivider(pa, pa->dx, WIDGET(pa)->h/2,
		    pa->wDiv, pa->dmoving);
		break;
	case AG_PANE_VERT:
		STYLE(pa)->PaneVertDivider(pa, WIDGET(pa)->w/2, pa->dx,
		    pa->wDiv, pa->dmoving);
		break;
	}
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Pane *pa = p;
	AG_SizeReq rDiv;
	int wMax = 0, hMax = 0;
	int i;
		
	r->w = 0;
	r->h = 0;

	for (i = 0; i < 2; i++) {
		AG_Widget *div = WIDGET(pa->div[i]);

		AG_WidgetSizeReq(div, &rDiv);
		if (pa->minw[i] == -1) { pa->minw[i] = rDiv.w; }
		if (pa->minh[i] == -1) { pa->minh[i] = rDiv.h; }
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
		r->w += pa->wDiv + 2;
		break;
	case AG_PANE_VERT:
		r->h += pa->wDiv + 2;
		break;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Pane *pa = p;
	AG_SizeReq r1, r2;
	AG_SizeAlloc a1, a2;

	a1.x = 0;
	a1.y = 0;

	AG_WidgetSizeReq(pa->div[0], &r1);
	AG_WidgetSizeReq(pa->div[1], &r2);
	
	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (pa->rx == -1) {
			if (pa->flags & AG_PANE_DIV) {
				pa->dx = a->w/2;
			} else if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = a->w - pa->minw[1];
			} else {
				pa->dx = pa->minw[0];
			}
			pa->rx = pa->dx;
		} else {
			if (pa->flags & AG_PANE_FORCE_DIV) {
				pa->dx = a->w/2;
			} else if (pa->flags & AG_PANE_FORCE_DIV1FILL) {
				pa->dx = a->w - pa->minw[1];
			} else if (pa->flags & AG_PANE_FORCE_DIV2FILL) {
				pa->dx = pa->minw[0];
			}
		}
		if (pa->dx < pa->minw[0]) {
			pa->rx = pa->dx = pa->minw[0];
		} else if (pa->dx > (a->w - pa->wDiv)) {
			pa->rx = pa->dx = a->w - pa->wDiv;
		} else if (pa->dx > (a->w - pa->minw[1])) {
			pa->rx = pa->dx = a->w - pa->minw[1];
		}
    		a1.w = pa->dx;
		a1.h = a->h;
		a2.x = pa->dx + pa->wDiv;
		a2.y = 0;
		a2.w = a->w - pa->dx - pa->wDiv;
		a2.h = a->h;
		break;
	case AG_PANE_VERT:
		if (pa->rx == -1) {
			if (pa->flags & AG_PANE_DIV) {
				pa->dx = a->h/2;
			} else if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = a->h - pa->minh[1];
			} else {
				pa->dx = pa->minh[0];
			}
			pa->rx = pa->dx;
		} else {
			if (pa->flags & AG_PANE_FORCE_DIV) {
				pa->dx = a->h/2;
			} else if (pa->flags & AG_PANE_FORCE_DIV1FILL) {
				pa->dx = a->h - pa->minh[1];
			} else if (pa->flags & AG_PANE_FORCE_DIV2FILL) {
				pa->dx = pa->minh[0];
			}
		}
		if (pa->dx < pa->minh[0]) {
			pa->rx = pa->dx = pa->minh[0];
		} else if (pa->dx > (a->h - pa->wDiv)) {
			pa->rx = pa->dx = a->h - pa->wDiv;
		} else if (pa->dx > (a->h - pa->minh[1])) {
			pa->rx = pa->dx = a->h - pa->minh[1];
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
	
	if (pa->flags & AG_PANE_INITSCALE) {
		pa->minw[0] = 0;
		pa->minh[0] = 0;
		pa->minw[1] = 0;
		pa->minh[1] = 0;
	}
	return (0);
}

const AG_WidgetOps agPaneOps = {
	{
		"AG_Widget:AG_Pane",
		sizeof(AG_Pane),
		{ 0,0 },
		NULL,		/* init */
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

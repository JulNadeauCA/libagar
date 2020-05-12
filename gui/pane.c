/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Paned container widget. It divides space into two partitions (which are
 * AG_Box(3) containers) with a user-movable separator between them.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/pane.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/cursors.h>

AG_Pane *
AG_PaneNew(void *parent, enum ag_pane_type type, Uint flags)
{
	AG_Pane *pa;

	pa = Malloc(sizeof(AG_Pane));
	AG_ObjectInit(pa, &agPaneClass);

	pa->type = type;

	if (flags & AG_PANE_HFILL) { WIDGET(pa)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PANE_VFILL) { WIDGET(pa)->flags |= AG_WIDGET_VFILL; }
	pa->flags |= flags;

#ifdef AG_LEGACY
	if (flags & AG_PANE_DIV)
		AG_PaneMoveDividerPct(pa, 50);
	if (flags & AG_PANE_FORCE_DIV1FILL)
		pa->resizeAction = AG_PANE_EXPAND_DIV1;
	if (flags & AG_PANE_FORCE_DIV2FILL)
		pa->resizeAction = AG_PANE_EXPAND_DIV2;
	if (flags & AG_PANE_FORCE_DIV)
		pa->resizeAction = AG_PANE_DIVIDE_EVEN;
#endif /* AG_LEGACY */

	AG_ObjectAttach(parent, pa);
	return (pa);
}

AG_Pane	*
AG_PaneNewHoriz(void *parent, Uint flags)
{
	return AG_PaneNew(parent, AG_PANE_HORIZ, flags);
}

AG_Pane	*
AG_PaneNewVert(void *parent, Uint flags)
{
	return AG_PaneNew(parent, AG_PANE_VERT, flags);
}

static __inline__ int
OverDiv(AG_Pane *_Nonnull pa, int pos)
{
	return (pos >= pa->dx &&
	        pos < (pa->dx + pa->wDiv));
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Pane *pa = AG_PANE_SELF();
	AG_Window *wParent = AG_ParentWindow(pa);
	const int button = AG_INT(1);

	if (!AG_WindowIsFocused(wParent))
		AG_WindowFocus(wParent);
	
	if (button == AG_MOUSE_LEFT &&
	    !(pa->flags & AG_PANE_UNMOVABLE)) {
		pa->dmoving = OverDiv(pa, (pa->type == AG_PANE_HORIZ) ? AG_INT(2) :
		                                                        AG_INT(3));
		if (pa->dmoving && WIDGET(pa)->window != NULL) {
			/* Set up for receiving motion events exclusively. */
			WIDGET(pa)->window->widExclMotion = WIDGET(pa);
		}
		AG_Redraw(pa);
	}
}

/* Move a divider to the given position. */
int
AG_PaneMoveDivider(AG_Pane *pa, int dx)
{
	AG_SizeAlloc a;
	int rv;

	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
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
		WIDGET(pa)->flags |= AG_WIDGET_UPDATE_WINDOW;
		pa->rx = rv;
	}

	AG_Redraw(pa);
	AG_ObjectUnlock(pa);
	return (rv);
}

/* Move a divider to the given position (%). */
int
AG_PaneMoveDividerPct(AG_Pane *pa, int pct)
{
	int rv;

	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
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
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Pane *pa = AG_PANE_SELF();
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	const int dx = AG_INT(3);
	const int dy = AG_INT(4);

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (pa->dmoving) {
			pa->rx = pa->dx;
			pa->rx += dx;
			if (pa->rx < 2) { pa->rx = 2; }
			AG_PaneMoveDivider(pa, pa->rx);
		} else if (OverDiv(pa, x)) {
			/*
			 * Synthesize the behavior of USE_MOUSEOVER since we want
			 * to set MOUSEOVER state only when the cursor is over
			 * the divider bar (and not the rest of the container).
			 */
			if ((WIDGET(pa)->flags & AG_WIDGET_MOUSEOVER) == 0) {
				WIDGET(pa)->flags |= AG_WIDGET_MOUSEOVER;
				AG_Redraw(pa);
			}
		} else {
			if (WIDGET(pa)->flags & AG_WIDGET_MOUSEOVER) {
				WIDGET(pa)->flags &= ~(AG_WIDGET_MOUSEOVER);
				AG_Redraw(pa);
			}
		}
		break;
	case AG_PANE_VERT:
		if (pa->dmoving) {
			pa->rx = pa->dx;
			pa->rx += dy;
			AG_PaneMoveDivider(pa, pa->rx);
			if (pa->rx < 2) { pa->rx = 2; }
		} else if (OverDiv(pa, y)) {
			/*
			 * Synthesize the behavior of USE_MOUSEOVER since we want
			 * to set MOUSEOVER state only when the cursor is over
			 * the divider bar (and not the rest of the container).
			 */
			if ((WIDGET(pa)->flags & AG_WIDGET_MOUSEOVER) == 0) {
				WIDGET(pa)->flags |= AG_WIDGET_MOUSEOVER;
				AG_Redraw(pa);
			}
		} else {
			if (WIDGET(pa)->flags & AG_WIDGET_MOUSEOVER) {
				WIDGET(pa)->flags &= ~(AG_WIDGET_MOUSEOVER);
				AG_Redraw(pa);
			}
		}
		break;
	default:
		break;
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Pane *pa = AG_PANE_SELF();

	if (pa->dmoving) {
		if (WIDGET(pa)->window != NULL) {
			/* No longer receiving motion events exclusively. */
			WIDGET(pa)->window->widExclMotion = NULL;
		}
		pa->dmoving = 0;
		AG_Redraw(pa);
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Pane *pa = obj;
	int i;

	WIDGET(pa)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP |
			     AG_WIDGET_UNFOCUSED_MOTION;

	pa->type = AG_PANE_VERT;
	pa->flags = 0;
	pa->div[0] = AG_BoxNew(pa, AG_BOX_VERT, 0);
	pa->div[1] = AG_BoxNew(pa, AG_BOX_VERT, 0);
	pa->dx = 0;
	pa->rx = -1;
	pa->rxPct = -1;
	pa->dmoving = 0;
	pa->wDiv = 8;
	pa->ca = NULL;
	pa->resizeAction = AG_PANE_EXPAND_DIV2;

	for (i = 0; i < 2; i++) {
		pa->wMin[i] = 0;
		pa->hMin[i] = 0;
		pa->wReq[i] = -1;
		pa->hReq[i] = -1;
	}

	AG_SetEvent(pa, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(pa, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(pa, "mouse-motion", MouseMotion, NULL);
}

/*
 * Set the width of the divider in pixels.
 * If 0, the divider will not be visible or selectable.
 * If -1, reset to the default which is zoom-dependent.
 */
void
AG_PaneSetDividerWidth(AG_Pane *pa, int wDiv)
{
	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
	AG_ObjectLock(pa);

	if (wDiv == -1) {
		pa->flags &= ~(AG_PANE_OVERRIDE_WDIV);
		pa->wDiv = 0;
	} else {
		pa->flags |= AG_PANE_OVERRIDE_WDIV;
		pa->wDiv = wDiv;
	}
	AG_Redraw(pa);

	AG_ObjectUnlock(pa);
}

void
AG_PaneSetDivisionPacking(AG_Pane *pa, int which, enum ag_box_type packing)
{
	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
	AG_ObjectLock(pa);

	AG_BoxSetType(pa->div[which], packing);

	AG_ObjectUnlock(pa);
}

void
AG_PaneSetDivisionMin(AG_Pane *pa, int which, int wMin, int hMin)
{
	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
	pa->wMin[which] = wMin;
	pa->hMin[which] = hMin;
}

void
AG_PaneAttachBox(AG_Pane *pa, int which, AG_Box *box)
{
	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
	AG_ObjectLock(pa);
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
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
	AG_Redraw(pa);
	AG_ObjectUnlock(pa);
}

void
AG_PaneAttachBoxes(AG_Pane *pa, AG_Box *box1, AG_Box *box2)
{
	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
	AG_ObjectLock(pa);

	AG_PaneAttachBox(pa, 0, box1);
	AG_PaneAttachBox(pa, 1, box2);

	AG_Redraw(pa);
	AG_ObjectUnlock(pa);
}

void
AG_PaneResizeAction(AG_Pane *pa, enum ag_pane_resize_action ra)
{
	AG_OBJECT_ISA(pa, "AG_Widget:AG_Pane:*");
	pa->resizeAction = ra;
}

static void
DrawHorizDivider(AG_Pane *_Nonnull pa, int x, int y)
{
	const AG_Color *cLine = &WCOLOR(pa, LINE_COLOR);
	AG_Rect r;
	const int wDiv = pa->wDiv;
	int xMid = x + (wDiv >> 1) - 1;

	r.x = x+1;
	r.y = 0;
	r.w = wDiv-2;
	r.h = HEIGHT(pa);

	if (pa->dmoving) {
		AG_DrawBoxSunk(pa, &r, &WCOLOR(pa, FG_COLOR));
		xMid++;
		y++;
	} else {
		AG_DrawBoxRaised(pa, &r, &WCOLOR(pa, FG_COLOR));
	}

	AG_PutPixel(pa, xMid, y,   cLine);
	if (y-5 > 0)   { AG_PutPixel(pa, xMid, y-5, cLine); }
	if (y+5 < r.h) { AG_PutPixel(pa, xMid, y+5, cLine); }
}

static void
DrawVertDivider(AG_Pane *_Nonnull pa, int x, int y)
{
	const AG_Color *cLine = &WCOLOR(pa, LINE_COLOR);
	AG_Rect r;
	const int wDiv = pa->wDiv;
	int yMid = y + (wDiv >> 1);

	r.x = 0;
	r.y = y+1;
	r.w = WIDTH(pa);
	r.h = wDiv-2;
	
	if (pa->dmoving) {
		AG_DrawBoxSunk(pa, &r, &WCOLOR(pa, FG_COLOR));
		x++;
		yMid++;
	} else {
		AG_DrawBoxRaised(pa, &r, &WCOLOR(pa, FG_COLOR));
	}

	AG_PutPixel(pa, x,   yMid, cLine);
	if (x-5 > 0)   { AG_PutPixel(pa, x-5, yMid, cLine); }
	if (x+5 < r.w) { AG_PutPixel(pa, x+5, yMid, cLine); }
}

static void
Draw(void *_Nonnull obj)
{
	AG_Pane *pa = obj;
	
	AG_WidgetDraw(pa->div[0]);
	AG_WidgetDraw(pa->div[1]);

	if (pa->wDiv > 0) {
		switch (pa->type) {
		case AG_PANE_HORIZ:
			DrawHorizDivider(pa, pa->dx, HEIGHT(pa) >> 1);
			break;
		case AG_PANE_VERT:
			DrawVertDivider(pa, WIDTH(pa) >> 1, pa->dx);
			break;
		default:
			break;
		}
	}
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
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
		default:
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
	default:
		break;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	static const int zoomSizes[] = {
	     6,  6,  7,  7,  7,  7,  7,  8,  /* 12.5% - 90% */
	     8,  9, 10, 11, 12, 12,          /* 100% - 170% */
	    13, 13, 14, 14, 15, 16           /* 200% - 650% */
	};
	AG_Pane *pa = obj;
	AG_SizeReq r1, r2;
	AG_SizeAlloc a1, a2;
	AG_Rect r;
	const int zoomLvl = WIDGET(pa)->window->zoom;
#ifdef AG_DEBUG
	if (zoomLvl < 0 || zoomLvl >= sizeof(zoomSizes)/sizeof(int))
		AG_FatalError("zoomLvl");
#endif
	a1.x = 0;
	a1.y = 0;

	AG_WidgetSizeReq(pa->div[0], &r1);
	AG_WidgetSizeReq(pa->div[1], &r2);

	pa->wDiv = zoomSizes[zoomLvl];

	switch (pa->type) {
	case AG_PANE_HORIZ:
		if (pa->dx == 0 && pa->rx == -1) {
			if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = a->w - pa->wReq[1];
			} else {
				if (pa->rxPct != -1) {
					pa->dx = pa->rxPct*WIDTH(pa)/100;
				} else {
					pa->dx = pa->wReq[0];
				}
			}
			pa->rx = pa->dx;
		} else if ((WIDTH(pa->div[0]) + pa->wDiv + WIDTH(pa->div[1])) 
			    != a->w) {
			/* Pane resize as opposed to divider move */
			switch (pa->resizeAction) {
			case AG_PANE_EXPAND_DIV1:
				pa->dx = a->w - (WIDTH(pa->div[1]) + pa->wDiv);
				break;
			case AG_PANE_EXPAND_DIV2:
				break;
			case AG_PANE_DIVIDE_EVEN:
				/* The difference halved */
				pa->dx += (a->w - (WIDTH(pa->div[0]) +
				    pa->wDiv + WIDTH(pa->div[1]))) / 2;
				break;
			case AG_PANE_DIVIDE_PCT:
				pa->dx = pa->rxPct*WIDTH(pa)/100;
				break;
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

		/* Set up cursor-change area */
		r.x = pa->dx;
		r.y = 0;
		r.w = pa->wDiv;
		r.h = a->h;
		AG_SetStockCursor(pa, &pa->ca, &r, AG_HRESIZE_CURSOR);
		break;
	case AG_PANE_VERT:
		if (pa->dx == 0 && pa->rx == -1) {
			if (pa->flags & AG_PANE_DIV1FILL) {
				pa->dx = a->h - pa->hReq[1];
			} else {
				if (pa->rxPct != -1) {
					pa->dx = pa->rxPct*HEIGHT(pa)/100;
				} else {
					pa->dx = pa->hReq[0];
				}
			}
			pa->rx = pa->dx;
		} else if ((HEIGHT(pa->div[0]) + pa->wDiv + HEIGHT(pa->div[1]))
		    != a->h) {
			/* Pane resize as opposed to divider move */
			switch (pa->resizeAction) {
			case AG_PANE_EXPAND_DIV1:
				pa->dx = a->h - (HEIGHT(pa->div[1]) + pa->wDiv);
				break;
			case AG_PANE_EXPAND_DIV2:
				break;
			case AG_PANE_DIVIDE_EVEN:
				/* The difference halved */
				pa->dx += (a->h - (HEIGHT(pa->div[0]) + 
				    pa->wDiv + HEIGHT(pa->div[1]))) / 2;
				break;
			case AG_PANE_DIVIDE_PCT:
				pa->dx = pa->rxPct*HEIGHT(pa)/100;
				break;
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

		/* Set up cursor-change area */
		r.x = 0;
		r.y = pa->dx;
		r.w = a->w;
		r.h = pa->wDiv;
		AG_SetStockCursor(pa, &pa->ca, &r, AG_VRESIZE_CURSOR);
		break;
	default:
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
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */

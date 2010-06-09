/*
 * Copyright (c) 2008-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "window.h"
#include "scrollview.h"
#include "primitive.h"
#include "gui_math.h"

#include <stdarg.h>
#include <string.h>

/*
 * Clip widgets completely outside of the view in a more efficient way,
 * and adjust the sensitivity rectangle of partially hidden widgets.
 */
static void
ClipWidgets(AG_Scrollview *sv, AG_Widget *wt)
{
	AG_Widget *chld;
	AG_Rect2 rView = WIDGET(sv)->rView;
	AG_Rect2 rx;

	rView.w -= sv->wBar;
	rView.h -= sv->hBar;
	rView.x2 = rView.x1+rView.w;
	rView.y2 = rView.y1+rView.h;
	
	if (rView.w < 0 || rView.h < 0)
		return;

	rx = AG_RectIntersect2(&rView, &wt->rView);
	if (rx.w == 0 || rx.h == 0) {
		wt->flags |= AG_WIDGET_HIDE;
	} else {
		wt->flags &= ~(AG_WIDGET_HIDE);
	}
	OBJECT_FOREACH_CHILD(chld, wt, ag_widget)
		ClipWidgets(sv, chld);
}

/* Place child widgets at the current offset in the Scrollview. */
static void
PlaceWidgets(AG_Scrollview *sv, int *wTot, int *hTot)
{
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	AG_Widget *chld;

	aChld.x = -sv->xOffs;
	aChld.y = -sv->yOffs;

	OBJECT_FOREACH_CHILD(chld, sv, ag_widget) {
		if (chld == WIDGET(sv->vbar) || chld == WIDGET(sv->hbar)) {
			continue;
		}
		AG_WidgetSizeReq(chld, &rChld);
		aChld.w = rChld.w;
		aChld.h = rChld.h;
		AG_WidgetSizeAlloc(chld, &aChld);
		switch (sv->pack) {
		case AG_PACK_HORIZ:
			aChld.x += aChld.w;
			break;
		case AG_PACK_VERT:
			aChld.y += aChld.h;
			break;
		}
		ClipWidgets(sv, chld);
	}
	switch (sv->pack) {
	case AG_PACK_HORIZ:
		if (wTot != NULL)
			*wTot = aChld.x + sv->xOffs;
		if (hTot != NULL)
			*hTot = aChld.y+aChld.h + sv->yOffs;
		break;
	case AG_PACK_VERT:
		if (wTot != NULL)
			*wTot = aChld.x+aChld.w + sv->xOffs;
		if (hTot != NULL)
			*hTot = aChld.y + sv->yOffs;
		break;
	}
}

static void
PanView(AG_Event *event)
{
	AG_Scrollview *sv = AG_PTR(1);

	AG_WidgetUpdate(sv);
	PlaceWidgets(sv, NULL, NULL);
	AG_WidgetUpdate(sv);
	AG_Redraw(sv);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Scrollview *sv = AG_SELF();
	int dx = AG_INT(3);
	int dy = AG_INT(4);
	AG_Event ev;

	if (sv->flags & AG_SCROLLVIEW_PANNING) {
		sv->xOffs -= dx;
		sv->yOffs -= dy;

		if (sv->xOffs+sv->r.w > sv->xMax)
			sv->xOffs = sv->xMax-sv->r.w;
		if (sv->yOffs+sv->r.h > sv->yMax)
			sv->yOffs = sv->yMax-sv->r.h;
		if (sv->xOffs < 0)
			sv->xOffs = 0;
		if (sv->yOffs < 0)
			sv->yOffs = 0;
	}
	AG_EventInit(&ev);
	AG_EventPushPointer(&ev, NULL, sv);
	PanView(&ev);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Scrollview *sv = AG_SELF();
	int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_MIDDLE:
		sv->flags &= ~(AG_SCROLLVIEW_PANNING);
		break;
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Scrollview *sv = AG_SELF();
	int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_MIDDLE:
		sv->flags |= AG_SCROLLVIEW_PANNING;
		AG_WidgetFocus(sv);
		break;
	default:
		break;
	}
}

AG_Scrollview *
AG_ScrollviewNew(void *parent, Uint flags)
{
	AG_Scrollview *sv;

	sv = Malloc(sizeof(AG_Scrollview));
	AG_ObjectInit(sv, &agScrollviewClass);
	sv->flags |= flags;

	if (flags & AG_SCROLLVIEW_HFILL) { AG_ExpandHoriz(sv); }
	if (flags & AG_SCROLLVIEW_VFILL) { AG_ExpandVert(sv); }

	if (!(flags & AG_SCROLLVIEW_NOPAN_X)) {
		sv->hbar = AG_ScrollbarNew(sv, AG_SCROLLBAR_HORIZ, 0);
		AG_BindInt(sv->hbar, "value", &sv->xOffs);
		AG_BindInt(sv->hbar, "min", &sv->xMin);
		AG_BindInt(sv->hbar, "max", &sv->xMax);
		AG_BindInt(sv->hbar, "visible", &sv->r.w);
		AG_SetEvent(sv->hbar, "scrollbar-changed", PanView, "%p", sv);
	}
	if (!(flags & AG_SCROLLVIEW_NOPAN_Y)) {
		sv->vbar = AG_ScrollbarNew(sv, AG_SCROLLBAR_VERT, 0);
		AG_BindInt(sv->vbar, "value", &sv->yOffs);
		AG_BindInt(sv->vbar, "min", &sv->yMin);
		AG_BindInt(sv->vbar, "max", &sv->yMax);
		AG_BindInt(sv->vbar, "visible", &sv->r.h);
		AG_SetEvent(sv->vbar, "scrollbar-changed", PanView, "%p", sv);
	}

	if (flags & AG_SCROLLVIEW_BY_MOUSE) {
		WIDGET(sv)->flags |= AG_WIDGET_FOCUSABLE;
		AG_SetEvent(sv, "mouse-button-down", MouseButtonDown, NULL);
		AG_SetEvent(sv, "mouse-button-up", MouseButtonUp, NULL);
		AG_SetEvent(sv, "mouse-motion", MouseMotion, NULL);
	}
	AG_ScrollviewSetIncrement(sv, 10);
	AG_ObjectAttach(parent, sv);
	return (sv);
}

void
AG_ScrollviewSetIncrement(AG_Scrollview *sv, int incr)
{
	AG_ObjectLock(sv);
	sv->incr = incr;
	if (sv->hbar != NULL) { AG_ScrollbarSetIntIncrement(sv->hbar, incr); }
	if (sv->vbar != NULL) { AG_ScrollbarSetIntIncrement(sv->vbar, incr); }
	AG_ObjectUnlock(sv);
}

static void
Init(void *obj)
{
	AG_Scrollview *sv = obj;

	sv->flags = 0;
	sv->wPre = 256;
	sv->hPre = 256;
	sv->xOffs = 0;
	sv->yOffs = 0;
	sv->xMin = 0;
	sv->yMin = 0;
	sv->xMax = 0;
	sv->yMax = 0;
	sv->hbar = NULL;
	sv->vbar = NULL;
	sv->wBar = 0;
	sv->hBar = 0;
	sv->pack = AG_PACK_VERT;
	sv->r = AG_RECT(0,0,0,0);
	sv->incr = 10;

#ifdef AG_DEBUG
	AG_BindUint(sv, "flags", &sv->flags);
	AG_BindUint(sv, "pack", &sv->pack);
	AG_BindInt(sv, "wPre", &sv->wPre);
	AG_BindInt(sv, "hPre", &sv->hPre);
	AG_BindInt(sv, "xOffs", &sv->xOffs);
	AG_BindInt(sv, "yOffs", &sv->yOffs);
	AG_BindInt(sv, "xMin", &sv->xMin);
	AG_BindInt(sv, "yMin", &sv->yMin);
	AG_BindInt(sv, "xMax", &sv->xMax);
	AG_BindInt(sv, "yMax", &sv->yMax);
	AG_BindInt(sv, "wBar", &sv->wBar);
	AG_BindInt(sv, "hBar", &sv->wBar);
	AG_BindInt(sv, "incr", &sv->incr);
#endif /* AG_DEBUG */
}

void
AG_ScrollviewSizeHint(AG_Scrollview *sv, Uint w, Uint h)
{
	AG_ObjectLock(sv);
	sv->wPre = w;
	sv->hPre = h;
	AG_ObjectUnlock(sv);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Scrollview *sv = p;
	AG_SizeReq rBar, rChld;
	AG_Widget *chld;
	int wMax = 0, hMax = 0;
	
	r->w = sv->wPre;
	r->h = sv->hPre;
	
	if (sv->hbar != NULL) {
		AG_WidgetSizeReq(sv->hbar, &rBar);
		r->h += rBar.h;
	}
	if (sv->vbar != NULL) {
		AG_WidgetSizeReq(sv->vbar, &rBar);
		r->w += rBar.w;
	}
	
	OBJECT_FOREACH_CHILD(chld, sv, ag_widget) {
		if (chld == WIDGET(sv->vbar) || chld == WIDGET(sv->hbar)) {
			continue;
		}
		AG_WidgetSizeReq(chld, &rChld);
		if (rChld.w > wMax) { wMax = rChld.w; }
		if (rChld.h > hMax) { hMax = rChld.h; }
		switch (sv->pack) {
		case AG_PACK_HORIZ:
			r->h = MAX(r->h, hMax);
			r->w += rChld.w;
			break;
		case AG_PACK_VERT:
			r->w = MAX(r->w, wMax);
			r->h += rChld.h;
			break;
		}
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Scrollview *sv = p;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;
	int wTot, hTot;

	sv->r.w = a->w;
	sv->r.h = a->h;

	if (sv->hbar != NULL) {
		AG_WidgetSizeReq(sv->hbar, &rBar);
		aBar.w = a->w - rBar.h;
		aBar.h = rBar.h;
		aBar.x = 0;
		aBar.y = a->h - rBar.h;
		AG_WidgetSizeAlloc(sv->hbar, &aBar);
		sv->r.h -= aBar.h;
		sv->hBar = aBar.h;
		if (sv->r.h < 0) { sv->r.h = 0; }
	} else {
		sv->hBar = 0;
	}
	if (sv->vbar != NULL) {
		AG_WidgetSizeReq(sv->vbar, &rBar);
		aBar.w = rBar.w;
		aBar.h = a->h - rBar.w;
		aBar.x = a->w - rBar.w;
		aBar.y = 0;
		AG_WidgetSizeAlloc(sv->vbar, &aBar);
		sv->r.w -= aBar.w;
		sv->wBar = aBar.w;
		if (sv->r.w < 0) { sv->r.w = 0; }
	} else {
		sv->wBar = 0;
	}

	PlaceWidgets(sv, &wTot, &hTot);
	sv->xMax = wTot;
	sv->yMax = hTot;

	if (sv->hbar != NULL) {
		if ((sv->xMax - sv->r.w - sv->xOffs) < 0)
			sv->xOffs = MAX(0, sv->xMax - sv->r.w);
	}
	if (sv->vbar != NULL) {
		if ((sv->yMax - sv->r.h - sv->yOffs) < 0)
			sv->yOffs = MAX(0, sv->yMax - sv->r.h);
	}
#if 0
	if (a->w >= (wTot - sv->xOffs)) {
		sv->xOffs = wTot - a->w;
		if (sv->xOffs < 0) { sv->xOffs = 0; }
	}
	if (a->h >= (hTot - sv->yOffs)) {
		sv->yOffs = hTot - a->h;
		if (sv->yOffs < 0) { sv->yOffs = 0; }
	}
#endif
	return (0);
}

static void
Draw(void *p)
{
	AG_Scrollview *sv = p;
	AG_Widget *chld;

	if (sv->flags & AG_SCROLLVIEW_FRAME) {
		AG_DrawBox(sv,
		    AG_RECT(0, 0, WIDTH(sv), HEIGHT(sv)), -1,
		    agColors[FRAME_COLOR]);
	}

	if (sv->hbar != NULL) { AG_WidgetDraw(sv->hbar); }
	if (sv->vbar != NULL) { AG_WidgetDraw(sv->vbar); }
	
	AG_PushClipRect(sv, sv->r);
	WIDGET_FOREACH_CHILD(chld, sv) {
		if (chld->flags & AG_WIDGET_HIDE ||
		    chld == WIDGET(sv->hbar) ||
		    chld == WIDGET(sv->vbar)) {
			continue;
		}
		AG_WidgetDraw(chld);
	
		if (chld->rView.x2 > WIDGET(sv)->rView.x2 - sv->wBar) {
			chld->rSens.w = WIDGET(sv)->rView.x2 - sv->wBar -
			                WIDGET(chld)->rView.x1;
		} else {
			chld->rSens.w = chld->w;
		}
		chld->rSens.x2 = chld->rSens.x1+chld->rSens.w;
		
		if (chld->rView.y2 > WIDGET(sv)->rView.y2 - sv->hBar) {
			chld->rSens.h = WIDGET(sv)->rView.y2 - sv->hBar -
			                WIDGET(chld)->rView.y1;
		} else {
			chld->rSens.h = chld->h;
		}
		chld->rSens.y2 = chld->rSens.y1+chld->rSens.h;
	}
	AG_PopClipRect(sv);
}

AG_WidgetClass agScrollviewClass = {
	{
		"Agar(Widget:Scrollview)",
		sizeof(AG_Scrollview),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

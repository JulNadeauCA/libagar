/*
 * Copyright (c) 2008-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Scrollable view container. It allows child widgets to extend past its own
 * boundaries. It handles clipping (including clipping of mouse events for
 * partially-visible widgets) and culling of non-visible widgets.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/window.h>
#include <agar/gui/scrollview.h>
#include <agar/gui/primitive.h>
#include <agar/gui/gui_math.h>

#include <stdarg.h>
#include <string.h>

#if 0
/* Detect and clip widgets which are completely hidden. */
static void
ClipWidgets(AG_Scrollview *_Nonnull sv, AG_Widget *_Nonnull wt)
{
	AG_Rect2 r, rx;
	AG_Widget *chld;

	r = WIDGET(sv)->rView;
	r.w -= sv->wBar;
	r.h -= sv->hBar;
	r.x2 = r.x1 + r.w;
	r.y2 = r.y1 + r.h;
	
	if (r.w < 0 || r.h < 0)
		return;

	if (!AG_RectIntersect2(&rx, &r, &wt->rView)) {
		AG_WidgetHideAll(wt);
	} else {
		AG_WidgetShowAll(wt);
	}

	OBJECT_FOREACH_CHILD(chld, wt, ag_widget)
		ClipWidgets(sv, chld);
}
#endif

/* Place child widgets at the current offset in the Scrollview. */
static void
PlaceWidgets(AG_Scrollview *_Nonnull sv, int *_Nullable wTot,
    int *_Nullable hTot)
{
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	AG_Widget *chld;

	aChld.x = WIDGET(sv)->paddingLeft - sv->xOffs;
	aChld.y = WIDGET(sv)->paddingTop - sv->yOffs;

	OBJECT_FOREACH_CHILD(chld, sv, ag_widget) {
		if (chld == WIDGET(sv->vbar) || chld == WIDGET(sv->hbar)) {
			continue;
		}
		AG_WidgetSizeReq(chld, &rChld);
		aChld.w = rChld.w;
		aChld.h = rChld.h;
		AG_WidgetSizeAlloc(chld, &aChld);
		switch (sv->type) {
		case AG_SCROLLVIEW_HORIZ:
			aChld.x += aChld.w;
			break;
		case AG_SCROLLVIEW_VERT:
			aChld.y += aChld.h;
			break;
		}
#if 0
		ClipWidgets(sv, chld);
#endif
		AG_WidgetUpdateCoords(chld, 0,0);
	}
	switch (sv->type) {
	case AG_SCROLLVIEW_HORIZ:
		if (wTot != NULL) {
			*wTot = aChld.x + sv->xOffs +
		                WIDGET(sv)->paddingRight;
		}
		if (hTot != NULL) {
			*hTot = aChld.y+aChld.h + sv->yOffs +
			        WIDGET(sv)->paddingBottom;
		}
		break;
	case AG_SCROLLVIEW_VERT:
		if (wTot != NULL) {
			*wTot = aChld.x+aChld.w + sv->xOffs +
			        WIDGET(sv)->paddingRight;
		}
		if (hTot != NULL) {
			*hTot = aChld.y + sv->yOffs +
			        WIDGET(sv)->paddingBottom;
		}
		break;
	}
}

static void
ScrollbarChanged(AG_Event *_Nonnull event)
{
	AG_Scrollview *sv = AG_SCROLLVIEW_PTR(1);

	PlaceWidgets(sv, NULL, NULL);                    /* Update clipping */
	WIDGET(sv)->flags |= AG_WIDGET_UPDATE_WINDOW;
	AG_Redraw(sv);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Scrollview *sv = AG_SCROLLVIEW_SELF();
	const int dx = AG_INT(3);
	const int dy = AG_INT(4);

	if (sv->flags & AG_SCROLLVIEW_PANNING) {
		sv->xOffs -= dx;
		sv->yOffs -= dy;

		if (sv->xOffs + sv->r.w > sv->xMax)
			sv->xOffs = sv->xMax - sv->r.w;
		if (sv->yOffs + sv->r.h > sv->yMax)
			sv->yOffs = sv->yMax - sv->r.h;
		if (sv->xOffs < 0)
			sv->xOffs = 0;
		if (sv->yOffs < 0)
			sv->yOffs = 0;
	}

	PlaceWidgets(sv, NULL, NULL);                    /* Update clipping */
	WIDGET(sv)->flags |= AG_WIDGET_UPDATE_WINDOW;
	AG_Redraw(sv);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Scrollview *sv = AG_SCROLLVIEW_SELF();
	const int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_MIDDLE:
		sv->flags &= ~(AG_SCROLLVIEW_PANNING);
		break;
	case AG_MOUSE_LEFT:
		if (sv->flags & AG_SCROLLVIEW_PAN_LEFT) {
			sv->flags &= ~(AG_SCROLLVIEW_PANNING);
		}
		break;
	case AG_MOUSE_RIGHT:
		if (sv->flags & AG_SCROLLVIEW_PAN_RIGHT) {
			sv->flags &= ~(AG_SCROLLVIEW_PANNING);
		}
		break;
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Scrollview *sv = AG_SCROLLVIEW_SELF();
	const int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_MIDDLE:
		sv->flags |= AG_SCROLLVIEW_PANNING;
		if (!AG_WidgetIsFocused(sv)) {
			AG_WidgetFocus(sv);
		}
		break;
	case AG_MOUSE_LEFT:
		if (sv->flags & AG_SCROLLVIEW_PAN_LEFT) {
			sv->flags |= AG_SCROLLVIEW_PANNING;
			if (!AG_WidgetIsFocused(sv))
				AG_WidgetFocus(sv);
		}
		break;
	case AG_MOUSE_RIGHT:
		if (sv->flags & AG_SCROLLVIEW_PAN_RIGHT) {
			sv->flags |= AG_SCROLLVIEW_PANNING;
			if (!AG_WidgetIsFocused(sv))
				AG_WidgetFocus(sv);
		}
		break;
	case AG_MOUSE_WHEELUP:
		if ((sv->yOffs -= AG_GetInt(sv,"y-scroll-amount")) < 0) {
			sv->yOffs = 0;
		}
		break;
	case AG_MOUSE_WHEELDOWN:
		if ((sv->yOffs += AG_GetInt(sv,"y-scroll-amount")) > sv->xMax) {
			sv->yOffs = sv->xMax;
		}
		break;
	case AG_MOUSE_X1:
		if ((sv->xOffs -= AG_GetInt(sv,"x-scroll-amount")) < 0) {
			sv->xOffs = 0;
		}
		break;
	case AG_MOUSE_X2:
		if ((sv->xOffs += AG_GetInt(sv,"x-scroll-amount")) > sv->xMax) {
			sv->xOffs = sv->xMax;
		}
		break;
	}

	PlaceWidgets(sv, NULL, NULL);                    /* Update clipping */
	WIDGET(sv)->flags |= AG_WIDGET_UPDATE_WINDOW;
	AG_Redraw(sv);
}

AG_Scrollview *
AG_ScrollviewNew(void *parent, Uint flags)
{
	AG_Scrollview *sv;
	AG_Scrollbar *sb;

	sv = Malloc(sizeof(AG_Scrollview));
	AG_ObjectInit(sv, &agScrollviewClass);

	if (flags & AG_SCROLLVIEW_HFILL) { WIDGET(sv)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_SCROLLVIEW_VFILL) { WIDGET(sv)->flags |= AG_WIDGET_VFILL; }
	sv->flags |= flags;

	if (!(flags & AG_SCROLLVIEW_NOPAN_X)) {
		sb = sv->hbar = AG_ScrollbarNew(sv, AG_SCROLLBAR_HORIZ,
		    AG_SCROLLBAR_EXCL);
		AG_BindInt(sb, "min", &sv->xMin);
		AG_BindInt(sb, "max", &sv->xMax);
		AG_BindInt(sb, "visible", &sv->r.w);
		AG_BindInt(sb, "value", &sv->xOffs);
		AG_SetEvent(sb, "scrollbar-changed", ScrollbarChanged,"%p",sv);
	}
	if (!(flags & AG_SCROLLVIEW_NOPAN_Y)) {
		sv->vbar = sb = AG_ScrollbarNew(sv, AG_SCROLLBAR_VERT,
		    AG_SCROLLBAR_EXCL);
		AG_BindInt(sb, "min", &sv->yMin);
		AG_BindInt(sb, "max", &sv->yMax);
		AG_BindInt(sb, "visible", &sv->r.h);
		AG_BindInt(sb, "value", &sv->yOffs);
		AG_SetEvent(sb, "scrollbar-changed", ScrollbarChanged,"%p",sv);
	}

	if (flags & AG_SCROLLVIEW_BY_MOUSE) {
		WIDGET(sv)->flags |= AG_WIDGET_UNFOCUSED_MOTION |
	                             AG_WIDGET_UNFOCUSED_BUTTONDOWN |
			             AG_WIDGET_UNFOCUSED_BUTTONUP;

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
	AG_OBJECT_ISA(sv, "AG_Widget:AG_Scrollview:*");
	AG_ObjectLock(sv);

	if (sv->hbar) { AG_SetInt(sv->hbar, "inc", incr); }
	if (sv->vbar) { AG_SetInt(sv->vbar, "inc", incr); }

	AG_ObjectUnlock(sv);
}

static void
Init(void *_Nonnull obj)
{
	AG_Scrollview *sv = obj;

	sv->flags = 0;
	sv->type = AG_SCROLLVIEW_VERT;			/* Pack vertically */
	sv->style = AG_SCROLLVIEW_STYLE_WELL;		/* 3D well */
	memset(&sv->wPre, 0, sizeof(int) +              /* wPre */
	                     sizeof(int) +		/* hPre */
	                     sizeof(int) +		/* xOffs */
	                     sizeof(int) +		/* yOffs */
	                     sizeof(int) +		/* xMin */
	                     sizeof(int) +		/* xMax */
	                     sizeof(int) +		/* yMin */
	                     sizeof(int) +		/* yMax */
	                     sizeof(int) +		/* wBar */
	                     sizeof(int) +		/* hBar */
	                     sizeof(AG_Rect) +		/* r */
	                     sizeof(AG_Scrollbar *) +	/* hbar */
	                     sizeof(AG_Scrollbar *));	/* vbar */

	AG_SetInt(sv, "x-scroll-amount", 40);
	AG_SetInt(sv, "y-scroll-amount", 40);
}

/* Request an explicit size in pixels (default = 0 = auto) */
void
AG_ScrollviewSizeHint(AG_Scrollview *sv, Uint w, Uint h)
{
	AG_OBJECT_ISA(sv, "AG_Widget:AG_Scrollview:*");
	sv->wPre = w;
	sv->hPre = h;
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	AG_Scrollview *sv = p;
	AG_SizeReq rBar, rChld;
	AG_Widget *chld;
	
	r->w = WIDGET(sv)->paddingLeft + sv->wPre + WIDGET(sv)->paddingRight;
	r->h = WIDGET(sv)->paddingTop + sv->hPre + WIDGET(sv)->paddingBottom;
	
	if (sv->hbar) {
		AG_WidgetSizeReq(sv->hbar, &rBar);
		r->h += rBar.h;
	}
	if (sv->vbar) {
		AG_WidgetSizeReq(sv->vbar, &rBar);
		r->w += rBar.w;
	}
	if (sv->wPre == 0 || sv->hPre == 0) {                  /* Auto-size */
		int wMax = 0, hMax = 0;

		OBJECT_FOREACH_CHILD(chld, sv, ag_widget) {
			if (chld == WIDGET(sv->vbar) ||
			    chld == WIDGET(sv->hbar)) {
				continue;
			}
			AG_WidgetSizeReq(chld, &rChld);
			if (rChld.w > wMax) { wMax = rChld.w; }
			if (rChld.h > hMax) { hMax = rChld.h; }
			switch (sv->type) {
			case AG_SCROLLVIEW_HORIZ:
				r->h = MAX(r->h, hMax);
				r->w += rChld.w;
				break;
			case AG_SCROLLVIEW_VERT:
				r->w = MAX(r->w, wMax);
				r->h += rChld.h;
				break;
			}
		}
	}
}

static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
{
	AG_Scrollview *sv = p;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;
	int wTot, hTot;

	sv->r.w = a->w;
	sv->r.h = a->h;

	if (sv->hbar) {
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
	if (sv->vbar) {
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

	if (sv->hbar) {
		if ((sv->xMax - sv->r.w - sv->xOffs) < 0)
			sv->xOffs = MAX(0, sv->xMax - sv->r.w);
	}
	if (sv->vbar) {
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
Draw(void *_Nonnull p)
{
	static void (*pfBox[])(void *, const AG_Rect *, const AG_Color *) = {
		ag_draw_rect_noop,  /* NONE */
		ag_draw_box_raised, /* BOX */
		ag_draw_box_sunk,   /* WELL */
		ag_draw_rect        /* PLAIN */
	};
	AG_Scrollview *sv = p;
	AG_Widget *chld;
	AG_Rect r = WIDGET(sv)->r;
	int x2,y2;

#ifdef AG_DEBUG
	if (sv->style >= AG_SCROLLVIEW_STYLE_LAST)
		AG_FatalError("style");
#endif
	pfBox[sv->style](sv, &r, &WCOLOR(sv, BG_COLOR));

	if (sv->hbar) {
		AG_WidgetDraw(sv->hbar);
		r.h -= HEIGHT(sv->hbar);
	}
	if (sv->vbar) {
		AG_WidgetDraw(sv->vbar);
		r.w -= WIDTH(sv->vbar);
	}

	x2 = WIDGET(sv)->rView.x2 - sv->wBar;
	y2 = WIDGET(sv)->rView.y2 - sv->hBar;

	r.x++;
	r.y++;
	r.w -= 2;
	r.h -= 2;
	
	/*
	 * TODO determine earlier if there are partially visible widgets
	 * and a clipping rectangle is really needed.
	 */
	AG_PushClipRect(sv, &r);

	OBJECT_FOREACH_CHILD(chld, sv, ag_widget) {
		if (!(chld->flags & AG_WIDGET_VISIBLE) ||
		    chld == WIDGET(sv->hbar) ||
		    chld == WIDGET(sv->vbar)) {
			continue;
		}
		AG_WidgetDraw(chld);
	
		if (chld->rView.x2 > x2) {
			chld->rSens.w = x2 - WIDGET(chld)->rView.x1;
		} else {
			chld->rSens.w = chld->w;
		}
		chld->rSens.x2 = chld->rSens.x1 + chld->rSens.w;
		
		if (chld->rView.y2 > y2) {
			chld->rSens.h = y2 - WIDGET(chld)->rView.y1;
		} else {
			chld->rSens.h = chld->h;
		}
		chld->rSens.y2 = chld->rSens.y1 + chld->rSens.h;
	}

	AG_PopClipRect(sv);
}

AG_WidgetClass agScrollviewClass = {
	{
		"Agar(Widget:Scrollview)",
		sizeof(AG_Scrollview),
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

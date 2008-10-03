/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

/* Disable drawing of widgets completely outside of the view. */
static void
ClipWidgets(AG_Scrollview *sv, AG_Widget *wid, int xRel, int yRel)
{
	AG_Widget *chld;

	if (xRel+wid->x+wid->w < 0 ||
	    yRel+wid->y+wid->h < 0 ||
	    xRel+wid->x > WIDTH(sv) ||
	    yRel+wid->y > HEIGHT(sv)) {
		wid->flags |= AG_WIDGET_HIDE;
	} else {
		wid->flags &= ~(AG_WIDGET_HIDE);
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		ClipWidgets(sv, chld, wid->x, wid->y);
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
		ClipWidgets(sv, chld, 0,0);
	}
	if (wTot != NULL) { *wTot = aChld.x+aChld.w; }
	if (hTot != NULL) { *hTot = aChld.y+aChld.h; }
}

static void
KeyDown(AG_Event *event)
{
	AG_Scrollview *sv = AG_SELF();
	int keysym = AG_INT(1);
	const int scrollIncr = 10; /* XXX */

	switch (keysym) {
	case SDLK_LEFT:
		sv->xOffs -= scrollIncr;
		break;
	case SDLK_RIGHT:
		sv->xOffs += scrollIncr;
		break;
	case SDLK_UP:
		sv->yOffs -= scrollIncr;
		break;
	case SDLK_DOWN:
		sv->yOffs += scrollIncr;
		break;
	case SDLK_0:
		sv->xOffs = 0;
		sv->yOffs = 0;
		break;
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Scrollview *sv = AG_SELF();
	int dx = AG_INT(3);
	int dy = AG_INT(4);

	if (sv->flags & AG_SCROLLVIEW_PANNING) {
		sv->xOffs -= dx;
		sv->yOffs -= dy;
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Scrollview *sv = AG_SELF();
	int button = AG_INT(1);

	switch (button) {
	case SDL_BUTTON_MIDDLE:
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
	case SDL_BUTTON_MIDDLE:
		sv->flags |= AG_SCROLLVIEW_PANNING;
		break;
	default:
		break;
	}
}

static void
Scrolled(AG_Event *event)
{
	AG_Scrollview *sv = AG_PTR(1);
	AG_Window *pWin = AG_ParentWindow(sv);

	PlaceWidgets(sv, NULL, NULL);

	if (pWin != NULL)
		AG_WindowUpdate(pWin);
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

	if (flags & AG_SCROLLVIEW_PAN_X) {
		sv->hbar = AG_ScrollbarNew(sv, AG_SCROLLBAR_HORIZ, 0);
		AG_WidgetBindInt(sv->hbar, "value", &sv->xOffs);
		AG_WidgetBindInt(sv->hbar, "min", &sv->xMin);
		AG_WidgetBindInt(sv->hbar, "max", &sv->xMax);
		AG_WidgetBindInt(sv->hbar, "visible", &WIDGET(sv)->w);
		AG_SetEvent(sv->hbar, "scrollbar-changed", Scrolled, "%p", sv);
	}
	if (flags & AG_SCROLLVIEW_PAN_Y) {
		sv->vbar = AG_ScrollbarNew(sv, AG_SCROLLBAR_VERT, 0);
		AG_WidgetBindInt(sv->vbar, "value", &sv->yOffs);
		AG_WidgetBindInt(sv->vbar, "min", &sv->yMin);
		AG_WidgetBindInt(sv->vbar, "max", &sv->yMax);
		AG_WidgetBindInt(sv->vbar, "visible", &WIDGET(sv)->h);
		AG_SetEvent(sv->vbar, "scrollbar-changed", Scrolled, "%p", sv);
	}

	if (flags & AG_SCROLLVIEW_BY_CURSOR) {
		AG_SetEvent(sv, "window-mousebuttondown", MouseButtonDown,
		    NULL);
		AG_SetEvent(sv, "window-mousebuttonup", MouseButtonUp, NULL);
		AG_SetEvent(sv, "window-mousemotion", MouseMotion, NULL);
	}
	if (flags & AG_SCROLLVIEW_BY_KBD) {
		AG_SetEvent(sv, "window-keydown", KeyDown, NULL);
	}

	AG_ObjectAttach(parent, sv);
	return (sv);
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
	sv->pack = AG_PACK_VERT;
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
	
	if (sv->hbar != NULL) {
		AG_WidgetSizeReq(sv->hbar, &rBar);
		aBar.w = a->w;
		aBar.h = rBar.h;
		aBar.x = 0;
		aBar.y = a->h - rBar.h;
		AG_WidgetSizeAlloc(sv->hbar, &aBar);
	}
	if (sv->vbar != NULL) {
		AG_WidgetSizeReq(sv->vbar, &rBar);
		aBar.w = rBar.w;
		aBar.h = a->h;
		aBar.x = a->w - rBar.w;
		aBar.y = 0;
		AG_WidgetSizeAlloc(sv->vbar, &aBar);
	}

	PlaceWidgets(sv, &wTot, &hTot);
	if (sv->hbar != NULL) { sv->xMax = wTot; }
	if (sv->vbar != NULL) { sv->yMax = hTot; }

	if (a->w >= (wTot - sv->xOffs)) {
		sv->xOffs = wTot - a->w;
		if (sv->xOffs < 0) { sv->xOffs = 0; }
	}
	if (a->h >= (hTot - sv->yOffs)) {
		sv->yOffs = hTot - a->h;
		if (sv->yOffs < 0) { sv->yOffs = 0; }
	}
	return (0);
}

static void
Draw(void *p)
{
	AG_Scrollview *sv = p;
	AG_Widget *chld;
	AG_Rect rView;

	AG_DrawBox(sv,
	    AG_RECT(0, 0, WIDTH(sv), HEIGHT(sv)), -1,
	    AG_COLOR(FRAME_COLOR));

	rView = AG_RECT(0, 0, WIDTH(sv), HEIGHT(sv));
	if (sv->hbar != NULL) { rView.h -= HEIGHT(sv->hbar); }
	if (sv->vbar != NULL) { rView.w -= WIDTH(sv->vbar); }

	if (sv->hbar != NULL) { AG_WidgetDraw(sv->hbar); }
	if (sv->vbar != NULL) { AG_WidgetDraw(sv->vbar); }

	AG_PushClipRect(sv, rView);
	WIDGET_FOREACH_CHILD(chld, sv) {
		if (chld == WIDGET(sv->hbar) || chld == WIDGET(sv->vbar)) {
			continue;
		}
		AG_WidgetDraw(chld);
	}
	AG_PopClipRect();
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

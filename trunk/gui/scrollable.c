/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Scrollable view (not meant to be used as a stand-alone widget).
 *
 * Subclasses of this widget (such as AG_Table) can assume arbitrary
 * geometries and scrollbars will appear if the widgets are being
 * displayed partially. The Scrollable class is mainly responsible for
 * translating the coordinates passed on to the widget draw function
 * and event handlers.
 */

#include <core/core.h>

#include "scrollable.h"
#include "window.h"
#include "primitive.h"
#include "label.h"

void
AG_ScrollableDrawBegin(AG_Scrollable *sa)
{
	sa->save.x = WIDGET(sa)->x;
	sa->save.y = WIDGET(sa)->y;
	sa->save.cx = WIDGET(sa)->cx;
	sa->save.cy = WIDGET(sa)->cy;

	WIDGET(sa)->x = sa->xOffs;
	WIDGET(sa)->y = sa->yOffs;
	WIDGET(sa)->cx = WIDGET(sa)->cx + sa->xOffs;
	WIDGET(sa)->cy = WIDGET(sa)->cy + sa->xOffs;

	AG_WidgetPushClipRect(sa, AG_RECT(0, 0, WIDGET(sa)->w, WIDGET(sa)->h));
}

void
AG_ScrollableDrawEnd(AG_Scrollable *sa)
{
	AG_WidgetPopClipRect(sa);

	WIDGET(sa)->x = sa->save.x;
	WIDGET(sa)->y = sa->save.y;
	WIDGET(sa)->cx = sa->save.cx;
	WIDGET(sa)->cy = sa->save.cy;
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	r->w = 0;
	r->h = 0;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Scrollable *sa = p;
	AG_SizeAlloc aBar;

	if (a->w < sa->vbar->bw ||
	    a->h < sa->vbar->bw*2) {
		WIDGET(sa->vbar)->flags |= AG_WIDGET_HIDE;
	} else {
		WIDGET(sa->vbar)->flags &= ~(AG_WIDGET_HIDE);
		aBar.x = a->w - sa->vbar->bw;
		aBar.y = 0;
		aBar.w = sa->vbar->bw;
		aBar.h = a->h;
		AG_WidgetSizeAlloc(sa->vbar, &aBar);
	}
	
	if (a->w < sa->hbar->bw*2 ||
	    a->h < sa->hbar->bw) {
		WIDGET(sa->hbar)->flags |= AG_WIDGET_HIDE;
	} else {
		WIDGET(sa->hbar)->flags &= ~(AG_WIDGET_HIDE);
		aBar.x = 0;
		aBar.y = a->h - sa->hbar->bw;
		aBar.w = a->w;
		aBar.h = sa->hbar->bw;
		AG_WidgetSizeAlloc(sa->hbar, &aBar);
	}
	return (0);
}

#if 0
static void
mousemotion(AG_Event *event)
{
	AG_Scrollable *sa = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dx = AG_INT(3);
	int dy = AG_INT(4);
	int state = AG_INT(5);

	AG_PostEvent(NULL, sa->chld, "window-mousemotion",
	    "%i,%i,%i,%i,%i",
	    x - sa->xOffs,
	    y - sa->yOffs,
	    dx, dy, state);
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Scrollable *sa = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	
	AG_PostEvent(NULL, sa->chld, "window-mousebuttondown",
	    "%i,%i,%i",
	    button,
	    x - sa->xOffs,
	    y - sa->yOffs);
}

static void
mousebuttonup(AG_Event *event)
{
	AG_Scrollable *sa = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	AG_PostEvent(NULL, sa->chld, "window-mousebuttondown",
	    "%i,%i,%i",
	    button,
	    x - sa->xOffs,
	    y - sa->yOffs);
}

static void
forward_event(AG_Event *event)
{
	AG_Scrollable *sa = AG_SELF();

	AG_ForwardEvent(NULL, sa->chld, event);
}
#endif

static void
Init(void *obj)
{
	AG_Scrollable *sa = obj;

	sa->flags = 0;
	sa->vbar = AG_ScrollbarNew(sa, AG_SCROLLBAR_VERT, 0);
	sa->hbar = AG_ScrollbarNew(sa, AG_SCROLLBAR_HORIZ, 0);
	sa->xOffs = 0;
	sa->yOffs = 0;
	sa->w = 0;
	sa->h = 0;
	
#if 0
	/* Translate events involving coordinates */
	AG_SetEvent(sa, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(sa, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(sa, "window-mousemotion", mousemotion, NULL);

	/* Forward all other events as-is */
	AG_SetEvent(sa, "window-keyup", forward_event, NULL);
	AG_SetEvent(sa, "window-keydown", forward_event, NULL);
	AG_SetEvent(sa, "widget-shown", forward_event, NULL);
	AG_SetEvent(sa, "widget-hidden", forward_event, NULL);
	AG_SetEvent(sa, "widget-lostfocus", forward_event, NULL);
	AG_SetEvent(sa, "widget-gainfocus", forward_event, NULL);
	AG_SetEvent(sa, "widget-moved", forward_event, NULL);
	AG_SetEvent(sa, "widget-bound", forward_event, NULL);
#endif
}

AG_WidgetClass agScrollableClass = {
	{
		"AG_Widget:AG_Scrollable",
		sizeof(AG_Scrollable),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,				/* draw */
	SizeRequest,
	SizeAllocate
};

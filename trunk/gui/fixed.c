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

#include "fixed.h"

#include "window.h"
#include "primitive.h"

AG_Fixed *
AG_FixedNew(void *parent, Uint flags)
{
	AG_Fixed *fx;

	fx = Malloc(sizeof(AG_Fixed));
	AG_ObjectInit(fx, &agFixedClass);
	fx->flags |= flags;

	if (flags & AG_FIXED_HFILL) { AG_ExpandHoriz(fx); }
	if (flags & AG_FIXED_VFILL) { AG_ExpandVert(fx); }

	AG_ObjectAttach(parent, fx);
	return (fx);
}

static void
Init(void *obj)
{
	AG_Fixed *fx = obj;

	fx->flags = 0;
	fx->wPre = 0;
	fx->hPre = 0;
}

void
AG_FixedSizeHint(AG_Fixed *fx, int w, int h)
{
	fx->wPre = w;
	fx->hPre = h;
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Fixed *fx = p;

	r->w = fx->wPre;
	r->h = fx->hPre;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Fixed *fx = p;
	AG_Widget *chld;
	AG_SizeAlloc aChld;

	OBJECT_FOREACH_CHILD(chld, fx, ag_widget) {
		aChld.x = chld->x;
		aChld.y = chld->y;
		aChld.w = chld->w;
		aChld.h = chld->h;

		if (chld->flags & AG_WIDGET_HFILL) { aChld.w = a->w; }
		if (chld->flags & AG_WIDGET_VFILL) { aChld.h = a->h; }
		
		AG_WidgetSizeAlloc(chld, &aChld);
	}
	return (0);
}

static void
Draw(void *obj)
{
	AG_Fixed *fx = obj;

	if (fx->flags & AG_FIXED_BOX) {
		AG_DrawBox(fx,
		    AG_RECT(0, 0, WIDGET(fx)->w, WIDGET(fx)->h), -1,
		    AG_COLOR(FRAME_COLOR));
	} else if (fx->flags & AG_FIXED_INVBOX) {
		AG_DrawBox(fx,
		    AG_RECT(0, 0, WIDGET(fx)->w, WIDGET(fx)->h), -1,
		    AG_COLOR(FRAME_COLOR));
	} else if (fx->flags & AG_FIXED_FRAME) {
		AG_DrawFrame(fx,
		    AG_RECT(0, 0, WIDGET(fx)->w, WIDGET(fx)->h), -1,
		    AG_COLOR(FRAME_COLOR));
	} else if (fx->flags & AG_FIXED_FILLBG) {
		AG_DrawRectFilled(fx,
		    AG_RECT(0, 0, WIDGET(fx)->w, WIDGET(fx)->h),
		    AG_COLOR(FIXED_BG_COLOR));
	}
}

static __inline__ void
UpdateWindow(AG_Fixed *fx)
{
	AG_Window *pwin;

	if ((fx->flags & AG_FIXED_NO_UPDATE) == 0 &&
	    (pwin = AG_WidgetParentWindow(fx)) != NULL)
		AG_WindowUpdate(pwin);
}

void
AG_FixedPut(AG_Fixed *fx, void *p, int x, int y)
{
	AG_Widget *chld = p;
	AG_SizeReq r;
	AG_SizeAlloc a;

	AG_ObjectAttach(fx, chld);
	AG_WidgetSizeReq(chld, &r);
	a.w = r.w;
	a.h = r.h;
	a.x = x;
	a.y = y;
	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);
}

void
AG_FixedMove(AG_Fixed *fx, void *p, int x, int y)
{
	AG_Widget *chld = p;
	AG_SizeReq r;
	AG_SizeAlloc a;

	AG_WidgetSizeReq(chld, &r);
	a.w = r.w;
	a.h = r.h;
	a.x = (x == -1) ? chld->x : x;
	a.y = (y == -1) ? chld->y : y;
	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);
}

void
AG_FixedSize(AG_Fixed *fx, void *p, int w, int h)
{
	AG_Widget *chld = p;
	AG_SizeAlloc a;

	a.w = (w == -1) ? chld->w : w;
	a.h = (h == -1) ? chld->h : h;
	a.x = chld->x;
	a.y = chld->y;
	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);
}

void
AG_FixedDel(AG_Fixed *fx, void *child)
{
	AG_ObjectDetach(child);
	UpdateWindow(fx);
}

const AG_WidgetClass agFixedClass = {
	{
		"AG_Widget:AG_Fixed",
		sizeof(AG_Fixed),
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

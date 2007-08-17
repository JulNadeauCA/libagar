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
#include <core/view.h>

#include "fixed.h"

#include "window.h"
#include "primitive.h"

AG_Fixed *
AG_FixedNew(void *parent, Uint flags)
{
	AG_Fixed *bo;

	bo = Malloc(sizeof(AG_Fixed), M_OBJECT);
	AG_FixedInit(bo, flags);
	AG_ObjectAttach(parent, bo);
	return (bo);
}

void
AG_FixedInit(AG_Fixed *fx, Uint flags)
{
	AG_WidgetInit(fx, &agFixedOps, 0);

	fx->flags = flags;
	fx->wPre = 0;
	fx->hPre = 0;

	if (flags & AG_FIXED_HFILL) { WIDGET(fx)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_FIXED_VFILL) { WIDGET(fx)->flags |= AG_WIDGET_VFILL; }

	if (flags & AG_FIXED_FILLBG)
		WIDGET_OPS(fx)->draw = AG_FixedDrawBg;
	if (flags & AG_FIXED_BOX)
		WIDGET_OPS(fx)->draw = AG_FixedDrawBox;
	if (flags & AG_FIXED_INVBOX)
		WIDGET_OPS(fx)->draw = AG_FixedDrawInvBox;
	if (flags & AG_FIXED_FRAME)
		WIDGET_OPS(fx)->draw = AG_FixedDrawFrame;
}

void
AG_FixedPrescale(AG_Fixed *fx, int w, int h)
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

void
AG_FixedDrawBg(void *p)
{
	AG_Widget *w = p;

	agPrim.rect_filled(w, 0, 0, w->w, w->h, AG_COLOR(FIXED_BG_COLOR));
}

void
AG_FixedDrawBox(void *p)
{
	AG_Widget *w = p;
	
	agPrim.box(w, 0, 0, w->w, w->h, -1, AG_COLOR(FRAME_COLOR));
}

void
AG_FixedDrawInvBox(void *p)
{
	AG_Widget *w = p;
	
	agPrim.box(w, 0, 0, w->w, w->h, -1, AG_COLOR(FRAME_COLOR));
}

void
AG_FixedDrawFrame(void *p)
{
	AG_Widget *w = p;

	agPrim.frame(w, 0, 0, w->w, w->h, -1, AG_COLOR(FRAME_COLOR));
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
	AG_Window *pwin;
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
	AG_SizeAlloc a;

	a.w = chld->w;
	a.h = chld->h;
	a.x = x;
	a.y = y;
	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);
}

void
AG_FixedSize(AG_Fixed *fx, void *p, int w, int h)
{
	AG_Widget *chld = p;
	AG_SizeAlloc a;

	a.w = w;
	a.h = h;
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

const AG_WidgetOps agFixedOps = {
	{
		"AG_Widget:AG_Fixed",
		sizeof(AG_Fixed),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,		/* draw */
	SizeRequest,
	SizeAllocate
};

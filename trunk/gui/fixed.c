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

#include "fixed.h"

#include "window.h"
#include "primitive.h"

static AG_WidgetOps agFixedOps = {
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
	AG_FixedScale
};

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
	AG_WidgetInit(fx, "fixed", &agFixedOps, 0);

	fx->flags = flags;
	if (flags & AG_FIXED_HFILL) { AGWIDGET(fx)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_FIXED_VFILL) { AGWIDGET(fx)->flags |= AG_WIDGET_VFILL; }

	if (flags & AG_FIXED_FILLBG)
		AGWIDGET_OPS(fx)->draw = AG_FixedDrawBg;
	if (flags & AG_FIXED_BOX)
		AGWIDGET_OPS(fx)->draw = AG_FixedDrawBox;
	if (flags & AG_FIXED_INVBOX)
		AGWIDGET_OPS(fx)->draw = AG_FixedDrawInvBox;
	if (flags & AG_FIXED_FRAME)
		AGWIDGET_OPS(fx)->draw = AG_FixedDrawFrame;
}

void
AG_FixedScale(void *p, int w, int h)
{
	AG_Fixed *fx = p;
	AG_Widget *cw;

	AGOBJECT_FOREACH_CHILD(cw, fx, ag_widget) {
		if (w != -1 && h != -1) {
			if (cw->flags & AG_WIDGET_HFILL) { cw->w = w; }
			if (cw->flags & AG_WIDGET_VFILL) { cw->h = h; }
		}
		AGWIDGET_OPS(cw)->scale(cw, cw->w, cw->h);
	}
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

	agPrim.frame(w, 0, 0, w->w, w->h, AG_COLOR(FRAME_COLOR));
}

static __inline__ void
AG_FixedUpdate(AG_Fixed *fx)
{
	AG_Window *pwin;

	if ((fx->flags & AG_FIXED_NO_UPDATE) == 0 &&
	    (pwin = AG_WidgetParentWindow(fx)) != NULL)
		AG_WINDOW_UPDATE(pwin);
}

void
AG_FixedPut(AG_Fixed *fx, void *child, int x, int y)
{
	AG_Widget *cw = child;
	AG_Window *pwin;

	AG_ObjectAttach(fx, cw);
	cw->x = x;
	cw->y = y;

	AGWIDGET_OPS(cw)->scale(cw, -1, -1);
	AG_FixedUpdate(fx);
}

void
AG_FixedMove(AG_Fixed *fx, void *child, int x, int y)
{
	AG_Widget *cw = child;

	cw->x = x;
	cw->y = y;
	AG_FixedUpdate(fx);
}

void
AG_FixedSize(AG_Fixed *fx, void *child, int w, int h)
{
	AG_Widget *cw = child;

	cw->w = w;
	cw->h = h;

	AGWIDGET_OPS(cw)->scale(cw, w, h);
	AG_FixedUpdate(fx);
}

void
AG_FixedDel(AG_Fixed *fx, void *child)
{
	AG_ObjectDetach(child);
	AG_FixedUpdate(fx);
}

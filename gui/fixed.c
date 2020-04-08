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
 * Fixed-position container widget. It allows widgets to be sized and
 * placed at specific positions in pixels.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/fixed.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

AG_Fixed *
AG_FixedNew(void *parent, Uint flags)
{
	AG_Fixed *fx;

	fx = Malloc(sizeof(AG_Fixed));
	AG_ObjectInit(fx, &agFixedClass);

	if (flags & AG_FIXED_HFILL) { WIDGET(fx)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_FIXED_VFILL) { WIDGET(fx)->flags |= AG_WIDGET_VFILL; }
	fx->flags |= flags;

#ifdef AG_LEGACY
	if (flags & AG_FIXED_BOX) { fx->style = AG_FIXED_STYLE_BOX; }
	else if (flags & (AG_FIXED_INVBOX | AG_FIXED_FRAME)) { fx->style = AG_FIXED_STYLE_WELL; }
	else if (flags & AG_FIXED_FILLBG) { fx->style = AG_FIXED_STYLE_PLAIN; }
#endif

	AG_ObjectAttach(parent, fx);
	return (fx);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Fixed *fx = AG_FIXED_SELF();
	AG_Window *wParent = AG_ParentWindow(fx);

	if (!AG_WindowIsFocused(wParent))
		AG_WindowFocus(wParent);
}

static void
Init(void *_Nonnull obj)
{
	AG_Fixed *fx = obj;

	fx->flags = 0;
	fx->style = AG_FIXED_STYLE_WELL;		/* 3D well */
	fx->wPre = 0;
	fx->hPre = 0;

	AG_SetEvent(fx, "mouse-button-down", MouseButtonDown, NULL);
}

void
AG_FixedSetStyle(AG_Fixed *fx, enum ag_fixed_style style)
{
	AG_OBJECT_ISA(fx, "AG_Widget:AG_Fixed:*");
	fx->style = style;
	AG_Redraw(fx);
}

void
AG_FixedSizeHint(AG_Fixed *fx, int w, int h)
{
	AG_OBJECT_ISA(fx, "AG_Widget:AG_Fixed:*");
	fx->wPre = w;
	fx->hPre = h;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Fixed *fx = obj;

	r->w = fx->wPre;
	r->h = fx->hPre;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Fixed *fx = obj;
	AG_Widget *chld;
	AG_SizeAlloc aChld;

	/*
	 * Trivially run sizeAllocate() over existing coordinates.
	 * Ignore HFILL and VFILL.
	 */
	OBJECT_FOREACH_CHILD(chld, fx, ag_widget) {
		aChld.x = chld->x;
		aChld.y = chld->y;
		aChld.w = chld->w;
		aChld.h = chld->h;
		AG_WidgetSizeAlloc(chld, &aChld);
	}
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	static void (*pfBox[])(void *, const AG_Rect *, const AG_Color *) = {
		ag_draw_rect_noop,  /* NONE */
		ag_draw_box_raised, /* BOX */
		ag_draw_box_sunk,   /* WELL */
		ag_draw_rect        /* PLAIN */
	};
	AG_Fixed *fx = obj;
	AG_Widget *chld;

#ifdef AG_DEBUG
	if (fx->style >= AG_FIXED_STYLE_LAST)
		AG_FatalError("style");
#endif
	pfBox[fx->style](fx, &WIDGET(fx)->r, &WCOLOR(fx,BG_COLOR));

	OBJECT_FOREACH_CHILD(chld, fx, ag_widget)
		AG_WidgetDraw(chld);
}

static __inline__ void
UpdateWindow(AG_Fixed *_Nonnull fx)
{
	if (!(fx->flags & AG_FIXED_NO_UPDATE))
		WIDGET(fx)->flags |= AG_WIDGET_UPDATE_WINDOW;
}

/*
 * Attach a new widget to the container and set initial coordinates to x,y
 * in pixels. Auto-size according to sizeRequest().
 */
void
AG_FixedPut(AG_Fixed *fx, void *p, int x, int y)
{
	AG_Widget *chld = p;
	AG_SizeReq r;
	AG_SizeAlloc a;

	AG_OBJECT_ISA(fx, "AG_Widget:AG_Fixed:*");
	AG_ObjectLock(fx);
	AG_OBJECT_ISA(chld, "AG_Widget:*");
	AG_ObjectLock(chld);
	
	AG_ObjectAttach(fx, chld);
	AG_WidgetSizeReq(chld, &r);
	a.w = r.w;
	a.h = r.h;
	a.x = x;
	a.y = y;
	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);
	
	AG_ObjectUnlock(chld);
	AG_Redraw(fx);
	AG_ObjectUnlock(fx);
}

/* Move an existing widget to coordinates x,y. */
void
AG_FixedMove(AG_Fixed *fx, void *p, int x, int y)
{
	AG_Widget *chld = p;
	AG_SizeReq r;
	AG_SizeAlloc a;
	
	AG_OBJECT_ISA(fx, "AG_Widget:AG_Fixed:*");
	AG_ObjectLock(fx);
	AG_OBJECT_ISA(chld, "AG_Widget:*");
	AG_ObjectLock(chld);

	AG_WidgetSizeReq(chld, &r);
	a.w = r.w;
	a.h = r.h;
	a.x = (x == -1) ? chld->x : x;
	a.y = (y == -1) ? chld->y : y;
	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);
	
	AG_ObjectUnlock(chld);
	AG_Redraw(fx);
	AG_ObjectUnlock(fx);
}

/* Resize a widget to w x h pixels. */
void
AG_FixedSize(AG_Fixed *fx, void *p, int w, int h)
{
	AG_Widget *chld = p;
	AG_SizeAlloc a;
	
	AG_OBJECT_ISA(fx, "AG_Widget:AG_Fixed:*");
	AG_ObjectLock(fx);
	AG_OBJECT_ISA(chld, "AG_Widget:*");
	AG_ObjectLock(chld);

	a.w = (w == -1) ? chld->w : w;
	a.h = (h == -1) ? chld->h : h;
	a.x = chld->x;
	a.y = chld->y;

	AG_WidgetSizeAlloc(chld, &a);
	UpdateWindow(fx);

	AG_ObjectUnlock(chld);
	AG_Redraw(fx);
	AG_ObjectUnlock(fx);
}

/* Detach a widget from the container. */
void
AG_FixedDel(AG_Fixed *fx, void *chld)
{
	AG_OBJECT_ISA(fx, "AG_Widget:AG_Fixed:*");
	AG_ObjectLock(fx);
	AG_OBJECT_ISA(chld, "AG_Widget:*");
	AG_ObjectLock(chld);
	
	AG_ObjectDetach(chld);
	UpdateWindow(fx);
	
	AG_ObjectUnlock(chld);
	AG_Redraw(fx);
	AG_ObjectUnlock(fx);
}

AG_WidgetClass agFixedClass = {
	{
		"Agar(Widget:Fixed)",
		sizeof(AG_Fixed),
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

/*
 * Copyright (c) 2003-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "box.h"

#include "window.h"
#include "primitive.h"
#include "text.h"

#ifdef AG_DEBUG
#include "numerical.h"
#include "checkbox.h"
#endif

AG_Box *
AG_BoxNew(void *parent, enum ag_box_type type, Uint flags)
{
	AG_Box *box;

	box = Malloc(sizeof(AG_Box));
	AG_ObjectInit(box, &agBoxClass);

	box->type = type;
	box->flags |= flags;

	if (flags & AG_BOX_HFILL) { AG_ExpandHoriz(box); }
	if (flags & AG_BOX_VFILL) { AG_ExpandVert(box); }

	AG_ObjectAttach(parent, box);
	return (box);
}

/* Set the label text (format string). */
void
AG_BoxSetLabel(AG_Box *box, const char *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_BoxSetLabelS(box, s);
}

/* Set the label text (format string). */
void
AG_BoxSetLabelS(AG_Box *box, const char *s)
{
	char *captionNew;

	if ((captionNew = TryStrdup(s)) == NULL) {
		return;
	}
	AG_ObjectLock(box);
	Free(box->caption);
	box->caption = captionNew;
	if (box->sCaption != -1) {
		AG_WidgetUnmapSurface(box, box->sCaption);
		box->sCaption = -1;
	}
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

static void
Init(void *obj)
{
	AG_Box *box = obj;

	box->flags = 0;
	box->type = AG_BOX_VERT;
	box->depth = -1;
	box->padding = 4;
	box->spacing = 2;
	box->caption = NULL;
	box->sCaption = -1;
	box->rCaption = AG_RECT(5, agTextFontHeight/2, 0, 0);
#ifdef AG_DEBUG
	AG_BindUint(box, "flags", &box->flags);
	AG_BindInt(box, "padding", &box->padding);
	AG_BindInt(box, "spacing", &box->spacing);
	AG_BindInt(box, "depth", &box->depth);
#endif
}

static void
Destroy(void *obj)
{
	AG_Box *box = obj;

	Free(box->caption);
}

static void
DrawCaption(AG_Box *box)
{
	int y = agTextFontHeight/2;

	if (box->sCaption == -1)  {
		AG_Surface *sText;

		sText = AG_TextRender(box->caption);
		box->sCaption = AG_WidgetMapSurface(box, sText);
	}
	STYLE(box)->BoxFrame(box,
	    AG_RECT(0, y, WIDTH(box), HEIGHT(box)-y),
	    box->depth);
	AG_WidgetBlitSurface(box, box->sCaption,
	    box->rCaption.x,
	    box->rCaption.y);
}

static void
Draw(void *obj)
{
	AG_Box *box = obj;
	AG_Widget *chld;

	if (box->caption != NULL) {
		DrawCaption(box);
	} else {
		if (box->flags & AG_BOX_FRAME) {
			STYLE(box)->BoxFrame(box,
			    AG_RECT(0, 0, WIDTH(box), HEIGHT(box)),
			    box->depth);
		}
	}
	
	WIDGET_FOREACH_CHILD(chld, box)
		AG_WidgetDraw(chld);
}

static int
CountChildWidgets(AG_Box *box, int *totFixed)
{
	AG_Widget *chld;
	AG_SizeReq r;
	Uint count = 0;
	int fixed = 0;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &r);
		switch (box->type) {
		case AG_BOX_HORIZ:
			if ((chld->flags & AG_WIDGET_HFILL) == 0)
				fixed += r.w;
			break;
		case AG_BOX_VERT:
			if ((chld->flags & AG_WIDGET_VFILL) == 0)
				fixed += r.h;
			break;
		}
		count++;
		fixed += box->spacing;
	}
	if (count > 0 && fixed >= box->spacing) {
		fixed -= box->spacing;
	}
	*totFixed = fixed;
	return (count);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Box *box = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	int wMax = 0, hMax = 0;
	int nWidgets, totArea;

	nWidgets = CountChildWidgets(box, &totArea);
	r->w = box->padding*2;
	r->h = box->padding*2;
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &rChld);
		if (rChld.w > wMax) { wMax = rChld.w; }
		if (rChld.h > hMax) { hMax = rChld.h; }
		switch (box->type) {
		case AG_BOX_HORIZ:
			r->h = MAX(r->h, hMax + box->padding*2);
			r->w += rChld.w + box->spacing;
			break;
		case AG_BOX_VERT:
			r->w = MAX(r->w, wMax + box->padding*2);
			r->h += rChld.h + box->spacing;
			break;
		}
	}
	if (nWidgets > 0) {
		switch (box->type) {
		case AG_BOX_HORIZ:
			if (r->w >= box->spacing)
				r->w -= box->spacing;
			break;
		case AG_BOX_VERT:
			if (r->h >= box->spacing)
				r->h -= box->spacing;
			break;
		}
	}
}

static int
SizeAllocateHomogenous(AG_Box *box, const AG_SizeAlloc *a, int nWidgets)
{
	int wSize, totUsed = 0, avail;
	AG_Widget *chld, *chldLast = NULL;
	AG_SizeAlloc aChld;

	avail = ((box->type == AG_BOX_HORIZ) ? a->w : a->h);
	avail -= box->padding*2;
	wSize = (avail - nWidgets - 1) / nWidgets;

	aChld.x = box->padding;
	aChld.y = box->padding;
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		aChld.w = (box->type==AG_BOX_HORIZ) ?
		          wSize : (a->w - box->padding*2);
		aChld.h = (box->type==AG_BOX_VERT) ?
		          wSize : (a->h - box->padding*2);

		if (AG_WidgetSizeAlloc(chld, &aChld) == -1)
			continue;

		if (OBJECT(chld) ==
		    TAILQ_LAST(&OBJECT(box)->children,ag_objectq)) {
			chldLast = chld;
		} else {
			if (box->type == AG_BOX_HORIZ) {
				aChld.x += aChld.w + 1;
			} else {
				aChld.y += aChld.h + 1;
			}
		}
		totUsed += ((box->type == AG_BOX_HORIZ) ? aChld.w : aChld.h)+1;
	}
	/* Compensate for rounding error due to division. */
	if (chldLast != NULL && totUsed < avail) {
		switch (box->type) {
		case AG_BOX_VERT:
			aChld.h += avail - totUsed;
			AG_WidgetSizeAlloc(chldLast, &aChld);
			break;
		case AG_BOX_HORIZ:
			aChld.w += avail - totUsed;
			AG_WidgetSizeAlloc(chldLast, &aChld);
			break;
		}
	}
	return (0);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Box *box = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	int nWidgets, totFixed;
	int wAvail, hAvail;
	int hLabel = (box->caption != NULL) ? agTextFontHeight/2 : 0;

	box->rCaption.w = a->w;
	box->rCaption.h = a->h - box->rCaption.y;

	if ((nWidgets = CountChildWidgets(box, &totFixed)) == 0) {
		return (0);
	}
	if (box->flags & AG_BOX_HOMOGENOUS) {
		if (SizeAllocateHomogenous(box, a, nWidgets) == -1) {
			return (-1);
		}
		return (0);
	}
	wAvail = a->w - box->padding*2;
	hAvail = a->h - box->padding*2 - hLabel;
	aChld.x = box->padding;
	aChld.y = box->padding + hLabel;
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &rChld);
		switch (box->type) {
		case AG_BOX_HORIZ:
			aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
			          (wAvail - totFixed) : MIN(rChld.w, wAvail);
			aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
				  hAvail : MIN(hAvail, rChld.h);
			AG_WidgetSizeAlloc(chld, &aChld);
			aChld.x += aChld.w + box->spacing;
			if (aChld.x > a->w+box->padding*2) {
				chld->flags |= AG_WIDGET_UNDERSIZE;
			} else {
				chld->flags &= ~(AG_WIDGET_UNDERSIZE);
			}
			break;
		case AG_BOX_VERT:
			aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
			          wAvail : MIN(wAvail, rChld.w);
			aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
			          (hAvail - totFixed) : MIN(rChld.h, hAvail);
			AG_WidgetSizeAlloc(chld, &aChld);
			aChld.y += aChld.h + box->spacing;
			if (aChld.y > a->h+box->padding*2) {
				chld->flags |= AG_WIDGET_UNDERSIZE;
			} else {
				chld->flags &= ~(AG_WIDGET_UNDERSIZE);
			}
			break;
		}
	}
	return (0);
}

void
AG_BoxSetHomogenous(AG_Box *box, int enable)
{
	AG_ObjectLock(box);
	AG_SETFLAGS(box->flags, AG_BOX_HOMOGENOUS, enable);
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

void
AG_BoxSetPadding(AG_Box *box, int padding)
{
	AG_ObjectLock(box);
	box->padding = padding;
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

void
AG_BoxSetSpacing(AG_Box *box, int spacing)
{
	AG_ObjectLock(box);
	box->spacing = spacing;
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

void
AG_BoxSetDepth(AG_Box *box, int depth)
{
	AG_ObjectLock(box);
	box->depth = depth;
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

void
AG_BoxSetType(AG_Box *box, enum ag_box_type type)
{
	AG_SizeAlloc a;

	AG_ObjectLock(box);
	box->type = type;
	a.x = WIDGET(box)->x;
	a.y = WIDGET(box)->y;
	a.w = WIDGET(box)->w;
	a.h = WIDGET(box)->h;
	SizeAllocate(box, &a);
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

#ifdef AG_DEBUG
static void
UpdateWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	AG_WindowUpdate(win);
}

static void *
Edit(void *obj)
{
	AG_Box *ctr = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	AG_Box *box = obj;
	AG_Numerical *num;
		
	num = AG_NumericalNewIntR(ctr, 0, "px", _("Padding: "),
	    &box->padding, 0, 255);
	AG_SetEvent(num, "numerical-changed",
	    UpdateWindow, "%p", AG_ParentWindow(box));
		
	num = AG_NumericalNewIntR(ctr, 0, "px", _("Spacing: "),
	    &box->spacing, 0, 255);
	AG_SetEvent(num, "numerical-changed",
	    UpdateWindow, "%p", AG_ParentWindow(box));
		
	AG_CheckboxNewFlag(ctr, 0, _("Homogenous"),
	    &box->flags, AG_BOX_HOMOGENOUS);
	AG_CheckboxNewFlag(ctr, 0, _("Visual frame"),
	    &box->flags, AG_BOX_FRAME);

	return (ctr);
}
#endif /* AG_DEBUG */

AG_WidgetClass agBoxClass = {
	{
		"Agar(Widget:Box)",
		sizeof(AG_Box),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
#ifdef AG_DEBUG
		Edit
#else
		NULL		/* edit */
#endif
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

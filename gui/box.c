/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

static void
Init(void *obj)
{
	AG_Box *box = obj;

	box->flags = 0;
	box->type = AG_BOX_VERT;
	box->depth = -1;
	box->padding = 4;
	box->spacing = 2;
}

void
AG_BoxDraw(void *obj)
{
	AG_Box *box = obj;

	if (box->flags & AG_BOX_FRAME)
		STYLE(box)->BoxFrame(box, box->depth);
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

void
AG_BoxSizeRequest(void *obj, AG_SizeReq *r)
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
			return (-1);

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

int
AG_BoxSizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Box *box = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	int nWidgets, totFixed;
	int wAvail, hAvail;

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
	hAvail = a->h - box->padding*2;
	aChld.x = box->padding;
	aChld.y = box->padding;
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
	if (enable) {
		box->flags |= AG_BOX_HOMOGENOUS;
	} else {
		box->flags &= ~(AG_BOX_HOMOGENOUS);
	}
	AG_ObjectUnlock(box);
}

void
AG_BoxSetPadding(AG_Box *box, int padding)
{
	AG_ObjectLock(box);
	box->padding = padding;
	AG_ObjectUnlock(box);
}

void
AG_BoxSetSpacing(AG_Box *box, int spacing)
{
	AG_ObjectLock(box);
	box->spacing = spacing;
	AG_ObjectUnlock(box);
}

void
AG_BoxSetDepth(AG_Box *box, int depth)
{
	AG_ObjectLock(box);
	box->depth = depth;
	AG_ObjectUnlock(box);
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
	AG_BoxSizeAllocate(box, &a);
	AG_ObjectUnlock(box);
}

AG_WidgetClass agBoxClass = {
	{
		"Agar(Widget:Box)",
		sizeof(AG_Box),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_BoxDraw,
	AG_BoxSizeRequest,
	AG_BoxSizeAllocate
};

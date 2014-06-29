/*
 * Copyright (c) 2003-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/box.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/label.h>
#ifdef AG_DEBUG
#include <agar/gui/numerical.h>
#include <agar/gui/checkbox.h>
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

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
		AG_BoxSetLabelS(box, s);
		free(s);
		box->flags |= AG_BOX_FRAME;
	} else {
		AG_BoxSetLabelS(box, NULL);
		box->flags &= ~(AG_BOX_FRAME);
	}
}

/* Set the label text (format string). */
void
AG_BoxSetLabelS(AG_Box *box, const char *s)
{
	AG_ObjectLock(box);
	if (s != NULL) {
		if (box->lbl == NULL) {
			box->lbl = AG_LabelNewS(box, 0, s);
			AG_SetStyle(box->lbl, "font-size", "80%");
		} else {
			AG_LabelTextS(box->lbl, s);
		}
	} else {
		AG_ObjectDetach(box->lbl);
		AG_ObjectDestroy(box->lbl);
		box->lbl = NULL;
	}
	AG_Redraw(box);
	AG_ObjectUnlock(box);
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
	box->lbl = NULL;
	box->hAlign = AG_BOX_LEFT;
	box->vAlign = AG_BOX_TOP;
}

static void
Draw(void *obj)
{
	AG_Box *box = obj;
	AG_Widget *chld;

	if (box->flags & AG_BOX_FRAME) {
		AG_DrawBox(box,
		    AG_RECT(0, 0, WIDTH(box), HEIGHT(box)),
		    box->depth, WCOLOR(box,AG_COLOR));
	}
	OBJECT_FOREACH_CHILD(chld, box, ag_widget)
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

		AG_WidgetSizeAlloc(chld, &aChld);
		if (chld->flags & AG_WIDGET_UNDERSIZE)
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
	int x, y;

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
	x = box->padding;
	y = box->padding;
	if (totFixed < wAvail) {
		switch (box->hAlign) {
		case AG_BOX_CENTER:	x = wAvail/2 - totFixed/2;	break;
		case AG_BOX_RIGHT:	x = wAvail - totFixed;		break;
		}
		switch (box->vAlign) {
		case AG_BOX_CENTER:	y = hAvail/2 - totFixed/2;	break;
		case AG_BOX_BOTTOM:	y = hAvail - totFixed;		break;
		}
	}
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &rChld);
		switch (box->type) {
		case AG_BOX_HORIZ:
			aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
			          (wAvail - totFixed) : MIN(rChld.w, wAvail);
			aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
				  hAvail : MIN(hAvail, rChld.h);
			aChld.x = x;
			if (aChld.x+aChld.w > a->w) {
				aChld.w = a->w - aChld.x;
			}
			switch (box->vAlign) {
			case AG_BOX_TOP:	aChld.y = y;			break;
			case AG_BOX_CENTER:	aChld.y = hAvail/2 - aChld.h/2;	break;
			case AG_BOX_BOTTOM:	aChld.y = hAvail - aChld.h;	break;
			}
			AG_WidgetSizeAlloc(chld, &aChld);
			x += aChld.w + box->spacing;
			break;
		case AG_BOX_VERT:
			aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
			          wAvail : MIN(wAvail, rChld.w);
			aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
			          (hAvail - totFixed) : MIN(rChld.h, hAvail);
			aChld.y = y;
			if (aChld.y+aChld.h > a->h) {
				aChld.h = a->h - aChld.y;
			}
			switch (box->hAlign) {
			case AG_BOX_TOP:	aChld.x = x;			break;
			case AG_BOX_CENTER:	aChld.x = wAvail/2 - aChld.w/2;	break;
			case AG_BOX_BOTTOM:	aChld.x = wAvail - aChld.w;	break;
			}
			AG_WidgetSizeAlloc(chld, &aChld);
			y += aChld.h + box->spacing;
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

void
AG_BoxSetHorizAlign(AG_Box *box, enum ag_box_align align)
{
	AG_ObjectLock(box);
	box->hAlign = align;
	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

void
AG_BoxSetVertAlign(AG_Box *box, enum ag_box_align align)
{
	AG_ObjectLock(box);
	box->vAlign = align;
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
		NULL,		/* destroy */
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

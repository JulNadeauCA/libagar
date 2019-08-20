/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#ifdef AG_WIDGETS

#include <agar/gui/box.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/label.h>
#ifdef AG_DEBUG
#include <agar/gui/numerical.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/radio.h>
#include <agar/gui/separator.h>
#endif

const char *agBoxHorizAlignNames[] = {
	N_("Left"),
	N_("Center"),
	N_("Right"),
	NULL
};
const char *agBoxVertAlignNames[] = {
	_("Top"),
	_("Middle"),
	_("Bottom"),
	NULL
};

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

AG_Box *
AG_BoxNewHoriz(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_HORIZ, flags);
}

AG_HBox *
AG_HBoxNew(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_HORIZ, flags);
}

AG_Box *
AG_BoxNewHorizNS(void *p, Uint flags)
{
	AG_Box *hBox;
	
	hBox = AG_BoxNew(p, AG_BOX_HORIZ, flags);
	AG_BoxSetSpacing(hBox, 0);
	AG_BoxSetPadding(hBox, 0);
	return (hBox);
}

AG_HBox *
AG_HBoxNewNS(void *p, Uint flags)
{
	AG_Box *hBox;
	
	hBox = AG_BoxNew(p, AG_BOX_HORIZ, flags);
	AG_BoxSetSpacing(hBox, 0);
	AG_BoxSetPadding(hBox, 0);
	return (hBox);
}

AG_Box *
AG_BoxNewVert(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_VERT, flags);
}

AG_VBox *
AG_VBoxNew(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_VERT, flags);
}

AG_Box *
AG_BoxNewVertNS(void *p, Uint flags)
{
	AG_Box *vBox;
	
	vBox = AG_BoxNew(p, AG_BOX_VERT, flags);
	AG_BoxSetSpacing(vBox, 0);
	AG_BoxSetPadding(vBox, 0);
	return (vBox);
}

AG_VBox *
AG_VBoxNewNS(void *p, Uint flags)
{
	AG_Box *vBox;
	
	vBox = AG_BoxNew(p, AG_BOX_VERT, flags);
	AG_BoxSetSpacing(vBox, 0);
	AG_BoxSetPadding(vBox, 0);
	return (vBox);
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
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Box *box = AG_BOX_SELF();
	AG_Window *wParent = AG_ParentWindow(box);

	if (!AG_WindowIsFocused(wParent))
		AG_WindowFocus(wParent);
}

static void
Init(void *_Nonnull obj)
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

	AG_SetEvent(box, "mouse-button-down", MouseButtonDown, NULL);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Box *box = obj;
	AG_Widget *chld;

	if (box->flags & AG_BOX_FRAME) {
		AG_Rect r;

		r.x = 0;
		r.y = 0;
		r.w = WIDTH(box);
		r.h = HEIGHT(box);
		AG_DrawBox(box, &r, box->depth, &WCOLOR(box,AG_BG_COLOR));
	}
	OBJECT_FOREACH_CHILD(chld, box, ag_widget)
		AG_WidgetDraw(chld);
}

static int
CountChildWidgets(AG_Box *_Nonnull box, int *_Nonnull totFixed)
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
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Box *box = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	int wMax = 0, hMax = 0;
	int nWidgets, totArea;
	const int padding2 = box->padding << 1;
	
	r->w = padding2;
	r->h = padding2;

	nWidgets = CountChildWidgets(box, &totArea);
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &rChld);
		if (rChld.w > wMax) { wMax = rChld.w; }
		if (rChld.h > hMax) { hMax = rChld.h; }
		switch (box->type) {
		case AG_BOX_HORIZ:
			r->h = MAX(r->h, hMax + padding2);
			r->w += rChld.w + box->spacing;
			break;
		case AG_BOX_VERT:
			r->w = MAX(r->w, wMax + padding2);
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
SizeAllocateHomogenous(AG_Box *_Nonnull box, const AG_SizeAlloc *_Nonnull a,
    int nWidgets)
{
	int wSize, totUsed = 0, avail;
	AG_Widget *chld, *chldLast = NULL;
	AG_SizeAlloc aChld;
	const int padding2 = box->padding << 1;

	avail = ((box->type == AG_BOX_HORIZ) ? a->w : a->h);
	avail -= padding2;
	wSize = (avail - nWidgets - 1) / nWidgets;

	aChld.x = box->padding;
	aChld.y = box->padding;
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		aChld.w = (box->type==AG_BOX_HORIZ) ? wSize : (a->w - padding2);
		aChld.h = (box->type==AG_BOX_VERT)  ? wSize : (a->h - padding2);

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
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Box *box = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	const int padding = box->padding, padding2 = padding << 1;
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
	wAvail = a->w - padding2;
	hAvail = a->h - padding2;
	x = padding;
	y = padding;
	if (totFixed < wAvail) {
		switch (box->hAlign) {
		case AG_BOX_CENTER: x = (wAvail>>1) - (totFixed>>1);  break;
		case AG_BOX_RIGHT:  x =  wAvail     -  totFixed;      break;
		default:                                              break;
		}
		switch (box->vAlign) {
		case AG_BOX_CENTER: y = (hAvail>>1) - (totFixed>>1);  break;
		case AG_BOX_BOTTOM: y =  hAvail     -  totFixed;      break;
		default:                                              break;
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
			case AG_BOX_TOP:    aChld.y = y;                          break;
			case AG_BOX_CENTER: aChld.y = (hAvail>>1) - (aChld.h>>1); break;
			case AG_BOX_BOTTOM: aChld.y =  hAvail     -  aChld.h;     break;
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
			case AG_BOX_TOP:    aChld.x = x;                          break;
			case AG_BOX_CENTER: aChld.x = (wAvail>>1) - (aChld.w>>1); break;
			case AG_BOX_BOTTOM: aChld.x =  wAvail     -  aChld.w;     break;
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
UpdateWindowOf(AG_Event *_Nonnull event)
{
	AG_Box *box = AG_BOX_PTR(1);
	AG_Window *win = AG_ParentWindow(box);

	AG_Redraw(box);
	AG_WindowUpdate(win);
}

static void *
Edit(void *_Nonnull obj)
{
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND), *hBox;
	AG_Box *tgt = obj;
	AG_Numerical *num;
	AG_Checkbox *cb;
	AG_Radio *rad;

	AG_LabelNewS(box, 0, _("Alignment:"));
	hBox = AG_BoxNewHoriz(box, 0);
	{
		rad = AG_RadioNewUint(hBox, 0, agBoxHorizAlignNames, &tgt->hAlign);
		AG_SetEvent(rad, "radio-changed", UpdateWindowOf,"%p",tgt);
	
		rad = AG_RadioNewUint(hBox, 0, agBoxVertAlignNames, &tgt->vAlign);
		AG_SetEvent(rad, "radio-changed", UpdateWindowOf,"%p",tgt);
	}

	AG_SpacerNewHoriz(box);

	num = AG_NumericalNewIntR(box, 0, NULL, _("Padding: "), &tgt->padding, 0, 255);
	AG_SetEvent(num, "numerical-changed", UpdateWindowOf,"%p",tgt);

	num = AG_NumericalNewIntR(box, 0, NULL, _("Spacing: "), &tgt->spacing, 0, 255);
	AG_SetEvent(num, "numerical-changed", UpdateWindowOf,"%p",tgt);

	num = AG_NumericalNewIntR(box, 0, NULL, _("Depth: "), &tgt->depth, -127, 127);
	AG_SetEvent(num, "numerical-changed", UpdateWindowOf,"%p",tgt);
	
	AG_SpacerNewHoriz(box);

	cb = AG_CheckboxNewFlag(box, 0, _("Homogenous"), &tgt->flags, AG_BOX_HOMOGENOUS);
	AG_SetEvent(cb, "checkbox-changed", UpdateWindowOf,"%p",tgt);

	cb = AG_CheckboxNewFlag(box, 0, _("Visual frame"), &tgt->flags, AG_BOX_FRAME);
	AG_SetEvent(cb, "checkbox-changed", UpdateWindowOf,"%p",tgt);

	return (box);
}
#endif /* AG_DEBUG */

AG_WidgetClass agBoxClass = {
	{
		"Agar(Widget:Box)",
		sizeof(AG_Box),
		{ 0,0 },
		Init,
		NULL,		/* reset */
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

#endif /* AG_WIDGETS */

/*
 * Copyright (c) 2003-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * General-purpose container widget which aligns and packs its widgets
 * horizontally or vertically based on their size requisitions.
 *
 * Spacing and padding is added according to style attributes. Widgets
 * with the HFILL / VFILL bits set are expanded to fill remaining space.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/box.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/label.h>
#include <agar/gui/numerical.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/radio.h>
#include <agar/gui/separator.h>

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

static Uint SizeRequestHoriz(AG_Box *_Nonnull, int *_Nonnull);
static Uint SizeRequestVert(AG_Box *_Nonnull, int *_Nonnull);
static void SizeAllocateHoriz(AG_Box *_Nonnull, const AG_SizeAlloc *_Nonnull, int);
static void SizeAllocateVert(AG_Box *_Nonnull, const AG_SizeAlloc *_Nonnull, int);
static void SizeAllocateHomogenousHoriz(AG_Box *_Nonnull, const AG_SizeAlloc *_Nonnull, int);
static void SizeAllocateHomogenousVert(AG_Box *_Nonnull, const AG_SizeAlloc *_Nonnull, int);

AG_Box *
AG_BoxNew(void *parent, enum ag_box_type type, Uint flags)
{
	AG_Box *box;

	box = Malloc(sizeof(AG_Box));
	AG_ObjectInit(box, &agBoxClass);

	box->type = type;
	box->flags |= flags;

	if (flags & AG_BOX_HFILL) { WIDGET(box)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_BOX_VFILL) { WIDGET(box)->flags |= AG_WIDGET_VFILL; }

	if (flags & AG_BOX_NO_SPACING) {
		AG_SetStyle(box, "spacing", "0");
		AG_SetStyle(box, "padding", "0");
	}

	AG_ObjectAttach(parent, box);
	return (box);
}

AG_Box *
AG_BoxNewHoriz(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_HORIZ, flags);
}

AG_Box *
AG_BoxNewVert(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_VERT, flags);
}

/* Set the graphical style of background and borders. */
void
AG_BoxSetStyle(AG_Box *box, enum ag_box_style which)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
	box->style = which;
	AG_Redraw(box);
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
	} else {
		AG_BoxSetLabelS(box, NULL);
	}
}

/* Set the label text (format string). */
void
AG_BoxSetLabelS(AG_Box *box, const char *s)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
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
	box->style = AG_BOX_STYLE_NONE;
	box->type = AG_BOX_VERT;
	box->wPre = -1;
	box->hPre = -1;
	box->depth = -1;
	box->lbl = NULL;
	box->hAlign = AG_BOX_LEFT;
	box->vAlign = AG_BOX_TOP;

	AG_SetEvent(box, "mouse-button-down", MouseButtonDown, NULL);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Box *box = obj;
	const AG_Color *cBg = &WCOLOR(box, BG_COLOR);
	AG_Widget *chld;
	AG_Rect r;

	if (box->style != AG_BOX_STYLE_NONE && cBg->a > 0) {
		static void (*pfBox[])(void *_Nonnull, const AG_Rect *_Nonnull,
		                       const AG_Color *_Nonnull) = {
			ag_draw_rect_noop,  /* NONE */
			ag_draw_box_raised, /* BOX */
			ag_draw_box_sunk,   /* WELL */
			ag_draw_rect        /* PLAIN */
		};
#ifdef AG_DEBUG
		if (box->style >= AG_BOX_STYLE_LAST)
			AG_FatalError("style");
#endif
		pfBox[box->style](box, &r, cBg);
	} else if (box->flags & AG_BOX_SHADING) {            /* Shading only */
		if (box->depth < 0) {
			AG_DrawFrameSunk(box, &r);
		} else {
			AG_DrawFrameRaised(box, &r);
		}
	}

	OBJECT_FOREACH_CHILD(chld, box, ag_widget)
		AG_WidgetDraw(chld);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	static Uint (*pfSizeRequest[])(AG_Box *_Nonnull, int *_Nonnull) = {
		SizeRequestHoriz,
		SizeRequestVert
	};
	AG_Box *box = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	int nWidgets, totFixed, wdMax=0;
	
	r->w = WIDGET(box)->paddingLeft + WIDGET(box)->paddingRight;
	r->h = WIDGET(box)->paddingTop  + WIDGET(box)->paddingBottom;

#ifdef AG_DEBUG
	if (box->type >= AG_BOX_TYPE_LAST) AG_FatalError("box->type");
#endif
	nWidgets = pfSizeRequest[box->type](box, &totFixed);

	switch (box->type) {
	case AG_BOX_HORIZ:
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			AG_WidgetSizeReq(chld, &rChld);
			if (rChld.h > wdMax) {
				wdMax = rChld.h;
			}
			r->h = MAX(r->h, wdMax + WIDGET(box)->paddingTop +
			                         WIDGET(box)->paddingBottom);
			r->w += rChld.w + WIDGET(box)->spacingHoriz;
		}
		if (nWidgets > 0) {
			if (r->w >= WIDGET(box)->spacingHoriz)
				r->w -= WIDGET(box)->spacingHoriz;
		}
		break;
	case AG_BOX_VERT:
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			AG_WidgetSizeReq(chld, &rChld);
			if (rChld.w > wdMax) {
				wdMax = rChld.w;
			}
			r->w = MAX(r->w, wdMax + WIDGET(box)->paddingLeft +
			                         WIDGET(box)->paddingRight);
			r->h += rChld.h + WIDGET(box)->spacingVert;
		}
		if (nWidgets > 0) {
			if (r->h >= WIDGET(box)->spacingVert)
				r->h -= WIDGET(box)->spacingVert;
		}
		break;
	case AG_BOX_TYPE_LAST:
	default:
		break;
	}
	if (box->wPre != -1) { r->w = box->wPre; }
	if (box->hPre != -1) { r->h = box->hPre; }
}

static Uint
SizeRequestHoriz(AG_Box *_Nonnull box, int *_Nonnull totFixed)
{
	AG_SizeReq r;
	AG_Widget *chld;
	const int spacing = WIDGET(box)->spacingHoriz;
	Uint count = 0;
	int fixed = 0;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &r);
		if ((chld->flags & AG_WIDGET_HFILL) == 0) {
			fixed += r.w;
		}
		count++;
		fixed += spacing;
	}
	if (count > 0 && fixed >= spacing) {
		fixed -= spacing;
	}
	*totFixed = fixed;
	return (count);
}

static Uint
SizeRequestVert(AG_Box *_Nonnull box, int *_Nonnull totFixed)
{
	AG_SizeReq r;
	AG_Widget *chld;
	const int spacing = WIDGET(box)->spacingVert;
	Uint count = 0;
	int fixed = 0;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &r);
		if ((chld->flags & AG_WIDGET_VFILL) == 0) {
			fixed += r.h;
		}
		count++;
		fixed += spacing;
	}
	if (count > 0 && fixed >= spacing) {
		fixed -= spacing;
	}
	*totFixed = fixed;
	return (count);
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	static Uint (*pfSizeRequest[])(AG_Box *_Nonnull, int *_Nonnull) = {
		SizeRequestHoriz,
		SizeRequestVert
	};
	static void (*pfSizeAllocate[])(AG_Box *_Nonnull,
	                                const AG_SizeAlloc *_Nonnull, int) = {
		SizeAllocateHoriz,
		SizeAllocateVert,
		SizeAllocateHomogenousHoriz,
		SizeAllocateHomogenousVert
	};
	AG_Box *box = obj;
	int nWidgets, totFixed;

#ifdef AG_DEBUG
	if (box->type >= AG_BOX_TYPE_LAST) AG_FatalError("box->type");
#endif
	nWidgets = pfSizeRequest[box->type](box, &totFixed);

	if (nWidgets < 1) {
		return (0);
	}
	if (box->flags & AG_BOX_HOMOGENOUS) {
		pfSizeAllocate[box->type + 2](box, a, nWidgets);
	} else {
		pfSizeAllocate[box->type](box, a, totFixed);
	}
	return (0);
}

static void
SizeAllocateHoriz(AG_Box *_Nonnull box, const AG_SizeAlloc *_Nonnull a,
    int totFixed)
{
	AG_Widget *chld;
	const int wAvail = a->w - (WIDGET(box)->paddingLeft +
	                           WIDGET(box)->paddingRight);
	const int hAvail = a->h - (WIDGET(box)->paddingTop +
	                           WIDGET(box)->paddingBottom);
	const int spacing = WIDGET(box)->spacingHoriz;
	const enum ag_box_align vAlign = box->vAlign;
	int x = WIDGET(box)->paddingLeft;
	int y = WIDGET(box)->paddingTop;

	if (totFixed < wAvail) {
		switch (box->hAlign) {
		case AG_BOX_CENTER: x = (wAvail>>1) - (totFixed>>1);  break;
		case AG_BOX_RIGHT:  x =  wAvail     -  totFixed;      break;
		default:                                              break;
		}
		switch (vAlign) {
		case AG_BOX_CENTER: y = (hAvail>>1) - (totFixed>>1);  break;
		case AG_BOX_BOTTOM: y =  hAvail     -  totFixed;      break;
		default:                                              break;
		}
	}
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		const Uint flags = chld->flags;
		AG_SizeReq rChld;
		AG_SizeAlloc aChld;

		AG_WidgetSizeReq(chld, &rChld);
		aChld.w = (flags & AG_WIDGET_HFILL) ? (wAvail - totFixed) :
		                                      MIN(wAvail, rChld.w);
		aChld.h = (flags & AG_WIDGET_VFILL) ? hAvail :
		                                      MIN(hAvail, rChld.h);
		aChld.x = x;
		if (aChld.x + aChld.w > a->w) {
			aChld.w = a->w - aChld.x;
		}
		switch (vAlign) {
		case AG_BOX_TOP:    aChld.y = y;                          break;
		case AG_BOX_CENTER: aChld.y = (hAvail>>1) - (aChld.h>>1); break;
		case AG_BOX_BOTTOM: aChld.y =  hAvail     -  aChld.h;     break;
		}
		AG_WidgetSizeAlloc(chld, &aChld);
		AG_SETFLAGS(chld->flags, AG_WIDGET_VISIBLE, (aChld.w > 0));
		x += aChld.w + spacing;
	}
}

static void
SizeAllocateVert(AG_Box *_Nonnull box, const AG_SizeAlloc *_Nonnull a,
    int totFixed)
{
	AG_Widget *chld;
	const int wAvail = a->w - (WIDGET(box)->paddingLeft +
	                           WIDGET(box)->paddingRight);
	const int hAvail = a->h - (WIDGET(box)->paddingTop +
	                           WIDGET(box)->paddingBottom);
	const int spacing = WIDGET(box)->spacingVert;
	const enum ag_box_align hAlign = box->hAlign;
	int x = WIDGET(box)->paddingLeft;
	int y = WIDGET(box)->paddingTop;

	if (totFixed < wAvail) {
		switch (hAlign) {
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
		const Uint flags = chld->flags;
		AG_SizeReq rChld;
		AG_SizeAlloc aChld;

		AG_WidgetSizeReq(chld, &rChld);

		aChld.w = (flags & AG_WIDGET_HFILL) ? wAvail :
		                                      MIN(wAvail, rChld.w);
		aChld.h = (flags & AG_WIDGET_VFILL) ? (hAvail - totFixed) :
		                                      MIN(hAvail,rChld.h);
		aChld.y = y;

		if (aChld.y+aChld.h > a->h) {
			aChld.h = a->h - aChld.y;
		}
		switch (hAlign) {
		case AG_BOX_TOP:    aChld.x = x;                          break;
		case AG_BOX_CENTER: aChld.x = (wAvail>>1) - (aChld.w>>1); break;
		case AG_BOX_BOTTOM: aChld.x =  wAvail     -  aChld.w;     break;
		}
		AG_WidgetSizeAlloc(chld, &aChld);
		AG_SETFLAGS(chld->flags, AG_WIDGET_VISIBLE, (aChld.h > 0));
		y += aChld.h + spacing;
	}
}

static void
SizeAllocateHomogenousHoriz(AG_Box *_Nonnull box,
    const AG_SizeAlloc *_Nonnull a, int nWidgets)
{
	AG_SizeAlloc aChld;
	AG_Widget *chld, *chldLast=NULL;
	const int paddingTopBot = WIDGET(box)->paddingTop +
	                          WIDGET(box)->paddingBottom;
	const int wAvail = a->w - (WIDGET(box)->paddingLeft +
	                           WIDGET(box)->paddingRight);
	int wUsed = 0;

	aChld.x = WIDGET(box)->paddingLeft;
	aChld.y = WIDGET(box)->paddingTop;
	aChld.w = (wAvail - nWidgets - 1) / nWidgets;
	aChld.h = (a->h - paddingTopBot);

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeAlloc(chld, &aChld);

		if (chld->flags & AG_WIDGET_UNDERSIZE) {
			continue;
		}
		if (OBJECT(chld) == TAILQ_LAST(&OBJECT(box)->children, ag_objectq)) {
			chldLast = chld;
		} else {
			aChld.x += aChld.w + 1;
		}
		wUsed += (aChld.w + 1);
	}
	if (chldLast != NULL && wUsed < wAvail) {    /* Roundoff compensation */
		aChld.w += wAvail - wUsed;
		AG_WidgetSizeAlloc(chldLast, &aChld);
	}
}

static void
SizeAllocateHomogenousVert(AG_Box *_Nonnull box,
    const AG_SizeAlloc *_Nonnull a, int nWidgets)
{
	AG_SizeAlloc aChld;
	AG_Widget *chld, *chldLast=NULL;
	const int paddingLeftRight = WIDGET(box)->paddingLeft +
	                             WIDGET(box)->paddingRight;
	const int hAvail = a->h - (WIDGET(box)->paddingTop +
	                           WIDGET(box)->paddingBottom);
	int hUsed = 0;

	aChld.x = WIDGET(box)->paddingLeft;
	aChld.y = WIDGET(box)->paddingTop;
	aChld.w = (a->w - paddingLeftRight);
	aChld.h = (hAvail - nWidgets - 1) / nWidgets;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeAlloc(chld, &aChld);

		if (chld->flags & AG_WIDGET_UNDERSIZE) {
			continue;
		}
		if (OBJECT(chld) == TAILQ_LAST(&OBJECT(box)->children, ag_objectq)) {
			chldLast = chld;
		} else {
			aChld.y += aChld.h + 1;
		}
		hUsed += (aChld.h + 1);
	}
	if (chldLast != NULL && hUsed < hAvail) {    /* Roundoff compensation */
		aChld.h += hAvail - hUsed;
		AG_WidgetSizeAlloc(chldLast, &aChld);
	}
}

/* Set a specific size requisition in pixels (-1 = auto). */
void
AG_BoxSizeHint(AG_Box *box, int w, int h)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
	box->wPre = w;
	box->hPre = h;
}

/* Enable/Disable HOMOGENOUS (divide space equally) mode. */
void
AG_BoxSetHomogenous(AG_Box *box, int enable)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
	AG_ObjectLock(box);

	AG_SETFLAGS(box->flags, AG_BOX_HOMOGENOUS, enable);

	AG_ObjectUnlock(box);
	AG_Redraw(box);
}

/* Set depth of shading in pixels. */
void
AG_BoxSetDepth(AG_Box *box, int depth)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
	box->depth = depth;
	AG_Redraw(box);
}

/* Set packing alignment (HORIZ or VERT). */
void
AG_BoxSetType(AG_Box *box, enum ag_box_type type)
{
	AG_SizeAlloc a;

#ifdef AG_DEBUG
	if (type >= AG_BOX_TYPE_LAST)
		AG_FatalError("Box type");
#endif
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
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

/* Set horizontal alignment (LEFT, CENTER, RIGHT). */
void
AG_BoxSetHorizAlign(AG_Box *box, enum ag_box_align align)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
	box->hAlign = align;
	AG_Redraw(box);
}

/* Set vertical alignment (TOP, MIDDLE, BOTTOM). */
void
AG_BoxSetVertAlign(AG_Box *box, enum ag_box_align align)
{
	AG_OBJECT_ISA(box, "AG_Widget:AG_Box:*");
	box->vAlign = align;
	AG_Redraw(box);
}

static void
UpdateWindowOfTgt(AG_Event *_Nonnull event)
{
	AG_Box *box = AG_BOX_PTR(1);
	AG_Window *win = AG_ParentWindow(box);

	AG_Redraw(box);
	AG_WindowUpdate(win);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	AG_Box *tgt = obj, *box, *hBox;
	AG_Numerical *num;
	AG_Checkbox *cb;
	AG_Radio *rad;
	const char *typeNames[] = {
		N_("Vertical"),
		N_("Horizontal"),
		NULL
	};
	const char *styleNames[] = {
		N_("None"),
		N_("Box"),
		N_("Well"),
		N_("Plain"),
		NULL
	};

	box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	AG_LabelNewS(box, 0, _("Disposition:"));
	rad = AG_RadioNewUint(box, 0, typeNames, (Uint *)&tgt->type);
	AG_SetEvent(rad, "radio-changed", UpdateWindowOfTgt,"%p",tgt);

	AG_LabelNewS(box, 0, _("Style:"));
	rad = AG_RadioNewUint(box, 0, styleNames, (Uint *)&tgt->style);
	AG_SetEvent(rad, "radio-changed", UpdateWindowOfTgt,"%p",tgt);

	AG_LabelNewS(box, 0, _("Alignment:"));
	hBox = AG_BoxNewHoriz(box, 0);
	{
		rad = AG_RadioNewUint(hBox, 0, agBoxHorizAlignNames, (Uint *)&tgt->hAlign);
		AG_SetEvent(rad, "radio-changed", UpdateWindowOfTgt,"%p",tgt);
	
		rad = AG_RadioNewUint(hBox, 0, agBoxVertAlignNames, (Uint *)&tgt->vAlign);
		AG_SetEvent(rad, "radio-changed", UpdateWindowOfTgt,"%p",tgt);
	}

	AG_SpacerNewHoriz(box);

	num = AG_NumericalNewIntR(box, 0, NULL, _("Depth: "), &tgt->depth, -127, 127);
	AG_SetEvent(num, "numerical-changed", UpdateWindowOfTgt,"%p",tgt);
	
	AG_SpacerNewHoriz(box);

	cb = AG_CheckboxNewFlag(box, 0, _("Homogenous"), &tgt->flags, AG_BOX_HOMOGENOUS);
	AG_SetEvent(cb, "checkbox-changed", UpdateWindowOfTgt,"%p",tgt);

	cb = AG_CheckboxNewFlag(box, 0, _("Shading"), &tgt->flags, AG_BOX_SHADING);
	AG_SetEvent(cb, "checkbox-changed", UpdateWindowOfTgt,"%p",tgt);
	
	return (box);
}

#ifdef AG_LEGACY
AG_HBox *
AG_HBoxNew(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_HORIZ, flags);
}

AG_VBox *
AG_VBoxNew(void *p, Uint flags)
{
	return AG_BoxNew(p, AG_BOX_VERT, flags);
}
#endif /* AG_LEGACY */

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
		Edit
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */

/*
 * Copyright (c) 2003-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
		AG_SetSpacing(box, "0");
		AG_SetPadding(box, "0");
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
			AG_SetFontSize(box->lbl, "80%");
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
MouseButtonDown(void *obj, AG_MouseButton button, int x, int y)
{
	AG_Box *box = obj;
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
	AG_SizeReq req;
	int nWidgets, totReqd, wdMax=0;
	
	r->w = WIDGET(box)->paddingLeft + WIDGET(box)->paddingRight;
	r->h = WIDGET(box)->paddingTop  + WIDGET(box)->paddingBottom;

#ifdef AG_DEBUG
	if (box->type >= AG_BOX_TYPE_LAST) AG_FatalError("box->type");
#endif
	nWidgets = pfSizeRequest[box->type](box, &totReqd);

	switch (box->type) {
	case AG_BOX_HORIZ:
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			AG_WidgetSizeReq(chld, &req);
			req.h += (chld->marginTop + chld->marginBottom);
			if (req.h > wdMax) {
				wdMax = req.h;
			}
			r->h = MAX(r->h, wdMax + WIDGET(box)->paddingTop +
			                         WIDGET(box)->paddingBottom);
			r->w += req.w + WIDGET(box)->spacingHoriz;
		}
		if (nWidgets > 0) {
			if (r->w >= WIDGET(box)->spacingHoriz)
				r->w -= WIDGET(box)->spacingHoriz;
		}
		break;
	case AG_BOX_VERT:
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			AG_WidgetSizeReq(chld, &req);
			req.w += (chld->marginLeft + chld->marginRight);
			if (req.w > wdMax) {
				wdMax = req.w;
			}
			r->w = MAX(r->w, wdMax + WIDGET(box)->paddingLeft +
			                         WIDGET(box)->paddingRight);
			r->h += req.h + WIDGET(box)->spacingVert;
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
SizeRequestHoriz(AG_Box *_Nonnull box, int *_Nonnull totReqd)
{
	AG_SizeReq req;
	AG_Widget *chld;
	const int spacing = WIDGET(box)->spacingHoriz;
	Uint count = 0;
	int wReqd = 0;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &req);
		if ((chld->flags & AG_WIDGET_HFILL) == 0) {
			wReqd += (req.w + chld->marginLeft + chld->marginRight);
		}
		count++;
		wReqd += spacing;
	}
	if (wReqd >= spacing) {
		wReqd -= spacing;
	}
	*totReqd = wReqd;
	return (count);
}

static Uint
SizeRequestVert(AG_Box *_Nonnull box, int *_Nonnull totReqd)
{
	AG_SizeReq req;
	AG_Widget *chld;
	const int spacing = WIDGET(box)->spacingVert;
	Uint count = 0;
	int hReqd = 0;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeReq(chld, &req);
		if ((chld->flags & AG_WIDGET_VFILL) == 0) {
			hReqd += (req.h + chld->marginTop + chld->marginBottom);
		}
		count++;
		hReqd += spacing;
	}
	if (hReqd >= spacing) {
		hReqd -= spacing;
	}
	*totReqd = hReqd;
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
	int nWidgets, totReqd;

#ifdef AG_DEBUG
	if (box->type >= AG_BOX_TYPE_LAST) AG_FatalError("box->type");
#endif
	nWidgets = pfSizeRequest[box->type](box, &totReqd);

	if (nWidgets < 1) {
		return (0);
	}
	if (box->flags & AG_BOX_HOMOGENOUS) {
		pfSizeAllocate[box->type + 2](box, a, nWidgets);
	} else {
		pfSizeAllocate[box->type](box, a, totReqd);
	}
	return (0);
}

static void
SizeAllocateHoriz(AG_Box *_Nonnull box, const AG_SizeAlloc *_Nonnull a,
    int wReqd)
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

	if (wReqd < wAvail) {
		switch (box->hAlign) {
		case AG_BOX_CENTER:
			x = (wAvail >> 1) - (wReqd >> 1);
			break;
		case AG_BOX_RIGHT:
			x =  wAvail - wReqd;
			break;
		default:
			break;
		}
		if (x < 0) { x = 0; }

		switch (vAlign) {
		case AG_BOX_CENTER:
			y = (hAvail >> 1) - (wReqd >> 1);
			break;
		case AG_BOX_BOTTOM:
			y =  hAvail - wReqd;
			break;
		default:
			break;
		}
		if (y < 0) { y = 0; }
	}

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		const Uint flags = chld->flags;
		AG_SizeReq req;
		AG_SizeAlloc alloc;

		AG_WidgetSizeReq(chld, &req);

		alloc.x = x + chld->marginLeft;
		alloc.w = (flags & AG_WIDGET_HFILL) ?
		          (wAvail - wReqd) :
		          MIN(wAvail, req.w);
		alloc.h = (flags & AG_WIDGET_VFILL) ?
		          (hAvail - chld->marginTop - chld->marginBottom) :
		          MIN(hAvail, req.h);

		if (x + alloc.w > a->w)
			alloc.w = a->w - x;

		switch (vAlign) {
		case AG_BOX_TOP:
			alloc.y = y + chld->marginTop;
			break;
		case AG_BOX_CENTER:
			alloc.y = (hAvail >> 1) - (alloc.h >> 1);
			if (alloc.y < 0) { alloc.y = 0; }
			break;
		case AG_BOX_BOTTOM:
			alloc.y = hAvail - alloc.h - chld->marginBottom;
			if (alloc.y < 0) { alloc.y = 0; }
			break;
		}

		AG_WidgetSizeAlloc(chld, &alloc);

		AG_SETFLAGS(chld->flags, AG_WIDGET_VISIBLE, (alloc.w > 0));

		x += alloc.w + spacing + chld->marginLeft + chld->marginRight;
	}
}

static void
SizeAllocateVert(AG_Box *_Nonnull box, const AG_SizeAlloc *_Nonnull a,
    int hReqd)
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

	if (hReqd < hAvail) {
		switch (hAlign) {
		case AG_BOX_CENTER:
			x = (wAvail >> 1) - (hReqd >> 1);
			break;
		case AG_BOX_RIGHT:
			x = wAvail - hReqd;
			break;
		default:
			break;
		}
		if (x < 0) { x = 0; }

		switch (box->vAlign) {
		case AG_BOX_CENTER:
			y = (hAvail >> 1) - (hReqd >> 1);
			break;
		case AG_BOX_BOTTOM:
			y = hAvail - hReqd;
			break;
		default:
			break;
		}
		if (y < 0) { y = 0; }
	}

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_SizeReq req;
		AG_SizeAlloc alloc;
		const Uint flags = chld->flags;

		AG_WidgetSizeReq(chld, &req);

		alloc.y = y + chld->marginTop;
		alloc.w = (flags & AG_WIDGET_HFILL) ?
		          (wAvail - chld->marginLeft - chld->marginRight) :
		          MIN(wAvail, req.w);
		alloc.h = (flags & AG_WIDGET_VFILL) ?
		          (hAvail - hReqd) :
		          MIN(hAvail, req.h);

		if (y + alloc.h > a->h)
			alloc.h = a->h - y;

		switch (hAlign) {
		case AG_BOX_LEFT:
			alloc.x = x + chld->marginLeft;
			break;
		case AG_BOX_CENTER:
			alloc.x = (wAvail >> 1) - (alloc.w >> 1);
			if (alloc.x < 0) { alloc.x = 0; }
			break;
		case AG_BOX_RIGHT:
			alloc.x = wAvail - alloc.w - chld->marginRight;
			if (alloc.x < 0) { alloc.x = 0; }
			break;
		}

		AG_WidgetSizeAlloc(chld, &alloc);

		AG_SETFLAGS(chld->flags, AG_WIDGET_VISIBLE, (alloc.h > 0));

		y += alloc.h + spacing + chld->marginTop + chld->marginBottom;
	}
}

static void
SizeAllocateHomogenousHoriz(AG_Box *_Nonnull box,
    const AG_SizeAlloc *_Nonnull a, int nWidgets)
{
	AG_SizeAlloc alloc;
	AG_Widget *chld, *chldLast=NULL;
	const int paddingTopBot = WIDGET(box)->paddingTop +
	                          WIDGET(box)->paddingBottom;
	const int wAvail = a->w - (WIDGET(box)->paddingLeft +
	                           WIDGET(box)->paddingRight);
	const int spacing = WIDGET(box)->spacingHoriz;
	int wUsed = 0;

	alloc.x = WIDGET(box)->paddingLeft;
	alloc.y = WIDGET(box)->paddingTop;
	alloc.w = (wAvail - nWidgets - 1) / nWidgets;
	alloc.h = (a->h - paddingTopBot);

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeAlloc(chld, &alloc);

		if (chld->flags & AG_WIDGET_UNDERSIZE) {
			continue;
		}
		if (OBJECT(chld) == TAILQ_LAST(&OBJECT(box)->children, ag_objectq)) {
			chldLast = chld;
		} else {
			alloc.x += alloc.w + spacing;
		}
		wUsed += (alloc.w + spacing);
	}

	if (chldLast != NULL && wUsed < wAvail) {    /* Roundoff compensation */
		alloc.w += wAvail - wUsed;
		AG_WidgetSizeAlloc(chldLast, &alloc);
	}
}

static void
SizeAllocateHomogenousVert(AG_Box *_Nonnull box,
    const AG_SizeAlloc *_Nonnull a, int nWidgets)
{
	AG_SizeAlloc alloc;
	AG_Widget *chld, *chldLast=NULL;
	const int paddingLeftRight = WIDGET(box)->paddingLeft +
	                             WIDGET(box)->paddingRight;
	const int hAvail = a->h - (WIDGET(box)->paddingTop +
	                           WIDGET(box)->paddingBottom);
	const int spacing = WIDGET(box)->spacingVert;
	int hUsed = 0;

	alloc.x = WIDGET(box)->paddingLeft;
	alloc.y = WIDGET(box)->paddingTop;
	alloc.w = (a->w - paddingLeftRight);
	alloc.h = (hAvail - nWidgets - 1) / nWidgets;

	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		AG_WidgetSizeAlloc(chld, &alloc);

		if (chld->flags & AG_WIDGET_UNDERSIZE) {
			continue;
		}
		if (OBJECT(chld) == TAILQ_LAST(&OBJECT(box)->children, ag_objectq)) {
			chldLast = chld;
		} else {
			alloc.y += alloc.h + spacing;
		}
		hUsed += (alloc.h + spacing);
	}

	if (chldLast != NULL && hUsed < hAvail) {    /* Roundoff compensation */
		alloc.h += hAvail - hUsed;
		AG_WidgetSizeAlloc(chldLast, &alloc);
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
		{ 1,0, AGC_BOX, 0xE010 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		Edit
	},
	Draw,
	SizeRequest,
	SizeAllocate,
	MouseButtonDown,
	NULL,			/* mouse_button_up */
	NULL,			/* mouse_motion */
	NULL,			/* key_down */
	NULL,			/* key_up */
	NULL,			/* touch */
	NULL,			/* ctrl */
	NULL			/* joy */
};

#endif /* AG_WIDGETS */

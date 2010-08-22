/*
 * Copyright (c) 2002-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Single or multi-line text input widget. This is a simple subclass of
 * AG_Editable(3) adding a built-in label, pixel padding and scrollbars.
 */

#include <core/core.h>
#include <core/config.h>

#include "ttf.h"
#include "textbox.h"

#include "text.h"
#include "keymap.h"
#include "primitive.h"
#include "window.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

static void
BeginScrollbarDrag(AG_Event *event)
{
	AG_Textbox *tb = AG_PTR(1);
	
	AG_WidgetFocus(tb->ed);
	tb->ed->flags |= AG_EDITABLE_NOSCROLL;
}

static void
EndScrollbarDrag(AG_Event *event)
{
	AG_Textbox *tb = AG_PTR(1);

	tb->ed->flags &= ~(AG_EDITABLE_NOSCROLL);
}

AG_Textbox *
AG_TextboxNew(void *parent, Uint flags, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(s, sizeof(s), fmt, ap);
		va_end(ap);
		return AG_TextboxNewS(parent, flags, s);
	} else {
		return AG_TextboxNewS(parent, flags, NULL);
	}
}

AG_Textbox *
AG_TextboxNewS(void *parent, Uint flags, const char *label)
{
	AG_Textbox *tb;

	tb = Malloc(sizeof(AG_Textbox));
	AG_ObjectInit(tb, &agTextboxClass);

	if (flags & AG_TEXTBOX_HFILL)
		AG_ExpandHoriz(tb);
	if (flags & AG_TEXTBOX_VFILL)
		AG_ExpandVert(tb);
	if (flags & AG_TEXTBOX_READONLY) {
		AG_WidgetDisable(tb);
		AG_WidgetDisable(tb->ed);
	}
	if (flags & AG_TEXTBOX_PASSWORD)
		tb->ed->flags |= AG_EDITABLE_PASSWORD;
	if (flags & AG_TEXTBOX_ABANDON_FOCUS)
		tb->ed->flags |= AG_EDITABLE_ABANDON_FOCUS;
	if (flags & AG_TEXTBOX_INT_ONLY)
		tb->ed->flags |= AG_EDITABLE_INT_ONLY;
	if (flags & AG_TEXTBOX_FLT_ONLY)
		tb->ed->flags |= AG_EDITABLE_FLT_ONLY;
	if (flags & AG_TEXTBOX_CATCH_TAB) {
		WIDGET(tb)->flags |= AG_WIDGET_CATCH_TAB;
		WIDGET(tb->ed)->flags |= AG_WIDGET_CATCH_TAB;
	}
	if (flags & AG_TEXTBOX_NOEMACS)
		tb->ed->flags |= AG_EDITABLE_NOEMACS;
	if (flags & AG_TEXTBOX_NOWORDSEEK)
		tb->ed->flags |= AG_EDITABLE_NOWORDSEEK;
	if (flags & AG_TEXTBOX_NOLATIN1)
		tb->ed->flags |= AG_EDITABLE_NOLATIN1;
	
	if (flags & AG_TEXTBOX_MULTILINE) {
		tb->ed->flags |= AG_EDITABLE_MULTILINE;

		tb->vBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_VERT,
		    AG_SCROLLBAR_AUTOHIDE);
		AG_BindInt(tb->vBar, "value", &tb->ed->y);
		AG_BindInt(tb->vBar, "max", &tb->ed->yMax);
		AG_BindInt(tb->vBar, "visible", &tb->ed->yVis);
		AG_SetEvent(tb->vBar, "scrollbar-drag-begin", BeginScrollbarDrag, "%p", tb);
		AG_SetEvent(tb->vBar, "scrollbar-drag-end", EndScrollbarDrag, "%p", tb);
	}
	
	AG_TextboxSetStatic(tb, (flags & AG_TEXTBOX_STATIC));
	AG_TextboxSetWordWrap(tb, (flags & AG_TEXTBOX_WORDWRAP));

	tb->flags |= flags;
	if (label != NULL) {
		tb->lbl = AG_LabelNewS(tb, 0, label);
		AG_LabelSetPadding(tb->lbl, -1, 10, -1, -1);
	}
	AG_ObjectAttach(parent, tb);
	return (tb);
}

/* Toggle word wrapping */
void
AG_TextboxSetWordWrap(AG_Textbox *tb, int flag)
{
	AG_EditableSetWordWrap(tb->ed, flag);

	if (tb->ed->flags & AG_EDITABLE_MULTILINE) {
		if (tb->hBar != NULL) {
			AG_ObjectDetach(tb->hBar);
			AG_ObjectDestroy(tb->hBar);
			tb->hBar = NULL;
		}
		if (!flag) {
			tb->hBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_HORIZ,
			    AG_SCROLLBAR_AUTOHIDE);
			AG_BindInt(tb->hBar, "value", &tb->ed->x);
			AG_BindInt(tb->hBar, "max", &tb->ed->xMax);
			AG_BindInt(tb->hBar, "visible", &WIDTH(tb->ed));
			AG_SetEvent(tb->hBar, "scrollbar-drag-begin", BeginScrollbarDrag, "%p", tb);
			AG_SetEvent(tb->hBar, "scrollbar-drag-end", EndScrollbarDrag, "%p", tb);
			AG_TextboxSizeHintLines(tb, 4);
		}
	}
}

/* Configure alternate font */
void
AG_TextboxSetFont(AG_Textbox *tb, AG_Font *font)
{
	AG_ObjectLock(tb);
	tb->font = font;
	AG_ObjectUnlock(tb);
	AG_Redraw(tb);
}

static void
Draw(void *p)
{
	AG_Textbox *tb = p;

	STYLE(tb)->TextboxBackground(tb, tb->r, (tb->flags & AG_TEXTBOX_COMBO));

	if (tb->lbl != NULL)
		AG_WidgetDraw(tb->lbl);

	AG_PushClipRect(tb, tb->r);

	if (tb->flags & AG_TEXTBOX_MULTILINE) {
		int d;

		if (tb->vBar != NULL && AG_ScrollbarVisible(tb->vBar)) {
			d = WIDTH(tb->vBar);
			AG_DrawBox(tb,
			    AG_RECT(WIDTH(tb)-d, HEIGHT(tb)-d, d, d), -1,
			    agColors[TEXTBOX_COLOR]);
		} else if (tb->hBar != NULL && AG_ScrollbarVisible(tb->hBar)) {
			d = HEIGHT(tb->hBar);
			AG_DrawBox(tb,
			    AG_RECT(WIDTH(tb)-d, HEIGHT(tb)-d, d, d), -1,
			    agColors[TEXTBOX_COLOR]);
		}
		AG_WidgetUpdate(tb);
	}

	/* Render the Editable widget, inheriting our Font. */
	tb->ed->font = tb->font;
	AG_WidgetDraw(tb->ed);

	if (tb->hBar != NULL)
		AG_WidgetDraw(tb->hBar);
	if (tb->vBar != NULL)
		AG_WidgetDraw(tb->vBar);

	AG_PopClipRect(tb);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Textbox *tb = obj;
	AG_SizeReq rEd, rLbl;

	AG_WidgetSizeReq(tb->ed, &rEd);

	r->w = tb->boxPadX*2 + rEd.w;
	r->h = tb->boxPadY*2 + rEd.h;

	if (tb->lbl != NULL) {
		AG_WidgetSizeReq(tb->lbl, &rLbl);
		r->w += rLbl.w;
	}
	r->h = MAX(r->h, agTextFontLineSkip);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Textbox *tb = obj;
	int boxPadW = tb->boxPadX*2;
	int boxPadH = tb->boxPadY*2;
	int wBar = 0, hBar = 0, wBarSz = 0, hBarSz = 0;
	AG_SizeAlloc aLbl, aEd, aSb;
	AG_SizeReq r;

	if (a->w < boxPadW*2 || a->h < boxPadH*2)
		return (-1);

	if (tb->lbl != NULL) {
		AG_WidgetSizeReq(tb->lbl, &r);
		aLbl.x = boxPadW;
		aLbl.y = boxPadH;
		aLbl.w = MIN(r.w, a->w);
		aLbl.h = MIN(r.h, a->h);
		AG_WidgetSizeAlloc(tb->lbl, &aLbl);
	}
	if (tb->flags & AG_TEXTBOX_MULTILINE) {
		if (tb->hBar != NULL &&
		    AG_WidgetVisible(tb->hBar)) {
			AG_WidgetSizeReq(tb->hBar, &r);
			hBarSz = MIN(r.h, a->h);
			aSb.x = 0;
			aSb.y = a->h - hBarSz;
			aSb.w = a->w - hBarSz + 1;
			aSb.h = hBarSz;
			AG_WidgetSizeAlloc(tb->hBar, &aSb);
			if (AG_ScrollbarVisible(tb->hBar))
				hBar = aSb.h;
		}
		if (tb->vBar != NULL &&
		    AG_WidgetVisible(tb->vBar)) {
			AG_WidgetSizeReq(tb->vBar, &r);
			wBarSz = MIN(r.w, a->w);
			aSb.x = a->w - wBarSz;
			aSb.y = 0;
			aSb.w = wBarSz;
			aSb.h = a->h - hBarSz + 1;
			AG_WidgetSizeAlloc(tb->vBar, &aSb);
			if (AG_ScrollbarVisible(tb->vBar))
				wBar = aSb.w;
		}

		tb->r.x = 0;
		tb->r.y = 0;
		tb->r.w = a->w;
		tb->r.h = a->h;
	} else {
		tb->r.x = (tb->lbl != NULL) ? WIDTH(tb->lbl) : 0;
		tb->r.y = 0;
		tb->r.w = a->w - tb->r.x;
		tb->r.h = a->h;
	}
	
	aEd.x = ((tb->lbl != NULL) ? WIDTH(tb->lbl) : 0) + tb->boxPadX;
	aEd.y = tb->boxPadY;
	aEd.w = a->w - boxPadW - aEd.x - wBar;
	aEd.h = a->h - boxPadH - hBar;
	AG_WidgetSizeAlloc(tb->ed, &aEd);

	return (0);
}

/* Set the text from a format string. */
void
AG_TextboxPrintf(AG_Textbox *tb, const char *fmt, ...)
{
	AG_Variable *stringb;
	va_list args;
	char *text;

	AG_ObjectLock(tb->ed);
	stringb = AG_GetVariable(tb->ed, "string", &text);
	if (fmt != NULL && fmt[0] != '\0') {
		va_start(args, fmt);
		Vsnprintf(text, stringb->info.size, fmt, args);
		va_end(args);
		tb->ed->pos = AG_LengthUTF8(text);
	} else {
		text[0] = '\0';
		tb->ed->pos = 0;
	}
	AG_TextboxBufferChanged(tb);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(tb->ed);

	/* XXX for AG_Numerical, etc. */
	AG_Redraw(tb);
}

/* Set the textbox label (format string). */
void
AG_TextboxSetLabel(AG_Textbox *tb, const char *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	AG_TextboxSetLabelS(tb, s);
	Free(s);
}

/* Set the textbox label (C string). */
void
AG_TextboxSetLabelS(AG_Textbox *tb, const char *s)
{
	AG_ObjectLock(tb);
	if (tb->lbl != NULL) {
		AG_LabelTextS(tb->lbl, s);
	} else {
		tb->lbl = AG_LabelNewS(tb, 0, s);
		AG_LabelSetPadding(tb->lbl, -1, 10, -1, -1);
	}
	AG_ObjectUnlock(tb);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	AG_WidgetFocus(tb);
	AG_ForwardEvent(NULL, tb->ed, event);
}

static void
Disabled(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	AG_WidgetDisable(tb->ed);
}

static void
Enabled(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	AG_WidgetEnable(tb->ed);
}

#ifdef AG_DEBUG
static void
Bound(AG_Event *event)
{
	AG_FatalError("Use AG_TextboxBindUTF8() or AG_TextboxBindASCII()");
}
#endif

static void
EditablePreChg(AG_Event *event)
{
	AG_PostEvent(NULL, AG_PTR(1), "textbox-prechg", NULL);
}

static void
EditablePostChg(AG_Event *event)
{
	AG_PostEvent(NULL, AG_PTR(1), "textbox-postchg", NULL);
}

static void
EditableReturn(AG_Event *event)
{
	AG_PostEvent(NULL, AG_PTR(1), "textbox-return", NULL);
}

static void
Init(void *obj)
{
	AG_Textbox *tb = obj;
	
	WIDGET(tb)->flags |= AG_WIDGET_TABLE_EMBEDDABLE;

	tb->ed = AG_EditableNew(tb, 0);
	tb->lbl = NULL;

	tb->boxPadX = 2;
	tb->boxPadY = 2;
	tb->flags = 0;
	tb->hBar = NULL;
	tb->vBar = NULL;
	tb->r = AG_RECT(0,0,0,0);
	tb->font = NULL;

	AG_SetEvent(tb, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tb, "widget-disabled", Disabled, NULL);
	AG_SetEvent(tb, "widget-enabled", Enabled, NULL);
	AG_SetEvent(tb->ed, "editable-prechg", EditablePreChg, "%p", tb);
	AG_SetEvent(tb->ed, "editable-postchg", EditablePostChg, "%p", tb);
	AG_SetEvent(tb->ed, "editable-return", EditableReturn, "%p", tb);
	
	AG_WidgetForwardFocus(tb, tb->ed);

#ifdef AG_DEBUG
	AG_BindUint(tb, "flags", &tb->flags);
	AG_BindInt(tb, "boxPadX", &tb->boxPadX);
	AG_BindInt(tb, "boxPadY", &tb->boxPadY);
	AG_SetEvent(tb, "bound", Bound, NULL);
#endif /* AG_DEBUG */
}

AG_WidgetClass agTextboxClass = {
	{
		"Agar(Widget:Textbox)",
		sizeof(AG_Textbox),
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

/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Single or multi-line text input widget. This augments the AG_Editable
 * class with a built-in label, pixel padding and scrollbars.
 */

#include <core/core.h>
#include <core/config.h>

#include "ttf.h"
#include "textbox.h"

#include "text.h"
#include "keymap.h"
#include "primitive.h"
#include "opengl.h"
#include "window.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

static void
BeginDrag(AG_Event *event)
{
	AG_Textbox *tb = AG_PTR(1);
	
	AG_WidgetFocus(tb->ed);
	tb->ed->flags |= AG_EDITABLE_NOSCROLL;
}

static void
EndDrag(AG_Event *event)
{
	AG_Textbox *tb = AG_PTR(1);

	tb->ed->flags &= ~(AG_EDITABLE_NOSCROLL);
}

AG_Textbox *
AG_TextboxNew(void *parent, Uint flags, const char *label)
{
	AG_Textbox *tb;

	tb = Malloc(sizeof(AG_Textbox));
	AG_ObjectInit(tb, &agTextboxClass);

	if (!(flags & AG_TEXTBOX_NO_HFILL))
		AG_ExpandHoriz(tb);
	if (  flags & AG_TEXTBOX_VFILL)
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
	if (flags & AG_TEXTBOX_STATIC)
		tb->ed->flags |= AG_EDITABLE_STATIC;
	if (flags & AG_TEXTBOX_NOEMACS)
		tb->ed->flags |= AG_EDITABLE_NOEMACS;
	if (flags & AG_TEXTBOX_NOWORDSEEK)
		tb->ed->flags |= AG_EDITABLE_NOWORDSEEK;
	if (flags & AG_TEXTBOX_NOLATIN1)
		tb->ed->flags |= AG_EDITABLE_NOLATIN1;
		
	if (flags & AG_TEXTBOX_MULTILINE) {
		tb->ed->flags |= AG_EDITABLE_MULTILINE;
		tb->hBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_HORIZ, 0);
		tb->vBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_VERT, 0);
		AG_WidgetBindInt(tb->hBar, "value", &tb->ed->x);
		AG_WidgetBindInt(tb->vBar, "value", &tb->ed->y);
		AG_WidgetBindInt(tb->hBar, "max", &tb->ed->xMax);
		AG_WidgetBindInt(tb->vBar, "max", &tb->ed->yMax);
		AG_WidgetBindInt(tb->hBar, "visible", &WIDTH(tb->ed));
		AG_WidgetBindInt(tb->vBar, "visible", &tb->ed->yVis);
		AG_SetEvent(tb->hBar, "scrollbar-drag-begin",
		    BeginDrag, "%p", tb);
		AG_SetEvent(tb->vBar, "scrollbar-drag-begin",
		    BeginDrag, "%p", tb);
		AG_SetEvent(tb->hBar, "scrollbar-drag-end", EndDrag, "%p", tb);
		AG_SetEvent(tb->vBar, "scrollbar-drag-end", EndDrag, "%p", tb);
	}

	tb->flags |= flags;
	if (label != NULL) {
		tb->labelText = Strdup(label);
	}
	AG_ObjectAttach(parent, tb);
	return (tb);
}

static void
Destroy(void *p)
{
	AG_Textbox *tb = p;

	Free(tb->labelText);
}

static void
Draw(void *p)
{
	AG_Textbox *tb = p;
	AG_Rect rBox;

	if (tb->flags & AG_TEXTBOX_MULTILINE) {
		rBox.x = 0;
		rBox.y = 0;
		rBox.w = WIDTH(tb);
		rBox.h = HEIGHT(tb);
	} else {
		rBox.x = tb->wLbl + tb->lblPadL + tb->lblPadR;
		rBox.y = 0;
		rBox.w = WIDTH(tb) - tb->wLbl - tb->lblPadL - tb->lblPadR;
		rBox.h = HEIGHT(tb);
	}
	STYLE(tb)->TextboxBackground(tb, rBox, (tb->flags & AG_TEXTBOX_COMBO));

	if (tb->labelText != NULL && tb->label == -1) {
		AG_PushTextState();
		AG_TextColor(TEXTBOX_TXT_COLOR);
		tb->label = AG_WidgetMapSurface(tb,
		    AG_TextRender(tb->labelText));
		AG_PopTextState();
	}
	if (tb->label != -1) {
		AG_Surface *lblSu = WSURFACE(tb,tb->label);
	
		AG_PushClipRect(tb, AG_RECT(0, 0, tb->wLbl, HEIGHT(tb)));
		AG_WidgetBlitSurface(tb, tb->label,
		    tb->lblPadL,
		    HEIGHT(tb)/2 - lblSu->h/2);
		AG_PopClipRect();
	}
	if (tb->flags & AG_TEXTBOX_MULTILINE) {
		int d;

		if (tb->vBar != NULL && AG_ScrollbarVisible(tb->vBar)) {
			d = WIDTH(tb->vBar);
			AG_DrawBox(tb,
			    AG_RECT(WIDTH(tb)-d, HEIGHT(tb)-d, d, d), -1,
			    AG_COLOR(TEXTBOX_COLOR));
		} else if (tb->hBar != NULL && AG_ScrollbarVisible(tb->hBar)) {
			d = HEIGHT(tb->hBar);
			AG_DrawBox(tb,
			    AG_RECT(WIDTH(tb)-d, HEIGHT(tb)-d, d, d), -1,
			    AG_COLOR(TEXTBOX_COLOR));
		}
		AG_WindowUpdate(AG_ParentWindow(tb));
	}
	AG_WidgetDraw(tb->ed);
	if (tb->hBar != NULL) { AG_WidgetDraw(tb->hBar); }
	if (tb->vBar != NULL) { AG_WidgetDraw(tb->vBar); }
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Textbox *tbox = obj;
	AG_SizeReq rEd;
	int wLbl, hLbl;

	AG_WidgetSizeReq(tbox->ed, &rEd);

	r->w = tbox->boxPadX*2 + rEd.w;
	r->h = tbox->boxPadY*2 + rEd.h;

	if (tbox->labelText != NULL) {
		if (tbox->label != -1) {
			wLbl = WSURFACE(tbox,tbox->label)->w;
			hLbl = WSURFACE(tbox,tbox->label)->h;
		} else {
			AG_TextSize(tbox->labelText, &wLbl, &hLbl);
		}
		r->w += tbox->lblPadL + wLbl + tbox->lblPadR;
	}
	r->h = MAX(r->h, agTextFontLineSkip);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Textbox *tbox = obj;
	int boxPadW = tbox->boxPadX*2;
	int boxPadH = tbox->boxPadY*2;
	int lblPadW = tbox->lblPadL + tbox->lblPadR;
	int wBar = 0, hBar = 0;
	AG_SizeAlloc aChld;
	AG_SizeReq r;
	int d;

	tbox->wLbl = 0;
	tbox->hLbl = 0;

	if (tbox->labelText == NULL && (a->w < boxPadW || a->h < boxPadH))
		return (-1);
	if (a->w < boxPadW + lblPadW || a->h < boxPadH)
		return (-1);

	if (tbox->label != -1) {
		tbox->wLbl = WSURFACE(tbox,tbox->label)->w;
		tbox->hLbl = WSURFACE(tbox,tbox->label)->h;
	} else {
		AG_TextSize(tbox->labelText, &tbox->wLbl, &tbox->hLbl);
	}
	if (a->w < (boxPadW+lblPadW) + tbox->wLbl+tbox->ed->wPre) {
		tbox->wLbl = a->w - (boxPadW-lblPadW) - tbox->ed->wPre;
		if (tbox->wLbl <= 0) {
			if (a->w > boxPadW+lblPadW) {
				tbox->wLbl = 0;
			} else {
				return (-1);
			}
		}
	}
	if (tbox->flags & AG_TEXTBOX_MULTILINE) {
		AG_WidgetSizeReq(tbox->hBar, &r);
		d = MIN(r.h, a->h);
		aChld.x = 0;
		aChld.y = a->h - d;
		aChld.w = a->w - d + 1;
		aChld.h = d;
		AG_WidgetSizeAlloc(tbox->hBar, &aChld);
		if (AG_ScrollbarVisible(tbox->hBar))
			hBar = aChld.h;
		
		AG_WidgetSizeReq(tbox->vBar, &r);
		d = MIN(r.w, a->w);
		aChld.x = a->w - d;
		aChld.y = 0;
		aChld.w = d;
		aChld.h = a->h - d + 1;
		AG_WidgetSizeAlloc(tbox->vBar, &aChld);
		if (AG_ScrollbarVisible(tbox->vBar))
			wBar = aChld.w;
	}
	aChld.x = lblPadW + tbox->wLbl + tbox->boxPadX;
	aChld.y = tbox->boxPadY;
	aChld.w = a->w - boxPadW - aChld.x - wBar;
	aChld.h = a->h - boxPadH - hBar;
	AG_WidgetSizeAlloc(tbox->ed, &aChld);
	return (0);
}

/* Set the text from a format string. */
void
AG_TextboxPrintf(AG_Textbox *tbox, const char *fmt, ...)
{
	AG_WidgetBinding *stringb;
	va_list args;
	char *text;

	AG_ObjectLock(tbox->ed);
	stringb = AG_WidgetGetBinding(tbox->ed, "string", &text);
	if (fmt != NULL && fmt[0] != '\0') {
		va_start(args, fmt);
		Vsnprintf(text, stringb->data.size, fmt, args);
		va_end(args);
		tbox->ed->pos = AG_LengthUTF8(text);
	} else {
		text[0] = '\0';
		tbox->ed->pos = 0;
	}
	AG_TextboxBufferChanged(tbox);
	AG_WidgetUnlockBinding(stringb);
	AG_ObjectUnlock(tbox->ed);
}

void
AG_TextboxSetLabel(AG_Textbox *tbox, const char *fmt, ...)
{
	va_list ap;
	
	AG_ObjectLock(tbox);

	va_start(ap, fmt);
	Free(tbox->labelText);
	Vasprintf(&tbox->labelText, fmt, ap);
	va_end(ap);

	if (tbox->label != -1) {
		AG_WidgetUnmapSurface(tbox, tbox->label);
		tbox->label = -1;
	}
	AG_ObjectUnlock(tbox);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	AG_ForwardEvent(NULL, tbox->ed, event);
}

static void
Disabled(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	AG_WidgetDisable(tbox->ed);
}

static void
Enabled(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	AG_WidgetEnable(tbox->ed);
}

#ifdef DEBUG
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
GainFocus(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	AG_WidgetFocus(tb->ed);
}

static void
Init(void *obj)
{
	AG_Textbox *tbox = obj;

	tbox->ed = AG_EditableNew(tbox, 0);

	WIDGET(tbox)->flags |= AG_WIDGET_FOCUSABLE;

	tbox->boxPadX = 2;
	tbox->boxPadY = 2;
	tbox->lblPadL = 2;
	tbox->lblPadR = 2;
	tbox->wLbl = 0;
	tbox->flags = 0;
	tbox->label = -1;
	tbox->labelText = NULL;
	tbox->hBar = NULL;
	tbox->vBar = NULL;

	AG_SetEvent(tbox, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(tbox, "widget-disabled", Disabled, NULL);
	AG_SetEvent(tbox, "widget-enabled", Enabled, NULL);
#ifdef DEBUG
	AG_SetEvent(tbox, "widget-bound", Bound, NULL);
#endif
	AG_SetEvent(tbox, "widget-gainfocus", GainFocus, NULL);
	
	AG_SetEvent(tbox->ed, "editable-prechg", EditablePreChg, "%p", tbox);
	AG_SetEvent(tbox->ed, "editable-postchg", EditablePostChg, "%p", tbox);
	AG_SetEvent(tbox->ed, "editable-return", EditableReturn, "%p", tbox);
}

AG_WidgetClass agTextboxClass = {
	{
		"Agar(Widget:Textbox)",
		sizeof(AG_Textbox),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

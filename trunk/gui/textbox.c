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
 * Single or multi-line text input widget. This is a simple subclass of
 * AG_Editable(3) adding a built-in label, pixel padding and scrollbars.
 */

#include "opengl.h"

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
		AG_BindInt(tb->hBar, "value", &tb->ed->x);
		AG_BindInt(tb->vBar, "value", &tb->ed->y);
		AG_BindInt(tb->hBar, "max", &tb->ed->xMax);
		AG_BindInt(tb->vBar, "max", &tb->ed->yMax);
		AG_BindInt(tb->hBar, "visible", &WIDTH(tb->ed));
		AG_BindInt(tb->vBar, "visible", &tb->ed->yVis);
		AG_SetEvent(tb->hBar, "scrollbar-drag-begin", BeginScrollbarDrag, "%p", tb);
		AG_SetEvent(tb->vBar, "scrollbar-drag-begin", BeginScrollbarDrag, "%p", tb);
		AG_SetEvent(tb->hBar, "scrollbar-drag-end", EndScrollbarDrag, "%p", tb);
		AG_SetEvent(tb->vBar, "scrollbar-drag-end", EndScrollbarDrag, "%p", tb);
		AG_TextboxSizeHintLines(tb, 4);
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

	STYLE(tb)->TextboxBackground(tb, tb->r, (tb->flags & AG_TEXTBOX_COMBO));

	if (tb->labelText != NULL && tb->label == -1) {
		AG_PushTextState();
		AG_TextColor(TEXTBOX_TXT_COLOR);
		tb->label = AG_WidgetMapSurface(tb,
		    AG_TextRender(tb->labelText));
		AG_PopTextState();
	}
	
	if (tb->label != -1) {
		AG_Surface *lblSu = WSURFACE(tb,tb->label);
	
		AG_PushClipRect(tb, tb->rLbl);
		AG_WidgetBlitSurface(tb, tb->label,
		    tb->lblPadL,
		    HEIGHT(tb)/2 - lblSu->h/2);
		AG_PopClipRect();
	}

	AG_PushClipRect(tb, tb->r);

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

	AG_PopClipRect();
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Textbox *tb = obj;
	AG_SizeReq rEd;
	int wLbl, hLbl;

	AG_WidgetSizeReq(tb->ed, &rEd);

	r->w = tb->boxPadX*2 + rEd.w;
	r->h = tb->boxPadY*2 + rEd.h;

	if (tb->labelText != NULL) {
		if (tb->label != -1) {
			wLbl = WSURFACE(tb,tb->label)->w;
			hLbl = WSURFACE(tb,tb->label)->h;
		} else {
			AG_TextSize(tb->labelText, &wLbl, &hLbl);
		}
		r->w += tb->lblPadL + wLbl + tb->lblPadR;
	}
	r->h = MAX(r->h, agTextFontLineSkip);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Textbox *tb = obj;
	int boxPadW = tb->boxPadX*2;
	int boxPadH = tb->boxPadY*2;
	int lblPadW = tb->lblPadL + tb->lblPadR;
	int wBar = 0, hBar = 0;
	AG_SizeAlloc aChld;
	AG_SizeReq r;
	int d;

	tb->wLbl = 0;
	tb->hLbl = 0;

	if (tb->labelText == NULL && (a->w < boxPadW || a->h < boxPadH))
		return (-1);
	if (a->w < boxPadW + lblPadW || a->h < boxPadH)
		return (-1);

	if (tb->label != -1) {
		tb->wLbl = WSURFACE(tb,tb->label)->w;
		tb->hLbl = WSURFACE(tb,tb->label)->h;
	} else {
		AG_TextSize(tb->labelText, &tb->wLbl, &tb->hLbl);
	}
	if (a->w < (boxPadW+lblPadW) + tb->wLbl+tb->ed->wPre) {
		tb->wLbl = a->w - (boxPadW-lblPadW) - tb->ed->wPre;
		if (tb->wLbl <= 0) {
			if (a->w > boxPadW+lblPadW) {
				tb->wLbl = 0;
			} else {
				return (-1);
			}
		}
	}
	
	if (tb->flags & AG_TEXTBOX_MULTILINE) {
		AG_WidgetSizeReq(tb->hBar, &r);
		d = MIN(r.h, a->h);
		aChld.x = 0;
		aChld.y = a->h - d;
		aChld.w = a->w - d + 1;
		aChld.h = d;
		AG_WidgetSizeAlloc(tb->hBar, &aChld);
		if (AG_ScrollbarVisible(tb->hBar))
			hBar = aChld.h;
		
		AG_WidgetSizeReq(tb->vBar, &r);
		d = MIN(r.w, a->w);
		aChld.x = a->w - d;
		aChld.y = 0;
		aChld.w = d;
		aChld.h = a->h - d + 1;
		AG_WidgetSizeAlloc(tb->vBar, &aChld);
		if (AG_ScrollbarVisible(tb->vBar))
			wBar = aChld.w;
		
		tb->r.x = 0;
		tb->r.y = 0;
		tb->r.w = a->w;
		tb->r.h = a->h;
	} else {
		tb->r.x = tb->wLbl + tb->lblPadL + tb->lblPadR;
		tb->r.y = 0;
		tb->r.w = a->w - tb->r.x;
		tb->r.h = a->h;
	}
	
	tb->rLbl = AG_RECT(0, 0, tb->wLbl, a->h);
	
	aChld.x = lblPadW + tb->wLbl + tb->boxPadX;
	aChld.y = tb->boxPadY;
	aChld.w = a->w - boxPadW - aChld.x - wBar;
	aChld.h = a->h - boxPadH - hBar;
	AG_WidgetSizeAlloc(tb->ed, &aChld);

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
}

void
AG_TextboxSetLabel(AG_Textbox *tb, const char *fmt, ...)
{
	va_list ap;
	
	AG_ObjectLock(tb);

	va_start(ap, fmt);
	Free(tb->labelText);
	Vasprintf(&tb->labelText, fmt, ap);
	va_end(ap);

	if (tb->label != -1) {
		AG_WidgetUnmapSurface(tb, tb->label);
		tb->label = -1;
	}
	AG_ObjectUnlock(tb);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
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

	tb->ed = AG_EditableNew(tb, 0);

	tb->boxPadX = 2;
	tb->boxPadY = 2;
	tb->lblPadL = 2;
	tb->lblPadR = 2;
	tb->wLbl = 0;
	tb->flags = 0;
	tb->label = -1;
	tb->labelText = NULL;
	tb->hBar = NULL;
	tb->vBar = NULL;
	tb->r = AG_RECT(0,0,0,0);
	tb->rLbl = AG_RECT(0,0,0,0);

	AG_SetEvent(tb, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(tb, "widget-disabled", Disabled, NULL);
	AG_SetEvent(tb, "widget-enabled", Enabled, NULL);
#ifdef AG_DEBUG
	AG_SetEvent(tb, "bound", Bound, NULL);
#endif
	AG_SetEvent(tb->ed, "editable-prechg", EditablePreChg, "%p", tb);
	AG_SetEvent(tb->ed, "editable-postchg", EditablePostChg, "%p", tb);
	AG_SetEvent(tb->ed, "editable-return", EditableReturn, "%p", tb);
	
	AG_WidgetForwardFocus(tb, tb->ed);
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

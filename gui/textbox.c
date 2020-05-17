/*
 * Copyright (c) 2002-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Text editor widget.
 *
 * It embeds an AG_Editable(3) field along with an optional text label.
 * In MULTILINE mode it also embeds horizontal & vertical scrollbars.
 * Direct buffer access routines are available through the AG_Editable(3) API.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/ttf.h>
#include <agar/gui/textbox.h>
#include <agar/gui/text.h>
#include <agar/gui/keymap.h>
#include <agar/gui/primitive.h>
#include <agar/gui/window.h>

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

static void EditableReturn(AG_Event *_Nonnull);

AG_Textbox *
AG_TextboxNew(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Textbox *tb;
	char *s;
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
		tb = AG_TextboxNewS(parent, flags, s);
		free(s);
	} else {
		tb = AG_TextboxNewS(parent, flags, NULL);
	}
	return (tb);
}

AG_Textbox *
AG_TextboxNewS(void *parent, Uint flags, const char *label)
{
	AG_Textbox *tb;
	AG_Scrollbar *sb;

	tb = Malloc(sizeof(AG_Textbox));
	AG_ObjectInit(tb, &agTextboxClass);

	if (flags & AG_TEXTBOX_HFILL)         WIDGET(tb)->flags |= AG_WIDGET_HFILL;
	if (flags & AG_TEXTBOX_VFILL)         WIDGET(tb)->flags |= AG_WIDGET_VFILL;
	if (flags & AG_TEXTBOX_READONLY)      tb->ed->flags |= AG_EDITABLE_READONLY;
	if (flags & AG_TEXTBOX_PASSWORD)      tb->ed->flags |= AG_EDITABLE_PASSWORD;
	if (flags & AG_TEXTBOX_UPPERCASE)     tb->ed->flags |= AG_EDITABLE_UPPERCASE;
	if (flags & AG_TEXTBOX_LOWERCASE)     tb->ed->flags |= AG_EDITABLE_LOWERCASE;
	if (flags & AG_TEXTBOX_INT_ONLY)      tb->ed->flags |= AG_EDITABLE_INT_ONLY;
	if (flags & AG_TEXTBOX_FLT_ONLY)      tb->ed->flags |= AG_EDITABLE_FLT_ONLY;
	if (flags & AG_TEXTBOX_ABANDON_FOCUS) tb->ed->flags |= AG_EDITABLE_ABANDON_FOCUS;
	if (flags & AG_TEXTBOX_NO_KILL_YANK)  tb->ed->flags |= AG_EDITABLE_NO_KILL_YANK;
	if (flags & AG_TEXTBOX_NO_ALT_LATIN1) tb->ed->flags |= AG_EDITABLE_NO_ALT_LATIN1;
	if (flags & AG_TEXTBOX_NOPOPUP)       tb->ed->flags |= AG_EDITABLE_NOPOPUP;
	if (flags & AG_TEXTBOX_MULTILINGUAL)  tb->ed->flags |= AG_EDITABLE_MULTILINGUAL;

	if (flags & AG_TEXTBOX_CATCH_TAB) {
		WIDGET(tb)->flags |= AG_WIDGET_CATCH_TAB;
		WIDGET(tb->ed)->flags |= AG_WIDGET_CATCH_TAB;
	}
	if (flags & AG_TEXTBOX_MULTILINE) {
		tb->ed->flags |= AG_EDITABLE_MULTILINE;

		sb = tb->vBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_VERT,
		                                    AG_SCROLLBAR_EXCL);
		AG_SetInt(sb,  "min",     0);
		AG_BindInt(sb, "max",     &tb->ed->yMax);
		AG_BindInt(sb, "visible", &tb->ed->yVis);
		AG_BindInt(sb, "value",   &tb->ed->y);

		if ((flags & AG_TEXTBOX_WORDWRAP) == 0) {
			sb = tb->hBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_HORIZ,
			                                    AG_SCROLLBAR_EXCL);
			AG_SetInt(sb,  "min",     0);
			AG_BindInt(sb, "max",     &tb->ed->xMax);
			AG_BindInt(sb, "visible", &WIDTH(tb->ed));
			AG_BindInt(sb, "value",   &tb->ed->x);
		}
	}
	if (flags & AG_TEXTBOX_RETURN_BUTTON) {
		tb->btnRet = AG_ButtonNewInt(tb, 0,
		    " \xe2\x8f\x8e  ",                             /* U+23CE */
		    &tb->ed->returnHeld);

		AG_SetEvent(tb->btnRet, "button-pushed", EditableReturn, "%p", tb);
	}
	if (flags & AG_TEXTBOX_EXCL)
		AG_EditableSetExcl(tb->ed, 1);

	if (flags & AG_TEXTBOX_WORDWRAP)
		AG_EditableSetWordWrap(tb->ed, 1);

	tb->flags |= flags;

	if (label != NULL)
		tb->label = TryStrdup(label);

	AG_ObjectAttach(parent, tb);
	return (tb);
}

/* Pre-size the textbox to accomodate given text. */
void
AG_TextboxSizeHint(AG_Textbox *tb, const char *text)
{
	AG_EditableSizeHint(tb->ed, text);
}

/* Pre-size the textbox to given size in pixels. */
void
AG_TextboxSizeHintPixels(AG_Textbox *tb, Uint w, Uint h)
{
	AG_EditableSizeHintPixels(tb->ed, w,h);
}

/* Pre-size the textbox to accomodate a given number of lines of text. */
void
AG_TextboxSizeHintLines(AG_Textbox *tb, Uint nLines)
{
	AG_EditableSizeHintLines(tb->ed, nLines);
}

/* Clear or set the PASSWORD mode. */
void
AG_TextboxSetPassword(AG_Textbox *tb, int flag)
{
	AG_EditableSetPassword(tb->ed, flag);
}

/* Clear or set the EXCL flag. */
void
AG_TextboxSetExcl(AG_Textbox *tb, int flag)
{
	AG_EditableSetExcl(tb->ed, flag);
}

/* Clear or set the FLT_ONLY option. */
void
AG_TextboxSetFltOnly(AG_Textbox *tb, int flag)
{
	AG_EditableSetFltOnly(tb->ed, flag);
}

/* Clear or set the INT_ONLY option. */
void
AG_TextboxSetIntOnly(AG_Textbox *tb, int flag)
{
	AG_EditableSetIntOnly(tb->ed, flag);
}

/* Toggle word wrapping */
void
AG_TextboxSetWordWrap(AG_Textbox *tb, int flag)
{
	AG_Window *winParent;

	AG_OBJECT_ISA(tb, "AG_Widget:AG_Textbox:*");
	AG_ObjectLock(tb);

	AG_EditableSetWordWrap(tb->ed, flag);

	if (tb->ed->flags & AG_EDITABLE_MULTILINE) {
		AG_Scrollbar *bar;

		if ((bar = tb->hBar) != NULL) {
			tb->hBar = NULL;
			AG_ObjectDetach(bar);
			AG_ObjectDestroy(bar);
		}
		if (!flag) {
			bar = tb->hBar = AG_ScrollbarNew(tb, AG_SCROLLBAR_HORIZ,
			                                 AG_SCROLLBAR_EXCL);
			AG_SetInt(bar,  "min",     0);
			AG_BindInt(bar, "max",     &tb->ed->xMax);
			AG_BindInt(bar, "visible", &WIDTH(tb->ed));
			AG_BindInt(bar, "value",   &tb->ed->x);

			AG_TextboxSizeHintLines(tb, 4);
		}
	}

	if ((winParent = WIDGET(tb)->window) != NULL)
		AG_WindowUpdate(winParent);

	AG_ObjectUnlock(tb);
}

/* Return the current cursor position. */
int
AG_TextboxGetCursorPos(const AG_Textbox *tb)
{
	return AG_EditableGetCursorPos(tb->ed);
}

#ifdef AG_UNICODE
/* Bind to a fixed-size UTF-8 encoded text buffer. */
void
AG_TextboxBindUTF8(AG_Textbox *tb, char *buf, AG_Size bufSize)
{
	AG_EditableBindUTF8(tb->ed, buf, bufSize);
}

/* Bind to a fixed-size buffer of specified encoding. */
void
AG_TextboxBindEncoded(AG_Textbox *tb, const char *encoding, char *buf,
    AG_Size bufSize)
{
	AG_EditableBindEncoded(tb->ed, encoding, buf, bufSize);
}

/* Bind to a multilingual text element. */
void
AG_TextboxBindText(AG_Textbox *tb, AG_TextElement *txt)
{
	AG_EditableBindText(tb->ed, txt);
}

/* Set the active language for a multilingual text element. */
void
AG_TextboxSetLang(AG_Textbox *tb, enum ag_language lang)
{
	AG_EditableSetLang(tb->ed, lang);
}
#endif /* AG_UNICODE */

/* Bind to a fixed-size buffer of text in US-ASCII encoding. */
void
AG_TextboxBindASCII(AG_Textbox *tb, char *buf, AG_Size bufSize)
{
	AG_EditableBindASCII(tb->ed, buf, bufSize);
}

/*
 * Set up an autocomplete context. Function fn will be called to populate
 * the list of autocomplete candidates.
 * 
 * If fn is NULL, disable autocomplete (closing any open windows).
 * If an autocomplete context already exists, update its function pointer
 * and arguments (cancelling any running timers).
 */
void
AG_TextboxAutocomplete(AG_Textbox *tb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(tb->ed);
	AG_EditableAutocomplete(tb->ed, fn, NULL);

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(tb->ed->complete->fn, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(tb->ed);
}

/* Set the "place holder" text to display when the textbox is empty. */
void
AG_TextboxSetPlaceholder(AG_Textbox *tb, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_SetString(tb->ed, "placeholder", s);

	free(s);
}

/* Set the "place holder" text to display when the textbox is empty. */
void
AG_TextboxSetPlaceholderS(AG_Textbox *tb, const char *s)
{
	AG_SetString(tb->ed, "placeholder", s);
}

/* Replace the contents of the text buffer with the given string. */
void
AG_TextboxSetString(AG_Textbox *tb, const char *s)
{
	AG_EditableSetString(tb->ed, s);
}

/* Clear the text buffer. */
void
AG_TextboxClearString(AG_Textbox *tb)
{
	AG_EditableSetString(tb->ed, NULL);
}

/* Return a copy of the buffer contents. */
char *
AG_TextboxDupString(AG_Textbox *tb)
{
	return AG_EditableDupString(tb->ed);
}

/* Copy the buffer contents to a fixed-size buffer. */
AG_Size
AG_TextboxCopyString(AG_Textbox *tb, char *dst, AG_Size dst_size)
{
	return AG_EditableCopyString(tb->ed, dst, dst_size);
}

/* Convert the buffer contents to an integer and return it. */
int
AG_TextboxInt(AG_Textbox *tb)
{
	return AG_EditableInt(tb->ed);
}

/* Convert the buffer contents to a float and return it. */
float
AG_TextboxFloat(AG_Textbox *tb)
{
	return AG_EditableFlt(tb->ed);
}

/* Convert the buffer contents to a double and return it. */
double
AG_TextboxDouble(AG_Textbox *tb)
{
	return AG_EditableDbl(tb->ed);
}

static __inline__ void
RenderLabel(AG_Textbox *_Nonnull tb)
{
	if (tb->surfaceLbl == -1) {
		const AG_Font *font = WFONT(tb);

		AG_TextFontLookup(OBJECT(font)->name,         /* TODO style */
		    font->spec.size*0.95f,
		    font->flags);

		tb->surfaceLbl = AG_WidgetMapSurface(tb,
		    AG_TextRender(tb->label));
	}
}

static void
Draw(void *_Nonnull p)
{
	AG_Textbox *tb = p;
	const int isUndersize = (tb->flags & AG_TEXTBOX_UNDERSIZE);

	if (!isUndersize && (tb->flags & AG_TEXTBOX_NO_SHADING) == 0) {
		if ((tb->flags & AG_TEXTBOX_COMBO)) {
			AG_DrawBoxRaised(tb, &tb->r, &WCOLOR(tb, FG_COLOR));
		} else {
			AG_DrawBoxSunk(tb, &tb->r, &WCOLOR(tb, BG_COLOR));
		}
	}
	if (tb->label != NULL &&
	    tb->label[0] != '\0') {
		if (isUndersize)
			AG_PushClipRect(tb, &WIDGET(tb)->r);

		RenderLabel(tb);

		AG_PushBlendingMode(tb, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

		AG_WidgetBlitSurface(tb, tb->surfaceLbl,
		    WIDGET(tb)->paddingLeft,
		    WIDGET(tb)->paddingTop);

		AG_PopBlendingMode(tb);

		if (isUndersize)
			AG_PopClipRect(tb);
	}

	if (!isUndersize)
		AG_WidgetDraw(tb->ed);

	if (tb->btnRet) { AG_WidgetDraw(tb->btnRet); }
	if (tb->hBar) { AG_WidgetDraw(tb->hBar); }
	if (tb->vBar) { AG_WidgetDraw(tb->vBar); }
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Textbox *tb = obj;
	AG_SizeReq rEd;

	r->w = WIDGET(tb)->paddingLeft + WIDGET(tb)->paddingRight;
	r->h = WIDGET(tb)->paddingTop + WIDGET(tb)->paddingBottom;

	AG_WidgetSizeReq(tb->ed, &rEd);                   /* Editable field */
	r->w += rEd.w;
	r->h += rEd.h;

	if (tb->label != NULL && tb->label[0] != '\0') {
		RenderLabel(tb);
		r->w += WIDGET(tb)->spacingHoriz;
		r->w += WSURFACE(tb,tb->surfaceLbl)->w;
		r->h = MAX(r->h, WSURFACE(tb,tb->surfaceLbl)->h);
	} else {
		r->h = MAX(r->h, WFONT(tb)->lineskip);
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Textbox *tb = obj;
	AG_Editable *ed = tb->ed;
	int wBar=0, hBar=0, wBarSz=0, hBarSz=0, wBtn, wLbl;
	AG_SizeAlloc aEd, aSb, aBtn;
	AG_SizeReq r;

	if (a->w < WIDGET(tb)->paddingLeft + WIDGET(tb)->paddingRight ||
	   (a->h < WIDGET(tb)->paddingTop  + WIDGET(tb)->paddingBottom))
		return (-1);

	if (tb->label) {
		RenderLabel(tb);
		wLbl = WSURFACE(tb,tb->surfaceLbl)->w;
	} else {
		wLbl = 0;
	}

	if (tb->flags & AG_TEXTBOX_MULTILINE) {
		if (tb->hBar) {
			AG_WidgetSizeReq(tb->hBar, &r);
			hBarSz = MIN(r.h, a->h);
			aSb.x = 0;
			aSb.y = a->h - hBarSz;
			aSb.w = a->w - hBarSz + 1;
			aSb.h = hBarSz;
			AG_WidgetSizeAlloc(tb->hBar, &aSb);
			hBar = aSb.h;
		}
		if (tb->vBar) {
			AG_WidgetSizeReq(tb->vBar, &r);
			wBarSz = MIN(r.w, a->w);
			aSb.x = a->w - wBarSz;
			aSb.y = 0;
			aSb.w = wBarSz;
			aSb.h = a->h - hBarSz + 1;
			AG_WidgetSizeAlloc(tb->vBar, &aSb);
			wBar = aSb.w;
		}

		tb->r.x = 0;
		tb->r.y = 0;
		tb->r.w = a->w - WIDGET(tb)->paddingRight;
		tb->r.h = a->h;
	} else {
		tb->r.x = WIDGET(tb)->paddingLeft;
		if (tb->label) {
			tb->r.x += wLbl + WIDGET(tb)->spacingHoriz;
		}
		tb->r.y = 0;
		tb->r.w = a->w - tb->r.x - WIDGET(tb)->paddingRight;
		tb->r.h = a->h;
	}
	
	if (tb->btnRet) {
		AG_WidgetSizeReq(tb->btnRet, &r);
		wBtn = r.w;
	} else {
		wBtn = 0;
	}

	aEd.x = WIDGET(tb)->paddingLeft;
	if (tb->label) {
		aEd.x += wLbl + WIDGET(tb)->spacingHoriz;
	}
	aEd.y = WIDGET(tb)->paddingTop;
	aEd.w = a->w - aEd.x - wBtn - wBar - WIDGET(tb)->paddingRight - 1;
	if (tb->btnRet) {
		aEd.w -= WIDGET(tb)->spacingHoriz;
	}
	aEd.h = a->h - hBar - WIDGET(tb)->paddingTop -
	        WIDGET(tb)->paddingBottom;

	if (aEd.w < 0 || aEd.h < 0) {
		aEd.w = 0;
		aEd.h = 0;
		tb->flags |= AG_TEXTBOX_UNDERSIZE;
	} else {
		tb->flags &= ~(AG_TEXTBOX_UNDERSIZE);
	}
	AG_WidgetSizeAlloc(ed, &aEd);

	if (tb->btnRet) {
		aBtn.x = aEd.x + aEd.w;
		aBtn.y = WIDGET(tb)->paddingTop;
		aBtn.w = wBtn;
		aBtn.h = aEd.h;
		AG_WidgetSizeAlloc(tb->btnRet, &aBtn);
	}

	if (ed->x + WIDTH(ed) >= ed->xMax) {
		ed->x = MAX(0, ed->xMax - WIDTH(ed));
	}
	if (ed->y + ed->yVis > ed->yMax) {
		ed->y = MAX(0, ed->yMax - ed->yVis);
	}
	return (0);
}

/* Set the textbox label (format string). */
void
AG_TextboxSetLabel(AG_Textbox *tb, const char *fmt, ...)
{
	va_list ap;

	AG_OBJECT_ISA(tb, "AG_Widget:AG_Textbox:*");
	AG_ObjectLock(tb);

	if (tb->surfaceLbl != -1) {
		AG_WidgetUnmapSurface(tb, tb->surfaceLbl);
		tb->surfaceLbl = -1;
	}
	Free(tb->label);
	va_start(ap, fmt);
	Vasprintf(&tb->label, fmt, ap);
	va_end(ap);

	AG_Redraw(tb);
	AG_ObjectUnlock(tb);
}

/* Set the textbox label (C string). */
void
AG_TextboxSetLabelS(AG_Textbox *tb, const char *s)
{
	AG_OBJECT_ISA(tb, "AG_Widget:AG_Textbox:*");
	AG_ObjectLock(tb);

	if (tb->surfaceLbl != -1) {
		AG_WidgetUnmapSurface(tb, tb->surfaceLbl);
		tb->surfaceLbl = -1;
	}
	Free(tb->label);
	tb->label = TryStrdup(s);

	AG_Redraw(tb);
	AG_ObjectUnlock(tb);
}

/* Set the text from a format string. */
void
AG_TextboxPrintf(AG_Textbox *tb, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_EditableSetString(tb->ed, s);

	free(s);
}

/*
 * Map pixel coordinates to a position in the text.
 * Return value is only valid as long as widget is locked.
 */
int
AG_TextboxMapPosition(AG_Textbox *tb, int x, int y, int *pos)
{
	AG_EditableBuffer *buf;
	int rv;

	if ((buf = AG_EditableGetBuffer(tb->ed)) == NULL) {
		return (-1);				/* XXX ambiguous */
	}
	rv = AG_EditableMapPosition(tb->ed, buf, x, y, pos);
	AG_EditableReleaseBuffer(tb->ed, buf);
	return (rv);
}

/* Move cursor as close as possible to specified pixel coordinates. */
void
AG_TextboxMoveCursor(AG_Textbox *tb, int x, int y)
{
	AG_EditableBuffer *buf;

	if ((buf = AG_EditableGetBuffer(tb->ed)) == NULL) {
		return;
	}
	AG_EditableMoveCursor(tb->ed, buf, x, y);
	AG_EditableReleaseBuffer(tb->ed, buf);
}

/* Move cursor to specified character position in text. */
void
AG_TextboxSetCursorPos(AG_Textbox *tb, int pos)
{
	AG_EditableBuffer *buf;

	if ((buf = AG_EditableGetBuffer(tb->ed)) == NULL) {
		return;
	}
	AG_EditableSetCursorPos(tb->ed, buf, pos);
	AG_EditableReleaseBuffer(tb->ed, buf);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	const int btn = AG_INT(1);
	const int mx = AG_INT(2);
	const int my = AG_INT(3);

	if (btn == AG_MOUSE_LEFT)
		AG_WidgetFocus(tb);

	if (mx >= tb->r.x &&
	    mx <= WIDTH(tb) - WIDGET(tb)->paddingRight &&
	    my >= WIDGET(tb)->paddingTop &&
	    my < HEIGHT(tb) - WIDGET(tb)->paddingBottom)
		AG_ForwardEvent(tb->ed, event);
}

static void
Disabled(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();

	AG_WidgetDisable(tb->ed);
}

static void
Enabled(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();

	AG_WidgetEnable(tb->ed);
}

static void
EditablePreChg(AG_Event *_Nonnull event)
{
	AG_PostEvent(AG_TEXTBOX_PTR(1), "textbox-prechg", NULL);
}

static void
EditablePostChg(AG_Event *_Nonnull event)
{
	AG_PostEvent(AG_TEXTBOX_PTR(1), "textbox-postchg", NULL);
}

static void
EditableReturn(AG_Event *_Nonnull event)
{
	AG_PostEvent(AG_TEXTBOX_PTR(1), "textbox-return", NULL);
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();

	if (tb->surfaceLbl != -1) {
		AG_WidgetUnmapSurface(tb, tb->surfaceLbl);
		tb->surfaceLbl = -1;
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Textbox *tb = obj;
	
	WIDGET(tb)->flags |= AG_WIDGET_USE_TEXT |
			     AG_WIDGET_USE_MOUSEOVER;

	tb->ed = AG_EditableNew(tb, 0);
	tb->flags = 0;
	tb->surfaceLbl = -1;
	tb->label = NULL;
	tb->hBar = NULL;
	tb->vBar = NULL;
	tb->r.x = 0;
	tb->r.y = 0;
	tb->r.w = 0;
	tb->r.h = 0;
	tb->text = tb->ed->text;
	tb->btnRet = NULL;

	AG_SetEvent(tb, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tb, "widget-disabled", Disabled, NULL);
	AG_SetEvent(tb, "widget-enabled", Enabled, NULL);
	AG_AddEvent(tb, "font-changed", OnFontChange, NULL);

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

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
 * Low-level single/multi-line text input widget. This is the base widget
 * used by AG_Textbox(3), editable cells in AG_Table(3), etc.
 */

#include <core/core.h>
#include <core/config.h>

#include <config/have_freetype.h>

#include "editable.h"
#include "text.h"
#include "window.h"

#include "ttf.h"
#include "keymap.h"
#include "primitive.h"
#include "cursors.h"

#ifdef AG_DEBUG
#include "numerical.h"
#include "separator.h"
#endif

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

AG_Editable *
AG_EditableNew(void *parent, Uint flags)
{
	AG_Editable *ed;

	ed = Malloc(sizeof(AG_Editable));
	AG_ObjectInit(ed, &agEditableClass);

	if (flags & AG_EDITABLE_HFILL)
		AG_ExpandHoriz(ed);
	if (flags & AG_EDITABLE_VFILL)
		AG_ExpandVert(ed);
	if (flags & AG_EDITABLE_CATCH_TAB)
		WIDGET(ed)->flags |= AG_WIDGET_CATCH_TAB;

	ed->flags |= flags;

	AG_EditableSetStatic(ed, (flags & AG_EDITABLE_STATIC));
	AG_ObjectAttach(parent, ed);
	return (ed);
}

/* Bind a UTF-8 text buffer to the widget. */
void
AG_EditableBindUTF8(AG_Editable *ed, char *buf, size_t bufSize)
{
	AG_ObjectLock(ed);
	AG_BindString(ed, "string", buf, bufSize);
	ed->encoding = AG_ENCODING_UTF8;
	AG_ObjectUnlock(ed);
}

/* Bind an ASCII text buffer to the widget. */
void
AG_EditableBindASCII(AG_Editable *ed, char *buf, size_t bufSize)
{
	AG_ObjectLock(ed);
	AG_BindString(ed, "string", buf, bufSize);
	ed->encoding = AG_ENCODING_ASCII;
	AG_ObjectUnlock(ed);
}

/* Bind a growable UTF-8 text buffer to the widget. */
void
AG_EditableBindAutoUTF8(AG_Editable *ed, char **buf, Uint *bufSize)
{
	AG_ObjectLock(ed);
	AG_BindPointer(ed, "buffer", (void **)buf);
	AG_BindUint(ed, "size", bufSize);
	AG_BindString(ed, "string", *buf, *bufSize);
	ed->encoding = AG_ENCODING_UTF8;
	AG_ObjectUnlock(ed);
}

/* Bind a growable ASCII text buffer to the widget. */
void
AG_EditableBindAutoASCII(AG_Editable *ed, char **buf, Uint *bufSize)
{
	AG_ObjectLock(ed);
	AG_BindPointer(ed, "buffer", (void **)buf);
	AG_BindUint(ed, "size", bufSize);
	AG_BindString(ed, "string", *buf, *bufSize);
	ed->encoding = AG_ENCODING_ASCII;
	AG_ObjectUnlock(ed);
}

/* Enable or disable password entry mode. */
void
AG_EditableSetPassword(AG_Editable *ed, int enable)
{
	AG_ObjectLock(ed);
	AG_SETFLAGS(ed->flags, AG_EDITABLE_PASSWORD, enable);
	AG_ObjectUnlock(ed);
}

/* Enable or disable word wrapping. */
void
AG_EditableSetWordWrap(AG_Editable *ed, int enable)
{
	AG_ObjectLock(ed);
	AG_SETFLAGS(ed->flags, AG_EDITABLE_WORDWRAP, enable);
	AG_ObjectUnlock(ed);
}

/* Specify whether the bound string is modified exclusively by the widget. */
void
AG_EditableSetStatic(AG_Editable *ed, int enable)
{
	AG_ObjectLock(ed);

	Free(ed->ucsBuf);
	ed->ucsBuf = NULL;
	ed->ucsLen = 0;

	if (enable) {
		ed->flags |= AG_EDITABLE_STATIC;
		AG_RedrawOnTick(ed, -1);
	} else {
		ed->flags &= ~(AG_EDITABLE_STATIC);
		AG_RedrawOnTick(ed, 1000);
	}

	AG_ObjectUnlock(ed);
}

/* Toggle floating-point only input */
void
AG_EditableSetFltOnly(AG_Editable *ed, int enable)
{
	AG_ObjectLock(ed);
	if (enable) {
		ed->flags |= AG_EDITABLE_FLT_ONLY;
		ed->flags &= ~(AG_EDITABLE_INT_ONLY);
	} else {
		ed->flags &= ~(AG_EDITABLE_FLT_ONLY);
	}
	AG_ObjectUnlock(ed);
}

/* Toggle integer only input */
void
AG_EditableSetIntOnly(AG_Editable *ed, int enable)
{
	AG_ObjectLock(ed);
	if (enable) {
		ed->flags |= AG_EDITABLE_INT_ONLY;
		ed->flags &= ~(AG_EDITABLE_FLT_ONLY);
	} else {
		ed->flags &= ~(AG_EDITABLE_INT_ONLY);
	}
	AG_ObjectUnlock(ed);
}

/* Set alternate font */
void
AG_EditableSetFont(AG_Editable *ed, AG_Font *font)
{
	AG_ObjectLock(ed);
	ed->font = font;
	AG_ObjectUnlock(ed);
}

/* Evaluate if a key is acceptable in integer-only mode. */
static int
KeyIsIntOnly(AG_KeySym ks)
{
	return (ks == AG_KEY_MINUS ||
	        ks == AG_KEY_PLUS ||
		isdigit((int)ks));
}

/* Evaluate if a key is acceptable in float-only mode. */
static int
KeyIsFltOnly(AG_KeySym ks, Uint32 unicode)
{
	return (ks == AG_KEY_PLUS ||
	        ks == AG_KEY_MINUS ||
	        ks == AG_KEY_PERIOD ||
	        ks == AG_KEY_E ||
	        ks == AG_KEY_I ||
	        ks == AG_KEY_N ||
	        ks == AG_KEY_F ||
	        ks == AG_KEY_A ||
	        unicode == 0x221e ||		/* Infinity */
	        isdigit((int)ks));
}

/*
 * Return a locked handle to the "string" binding, and perform
 * UTF-8 -> UCS-4 conversion if necessary.
 */
static __inline__ AG_Variable *
GetStringUCS4(AG_Editable *ed, char **s, Uint32 **ucs, size_t *len)
{
	AG_Variable *stringb;

	stringb = AG_GetVariable(ed, "string", s);
	if (ed->flags & AG_EDITABLE_STATIC) {
		if (ed->ucsBuf == NULL) {
			ed->ucsBuf = AG_ImportUnicode(AG_UNICODE_FROM_UTF8,
			    *s, stringb->info.size);
			ed->ucsLen = AG_LengthUCS4(ed->ucsBuf);
		}
		*ucs = ed->ucsBuf;
		*len = ed->ucsLen;
	} else {
		*ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, *s,
		    stringb->info.size);
		*len = AG_LengthUCS4(*ucs);
	}
	return (stringb);
}
static __inline__ void
FreeStringUCS4(AG_Editable *ed, Uint32 *ucs)
{
	if (!(ed->flags & AG_EDITABLE_STATIC))
		Free(ucs);
}

/*
 * Process a keystroke. May be invoked from the repeat timeout routine or
 * the keydown handler. If we return 1, the current delay/repeat cycle will
 * be maintained, otherwise it will be cancelled.
 */
static int
ProcessKey(AG_Editable *ed, AG_KeySym ks, AG_KeyMod kmod, Uint32 unicode)
{
	AG_Variable *stringb;
	char *s;
	int i, rv = 0;
	size_t len;
	Uint32 *ucs;

	if (ks == AG_KEY_ESCAPE) {
		return (0);
	}
	if (ks == AG_KEY_RETURN &&
	   (ed->flags & AG_EDITABLE_MULTILINE) == 0)
		return (0);

	if (kmod == AG_KEYMOD_NONE &&
	    isascii((int)ks) &&
	    isprint((int)ks)) {
		if ((ed->flags & AG_EDITABLE_INT_ONLY) &&
		    !KeyIsIntOnly(ks)) {
			return (0);
		} else if ((ed->flags & AG_EDITABLE_FLT_ONLY) &&
		           !KeyIsFltOnly(ks, unicode)) {
			return (0);
		}
	}

	stringb = GetStringUCS4(ed, &s, &ucs, &len);

	if (ed->pos < 0) { ed->pos = 0; }
	if (ed->pos > (int)len) { ed->pos = (int)len; }
	for (i = 0; ; i++) {
		const struct ag_keycode_utf8 *kc = &agKeymapUTF8[i];
		
		if (kc->key != AG_KEY_LAST &&
		   (kc->key != ks || kc->func == NULL)) {
			continue;
		}
		if (kc->key == AG_KEY_LAST ||
		    kc->modmask == 0 || (kmod & kc->modmask)) {
			AG_PostEvent(NULL, ed, "editable-prechg", NULL);
			rv = kc->func(ed, ks, kmod, unicode, ucs, (int)len,
			    stringb->info.size);
			break;
		}
	}
	if (rv == 1) {
		if (ed->flags & AG_EDITABLE_STATIC) {
			/* Update the cached UCS-4 length. */
			ed->ucsLen = AG_LengthUCS4(ucs);
		}
		AG_ExportUnicode(AG_UNICODE_TO_UTF8, stringb->data.s, ucs,
		    stringb->info.size);
		ed->flags |= AG_EDITABLE_MARKPREF;
		AG_PostEvent(NULL, ed, "editable-postchg", NULL);
	}

	FreeStringUCS4(ed, ucs);
	AG_UnlockVariable(stringb);
	return (1);
}

static Uint32
RepeatTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Editable *ed = obj;

	if (ProcessKey(ed, ed->repeatKey, ed->repeatMod, ed->repeatUnicode)
	    == 0) {
		return (0);
	}
	AG_Redraw(ed);
	return (agKbdRepeat);
}

static Uint32
DelayTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Editable *ed = obj;

	AG_ScheduleTimeout(ed, &ed->toRepeat, agKbdRepeat);
	AG_DelTimeout(ed, &ed->toCursorBlink);
	ed->flags |= AG_EDITABLE_BLINK_ON;
	AG_Redraw(ed);
	return (0);
}

static Uint32
BlinkTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Editable *ed = obj;

	if ((ed->flags & AG_EDITABLE_CURSOR_MOVING) == 0) {
		AG_INVFLAGS(ed->flags, AG_EDITABLE_BLINK_ON);
	}
	AG_Redraw(ed);
	return (ival);
}

static void
GainedFocus(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();

	AG_LockTimeouts(ed);
	
	AG_DelTimeout(ed, &ed->toDelay);
	AG_DelTimeout(ed, &ed->toRepeat);
	AG_ScheduleTimeout(ed, &ed->toCursorBlink, agTextBlinkRate);
	ed->flags |= AG_EDITABLE_BLINK_ON;
	AG_UnlockTimeouts(ed);

	AG_Redraw(ed);
}

static void
LostFocus(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();

	AG_LockTimeouts(ed);
	AG_DelTimeout(ed, &ed->toDelay);
	AG_DelTimeout(ed, &ed->toRepeat);
	AG_DelTimeout(ed, &ed->toCursorBlink);
	ed->flags &= ~(AG_EDITABLE_BLINK_ON|AG_EDITABLE_CURSOR_MOVING);
	AG_UnlockTimeouts(ed);
	
	AG_Redraw(ed);
}

/* Evaluate word wrapping at given character. */
static __inline__ int
WrapAtChar(AG_Editable *ed, int x, Uint32 *s)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_Glyph *gl;
	Uint32 *t;
	int x2;

	if (!(ed->flags & AG_EDITABLE_WORDWRAP) ||
	    x == 0 || !isspace((int)*s)) {
		return (0);
	}
	for (t = &s[1], x2 = x;
	     *t != '\0';
	     t++) {
		gl = AG_TextRenderGlyph(drv, *t);
		x2 += gl->advance;
		if (isspace((int)*t) || *t == '\n') {
			if (x2 > WIDTH(ed)) {
				return (1);
			} else {
				break;
			}
		}
	}
	return (0);
}

/*
 * Map mouse coordinates to a position within the string.
 */
#define ON_LINE(my,y) \
	( ((my) >= (y) && (my) <= (y)+agTextFontLineSkip) || \
	  (!absflag && \
	   (((my) <= ed->y*agTextFontLineSkip && line == 0) || \
	    ((my) > (nLines*agTextFontLineSkip) && (line == nLines-1)))) )
#define ON_CHAR(mx,x,glyph) \
	((mx) >= (x) && (mx) <= (x)+(glyph)->advance)
int
AG_EditableMapPosition(AG_Editable *ed, int mx, int my, int *pos, int absflag)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_Variable *stringb;
	AG_Font *font;
	size_t len;
	char *s;
	Uint32 *ucs, ch;
	int i, x, y, line = 0;
	int nLines = 1;
	int yMouse;
	
	AG_ObjectLock(ed);

	yMouse = my + ed->y*agTextFontLineSkip;
	if (yMouse < 0) {
		AG_ObjectUnlock(ed);
		return (-1);
	}
	x = 0;
	y = 0;

	stringb = GetStringUCS4(ed, &s, &ucs, &len);

	if ((font = ed->font) == NULL &&
	    (font = AG_FetchFont(NULL, -1, -1)) == NULL)
		AG_FatalError("AG_Editable: %s", AG_GetError());

 	for (i = 0; i < len; i++) {
		Uint32 ch = ucs[i];

		if (WrapAtChar(ed, x, &ucs[i])) {
			x = 0;
			nLines++;
		}
		if (ch == '\n') {
			x = 0;
			nLines++;
		} else if (ch == '\t') {
			x += agTextTabWidth;
		} else {
			switch (font->type) {
#ifdef HAVE_FREETYPE
			case AG_FONT_VECTOR:
			{
				AG_TTFFont *ttf = font->ttf;
				AG_TTFGlyph *glyph;

				if (AG_TTFFindGlyph(ttf, ch,
				    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
					continue;
				}
				glyph = ttf->current;
				x += glyph->advance;
				break;
			}
#endif /* HAVE_FREETYPE */
			case AG_FONT_BITMAP:
			{
				AG_Glyph *gl;
			
				gl = AG_TextRenderGlyph(drv, ch);
				x += gl->su->w;
				break;
			}
			default:
				break;
			}
		}
	}

	x = 0;
	for (i = 0; i < len; i++) {
		ch = ucs[i];
		if (mx <= 0 && ON_LINE(yMouse,y)) {
			*pos = i;
			goto in;
		}
		if (WrapAtChar(ed, x, &ucs[i])) {
			if (ON_LINE(yMouse,y) && mx > x) {
				*pos = i;
				goto in;
			}
			y += agTextFontLineSkip;
			x = 0;
			line++;
		}
		if (ch == '\n') {
			if (ON_LINE(yMouse,y) && mx > x) {
				*pos = i;
				goto in;
			}
			y += agTextFontLineSkip;
			x = 0;
			line++;
			continue;
		} else if (ch == '\t') {
			if (ON_LINE(yMouse,y) &&
			    mx >= x && mx <= x+agTextTabWidth) {
				*pos = (mx < x + agTextTabWidth/2) ? i : i+1;
				goto in;
			}
			x += agTextTabWidth;
			continue;
		}
		
		switch (font->type) {
#ifdef HAVE_FREETYPE
		case AG_FONT_VECTOR:
		{
			AG_TTFFont *ttf = font->ttf;
			AG_TTFGlyph *glyph;

			if (AG_TTFFindGlyph(ttf, ch,
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
				continue;
			}
			glyph = ttf->current;

			if (ON_LINE(yMouse,y) && ON_CHAR(mx,x,glyph)) {
				*pos = (mx < x+glyph->advance/2) ? i : i+1;
				goto in;
			}
			x += glyph->advance;
			break;
		}
#endif /* HAVE_FREETYPE */
		case AG_FONT_BITMAP:
		{
			AG_Glyph *gl;
			
			gl = AG_TextRenderGlyph(drv, ch);
			if (ON_LINE(yMouse,y) && mx >= x && mx <= x+gl->su->w) {
				*pos = i;
				goto in;
			}
			x += gl->su->w;
			break;
		}
		default:
			AG_FatalError("AG_Editable: Unknown font format");
		}
	}
	FreeStringUCS4(ed, ucs);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	return (1);
in:
	FreeStringUCS4(ed, ucs);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	return (0);
}
#undef ON_LINE
#undef ON_CHAR

/* Move cursor to the given position in pixels. */
void
AG_EditableMoveCursor(AG_Editable *ed, int mx, int my, int absflag)
{
	AG_Variable *stringb;
	char *s;
	int rv;

	AG_ObjectLock(ed);
	rv = AG_EditableMapPosition(ed, mx, my, &ed->pos, absflag);
	if (rv == -1) {
		ed->pos = 0;
	} else if (rv == 1) {
		stringb = AG_GetVariable(ed, "string", &s);
		ed->pos = AG_LengthUTF8(s);
		AG_UnlockVariable(stringb);
	}
	AG_ObjectUnlock(ed);

	AG_Redraw(ed);
}

/* Return the last cursor position. */
int
AG_EditableGetCursorPos(AG_Editable *ed)
{
	int rv;

	AG_ObjectLock(ed);
	rv = ed->pos;
	AG_ObjectUnlock(ed);
	return (rv);
}

/* Set the cursor position (-1 = end of the string) with bounds checking. */
int
AG_EditableSetCursorPos(AG_Editable *ed, int pos)
{
	AG_Variable *stringb;
	size_t len;
	char *s;
	int rv;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &s);

	ed->pos = pos;
	if (ed->pos < 0) {
		len = AG_LengthUTF8(s);
		if (pos == -1 || ed->pos > len)
			ed->pos = len+1;
	}
	rv = ed->pos;

	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	
	AG_Redraw(ed);
	return (rv);
}

static void
Draw(void *obj)
{
	AG_Editable *ed = obj;
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_DriverClass *drvOps = WIDGET(ed)->drvOps;
	AG_Variable *stringb;
	char *s;
	int i, dx, dy, x, y;
	Uint32 *ucs;
	size_t len;

	stringb = GetStringUCS4(ed, &s, &ucs, &len);

	AG_PushBlendingMode(ed, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	AG_PushClipRect(ed, ed->r);

	AG_PushTextState();

	if (ed->font != NULL) { AG_TextFont(ed->font); }
	AG_TextColor(agColors[TEXTBOX_TXT_COLOR]);

	x = 0;
	y = -ed->y*agTextFontLineSkip;
	ed->xMax = 10;
	ed->yMax = 1;
	for (i = 0; i <= len; i++) {
		AG_Glyph *gl;
		Uint32 c = ucs[i];

		if (i == ed->pos && AG_WidgetIsFocused(ed)) {
			if ((ed->flags & AG_EDITABLE_BLINK_ON) &&
			    ed->y >= 0 && ed->y <= ed->yMax-1) {
				AG_DrawLineV(ed,
				    x - ed->x, (y + 1),
				    (y + agTextFontLineSkip - 1),
				    agColors[TEXTBOX_CURSOR_COLOR]);
			}
			ed->xCurs = x;
			if (ed->flags & AG_EDITABLE_MARKPREF) {
				ed->flags &= ~(AG_EDITABLE_MARKPREF);
				ed->xCursPref = x;
			}
			ed->yCurs = y/agTextFontLineSkip + ed->y;
		}
		if (i == len)
			break;

		if (WrapAtChar(ed, x, &ucs[i])) {
			y += agTextFontLineSkip;
			ed->xMax = MAX(ed->xMax, x);
			ed->yMax++;
			x = 0;
		}
		if (c == '\n') {
			y += agTextFontLineSkip;
			ed->xMax = MAX(ed->xMax, x+10);
			ed->yMax++;
			x = 0;
			continue;
		} else if (c == '\t') {
			x += agTextTabWidth;
			continue;
		}

		c = (ed->flags & AG_EDITABLE_PASSWORD) ? '*' : c;
		gl = AG_TextRenderGlyph(drv, c);
		dx = WIDGET(ed)->rView.x1 + x - ed->x;
		dy = WIDGET(ed)->rView.y1 + y;
	
		if (!AG_RectInside2(&WIDGET(ed)->rView, dx, dy)) {
			x += gl->advance;
			continue;
		}
		drvOps->drawGlyph(drv, gl, dx,dy);
		x += gl->advance;
	}
	if (ed->yMax == 1)
		ed->xMax = x;
	
	if ( !(ed->flags & (AG_EDITABLE_NOSCROLL|AG_EDITABLE_NOSCROLL_ONCE)) ) {
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			if (ed->yCurs < ed->y) {
				ed->y = ed->yCurs;
				if (ed->y < 0) { ed->y = 0; }
			} else if (ed->yCurs > ed->y + ed->yVis - 1) {
				ed->y = ed->yCurs - ed->yVis + 1;
			}
		}
		if (!(ed->flags & AG_EDITABLE_WORDWRAP)) {
			if (ed->xCurs < ed->x) {
				ed->x = ed->xCurs;
				if (ed->x < 0) { ed->x = 0; }
			} else if (ed->xCurs > ed->x + WIDTH(ed) - 10) {
				ed->x = ed->xCurs - WIDTH(ed) + 10;
			}
		}
	} else {
		if (ed->yCurs < ed->y) {
			if (ed->flags & AG_EDITABLE_MULTILINE) {
				AG_EditableMoveCursor(ed,
				    ed->xCursPref - ed->x, 1,
				    0);
			}
		} else if (ed->yCurs > ed->y + ed->yVis - 1) {
			if (ed->flags & AG_EDITABLE_MULTILINE) {
				AG_EditableMoveCursor(ed,
				    ed->xCursPref - ed->x,
				    ed->yVis*agTextFontLineSkip - 1,
				    0);
			}
		} else if (ed->xCurs < ed->x+10) {
			if (!(ed->flags & AG_EDITABLE_WORDWRAP)) {
				AG_EditableMoveCursor(ed,
				    ed->x+10,
				    (ed->yCurs - ed->y)*agTextFontLineSkip + 1,
				    1);
			}
		} else if (ed->xCurs > ed->x+WIDTH(ed)-10) {
			if (!(ed->flags & AG_EDITABLE_WORDWRAP)) {
				AG_EditableMoveCursor(ed,
				    ed->x+WIDTH(ed)-10,
				    (ed->yCurs - ed->y)*agTextFontLineSkip + 1,
				    1);
			}
		}
	}
	ed->flags &= ~(AG_EDITABLE_NOSCROLL_ONCE);
	AG_UnlockVariable(stringb);
	AG_PopTextState();

	AG_PopClipRect(ed);
	AG_PopBlendingMode(ed);

	FreeStringUCS4(ed, ucs);
}

void
AG_EditableSizeHint(AG_Editable *ed, const char *text)
{
	AG_ObjectLock(ed);
	AG_TextSize(text, &ed->wPre, &ed->hPre);
	AG_ObjectUnlock(ed);
}

void
AG_EditableSizeHintPixels(AG_Editable *ed, Uint w, Uint h)
{
	AG_ObjectLock(ed);
	ed->wPre = w;
	ed->hPre = h;
	AG_ObjectUnlock(ed);
}

void
AG_EditableSizeHintLines(AG_Editable *ed, Uint nLines)
{
	AG_ObjectLock(ed);
	ed->hPre = nLines*agTextFontHeight;
	AG_ObjectUnlock(ed);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Editable *ed = obj;

	r->w = ed->wPre;
	r->h = ed->hPre;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Editable *ed = obj;

	if (a->w < 2 || a->h < 2) {
		if (WIDGET(ed)->window != NULL &&
		    ed->ca != NULL) {
			AG_UnmapCursor(ed, ed->ca);
			ed->ca = NULL;
		}
		return (-1);
	}
	ed->yVis = a->h/agTextFontLineSkip;
	ed->r = AG_RECT(-1, -1, a->w-1, a->h-1);

	if (WIDGET(ed)->window != NULL &&
	    WIDGET(ed)->window->visible) {
		AG_Rect r = AG_RECT(0, 0, a->w, a->h);

		if (ed->ca == NULL) {
			ed->ca = AG_MapStockCursor(ed, r, AG_TEXT_CURSOR);
		} else {
			ed->ca->r = r;
		}
	}
	return (0);
}

static void
KeyDown(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();
	int keysym = AG_INT(1);
	int keymod = AG_INT(2);
	Uint32 unicode = (Uint32)AG_ULONG(3);

	switch (keysym) {
	case AG_KEY_LSHIFT:
	case AG_KEY_RSHIFT:
	case AG_KEY_LALT:
	case AG_KEY_RALT:
	case AG_KEY_LMETA:
	case AG_KEY_RMETA:
	case AG_KEY_LCTRL:
	case AG_KEY_RCTRL:
		return;
	case AG_KEY_TAB:
		if (!(WIDGET(ed)->flags & AG_WIDGET_CATCH_TAB)) {
			return;
		}
		break;
	default:
		break;
	}

	ed->repeatKey = keysym;
	ed->repeatMod = keymod;
	ed->repeatUnicode = unicode;
	ed->flags |= AG_EDITABLE_BLINK_ON;

	AG_LockTimeouts(ed);
	AG_DelTimeout(ed, &ed->toRepeat);
	if (ProcessKey(ed, keysym, keymod, unicode) == 1) {
		AG_ScheduleTimeout(ed, &ed->toDelay, agKbdDelay);
	} else {
		AG_DelTimeout(ed, &ed->toDelay);
	}
	AG_UnlockTimeouts(ed);

	AG_Redraw(ed);
}

static void
KeyUp(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();
	int keysym = AG_INT(1);
	
	if (ed->repeatKey == keysym) {
		AG_LockTimeouts(ed);
		AG_DelTimeout(ed, &ed->toRepeat);
		AG_DelTimeout(ed, &ed->toDelay);
		AG_ScheduleTimeout(ed, &ed->toCursorBlink, agTextBlinkRate);
		AG_UnlockTimeouts(ed);
	}
	if (keysym == AG_KEY_RETURN &&
	   (ed->flags & AG_EDITABLE_MULTILINE) == 0) {
		if (ed->flags & AG_EDITABLE_ABANDON_FOCUS) {
			AG_WidgetUnfocus(ed);
		}
		AG_PostEvent(NULL, ed, "editable-return", NULL);
	}
	
	AG_Redraw(ed);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();
	int btn = AG_INT(1);
	int mx = AG_INT(2);
	int my = AG_INT(3);
	
	AG_WidgetFocus(ed);

	switch (btn) {
	case AG_MOUSE_LEFT:
		ed->flags |= AG_EDITABLE_CURSOR_MOVING|AG_EDITABLE_BLINK_ON;
		mx += ed->x;
		AG_EditableMoveCursor(ed, mx, my, 0);
		ed->flags |= AG_EDITABLE_MARKPREF;
		break;
	case AG_MOUSE_WHEELUP:
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			ed->flags |= AG_EDITABLE_NOSCROLL_ONCE;
			ed->y -= AG_WidgetScrollDelta(&ed->wheelTicks);
			if (ed->y < 0) { ed->y = 0; }
		}
		break;
	case AG_MOUSE_WHEELDOWN:
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			ed->flags |= AG_EDITABLE_NOSCROLL_ONCE;
			ed->y += AG_WidgetScrollDelta(&ed->wheelTicks);
			ed->y = MIN(ed->y, ed->yMax - ed->yVis);
		}
		break;
	default:
		break;
	}
	
	AG_Redraw(ed);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();
	int btn = AG_INT(1);

	switch (btn) {
	case AG_MOUSE_LEFT:
		ed->flags &= ~(AG_EDITABLE_CURSOR_MOVING);
		break;
	default:
		break;
	}
	
	AG_Redraw(ed);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Editable *ed = AG_SELF();
	int mx = AG_INT(1);
	int my = AG_INT(2);

	if (!AG_WidgetIsFocused(ed))
		return;
	if ((ed->flags & AG_EDITABLE_CURSOR_MOVING) == 0)
		return;

	mx += ed->x;
	AG_EditableMoveCursor(ed, mx, my, 1);
	ed->flags |= AG_EDITABLE_MARKPREF;
	AG_Redraw(ed);
}

/* Overwrite the contents of the buffer with the given UTF-8 string. */
void
AG_EditableSetString(AG_Editable *ed, const char *text)
{
	AG_Variable *stringb;
	char *buf;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &buf);
	if (text != NULL) {
		Strlcpy(buf, text, stringb->info.size);
		ed->pos = AG_LengthUTF8(text);
	} else {
		buf[0] = '\0';
		ed->pos = 0;
	}
	AG_EditableBufferChanged(ed);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	AG_Redraw(ed);
}

/* Overwrite the contents of the buffer with the given UCS-4 string. */
void
AG_EditableSetStringUCS4(AG_Editable *ed, const Uint32 *ucs)
{
	AG_Variable *stringb;
	char *buf;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &buf);
	if (ucs != NULL) {
		AG_ExportUnicode(AG_UNICODE_TO_UTF8, buf, ucs,
		    stringb->info.size);
		ed->pos = (int)AG_LengthUCS4(ucs);
	} else {
		buf[0] = '\0';
		ed->pos = 0;
	}
	AG_EditableBufferChanged(ed);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	AG_Redraw(ed);
}

/* Write a formatted string to the buffer. */
void
AG_EditablePrintf(AG_Editable *ed, const char *fmt, ...)
{
	AG_Variable *stringb;
	va_list args;
	char *s;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &s);
	if (fmt != NULL && fmt[0] != '\0') {
		va_start(args, fmt);
		Vsnprintf(s, stringb->info.size, fmt, args);
		va_end(args);
		ed->pos = AG_LengthUTF8(s);
	} else {
		s[0] = '\0';
		ed->pos = 0;
	}
	AG_EditableBufferChanged(ed);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	AG_Redraw(ed);
}

/* Return a duplicate of the current string. */
char *
AG_EditableDupString(AG_Editable *ed)
{
	AG_Variable *stringb;
	char *s, *sd;

	stringb = AG_GetVariable(ed, "string", &s);
	sd = Strdup(s);
	AG_UnlockVariable(stringb);
	return (sd);
}

/* Return a duplicate of the current buffer in UCS-4 format. */
Uint32 *
AG_EditableDupStringUCS4(AG_Editable *ed)
{
	AG_Variable *stringb;
	char *s;
	Uint32 *ucs;

	stringb = AG_GetVariable(ed, "string", &s);
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, s, 0);
	AG_UnlockVariable(stringb);
	return (ucs);
}

/* Copy text to a fixed-size buffer and always NUL-terminate. */
size_t
AG_EditableCopyString(AG_Editable *ed, char *dst, size_t dst_size)
{
	AG_Variable *stringb;
	size_t rv;
	char *s;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &s);
	rv = Strlcpy(dst, s, dst_size);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	return (rv);
}

/* Copy text to a fixed-size buffer and always NUL-terminate. */
size_t
AG_EditableCopyStringUCS4(AG_Editable *ed, Uint32 *dst, size_t dst_size)
{
	AG_Variable *stringb;
	size_t rv;
	char *s;
	Uint32 *ucs;
	size_t len;

	AG_ObjectLock(ed);

	stringb = GetStringUCS4(ed, &s, &ucs, &len);
	rv = StrlcpyUCS4(dst, ucs, dst_size);
	FreeStringUCS4(ed, ucs);
	AG_UnlockVariable(stringb);

	AG_ObjectUnlock(ed);
	return (rv);
}

/* Perform trivial conversion from string to int. */
int
AG_EditableInt(AG_Editable *ed)
{
	AG_Variable *stringb;
	char *text;
	int i;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &text);
	i = atoi(text);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	return (i);
}

/* Perform trivial conversion from string to float . */
float
AG_EditableFlt(AG_Editable *ed)
{
	AG_Variable *stringb;
	char *text;
	float flt;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &text);
	flt = (float)strtod(text, NULL);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	return (flt);
}

/* Perform trivial conversion from string to double. */
double
AG_EditableDbl(AG_Editable *ed)
{
	AG_Variable *stringb;
	char *text;
	double flt;

	AG_ObjectLock(ed);
	stringb = AG_GetVariable(ed, "string", &text);
	flt = strtod(text, NULL);
	AG_UnlockVariable(stringb);
	AG_ObjectUnlock(ed);
	return (flt);
}

static void
Init(void *obj)
{
	AG_Editable *ed = obj;

	WIDGET(ed)->flags |= AG_WIDGET_FOCUSABLE|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_TABLE_EMBEDDABLE;

	ed->string[0] = '\0';
	ed->encoding = AG_ENCODING_UTF8;

	ed->flags = AG_EDITABLE_BLINK_ON|AG_EDITABLE_MARKPREF;
	ed->pos = 0;
	ed->sel_x1 = 0;
	ed->sel_x2 = 0;
	ed->sel_edit = 0;
	ed->compose = 0;
	ed->wPre = 0;
	ed->hPre = agTextFontLineSkip + 2;
	ed->xCurs = 0;
	ed->yCurs = 0;
	ed->xCursPref = 0;

	ed->x = 0;
	ed->xMax = 10;
	ed->y = 0;
	ed->yMax = 1;
	ed->yVis = 1;
	ed->wheelTicks = 0;
	ed->repeatKey = 0;
	ed->repeatMod = AG_KEYMOD_NONE;
	ed->repeatUnicode = 0;
	ed->ucsBuf = NULL;
	ed->ucsLen = 0;
	ed->r = AG_RECT(0,0,0,0);
	ed->ca = NULL;
	ed->font = NULL;

	AG_SetEvent(ed, "key-down", KeyDown, NULL);
	AG_SetEvent(ed, "key-up", KeyUp, NULL);
	AG_SetEvent(ed, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(ed, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(ed, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(ed, "widget-gainfocus", GainedFocus, NULL);
	AG_SetEvent(ed, "widget-lostfocus", LostFocus, NULL);
	AG_AddEvent(ed, "widget-hidden", LostFocus, NULL);

	AG_SetTimeout(&ed->toRepeat, RepeatTimeout, NULL, 0);
	AG_SetTimeout(&ed->toDelay, DelayTimeout, NULL, 0);
	AG_SetTimeout(&ed->toCursorBlink, BlinkTimeout, NULL, 0);

	AG_BindString(ed, "string", ed->string, sizeof(ed->string));

	AG_RedrawOnTick(ed, 1000);

#ifdef AG_DEBUG
	AG_BindUint(ed, "flags", &ed->flags);
	AG_BindUint(ed, "encoding", &ed->encoding);
	AG_BindInt(ed, "wPre", &ed->wPre);
	AG_BindInt(ed, "hPre", &ed->hPre);
	AG_BindInt(ed, "pos", &ed->pos);
	AG_BindUint32(ed, "compose", &ed->compose);
	AG_BindInt(ed, "xCurs", &ed->xCurs);
	AG_BindInt(ed, "yCurs", &ed->yCurs);
	AG_BindInt(ed, "xCursPref", &ed->xCursPref);
	AG_BindInt(ed, "sel_x1", &ed->sel_x1);
	AG_BindInt(ed, "sel_x2", &ed->sel_x2);
	AG_BindInt(ed, "sel_edit", &ed->sel_edit);
	AG_BindInt(ed, "x", &ed->x);
	AG_BindInt(ed, "xMax", &ed->xMax);
	AG_BindInt(ed, "y", &ed->y);
	AG_BindInt(ed, "yMax", &ed->yMax);
	AG_BindInt(ed, "yVis", &ed->yVis);
	AG_BindUint32(ed, "wheelTicks", &ed->wheelTicks);
	AG_BindUint32(ed, "repeatUnicode", &ed->repeatUnicode);
	AG_BindUint(ed, "ucsLen", &ed->ucsLen);
#endif /* AG_DEBUG */
}

static void
Destroy(void *obj)
{
	AG_Editable *ed = obj;

	if (ed->flags & AG_EDITABLE_STATIC)
		Free(ed->ucsBuf);
}

AG_WidgetClass agEditableClass = {
	{
		"Agar(Widget:Editable)",
		sizeof(AG_Editable),
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

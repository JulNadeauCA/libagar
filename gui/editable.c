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
 * The base Agar text editor (the low-level widget behind AG_Textbox(3)).
 * It enables the user to edit single- or multi-line, NUL-terminated strings.
 * It supports ASCII, UTF-8, UCS-4 (and other encodings through iconv), and
 * also interprets ANSI SGR attributes.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/editable.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/ttf.h>
#include <agar/gui/keymap.h>
#include <agar/gui/primitive.h>
#include <agar/gui/cursors.h>
#include <agar/gui/menu.h>
#include <agar/gui/tlist.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/label.h>
#include <agar/gui/separator.h>
#include <agar/gui/icons.h>
#include <agar/gui/gui_math.h>

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <agar/config/have_freetype.h>

AG_EditableClipboard agEditableClipbrd;		/* For Copy/Cut/Paste */
AG_EditableClipboard agEditableKillring;	/* For Emacs-style Kill/Yank */

static Uint32 AutocompleteTimeout(AG_Timer *_Nonnull, AG_Event *_Nonnull);

/*
 * Return a new working buffer. The variable is returned locked; the caller
 * should invoke ReleaseBuffer() after use.
 */
static __inline__ AG_EditableBuffer *_Nullable
GetBuffer(AG_Editable *_Nonnull ed)
{
	AG_EditableBuffer *buf;

	if (ed->flags & AG_EDITABLE_EXCL) {
		buf = &ed->sBuf;
	} else {
		if ((buf = TryMalloc(sizeof(AG_EditableBuffer))) == NULL) {
			return (NULL);
		}
		buf->s = NULL;
		buf->len = 0;
		buf->maxLen = 0;
	}

#ifdef AG_UNICODE
	if (AG_Defined(ed, "text")) {                    /* AG_TextElement(3) */
		AG_TextElement *txt;

		buf->var = AG_GetVariable(ed, "text", (void *)&txt);
		buf->reallocable = 1;

		AG_MutexLock(&txt->lock);

		if ((ed->flags & AG_EDITABLE_EXCL) == 0 || buf->s == NULL) {
			const AG_TextEnt *te = &txt->ent[ed->lang];

			if (te->buf != NULL) {
				buf->s = AG_ImportUnicode("UTF-8", te->buf,
				                          &buf->len,
				                          &buf->maxLen);
			} else {
				if ((buf->s = TryMalloc(sizeof(AG_Char))) != NULL) {
					buf->s[0] = (AG_Char)'\0';
				}
				buf->len = 0;
			}
			if (buf->s == NULL) {
				AG_MutexUnlock(&txt->lock);
				AG_UnlockVariable(buf->var);
				buf->var = NULL;
				goto fail;
			}
		}
	} else
#endif /* AG_UNICODE */
	{                                                       /* "C" string */
		char *s;

		buf->var = AG_GetVariable(ed, "string", (void *)&s);
		buf->reallocable = 0;

		if ((ed->flags & AG_EDITABLE_EXCL) == 0 || buf->s == NULL) {
#ifdef AG_UNICODE
			buf->s = AG_ImportUnicode(ed->encoding, s, &buf->len,
			                          &buf->maxLen);
#else
			buf->s = (Uint8 *)TryStrdup(s);
			buf->maxLen = buf->len = strlen(s);
#endif
			if (buf->s == NULL) {
				AG_UnlockVariable(buf->var);
				goto fail;
			}
		}
	}
	return (buf);
fail:
	if ((ed->flags & AG_EDITABLE_EXCL) == 0) { free(buf); }
	return (NULL);
}

/* Clear a working buffer. */
static __inline__ void
ClearBuffer(AG_EditableBuffer *_Nonnull buf)
{
	AG_Free(buf->s);
	buf->s = NULL;
	buf->len = 0;
	buf->maxLen = 0;
}

/* Commit changes to the working buffer. */
static void
CommitBuffer(AG_Editable *_Nonnull ed, AG_EditableBuffer *_Nonnull buf)
{
#ifdef AG_UNICODE
	if (AG_Defined(ed, "text")) {                    /* AG_TextElement(3) */
		AG_TextElement *txt = buf->var->data.p;
		AG_TextEnt *te = &txt->ent[ed->lang];
		AG_Size len;

		if (AG_LengthUTF8FromUCS4(buf->s, &len) == -1)
			goto fail;

		if ((++len > te->maxLen) && AG_TextRealloc(te, len) == -1)
			goto fail;

		if (AG_ExportUnicode(ed->encoding, te->buf, buf->s,
		                     te->maxLen + 1) == -1)
			goto fail;
	} else {                                                /* "C" string */
		if (AG_ExportUnicode(ed->encoding,
		                     buf->var->data.s, buf->s,
				     buf->var->info.size) == -1)
			goto fail;
	}
#else  /* !AG_UNICODE */

	Strlcpy(buf->var->data.s, (const char *)buf->s, buf->var->info.size);

#endif /* !AG_UNICODE */

	ed->flags |= AG_EDITABLE_MARKPREF;
	AG_PostEvent(ed, "editable-postchg", NULL);
	return;
#ifdef AG_UNICODE
fail:
	Verbose("CommitBuffer: %s; ignoring\n", AG_GetError());
#endif
}

/* Release the working buffer. */
static __inline__ void
ReleaseBuffer(AG_Editable *_Nonnull ed, AG_EditableBuffer *_Nonnull buf)
{
#ifdef AG_UNICODE
	if (AG_Defined(ed, "text"))
		AG_MutexUnlock(&AGTEXTELEMENT(buf->var->data.p)->lock);
#endif
	if (buf->var != NULL) {
		AG_UnlockVariable(buf->var);
		buf->var = NULL;
	}
	if (!(ed->flags & AG_EDITABLE_EXCL)) {
		ClearBuffer(buf);
		Free(buf);
	}
}

/* Allocate and return a new buffer handle in a locked condition */
AG_EditableBuffer *
AG_EditableGetBuffer(AG_Editable *ed)
{
	AG_EditableBuffer *buf;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if ((buf = GetBuffer(ed)) == NULL) {
		AG_ObjectUnlock(ed);
	}
	return (buf);
}

/* Clear a working buffer. */
void
AG_EditableClearBuffer(AG_Editable *ed, AG_EditableBuffer *buf)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");

	ClearBuffer(buf);
}

/* Increase the working buffer size to accomodate new characters. */
int
AG_EditableGrowBuffer(AG_Editable *ed, AG_EditableBuffer *buf, AG_Char *ins,
    AG_Size nIns)
{
	AG_Size newLen;		/* UCS-4 buffer size in bytes */
	AG_Size convLen;	/* Converted string length in bytes */
	AG_Char *sNew;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");

	newLen = (buf->len + nIns + 1)*sizeof(AG_Char);

#ifdef AG_UNICODE
	if (strcmp(ed->encoding, "UTF-8") == 0) {
		AG_Size sLen, insLen;

		if (AG_LengthUTF8FromUCS4(buf->s, &sLen) == -1 ||
		    AG_LengthUTF8FromUCS4(ins, &insLen) == -1) {
			return (-1);
		}
		convLen = sLen + insLen + 1;
	} else if (strcmp(ed->encoding, "US-ASCII") == 0) {
		convLen = AG_LengthUCS4(buf->s) + nIns + 1;
	} else {
		/* TODO Proper estimates for other charsets */
		convLen = newLen;
	}
#else /* !AG_UNICODE */
	if (strcmp(ed->encoding, "US-ASCII") == 0) {
		convLen = strlen((const char *)buf->s) + nIns + 1;
	} else {
		convLen = newLen;
	}
#endif /* AG_UNICODE */

	if (!buf->reallocable) {
		if (convLen > buf->var->info.size) {
			AG_SetError("%u > %u bytes", (Uint)convLen, (Uint)buf->var->info.size);
			return (-1);
		}
	}
	if (newLen > buf->maxLen) {
		if ((sNew = TryRealloc(buf->s, newLen)) == NULL) {
			return (-1);
		}
		buf->s = sNew;
		buf->maxLen = newLen;
	}
	return (0);
}

/* Release a working buffer. */
void
AG_EditableReleaseBuffer(AG_Editable *ed, AG_EditableBuffer *buf)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");

	ReleaseBuffer(ed, buf);

	AG_ObjectUnlock(ed);
}

AG_Editable *
AG_EditableNew(void *parent, Uint flags)
{
	AG_Editable *ed;

	ed = Malloc(sizeof(AG_Editable));
	AG_ObjectInit(ed, &agEditableClass);

	if (flags & AG_EDITABLE_HFILL) { WIDGET(ed)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_EDITABLE_VFILL) { WIDGET(ed)->flags |= AG_WIDGET_VFILL; }
	if (flags & AG_EDITABLE_CATCH_TAB) { WIDGET(ed)->flags |= AG_WIDGET_CATCH_TAB; }

	ed->flags |= flags;

	if ((flags & AG_EDITABLE_EXCL) == 0)
		AG_RedrawOnTick(ed, 1000);           /* Default update rate */

	AG_ObjectAttach(parent, ed);
	return (ed);
}

/* Bind to a C string containing ASCII text. */
void
AG_EditableBindASCII(AG_Editable *ed, char *buf, AG_Size bufSize)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_Unset(ed, "text");
	AG_BindString(ed, "string", buf, bufSize);
	ed->encoding = "US-ASCII";

	AG_ObjectUnlock(ed);
}

#ifdef AG_UNICODE
/* Bind to a C string containing UTF-8 encoded text. */
void
AG_EditableBindUTF8(AG_Editable *ed, char *buf, AG_Size bufSize)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_Unset(ed, "text");
	AG_BindString(ed, "string", buf, bufSize);
	ed->encoding = "UTF-8";

	AG_ObjectUnlock(ed);
}

/* Bind to a C string containing text in specified encoding. */
void
AG_EditableBindEncoded(AG_Editable *ed, const char *encoding, char *buf,
    AG_Size bufSize)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_Unset(ed, "text");
	AG_BindString(ed, "string", buf, bufSize);
	ed->encoding = encoding;

	AG_ObjectUnlock(ed);
}

/* Bind to an AG_TextElement(3). */
void
AG_EditableBindText(AG_Editable *ed, AG_TextElement *txt)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_Unset(ed, "string");
	AG_BindPointer(ed, "text", (void *)txt);
	ed->encoding = "UTF-8";

	AG_ObjectUnlock(ed);
}

/* Set the active language (when bound to an AG_TextElement(3)) */
void
AG_EditableSetLang(AG_Editable *ed, enum ag_language lang)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	ed->lang = lang;
	ed->pos = 0;
	ed->selStart = 0;
	ed->selEnd = 0;

	AG_Redraw(ed);
	AG_ObjectUnlock(ed);
}
#endif /* AG_UNICODE */

/* Enable or disable password entry mode. */
void
AG_EditableSetPassword(AG_Editable *ed, int enable)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_SETFLAGS(ed->flags, AG_EDITABLE_PASSWORD, enable);

	AG_ObjectUnlock(ed);
}

/* Enable or disable word wrapping. */
void
AG_EditableSetWordWrap(AG_Editable *ed, int enable)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	ed->x = 0;
	ed->y = 0;
	ed->pos = 0;
	ed->selStart = 0;
	ed->selEnd = 0;

	AG_SETFLAGS(ed->flags, AG_EDITABLE_WORDWRAP, enable);

	AG_ObjectUnlock(ed);
}

/*
 * Advise as to whether the buffer contents are accessed exclusively by this
 * particular widget instance. If so, disable periodic redraw (otherwise set
 * a default refresh rate of 1s).
 */
void
AG_EditableSetExcl(AG_Editable *ed, int enable)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	ClearBuffer(&ed->sBuf);

	if (enable) {
		ed->flags |= AG_EDITABLE_EXCL;
		AG_RedrawOnTick(ed, -1);
	} else {
		ed->flags &= ~(AG_EDITABLE_EXCL);
		AG_RedrawOnTick(ed, 1000);
	}
	AG_ObjectUnlock(ed);
}

/* Toggle floating-point only input */
void
AG_EditableSetFltOnly(AG_Editable *ed, int enable)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
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
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (enable) {
		ed->flags |= AG_EDITABLE_INT_ONLY;
		ed->flags &= ~(AG_EDITABLE_FLT_ONLY);
	} else {
		ed->flags &= ~(AG_EDITABLE_INT_ONLY);
	}

	AG_ObjectUnlock(ed);
}

/* Evaluate if a character is acceptable in integer-only mode. */
static __inline__ _Const_Attribute int
CharIsIntOnly(AG_Char c)
{
	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
	case '+':
		return (1);
	default:
		return (0);
	}
}

/* Evaluate if a character is acceptable in float-only mode. */
static __inline__ _Const_Attribute int
CharIsFltOnly(AG_Char c)
{
	switch (c) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '.':
	case '-':
	case '+':
	case 'e':
	case 'i':
	case 'n':
	case 'f':
	case 'a':
	case 0x221e:		/* inf */
		return (1);
	default:
		return (0);
	}
}

/*
 * Process a keystroke. May be invoked from the repeat timeout routine or
 * the keydown handler. If we return 1, the current delay/repeat cycle will
 * be maintained, otherwise it will be cancelled.
 */
static int
ProcessKey(AG_Editable *_Nonnull ed, AG_KeySym ks, AG_KeyMod kmod, AG_Char ch)
{
	AG_EditableBuffer *buf;
	int i, rv;

	if (ks == AG_KEY_ESCAPE) {
		return (0);
	}
	if ((ks == AG_KEY_RETURN || ks == AG_KEY_KP_ENTER) &&
	   (ed->flags & AG_EDITABLE_MULTILINE) == 0)
		return (0);

	if (kmod == AG_KEYMOD_NONE &&
	    (ks & ~0x7f) == 0 &&				/* is ascii */
	    isprint((int)ks)) {
		if ((ed->flags & AG_EDITABLE_INT_ONLY) &&
		    !CharIsIntOnly((AG_Char)ks)) {
			return (0);
		} else if ((ed->flags & AG_EDITABLE_FLT_ONLY) &&
		           !CharIsFltOnly((AG_Char)ks)) {
			return (0);
		}
	}

	if ((buf = GetBuffer(ed)) == NULL)
		return (0);

	if (ed->pos < 0) { ed->pos = 0; }
	if (ed->pos > buf->len) { ed->pos = buf->len; }

	for (i=0, rv=0; ; i++) {
		const struct ag_keycode *kc = &agKeymap[i];
		const char *flag;
	
		if ((kc->key != AG_KEY_LAST) &&
		    (kc->key != ks)) {
			continue;
		}
		if (kc->modKeys != 0 && ((kmod & kc->modKeys) == 0)) {
			continue;
		}
		for (flag = &kc->flags[0]; *flag != '\0'; flag++) {
			switch (*flag) {
			case 'w':
				if (AGEDITABLE_IS_READONLY(ed)) {
					goto next_key;
				}
				break;
			case 'k':
				if (ed->flags & AG_EDITABLE_NO_KILL_YANK) {
					goto next_key;
				}
				break;
			}
		}
		AG_PostEvent(ed, "editable-prechg", NULL);
		rv = kc->func(ed, buf, ks, kmod, ch);
		break;
next_key:
		if (kc->key == AG_KEY_LAST) {
			break;
		} else {
			continue;
		}
	}

	if (rv == 1) {
		CommitBuffer(ed, buf);
	}
	ReleaseBuffer(ed, buf);
	return (1);
}

/* Timer callback for handling key repeat */
static Uint32
KeyRepeatTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int keysym = AG_INT(1);
	const int keymod = AG_INT(2);
	const AG_Char ch = AG_CHAR(3);
	
	if (ProcessKey(ed, keysym, keymod, ch) == 0) {
		return (0);
	}
	ed->flags |= AG_EDITABLE_BLINK_ON;
	AG_Redraw(ed);
	return (agKbdRepeat);
}

/* Timer callback for blinking cursor */
static Uint32
BlinkTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();

	if ((ed->flags & AG_EDITABLE_CURSOR_MOVING) == 0) {
		AG_INVFLAGS(ed->flags, AG_EDITABLE_BLINK_ON);
		AG_Redraw(ed);
	}
	return (to->ival);
}

static void
OnFocusGain(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	int i;

	AG_LockTimers(ed);
	AG_DelTimer(ed, &ed->toRepeat);
	for (i = 0; i < 4; i++) {
		AG_DelTimer(ed, &ed->toRepeatDirs[i]);
	}
	AG_AddTimer(ed, &ed->toCursorBlink, agTextBlinkRate, BlinkTimeout, NULL);
	ed->flags |= AG_EDITABLE_BLINK_ON;
	AG_UnlockTimers(ed);

	AG_Redraw(ed);
}

static void
OnFocusLoss(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	int i;

	AG_LockTimers(ed);
	AG_DelTimer(ed, &ed->toRepeat);
	for (i = 0; i < 4; i++) {
		AG_DelTimer(ed, &ed->toRepeatDirs[i]);
	}
	AG_DelTimer(ed, &ed->toCursorBlink);
	AG_DelTimer(ed, &ed->toDblClick);
	ed->flags &= ~(AG_EDITABLE_BLINK_ON | AG_EDITABLE_CURSOR_MOVING);
	ed->returnHeld = 0;
	AG_UnlockTimers(ed);

	AG_Redraw(ed);
}

static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();

	if (ed->complete) {
		AG_DelTimer(ed, &ed->complete->to);
		free(ed->complete);
		ed->complete = NULL;
	}
	if (ed->pm != NULL) {
		AG_PopupHide(ed->pm);
	}
	OnFocusLoss(event);
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int height = WFONT(ed)->height;
	const int lineskip = WFONT(ed)->lineskip;

	ed->fontMaxHeight = height;
	ed->lineSkip = lineskip;
	ed->yVis = HEIGHT(ed) / lineskip;

	if (ed->suPlaceholder != -1) {
		AG_WidgetUnmapSurface(ed, ed->suPlaceholder);
		ed->suPlaceholder = -1;
	}
}

/*
 * Evaluate word wrapping at given character (possibly rendering it).
 */
static __inline__ int
WrapAtChar(AG_Editable *_Nonnull ed, int x, AG_Char *_Nonnull s,
    AG_Font *_Nonnull font, const AG_Color *_Nonnull cBg,
    const AG_Color *_Nonnull cFg)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_Glyph *G;
	AG_Char *t;
	int x2;

	if (!(ed->flags & AG_EDITABLE_WORDWRAP) ||
	    x == 0 || !AG_CharIsSpace(*s)) {
		return (0);
	}
	for (t = &s[1], x2 = x;
	     *t != '\0';
	     t++) {
		G = AG_TextRenderGlyph(drv, font, cBg, cFg, *t);
		x2 += G->advance;
		if (AG_CharIsSpace(*t) || *t == '\n') {
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
 * Map mouse coordinates to a character position within the buffer.
 */
#define ON_LINE(my,y)       ((my) >= (y) && (my) <= (y)+ed->lineSkip)
#define ON_CHAR(mx,x,glyph) ((mx) >= (x) && (mx) <= (x)+(glyph)->advance)
int
AG_EditableMapPosition(AG_Editable *ed, AG_EditableBuffer *buf, int mx, int my,
    int *pos)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_Font *font = WFONT(ed);
	AG_TextANSI ansi;
	int i, x, y, yMouse;
	
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if ((yMouse = my + ed->y*ed->lineSkip) < 0) {
		*pos = 0;
		goto out;
	}
	for (i=0, x=0, y=0; i < buf->len; i++) {
		const AG_Char ch = buf->s[i];

		if (mx <= 0 && ON_LINE(yMouse,y)) {
			*pos = i;
			goto out;
		}
		if (WrapAtChar(ed, x, &buf->s[i], font, &ts->colorBG, &ts->color)) {
			if (ON_LINE(yMouse,y) && mx > x) {
				*pos = i;
				goto out;
			}
			y += ed->lineSkip;
			x = 0;
		}
		if (ch == '\n') {
			if (ON_LINE(yMouse,y) && mx > x) {
				*pos = i;
				goto out;
			}
			y += ed->lineSkip;
			x = 0;
			continue;
		} else if (ch == '\t') {
			if (ON_LINE(yMouse,y) &&
			    mx >= x && mx <= x+agTextTabWidth) {
				*pos = (mx < x + (agTextTabWidth >> 1)) ? i : i+1;
				goto out;
			}
			x += agTextTabWidth;
			continue;
		} else if (  ch == 0x1b &&
		    buf->s[i+1] >= 0x40 &&
		    buf->s[i+1] <= 0x5f &&
		    buf->s[i+2] != '\0') {
			if (AG_TextParseANSI(ts, &ansi, &buf->s[i+1]) == 0) {
				i += ansi.len;
				continue;
			}
		}
		
		switch (font->spec.type) {
#ifdef HAVE_FREETYPE
		case AG_FONT_VECTOR:
			{
				AG_TTFFont *ttf = font->data.vec.ttf;
				AG_TTFGlyph *Gttf;

				if (AG_TTFFindGlyph(ttf, ch,
				    TTF_CACHED_METRICS | TTF_CACHED_BITMAP) != 0) {
					continue;
				}
				Gttf = ttf->current;

				if (ON_LINE(yMouse, y) &&
				    ON_CHAR(mx, x, Gttf)) {
					*pos = (mx < x + (Gttf->advance >> 1)) ?
					       i : i+1;
					if (buf->s[*pos]   == 0x1b &&
					    buf->s[*pos+1] >= 0x40 &&
					    buf->s[*pos+1] <= 0x5f &&
					    buf->s[*pos+2] != '\0') {
						if (AG_TextParseANSI(ts, &ansi,
						    &buf->s[*pos+1]) == 0)
							*pos += ansi.len+1;
					}
					goto out;
				}
				x += Gttf->advance;
			}
			break;
#endif /* HAVE_FREETYPE */
		case AG_FONT_BITMAP:
			{
				AG_Glyph *G;
			
				G = AG_TextRenderGlyph(drv, font,
				    &ts->colorBG, &ts->color, ch);

				if (ON_LINE(yMouse,y) &&
				    mx >= x &&
				    mx <= x + G->su->w) {
					*pos = i;
					if (buf->s[*pos]   == 0x1b &&
					    buf->s[*pos+1] >= 0x40 &&
					    buf->s[*pos+1] <= 0x5f &&
					    buf->s[*pos+2] != '\0') {
						if (AG_TextParseANSI(ts, &ansi,
						    &buf->s[*pos+1]) == 0)
							*pos += ansi.len+1;
					}
					goto out;
				}
				x += G->su->w;
			}
			break;
		case AG_FONT_DUMMY:
		default:
			break;
		}
	}
	*pos = buf->len;
out:
	AG_ObjectUnlock(ed);
	return (0);
}
#undef ON_LINE
#undef ON_CHAR

/* Move cursor to the given position in pixels. */
void
AG_EditableMoveCursor(AG_Editable *ed, AG_EditableBuffer *buf, int mx, int my)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (AG_EditableMapPosition(ed, buf, mx, my, &ed->pos) == 0) {
		ed->selStart = 0;
		ed->selEnd = 0;
		AG_Redraw(ed);
	}

	AG_ObjectUnlock(ed);
}

/* Set the cursor position (-1 = end of the string) with bounds checking. */
int
AG_EditableSetCursorPos(AG_Editable *ed, AG_EditableBuffer *buf, int pos)
{
	int rv;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	ed->pos = pos;
	ed->selStart = 0;
	ed->selEnd = 0;
	if (ed->pos < 0) {
		if (pos == -1 || ed->pos > buf->len)
			ed->pos = buf->len;
	}
	rv = ed->pos;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;

	AG_Redraw(ed);
	AG_ObjectUnlock(ed);

	return (rv);
}

/* Try to maintain visibility of the cursor (KEEPVISCURSOR option). */
static void
KeepVisibleCursor(AG_Editable *_Nonnull ed, AG_EditableBuffer *_Nonnull buf)
{
	if (ed->yCurs < ed->y) {
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			AG_EditableMoveCursor(ed, buf,
			    ed->xCursPref - ed->x,
			    1);
		}
	} else if (ed->yCurs > ed->y + ed->yVis - 1) {
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			AG_EditableMoveCursor(ed, buf,
			    ed->xCursPref - ed->x,
			    ed->yVis*ed->lineSkip - 1);
		}
	} else if (ed->xCurs < ed->x+10) {
		if (!(ed->flags & AG_EDITABLE_WORDWRAP)) {
			AG_EditableMoveCursor(ed, buf,
			    ed->x+10,
			    (ed->yCurs - ed->y)*ed->lineSkip + 1);
		}
	} else if (ed->xCurs > ed->x+WIDTH(ed)-10) {
		if (!(ed->flags & AG_EDITABLE_WORDWRAP)) {
			AG_EditableMoveCursor(ed, buf,
			    ed->x+WIDTH(ed)-10,
			    (ed->yCurs - ed->y)*ed->lineSkip + 1);
		}
	}
}

static void
Draw(void *_Nonnull obj)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Editable *ed = obj;
	AG_Driver *drv = WIDGET(ed)->drv;
	const AG_DriverClass *drvOps = WIDGET(ed)->drvOps;
	AG_EditableBuffer *buf;
	const AG_Color *cEditableBg = &WCOLOR(ed, BG_COLOR);
	const AG_Color *cSel = &WCOLOR(ed, SELECTION_COLOR);
	AG_Color cFg = ts->color;
	AG_Color cBg = ts->colorBG;
	AG_Font *font = ts->font;
	const int flags = ed->flags;
	const int pos = ed->pos;
	const int lineSkip = ed->lineSkip;
	const int clipX1 = WIDGET(ed)->rView.x1 - (ed->fontMaxHeight << 1);
	const int clipX2 = WIDGET(ed)->rView.x2 + (ed->fontMaxHeight << 1);
	const int clipY1 = WIDGET(ed)->rView.y1 - lineSkip;
	const int clipY2 = WIDGET(ed)->rView.y2 + lineSkip;
	const int selStart = ed->selStart;
	const int selEnd = ed->selEnd;
	const int paddingLeft = WIDGET(ed)->paddingLeft;
	int i, dx,dy, x,y, yCurs=0, selected=0;

	if (cEditableBg->a > 0)
		AG_DrawRectFilled(ed, &WIDGET(ed)->r, cEditableBg);

	if ((buf = GetBuffer(ed)) == NULL) {
		return;
	}
	AG_EditableValidateSelection(ed, buf);
	
	AG_PushBlendingMode(ed, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	AG_PushClipRect(ed, &ed->r);

	if (buf->len == 0 && AG_Defined(ed, "placeholder")) {
		AG_Variable *Vph;

		if (ed->suPlaceholder == -1 &&
		    (Vph = AG_AccessVariable(ed, "placeholder")) != NULL) {
			AG_Color c = WCOLOR(ed, TEXT_COLOR);

			AG_PushTextState();
			AG_ColorDarken(&c,8);   /* XXX color scheme dependent */
			AG_TextColor(&c);
			ed->suPlaceholder = AG_WidgetMapSurface(ed,
			    AG_TextRender((char *)Vph->data.p));
			AG_UnlockVariable(Vph);
			AG_PopTextState();
		}
		AG_WidgetBlitFrom(ed, ed->suPlaceholder, NULL, 0,0);
	}

	x = paddingLeft;
	y = WIDGET(ed)->paddingTop - ed->y*lineSkip;
	ed->xMax = 10;
	ed->yMax = 1;
	for (i = 0; i <= buf->len; i++) {
		AG_Glyph *G;
		AG_Char c = buf->s[i];
		AG_Rect r;

		if (i == pos) {                                /* At cursor */
			ed->xCurs = x;
			if (flags & AG_EDITABLE_MARKPREF) {
				ed->flags &= ~(AG_EDITABLE_MARKPREF);
				ed->xCursPref = x;
			}
			ed->yCurs = y/lineSkip + ed->y;
			yCurs = y;
		}
		if (selEnd > selStart) {
			if (i >= selStart && i < selEnd) {
				if (!selected) {
					selected = 1;
					ed->xSelStart = x;
					ed->ySelStart = y/lineSkip + ed->y;
				}
			} else {
				if (selected) {
					selected = 0;
					ed->xSelEnd = x;
					ed->ySelEnd = y/lineSkip + ed->y;
				}
			}
		}
		if (i == buf->len || c == '\0')
			break;

		if (WrapAtChar(ed, x, &buf->s[i], font, &cBg, &cFg)) {
			y += lineSkip;
			ed->xMax = MAX(ed->xMax, x);
			ed->yMax++;
			x = paddingLeft;
		}
		if (c == '\n') {
			y += lineSkip;
			if (selected) {
				AG_DrawLineV(ed, 1, y, y+lineSkip, cSel);
			}
			ed->xMax = MAX(ed->xMax, x+10);
			ed->yMax++;
			x = paddingLeft;
			continue;
		} else if (c == '\t') {
			if (selected) {
				r.x = x - ed->x;
				r.y = y;
				r.w = agTextTabWidth + 1;
				r.h = lineSkip + 1;
				AG_DrawRectFilled(ed, &r, cSel);
			}
			x += agTextTabWidth;
			continue;
		} else if (c == 0x1b &&
		    buf->s[i+1] >= 0x40 &&
		    buf->s[i+1] <= 0x5f &&
		    buf->s[i+2] != '\0') {
			AG_TextANSI ansi;
			
			if (AG_TextParseANSI(ts, &ansi, &buf->s[i+1]) == 0) {
				if (ansi.ctrl == AG_ANSI_CSI_SGR) {
					switch (ansi.sgr) {
					case AG_SGR_RESET:
					case AG_SGR_NO_FG_NO_BG:
						cFg = ts->color;
						cBg = ts->colorBG;
						break;
					case AG_SGR_FG:
						cFg = ansi.color;
						break;
					case AG_SGR_BG:
						cBg = ansi.color;
						break;
					case AG_SGR_BOLD:
						break;
					case AG_SGR_UNDERLINE:
						break;
					default:
						break;
					}
				}
				i += ansi.len;
				continue;
			}
		}

		if      (flags & AG_EDITABLE_PASSWORD)  { c = '*'; }
		else if (flags & AG_EDITABLE_UPPERCASE) { c = toupper(c); }
		else if (flags & AG_EDITABLE_LOWERCASE) { c = tolower(c); }

		G = AG_TextRenderGlyph(drv, font, &cBg, &cFg, c);
		dx = WIDGET(ed)->rView.x1 + x - ed->x;
		dy = WIDGET(ed)->rView.y1 + y;

		if (dx < clipX1 || dx >= clipX2 ||               /* Outside */
		    dy < clipY1 || dy >= clipY2) {
			x += G->advance;
			continue;
		}
		if (selected) {
			r.x = x - ed->x;
			r.y = y + 1;
			r.w = G->su->w + 1;
			r.h = G->su->h - 1;
			AG_DrawRectFilled(ed, &r, cSel);
		}

		drvOps->drawGlyph(drv, G, dx,dy);

		x += G->advance;
	}
	if (ed->yMax == 1)
		ed->xMax = x;
	
	/*
	 * Draw the cursor.
	 * TODO different styles.
	 * TODO an option to hide the cursor behind any overlapping selection.
	 */
	if ((flags & AG_EDITABLE_BLINK_ON) && AG_WidgetIsFocused(ed) &&
	    (ed->y >= 0 && ed->y <= ed->yMax-1)) {
		AG_DrawLineV(ed,
		    ed->xCurs - ed->x + 1, (yCurs + 1),
		    (yCurs + lineSkip - 1),
		    &WCOLOR(ed, TEXT_COLOR));
	}
	
	if (flags & AG_EDITABLE_KEEPVISCURSOR) {        /* Maintain visible */
		KeepVisibleCursor(ed, buf);
	}
	if (ed->xScrollTo != NULL) {                    /* X scroll request */
		const int xScrollTo = *ed->xScrollTo;

		if ((xScrollTo - ed->x) < 0) {
			ed->x += (xScrollTo - ed->x);
			if (ed->x < 0) { ed->x = 0; }
		}
		if ((xScrollTo - ed->x) > WIDTH(ed) - 10) {
			ed->x = xScrollTo - WIDTH(ed) + 10;
		}
		ed->xScrollTo = NULL;
		WIDGET(ed)->window->dirty = 1;
	}
	if (ed->yScrollTo != NULL) {                    /* Y scroll request */
		const int yScrollTo = *ed->yScrollTo;

		if ((yScrollTo - ed->y) < 0) {
			ed->y += (yScrollTo - ed->y);
			if (ed->y < 0) { ed->y = 0; }
		} else if ((yScrollTo - ed->y) > ed->yVis - 1) {
			ed->y = yScrollTo - ed->yVis + 1;
		} else if (yScrollTo <= ed->y) {
			if (ed->y > 0)
				ed->y--;
		}
		ed->yScrollTo = NULL;
		WIDGET(ed)->window->dirty = 1;
	}
	if (ed->xScrollPx != 0) {             /* X scroll request in pixels */
		if (ed->xCurs < ed->x - ed->xScrollPx ||
		    ed->xCurs > ed->x + WIDTH(ed) - ed->xScrollPx) {
			ed->x += ed->xScrollPx;
		}
		ed->xScrollPx = 0;
		WIDGET(ed)->window->dirty = 1;
	}

	AG_PopClipRect(ed);
	AG_PopBlendingMode(ed);

	ReleaseBuffer(ed, buf);
}

void
AG_EditableSizeHint(AG_Editable *ed, const char *text)
{
	int hPre;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_TextSize(text, &ed->wPre, &hPre);
	ed->hPre = MIN(1, hPre/ed->lineSkip);

	AG_ObjectUnlock(ed);
}

void
AG_EditableSizeHintPixels(AG_Editable *ed, Uint w, Uint h)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	ed->wPre = w;
	ed->hPre = MIN(1, h/ed->lineSkip);

	AG_ObjectUnlock(ed);
}

void
AG_EditableSizeHintLines(AG_Editable *ed, Uint nLines)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	ed->hPre = nLines;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Editable *ed = obj;

	r->w = WIDGET(ed)->paddingLeft + ed->wPre +
	       WIDGET(ed)->paddingRight;

	r->h = WIDGET(ed)->paddingTop + ed->hPre*ed->lineSkip +
	       WIDGET(ed)->paddingBottom;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Editable *ed = obj;
	AG_Rect r;
	const int paddingLeft   = WIDGET(ed)->paddingLeft;
	const int paddingRight  = WIDGET(ed)->paddingRight;
	const int paddingTop    = WIDGET(ed)->paddingTop;
	const int paddingBottom = WIDGET(ed)->paddingBottom;

	if (a->w < paddingLeft + paddingRight ||
	   (a->h < paddingTop + paddingBottom))
		return (-1);

	ed->yVis = (a->h - paddingTop - paddingBottom) / ed->lineSkip;
	ed->r.x = paddingLeft;
	ed->r.y = paddingTop;
	ed->r.w = a->w - paddingLeft - paddingRight - 1;
	ed->r.h = a->h - paddingTop - paddingBottom - 1;

	r.x = 0;
	r.y = 0;
	r.w = a->w;
	r.h = a->h;
	AG_SetStockCursor(ed, &ed->ca, &r, AG_TEXT_CURSOR);
	return (0);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int keysym = AG_INT(1);
	const int keymod = AG_INT(2);
	const AG_Char ch = AG_CHAR(3);
	int rv;

	switch (keysym) {
	case AG_KEY_RETURN:
	case AG_KEY_KP_ENTER:
		if ((ed->flags & AG_EDITABLE_MULTILINE) == 0) {
			ed->returnHeld = 1;
		}
		break;
	case AG_KEY_LSHIFT:
	case AG_KEY_RSHIFT:
		ed->flags |= AG_EDITABLE_SHIFT_SELECT;
		ed->posKbdSel = ed->pos;
		return;
	case AG_KEY_LALT:
	case AG_KEY_RALT:
	case AG_KEY_LMETA:
	case AG_KEY_RMETA:
	case AG_KEY_LCTRL:
	case AG_KEY_RCTRL:
		return;
	case AG_KEY_TAB:
		if (ed->complete &&                           /* Autocomplete */
		   (ed->flags & AG_EDITABLE_MULTILINE) == 0) {
			AG_Window *win;
			AG_Tlist *tl;

			if (ed->complete->winName[0] == '\0') {
				AG_AddTimer(ed, &ed->complete->to,
				    agAutocompleteDelay,
				    AutocompleteTimeout, NULL);
				AG_ExecTimer(&ed->complete->to);
			}
			if ((win = AG_WindowFind(ed->complete->winName)) &&
			    (tl = AG_ObjectFindChild(win, "suggestions")) != NULL) {
				AG_TlistSelectIdx(tl, 0);
			}
			return;
		} else if (!(WIDGET(ed)->flags & AG_WIDGET_CATCH_TAB)) {
			return;
		}
		break;
	}

	ed->flags |= AG_EDITABLE_BLINK_ON;

	rv = ProcessKey(ed, keysym, keymod, ch);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
	case AG_KEY_LEFT:
		if (rv == 1) {
			AG_AddTimer(ed, &ed->toRepeatDirs[keysym - AG_KEY_UP],
			    agKbdDelay,
			    KeyRepeatTimeout, "%i,%i,%lu", keysym, keymod, ch);
		} else {
			AG_DelTimer(ed, &ed->toRepeatDirs[keysym - AG_KEY_UP]);
		}
	default:
		if (rv == 1) {
			AG_AddTimer(ed, &ed->toRepeat, agKbdDelay,
			    KeyRepeatTimeout, "%i,%i,%lu", keysym, keymod, ch);
		} else {
			AG_DelTimer(ed, &ed->toRepeat);
		}
		break;
	}
	AG_Redraw(ed);
}

static void
AutocompletePoll(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Editable *ed = AG_EDITABLE_PTR(1);

	if (ed->complete && ed->complete->fn)
		AG_PostEventByPtr(ed, ed->complete->fn, "%p", tl);
}

static void
AutocompleteSelected(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);

	AG_EditableSetString(ed, it->text);

	AG_PostEvent(WIDGET(tl)->window, "window-close", NULL);
}

static Uint32
AutocompleteTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	AG_Window *win, *winParent = WIDGET(ed)->window;
	AG_Tlist *tl;
	int xParent, yParent;

	if (AGDRIVER_MULTIPLE(WIDGET(ed)->drv)) {
		xParent = WIDGET(winParent)->x;
		yParent = WIDGET(winParent)->y;
	} else {
		xParent = 0;
		yParent = 0;
	}

	if (ed->complete->winName[0] != '\0') {            /* Focus existing */
		if ((win = AG_WindowFind(ed->complete->winName)) != NULL) {
			AG_WindowSetGeometry(win,
			    xParent + WIDGET(ed)->rView.x1,
			    yParent + WIDGET(ed)->rView.y1 + HEIGHT(ed),
			    WIDTH(ed), 200);
			AG_WindowShow(win);
			AG_WindowFocus(win);
		}
		return (0);
	}

	win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_DENYFOCUS |
	                   AG_WINDOW_NOTITLE | AG_WINDOW_KEEPABOVE);
	if (win == NULL) {
		return (0);
	}
	win->wmType = AG_WINDOW_WM_COMBO;
	AG_WindowAttach(winParent, win);
	AG_WindowSetCloseAction(win, AG_WINDOW_HIDE);
	AG_SetStyle(win, "padding", "0");

	tl = AG_TlistNewPolledMs(win, AG_TLIST_EXPAND, agAutocompleteRate,
	    AutocompletePoll, "%p", ed);
	AG_ObjectSetNameS(tl, "suggestions");
	AG_SetEvent(tl, "tlist-selected", AutocompleteSelected, "%p", ed);

	AG_WindowSetGeometry(win,
	    xParent + WIDGET(ed)->rView.x1,
	    yParent + WIDGET(ed)->rView.y1 + HEIGHT(ed),
	    WIDTH(ed),
	    200);

	AG_WindowShow(win);

	Strlcpy(ed->complete->winName, OBJECT(win)->name,
	    sizeof(ed->complete->winName));

	return (0);
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int keysym = AG_INT(1);
	
	AG_DelTimer(ed, &ed->toRepeat);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
	case AG_KEY_LEFT:
		AG_DelTimer(ed, &ed->toRepeatDirs[keysym - AG_KEY_UP]);
		break;
	case AG_KEY_LSHIFT:
	case AG_KEY_RSHIFT:
		ed->flags &= ~(AG_EDITABLE_SHIFT_SELECT);
		ed->posKbdSel = 0;
		return;
	case AG_KEY_RETURN:
	case AG_KEY_KP_ENTER:
		if ((ed->flags & AG_EDITABLE_MULTILINE) == 0) {
			if (ed->flags & AG_EDITABLE_ABANDON_FOCUS) {
				AG_WidgetUnfocus(ed);
			}
			AG_PostEvent(ed, "editable-return", NULL);
			ed->returnHeld = 0;
			AG_Redraw(ed);
		}
		break;
	default:
		break;
	}
	
	if (ed->complete) {
		switch (keysym) {
		case AG_KEY_HOME:
		case AG_KEY_END:
		case AG_KEY_LEFT:
		case AG_KEY_RIGHT:
		case AG_KEY_UP:
		case AG_KEY_DOWN:
		case AG_KEY_PAGEUP:
		case AG_KEY_PAGEDOWN:
			/* Don't let directional keys trigger autocomplete */
			break;
		default:
			AG_AddTimer(ed, &ed->complete->to, agAutocompleteDelay,
			    AutocompleteTimeout, NULL);
			break;
		}
	}
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
AG_EditableAutocomplete(AG_Editable *ed, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (fn == NULL) {                           /* Disable autocomplete */
		if (ed->complete) {
			AG_EditableCloseAutocomplete(ed);
			AG_UnsetEventByPtr(ed, ed->complete->fn);
			free(ed->complete);
			ed->complete = NULL;
			WIDGET(ed)->flags &= ~(AG_WIDGET_CATCH_TAB);
		}
		goto out;
	}
	if (ed->complete == NULL) {
		ed->complete = Malloc(sizeof(AG_Autocomplete));
		AG_InitTimer(&ed->complete->to, "complete", 0);
	} else {                                                /* Replace */
		AG_UnsetEventByPtr(ed, ed->complete->fn);
		AG_DelTimer(ed, &ed->complete->to);
	}
	ed->complete->fn = AG_SetEvent(ed, NULL, fn, NULL);
	ed->complete->winName[0] = '\0';

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ed->complete->fn, fmt, ap);
		va_end(ap);
	}
	WIDGET(ed)->flags |= AG_WIDGET_CATCH_TAB;
out:
	AG_ObjectUnlock(ed);
}

/* Close any active autocomplete windows and cancel any running timers. */
void
AG_EditableCloseAutocomplete(AG_Editable *ed)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");

	if (ed->complete == NULL)
		return;

	AG_DelTimer(ed, &ed->complete->to);

	if (ed->complete->winName[0] != '\0') {
		AG_Window *win;

		if ((win = AG_WindowFind(ed->complete->winName)) != NULL)
			AG_PostEvent(win, "window-close", NULL);
	}
}

static void
DoubleClick(AG_Editable *_Nonnull ed)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_EditableBuffer *buf;
	const AG_Char *c, *cEnd, *cAns;
	AG_TextANSI ansi;
	int pos, offsAns, ansiLen;

	AG_DelTimer(ed, &ed->toDblClick);
	ed->flags |= AG_EDITABLE_WORDSELECT;
	ed->selDblClick = -1;

	if ((buf = GetBuffer(ed)) == NULL) {
		return;
	}
	if (buf->len == 0) {
		goto out;
	}
	if ((pos = ed->pos) < 0) {                       /* Select first word */
		/* TODO */
		goto out;
	} else if (pos >= buf->len) {                     /* Select last word */
		pos = buf->len-1;
		c = &buf->s[pos];
		if (*c == '\n') {
			goto out;
		}
		while (AG_CharIsSpace(*c) && c >= &buf->s[0]) {
			c--;
			pos--;
		}
		for (ed->selStart = pos, ed->selEnd = pos+1;
		     c >= &buf->s[0];
		     c--, ed->selStart--) {
			if (AG_CharIsSpaceOrLF(*c)) {
				ed->selStart++;
				break;
			}
		}
		goto out;
	}

	cEnd = &buf->s[buf->len];
	c = &buf->s[pos];
	if (*c == '\n')                                      /* An empty line */
		goto out;

	if (AG_CharIsSpace(*c)) {             /* Select contiguous whitespace */
		for (ed->selStart = pos-1, c--;
		     c >= &buf->s[0];
		     c--) {
			if (!AG_CharIsSpace(*c)) {
				break;
			}
			ed->selStart--;
		}
		ed->selStart++;
		c = &buf->s[pos];
		for (ed->selEnd = pos; c < cEnd; c++) {
			if (!AG_CharIsSpace(*c)) {
				break;
			}
			ed->selEnd++;
		}
		if (ed->selStart < ed->selEnd) {
			ed->pos = ed->selStart;
		}
		goto out;
	} else if (AG_CharIsPunct(*c)) {     /* Select a block of punctuation */
		for (ed->selStart = pos;
		     c >= &buf->s[0];
		     c--) {
			if (!AG_CharIsPunct(*c)) {
				break;
			}
			ed->selStart--;
		}
		ed->selStart++;
		c = &buf->s[pos];
		for (ed->selEnd = pos; c < cEnd; c++) {
			if (!AG_CharIsPunct(*c)) {
				break;
			}
			ed->selEnd++;
		}
		if (ed->selStart < ed->selEnd) {
			ed->pos = ed->selStart;
		}
		goto out;
	}

	/* Seek backwards for an ANSI sequence and remember its length. */
	for (cAns=c, offsAns=0, ansiLen=0;
	     cAns >= &buf->s[0] && offsAns < AG_TEXT_ANSI_SEQ_MAX;
	     cAns--, offsAns++) {
		if (cAns[0] == 0x1b &&
		    cAns[1] >= 0x40 && cAns[1] <= 0x5f &&
		    cAns[2] != '\0' &&
		    AG_TextParseANSI(ts, &ansi, &cAns[1]) == 0) {
			ansiLen = ansi.len;
			break;
		}
	}

	/*
	 * Select a word, including ANSI sequences but excluding adjacent
	 * punctuation (further "mouse-motion" would include punctuation).
	 */
	for (ed->selStart = pos; c >= &buf->s[0]; c--) {
		if (*c == '\n' || AG_CharIsSpaceOrPunct(*c)) {
			ed->selStart++;
			break;
		}
		if (ansiLen > 0 && (ed->selStart == (pos - offsAns + ansiLen))) {
			ed->selStart -= ansiLen;
			break;
		}
		ed->selStart--;
	}
	if (ed->selStart < 0)
		ed->selStart = 0;

	c = &buf->s[pos];
	for (ed->selEnd = pos; c < cEnd; c++) {
		if (*c == '\n' || AG_CharIsSpaceOrPunct(*c)) {
			break;
		}
		if (c[0] == 0x1b &&
		    c[1] >= 0x40 && c[1] <= 0x5f &&
		    c[2] != '\0' &&
		    AG_TextParseANSI(ts, &ansi, &c[1]) == 0) {
			ed->selEnd += ansi.len+1;
			break;
		}
		ed->selEnd++;
	}
	if (ed->selEnd < ed->selStart) {
		int temp = ed->selStart;
		ed->selStart = ed->selEnd;
		ed->selEnd = temp;
	}
out:
	ReleaseBuffer(ed, buf);
}

/* Copy specified range to given internal clipboard. */
void
AG_EditableCopyChunk(AG_Editable *ed, AG_EditableClipboard *cb, AG_Char *s,
    AG_Size len)
{
	AG_Char *sNew;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);
	AG_MutexLock(&cb->lock);

	sNew = TryRealloc(cb->s, (len+1)*sizeof(AG_Char));
	if (sNew != NULL) {
		cb->s = sNew;
		memcpy(cb->s, s, len*sizeof(AG_Char));
		cb->s[len] = '\0';
		cb->len = len;
	}
	Strlcpy(cb->encoding, ed->encoding, sizeof(cb->encoding));

	AG_MutexUnlock(&cb->lock);
	AG_ObjectUnlock(ed);
}

/*
 * Perform Cut action on buffer.
 * Return 1 = buffer changed or 0 = copy failed or no change.
 */
int
AG_EditableCut(AG_Editable *ed, AG_EditableBuffer *buf, AG_EditableClipboard *cb,
    int internalOnly)
{
	AG_Size selLen;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	selLen = (ed->selEnd - ed->selStart);
	if (AGEDITABLE_IS_READONLY(ed) || selLen == 0) {
		goto no_change;
	}
	AG_EditableValidateSelection(ed, buf);

	if (AG_EditableCopy(ed, buf, cb, internalOnly)) {
		AG_EditableDelete(ed, buf);
	}
	AG_ObjectUnlock(ed);
	return (1);
no_change:
	AG_ObjectUnlock(ed);
	return (1);
}

/*
 * Perform Copy action against clipboard (internal or external).
 * Return 1 = buffer changed or 0 = copy failed or no change.
 */
int
AG_EditableCopy(AG_Editable *ed, AG_EditableBuffer *buf,
    AG_EditableClipboard *cb, int internalOnly)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	const AG_DriverClass *drvOps = AGDRIVER_CLASS(drv);
	AG_Size selLen;
	const int selStart = ed->selStart;
	const int selEnd   = ed->selEnd;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	AG_EditableValidateSelection(ed, buf);

	if ((selLen = (selEnd - selStart)) == 0)
		goto no_change;

	if (internalOnly || !agClipboardIntegration || /* Internal clipboard */
	    drvOps->setClipboardText == NULL) {
		AG_EditableCopyChunk(ed, cb,
		    &buf->s[selStart], selLen);
	} else {                                     /* Integrated clipboard */
		char *s;
#ifdef AG_UNICODE
		AG_Char cEndSave;
		AG_Size utf8len;

		cEndSave = buf->s[selEnd];
		buf->s[selEnd] = '\0';
		if (AG_LengthUTF8FromUCS4(&buf->s[selStart], &utf8len) == -1) {
			buf->s[selEnd] = cEndSave;
			goto no_change;
		}
		utf8len++;
#if 0
		Debug(ed, "selLen=%ld, utf8len=%ld, startChar=%c, lastChar=%c\n",
		    (long)selLen, (long)utf8len, (char)buf->s[selStart], (char)cEndSave);
#endif
		if ((s = TryMalloc(utf8len)) == NULL) {
			Verbose(_("Out of memory for Copy (%lu bytes)\n"), (Ulong)utf8len);
			buf->s[selEnd] = cEndSave;
			goto no_change;
		}
		if (AG_ExportUnicode("UTF-8", s, &buf->s[selStart], utf8len)
		    == -1) {
			buf->s[selEnd] = cEndSave;
			Verbose(_("Copy failed (%s)\n"), AG_GetError());
			goto no_change;
		}
		buf->s[selEnd] = cEndSave;
#else
		if ((s = TryMalloc(selLen+1)) == NULL) {
			Verbose(_("Out of memory for Copy (%lu bytes)\n"),
			    selLen+1);
			goto no_change;
		}
		memcpy(s, &buf->s[selStart], selLen);
		s[selLen] = '\0';
#endif /* !AG_UNICODE */

		if (drvOps->setClipboardText(drv, s) == -1) {
			free(s);
			goto no_change;
		}
		free(s);
	}
	AG_ObjectUnlock(ed);
	return (1);
no_change:
	AG_ObjectUnlock(ed);
	return (0);
}

/*
 * Perform Paste action from clipboard (internal or external).
 * Return 1 = buffer changed (paste ok) or 0 = paste failed or empty.
 */
int
AG_EditablePaste(AG_Editable *ed, AG_EditableBuffer *buf,
    AG_EditableClipboard *cb, int internalOnly)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	const AG_DriverClass *drvOps = AGDRIVER_CLASS(drv);

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (AGEDITABLE_IS_READONLY(ed))
		goto no_change;

	if (ed->selEnd > ed->selStart)
		AG_EditableDelete(ed, buf);

	if (!internalOnly && agClipboardIntegration &&
	    drvOps->getClipboardText != NULL) {     /* Integrated clipboard */
		char *s, *c;
		AG_Char *ucs;
		AG_Size ucsLen;

		if ((s = drvOps->getClipboardText(drv)) == NULL) {
			goto out;
		}
		if (!(ed->flags & AG_EDITABLE_MULTILINE) &&
		    (c = strchr(s, '\n')) != NULL) {
			*c = '\0';
		}

		Debug(drv, "GetClipboardText() -> \"%s\"\n", s);

		if (ed->flags & AG_EDITABLE_INT_ONLY) {
			for (c = &s[0]; *c != '\0'; c++) {
				if (!CharIsIntOnly(*c)) {
					AG_SetError(_("Non-integer input near `%c'"), (char)*c);
					goto fail;
				}
			}
		} else if (ed->flags & AG_EDITABLE_FLT_ONLY) {
			for (c = &s[0]; *c != '\0'; c++) {
				if (!CharIsFltOnly(*c)) {
					AG_SetError(_("Non-float input near `%c'"), (char)*c);
					goto fail;
				}
			}
		}

		if ((ucs = AG_ImportUnicode("UTF-8", s, &ucsLen, NULL)) == NULL) {
			goto fail;
		}
		if (AG_EditableGrowBuffer(ed, buf, ucs, ucsLen) == -1) {
			free(ucs);
			goto fail;
		}
		if (ed->pos < buf->len) {
			memmove(&buf->s[ed->pos + ucsLen], &buf->s[ed->pos],
			    (buf->len - ed->pos)*sizeof(AG_Char));
		}
		memcpy(&buf->s[ed->pos], ucs, ucsLen*sizeof(AG_Char));
		buf->len += ucsLen;
		buf->s[buf->len] = '\0';
		ed->pos += ucsLen;

		free(ucs);
		free(s);
	} else {                                      /* Internal clipboard */
		AG_Char *c;

		AG_MutexLock(&cb->lock);

		if (cb->s == NULL) {
			AG_MutexUnlock(&cb->lock);
			goto out;
		}
		if (!(ed->flags & AG_EDITABLE_MULTILINE)) {
			for (c = &cb->s[0]; *c != '\0'; c++) {
				if (*c == '\n') {
					*c = '\0';
					break;
				}
			}
		}
		if (ed->flags & AG_EDITABLE_INT_ONLY) {
			for (c = &cb->s[0]; *c != '\0'; c++) {
				if (!CharIsIntOnly(*c)) {
					AG_SetError(_("Non-integer input near `%c'"), (char)*c);
					AG_MutexUnlock(&cb->lock);
					goto fail;
				}
			}
		} else if (ed->flags & AG_EDITABLE_FLT_ONLY) {
			for (c = &cb->s[0]; *c != '\0'; c++) {
				if (!CharIsFltOnly(*c)) {
					AG_SetError(_("Non-float input near `%c'"), (char)*c);
					AG_MutexUnlock(&cb->lock);
					goto fail;
				}
			}
		}

		if (AG_EditableGrowBuffer(ed, buf, cb->s, cb->len) == -1) {
			AG_MutexUnlock(&cb->lock);
			goto fail;
		}
		if (ed->pos < buf->len) {
			memmove(&buf->s[ed->pos + cb->len], &buf->s[ed->pos],
			    (buf->len - ed->pos)*sizeof(AG_Char));
		}
		memcpy(&buf->s[ed->pos], cb->s, cb->len*sizeof(AG_Char));
		buf->len += cb->len;
		buf->s[buf->len] = '\0';
		ed->pos += cb->len;
		AG_MutexUnlock(&cb->lock);
	}
out:
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_ObjectUnlock(ed);
	return (1);
fail:
	Verbose(_("Paste Failed (%s)\n"), AG_GetError());
no_change:
	AG_ObjectUnlock(ed);
	return (0);
}

/*
 * Delete the current selection.
 * Return 1 = buffer changed (delete OK) or 0 = delete failed or empty.
 */
int
AG_EditableDelete(AG_Editable *ed, AG_EditableBuffer *buf)
{
	AG_Size selLen;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	selLen = (ed->selEnd - ed->selStart);
	if (AGEDITABLE_IS_READONLY(ed) || selLen == 0)
		goto no_change;

	AG_EditableValidateSelection(ed, buf);
	if (ed->selEnd == buf->len) {
		buf->s[ed->selStart] = '\0';
	} else {
		memmove(&buf->s[ed->selStart], &buf->s[ed->selEnd],
		    (buf->len - selLen + 1 - ed->selStart)*sizeof(AG_Char));
	}
	buf->len -= selLen;
	ed->pos = ed->selStart;
	ed->selStart = 0;
	ed->selEnd = 0;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;

	AG_ObjectUnlock(ed);
	return (1);
no_change:
	AG_ObjectUnlock(ed);
	return (0);
}

/* Perform "Select All" on buffer. */
void
AG_EditableSelectAll(AG_Editable *ed, AG_EditableBuffer *buf)
{
	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

/*	ed->pos = 0; */
	ed->selStart = 0;
	ed->selEnd = buf->len;

	AG_Redraw(ed);
	AG_ObjectUnlock(ed);
}

/*
 * Right-click popup menu actions.
 */
static void
MenuCut(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		if (AG_EditableCut(ed, buf, &agEditableClipbrd, 0)) {
			CommitBuffer(ed, buf);
		}
		ReleaseBuffer(ed, buf);
	}
}
static void
MenuCutActive(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	int *enable = AG_PTR(2);

	*enable = (!AG_EditableReadOnly(ed) && (ed->selEnd > ed->selStart));
}

static void
MenuCopy(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		AG_EditableCopy(ed, buf, &agEditableClipbrd, 0);
		ReleaseBuffer(ed, buf);
	}
}
static void
MenuCopyActive(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	int *enable = AG_PTR(2);

	*enable = (ed->selEnd > ed->selStart);
}

static void
MenuPaste(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		if (AG_EditablePaste(ed, buf, &agEditableClipbrd, 0)) {
			CommitBuffer(ed, buf);
		}
		ReleaseBuffer(ed, buf);
	}
}
static void
MenuPasteActive(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	int *enable = AG_PTR(2);
#if 1
	*enable = (!AG_EditableReadOnly(ed) && (agClipboardIntegration ||
	                                       (agEditableClipbrd.len > 0)));
#else
	AG_Driver *drv = WIDGET(ed)->drv;
	const AG_DriverClass *drvOps = AGDRIVER_CLASS(drv);
	char *s;

	if (AG_EditableReadOnly(ed)) {
		*enable = 0;
		return;
	}
	if (!agClipboardIntegration) {
		*enable = (agEditableClipbrd.len > 0);
	} else {
		if (drvOps->getClipboardText == NULL) {
			*enable = 1;
			return;
		}
		if ((s = drvOps->getClipboardText(drv)) != NULL) {
			*enable = !( (s[0] == '\0' || (s[0] == ' ' &&
			                               s[1] == '\0')) );
			free(s);
		} else {
			*enable = 1;
		}
	}
#endif
}

static void
MenuDelete(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		if (AG_EditableDelete(ed, buf)) {
			CommitBuffer(ed, buf);
		}
		ReleaseBuffer(ed, buf);
	}
}
static void
MenuDeleteActive(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	int *enable = AG_PTR(2);

	*enable = (!AG_EditableReadOnly(ed) && (ed->selEnd > ed->selStart));
}

static void
MenuSelectAll(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		AG_EditableSelectAll(ed, buf);
		ReleaseBuffer(ed, buf);
	}
}

#ifdef AG_UNICODE
static void
MenuSetLang(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	const enum ag_language lang = (enum ag_language)AG_INT(2);

	AG_EditableSetLang(ed, lang);
}
#endif

static AG_PopupMenu *_Nullable
PopupMenu(AG_Editable *_Nonnull ed)
{
	AG_PopupMenu *pm;
	AG_MenuItem *mi;
#ifdef AG_UNICODE
	AG_Variable *vText;
	AG_TextElement *txt;
#endif
	if ((pm = AG_PopupNew(ed)) == NULL) {
		return (NULL);
	}

	AG_MenuSeparator(pm->root);

	mi = AG_MenuAction(pm->root, _("Cut"), NULL, MenuCut, "%p", ed);
	mi->stateFn = AG_SetEvent(pm->menu, NULL, MenuCutActive, "%p", ed);

	mi = AG_MenuAction(pm->root, _("Copy"), NULL, MenuCopy, "%p", ed);
	mi->stateFn = AG_SetEvent(pm->menu, NULL, MenuCopyActive, "%p", ed);

	mi = AG_MenuAction(pm->root, _("Paste"), NULL, MenuPaste, "%p", ed);
	mi->stateFn = AG_SetEvent(pm->menu, NULL, MenuPasteActive, "%p", ed);

	mi = AG_MenuAction(pm->root, _("Delete"), NULL, MenuDelete, "%p", ed);
	mi->stateFn = AG_SetEvent(pm->menu, NULL, MenuDeleteActive, "%p", ed);

	AG_MenuSeparator(pm->root);

	AG_MenuAction(pm->root, _("Select All"), NULL, MenuSelectAll, "%p", ed);

#ifdef AG_UNICODE
	if ((ed->flags & AG_EDITABLE_MULTILINGUAL) &&
	    AG_Defined(ed, "text") &&
	    (vText = AG_GetVariable(ed, "text", (void *)&txt)) != NULL) {
		AG_MenuItem *mLang;
		int i;
		
		AG_MenuSeparator(pm->root);
		mLang = AG_MenuNode(pm->root, _("Select Language"), NULL);
		for (i = 0; i < AG_LANG_LAST; i++) {
			AG_TextEnt *te = &txt->ent[i];

			if (te->len == 0)
				continue;

			AG_MenuAction(mLang, _(agLanguageNames[i]),
			    NULL, MenuSetLang, "%p,%i", ed, i);
		}
		AG_UnlockVariable(vText);
	}
#endif /* AG_UNICODE */

	return (pm);
}

/* Timer for detecting double clicks. */
static Uint32
DoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();

	ed->selDblClick = -1;
	return (0);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int btn = AG_INT(1);
	const int mx = AG_INT(2) - WIDGET(ed)->paddingLeft;
	const int my = AG_INT(3) - WIDGET(ed)->paddingTop;
	AG_EditableBuffer *buf;

	if (!AG_WidgetIsFocused(ed))
		AG_WidgetFocus(ed);

	switch (btn) {
	case AG_MOUSE_LEFT:
		if (ed->pm != NULL) {
			AG_PopupHide(ed->pm);
		}
		ed->flags |= AG_EDITABLE_CURSOR_MOVING | AG_EDITABLE_BLINK_ON;
		if ((buf = GetBuffer(ed)) == NULL) {
			return;
		}
		if (AG_EditableMapPosition(ed, buf, (ed->x + mx), my, &ed->pos) == 0) {
			ed->selStart = 0;
			ed->selEnd = 0;
			AG_Redraw(ed);
		}
		ReleaseBuffer(ed, buf);
		ed->flags |= AG_EDITABLE_MARKPREF;

		if (ed->selDblClick != -1 && abs(ed->selDblClick - ed->pos) <= 1) {
			DoubleClick(ed);
		} else {
			ed->selDblClick = ed->pos;
			AG_AddTimer(ed, &ed->toDblClick, agMouseDblclickDelay,
			    DoubleClickTimeout, NULL);
		}
		break;
	case AG_MOUSE_RIGHT:
		if ((ed->flags & AG_EDITABLE_NOPOPUP) == 0) {
			if (ed->pm != NULL) {
				AG_PopupShowAt(ed->pm, mx, my);
			} else {
				if ((ed->pm = PopupMenu(ed)) != NULL)
					AG_PopupShowAt(ed->pm, mx,my);
			}
		}
		break;
	case AG_MOUSE_WHEELUP:
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			ed->y -= ed->lineScrollAmount;
			if (ed->y < 0) { ed->y = 0; }
		}
		break;
	case AG_MOUSE_WHEELDOWN:
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			ed->y += ed->lineScrollAmount;
			ed->y = MIN(ed->y, ed->yMax - ed->yVis);
		}
		break;
	}
	
	AG_Redraw(ed);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int button = AG_INT(1);
	const int x = AG_INT(2) - WIDGET(ed)->paddingLeft;
	const int y = AG_INT(3) - WIDGET(ed)->paddingTop;

	if (button != AG_MOUSE_LEFT)
		return;
	
	ed->flags &= ~(AG_EDITABLE_WORDSELECT);

	if ((ed->flags & AG_EDITABLE_CURSOR_MOVING)) {
		AG_EditableBuffer *buf;

		ed->flags &= ~(AG_EDITABLE_CURSOR_MOVING);

		if ((buf = GetBuffer(ed)) == NULL) {
			return;
		}
		AG_EditableMapPosition(ed, buf, ed->x + x, y, &ed->pos);
		ReleaseBuffer(ed, buf);
		ed->flags |= AG_EDITABLE_MARKPREF;
	}

	AG_Redraw(ed);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	AG_EditableBuffer *buf;
	const int mx = AG_INT(1) - WIDGET(ed)->paddingLeft;
	const int my = AG_INT(2) - WIDGET(ed)->paddingTop;
	int newPos;

	if (!AG_WidgetIsFocused(ed) ||
	    (ed->flags & AG_EDITABLE_CURSOR_MOVING) == 0)
		return;

	if ((buf = GetBuffer(ed)) == NULL)
		return;

	if (AG_EditableMapPosition(ed, buf, ed->x + mx, my, &newPos) == -1 ||
	    newPos == ed->pos)
		goto out;

	if (ed->flags & AG_EDITABLE_WORDSELECT) {           /* Word selection */
		const AG_Char *c, *cEnd = &buf->s[buf->len];

		c = &buf->s[newPos];
		if (newPos > ed->pos) {
			ed->selStart = ed->pos;
			for (c = &buf->s[ed->pos]; c >= &buf->s[0]; c--) {
				if (*c == '\n' || AG_CharIsSpace(*c)) {
					break;
				}
				ed->selStart--;
			}
			ed->selStart++;
			ed->selEnd = newPos;
			for (c = &buf->s[newPos]; c <= cEnd; c++) {
				if (*c == '\n' || AG_CharIsSpace(*c)) {
					break;
				}
				ed->selEnd++;
			}
			ed->xScrollTo = &ed->xSelEnd;
			ed->yScrollTo = &ed->ySelEnd;
		} else if (newPos < ed->pos) {
			ed->selStart = newPos;
			for (c = &buf->s[newPos]; c >= &buf->s[0]; c--) {
				if (*c == '\n' || AG_CharIsSpace(*c)) {
					break;
				}
				ed->selStart--;
			}
			ed->selStart++;
			ed->selEnd = ed->pos;
			for (c = &buf->s[ed->pos]; c < cEnd; c++) {
				if (*c == '\n' || AG_CharIsSpace(*c)) {
					break;
				}
				ed->selEnd++;
			}
			ed->xScrollTo = &ed->xSelStart;
			ed->yScrollTo = &ed->ySelStart;
		}
	} else {                                       /* Character selection */
		if (newPos > ed->pos) {
			ed->selStart  = ed->pos;
			ed->selEnd    = newPos;
			ed->xScrollTo = &ed->xSelEnd;
			ed->yScrollTo = &ed->ySelEnd;
		} else if (newPos <= ed->pos) {
			ed->selStart  = newPos;
			ed->selEnd    = ed->pos;
			ed->xScrollTo = &ed->xSelStart;
			ed->yScrollTo = &ed->ySelStart;
		}
	}
	AG_Redraw(ed);
out:
	ReleaseBuffer(ed, buf);
}

/* Clear the buffer contents. */
void
AG_EditableClearString(AG_Editable *ed)
{
	AG_EditableSetString(ed, NULL);
}

/*
 * Replace the buffer contents with a formatted string.
 * Cancel any selection and move the cursor to the end.
 * If fmt is NULL, replace with empty string ("").
 */
void
AG_EditablePrintf(AG_Editable *ed, const char *fmt, ...)
{
	va_list ap;
	char *s;
	
	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	AG_EditableSetString(ed, s);
	free(s);
}

/*
 * Replace the buffer contents with a given string.
 * Cancel any selection and move the cursor to the end.
 * If text is NULL, replace with empty string ("").
 */
void
AG_EditableSetString(AG_Editable *ed, const char *text)
{
	AG_EditableBuffer *buf;
	AG_Char *sNew;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if ((buf = GetBuffer(ed)) == NULL) {
		goto out;
	}
	if (text != NULL) {
#ifdef AG_UNICODE
		if ((sNew = AG_ImportUnicode("UTF-8", text, &buf->len, &buf->maxLen)) == NULL)
			goto out;
#else
		if ((sNew = (Uint8 *)TryStrdup(text)) == NULL) {
			goto out;
		}
		buf->maxLen = buf->len = strlen(text);
#endif
		Free(buf->s);
		buf->s = sNew;
		ed->pos = buf->len;
	} else {                                            /* Empty string */
		if ((sNew = TryMalloc(sizeof(AG_Char))) == NULL) {
			goto out;
		}
		sNew[0] = '\0';
		Free(buf->s);
		buf->s = sNew;
		ed->pos = 0;
		buf->len = 0;
	}
	ed->selStart = 0;
	ed->selEnd = 0;
	CommitBuffer(ed, buf);
	ReleaseBuffer(ed, buf);
out:
	AG_Redraw(ed);
	AG_ObjectUnlock(ed);
}


/* Return a duplicate of the buffer contents. */
char *
AG_EditableDupString(AG_Editable *ed)
{
	AG_Variable *V;
	char *sDup, *s;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (AG_Defined(ed, "text")) {
		AG_TextElement *txt;

		V = AG_GetVariable(ed, "text", (void *)&txt);
		s = txt->ent[ed->lang].buf;
		sDup = TryStrdup((s != NULL) ? s : "");
	} else {
		V = AG_GetVariable(ed, "string", (void *)&s);
		sDup = TryStrdup(s);
	}
	AG_UnlockVariable(V);

	AG_ObjectUnlock(ed);
	return (sDup);
}

/* Copy buffer contents to a fixed-size buffer (and NUL-terminate always). */
AG_Size
AG_EditableCopyString(AG_Editable *ed, char *dst, AG_Size dst_size)
{
	AG_Variable *V;
	AG_Size rv;
	char *s;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (AG_Defined(ed, "text")) {
		AG_TextElement *txt;

		V = AG_GetVariable(ed, "text", (void *)&txt);
		s = txt->ent[ed->lang].buf;
		rv = Strlcpy(dst, (s != NULL) ? s : "", dst_size);
	} else {
		V = AG_GetVariable(ed, "string", (void *)&s);
		rv = Strlcpy(dst, s, dst_size);
	}
	AG_UnlockVariable(V);

	AG_ObjectUnlock(ed);
	return (rv);
}

/* Append a string to the contents of the text buffer. */
int
AG_EditableCatString(AG_Editable *ed, const char *fmt, ...)
{
	char *s;
	va_list ap;
	int rv;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	rv = AG_EditableCatStringS(ed, s);
	free(s);
	return (rv);
}

/* Append a string to the end of the text buffer. */
int
AG_EditableCatStringS(AG_Editable *ed, const char *s)
{
	AG_Variable *V;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if (AG_Defined(ed, "text")) {
		AG_TextElement *txt;
		AG_Language langSave;

		V = AG_GetVariable(ed, "text", (void *)&txt);
		langSave = txt->lang;
		txt->lang = ed->lang;
		if (AG_TextCatS(txt, s) == -1) {		/* XXX pos */
			txt->lang = langSave;
			goto fail;
		}
		txt->lang = langSave;
	} else {
		AG_EditableBuffer *buf;
		AG_Char *ucs;
		char *dst;
		AG_Size ucsLen;

		V = AG_GetVariable(ed, "string", (void *)&dst);
		if ((ucs = AG_ImportUnicode(ed->encoding, s, &ucsLen, NULL)) == NULL) {
			goto fail;
		}
		if ((buf = GetBuffer(ed)) == NULL) {
			free(ucs);
			goto fail;
		}
		if (AG_EditableGrowBuffer(ed, buf, ucs, ucsLen) == -1) {
			ReleaseBuffer(ed, buf);
			free(ucs);
			goto fail;
		}
		memcpy(&buf->s[buf->len], ucs, ucsLen*sizeof(AG_Char));
		buf->len += ucsLen;
		ed->pos += ucsLen;
		CommitBuffer(ed, buf);
		ReleaseBuffer(ed, buf);
		free(ucs);
	}
	AG_UnlockVariable(V);

	AG_Redraw(ed);
	AG_ObjectUnlock(ed);
	return (0);
fail:
	AG_UnlockVariable(V);
	AG_ObjectUnlock(ed);
	return (-1);
}

/* Convert buffer contents to an integer value and return it. */
int
AG_EditableInt(AG_Editable *ed)
{
	char abuf[32];
	AG_EditableBuffer *buf;
	int i;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if ((buf = GetBuffer(ed)) == NULL)
		AG_FatalError(NULL);
#ifdef AG_UNICODE
	AG_ExportUnicode("UTF-8", abuf, buf->s, sizeof(abuf));
#else
	Strlcpy(abuf, (const char *)buf->s, sizeof(abuf));
#endif
	i = atoi(abuf);
	ReleaseBuffer(ed, buf);

	AG_ObjectUnlock(ed);
	return (i);
}

/* Convert buffer contents to a single-precision float and return it. */
float
AG_EditableFlt(AG_Editable *ed)
{
	char abuf[32];
	AG_EditableBuffer *buf;
	float flt;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if ((buf = GetBuffer(ed)) == NULL)
		AG_FatalError(NULL);
#ifdef AG_UNICODE
	AG_ExportUnicode("UTF-8", abuf, buf->s, sizeof(abuf));
#else
	Strlcpy(abuf, (const char *)buf->s, sizeof(abuf));
#endif
	flt = (float)strtod(abuf, NULL);
	ReleaseBuffer(ed, buf);

	AG_ObjectUnlock(ed);
	return (flt);
}

/* Convert buffer contents to a double and return it. */
double
AG_EditableDbl(AG_Editable *ed)
{
	char abuf[32];
	AG_EditableBuffer *buf;
	double flt;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	if ((buf = GetBuffer(ed)) == NULL)
		AG_FatalError(NULL);
#ifdef AG_UNICODE
	AG_ExportUnicode("UTF-8", abuf, buf->s, sizeof(abuf));
#else
	Strlcpy(abuf, (const char *)buf->s, sizeof(abuf));
#endif
	flt = strtod(abuf, NULL);
	ReleaseBuffer(ed, buf);

	AG_ObjectUnlock(ed);
	return (flt);
}

static void
OnBindingChange(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const AG_Variable *binding = AG_PTR(1);

	if (strcmp(binding->name, "string") == 0) {
		AG_Unset(ed, "text");
	} else if (strcmp(binding->name, "text") == 0) {
		AG_Unset(ed, "string");
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Editable *ed = obj;

	WIDGET(ed)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_UNFOCUSED_MOTION |
			     AG_WIDGET_USE_TEXT | AG_WIDGET_USE_MOUSEOVER;

	ed->flags = AG_EDITABLE_BLINK_ON | AG_EDITABLE_MARKPREF;

	ed->lineScrollAmount = 5;

#ifdef AG_UNICODE
	ed->encoding = "UTF-8";
#else
	ed->encoding = "US-ASCII";
#endif
	ed->text[0] = '\0';

	ed->hPre = 1;
	memset(&ed->wPre, 0, sizeof(int) +               /* wPre */
			     sizeof(int) +               /* pos */
			     sizeof(int) +               /* sel */
			     sizeof(int) +               /* selDblClick */
			     sizeof(AG_Char) +           /* compose */
			     sizeof(int) +               /* xCurs */
			     sizeof(int) +               /* yCurs */
			     sizeof(int) +               /* xSelStart */
			     sizeof(int) +               /* ySelStart */
			     sizeof(int) +               /* xSelEnd */
			     sizeof(int) +               /* ySelEnd */
			     sizeof(int) +               /* xCursPref */
			     sizeof(int) +               /* x */
			     sizeof(int) +               /* xMax */
			     sizeof(int) +               /* y */
			     sizeof(int) +               /* yMax */
			     sizeof(int) +               /* yVis */
			     sizeof(int) +               /* posKbdSel */
			     sizeof(int) +               /* returnHeld */
			     sizeof(AG_EditableBuffer) + /* sBuf */
			     sizeof(AG_Rect) +           /* r */
			     sizeof(AG_CursorArea) +     /* ca */
			     sizeof(enum ag_language) +  /* lang */
			     sizeof(int) +               /* xScrollPx */
			     sizeof(AG_PopupMenu *) +    /* pm */
			     sizeof(int *) +             /* xScrollTo */
			     sizeof(int *) +
			     sizeof(AG_Autocomplete *)); /* complete */
	
	ed->selDblClick = -1;
	ed->fontMaxHeight = agTextFontHeight;
	ed->lineSkip = agTextFontLineSkip;
	ed->suPlaceholder = -1;

	AG_InitTimer(&ed->toRepeat, "repeat", 0);
	AG_InitTimer(&ed->toRepeatDirs[0], "repeatU", 0);
	AG_InitTimer(&ed->toRepeatDirs[1], "repeatD", 0);
	AG_InitTimer(&ed->toRepeatDirs[2], "repeatL", 0);
	AG_InitTimer(&ed->toRepeatDirs[3], "repeatR", 0);
	AG_InitTimer(&ed->toCursorBlink, "cursorBlink", 0);
	AG_InitTimer(&ed->toDblClick, "dblClick", 0);

	AG_AddEvent(ed, "font-changed", OnFontChange, NULL);
	AG_AddEvent(ed, "widget-hidden", OnHide, NULL);

	AG_SetEvent(ed, "key-down", KeyDown, NULL);
	AG_SetEvent(ed, "key-up", KeyUp, NULL);
	AG_SetEvent(ed, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(ed, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(ed, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(ed, "widget-gainfocus", OnFocusGain, NULL);
	AG_SetEvent(ed, "widget-lostfocus", OnFocusLoss, NULL);

	/* Default to a small, built-in text buffer. */
	AG_BindString(ed, "string", ed->text, sizeof(ed->text));

	/* Make binding "string" and "text" mutually exclusive. */
	AG_SetEvent(ed, "bound", OnBindingChange, NULL);
	OBJECT(ed)->flags |= AG_OBJECT_BOUND_EVENTS;
}

static void
Destroy(void *_Nonnull obj)
{
	AG_Editable *ed = obj;

	if (ed->complete != NULL) {
		free(ed->complete);
	}
	if (ed->pm != NULL) {
		AG_PopupDestroy(ed->pm);
	}
	if (ed->flags & AG_EDITABLE_EXCL)
		Free(ed->sBuf.s);
}

/* Initialize/release the global clipboards. */
static void
InitClipboard(AG_EditableClipboard *_Nonnull cb)
{
	AG_MutexInit(&cb->lock);
#ifdef AG_UNICODE
	Strlcpy(cb->encoding, "UTF-8", sizeof(cb->encoding));
#else
	Strlcpy(cb->encoding, "US-ASCII", sizeof(cb->encoding));
#endif
	cb->s = NULL;
	cb->len = 0;
}
static void
FreeClipboard(AG_EditableClipboard *_Nonnull cb)
{
	Free(cb->s);
	AG_MutexDestroy(&cb->lock);
}
void
AG_EditableInitClipboards(void)
{
	InitClipboard(&agEditableClipbrd);
	InitClipboard(&agEditableKillring);
}
void
AG_EditableDestroyClipboards(void)
{
	FreeClipboard(&agEditableClipbrd);
	FreeClipboard(&agEditableKillring);
}

/* Return current cursor position in text. */
int
AG_EditableGetCursorPos(const AG_Editable *ed)
{
	return (ed->pos);
}

/* Return 1 if the Editable is effectively read-only. */
int 
AG_EditableReadOnly(AG_Editable *_Nonnull ed)
{
	int flag;

	AG_OBJECT_ISA(ed, "AG_Widget:AG_Editable:*");
	AG_ObjectLock(ed);

	flag = AGEDITABLE_IS_READONLY(ed);

	AG_ObjectUnlock(ed);
	return (flag);
}

/*
 * Ensure that the cursor position and selection range lie within the size
 * of the given buffer; clamp or normalize them if necessary.
 * 
 * The Editable and buffer must both be locked.
 */
void
AG_EditableValidateSelection(AG_Editable *ed, const AG_EditableBuffer *buf)
{
	if (ed->pos > (int)buf->len) {   /* Allow one character past the end */
		ed->pos = (int)buf->len;
		ed->selStart = 0;
		ed->selEnd = 0;
	}
	if (ed->selEnd < ed->selStart) {
		int temp = ed->selEnd;

		ed->selEnd = ed->selStart;
		ed->selStart = temp;
	}
	if (ed->selEnd > buf->len) {
		ed->selEnd = buf->len;
		if (ed->selEnd < ed->selStart) {
			ed->selStart = 0;
			ed->selEnd = 0;
		}
	}
}	

static void *_Nullable
Edit(void *_Nonnull obj)
{
	static const AG_FlagDescr flagDescr[] = {
	    { AG_EDITABLE_CURSOR_MOVING, N_("Cursor is moving"),          1 },
	    { AG_EDITABLE_WORDSELECT,    N_("Word selection mode"),       1 },
	    { AG_EDITABLE_WORDWRAP,      N_("Word wrapping"),             1 },
	    { AG_EDITABLE_PASSWORD,      N_("Password entry"),            1 },
	    { AG_EDITABLE_INT_ONLY,      N_("Integers only"),             1 },
	    { AG_EDITABLE_FLT_ONLY,      N_("Real numbers only"),         1 },
	    { AG_EDITABLE_UPPERCASE,     N_("Display in uppercase"),      1 },
	    { AG_EDITABLE_LOWERCASE,     N_("Display in lowercase"),      1 },
	    { AG_EDITABLE_NO_KILL_YANK,  N_("No emacs-style functions"),  1 },
	    { AG_EDITABLE_NO_ALT_LATIN1, N_("No latin1 key composition"), 1 },
	    { AG_EDITABLE_EXCL,          N_("Exclusive buffer access"),   0 },
	    { AG_EDITABLE_MULTILINE,     N_("Multiline mode"),            0 },
	    { AG_EDITABLE_MULTILINGUAL,  N_("Multilingual field"),        0 },
	    { 0,                         NULL,                            0 }
	};
	AG_Editable *tgt = obj;
	AG_Box *box;

	box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	AG_SetStyle(box, "font-size", "80%");

	AG_LabelNewPolled(box, AG_LABEL_HFILL,
	    _("Cursor: " AGSI_BOLD "%i" AGSI_RST " (@ %d,%d)"),
	    &tgt->pos, &tgt->xCurs, &tgt->yCurs);

	AG_LabelNewPolled(box, AG_LABEL_HFILL,
	    _("Selection: " AGSI_BOLD "%i-%i" AGSI_RST " (@ %d,%d - %d,%d)"),
	    &tgt->selStart, &tgt->selEnd,
	    &tgt->xSelStart, &tgt->ySelEnd,
	    &tgt->ySelStart, &tgt->ySelEnd);

	AG_LabelNewPolled(box, AG_LABEL_HFILL,
	    _("Offset: X=" AGSI_FONT5 "%i" AGSI_RST
	         "/%i, Y=" AGSI_FONT5 "%i" AGSI_RST "/%i"),
	    &tgt->x, &tgt->xMax,
	    &tgt->y, &tgt->yMax);

	AG_SeparatorNewHoriz(box);

	AG_CheckboxSetFromFlags(box, 0, &tgt->flags, flagDescr);

	return (box);
}

AG_WidgetClass agEditableClass = {
	{
		"Agar(Widget:Editable)",
		sizeof(AG_Editable),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		Edit
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */

/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * used by other widgets such as AG_Textbox(3).
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
#include <agar/gui/icons.h>
#include <agar/gui/gui_math.h>

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <agar/config/have_freetype.h>

AG_EditableClipboard agEditableClipbrd;		/* For Copy/Cut/Paste */
AG_EditableClipboard agEditableKillring;	/* For Emacs-style Kill/Yank */

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
	if (AG_Defined(ed, "text")) {			/* AG_Text element */
		AG_Text *txt;

		buf->var = AG_GetVariable(ed, "text", &txt);
		buf->reallocable = 1;

		AG_MutexLock(&txt->lock);

		if ((ed->flags & AG_EDITABLE_EXCL) == 0 ||
		    buf->s == NULL) {
			AG_TextEnt *te = &txt->ent[ed->lang];

			if (te->buf != NULL) {
				buf->s = AG_ImportUnicode("UTF-8", te->buf,
				    &buf->len, &buf->maxLen);
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
	{					/* Fixed-size buffer */
		char *s;

		buf->var = AG_GetVariable(ed, "string", &s);
		buf->reallocable = 0;

		if ((ed->flags & AG_EDITABLE_EXCL) == 0 ||
		    buf->s == NULL) {
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
	if ((ed->flags & AG_EDITABLE_EXCL) == 0) { Free(buf); }
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
	if (AG_Defined(ed, "text")) {			/* AG_Text binding */
		AG_Text *txt = buf->var->data.p;
		AG_TextEnt *te = &txt->ent[ed->lang];
		AG_Size lenEnc;

		if (AG_LengthUTF8FromUCS4(buf->s, &lenEnc) == -1) {
			goto fail;
		}
		lenEnc++;

		if (lenEnc > te->maxLen &&
		    AG_TextRealloc(te, lenEnc) == -1) {
			goto fail;
		}
		if (AG_ExportUnicode(ed->encoding, te->buf, buf->s,
		    te->maxLen+1) == -1) {
			goto fail;
		}
	} else {					/* C string binding */
		if (AG_ExportUnicode(ed->encoding, buf->var->data.s, buf->s,
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
	if (AG_Defined(ed, "text")) {
		AG_Text *txt = buf->var->data.p;
		AG_MutexUnlock(&txt->lock);
	}
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

	newLen = (buf->len + nIns + 1)*sizeof(AG_Char);

#ifdef AG_UNICODE
	if (Strcasecmp(ed->encoding, "UTF-8") == 0) {
		AG_Size sLen, insLen;

		if (AG_LengthUTF8FromUCS4(buf->s, &sLen) == -1 ||
		    AG_LengthUTF8FromUCS4(ins, &insLen) == -1) {
			return (-1);
		}
		convLen = sLen + insLen + 1;
	} else if (Strcasecmp(ed->encoding, "US-ASCII") == 0) {
		convLen = AG_LengthUCS4(buf->s) + nIns + 1;
	} else {
		/* TODO Proper estimates for other charsets */
		convLen = newLen;
	}
#else /* !AG_UNICODE */
	if (Strcasecmp(ed->encoding, "US-ASCII") == 0) {
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
	ReleaseBuffer(ed, buf);
	AG_ObjectUnlock(ed);
}

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

	/* Set exclusive mode if requested; also sets default redraw rate. */
	AG_EditableSetExcl(ed, (flags & AG_EDITABLE_EXCL));

	AG_ObjectAttach(parent, ed);
	return (ed);
}

/* Bind to a C string containing ASCII text. */
void
AG_EditableBindASCII(AG_Editable *ed, char *buf, AG_Size bufSize)
{
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
	AG_ObjectLock(ed);
	AG_Unset(ed, "text");
	AG_BindString(ed, "string", buf, bufSize);
	ed->encoding = encoding;
	AG_ObjectUnlock(ed);
}

/* Bind to an AG_Text element. */
void
AG_EditableBindText(AG_Editable *ed, AG_Text *txt)
{
	AG_ObjectLock(ed);
	AG_Unset(ed, "string");
	AG_BindPointer(ed, "text", (void *)txt);
	ed->encoding = "UTF-8";
	AG_ObjectUnlock(ed);
}

/* Set the active language (when bound to an AG_Text(3) element) */
void
AG_EditableSetLang(AG_Editable *ed, enum ag_language lang)
{
	AG_ObjectLock(ed);
	ed->lang = lang;
	ed->pos = 0;
	ed->sel = 0;
	AG_ObjectUnlock(ed);
	AG_Redraw(ed);
}
#endif /* AG_UNICODE */

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
	ed->x = 0;
	ed->y = 0;
	ed->pos = 0;
	ed->sel = 0;
	AG_SETFLAGS(ed->flags, AG_EDITABLE_WORDWRAP, enable);
	AG_ObjectUnlock(ed);
}

/*
 * Specify whether the buffer is accessed exclusively by the widget.
 * We can disable periodic redraws in exclusive mode.
 */
void
AG_EditableSetExcl(AG_Editable *ed, int enable)
{
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

#ifdef HAVE_FLOAT
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
#endif /* HAVE_FLOAT */

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

#ifdef HAVE_FLOAT
/* Evaluate if a character is acceptable in float-only mode. */
static __inline__ int
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
#endif /* HAVE_FLOAT */

/*
 * Process a keystroke. May be invoked from the repeat timeout routine or
 * the keydown handler. If we return 1, the current delay/repeat cycle will
 * be maintained, otherwise it will be cancelled.
 */
static int
ProcessKey(AG_Editable *_Nonnull ed, AG_KeySym ks, AG_KeyMod kmod, AG_Char ch)
{
	AG_EditableBuffer *buf;
	int i, rv = 0;

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
		}
#ifdef HAVE_FLOAT
		else if ((ed->flags & AG_EDITABLE_FLT_ONLY) &&
		           !CharIsFltOnly((AG_Char)ks)) {
			return (0);
		}
#endif
	}

	if ((buf = GetBuffer(ed)) == NULL)
		return (0);

	if (ed->pos < 0) { ed->pos = 0; }
	if (ed->pos > buf->len) { ed->pos = buf->len; }

	for (i = 0; ; i++) {
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
				if (AG_EditableReadOnly(ed)) {
					rv = 0;
					goto out;
				}
				break;
			case 'e':
				if (ed->flags & AG_EDITABLE_NOEMACS) {
					rv = 0;
					goto out;
				}
				break;
			}
		}
		AG_PostEvent(ed, "editable-prechg", NULL);
		rv = kc->func(ed, buf, ks, kmod, ch);
		break;
	}
out:
	if (rv == 1) {
		CommitBuffer(ed, buf);
	}
	ReleaseBuffer(ed, buf);
	return (1);
}

#ifdef AG_TIMERS
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

	AG_LockTimers(ed);
	AG_DelTimer(ed, &ed->toRepeat);
	AG_AddTimer(ed, &ed->toCursorBlink, agTextBlinkRate, BlinkTimeout, NULL);
	ed->flags |= AG_EDITABLE_BLINK_ON;
	AG_UnlockTimers(ed);

	AG_Redraw(ed);
}

static void
OnFocusLoss(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();

	AG_LockTimers(ed);
	AG_DelTimer(ed, &ed->toRepeat);
	AG_DelTimer(ed, &ed->toCursorBlink);
	AG_DelTimer(ed, &ed->toDblClick);
	ed->flags &= ~(AG_EDITABLE_BLINK_ON | AG_EDITABLE_CURSOR_MOVING);
	AG_UnlockTimers(ed);

	AG_Redraw(ed);
}
#endif /* AG_TIMERS */

static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();

	if (ed->pm != NULL) {
		AG_PopupHide(ed->pm);
	}
#ifdef AG_TIMERS
	OnFocusLoss(event);
#endif
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
}

/*
 * Evaluate whether the given character should be considered
 * a space for word wrapping and word selection.
 */
static __inline__ int
IsSpaceNat(AG_Char c)
{
	switch (c) {
	case ' ':		/* SPACE */
	case '\t':		/* TAB */
		return (1);
#ifdef AG_UNICODE
	case 0x00a0:		/* NO-BREAK SPACE */
	case 0x1680:		/* OGHAM SPACE MARK */
	case 0x180e:		/* MONGOLIAN VOWEL SEPARATOR */
	case 0x202f:		/* NARROW NO-BREAK SPACE */
	case 0x205f:		/* MEDIUM MATHEMATICAL SPACE */
	case 0x3000:		/* IDEOGRAPHIC SPACE */
	case 0xfeff:		/* ZERO WIDTH NO-BREAK SPACE */
		return (1);
#endif
	}
#ifdef AG_UNICODE
	if (c >= 0x2000 && c <= 0x200b)	/* EN/EM SPACES */
		return (1);
#endif
	return (0);
}

/* Evaluate word wrapping at given character. */
static __inline__ int
WrapAtChar(AG_Editable *_Nonnull ed, int x, AG_Char *_Nonnull s)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_Glyph *gl;
	AG_Char *t;
	int x2;

	if (!(ed->flags & AG_EDITABLE_WORDWRAP) ||
	    x == 0 || !IsSpaceNat(*s)) {
		return (0);
	}
	for (t = &s[1], x2 = x;
	     *t != '\0';
	     t++) {
		gl = AG_TextRenderGlyph(drv, *t);
		x2 += gl->advance;
		if (IsSpaceNat(*t) || *t == '\n') {
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
 * Map mouse coordinates to a position within the buffer.
 */
#define ON_LINE(my,y)       ((my) >= (y) && (my) <= (y)+ed->lineSkip)
#define ON_CHAR(mx,x,glyph) ((mx) >= (x) && (mx) <= (x)+(glyph)->advance)
int
AG_EditableMapPosition(AG_Editable *ed, AG_EditableBuffer *buf, int mx, int my,
    int *pos)
{
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_Font *font = WFONT(ed);
	AG_Char ch;
	int i, x, y, line = 0;
	int nLines = 1;
	int yMouse;
	
	AG_ObjectLock(ed);

	yMouse = my + ed->y*ed->lineSkip;
	if (yMouse < 0) {
		*pos = 0;
		goto out;
	}

	x = 0;
	y = 0;
 	for (i = 0; i < buf->len; i++) {
		AG_Char ch = buf->s[i];

		if (WrapAtChar(ed, x, &buf->s[i])) {
			x = 0;
			nLines++;
		}
		if (ch == '\n') {
			x = 0;
			nLines++;
		} else if (ch == '\t') {
			x += agTextTabWidth;
		} else {
			switch (font->spec.type) {
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
				}
				break;
#endif /* HAVE_FREETYPE */
#ifdef AG_SERIALIZATION
			case AG_FONT_BITMAP:
				{
					AG_Glyph *gl;
			
					gl = AG_TextRenderGlyph(drv, ch);
					x += gl->su->w;
				}
				break;
#endif /* AG_SERIALIZATION */
			case AG_FONT_DUMMY:
				break;
			}
		}
	}

	x = 0;
	for (i = 0; i < buf->len; i++) {
		ch = buf->s[i];
		if (mx <= 0 && ON_LINE(yMouse,y)) {
			*pos = i;
			goto out;
		}
		if (WrapAtChar(ed, x, &buf->s[i])) {
			if (ON_LINE(yMouse,y) && mx > x) {
				*pos = i;
				goto out;
			}
			y += ed->lineSkip;
			x = 0;
			line++;
		}
		if (ch == '\n') {
			if (ON_LINE(yMouse,y) && mx > x) {
				*pos = i;
				goto out;
			}
			y += ed->lineSkip;
			x = 0;
			line++;
			continue;
		} else if (ch == '\t') {
			if (ON_LINE(yMouse,y) &&
			    mx >= x && mx <= x+agTextTabWidth) {
				*pos = (mx < x + (agTextTabWidth >> 1)) ? i : i+1;
				goto out;
			}
			x += agTextTabWidth;
			continue;
		}
		
		switch (font->spec.type) {
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
					*pos = (mx < x+(glyph->advance >> 1)) ?
					       i : i+1;
					goto out;
				}
				x += glyph->advance;
			}
			break;
#endif /* HAVE_FREETYPE */
#ifdef AG_SERIALIZATION
		case AG_FONT_BITMAP:
			{
				AG_Glyph *gl;
			
				gl = AG_TextRenderGlyph(drv, ch);
				if (ON_LINE(yMouse,y) &&
				    mx >= x &&
				    mx <= x+gl->su->w) {
					*pos = i;
					goto out;
				}
				x += gl->su->w;
			}
			break;
#endif /* AG_SERIALIZATION */
		case AG_FONT_DUMMY:
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
	AG_ObjectLock(ed);
	if (AG_EditableMapPosition(ed, buf, mx, my, &ed->pos) == 0) {
		ed->sel = 0;
		AG_Redraw(ed);
	}
	AG_ObjectUnlock(ed);
}

/* Set the cursor position (-1 = end of the string) with bounds checking. */
int
AG_EditableSetCursorPos(AG_Editable *ed, AG_EditableBuffer *buf, int pos)
{
	int rv;

	AG_ObjectLock(ed);
	ed->pos = pos;
	ed->sel = 0;
	if (ed->pos < 0) {
		if (pos == -1 || ed->pos > buf->len)
			ed->pos = buf->len;
	}
	rv = ed->pos;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_ObjectUnlock(ed);
	
	AG_Redraw(ed);
	return (rv);
}

/* Do whatever we can do to make the cursor visible. */
static void
MoveCursorToView(AG_Editable *_Nonnull ed, AG_EditableBuffer *_Nonnull buf)
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
	AG_Editable *ed = obj;
	AG_Driver *drv = WIDGET(ed)->drv;
	AG_DriverClass *drvOps = WIDGET(ed)->drvOps;
	AG_EditableBuffer *buf;
	const AG_Color *bgColor = &WCOLOR(ed, AG_BG_COLOR);
	AG_Rect2 rClip;
	const int pos = ed->pos;
	const int sel = ed->sel;
	const int flags = ed->flags;
	const int lineSkip = ed->lineSkip;
	int i, dx,dy, x,y, yCurs, inSel=0;

	if (bgColor->a != AG_TRANSPARENT) {
		AG_Rect r;
		r.x = 0;
		r.y = 0;
		r.w = WIDTH(ed);
		r.h = HEIGHT(ed);
		AG_DrawRectFilled(ed, &r, bgColor);
	}
	if ((buf = GetBuffer(ed)) == NULL) {
		return;
	}
	AG_EditableValidateSelection(ed, buf);
	
	/* Tweak the clipping rectangle to allow a bit of overflow. */
	rClip = WIDGET(ed)->rView;
	rClip.x1 -= (ed->fontMaxHeight << 1);
	rClip.y1 -= lineSkip;
	rClip.x2 += (ed->fontMaxHeight << 1);
	rClip.y2 += lineSkip;

	AG_PushBlendingMode(ed, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC,
	                    AG_TEXTURE_ENV_REPLACE);
	AG_PushClipRect(ed, &ed->r);

	x = 0;
	y = -ed->y * lineSkip;
	ed->xMax = 10;
	ed->yMax = 1;
	for (i = 0; i <= buf->len; i++) {
		AG_Glyph *gl;
		AG_Char c = buf->s[i];

		if (i == pos) {				/* At cursor */
			ed->xCurs = x;
			if (flags & AG_EDITABLE_MARKPREF) {
				ed->flags &= ~(AG_EDITABLE_MARKPREF);
				ed->xCursPref = x;
			}
			ed->yCurs = y/lineSkip + ed->y;
			yCurs = y;
		}
		if ((sel > 0 && i >= pos && i < pos+sel) ||
		    (sel < 0 && i <  pos && i > pos+sel-1)) {
			if (!inSel) {
				inSel = 1;
				ed->xSelStart = x;
				ed->ySelStart = y/lineSkip + ed->y;
			}
		} else {
			if (inSel) {
				inSel = 0;
				ed->xSelEnd = x;
				ed->ySelEnd = y/lineSkip + ed->y;
			}
		}
		if (i == buf->len)
			break;

		if (WrapAtChar(ed, x, &buf->s[i])) {
			y += lineSkip;
			ed->xMax = MAX(ed->xMax, x);
			ed->yMax++;
			x = 0;
		}
		if (c == '\n') {
			y += lineSkip;
			ed->xMax = MAX(ed->xMax, x+10);
			ed->yMax++;
			x = 0;
			continue;
		} else if (c == '\t') {
			/* TODO subroutine this */
			if (inSel) {
				AG_Rect r;

				r.x = x - ed->x;
				r.y = y;
				r.w = agTextTabWidth + 1;
				r.h = lineSkip + 1;
				AG_DrawRectFilled(ed, &r, &WCOLOR_SEL(ed,0));
			}
			x += agTextTabWidth;
			continue;
		}

		if      (flags & AG_EDITABLE_PASSWORD)  { c = '*'; }
		else if (flags & AG_EDITABLE_UPPERCASE) { c = toupper(c); }
		else if (flags & AG_EDITABLE_LOWERCASE) { c = tolower(c); }

		gl = AG_TextRenderGlyph(drv, c);
		dx = WIDGET(ed)->rView.x1 + x - ed->x;
		dy = WIDGET(ed)->rView.y1 + y;

		if (!AG_RectInside2(&rClip, dx, dy)) {
			x += gl->advance;
			continue;
		}

		/* Indicate active selection */
		if (inSel) {
			AG_Rect r;

			r.x = x - ed->x;
			r.y = y;
			r.w = gl->su->w + 1;
			r.h = gl->su->h;
			AG_DrawRectFilled(ed, &r, &WCOLOR_SEL(ed,0));
		}

		/* Render the glyph */
		drvOps->drawGlyph(drv, gl, dx,dy);

		x += gl->advance;
	}
	if (ed->yMax == 1)
		ed->xMax = x;
	
	/* Render the cursor */
	if (AG_WidgetIsFocused(ed) && (flags & AG_EDITABLE_BLINK_ON) &&
	    (ed->y >= 0 && ed->y <= ed->yMax-1)) {
		AG_DrawLineV(ed,
		    ed->xCurs - ed->x, (yCurs + 1),
		    (yCurs + lineSkip - 1),
		    &WCOLOR(ed,AG_TEXT_COLOR));
	}
	
	/* Process any scrolling requests. */
	if (flags & AG_EDITABLE_KEEPVISCURSOR) {
		MoveCursorToView(ed, buf);
	}
	/* TODO subroutine these */
	if (ed->xScrollTo != NULL) {
		if ((*ed->xScrollTo - ed->x) < 0) {
			ed->x += (*ed->xScrollTo - ed->x);
			if (ed->x < 0) { ed->x = 0; }
		}
		if ((*ed->xScrollTo - ed->x) > WIDTH(ed) - 10) {
			ed->x = *ed->xScrollTo - WIDTH(ed) + 10;
		}
		ed->xScrollTo = NULL;
		WIDGET(ed)->window->dirty = 1;		/* Redraw once */
	}
	if (ed->yScrollTo != NULL) {
		if ((*ed->yScrollTo - ed->y) < 0) {
			ed->y += (*ed->yScrollTo - ed->y);
			if (ed->y < 0) { ed->y = 0; }
		}
		if ((*ed->yScrollTo - ed->y) > ed->yVis - 1) {
			ed->y = *ed->yScrollTo - ed->yVis + 1;
		}
		ed->yScrollTo = NULL;
		WIDGET(ed)->window->dirty = 1;		/* Redraw once */
	}
	if (ed->xScrollPx != 0) {
		if (ed->xCurs < ed->x - ed->xScrollPx ||
		    ed->xCurs > ed->x + WIDTH(ed) - ed->xScrollPx) {
			ed->x += ed->xScrollPx;
		}
		ed->xScrollPx = 0;
		WIDGET(ed)->window->dirty = 1;		/* Redraw once */
	}

	AG_PopClipRect(ed);
	AG_PopBlendingMode(ed);

	ReleaseBuffer(ed, buf);
}

void
AG_EditableSizeHint(AG_Editable *ed, const char *text)
{
	int hPre;

	AG_ObjectLock(ed);
	AG_TextSize(text, &ed->wPre, &hPre);
	ed->hPre = MIN(1, hPre/ed->lineSkip);
	AG_ObjectUnlock(ed);
}

void
AG_EditableSizeHintPixels(AG_Editable *ed, Uint w, Uint h)
{
	AG_ObjectLock(ed);
	ed->wPre = w;
	ed->hPre = MIN(1, h/ed->lineSkip);
	AG_ObjectUnlock(ed);
}

void
AG_EditableSizeHintLines(AG_Editable *ed, Uint nLines)
{
	ed->hPre = nLines;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Editable *ed = obj;

	r->w = ed->wPre;
	r->h = ed->hPre*ed->lineSkip + 2;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Editable *ed = obj;
	AG_Rect r;
	int w = a->w;
	int h = a->h;

	if (w < 2 || h < 2) {
		return (-1);
	}
	ed->yVis = h / ed->lineSkip;
	ed->r.x = -1;
	ed->r.y = -1;
	ed->r.w = w-1;
	ed->r.h = h-1;

	r.x = 0;
	r.y = 0;
	r.w = w;
	r.h = h;
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
	}

	ed->flags |= AG_EDITABLE_BLINK_ON;

	if (ProcessKey(ed, keysym, keymod, ch) == 1) {
#ifdef AG_TIMERS
		AG_AddTimer(ed, &ed->toRepeat, agKbdDelay,
		    KeyRepeatTimeout, "%i,%i,%lu", keysym, keymod, ch);
#endif
	} else {
#ifdef AG_TIMERS
		AG_DelTimer(ed, &ed->toRepeat);
#endif
	}
	AG_Redraw(ed);
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int keysym = AG_INT(1);

#ifdef AG_TIMERS
	AG_DelTimer(ed, &ed->toRepeat);
#endif
	if ((keysym == AG_KEY_RETURN || keysym == AG_KEY_KP_ENTER) &&
	   (ed->flags & AG_EDITABLE_MULTILINE) == 0) {
		if (ed->flags & AG_EDITABLE_ABANDON_FOCUS) {
			AG_WidgetUnfocus(ed);
		}
		AG_PostEvent(ed, "editable-return", NULL);
	}
	AG_Redraw(ed);
}

static void
MouseDoubleClick(AG_Editable *_Nonnull ed)
{
	AG_EditableBuffer *buf;
	AG_Char *c;

#ifdef AG_TIMERS
	AG_DelTimer(ed, &ed->toDblClick);
#endif
	ed->selDblClick = -1;
	ed->flags |= AG_EDITABLE_WORDSELECT;

	if ((buf = GetBuffer(ed)) == NULL) {
		return;
	}
	if (ed->pos >= 0 && ed->pos < buf->len) {
		ed->sel = 0;

		c = &buf->s[ed->pos];
		if (*c == '\n') {
			goto out;
		}
		for (;
		     ed->pos > 0;
		     ed->pos--, c--) {
			if (IsSpaceNat(*c) && ed->pos < buf->len) {
				c++;
				ed->pos++;
				break;
			}
		}
		while ((ed->pos + ed->sel) < buf->len &&
		    !IsSpaceNat(*c)) {
			c++;
			ed->sel++;
		}
	}
out:
	ReleaseBuffer(ed, buf);
}

/* Ensure selection offset is positive. */
static __inline__ void
NormSelection(AG_Editable *_Nonnull ed)
{
	if (ed->sel < 0) {
		ed->pos += ed->sel;
		ed->sel = -(ed->sel);
	}
}

/* Copy specified range to specified clipboard. */
void
AG_EditableCopyChunk(AG_Editable *ed, AG_EditableClipboard *cb, AG_Char *s,
    AG_Size len)
{
	AG_Char *sNew;

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
}

/* Perform Cut action on buffer. */
int
AG_EditableCut(AG_Editable *ed, AG_EditableBuffer *buf, AG_EditableClipboard *cb)
{
	if (AG_EditableReadOnly(ed) || ed->sel == 0) {
		return (0);
	}
	AG_EditableValidateSelection(ed, buf);
	NormSelection(ed);
	AG_EditableCopyChunk(ed, cb, &buf->s[ed->pos], ed->sel);
	AG_EditableDelete(ed, buf);
	return (1);
}

/* Perform Copy action on standard clipboard. */
int
AG_EditableCopy(AG_Editable *ed, AG_EditableBuffer *buf, AG_EditableClipboard *cb)
{
	if (ed->sel == 0) {
		return (0);
	}
	AG_EditableValidateSelection(ed, buf);
	NormSelection(ed);
	AG_EditableCopyChunk(ed, cb, &buf->s[ed->pos], ed->sel);
	return (1);
}

/* Perform Paste action on buffer. */
int
AG_EditablePaste(AG_Editable *ed, AG_EditableBuffer *buf,
    AG_EditableClipboard *cb)
{
	AG_Char *c;

	if (AG_EditableReadOnly(ed))
		return (0);

	if (ed->sel != 0)
		AG_EditableDelete(ed, buf);

	AG_MutexLock(&cb->lock);

	if (cb->s == NULL)
		goto out;

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
				goto fail;
			}
		}
	} else if (ed->flags & AG_EDITABLE_FLT_ONLY) {
		for (c = &cb->s[0]; *c != '\0'; c++) {
#ifdef HAVE_FLOAT
			if (!CharIsFltOnly(*c)) {
				AG_SetError(_("Non-float input near `%c'"), (char)*c);
				goto fail;
			}
#endif
		}
	}

	if (AG_EditableGrowBuffer(ed, buf, cb->s, cb->len) == -1) {
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
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
out:
	AG_MutexUnlock(&cb->lock);
	return (1);
fail:
	Verbose("Paste Failed: %s\n", AG_GetError());
	AG_MutexUnlock(&cb->lock);
	return (0);
}

/* Delete the current selection. */
int
AG_EditableDelete(AG_Editable *ed, AG_EditableBuffer *buf)
{
	if (AG_EditableReadOnly(ed) || ed->sel == 0)
		return (0);

	AG_EditableValidateSelection(ed, buf);
	NormSelection(ed);
	if (ed->pos + ed->sel == buf->len) {
		buf->s[ed->pos] = '\0';
	} else {
		memmove(&buf->s[ed->pos], &buf->s[ed->pos + ed->sel],
		    (buf->len - ed->sel + 1 - ed->pos)*sizeof(AG_Char));
	}
	buf->len -= ed->sel;
	ed->sel = 0;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	return (1);
}

/* Perform "Select All" on buffer. */
void
AG_EditableSelectAll(AG_Editable *ed, AG_EditableBuffer *buf)
{
	ed->pos = 0;
	ed->sel = buf->len;
	AG_Redraw(ed);
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
		if (AG_EditableCut(ed, buf, &agEditableClipbrd)) {
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

	*enable = (!AG_EditableReadOnly(ed) && ed->sel != 0);
}

static void
MenuCopy(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		AG_EditableCopy(ed, buf, &agEditableClipbrd);
		ReleaseBuffer(ed, buf);
	}
}
static void
MenuCopyActive(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	int *enable = AG_PTR(2);

	*enable = (ed->sel != 0);
}

static void
MenuPaste(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_PTR(1);
	AG_EditableBuffer *buf;
	
	if ((buf = GetBuffer(ed)) != NULL) {
		if (AG_EditablePaste(ed, buf, &agEditableClipbrd)) {
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

	*enable = (!AG_EditableReadOnly(ed) && agEditableClipbrd.len > 0);
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

	*enable = (!AG_EditableReadOnly(ed) && ed->sel != 0);
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
	AG_Text *txt;
#endif
	if ((pm = AG_PopupNew(ed)) == NULL) {
		return (NULL);
	}
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
	    (vText = AG_GetVariable(ed, "text", &txt)) != NULL) {
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

#ifdef AG_TIMERS
/* Timer for detecting double clicks. */
static Uint32
DoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();

	ed->selDblClick = -1;
	return (0);
}
#endif /* AG_TIMERS */

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	const int btn = AG_INT(1);
	int mx = AG_INT(2);
	const int my = AG_INT(3);
	AG_EditableBuffer *buf;

	if (!AG_WidgetIsFocused(ed))
		AG_WidgetFocus(ed);

	switch (btn) {
	case AG_MOUSE_LEFT:
		if (ed->pm != NULL) {
			AG_PopupHide(ed->pm);
		}
		ed->flags |= AG_EDITABLE_CURSOR_MOVING | AG_EDITABLE_BLINK_ON;
		mx += ed->x;
		if ((buf = GetBuffer(ed)) == NULL) {
			return;
		}
		AG_EditableMoveCursor(ed, buf, mx, my);
		ReleaseBuffer(ed, buf);
		ed->flags |= AG_EDITABLE_MARKPREF;

		if (ed->selDblClick != -1 &&
		    abs(ed->selDblClick - ed->pos) <= 1) {
			MouseDoubleClick(ed);
		} else {
			ed->selDblClick = ed->pos;
#ifdef AG_TIMERS
			AG_AddTimer(ed, &ed->toDblClick, agMouseDblclickDelay,
			    DoubleClickTimeout, NULL);
#endif
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
			ed->y -= AG_GetInt(ed, "line-scroll-amount");
			if (ed->y < 0) { ed->y = 0; }
		}
		break;
	case AG_MOUSE_WHEELDOWN:
		if (ed->flags & AG_EDITABLE_MULTILINE) {
			ed->y += AG_GetInt(ed, "line-scroll-amount");
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
	const int btn = AG_INT(1);

	switch (btn) {
	case AG_MOUSE_LEFT:
		ed->flags &= ~(AG_EDITABLE_CURSOR_MOVING);
		ed->flags &= ~(AG_EDITABLE_WORDSELECT);
		AG_Redraw(ed);
		break;
	}
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Editable *ed = AG_EDITABLE_SELF();
	AG_EditableBuffer *buf;
	const int mx = AG_INT(1);
	const int my = AG_INT(2);
	int newPos;

	if (!AG_WidgetIsFocused(ed) ||
	    (ed->flags & AG_EDITABLE_CURSOR_MOVING) == 0)
		return;

	if ((buf = GetBuffer(ed)) == NULL)
		return;
	if (AG_EditableMapPosition(ed, buf, ed->x + mx, my, &newPos) == -1)
		goto out;

	if (ed->flags & AG_EDITABLE_WORDSELECT) {
		AG_Char *c;

		c = &buf->s[newPos];
		if (*c == '\n') {
			goto out;
		}
		if (newPos > ed->pos) {
			if (ed->sel < 0) {
				ed->pos += ed->sel;
				ed->sel = -(ed->sel);
			}
			ed->sel = newPos - ed->pos;
			while (c < &buf->s[buf->len] &&
			    !IsSpaceNat(*c) && *c != '\n') {
				c++;
				ed->sel++;
			}
			ed->xScrollTo = &ed->xSelEnd;
			ed->yScrollTo = &ed->ySelEnd;
		} else if (newPos < ed->pos) {
			if (ed->sel > 0) {
				ed->pos += ed->sel;
				ed->sel = -(ed->sel);
			}
			ed->sel = newPos - ed->pos;
			while (c > &buf->s[0] &&
			    !IsSpaceNat(*c) && *c != '\n') {
				c--;
				ed->sel--;
			}
			if (IsSpaceNat(buf->s[ed->pos + ed->sel])) {
				ed->sel++;
			}
			ed->xScrollTo = &ed->xSelStart;
			ed->yScrollTo = &ed->ySelStart;
		}
		AG_Redraw(ed);
	} else {
		ed->sel = newPos - ed->pos;
		if (ed->sel > 0) {
			ed->xScrollTo = &ed->xSelEnd;
			ed->yScrollTo = &ed->ySelEnd;
		} else if (ed->sel < 0) {
			ed->xScrollTo = &ed->xSelStart;
			ed->yScrollTo = &ed->ySelStart;
		}
		AG_Redraw(ed);
	}
out:
	ReleaseBuffer(ed, buf);
}

/*
 * Overwrite the contents of the text buffer with the given string.
 */
void
AG_EditableSetString(AG_Editable *ed, const char *text)
{
	AG_EditableBuffer *buf;

	AG_ObjectLock(ed);
	if ((buf = GetBuffer(ed)) == NULL) {
		goto out;
	}
	if (text != NULL) {
		Free(buf->s);
#ifdef AG_UNICODE
		buf->s = AG_ImportUnicode("UTF-8", text, &buf->len, &buf->maxLen);
#else
		buf->s = (Uint8 *)TryStrdup(text);
		buf->maxLen = buf->len = strlen(text);
#endif
		if (buf->s != NULL) {
			ed->pos = buf->len;
		} else {
			buf->len = 0;
		}
	} else {
		if (buf->maxLen >= sizeof(AG_Char)) {
			buf->s[0] = '\0';
			ed->pos = 0;
			buf->len = 0;
		}
	}
	ed->sel = 0;
	CommitBuffer(ed, buf);
	ReleaseBuffer(ed, buf);
out:
	AG_ObjectUnlock(ed);
	AG_Redraw(ed);
}

void
AG_EditableClearString(AG_Editable *ed)
{
	AG_EditableSetString(ed, NULL);
}

/*
 * Overwrite the contents of the text buffer with the given formatted
 * C string (in UTF-8 encoding).
 */
void
AG_EditablePrintf(void *obj, const char *fmt, ...)
{
	AG_Editable *ed = obj;
	AG_EditableBuffer *buf;
	va_list ap;
	char *s;

#ifdef AG_DEBUG
	if (!AG_OfClass(obj, "AG_Widget:AG_Editable:*") &&
	    !AG_OfClass(obj, "AG_Widget:AG_Textbox:*"))
		AG_FatalError(NULL);
#endif

	AG_ObjectLock(ed);
	if ((buf = GetBuffer(ed)) == NULL) {
		goto out;
	}
	if (fmt != NULL && fmt[0] != '\0') {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);

		Free(buf->s);
#ifdef AG_UNICODE
		buf->s = AG_ImportUnicode("UTF-8", s, &buf->len, &buf->maxLen);
#else
		buf->s = (Uint8 *)TryStrdup(s);
		buf->maxLen = buf->len = strlen(s);
#endif
		free(s);

		if (buf->s != NULL) {
			ed->pos = buf->len;
		} else {
			buf->len = 0;
		}
	} else {
		Free(buf->s);
		if ((buf->s = TryMalloc(sizeof(AG_Char))) != NULL) {
			buf->s[0] = '\0';
		}
		ed->pos = 0;
		buf->len = 0;
	}
	ed->sel = 0;
	CommitBuffer(ed, buf);
	ReleaseBuffer(ed, buf);
out:
	AG_ObjectUnlock(ed);
	AG_Redraw(ed);
}

/* Return a duplicate of the current string. */
char *
AG_EditableDupString(AG_Editable *ed)
{
	AG_Variable *var;
	char *sDup, *s;

	AG_ObjectLock(ed);
	if (AG_Defined(ed, "text")) {
		AG_Text *txt;
		var = AG_GetVariable(ed, "text", &txt);
		s = txt->ent[ed->lang].buf;
		sDup = TryStrdup((s != NULL) ? s : "");
	} else {
		var = AG_GetVariable(ed, "string", &s);
		sDup = TryStrdup(s);
	}
	AG_UnlockVariable(var);
	AG_ObjectUnlock(ed);
	return (sDup);
}

/* Copy text to a fixed-size buffer and always NUL-terminate. */
AG_Size
AG_EditableCopyString(AG_Editable *ed, char *dst, AG_Size dst_size)
{
	AG_Variable *var;
	AG_Size rv;
	char *s;

	AG_ObjectLock(ed);
	if (AG_Defined(ed, "text")) {
		AG_Text *txt;
		var = AG_GetVariable(ed, "text", &txt);
		s = txt->ent[ed->lang].buf;
		rv = Strlcpy(dst, (s != NULL) ? s : "", dst_size);
	} else {
		var = AG_GetVariable(ed, "string", &s);
		rv = Strlcpy(dst, s, dst_size);
	}
	AG_UnlockVariable(var);
	AG_ObjectUnlock(ed);
	return (rv);
}

/* Perform trivial conversion from string to int. */
int
AG_EditableInt(AG_Editable *ed)
{
	char abuf[32];
	AG_EditableBuffer *buf;
	int i;

	AG_ObjectLock(ed);
	if ((buf = GetBuffer(ed)) == NULL) {
		AG_FatalError(NULL);
	}
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

#ifdef HAVE_FLOAT
/* Perform trivial conversion from string to float . */
float
AG_EditableFlt(AG_Editable *ed)
{
	char abuf[32];
	AG_EditableBuffer *buf;
	float flt;

	AG_ObjectLock(ed);
	if ((buf = GetBuffer(ed)) == NULL) {
		AG_FatalError(NULL);
	}
# ifdef AG_UNICODE
	AG_ExportUnicode("UTF-8", abuf, buf->s, sizeof(abuf));
# else
	Strlcpy(abuf, (const char *)buf->s, sizeof(abuf));
# endif
	flt = (float)strtod(abuf, NULL);
	ReleaseBuffer(ed, buf);
	AG_ObjectUnlock(ed);
	return (flt);
}

/* Perform trivial conversion from string to double. */
double
AG_EditableDbl(AG_Editable *ed)
{
	char abuf[32];
	AG_EditableBuffer *buf;
	double flt;

	AG_ObjectLock(ed);
	if ((buf = GetBuffer(ed)) == NULL) {
		AG_FatalError(NULL);
	}
# ifdef AG_UNICODE
	AG_ExportUnicode("UTF-8", abuf, buf->s, sizeof(abuf));
#else
	Strlcpy(abuf, (const char *)buf->s, sizeof(abuf));
#endif
	flt = strtod(abuf, NULL);
	ReleaseBuffer(ed, buf);
	AG_ObjectUnlock(ed);
	return (flt);
}
#endif /* HAVE_FLOAT */

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
			     AG_WIDGET_TABLE_EMBEDDABLE | AG_WIDGET_USE_TEXT |
			     AG_WIDGET_USE_MOUSEOVER;
#ifdef AG_UNICODE
	ed->encoding = "UTF-8";
	ed->text = AG_TextNew(0);
#else
	ed->encoding = "US-ASCII";
	ed->text[0] = '\0';
#endif
	ed->flags = AG_EDITABLE_BLINK_ON | AG_EDITABLE_MARKPREF;
	ed->pos = 0;
	ed->sel = 0;
	ed->selDblClick = -1;
	ed->compose = 0;
	ed->xCurs = 0;
	ed->yCurs = 0;
	ed->xCursPref = 0;
	ed->xSelStart = 0;
	ed->ySelStart = 0;
	ed->xScrollTo = NULL;
	ed->yScrollTo = NULL;
	ed->xScrollPx = 0;

	ed->x = 0;
	ed->xMax = 10;
	ed->y = 0;
	ed->yMax = 1;
	ed->yVis = 1;
	ed->wheelTicks = 0;
	ed->r.x = 0;
	ed->r.y = 0;
	ed->r.w = 0;
	ed->r.h = 0;
	ed->ca = NULL;
	ed->fontMaxHeight = agTextFontHeight;
	ed->lineSkip = agTextFontLineSkip;
	ed->pm = NULL;
	ed->lang = AG_LANG_NONE;
	ed->wPre = 0;
	ed->hPre = 1;

	ed->sBuf.var = NULL;
	ed->sBuf.s = NULL;
	ed->sBuf.len = 0;
	ed->sBuf.maxLen = 0;
	ed->sBuf.reallocable = 0;

	AG_SetInt(ed, "line-scroll-amount", 5);

	AG_SetEvent(ed, "bound", OnBindingChange, NULL);
	OBJECT(ed)->flags |= AG_OBJECT_BOUND_EVENTS;

	AG_AddEvent(ed, "font-changed", OnFontChange, NULL);
	AG_AddEvent(ed, "widget-hidden", OnHide, NULL);
	AG_SetEvent(ed, "key-down", KeyDown, NULL);
	AG_SetEvent(ed, "key-up", KeyUp, NULL);
	AG_SetEvent(ed, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(ed, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(ed, "mouse-motion", MouseMotion, NULL);
#ifdef AG_TIMERS
	AG_SetEvent(ed, "widget-gainfocus", OnFocusGain, NULL);
	AG_SetEvent(ed, "widget-lostfocus", OnFocusLoss, NULL);
	AG_InitTimer(&ed->toRepeat, "repeat", 0);
	AG_InitTimer(&ed->toCursorBlink, "cursorBlink", 0);
	AG_InitTimer(&ed->toDblClick, "dblClick", 0);
#endif
#ifdef AG_UNICODE
	AG_BindPointer(ed, "text", (void *)ed->text);
#else
	AG_BindString(ed, "string", ed->text, sizeof(ed->text));
#endif
	AG_RedrawOnTick(ed, 1000);
}

static void
Destroy(void *_Nonnull obj)
{
	AG_Editable *ed = obj;

	if (ed->pm != NULL) {
		AG_PopupDestroy(ed->pm);
	}
	if (ed->flags & AG_EDITABLE_EXCL)
		Free(ed->sBuf.s);

#ifdef AG_UNICODE
	AG_TextFree(ed->text);
#endif
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

	AG_ObjectLock(ed);
	flag = (ed->flags & AG_EDITABLE_READONLY) || AG_WidgetDisabled(ed);
	AG_ObjectUnlock(ed);
	return (flag);
}

/*
 * Ensure that the selection range is valid. The Editable and buffer
 * must both be locked.
 */
void
AG_EditableValidateSelection(AG_Editable *ed, AG_EditableBuffer *buf)
{
	if ((Uint)ed->pos > buf->len) {
		ed->pos = (int)buf->len;
		ed->sel = 0;
	}
	if (ed->sel != 0) {
		int ep = ed->pos + ed->sel;
		if (ep < 0) {
			ed->pos = 0;
			ed->sel = 0;
		} else if ((Uint)ep > buf->len) {
			ed->pos = (int)buf->len;
			ed->sel = 0;
		}
	}
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
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */

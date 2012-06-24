/*
 * Copyright (c) 2002-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Basic keyboard input processing in UTF-8 mode.
 */

#include <core/core.h>
#include <core/config.h>

#include "widget.h"
#include "editable.h"
#include "keymap.h"
#include "text.h"
#include "gui_math.h"

#include <ctype.h>
#include <string.h>

/* Increase the working buffer size to accomodate new characters. */
static int
GrowBuffer(AG_Editable *ed, AG_EditableBuffer *buf, Uint32 *ins, size_t nIns)
{
	size_t ucsSize;		/* UCS-4 buffer size in bytes */
	size_t convLen;		/* Converted string length in bytes */
	Uint32 *sNew;

	ucsSize = (buf->len + nIns + 1)*sizeof(Uint32);

	if (Strcasecmp(ed->encoding, "UTF-8") == 0) {
		convLen = AG_LengthUTF8FromUCS4(buf->s) +
		          AG_LengthUTF8FromUCS4(ins) + 1;
	} else {
		/* TODO Proper estimates for other charsets */
		convLen = ucsSize;
	}
	
	if (!buf->reallocable) {
		if (convLen > buf->var->info.size) {
			AG_SetError("%u > %u bytes", (Uint)convLen, (Uint)buf->var->info.size);
			return (-1);
		}
	}
	if (ucsSize > buf->maxLen) {
		if ((sNew = TryRealloc(buf->s, ucsSize)) == NULL) {
			return (-1);
		}
		buf->s = sNew;
		buf->maxLen = ucsSize;
	}
	return (0);
}

/* Ensure sel is a positive number. */
static __inline__ void
NormSelection(AG_Editable *ed)
{
	if (ed->sel < 0) {
		ed->pos += ed->sel;
		ed->sel = -(ed->sel);
	}
}

/* Delete the current selection. */
static void
KillSelection(AG_Editable *ed, AG_EditableBuffer *buf)
{
	NormSelection(ed);
	if (ed->pos + ed->sel == buf->len) {
		buf->s[ed->pos] = '\0';
	} else {
		memmove(&buf->s[ed->pos], &buf->s[ed->pos + ed->sel],
		    (buf->len - ed->sel + 1 - ed->pos)*sizeof(Uint32));
	}
	buf->len -= ed->sel;
	ed->sel = 0;
}

/* Copy given characters to clipboard. */
static void
CopyToClipboard(AG_Editable *ed, AG_EditableClipboard *cb, Uint32 *s,
    size_t len)
{
	Uint32 *sNew;

	AG_MutexLock(&cb->lock);
	sNew = TryRealloc(cb->s, (len+1)*sizeof(Uint32));
	if (sNew != NULL) {
		cb->s = sNew;
		memcpy(cb->s, s, len*sizeof(Uint32));
		cb->s[len] = '\0';
		cb->len = len;
	}
	Strlcpy(cb->encoding, ed->encoding, sizeof(cb->encoding));
	AG_MutexUnlock(&cb->lock);
}

/* Paste contents of clipboard to current cursor position. */
static int
PasteFromClipboard(AG_Editable *ed, AG_EditableBuffer *buf,
    AG_EditableClipboard *cb)
{
	AG_MutexLock(&cb->lock);

	if (cb->s == NULL)
		goto out;

	if (strcmp(ed->encoding, cb->encoding) != 0) {
		/* TODO: charset conversion */
		goto fail;
	}
	if (GrowBuffer(ed, buf, cb->s, cb->len) == -1) {
		Verbose("Paste Failed: %s\n", AG_GetError());
		goto out;
	}
	if (ed->pos < buf->len) {
		memmove(&buf->s[ed->pos + cb->len], &buf->s[ed->pos],
		    (buf->len - ed->pos)*sizeof(Uint32));
	}
	memcpy(&buf->s[ed->pos], cb->s, cb->len*sizeof(Uint32));
	buf->len += cb->len;
	buf->s[buf->len] = '\0';
	ed->pos += cb->len;
out:
	AG_MutexUnlock(&cb->lock);
	return (0);
fail:
	AG_MutexUnlock(&cb->lock);
	return (-1);
}

/* Insert a new character at current cursor position. */
static int
Insert(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 ch)
{
	Uint32 ins[3];
	int i, nIns;
	Uint32 uch = ch;

	if (AG_WidgetDisabled(ed) || keysym == 0)
		return (0);
#if 0
	if (!unicodeKbd) {
		uch = AG_ApplyModifiersASCII((Uint32)keysym, keymod);
	}
#endif
	if (!(ed->flags & AG_EDITABLE_NOLATIN1)) {
		for (i = 0; ; i++) {
			const struct ag_key_mapping *km = &agKeymapLATIN1[i];

			if (keysym == km->key) {
				if (((keymod & AG_KEYMOD_ALT) &&
				     (keymod & AG_KEYMOD_SHIFT) &&
				     (km->modmask == (AG_KEYMOD_ALT|AG_KEYMOD_SHIFT)))) {
					uch = km->unicode;
					break;
				} else if (keymod & AG_KEYMOD_ALT &&
				    km->modmask == AG_KEYMOD_ALT) {
					uch = km->unicode;
					break;
				}
			} else if (km->key == AG_KEY_LAST) {
				break;
			}
		}
	}
	
	if (uch == 0) { return (0); }
	if (uch == '\r') { uch = '\n'; }

	if (Strcasecmp(ed->encoding, "US-ASCII") == 0 &&
	    !isascii((int)uch))
		return (0);

	if (agTextComposition) {
		if ((nIns = AG_KeyInputCompose(ed, uch, ins)) == 0)
			return (0);
	} else {
		ins[0] = uch;
		nIns = 1;
	}
	ins[nIns] = '\0';

	if (ed->sel != 0) {
		KillSelection(ed, buf);
	}
	if (GrowBuffer(ed, buf, ins, (size_t)nIns) == -1) {
		Verbose("Insert Failed: %s\n", AG_GetError());
		return (0);
	}

	if (ed->pos == buf->len) {				/* Append */
		for (i = 0; i < nIns; i++)
			buf->s[buf->len + i] = ins[i];
	} else {						/* Insert */
		memmove(&buf->s[ed->pos + nIns], &buf->s[ed->pos],
		       (buf->len - ed->pos)*sizeof(Uint32));
		for (i = 0; i < nIns; i++)
			buf->s[ed->pos + i] = ins[i];
	}
	buf->len += nIns;
	buf->s[buf->len] = '\0';
	ed->pos += nIns;
	return (1);
}

/* Delete the character at cursor, or the active selection. */
static int
Delete(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 unicode)
{
	Uint32 *c;

	if (AG_WidgetDisabled(ed))
		return (0);
	if (buf->len == 0)
		return (0);

	if (ed->sel != 0) {
		KillSelection(ed, buf);
		return (1);
	}
	if (keysym == AG_KEY_BACKSPACE && ed->pos == 0) {
		return (0);
	}
	if (ed->pos == buf->len) { 
		ed->pos--;
		buf->s[--buf->len] = '\0';
		return (1);
	}
	if (keysym == AG_KEY_BACKSPACE) {
		ed->pos--;
	}
	for (c = &buf->s[ed->pos];
	     c < &buf->s[buf->len + 1];
	     c++) {
		*c = c[1];
		if (*c == '\0')
			break;
	}
	buf->len--;
	return (1);
}

/* Copy the selection to clipboard. */
static int
Copy(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	if (ed->sel == 0) {
		return (0);
	}
	NormSelection(ed);
	CopyToClipboard(ed, &agEditableClipbrd, &buf->s[ed->pos], ed->sel);
	return (1);
}

/* Copy selection to clipboard and subsequently delete it. */
static int
Cut(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	if (ed->sel == 0) {
		return (0);
	}
	NormSelection(ed);
	CopyToClipboard(ed, &agEditableClipbrd, &buf->s[ed->pos], ed->sel);
	KillSelection(ed, buf);
	return (1);
}

/* Paste clipboard contents to current cursor position. */
static int
Paste(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	if (ed->sel != 0) {
		KillSelection(ed, buf);
	}
	if (PasteFromClipboard(ed, buf, &agEditableClipbrd) == -1) {
		return (0);
	}
	KillSelection(ed, buf);
	return (1);
}

/*
 * Kill the current selection; if there is no selection, cut the
 * characters up to the end of the line (Emacs-style).
 */
static int
Kill(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	Uint32 *c;
	
	if (AG_WidgetDisabled(ed)) {
		return (0);
	}
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_K) && (keymod & AG_KEYMOD_CTRL))
		return (0);

	if (ed->sel != 0) {
		NormSelection(ed);
	} else {
		for (c = &buf->s[ed->pos]; c < &buf->s[buf->len]; c++) {
			if (*c == '\n') {
				break;
			}
			ed->sel++;
		}
		if (ed->sel == 0)
			return (0);
	}
	
	CopyToClipboard(ed, &agEditableKillring, &buf->s[ed->pos], ed->sel);
	KillSelection(ed, buf);
	return (1);
}

/* Paste the contents of the Emacs-style kill ring at cursor position. */
static int
Yank(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	if (AG_WidgetDisabled(ed)) {
		return (0);
	}
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_Y) && (keymod & AG_KEYMOD_CTRL))
		return (0);

	if (ed->sel != 0) {
		KillSelection(ed, buf);
	}
	if (PasteFromClipboard(ed, buf, &agEditableKillring) == -1) {
		return (0);
	}
	return (1);
}

/* Seek one word backwards. */
static int
WordBack(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	Uint32 *c;

	if (ed->flags & AG_EDITABLE_NOWORDSEEK)
		return (0);
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_B) && (keymod & AG_KEYMOD_ALT))
		return (0);

	/* XXX: handle other types of spaces */
	if (ed->pos > 1 && buf->s[ed->pos - 1] == ' ') {
		ed->pos -= 2;
	}
	for (c = &buf->s[ed->pos];
	     c > &buf->s[0] && *c != ' ';
	     c--, ed->pos--)
		;;
	if (*c == ' ') {
		ed->pos++;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

/* Seek one word forward. */
static int
WordForw(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	Uint32 *c;
	
	if (ed->flags & AG_EDITABLE_NOWORDSEEK)
		return (0);
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_F) && (keymod & AG_KEYMOD_ALT))
		return (0);

	if (ed->pos == buf->len) {
		return (0);
	}
	if (buf->len > 1 && buf->s[ed->pos] == ' ') {
		ed->pos++;
	}
	for (c = &buf->s[ed->pos];
	     *c != '\0' && *c != ' ';
	     c++, ed->pos++)
		;;
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

/* Move cursor to beginning of line. */
static int
CursorHome(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	Uint32 *c;

	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_A) && (keymod & AG_KEYMOD_CTRL)) {
		return (0);
	}
	if (ed->flags & AG_EDITABLE_MULTILINE) {
		if (ed->pos == 0) {
			return (0);
		}
		for (c = &buf->s[ed->pos - 1];
		     c >= &buf->s[0] && ed->pos >= 0;
		     c--, ed->pos--) {
			if (*c == '\n')
				break;
		}
	} else {
		ed->pos = 0;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

/* Move cursor to end of line. */
static int
CursorEnd(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	Uint32 *c;

	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_E) && (keymod & AG_KEYMOD_CTRL)) {
		return (0);
	}
	if (ed->flags & AG_EDITABLE_MULTILINE) {
		if (ed->pos == buf->len || buf->s[ed->pos] == '\n') {
			return (0);
		}
		for (c = &buf->s[ed->pos + 1];
		     c <= &buf->s[buf->len] && ed->pos <= buf->len;
		     c++, ed->pos++) {
			if (*c == '\n') {
				ed->pos++;
				break;
			}
		}
		if (ed->pos > buf->len)
			ed->pos = buf->len;
	} else {
		ed->pos = buf->len;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

/* Move cursor left. */
static int
CursorLeft(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	if (--ed->pos < 1) {
		ed->pos = 0;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

/* Move cursor right. */
static int
CursorRight(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	if (ed->pos < buf->len) {
		ed->pos++;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

/* Move cursor up in a multi-line string. */
static int
CursorUp(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCursPref,
	    (ed->yCurs - ed->y - 1)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

/* Move cursor down in a multi-line string. */
static int
CursorDown(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCursPref,
	    (ed->yCurs - ed->y + 1)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

/* Move cursor one page up in a multi-line string. */
static int
PageUp(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y - ed->yVis)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

/* Move cursor one page down in a multi-line string. */
static int
PageDown(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y + ed->yVis)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

const struct ag_keycode agKeymap[] = {
	{ AG_KEY_HOME,		0,		CursorHome },
	{ AG_KEY_A,		AG_KEYMOD_CTRL,	CursorHome },
	{ AG_KEY_END,		0,		CursorEnd },
	{ AG_KEY_E,		AG_KEYMOD_CTRL,	CursorEnd },
	{ AG_KEY_LEFT,		0,		CursorLeft },
	{ AG_KEY_RIGHT,		0,		CursorRight },
	{ AG_KEY_UP,		0,		CursorUp },
	{ AG_KEY_DOWN,		0,		CursorDown },
	{ AG_KEY_PAGEUP,	0,		PageUp },
	{ AG_KEY_PAGEDOWN,	0,		PageDown },
	{ AG_KEY_BACKSPACE,	0,		Delete },
	{ AG_KEY_DELETE,	0,		Delete },
	{ AG_KEY_C,		AG_KEYMOD_CTRL,	Copy },
	{ AG_KEY_X,		AG_KEYMOD_CTRL,	Cut },
	{ AG_KEY_V,		AG_KEYMOD_CTRL,	Paste },
	{ AG_KEY_K,		AG_KEYMOD_CTRL,	Kill },
	{ AG_KEY_Y,		AG_KEYMOD_CTRL,	Yank },
	{ AG_KEY_B,		AG_KEYMOD_ALT,	WordBack },
	{ AG_KEY_F,		AG_KEYMOD_ALT,	WordForw },
	{ AG_KEY_LAST,		0,		Insert },
};

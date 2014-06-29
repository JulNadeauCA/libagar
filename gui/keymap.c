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
 * Keyboard input processing for AG_Editable(3).
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/editable.h>
#include <agar/gui/keymap.h>
#include <agar/gui/text.h>
#include <agar/gui/gui_math.h>

#include <ctype.h>
#include <string.h>

/* Insert a new character at current cursor position. */
static int
Insert(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 ch)
{
	Uint32 ins[3];
	int i, nIns;
	Uint32 uch = ch;

	if (keysym == 0)
		return (0);
#ifdef __APPLE__
	if ((keymod & AG_KEYMOD_LMETA) ||
	    (keymod & AG_KEYMOD_RMETA))
		return (0);
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
		AG_EditableDelete(ed, buf);
	}
	if (AG_EditableGrowBuffer(ed, buf, ins, (size_t)nIns) == -1) {
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

	if (!(ed->flags & AG_EDITABLE_MULTILINE)) {	/* Optimize case */
		int wIns;
		AG_TextSizeUCS4(ins, &wIns, NULL);
		ed->xScrollPx += wIns;
	} else {
		ed->xScrollTo = &ed->xCurs;
		ed->yScrollTo = &ed->yCurs;
	}
	ed->flags |= AG_EDITABLE_BLINK_ON;
	return (1);
}

/* Delete the character at cursor, or the active selection. */
static int
Delete(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 unicode)
{
	Uint32 *c;
	int wDel;

	if (buf->len == 0)
		return (0);

	if (ed->sel != 0) {
		AG_EditableDelete(ed, buf);
		return (1);
	}
	if (keysym == AG_KEY_BACKSPACE && ed->pos == 0) {
		return (0);
	}
	if (ed->pos == buf->len) { 
		ed->pos--;
		buf->s[--buf->len] = '\0';

		if (ed->flags & AG_EDITABLE_MULTILINE) {
			ed->xScrollTo = &ed->xCurs;
			ed->yScrollTo = &ed->yCurs;
		} else {
			AG_TextSizeUCS4(&buf->s[buf->len-1], &wDel, NULL);
			if (ed->x > 0) { ed->x -= wDel; }
		}
		return (1);
	}
	if (keysym == AG_KEY_BACKSPACE)
		ed->pos--;

	if (ed->flags & AG_EDITABLE_MULTILINE) {
		ed->xScrollTo = &ed->xCurs;
		ed->yScrollTo = &ed->yCurs;
	} else {
		Uint32 cDel[2];
	
		cDel[0] = buf->s[ed->pos];
		cDel[1] = '\0';
		AG_TextSizeUCS4(cDel, &wDel, NULL);
		if (ed->x > 0) { ed->x -= wDel; }
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
Copy(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	AG_EditableCopy(ed, buf, &agEditableClipbrd);
	return (0);
}

/* Copy selection to clipboard and subsequently delete it. */
static int
Cut(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	return AG_EditableCut(ed, buf, &agEditableClipbrd);
}

/* Paste clipboard contents to current cursor position. */
static int
Paste(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	return AG_EditablePaste(ed, buf, &agEditableClipbrd);
}

/*
 * Kill the current selection; if there is no selection, cut the
 * characters up to the end of the line (Emacs-style).
 */
static int
Kill(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	Uint32 *c;
	
	if (ed->sel != 0) {
		AG_EditableValidateSelection(ed, buf);
		if (ed->sel < 0) {
			ed->pos += ed->sel;
			ed->sel = -(ed->sel);
		}
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
	
	AG_EditableCopyChunk(ed, &agEditableKillring, &buf->s[ed->pos], ed->sel);
	AG_EditableDelete(ed, buf);
	return (1);
}

/* Paste the contents of the Emacs-style kill ring at cursor position. */
static int
Yank(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	return AG_EditablePaste(ed, buf, &agEditableKillring);
}

/* Seek one word backwards. */
static int
WordBack(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int newPos = ed->pos;
	Uint32 *c;

	/* XXX: handle other types of spaces */
	if (ed->pos > 1 && buf->s[newPos-1] == ' ') {
		newPos -= 2;
	}
	for (c = &buf->s[newPos];
	     c > &buf->s[0] && *c != ' ';
	     c--, newPos--)
		;;
	if (*c == ' ')
		newPos++;

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel += (ed->pos - newPos);
	} else {
		ed->sel = 0;
	}
	ed->pos = newPos;

	ed->flags |= AG_EDITABLE_MARKPREF;
	ed->flags |= AG_EDITABLE_BLINK_ON;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Seek one word forward. */
static int
WordForw(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int newPos = ed->pos;
	Uint32 *c;
	
	if (newPos == buf->len) {
		return (0);
	}
	if (buf->len > 1 && buf->s[newPos] == ' ') {
		newPos++;
	}
	for (c = &buf->s[newPos];
	     *c != '\0' && *c != ' ';
	     c++, newPos++)
		;;

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel += (ed->pos - newPos);
	} else {
		ed->sel = 0;
	}
	ed->pos = newPos;

	ed->flags |= AG_EDITABLE_MARKPREF;
	ed->flags |= AG_EDITABLE_BLINK_ON;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Select all. */
static int
SelectAll(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	AG_EditableSelectAll(ed, buf);
	return (0);
}

/* Move cursor to beginning of line. */
static int
CursorHome(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int newPos = ed->pos;
	Uint32 *c;

	if (ed->flags & AG_EDITABLE_MULTILINE) {
		if (newPos == 0) {
			return (0);
		}
		for (c = &buf->s[newPos - 1];
		     c >= &buf->s[0] && newPos >= 0;
		     c--, newPos--) {
			if (*c == '\n')
				break;
		}
	} else {
		newPos = 0;
	}

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel += (ed->pos - newPos);
	} else {
		ed->sel = 0;
	}
	ed->pos = newPos;

	ed->x = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor to end of line. */
static int
CursorEnd(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int newPos = ed->pos;
	Uint32 *c;

	if (ed->flags & AG_EDITABLE_MULTILINE) {
		if (newPos == buf->len || buf->s[newPos] == '\n') {
			return (0);
		}
		for (c = &buf->s[newPos + 1];
		     c <= &buf->s[buf->len] && newPos <= buf->len;
		     c++, newPos++) {
			if (*c == '\n') {
				newPos++;
				break;
			}
		}
		if (newPos > buf->len)
			newPos = buf->len;
	} else {
		newPos = buf->len;
	}

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel += (ed->pos - newPos);
	} else {
		ed->sel = 0;
	}
	ed->pos = newPos;

	ed->flags |= AG_EDITABLE_MARKPREF;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor left. */
static int
CursorLeft(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	if ((ed->pos - 1) >= 0) {
		ed->pos--;
		if (keymod & AG_KEYMOD_SHIFT) {
			ed->sel++;
		} else {
			ed->sel = 0;
		}
	} else {
		ed->pos = 0;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	ed->flags |= AG_EDITABLE_BLINK_ON;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor right. */
static int
CursorRight(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	if (ed->pos < buf->len) {
		ed->pos++;
		if (keymod & AG_KEYMOD_SHIFT) {
			ed->sel--;
		} else {
			ed->sel = 0;
		}
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	ed->flags |= AG_EDITABLE_BLINK_ON;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor up in a multi-line string. */
static int
CursorUp(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int prevPos = ed->pos;
	int prevSel = ed->sel;

	if (!(ed->flags & AG_EDITABLE_MULTILINE))
		return (0);

	AG_EditableMoveCursor(ed, buf, ed->xCursPref,
	    (ed->yCurs - ed->y - 1)*agTextFontLineSkip + 1);

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel = prevSel - (ed->pos - prevPos);
	} else {
		ed->sel = 0;
	}

	ed->flags |= AG_EDITABLE_BLINK_ON;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor down in a multi-line string. */
static int
CursorDown(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int prevPos = ed->pos;
	int prevSel = ed->sel;

	if (!(ed->flags & AG_EDITABLE_MULTILINE))
		return (0);

	AG_EditableMoveCursor(ed, buf, ed->xCursPref,
	    (ed->yCurs - ed->y + 1)*agTextFontLineSkip + 1);

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel = prevSel - (ed->pos - prevPos);
	} else {
		ed->sel = 0;
	}

	ed->flags |= AG_EDITABLE_BLINK_ON;
	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor one page up in a multi-line string. */
static int
PageUp(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int prevPos = ed->pos;
	int prevSel = ed->sel;

	if (!(ed->flags & AG_EDITABLE_MULTILINE))
		return (0);

	AG_EditableMoveCursor(ed, buf, ed->xCurs,
	    (ed->yCurs - ed->y - ed->yVis)*agTextFontLineSkip + 1);
	
	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel = prevSel - (ed->pos - prevPos);
	} else {
		ed->sel = 0;
	}

	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/* Move cursor one page down in a multi-line string. */
static int
PageDown(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, Uint keymod, Uint32 uch)
{
	int prevPos = ed->pos;
	int prevSel = ed->sel;

	if (!(ed->flags & AG_EDITABLE_MULTILINE))
		return (0);

	AG_EditableMoveCursor(ed, buf, ed->xCurs,
	    (ed->yCurs - ed->y + ed->yVis)*agTextFontLineSkip + 1);

	if (keymod & AG_KEYMOD_SHIFT) {
		ed->sel = prevSel - (ed->pos - prevPos);
	} else {
		ed->sel = 0;
	}

	ed->xScrollTo = &ed->xCurs;
	ed->yScrollTo = &ed->yCurs;
	AG_Redraw(ed);
	return (0);
}

/*
 * Map AG_KeySym(3) and AG_KeyMod(3) values to AG_Editable functions.
 * Variants with modifier keys must appear first in the list. The
 * modifiers string is passed to AG_CompareKeyMods(3).
 *
 * Available flags:
 *	"w" = Require a writeable buffer
 *	"e" = AG_EDITABLE_NOEMACS must be unset
 */
const struct ag_keycode agKeymap[] = {
#ifdef __APPLE__
	{ AG_KEY_LEFT,		"CM",		CursorHome,	"" },
	{ AG_KEY_RIGHT,		"CM",		CursorEnd,	"" },
	{ AG_KEY_LEFT,		"A",		WordBack,	"" },
	{ AG_KEY_RIGHT,		"A",		WordForw,	"" },
	{ AG_KEY_A,		"M",		SelectAll,	"" },
	{ AG_KEY_C,		"M",		Copy,		"" },
	{ AG_KEY_X,		"M",		Cut,		"w" },
	{ AG_KEY_V,		"M",		Paste,		"w" },
	{ AG_KEY_K,		"M",		Kill,		"we" },
	{ AG_KEY_Y,		"M",		Yank,		"we" },
#else /* __APPLE__ */
	{ AG_KEY_LEFT,		"C",		WordBack,	"" },
	{ AG_KEY_RIGHT,		"C",		WordForw,	"" },
	{ AG_KEY_A,		"C",		SelectAll,	"" },
	{ AG_KEY_C,		"C",		Copy,		"" },
	{ AG_KEY_X,		"C",		Cut,		"w" },
	{ AG_KEY_V,		"C",		Paste,		"w" },
	{ AG_KEY_K,		"C",		Kill,		"we" },
	{ AG_KEY_Y,		"C",		Yank,		"we" },
#endif /* !__APPLE__ */
	{ AG_KEY_HOME,		"",		CursorHome,	"" },
	{ AG_KEY_END,		"",		CursorEnd,	"" },
	{ AG_KEY_LEFT,		"",		CursorLeft,	"" },
	{ AG_KEY_RIGHT,		"",		CursorRight,	"" },
	{ AG_KEY_UP,		"",		CursorUp,	"" },
	{ AG_KEY_DOWN,		"",		CursorDown,	"" },
	{ AG_KEY_PAGEUP,	"",		PageUp,		"" },
	{ AG_KEY_PAGEDOWN,	"",		PageDown,	"" },
	{ AG_KEY_BACKSPACE,	"",		Delete,		"w" },
	{ AG_KEY_DELETE,	"",		Delete,		"w" },
	{ AG_KEY_LAST,		"",		Insert,		"w" },
};

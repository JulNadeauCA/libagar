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

#include <ctype.h>
#include <string.h>

#ifdef AG_THREADS
static AG_Mutex killRingLock = AG_MUTEX_INITIALIZER;
#endif
static Uint32  *killRing = NULL;
static size_t	killRingLen = 0;
static char     killRingEnc[32] = { '\0' };

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

static void
KillSelection(AG_Editable *ed, AG_EditableBuffer *buf)
{
	if (ed->sel < 0) {
		ed->pos += ed->sel;
		ed->sel = -(ed->sel);
	}
	if (ed->pos + ed->sel == buf->len) {
		buf->s[ed->pos] = '\0';
	} else {
		memmove(&buf->s[ed->pos], &buf->s[ed->pos + ed->sel],
		    (buf->len - ed->sel + 1 - ed->pos)*sizeof(Uint32));
	}
	buf->len -= ed->sel;
	ed->sel = 0;
}

static int
InsertUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
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

static int
DeleteUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 unicode)
{
	Uint32 *c;

	if (AG_WidgetDisabled(ed))
		return (0);
	if (buf->len == 0)
		return (0);

	if (ed->sel == 0) {
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
	} else {
		KillSelection(ed, buf);
	}
	return (1);
}

static int
KillUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	Uint32 *killRingNew;
	size_t lenKill = 0;
	Uint32 *c;
	
	if (AG_WidgetDisabled(ed)) {
		return (0);
	}
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_K) && (keymod & AG_KEYMOD_CTRL))
		return (0);

	for (c = &buf->s[ed->pos]; c < &buf->s[buf->len]; c++) {
		if (*c == '\n') {
			break;
		}
		lenKill++;
	}
	if (lenKill == 0)
		return (0);

	AG_MutexLock(&killRingLock);
	killRingNew = TryRealloc(killRing, (lenKill+1)*sizeof(Uint32));
	if (killRingNew != NULL) {
		killRing = killRingNew;
		memcpy(killRing, &buf->s[ed->pos], lenKill*sizeof(Uint32));
		killRing[lenKill] = '\0';
		killRingLen = lenKill;
	}
	Strlcpy(killRingEnc, ed->encoding, sizeof(killRingEnc));
	AG_MutexUnlock(&killRingLock);

	if (ed->pos + lenKill == buf->len) {
		buf->s[ed->pos] = '\0';
	} else {
		memmove(&buf->s[ed->pos], &buf->s[ed->pos + lenKill],
		    (buf->len - lenKill + 1 - ed->pos)*sizeof(Uint32));
	}
	buf->len -= lenKill;
	ed->sel = 0;
	return (1);
}

static int
YankUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym, int keymod,
    Uint32 uch)
{
	int i;

	if (AG_WidgetDisabled(ed)) {
		return (0);
	}
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_Y) && (keymod & AG_KEYMOD_CTRL))
		return (0);

	AG_MutexLock(&killRingLock);
	if (killRing == NULL ||
	    strcmp(ed->encoding, killRingEnc) != 0) {	/* XXX TODO conv */
		goto fail;
	}
	if (GrowBuffer(ed, buf, killRing, killRingLen) == -1) {
		Verbose("Yank Failed: %s\n", AG_GetError());
		goto fail;
	}

	/* XXX TODO: handle other charsets */
	if (Strcasecmp(ed->encoding, "US-ASCII") == 0) {
		for (i = 0; i < killRingLen; i++) {
			if (!isascii((int)killRing[i]))
				goto fail;
		}
	}

	if (ed->sel != 0) {
		KillSelection(ed, buf);
	}
	if (ed->pos < buf->len) {
		memmove(&buf->s[ed->pos + killRingLen], &buf->s[ed->pos],
		    (buf->len - ed->pos)*sizeof(Uint32));
	}
	memcpy(&buf->s[ed->pos], killRing, killRingLen*sizeof(Uint32));
	buf->s[buf->len + killRingLen] = '\0';
	ed->pos += killRingLen;
	AG_MutexUnlock(&killRingLock);
	buf->len = AG_LengthUCS4(buf->s);
	return (1);
fail:
	AG_MutexUnlock(&killRingLock);
	return (0);
}

static int
WordBackUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	Uint32 *c;

	if (ed->flags & AG_EDITABLE_NOWORDSEEK)
		return (0);
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_B) && (keymod & AG_KEYMOD_ALT))
		return (0);

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

static int
WordForwUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
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

static int
CursorHomeUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
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

static int
CursorEndUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
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

static int
CursorLeftUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	if (--ed->pos < 1) {
		ed->pos = 0;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorRightUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	if (ed->pos < buf->len) {
		ed->pos++;
	}
	ed->sel = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorUpUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCursPref,
	    (ed->yCurs - ed->y - 1)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
CursorDownUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCursPref,
	    (ed->yCurs - ed->y + 1)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
PageUpUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y - ed->yVis)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
PageDownUTF8(AG_Editable *ed, AG_EditableBuffer *buf, AG_KeySym keysym,
    int keymod, Uint32 uch)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y + ed->yVis)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

const struct ag_keycode_utf8 agKeymapUTF8[] = {
	{ AG_KEY_HOME,		0,		CursorHomeUTF8 },
	{ AG_KEY_A,		AG_KEYMOD_CTRL,	CursorHomeUTF8 },
	{ AG_KEY_END,		0,		CursorEndUTF8 },
	{ AG_KEY_E,		AG_KEYMOD_CTRL,	CursorEndUTF8 },
	{ AG_KEY_LEFT,		0,		CursorLeftUTF8 },
	{ AG_KEY_RIGHT,		0,		CursorRightUTF8 },
	{ AG_KEY_UP,		0,		CursorUpUTF8 },
	{ AG_KEY_DOWN,		0,		CursorDownUTF8 },
	{ AG_KEY_PAGEUP,	0,		PageUpUTF8 },
	{ AG_KEY_PAGEDOWN,	0,		PageDownUTF8 },
	{ AG_KEY_BACKSPACE,	0,		DeleteUTF8 },
	{ AG_KEY_DELETE,	0,		DeleteUTF8 },
	{ AG_KEY_K,		AG_KEYMOD_CTRL,	KillUTF8 },
	{ AG_KEY_Y,		AG_KEYMOD_CTRL,	YankUTF8 },
	{ AG_KEY_B,		AG_KEYMOD_ALT,	WordBackUTF8 },
	{ AG_KEY_F,		AG_KEYMOD_ALT,	WordForwUTF8 },
	{ AG_KEY_LAST,		0,		InsertUTF8 },
};

/*
 * Copyright (c) 2002-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

static int
InsertUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 ch,
    Uint32 *ucs, int len, int bufSize)
{
	Uint32 ins[3];
	int i, nins;
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

	switch (ed->encoding) {
	case AG_ENCODING_UTF8:
		break;
	case AG_ENCODING_ASCII:
		if (!isascii((int)uch)) {
			return (0);
		}
		break;
	}

	if (agTextComposition) {
		if ((nins = AG_KeyInputCompose(ed, uch, ins)) == 0)
			return (0);
	} else {
		ins[0] = uch;
		nins = 1;
	}
	ins[nins] = '\0';

	/* We need the expanded UTF-8 length to check bounds. */
	/* XXX optimize for STATIC */
	if (AG_LengthUTF8FromUCS4(ucs) + 
	    AG_LengthUTF8FromUCS4(ins) + 1 >= bufSize)
		return (0);

	if (ed->pos == len) {					/* Append */
		for (i = 0; i < nins; i++)
			ucs[len+i] = ins[i];
	} else {						/* Insert */
		memmove(&ucs[ed->pos+nins], &ucs[ed->pos],
		       (len - ed->pos)*sizeof(Uint32));
		for (i = 0; i < nins; i++)
			ucs[ed->pos+i] = ins[i];
	}
	ucs[len+nins] = '\0';
	ed->pos += nins;
	return (1);
}

static int
DeleteUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 unicode,
    Uint32 *ucs, int len, int bufSize)
{
	Uint32 *c;

	if (AG_WidgetDisabled(ed))
		return (0);
	if (len == 0)
		return (0);

	switch (keysym) {
	case AG_KEY_BACKSPACE:
		if (ed->pos == 0) {
			return (0);
		}
		if (ed->pos == len) { 
			ucs[len-1] = '\0';
			ed->pos--;
			return (1);
		}
		ed->pos--;
		break;
	case AG_KEY_DELETE:
		if (ed->pos == len) {
			ucs[len-1] = '\0';
			ed->pos--;
			return (1);
		}
		break;
	default:
		break;
	}
	for (c = &ucs[ed->pos]; c < &ucs[len+1]; c++) {
		*c = c[1];
		if (*c == '\0')
			break;
	}
	return (1);
}

static int
KillUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	size_t lenKill = 0;
	Uint32 *c;
	
	if (AG_WidgetDisabled(ed)) {
		return (0);
	}
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_K) && (keymod & AG_KEYMOD_CTRL))
		return (0);

	for (c = &ucs[ed->pos]; c < &ucs[len]; c++) {
		if (*c == '\n') {
			break;
		}
		lenKill++;
	}
	if (lenKill == 0)
		return (0);

	AG_MutexLock(&killRingLock);
	killRing = Realloc(killRing, (lenKill+1)*sizeof(Uint32));
	memcpy(killRing, &ucs[ed->pos], lenKill*sizeof(Uint32));
	killRing[lenKill] = '\0';
	killRingLen = lenKill;
	AG_MutexUnlock(&killRingLock);

	if (ed->pos+lenKill == len) {
		ucs[ed->pos] = '\0';
	} else {
		memmove(&ucs[ed->pos], &ucs[ed->pos+lenKill],
		    (len-lenKill+1 - ed->pos)*sizeof(Uint32));
	}
	return (1);
}

static int
YankUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	int i;

	if (AG_WidgetDisabled(ed)) {
		return (0);
	}
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_Y) && (keymod & AG_KEYMOD_CTRL))
		return (0);

	AG_MutexLock(&killRingLock);
	if (killRing == NULL) {
		goto nochange;
	}
	if (AG_LengthUTF8FromUCS4(ucs) +
	    AG_LengthUTF8FromUCS4(killRing) + 1 >= bufSize) {
		/* TODO truncate */
		goto nochange;
	}
	switch (ed->encoding) {
	case AG_ENCODING_UTF8:
		break;
	case AG_ENCODING_ASCII:
		for (i = 0; i < killRingLen; i++) {
			if (!isascii((int)killRing[i]))
				goto nochange;
		}
		break;
	}

	if (ed->pos < len) {
		memmove(&ucs[ed->pos+killRingLen], &ucs[ed->pos],
		    (len - ed->pos)*sizeof(Uint32));
	}
	memcpy(&ucs[ed->pos], killRing, killRingLen*sizeof(Uint32));
	ucs[len+killRingLen] = '\0';
	ed->pos += killRingLen;
	AG_MutexUnlock(&killRingLock);
	return (1);
nochange:
	AG_MutexUnlock(&killRingLock);
	return (0);
}

static int
WordBackUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	Uint32 *c;

	if (ed->flags & AG_EDITABLE_NOWORDSEEK)
		return (0);
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_B) && (keymod & AG_KEYMOD_ALT))
		return (0);

	if (ed->pos > 1 && ucs[ed->pos-1] == ' ') {
		ed->pos -= 2;
	}
	for (c = &ucs[ed->pos];
	     c > &ucs[0] && *c != ' ';
	     c--, ed->pos--)
		;;
	if (*c == ' ') {
		ed->pos++;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
WordForwUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	Uint32 *c;
	
	if (ed->flags & AG_EDITABLE_NOWORDSEEK)
		return (0);
	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_F) && (keymod & AG_KEYMOD_ALT))
		return (0);

	if (ed->pos == len) {
		return (0);
	}
	if (len > 1 && ucs[ed->pos] == ' ') {
		ed->pos++;
	}
	for (c = &ucs[ed->pos];
	     *c != '\0' && *c != ' ';
	     c++, ed->pos++)
		;;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorHomeUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
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
		for (c = &ucs[ed->pos-1];
		     c >= &ucs[0] && ed->pos >= 0;
		     c--, ed->pos--) {
			if (*c == '\n')
				break;
		}
	} else {
		ed->pos = 0;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorEndUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	Uint32 *c;

	if ((ed->flags & AG_EDITABLE_NOEMACS) &&
	    (keysym == AG_KEY_E) && (keymod & AG_KEYMOD_CTRL)) {
		return (0);
	}
	if (ed->flags & AG_EDITABLE_MULTILINE) {
		if (ed->pos == len || ucs[ed->pos] == '\n') {
			return (0);
		}
		for (c = &ucs[ed->pos+1];
		     c <= &ucs[len] && ed->pos <= len;
		     c++, ed->pos++) {
			if (*c == '\n') {
				ed->pos++;
				break;
			}
		}
		if (ed->pos > len) {
			ed->pos = len;
		}
	} else {
		ed->pos = len;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorLeftUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	if (--ed->pos < 1) {
		ed->pos = 0;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorRightUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	if (ed->pos < len) {
		ed->pos++;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorUpUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	AG_EditableMoveCursor(ed, ed->xCursPref,
	    (ed->yCurs - ed->y - 1)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
CursorDownUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	AG_EditableMoveCursor(ed, ed->xCursPref,
	    (ed->yCurs - ed->y + 1)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
PageUpUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y - ed->yVis)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
PageDownUTF8(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    Uint32 *ucs, int len, int bufSize)
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

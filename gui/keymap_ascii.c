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
 * Basic keyboard input processing in ASCII mode.
 */

#include <core/core.h>
#include <core/config.h>

#include "widget.h"
#include "editable.h"
#include "keymap.h"
#include "text.h"

#include <ctype.h>
#include <string.h>

/* Emulate "shift" key for US keyboard layout. */
/* XXX move to keymap */
static Uint32
EmulateShiftUSKBD(Uint32 key)
{
	if (isalpha(key)) {
		return (toupper(key));
	} else {
		switch (key) {
		case AG_KEY_1:			return ('!');
		case AG_KEY_2:			return ('@');
		case AG_KEY_3:			return ('#');
		case AG_KEY_4:			return ('$');
		case AG_KEY_5:			return ('%');
		case AG_KEY_6:			return ('^');
		case AG_KEY_7:			return ('&');
		case AG_KEY_8:			return ('*');
		case AG_KEY_9:			return ('(');
		case AG_KEY_0:			return (')');
		case AG_KEY_BACKQUOTE:		return ('~');
		case AG_KEY_MINUS:		return ('_');
		case AG_KEY_EQUALS:		return ('+');
		case AG_KEY_LEFTBRACKET:	return ('{');
		case AG_KEY_RIGHTBRACKET:	return ('}');
		case AG_KEY_BACKSLASH:		return ('|');
		case AG_KEY_SEMICOLON:		return (':');
		case AG_KEY_QUOTE:		return ('"');
		case AG_KEY_COMMA:		return ('<');
		case AG_KEY_PERIOD:		return ('>');
		case AG_KEY_SLASH:		return ('?');
		}
	}
	return ('?');
}

/* Apply modifiers (when not using Unicode keyboard translation). */
Uint32
AG_ApplyModifiersASCII(Uint32 key, int mod)
{
	if (mod & AG_KEYMOD_CAPSLOCK) {
		if (mod & AG_KEYMOD_SHIFT) {
			return (Uint32)tolower((int)key);
		} else {
			return (EmulateShiftUSKBD(key));
		}
	} else {
		if (mod & AG_KEYMOD_SHIFT) {
			return (EmulateShiftUSKBD(key));
		} else {
			return (key);
		}
	}
}

static int
InsertASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 unicode,
    char *s, int len, int lenMax)
{
	Uint32 ins[3];
	int nins, i;
	char ch;
	
	if (AG_WidgetDisabled(ed))
		return (0);
	if (keysym == 0 || !isascii(keysym))
		return (0);

	ch = (char)AG_ApplyModifiersASCII((Uint32)keysym, keymod);
	if (ch == 0) { return (0); }
	if (ch == '\r') { ch = '\n'; }
	
	switch (ed->encoding) {
	case AG_ENCODING_UTF8:
		break;
	case AG_ENCODING_ASCII:
		if (!isascii((int)ch)) {
			return (0);
		}
		break;
	}

	if (agTextComposition) {
		if ((nins = AG_KeyInputCompose(ed, (Uint32)ch, ins)) == 0)
			return (0);
	} else {
		ins[0] = (Uint32)ch;
		nins = 1;
	}
	if (len+nins >= lenMax) {
		return (0);
	}
	ins[nins] = '\0';

	if (ed->pos == len) {		       			/* Append */
		for (i = 0; i < nins+1; i++)
			s[len+i] = ins[i];
	} else {						/* Insert */
		char *p = &s[ed->pos];
		
		memcpy(&p[nins], &p[0], (len - ed->pos));
		for (i = 0; i < nins; i++) {
			p[i] = ins[i];
		}
		s[len+1] = '\0';
	}
	ed->pos++;
	return (1);
}

static int
DeleteASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 unicode,
    char *s, int len, int lenMax)
{
	char *c;

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
			s[len-1] = '\0';
			ed->pos--;
			return (1);
		}
		ed->pos--;
		break;
	case AG_KEY_DELETE:
		if (ed->pos == len) {
			s[len-1] = '\0';
			ed->pos--;
			return (1);
		}
		break;
	default:
		break;
	}
	for (c = &s[ed->pos]; c[1] != '\0'; c++) {
		*c = c[1];
	}
	*c = '\0';
	return (1);
}

static int
KillASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (AG_WidgetDisabled(ed))
		return (0);

	/* TODO Save to a kill buffer, etc. */
	s[ed->pos] = '\0';
	return (1);
}

static int
YankASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (AG_WidgetDisabled(ed))
		return (0);

	/* TODO */
	return (1);
}

static int
WordBackASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	/* TODO */
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
WordForwASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	/* TODO */
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorHomeASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	ed->pos = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorEndASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	ed->pos = len;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorLeftASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (--ed->pos < 1) {
		ed->pos = 0;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorRightASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (ed->pos < len) {
		ed->pos++;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorUpASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	char *p;

	if ((p = strrchr(&s[ed->pos], '\n')) != NULL) {
		ed->pos -= (s - p);
	}
	return (0);
}

static int
CursorDownASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	char *p;

	if ((p = strchr(&s[ed->pos], '\n')) != NULL) {
		ed->pos -= (p - s);
	}
	return (0);
}

static int
PageUpASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y - ed->yVis*2 + 2)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
PageDownASCII(AG_Editable *ed, AG_KeySym keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y + ed->yVis*2 - 2)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

const struct ag_keycode_ascii agKeymapASCII[] = {
	{ AG_KEY_HOME,		0,		CursorHomeASCII },
	{ AG_KEY_A,		AG_KEYMOD_CTRL,	CursorHomeASCII },
	{ AG_KEY_END,		0,		CursorEndASCII },
	{ AG_KEY_E,		AG_KEYMOD_CTRL,	CursorEndASCII },
	{ AG_KEY_LEFT,		0,		CursorLeftASCII },
	{ AG_KEY_RIGHT,		0,		CursorRightASCII },
	{ AG_KEY_UP,		0,		CursorUpASCII },
	{ AG_KEY_DOWN,		0,		CursorDownASCII },
	{ AG_KEY_PAGEUP,	0,		PageUpASCII },
	{ AG_KEY_PAGEDOWN,	0,		PageDownASCII },
	{ AG_KEY_BACKSPACE,	0,		DeleteASCII },
	{ AG_KEY_DELETE,	0,		DeleteASCII },
	{ AG_KEY_K,		AG_KEYMOD_CTRL,	KillASCII },
	{ AG_KEY_Y,		AG_KEYMOD_CTRL,	YankASCII },
	{ AG_KEY_B,		AG_KEYMOD_ALT,	WordBackASCII },
	{ AG_KEY_F,		AG_KEYMOD_ALT,	WordForwASCII },
	{ AG_KEY_LAST,		0,		InsertASCII },
};

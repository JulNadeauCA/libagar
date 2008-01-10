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
#include <core/prop.h>

#include "widget.h"
#include "editable.h"
#include "keymap.h"

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
		case SDLK_1:		return ('!');
		case SDLK_2:		return ('@');
		case SDLK_3:		return ('#');
		case SDLK_4:		return ('$');
		case SDLK_5:		return ('%');
		case SDLK_6:		return ('^');
		case SDLK_7:		return ('&');
		case SDLK_8:		return ('*');
		case SDLK_9:		return ('(');
		case SDLK_0:		return (')');
		case SDLK_BACKQUOTE:	return ('~');
		case SDLK_MINUS:	return ('_');
		case SDLK_EQUALS:	return ('+');
		case SDLK_LEFTBRACKET:	return ('{');
		case SDLK_RIGHTBRACKET:	return ('}');
		case SDLK_BACKSLASH:	return ('|');
		case SDLK_SEMICOLON:	return (':');
		case SDLK_QUOTE:	return ('"');
		case SDLK_COMMA:	return ('<');
		case SDLK_PERIOD:	return ('>');
		case SDLK_SLASH:	return ('?');
		}
	}
	return ('?');
}

/* Apply modifiers (when not using Unicode keyboard translation). */
Uint32
AG_ApplyModifiersASCII(Uint32 key, int kmod)
{
	if (kmod & KMOD_CAPS) {
		if (kmod & KMOD_SHIFT) {
			return (Uint32)tolower((int)key);
		} else {
			return (EmulateShiftUSKBD(key));
		}
	} else {
		if (kmod & KMOD_SHIFT) {
			return (EmulateShiftUSKBD(key));
		} else {
			return (key);
		}
	}
}

static int
InsertASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 unicode,
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

	if (AG_Bool(agConfig,"input.composition")) {
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
DeleteASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 unicode,
    char *s, int len, int lenMax)
{
	char *c;

	if (AG_WidgetDisabled(ed))
		return (0);
	if (len == 0)
		return (0);

	switch (keysym) {
	case SDLK_BACKSPACE:
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
	case SDLK_DELETE:
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
KillASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (AG_WidgetDisabled(ed))
		return (0);

	/* TODO Save to a kill buffer, etc. */
	s[ed->pos] = '\0';
	return (1);
}

static int
YankASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (AG_WidgetDisabled(ed))
		return (0);

	/* TODO */
	return (1);
}

static int
WordBackASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	/* TODO */
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
WordForwASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	/* TODO */
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorHomeASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	ed->pos = 0;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorEndASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	ed->pos = len;
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorLeftASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (--ed->pos < 1) {
		ed->pos = 0;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorRightASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	if (ed->pos < len) {
		ed->pos++;
	}
	ed->flags |= AG_EDITABLE_MARKPREF;
	return (0);
}

static int
CursorUpASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	char *p;

	if ((p = strrchr(&s[ed->pos], '\n')) != NULL) {
		ed->pos -= (s - p);
	}
	return (0);
}

static int
CursorDownASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	char *p;

	if ((p = strchr(&s[ed->pos], '\n')) != NULL) {
		ed->pos -= (p - s);
	}
	return (0);
}

static int
PageUpASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y - ed->yVis*2 + 2)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

static int
PageDownASCII(AG_Editable *ed, SDLKey keysym, int keymod, Uint32 uch,
    char *s, int len, int lenMax)
{
	AG_EditableMoveCursor(ed, ed->xCurs,
	    (ed->yCurs - ed->y + ed->yVis*2 - 2)*agTextFontLineSkip + 1,
	    1);
	return (0);
}

const struct ag_keycode_ascii agKeymapASCII[] = {
	{ SDLK_HOME,		0,		CursorHomeASCII },
	{ SDLK_a,		KMOD_CTRL,	CursorHomeASCII },
	{ SDLK_END,		0,		CursorEndASCII },
	{ SDLK_e,		KMOD_CTRL,	CursorEndASCII },
	{ SDLK_LEFT,		0,		CursorLeftASCII },
	{ SDLK_RIGHT,		0,		CursorRightASCII },
	{ SDLK_UP,		0,		CursorUpASCII },
	{ SDLK_DOWN,		0,		CursorDownASCII },
	{ SDLK_PAGEUP,		0,		PageUpASCII },
	{ SDLK_PAGEDOWN,	0,		PageDownASCII },
	{ SDLK_BACKSPACE,	0,		DeleteASCII },
	{ SDLK_DELETE,		0,		DeleteASCII },
	{ SDLK_k,		KMOD_CTRL,	KillASCII },
	{ SDLK_y,		KMOD_CTRL,	YankASCII },
	{ SDLK_b,		KMOD_ALT,	WordBackASCII },
	{ SDLK_f,		KMOD_ALT,	WordForwASCII },
	{ SDLK_LAST,		0,		InsertASCII },
};

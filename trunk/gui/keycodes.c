/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/utf8.h>

#include <core/core.h>
#include <core/config.h>

#include "widget.h"
#include "textbox.h"
#include "keycodes.h"
#include "unicode.h"

#include <ctype.h>
#include <string.h>

#ifdef UTF8
static struct {
	Uint32 comp, key, res;
} compose[] = {
	{ 0x0060, 0x0020, 0x0060 },  /* GRAVE ACCENT */
	{ 0x0060, 0x0061, 0x00e0 },  /* LATIN SMALL LETTER A */
	{ 0x0060, 0x0041, 0x00c0 },  /* LATIN CAPITAL LETTER A */
	{ 0x0060, 0x0065, 0x00e8 },  /* LATIN SMALL LETTER E */
	{ 0x0060, 0x0045, 0x00c8 },  /* LATIN CAPITAL LETTER E */
	{ 0x0060, 0x0069, 0x00ec },  /* LATIN SMALL LETTER I */
	{ 0x0060, 0x0049, 0x00cc },  /* LATIN CAPITAL LETTER I */
	{ 0x0060, 0x006f, 0x00f2 },  /* LATIN SMALL LETTER O */
	{ 0x0060, 0x004f, 0x00d2 },  /* LATIN CAPITAL LETTER O */
	{ 0x0060, 0x0075, 0x00f9 },  /* LATIN SMALL LETTER U */
	{ 0x0060, 0x0055, 0x00d9 },  /* LATIN CAPITAL LETTER U */
	
	{ 0x00b4, 0x0020, 0x0060 },  /* ACUTE ACCENT */
	{ 0x00b4, 0x0065, 0x00e9 },  /* LATIN SMALL LETTER E */
	{ 0x00b4, 0x0045, 0x00c9 },  /* LATIN CAPITAL LETTER E */
	
	{ 0x02db, 0x0020, 0x02db },  /* OGONEK */
	{ 0x02db, 0x0061, 0x0105 },  /* LATIN SMALL LETTER C */
	{ 0x02db, 0x0041, 0x0104 },  /* LATIN CAPITAL LETTER C */
	{ 0x02db, 0x0075, 0x0173 },  /* LATIN SMALL LETTER U */
	{ 0x02db, 0x0055, 0x0172 },  /* LATIN CAPITAL LETTER U */

	{ 0x00b8, 0x0020, 0x00b8 },  /* CEDILLA */
	{ 0x00b8, 0x0063, 0x00e7 },  /* LATIN SMALL LETTER C */
	{ 0x00b8, 0x0043, 0x00c7 },  /* LATIN CAPITAL LETTER C */
	{ 0x00b8, 0x0067, 0x0123 },  /* LATIN SMALL LETTER G */
	{ 0x00b8, 0x0047, 0x0122 },  /* LATIN CAPITAL LETTER G */
	{ 0x00b8, 0x006e, 0x0146 },  /* LATIN SMALL LETTER N */
	{ 0x00b8, 0x004e, 0x0145 },  /* LATIN CAPITAL LETTER N */
	{ 0x00b8, 0x006b, 0x0137 },  /* LATIN SMALL LETTER K */
	{ 0x00b8, 0x004b, 0x0136 },  /* LATIN CAPITAL LETTER K */
	{ 0x00b8, 0x0072, 0x0157 },  /* LATIN SMALL LETTER R */
	{ 0x00b8, 0x0052, 0x0156 },  /* LATIN CAPITAL LETTER R */
	{ 0x00b8, 0x0074, 0x0163 },  /* LATIN SMALL LETTER T */
	{ 0x00b8, 0x0054, 0x0162 },  /* LATIN CAPITAL LETTER T */
	{ 0x00b8, 0x0073, 0x015f },  /* LATIN SMALL LETTER S */
	{ 0x00b8, 0x0053, 0x015e },  /* LATIN CAPITAL LETTER S */
	
	{ 0x00a8, 0x0020, 0x00a8 },  /* DIAERESIS */
	{ 0x00a8, 0x0061, 0x00e4 },  /* LATIN SMALL LETTER A */
	{ 0x00a8, 0x0041, 0x00c4 },  /* LATIN CAPITAL LETTER A */
	{ 0x00a8, 0x0065, 0x00eb },  /* LATIN SMALL LETTER E */
	{ 0x00a8, 0x0045, 0x00cb },  /* LATIN CAPITAL LETTER E */
	{ 0x00a8, 0x0069, 0x00ef },  /* LATIN SMALL LETTER I */
	{ 0x00a8, 0x0049, 0x00cf },  /* LATIN CAPITAL LETTER I */
	{ 0x00a8, 0x006f, 0x00f6 },  /* LATIN SMALL LETTER O */
	{ 0x00a8, 0x004f, 0x00d6 },  /* LATIN CAPITAL LETTER O */
	{ 0x00a8, 0x0079, 0x00ff },  /* LATIN SMALL LETTER Y */
	{ 0x00a8, 0x0059, 0x0178 },  /* LATIN CAPITAL LETTER Y */
	{ 0x00a8, 0x0075, 0x00fc },  /* LATIN SMALL LETTER U */
	{ 0x00a8, 0x0055, 0x00dc },  /* LATIN CAPITAL LETTER U */
	
	{ 0x005e, 0x0020, 0x005e },  /* CIRCUMFLEX ACCENT */
	{ 0x005e, 0x0061, 0x00e2 },  /* LATIN SMALL LETTER A */
	{ 0x005e, 0x0041, 0x00c2 },  /* LATIN CAPITAL LETTER A */
	{ 0x005e, 0x0063, 0x0109 },  /* LATIN SMALL LETTER C */
	{ 0x005e, 0x0043, 0x0108 },  /* LATIN CAPITAL LETTER C */
	{ 0x005e, 0x0065, 0x00ea },  /* LATIN SMALL LETTER E */
	{ 0x005e, 0x0045, 0x00ca },  /* LATIN CAPITAL LETTER E */
	{ 0x005e, 0x0067, 0x011d },  /* LATIN SMALL LETTER G */
	{ 0x005e, 0x0047, 0x011c },  /* LATIN CAPITAL LETTER G */
	{ 0x005e, 0x0069, 0x00ee },  /* LATIN SMALL LETTER I */
	{ 0x005e, 0x0049, 0x00ce },  /* LATIN CAPITAL LETTER I */
	{ 0x005e, 0x006f, 0x00f4 },  /* LATIN SMALL LETTER O */
	{ 0x005e, 0x004f, 0x00d4 },  /* LATIN CAPITAL LETTER O */
	{ 0x005e, 0x0073, 0x015d },  /* LATIN SMALL LETTER S */
	{ 0x005e, 0x0053, 0x015c },  /* LATIN CAPITAL LETTER S */
	{ 0x005e, 0x0079, 0x0177 },  /* LATIN SMALL LETTER Y */
	{ 0x005e, 0x0059, 0x0176 },  /* LATIN CAPITAL LETTER Y */
	{ 0x005e, 0x0075, 0x00fb },  /* LATIN SMALL LETTER U */
	{ 0x005e, 0x0055, 0x00db },  /* LATIN CAPITAL LETTER U */
	{ 0x005e, 0x0077, 0x0175 },  /* LATIN SMALL LETTER W */
	{ 0x005e, 0x0057, 0x0174 },  /* LATIN CAPITAL LETTER W */
};
static const int ncompose = sizeof(compose) / sizeof(compose[0]);
extern int agTextComposition;
#endif /* UTF8 */

static Uint32
ToUpperASCII(Uint32 key)
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

static __inline__ Uint32
ToLowerASCII(Uint32 key)
{
	return (tolower(key));
}

/* Apply modifiers (when not using Unicode keyboard translation). */
static __inline__ Uint32
InputApplyModifiers(Uint32 key, int kmod)
{
	if (kmod & KMOD_CAPS) {
		if (kmod & KMOD_SHIFT) {
			return (ToLowerASCII(key));
		} else {
			return (ToUpperASCII(key));
		}
	} else {
		if (kmod & KMOD_SHIFT) {
			return (ToUpperASCII(key));
		} else {
			return (key);
		}
	}
}

/*
 * UTF-8 Input Routines
 */
#ifdef UTF8

static int
InputCompose(AG_Textbox *tbox, Uint32 key, Uint32 *ins)
{
	int i;

	if (!agTextComposition) {
		ins[0] = key;
		return (1);
	}

	if (tbox->compose != 0) {
		for (i = 0; i < ncompose; i++) {
			if (compose[i].comp == tbox->compose &&
			    compose[i].key == key)
				break;
		}
		if (i < ncompose) {
			ins[0] = compose[i].res;
			tbox->compose = 0;
			return (1);
		} else {
			ins[0] = tbox->compose;
			ins[1] = key;
			tbox->compose = 0;
			return (2);
		}
	} else {
		for (i = 0; i < ncompose; i++) {
			if (compose[i].comp == key)
				break;
		}
		if (i < ncompose) {
			tbox->compose = key;
			return (0);
		} else {
			ins[0] = key;
			return (1);
		}
	}
}

static int
InsertUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	extern int agKbdUnicode;			/* input/kbd.c */
	AG_WidgetBinding *stringb;
	size_t len;
	Uint32 *ucs4;
	char *utf8;
	Uint32 ins[2];
	int i, nchars;

	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);
	len = AG_UCS4Len(ucs4);
#ifdef DEBUG
	if (tbox->pos < 0 || tbox->pos > len)
		fatal("bad position");
#endif
	if (agKbdUnicode) {
		if (uch == 0) {
			goto skip;
		}
		if (agTextComposition) {
			if ((nchars = InputCompose(tbox, uch, ins)) == 0)
				goto skip;
		} else {
			ins[0] = (Uint32)keysym;
			nchars = 1;
		}
	} else {
		ins[0] = InputApplyModifiers((Uint32)keysym, keymod);
		nchars = 1;
	}
	ucs4 = Realloc(ucs4, (len+nchars+1)*sizeof(Uint32));
	
	/* Ensure the new character(s) fit inside the buffer. */
	/* XXX use utf8 size for space efficiency */
	if (len+nchars >= stringb->data.size/sizeof(Uint32)) {
		dprintf("character does not fit into buffer\n");
		goto skip;
	}
	if (tbox->pos == len) {					/* Append */
#if 0
		dprintf("append at %d/%d\n", (int)tbox->pos, (int)len);
#endif
		if (agKbdUnicode) {
			if (uch != 0) {
				for (i = 0; i < nchars; i++)
					ucs4[len+i] = ins[i];
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			for (i = 0; i < nchars; i++)
				ucs4[len+i] = ins[i];
		} else {
			goto skip;
		}
	} else {						/* Insert */
#if 0
		dprintf("insert at %d/%d\n", (int)tbox->pos, (int)len);
#endif
		memmove(&ucs4[tbox->pos+nchars], &ucs4[tbox->pos],
		       (len - tbox->pos)*sizeof(Uint32));
		if (agKbdUnicode) {
			if (uch != 0) {
				for (i = 0; i < nchars; i++) {
					ucs4[tbox->pos+i] = ins[i];
				}
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			for (i = 0; i < nchars; i++)
				ucs4[len+i] = ins[i];
		} else {
			goto skip;
		}
	}
out:
#if 0
	dprintf("NUL terminating at %d+%d (sz=%d)\n", (int)len, (int)nchars,
	    (int)stringb->data.size);
#endif
	ucs4[len+nchars] = '\0';
	tbox->pos += nchars;
	AG_ExportUnicode(AG_UNICODE_TO_UTF8, stringb->p1, ucs4,
	    stringb->data.size);
skip:
	AG_WidgetBindingChanged(stringb);
	AG_WidgetUnlockBinding(stringb);
	free(ucs4);
	return (1);
}

static int
DeleteUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 unicode)
{
	AG_WidgetBinding *stringb;
	size_t len;
	Uint32 *ucs4, *c;
	char *utf8;
	int i;

	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);
	len = AG_UCS4Len(ucs4);

	if (len == 0)
		goto skip;
#ifdef DEBUG
	if (tbox->pos < 0 || tbox->pos > len)
		fatal("bad position");
#endif
	switch (keysym) {
	case SDLK_BACKSPACE:
		if (tbox->pos == 0) {
			goto skip;
		}
		if (tbox->pos == len) { 
			ucs4[len-1] = '\0';
			tbox->pos--;
			goto out;
		}
		tbox->pos--;
		break;
	case SDLK_DELETE:
		if (tbox->pos == len) {
			ucs4[len-1] = '\0';
			tbox->pos--;
			goto out;
		}
		break;
	default:
		break;
	}
	for (c = &ucs4[tbox->pos]; c[1] != '\0'; c++) {
		*c = c[1];
	}
	*c = '\0';

	if (tbox->pos == tbox->offs) {
		if ((tbox->offs -= 4) < 1)			/* XXX */
			tbox->offs = 0;
	}
out:
	AG_ExportUnicode(AG_UNICODE_TO_UTF8, stringb->p1, ucs4,
	    stringb->data.size);
skip:
	AG_WidgetBindingChanged(stringb);
	AG_WidgetUnlockBinding(stringb);
	free(ucs4);
	return (1);
}

static int
CursorEndUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	Uint32 *ucs;
	char *utf8;
	
	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);
	tbox->pos = AG_UCS4Len(ucs);
	tbox->offs = 0;
	AG_WidgetUnlockBinding(stringb);
	free(ucs);
	return (0);
}

static int
KillUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	Uint32 *ucs;
	char *utf8;

	/* XXX save to a kill buffer, etc */
	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);

	ucs[tbox->pos] = '\0';

	AG_ExportUnicode(AG_UNICODE_TO_UTF8, stringb->p1, ucs,
	    stringb->data.size);
	AG_WidgetBindingChanged(stringb);
	AG_WidgetUnlockBinding(stringb);
	free(ucs);
	return (0);
}

static int
CursorRightUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	Uint32 *ucs;
	char *utf8;

	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);

	if (tbox->pos < AG_UCS4Len(ucs)) {
		tbox->pos++;
	}

	AG_WidgetUnlockBinding(stringb);
	free(ucs);
	return (1);
}

static int
WordBackUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	Uint32 *ucs4, *c;
	char *utf8;

	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);

	if (tbox->pos > 1 && ucs4[tbox->pos-1] == ' ') {
		tbox->pos -= 2;
	}
	for (c = &ucs4[tbox->pos];
	     c > &ucs4[0] && *c != ' ';
	     c--, tbox->pos--)
		;;
	if (*c == ' ')
		tbox->pos++;

	AG_WidgetUnlockBinding(stringb);
	free(ucs4);
	return (1);
}

static int
WordForwUTF8(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	Uint32 *ucs4, *c;
	char *utf8;
	size_t len;

	stringb = AG_WidgetGetBinding(tbox, "string", &utf8);
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, utf8);
	len = AG_UCS4Len(ucs4);

	if (tbox->pos == len) {
		return (1);
	}
	if (len > 1 && ucs4[tbox->pos] == ' ') {
		tbox->pos++;
	}
	for (c = &ucs4[tbox->pos];
	     *c != '\0' && *c != ' ';
	     c++, tbox->pos++)
		;;

	AG_WidgetUnlockBinding(stringb);
	free(ucs4);
	return (1);
}

#else /* !UTF8 */

/*
 * ASCII Input Routines
 */

static int
InsertASCII(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 unicode)
{
	AG_WidgetBinding *stringb;
	size_t len;
	Uint32 *ucs;
	char *s, ch;

	if (keysym == 0 || !isascii(keysym))
		return (0);

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	len = strlen(s);
	ch = (char)InputApplyModifiers((Uint32)keysym, keymod);
#ifdef DEBUG
	if (tbox->pos < 0 || tbox->pos > len)
		fatal("bad position");
#endif

	if ((len+1)+1 >= stringb->data.size)
		goto skip;

	if (tbox->pos == len) {		       			/* Append */
		s[len] = ch;
	} else {						/* Insert */
		char *p = &s[tbox->pos];
		
		memcpy(&p[1], &p[0], (len - tbox->pos));
		*p = ch;
	}
out:
	s[len+1] = '\0';
	tbox->pos++;
skip:
	AG_WidgetBindingChanged(stringb);
	AG_WidgetUnlockBinding(stringb);
	return (1);
}

static int
DeleteASCII(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 unicode)
{
	AG_WidgetBinding *stringb;
	size_t len;
	char *s, *c;
	int i;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	len = strlen(s);

	if (len == 0)
		goto skip;
#ifdef DEBUG
	if (tbox->pos < 0 || tbox->pos > len)
		fatal("bad position");
#endif
	switch (keysym) {
	case SDLK_BACKSPACE:
		if (tbox->pos == 0) {
			goto skip;
		}
		if (tbox->pos == len) { 
			s[len-1] = '\0';
			tbox->pos--;
			goto out;
		}
		tbox->pos--;
		break;
	case SDLK_DELETE:
		if (tbox->pos == len) {
			s[len-1] = '\0';
			tbox->pos--;
			goto out;
		}
		break;
	default:
		break;
	}
	for (c = &s[tbox->pos]; c[1] != '\0'; c++) {
		*c = c[1];
	}
	*c = '\0';
out:
	if (tbox->pos == tbox->offs) {
		if ((tbox->offs -= 4) < 1)			/* XXX */
			tbox->offs = 0;
	}
skip:
	AG_WidgetBindingChanged(stringb);
	AG_WidgetUnlockBinding(stringb);
	return (1);
}

static int
CursorEndASCII(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	char *s;
	
	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	tbox->pos = strlen(s);
	tbox->offs = 0;
	AG_WidgetUnlockBinding(stringb);
	return (0);
}

static int
KillASCII(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	AG_WidgetBinding *stringb;
	char *s;

	/* XXX save to a kill buffer, etc */
	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	s[tbox->pos] = '\0';
	AG_WidgetBindingChanged(stringb);
	AG_WidgetUnlockBinding(stringb);
	return (0);
}

static int
CursorRightASCII(AG_Textbox *tbox, SDLKey keysym, int keymod,
    const char *arg, Uint32 uch)
{
	AG_WidgetBinding *stringb;
	char *s;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);

	if (tbox->pos < strlen(s)) {
		tbox->pos++;
		tbox->offs++;
	}

	AG_WidgetUnlockBinding(stringb);
	return (1);
}

static int
WordBackASCII(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	/* TODO */
	return (1);
}

static int
WordForwASCII(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	/* TODO */
	return (1);
}

#endif /* UTF8 */

static int
CursorHome(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	if (tbox->offs > 0) {
		tbox->offs = 0;
	}
	tbox->pos = 0;
	return (0);
}

static int
CursorLeft(AG_Textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	if (--tbox->pos < 1) {
		tbox->pos = 0;
	}
	if (tbox->pos == tbox->offs) {
		if (--tbox->offs < 0)
			tbox->offs = 0;
	}
	return (1);
}

const struct ag_keycode agKeyCodes[] = {
	{ SDLK_HOME,		0,		CursorHome,	NULL, 1 },
	{ SDLK_a,		KMOD_CTRL,	CursorHome,	NULL, 1 },
	{ SDLK_LEFT,		0,		CursorLeft,	NULL, 1 },
#ifdef UTF8
	{ SDLK_BACKSPACE,	0,		DeleteUTF8,	NULL, 1 },
	{ SDLK_DELETE,		0,		DeleteUTF8,	NULL, 1 },
	{ SDLK_END,		0,		CursorEndUTF8,	NULL, 1 },
	{ SDLK_e,		KMOD_CTRL,	CursorEndUTF8,	NULL, 1 },
	{ SDLK_k,		KMOD_CTRL,	KillUTF8,	NULL, 1 },
	{ SDLK_b,		KMOD_ALT,	WordBackUTF8,	NULL, 1 },
	{ SDLK_f,		KMOD_ALT,	WordForwUTF8,	NULL, 1 },
	{ SDLK_RIGHT,		0,		CursorRightUTF8, NULL, 1 },
	{ SDLK_LAST,		0,		InsertUTF8,	NULL, 0 },
#else
	{ SDLK_BACKSPACE,	0,		DeleteASCII,	NULL, 1 },
	{ SDLK_DELETE,		0,		DeleteASCII, NULL, 1 },
	{ SDLK_END,		0,		CursorEndASCII,	NULL, 1 },
	{ SDLK_e,		KMOD_CTRL,	CursorEndASCII,	NULL, 1 },
	{ SDLK_k,		KMOD_CTRL,	KillASCII,	NULL, 1 },
	{ SDLK_b,		KMOD_ALT,	WordBackASCII, NULL, 1 },
	{ SDLK_f,		KMOD_ALT,	WordForwASCII, NULL, 1 },
	{ SDLK_RIGHT,		0,		CursorRightASCII, NULL, 1 },
	{ SDLK_LAST,		0,		InsertASCII,	NULL, 0 },
#endif
};

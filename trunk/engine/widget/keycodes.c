/*	$Csoft: keycodes.c,v 1.39 2005/02/07 13:17:16 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/input.h>
#include <engine/prop.h>
#include <engine/config.h>

#include <ctype.h>
#include <string.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

#ifdef UTF8
static int key_del_utf8(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_end_utf8(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_kill_utf8(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_right_utf8(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_char_utf8(struct textbox *, SDLKey, int, const char *, Uint32);
#else
static int key_del_ascii(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_end_ascii(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_kill_ascii(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_right_ascii(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_char_ascii(struct textbox *, SDLKey, int, const char *, Uint32);
#endif

static int key_home(struct textbox *, SDLKey, int, const char *, Uint32);
static int key_left(struct textbox *, SDLKey, int, const char *, Uint32);

const struct keycode keycodes[] = {
	{ SDLK_HOME,		0,		key_home,	NULL, 1 },
	{ SDLK_a,		KMOD_CTRL,	key_home,	NULL, 1 },
	{ SDLK_LEFT,		0,		key_left,	NULL, 1 },
#ifdef UTF8
	{ SDLK_BACKSPACE,	0,		key_del_utf8,	NULL, 1 },
	{ SDLK_DELETE,		0,		key_del_utf8,	NULL, 1 },
	{ SDLK_END,		0,		key_end_utf8,	NULL, 1 },
	{ SDLK_e,		KMOD_CTRL,	key_end_utf8,	NULL, 1 },
	{ SDLK_k,		KMOD_CTRL,	key_kill_utf8,	NULL, 1 },
	{ SDLK_RIGHT,		0,		key_right_utf8,	NULL, 1 },
	{ SDLK_LAST,		0,		key_char_utf8,	NULL, 0 },
#else
	{ SDLK_BACKSPACE,	0,		key_del_ascii,	NULL, 1 },
	{ SDLK_DELETE,		0,		key_del_ascii, NULL, 1 },
	{ SDLK_END,		0,		key_end_ascii,	NULL, 1 },
	{ SDLK_e,		KMOD_CTRL,	key_end_ascii,	NULL, 1 },
	{ SDLK_k,		KMOD_CTRL,	key_kill_ascii,	NULL, 1 },
	{ SDLK_RIGHT,		0,		key_right_ascii, NULL, 1 },
	{ SDLK_LAST,		0,		key_char_ascii,	NULL, 0 },
#endif
};

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
#endif /* UTF8 */

/* Apply modifiers (when not using Unicode keyboard translation). */
static __inline__ Uint32
key_apply_mod(Uint32 key, int kmod)
{
	if (kmod & KMOD_CAPS) {
		if (kmod & KMOD_SHIFT) {
			return (tolower(key));
		} else {
			return (toupper(key));
		}
	} else {
		if (kmod & KMOD_SHIFT) {
			return (toupper(key));
		} else {
			return (key);
		}
	}
}

#ifdef UTF8

/* Perform simple input composition. */
static int
key_compose(struct textbox *tbox, Uint32 key, Uint32 *ins)
{
	extern int text_composition;
	int i;

	if (!text_composition) {
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

/* Insert an Unicode character. */
static int
key_char_utf8(struct textbox *tbox, SDLKey keysym, int keymod,
    const char *arg, Uint32 uch)
{
	extern int kbd_unitrans;			/* input/kbd.c */
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	Uint32 ins[2];
	int i, nins;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	if (kbd_unitrans) {
		if (uch == 0) {
			goto skip;
		}
		nins = key_compose(tbox, uch, ins);
	} else {
		ins[0] = key_apply_mod((Uint32)keysym, keymod);
		nins = 1;
	}

	/* Ensure the new character(s) fit inside the buffer. */
	/* XXX use utf8 size for space efficiency */
	if (len+nins >= stringb->size/sizeof(Uint32))
		goto skip;

	dprintf("len=%d nins=%d s32=%d pos=%d\n", len, nins,
	    stringb->size/sizeof(Uint32), tbox->pos);

	if (tbox->pos == len) {
		/* Append to the end of string */
		if (kbd_unitrans) {
			if (uch != 0) {
				for (i = 0; i < nins; i++) {
					dprintf("append ucs %d: %d\n", len+i, 
					    ins[i]);
					ucs[len+i] = ins[i];
				}
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			for (i = 0; i < nins; i++)
				dprintf("append ascii %d: %d\n", len+i, ins[i]);
				ucs[len+i] = ins[i];
		} else {
			goto skip;
		}
	} else {
		Uint32 *p = ucs + tbox->pos;
		
		dprintf("insert ucs at %d (%d bytes)\n", tbox->pos,
		    (len - tbox->pos)*sizeof(Uint32));

		/* Insert at the cursor position in the string. */
		memcpy(p+nins, p, (len - tbox->pos)*sizeof(Uint32));
		if (kbd_unitrans) {
			if (uch != 0) {
				for (i = 0; i < nins; i++) {
					dprintf("insert ucs %d: %d\n",
					    tbox->pos+i, ins[i]);
					ucs[tbox->pos+i] = ins[i];
				}
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			for (i = 0; i < nins; i++) {
				dprintf("insert ascii %d: %d\n",
				    tbox->pos+i, ins[i]);
				ucs[len+i] = ins[i];
			}
		} else {
			goto skip;
		}
	}
out:
	ucs[len+nins] = '\0';
	tbox->pos += nins;
	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
skip:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
	return (1);
}

#else /* !UTF8 */

/* Insert an ASCII character. */
static int
key_char_ascii(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 unicode)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *s;
	char c;
	int i;

	if (keysym == 0 || !isascii(keysym))
		return (0);

	stringb = widget_get_binding(tbox, "string", &s);
	len = strlen(s);
	c = (char)key_apply_mod((Uint32)keysym, keymod);

	if ((len+1)+1 >= stringb->size)
		goto skip;

	if (tbox->pos == len) {		       			/* Append */
		s[len] = c;
	} else {						/* Insert */
		char *p = &s[tbox->pos];
		
		memcpy(&p[1], &p[0], (len - tbox->pos));
		*p = c;
	}
out:
	s[len+1] = '\0';
	tbox->pos++;
	dprintf("pos -> %d\n", tbox->pos);
skip:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	return (1);
}

#endif /* UTF8 */

#ifdef UTF8

static int
key_del_utf8(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 unicode)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	int i;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	if (tbox->pos == 0 || len == 0)
		goto out;

	if (tbox->pos == len) {
		ucs[--tbox->pos] = '\0';
	} else if (tbox->pos > 0) {
		/* XXX use memmove */
		for (i = tbox->pos-1; i < len; i++) {
			ucs[i] = ucs[i+1];
		}
		tbox->pos--;
	}

	if (tbox->pos == tbox->offs) {
		if ((tbox->offs -= 4) < 1)
			tbox->offs = 0;
	}
	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
out:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
	return (1);
}

#else /* !UTF8 */

/* Destroy the ASCII character before the cursor. */
static int
key_del_ascii(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 unicode)
{
	struct widget_binding *stringb;
	size_t len;
	char *s;
	int pos = tbox->pos;
	int i;

	stringb = widget_get_binding(tbox, "string", &s);
	len = strlen(s);

	if (len == 0)
		goto out;
	
	if (keysym == SDLK_BACKSPACE) {
		if (pos == 0) {
			goto out;
		}
		pos -= 1;
		tbox->pos--;
	}

	if (pos == len) {
		s[len-1] = '\0';
		tbox->pos--;
	} else if (pos >= 0) {
		memmove(&s[pos], &s[pos+1], len+1);
	}

	if (tbox->pos == tbox->offs) {
		if ((tbox->offs -= 4) < 1)
			tbox->offs = 0;
	}
out:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	return (1);
}
#endif /* UTF8 */

/* Move the cursor to the start of the string. */
static int
key_home(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	if (tbox->offs > 0) {
		tbox->offs = 0;
	}
	tbox->pos = 0;
	return (0);
}

#ifdef UTF8
/* Move the cursor to the end of the UTF-8 string. */
static int
key_end_utf8(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	Uint32 *ucs;
	char *utf8;
	
	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	tbox->pos = ucs4_len(ucs);
	tbox->offs = 0;
	widget_binding_unlock(stringb);
	free(ucs);
	return (0);
}

#else /* !UTF8 */

/* Move the cursor to the end of the ASCII string. */
static int
key_end_ascii(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	char *s;
	
	stringb = widget_get_binding(tbox, "string", &s);
	tbox->pos = strlen(s);
	tbox->offs = 0;
	widget_binding_unlock(stringb);
	return (0);
}
#endif /* UTF8 */

#ifdef UTF8
/* Kill the text after the cursor. */
/* XXX save to a kill buffer, etc */
static int
key_kill_utf8(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	Uint32 *ucs;
	char *utf8;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);

	ucs[tbox->pos] = '\0';

	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
	return (0);
}
#else /* !UTF8 */
/* Kill the text after the cursor. */
/* XXX save to a kill buffer, etc */
static int
key_kill_ascii(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_get_binding(tbox, "string", &s);
	s[tbox->pos] = '\0';
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	return (0);
}
#endif

/* Move the cursor to the left. */
static int
key_left(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
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

#ifdef UTF8
/* Move the cursor to the right. */
static int
key_right_utf8(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	Uint32 *ucs;
	char *utf8;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);

	if (tbox->pos < ucs4_len(ucs)) {
		tbox->pos++;
	}

	widget_binding_unlock(stringb);
	free(ucs);
	return (1);
}

#else /* !UTF8 */

/* Move the cursor to the right. */
static int
key_right_ascii(struct textbox *tbox, SDLKey keysym, int keymod,
    const char *arg, Uint32 uch)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_get_binding(tbox, "string", &s);

	if (tbox->pos < strlen(s))
		tbox->pos++;

	widget_binding_unlock(stringb);
	return (1);
}

#endif /* UTF8 */

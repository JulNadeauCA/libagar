/*	$Csoft: keycodes.c,v 1.27 2003/06/15 05:08:43 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

static void key_bspace(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_delete(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_home(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_end(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_kill(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_left(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_right(struct textbox *, SDLKey, int, const char *, Uint16);
static void key_character(struct textbox *, SDLKey, int, const char *, Uint16);

const struct keycode keycodes[] = {
	{ SDLK_BACKSPACE,	0,		key_bspace,	NULL },
	{ SDLK_DELETE,		0,		key_delete,	NULL },
	{ SDLK_HOME,		0,		key_home,	NULL },
	{ SDLK_END,		0,		key_end,	NULL },
	{ SDLK_a,		KMOD_CTRL,	key_home,	NULL },
	{ SDLK_e,		KMOD_CTRL,	key_end,	NULL },
	{ SDLK_k,		KMOD_CTRL,	key_kill,	NULL },
	{ SDLK_LEFT,		0,		key_left,	NULL },
	{ SDLK_RIGHT,		0,		key_right,	NULL },
	{ SDLK_LAST,		0,		key_character,	NULL }
};

static void
key_character(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	struct widget_binding *stringb;
	size_t len;
	void *text;
	char *s, *sp;
	Uint16 *ucs, *ucsp;
	int trans;

	trans = prop_get_bool(config, "input.unicode");
	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		s = text;
		len = strlen(s);
		if (len+1 >= stringb->size) {
			goto fail;
		}
		if (tbox->pos == len) {
			if (trans && uch != 0) {
				s[len] = (unsigned char)uch;
			} else if (keysym != 0) {
				dprintf("keysym fallback: 0x%02x\n", keysym);
				s[len] = (unsigned char)keysym;
			} else {
				goto fail;
			}
		} else {
			sp = s + tbox->pos;
			memcpy(sp+1, sp, len - tbox->pos);
			if (trans && uch != 0) {
				s[tbox->pos] = (unsigned char)uch;
			} else if (keysym != 0) {
				dprintf("keysym fallback: 0x%02x\n", keysym);
				s[tbox->pos] = (unsigned char)keysym;
			} else {
				goto fail;
			}
		}
		s[len+1] = '\0';
		break;
	case WIDGET_UNICODE:
		ucs = text;
		len = ucslen(ucs);
		if (len+1 >= stringb->size/sizeof(Uint16)) {
			goto fail;
		}
		if (tbox->pos == len) {
			if (trans && uch != 0) {
				ucs[len] = uch;
			} else if (keysym != 0) {
				dprintf("keysym fallback: 0x%04x\n", keysym);
				ucs[len] = (Uint16)keysym;
			} else {
				goto fail;
			}
		} else {
			ucsp = ucs + tbox->pos;
			memcpy(ucsp+1, ucsp, (len - tbox->pos)*sizeof(Uint16));
			if (trans && uch != 0) {
				ucs[tbox->pos] = uch;
			} else if (keysym != 0) {
				dprintf("keysym fallback: 0x%04x\n", keysym);
				ucs[tbox->pos] = (Uint16)keysym;
			} else {
				goto fail;
			}
		}
		ucs[len+1] = '\0';
		break;
	}
	tbox->pos++;
	widget_binding_unlock(stringb);
	return;
fail:
	tbox->pos = 0;
	widget_binding_unlock(stringb);
}

static void
key_bspace(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	struct widget_binding *stringb;
	size_t len;
	void *text;
	char *s;
	Uint16 *ucs;
	int i;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		s = text;
		len = strlen(s);
		if (tbox->pos == 0 || len == 0) {
			goto out;
		}
		if (tbox->pos == len) {
			s[--tbox->pos] = '\0';
		} else if (tbox->pos > 0) {
			/* XXX use memmove */
			for (i = tbox->pos-1; i < len; i++) {
				s[i] = s[i+1];
			}
			tbox->pos--;
		}
		break;
	case WIDGET_UNICODE:
		ucs = text;
		len = ucslen(ucs);
		if (tbox->pos == 0 || len == 0) {
			goto out;
		}
		if (tbox->pos == len) {
			ucs[--tbox->pos] = '\0';
		} else if (tbox->pos > 0) {
			/* XXX use memmove */
			for (i = tbox->pos-1; i < len; i++) {
				ucs[i] = ucs[i+1];
			}
			tbox->pos--;
		}
		break;
	}

	if (tbox->pos == tbox->offs) {
		if ((tbox->offs -= 4) < 1)
			tbox->offs = 0;
	}
out:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_delete(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	struct widget_binding *stringb;
	size_t len;
	void *text;
	char *s;
	Uint16 *ucs;
	int i;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		s = text;
		len = strlen(s);
		if (tbox->pos == len && len > 0) {
			s[--tbox->pos] = '\0';
		} else if (tbox->pos >= 0) {
			/* XXX use memmove */
			for (i = tbox->pos; i < len; i++)
				s[i] = s[i+1];
		}
		break;
	case WIDGET_UNICODE:
		ucs = text;
		len = ucslen(ucs);
		if (tbox->pos == len && len > 0) {
			ucs[--tbox->pos] = '\0';
		} else if (tbox->pos >= 0) {
			/* XXX use memmove */
			for (i = tbox->pos; i < len; i++)
				ucs[i] = ucs[i+1];
		}
		break;
	}
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_home(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	if (tbox->offs > 0) {
		tbox->offs = 0;
	}
	tbox->pos = 0;
}

static void
key_end(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	struct widget_binding *stringb;
	void *text;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		tbox->pos = strlen((char *)text);
		break;
	case WIDGET_UNICODE:
		tbox->pos = ucslen((Uint16 *)text);
		break;
	}
	tbox->offs = 0;					/* Will seek */
	widget_binding_unlock(stringb);
}

static void
key_kill(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	struct widget_binding *stringb;
	void *text;
	char *s;
	Uint16 *ucs;

	/* XXX save to a kill buffer */
	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		s = text;
		s[tbox->pos] = '\0';
		break;
	case WIDGET_UNICODE:
		ucs = text;
		ucs[tbox->pos] = '\0';
		break;
	}
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_left(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	if (--tbox->pos < 1) {
		tbox->pos = 0;
	}
	if (tbox->pos == tbox->offs) {
		if (--tbox->offs < 0)
			tbox->offs = 0;
	}
}

static void
key_right(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint16 uch)
{
	struct widget_binding *stringb;
	void *text;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		if (tbox->pos < strlen((char *)text))
			tbox->pos++;
		break;
	case WIDGET_UNICODE:
		if (tbox->pos < ucslen((Uint16 *)text))
			tbox->pos++;
		break;
	}
	widget_binding_unlock(stringb);
}


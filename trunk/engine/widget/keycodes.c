/*	$Csoft: keycodes.c,v 1.31 2003/08/30 02:32:48 vedge Exp $	    */

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

static void key_bspace(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_delete(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_home(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_end(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_kill(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_left(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_right(struct textbox *, SDLKey, int, const char *, Uint32);
static void key_character(struct textbox *, SDLKey, int, const char *, Uint32);

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
	{ SDLK_LAST,		0,		key_character,	NULL },
};

static void
key_character(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	int trans;

	trans = prop_get_bool(config, "input.unicode");

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	/* Ensure the new character fits inside the buffer. */
	if (len+1 >= stringb->size/sizeof(Uint32))
		goto out;

	if (tbox->pos == len) {
		/* Append to the end of string */
		if (trans) {
			if (uch != 0) {
				ucs[len] = uch;
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			ucs[len] = (Uint32)keysym;
		} else {
			goto out;
		}
	} else {
		Uint32 *p = ucs + tbox->pos;

		/* Insert at the cursor position in the string. */
		memcpy(p+1, p, (len - tbox->pos)*sizeof(Uint32));
		if (trans) {
			if (uch != 0) {
				ucs[tbox->pos] = uch;
			} else {
				goto out;
			}
		} else if (keysym != 0) {
			ucs[tbox->pos] = (Uint32)keysym;
		} else {
			goto out;
		}
	}
	ucs[len+1] = '\0';
	tbox->pos++;
	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
out:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Destroy the character before the cursor. */
static void
key_bspace(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
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
}

/* Eliminate the character at the cursor position. */
static void
key_delete(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	struct widget_binding *stringb;
	size_t len;
	Uint32 *ucs;
	char *utf8;
	int i;

	stringb = widget_get_binding(tbox, "string", &utf8);
	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	len = ucs4_len(ucs);

	if (tbox->pos == len && len > 0) {		/* End of string */
		ucs[--tbox->pos] = '\0';
	} else if (tbox->pos >= 0) {			/* Middle of string */
		/* XXX use memmove */
		for (i = tbox->pos; i < len; i++)
			ucs[i] = ucs[i+1];
	}

	unicode_export(UNICODE_TO_UTF8, stringb->p1, ucs, stringb->size);
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
	free(ucs);
}

/* Move the cursor to the start of the string. */
static void
key_home(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
    Uint32 uch)
{
	if (tbox->offs > 0) {
		tbox->offs = 0;
	}
	tbox->pos = 0;
}

/* Move the cursor to the end of the string. */
static void
key_end(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
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
}

/* Kill the text after the cursor. */
/* XXX save to a kill buffer, etc */
static void
key_kill(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
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
}

/* Move the cursor to the left. */
static void
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
}

/* Move the cursor to the right. */
static void
key_right(struct textbox *tbox, SDLKey keysym, int keymod, const char *arg,
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
}


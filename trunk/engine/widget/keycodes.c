/*	$Csoft: keycodes.c,v 1.24 2003/03/25 13:48:08 vedge Exp $	    */

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

#include <ctype.h>
#include <string.h>

#include <engine/widget/text.h>
#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

static void	 insert_alpha(struct textbox *, SDLKey, int, char *);
static void	 insert_ascii(struct textbox *, SDLKey, int, char *);

static void	 key_bspace(struct textbox *, SDLKey, int, char *);
static void	 key_delete(struct textbox *, SDLKey, int, char *);
static void	 key_home(struct textbox *, SDLKey, int, char *);
static void	 key_end(struct textbox *, SDLKey, int, char *);
static void	 key_kill(struct textbox *, SDLKey, int, char *);
static void	 key_left(struct textbox *, SDLKey, int, char *);
static void	 key_right(struct textbox *, SDLKey, int, char *);

#include "keymaps/us.h"

static char *
insert_char(struct textbox *tbox, char c, char *s, size_t buflen)
{
	size_t len;
	
	len = strlen(s);
	if (len+1 >= buflen)
		return (NULL);

	if (tbox->text.pos == len) {
		s[len] = c;
	} else {
		char *sp = s + tbox->text.pos;

		memcpy(sp+1, sp, len-tbox->text.pos);
		s[tbox->text.pos] = c;
	}
	s[len+1] = '\0';
	tbox->text.pos++;
	return (&s[len]);
}

static void
insert_alpha(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s, *c;
	int i;
	size_t arglen;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	arglen = strlen(arg);

	for (i = 0; i < arglen; i++) {
		if ((c = insert_char(tbox, arg[i], s, stringb->size)) == NULL)
			break;
		if (keymod & KMOD_CAPS) {
			(int)*c = (keymod & KMOD_SHIFT) ?
			    tolower((int)*c) : toupper((int)*c);
		} else if (keymod & KMOD_SHIFT) {
			(int)*c = toupper((int)*c);
		}
	}

	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
insert_ascii(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;
	int i;
	size_t arglen;
	
	stringb = widget_binding_get_locked(tbox, "string", &s);
	arglen = strlen(arg);

	for (i = 0; i < arglen; i++) {
		if (insert_char(tbox, arg[i], s, stringb->size) == NULL)
			break;
	}

	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_bspace(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	size_t len;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	len = strlen(s);
	if (tbox->text.pos == 0 || len == 0)
		goto out;

	if (tbox->text.pos == len) {
		s[--tbox->text.pos] = '\0';
	} else if (tbox->text.pos > 0) {
		int i;

		/* XXX use memmove */
		for (i = tbox->text.pos-1; i < len; i++)
			s[i] = s[i+1];
		tbox->text.pos--;
	}
	
	if (tbox->text.pos == tbox->text.offs) {
		if ((tbox->text.offs -= 4) < 1)
			tbox->text.offs = 0;
	}
out:
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_delete(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	size_t len;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	len = strlen(s);

	if (tbox->text.pos == len && len > 0) {
		s[--tbox->text.pos] = '\0';
	} else if (tbox->text.pos >= 0) {
		int i;

		/* XXX use memmove */
		for (i = tbox->text.pos; i < len; i++)
			s[i] = s[i+1];
	}
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_home(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	tbox->text.pos = 0;

	if (tbox->text.offs > 0)
		tbox->text.offs = 0;
}

static void
key_end(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	tbox->text.pos = strlen(s);
	widget_binding_unlock(stringb);

	/* XXX botch */
	tbox->text.offs = 0;
}

static void
key_kill(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	s[tbox->text.pos] = '\0';
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_left(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	if (--tbox->text.pos < 1)
		tbox->text.pos = 0;

	if (tbox->text.pos == tbox->text.offs) {
		if (--tbox->text.offs < 0)
			tbox->text.offs = 0;
	}
}

static void
key_right(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	if (tbox->text.pos < strlen(s))
		tbox->text.pos++;
	widget_binding_unlock(stringb);
}


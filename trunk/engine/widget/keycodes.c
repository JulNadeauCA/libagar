/*	$Csoft: keycodes.c,v 1.25 2003/05/24 15:43:55 vedge Exp $	    */

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

	if (tbox->pos == len) {
		s[len] = c;
	} else {
		char *sp = s + tbox->pos;

		memcpy(sp+1, sp, len-tbox->pos);
		s[tbox->pos] = c;
	}
	s[len+1] = '\0';
	tbox->pos++;
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
	if (tbox->pos == 0 || len == 0)
		goto out;

	if (tbox->pos == len) {
		s[--tbox->pos] = '\0';
	} else if (tbox->pos > 0) {
		int i;

		/* XXX use memmove */
		for (i = tbox->pos-1; i < len; i++)
			s[i] = s[i+1];
		tbox->pos--;
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
key_delete(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	size_t len;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	len = strlen(s);

	if (tbox->pos == len && len > 0) {
		s[--tbox->pos] = '\0';
	} else if (tbox->pos >= 0) {
		int i;

		/* XXX use memmove */
		for (i = tbox->pos; i < len; i++)
			s[i] = s[i+1];
	}
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_home(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	tbox->pos = 0;

	if (tbox->offs > 0)
		tbox->offs = 0;
}

static void
key_end(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	tbox->pos = strlen(s);
	widget_binding_unlock(stringb);

	/* XXX botch */
	tbox->offs = 0;
}

static void
key_kill(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	s[tbox->pos] = '\0';
	widget_binding_modified(stringb);
	widget_binding_unlock(stringb);
}

static void
key_left(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	if (--tbox->pos < 1)
		tbox->pos = 0;

	if (tbox->pos == tbox->offs) {
		if (--tbox->offs < 0)
			tbox->offs = 0;
	}
}

static void
key_right(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(tbox, "string", &s);
	if (tbox->pos < strlen(s))
		tbox->pos++;
	widget_binding_unlock(stringb);
}


/*	$Csoft: keycodes.c,v 1.18 2002/09/07 05:11:40 vedge Exp $	    */

/*
 * Copyright (c) 2002 CubeSoft Communications <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <ctype.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "textbox.h"
#include "keycodes.h"

static char	*insert_char(struct textbox *, char);

static void	 insert_alpha(struct textbox *, SDLKey, int, char *);
static void	 insert_ascii(struct textbox *, SDLKey, int, char *);

static void	 key_bspace(struct textbox *, SDLKey, int, char *);
static void	 key_delete(struct textbox *, SDLKey, int, char *);
static void	 key_home(struct textbox *, SDLKey, int, char *);
static void	 key_end(struct textbox *, SDLKey, int, char *);
static void	 key_kill(struct textbox *, SDLKey, int, char *);
static void	 key_left(struct textbox *, SDLKey, int, char *);
static void	 key_right(struct textbox *, SDLKey, int, char *);

extern TTF_Font *font;		/* XXX pref */

#if KEYCODES_KEYMAP == KEYMAP_US
# include "keymaps/us.h"
#elif KEYCODES_KEYMAP == KEYMAP_UTU
# include "keymaps/utu.h"
#else
# error "Unknown KEYCODES_KEYMAP"
#endif

static char *
insert_char(struct textbox *tbox, char c)
{
	int end;
	char *s;
	
	end = strlen(tbox->text.s);
	tbox->text.s = erealloc(tbox->text.s, end + 2);
	if (tbox->text.pos == end) {
		tbox->text.s[end] = c;
	} else {
		s = tbox->text.s + tbox->text.pos;
		memcpy(s + 1, s, end - tbox->text.pos);
		tbox->text.s[tbox->text.pos] = c;
	}
	tbox->text.s[end + 1] = '\0';
	tbox->text.pos++;

	return (&tbox->text.s[end]);
}

static void
insert_alpha(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	char *c;
	int i, il;

	for (i = 0, il = strlen(arg); i < il; i++) {
		c = insert_char(tbox, arg[i]);
		if (keymod & KMOD_CAPS) {
			(int)*c = (keymod & KMOD_SHIFT) ?
			    tolower((int)*c) : toupper((int)*c);
		} else if (keymod & KMOD_SHIFT) {
			(int)*c = toupper((int)*c);
		}
	}
}

static void
insert_ascii(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	int i, il;

	for (il = strlen(arg), i = 0; i < il; i++) {
		insert_char(tbox, arg[i]);
	}
}

static void
key_bspace(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	int textlen;

	textlen = strlen(tbox->text.s);

	if (tbox->text.pos == textlen) {
		tbox->text.s[--tbox->text.pos] = '\0';
	} else if (tbox->text.pos > 0) {
		int i;

		for (i = tbox->text.pos-1; i < textlen; i++) {
			tbox->text.s[i] =
			tbox->text.s[i+1];
		}
		tbox->text.pos--;
	}
	
	if (tbox->text.pos == tbox->text.offs) {
		tbox->text.offs -= 4;
		if (tbox->text.offs < 1)
			tbox->text.offs = 0;
	}
}

static void
key_delete(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	int textlen, i;

	textlen = strlen(tbox->text.s);

	if (tbox->text.pos == textlen) {
		tbox->text.s[--tbox->text.pos] = '\0';
	} else if (tbox->text.pos > 0) {
		for (i = tbox->text.pos; i < textlen; i++) {
			tbox->text.s[i] = tbox->text.s[i + 1];
		}
	}
}

static void
key_home(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	tbox->text.pos = 0;
	if (tbox->text.offs > 0) {
		tbox->text.offs = 0;
	}
}

static void
key_end(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	tbox->text.pos = strlen(tbox->text.s);

	/* XXX botch */
	tbox->text.offs = 0;
}

static void
key_kill(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	tbox->text.s[tbox->text.pos] = '\0';
}

static void
key_left(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	if (--tbox->text.pos < 1) {
		tbox->text.pos = 0;
	}
	if (tbox->text.pos == tbox->text.offs) {
		tbox->text.offs--;
	}
}

static void
key_right(struct textbox *tbox, SDLKey keysym, int keymod, char *arg)
{
	if (tbox->text.pos < strlen(tbox->text.s)) {
		tbox->text.pos++;
	}
}

void
keycodes_init(void)
{
	memset(keycodes_cache, (int)NULL, sizeof(*keycodes_cache));
}

void
keycodes_loadglyphs(void)
{
	static SDL_Color white = { 255, 255, 255 };	/* XXX pref */
	char c[2];
	unsigned char an;
	int i;

	for (i = (int)KEYCODES_CACHE_START,
	     an = (unsigned char)KEYCODES_CACHE_START;
	     i <= (int)KEYCODES_CACHE_END; i++) {
		SDL_Surface *s;
	
		s = keycodes_cache[i-KEYCODES_CACHE_START];
		if (s != NULL) {
			SDL_FreeSurface(s);
		}

		c[0] = an++;
		c[1] = '\0';
		s = TTF_RenderText_Solid(font, c, white);
		if (s == NULL) {
			warning("TTF_RenderText_Solid: %s\n", SDL_GetError());
			keycodes_cache[i-KEYCODES_CACHE_START] = NULL;
		} else {
			keycodes_cache[i-KEYCODES_CACHE_START] = s;
		}
	}
}

void
keycodes_freeglyphs(void)
{
	int i;
	SDL_Surface *s;

	for (i = KEYCODES_CACHE_START; i <= KEYCODES_CACHE_END; i++) {
		s = keycodes_cache[i-KEYCODES_CACHE_START];
		if (s != NULL) {
			SDL_FreeSurface(s);
			keycodes_cache[i-KEYCODES_CACHE_START] = NULL;
		}
	}
}


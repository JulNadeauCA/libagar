/*	$Csoft	    */

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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <engine/engine.h>
#include <engine/queue.h>

#include "text.h"
#include "window.h"
#include "widget.h"
#include "textbox.h"
#include "keycodes.h"

static char	*insert_char(struct textbox *, char);

static void	 insert_alpha(struct textbox *, SDL_Event *, char *);
static void	 insert_ascii(struct textbox *, SDL_Event *, char *);

static void	 key_bspace(struct textbox *, SDL_Event *, char *);
static void	 key_delete(struct textbox *, SDL_Event *, char *);
static void	 key_home(struct textbox *, SDL_Event *, char *);
static void	 key_end(struct textbox *, SDL_Event *, char *);
static void	 key_kill(struct textbox *, SDL_Event *, char *);
static void	 key_left(struct textbox *, SDL_Event *, char *);
static void	 key_right(struct textbox *, SDL_Event *, char *);

extern TTF_Font *font;		/* XXX pref */

#if KEYCODES_KEYMAP == KEYMAP_US
# include "keymaps/us.h"
#else
# error "Unknown KEYCODES_KEYMAP"
#endif

static char *
insert_char(struct textbox *tbox, char c)
{
	int end;
	char *s;
	
	end = strlen(tbox->text);
	tbox->text = erealloc(tbox->text, end+1);
	if (tbox->textpos == end) {
		tbox->text[end] = c;
	} else {
		s = tbox->text + tbox->textpos;
		memcpy(s+1, s, end - tbox->textpos);
		tbox->text[tbox->textpos] = c;
	}
	tbox->text[end+1] = '\0';
	tbox->textpos++;

	WIDGET(tbox)->win->redraw++;

	return (&tbox->text[end]);
}

static void
insert_alpha(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	char *c;

	c = insert_char(tbox, (char)ev->key.keysym.sym);
	if ((ev->key.keysym.mod & KMOD_SHIFT) && isalpha((int)*c)) {
		(int)*c = toupper((int)*c);
	}
}

static void
insert_ascii(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	int i, il;

	for (il = strlen(arg), i = 0; i < il; i++) {
		insert_char(tbox, arg[i]);
	}

}

static void
key_bspace(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	int textlen;

	textlen = strlen(tbox->text);

	if (tbox->textpos == textlen) {
		tbox->text[--tbox->textpos] = '\0';
	} else if (tbox->textpos > 0) {
		int i;

		for (i = tbox->textpos-1; i < textlen; i++) {
			tbox->text[i] =
			tbox->text[i+1];
		}
		tbox->textpos--;
	}
}

static void
key_delete(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	int textlen;

	textlen = strlen(tbox->text);

	if (tbox->textpos == textlen) {
		tbox->text[--tbox->textpos] = '\0';
	} else if (tbox->textpos > 0) {
		int i;

		for (i = tbox->textpos-1; i < textlen; i++) {
			tbox->text[i] =
			tbox->text[i+1];
		}
		tbox->textpos--;
	}
}

static void
key_home(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	tbox->textpos = 0;
}

static void
key_end(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	tbox->textpos = strlen(tbox->text);
}

static void
key_kill(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	tbox->text[tbox->textpos] = '\0';
}

static void
key_left(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	if (--tbox->textpos < 1) {
		tbox->textpos = 0;
	}
}

static void
key_right(struct textbox *tbox, SDL_Event *ev, char *arg)
{
	if (tbox->textpos < strlen(tbox->text)) {
		tbox->textpos++;
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
	dprintf("cached %d glyphs\n", i);
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
	dprintf("freed %d glyphs\n", i);
}


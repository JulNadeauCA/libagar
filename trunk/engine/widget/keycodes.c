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

#include "window.h"
#include "widget.h"
#include "textbox.h"
#include "keycodes.h"

static void	 textbox_insert_alpha(struct textbox *, SDL_Event *);
static void	 textbox_insert_digit(struct textbox *, SDL_Event *);
static void	 textbox_insert_ascii(struct textbox *, SDL_Event *);
static char	*textbox_insert_char(struct textbox *, char);

static void	 textbox_key_backspace(struct textbox *, SDL_Event *);
static void	 textbox_key_home(struct textbox *, SDL_Event *);
static void	 textbox_key_end(struct textbox *, SDL_Event *);
static void	 textbox_key_kill(struct textbox *, SDL_Event *);
static void	 textbox_key_left(struct textbox *, SDL_Event *);
static void	 textbox_key_right(struct textbox *, SDL_Event *);

const struct keycode textbox_keycodes[] = {
	/* Ascii characters */
	{ "i-exclaim",	SDLK_1,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-amper",	SDLK_2,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-hash",	SDLK_3,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-dollar",	SDLK_4,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-percent",	SDLK_5,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-caret",	SDLK_6,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-amper",	SDLK_7,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-asterisk",	SDLK_8,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-lparent",	SDLK_9,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-rparen",	SDLK_0,		KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-tilde",	SDLK_BACKQUOTE,	KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-bckquote",	SDLK_BACKQUOTE,	0,		textbox_insert_ascii },
	{ "i-uscore",	SDLK_MINUS,	KMOD_SHIFT,	textbox_insert_ascii },
	{ "i-minus",	SDLK_MINUS,	0,		textbox_insert_ascii },
	{ "i-space",	SDLK_SPACE,	0,		textbox_insert_ascii },
	
	/* Control characters */
	{ "c-bspace",	SDLK_BACKSPACE, 0,		textbox_key_backspace },
	{ "c-home",	SDLK_HOME,	0,		textbox_key_home },
	{ "c-end",	SDLK_END,	0,		textbox_key_end },
	{ "c-home",	SDLK_a,		KMOD_CTRL,	textbox_key_home },
	{ "c-end",	SDLK_e,		KMOD_CTRL,	textbox_key_end },
	{ "c-kill",	SDLK_k,		KMOD_CTRL,	textbox_key_kill },
	{ "c-left",	SDLK_LEFT,	0,		textbox_key_left },
	{ "c-right",	SDLK_RIGHT,	0,		textbox_key_right },

	/* Alphabetic characters */
	{ "a",		SDLK_a,		0,		textbox_insert_alpha },
	{ "b",		SDLK_b,		0,		textbox_insert_alpha },
	{ "c",		SDLK_c,		0,		textbox_insert_alpha },
	{ "d",		SDLK_d,		0,		textbox_insert_alpha },
	{ "e",		SDLK_e,		0,		textbox_insert_alpha },
	{ "f",		SDLK_f,		0,		textbox_insert_alpha },
	{ "g",		SDLK_g,		0,		textbox_insert_alpha },
	{ "h",		SDLK_h,		0,		textbox_insert_alpha },
	{ "i",		SDLK_i,		0,		textbox_insert_alpha },
	{ "j",		SDLK_j,		0,		textbox_insert_alpha },
	{ "k",		SDLK_k,		0,		textbox_insert_alpha },
	{ "l",		SDLK_l,		0,		textbox_insert_alpha },
	{ "m",		SDLK_m,		0,		textbox_insert_alpha },
	{ "n",		SDLK_n,		0,		textbox_insert_alpha },
	{ "o",		SDLK_o,		0,		textbox_insert_alpha },
	{ "p",		SDLK_p,		0,		textbox_insert_alpha },
	{ "q",		SDLK_q,		0,		textbox_insert_alpha },
	{ "r",		SDLK_r,		0,		textbox_insert_alpha },
	{ "s",		SDLK_s,		0,		textbox_insert_alpha },
	{ "t",		SDLK_t,		0,		textbox_insert_alpha },
	{ "u",		SDLK_u,		0,		textbox_insert_alpha },
	{ "v",		SDLK_v,		0,		textbox_insert_alpha },
	{ "w",		SDLK_w,		0,		textbox_insert_alpha },
	{ "x",		SDLK_x,		0,		textbox_insert_alpha },
	{ "y",		SDLK_y,		0,		textbox_insert_alpha },
	{ "z",		SDLK_z,		0,		textbox_insert_alpha },

	/* Digits */
	{ "one",	SDLK_1,		0,		textbox_insert_digit },
	{ "two",	SDLK_2,		0,		textbox_insert_digit },
	{ "three",	SDLK_3,		0,		textbox_insert_digit },
	{ "four",	SDLK_4,		0,		textbox_insert_digit },
	{ "five",	SDLK_5,		0,		textbox_insert_digit },
	{ "six",	SDLK_6,		0,		textbox_insert_digit },
	{ "seven",	SDLK_7,		0,		textbox_insert_digit },
	{ "eight",	SDLK_8,		0,		textbox_insert_digit },
	{ "nine",	SDLK_9,		0,		textbox_insert_digit },
	{ "ten",	SDLK_0,		0,		textbox_insert_digit },

	{ NULL,		SDLK_LAST,	0,		NULL }
};

static char *
textbox_insert_char(struct textbox *tbox, char c)
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
textbox_insert_alpha(struct textbox *tbox, SDL_Event *ev)
{
	char *c;

	c = textbox_insert_char(tbox, (char)ev->key.keysym.sym);
	if ((ev->key.keysym.mod & KMOD_SHIFT) && isalpha((int)*c)) {
		(int)*c = toupper((int)*c);
	}
}

static void
textbox_insert_digit(struct textbox *tbox, SDL_Event *ev)
{
	textbox_insert_char(tbox, ev->key.keysym.sym);
}

static void
textbox_insert_ascii(struct textbox *tbox, SDL_Event *ev)
{
	if (ev->key.keysym.mod & KMOD_SHIFT) {
		switch (ev->key.keysym.sym) {
		case SDLK_BACKQUOTE:
			textbox_insert_char(tbox, '~');
			break;
		case SDLK_MINUS:
			textbox_insert_char(tbox, '_');
			break;
		case SDLK_1:
			textbox_insert_char(tbox, '!');
			break;
		case SDLK_2:
			textbox_insert_char(tbox, '@');
			break;
		case SDLK_3:
			textbox_insert_char(tbox, '#');
			break;
		case SDLK_4:
			textbox_insert_char(tbox, '$');
			break;
		case SDLK_5:
			textbox_insert_char(tbox, '%');
			break;
		case SDLK_6:
			textbox_insert_char(tbox, '^');
			break;
		case SDLK_7:
			textbox_insert_char(tbox, '&');
			break;
		case SDLK_8:
			textbox_insert_char(tbox, '*');
			break;
		case SDLK_9:
			textbox_insert_char(tbox, '(');
			break;
		case SDLK_0:
			textbox_insert_char(tbox, ')');
			break;
		default:
			break;
		}
	} else {
		textbox_insert_char(tbox, (char)ev->key.keysym.sym);
	}
}

static void
textbox_key_backspace(struct textbox *tbox, SDL_Event *ev)
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
textbox_key_home(struct textbox *tbox, SDL_Event *ev)
{
	tbox->textpos = 0;
}

static void
textbox_key_end(struct textbox *tbox, SDL_Event *ev)
{
	tbox->textpos = strlen(tbox->text);
}

static void
textbox_key_kill(struct textbox *tbox, SDL_Event *ev)
{
	tbox->text[0] = '\0';
	tbox->textpos = 0;
}

static void
textbox_key_left(struct textbox *tbox, SDL_Event *ev)
{
	if (--tbox->textpos < 1) {
		tbox->textpos = 0;
	}
}

static void
textbox_key_right(struct textbox *tbox, SDL_Event *ev)
{
	if (tbox->textpos < strlen(tbox->text)) {
		tbox->textpos++;
	}
}


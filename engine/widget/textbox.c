/*	$Csoft: textbox.c,v 1.16 2002/05/22 02:03:01 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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
#include <engine/version.h>

#include "text.h"
#include "window.h"
#include "widget.h"
#include "textbox.h"
#include "keycodes.h"

extern TTF_Font *font;		/* text */
static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */

static const struct widget_ops textbox_ops = {
	{
		textbox_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL,		/* onattach */
		NULL,		/* ondetach */
		NULL,		/* attach */
		NULL		/* detach */
	},
	textbox_draw,
	textbox_event
};

struct textbox *
textbox_new(struct region *reg, int flags, int rw, int rh)
{
	struct textbox *textbox;

	textbox = emalloc(sizeof(struct textbox));
	textbox_init(textbox, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, textbox);
	pthread_mutex_unlock(&reg->win->lock);

	return (textbox);
}

void
textbox_init(struct textbox *b, int flags, int rw, int rh)
{
	int h;
	SDL_Surface *s;

	s = TTF_RenderText_Solid(font, "ABCD1234", white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	h = (rh > s->h) ? rh : s->h;

	widget_init(&b->wid, "textbox", "widget", &textbox_ops, rw, h);
	b->flags = flags;
	b->flags |= TEXTBOX_CURSOR;
	b->text = strdup("foo");
	b->textpos = -1;
	b->typed = NULL;
	b->xmargin = 4;
	b->ymargin = 2;
}

void
textbox_destroy(void *ob)
{
	struct textbox *b = ob;

	free(b->text);
}

void
textbox_draw(void *p)
{
	struct textbox *tbox = p;
	int i, j, x, y, tw;
	int bx, by;
	Uint32 col;

	/* Background */
	SDL_LockSurface(WIDGET_SURFACE(tbox));
	for (by = 0; by < WIDGET(tbox)->h; by++) {
		for (bx = 0; bx < WIDGET(tbox)->w; bx++) {
			col = SDL_MapRGB(WIDGET_SURFACE(tbox)->format,
			    255, 255, 255);
			WIDGET_PUT_ALPHAPIXEL(tbox, bx, by, col, 40);
		}
	}
	SDL_UnlockSurface(WIDGET_SURFACE(tbox));

	tw = strlen(tbox->text);
	if (tbox->textpos < 0) {
		tbox->textpos = tw;
	}
	dprintf("textpos = %d/%d\n", tbox->textpos, tw);

	for (i = 0, x = tbox->xmargin, y = tbox->ymargin;
	     (i < tw + 1) && (x < (WIDGET(tbox)->w - tbox->xmargin));
	     i++) {
	     	dprintf("render '%c'\n", tbox->text[i]);

		if (i == tbox->textpos && tbox->flags & TEXTBOX_CURSOR) {
			x++;
			SDL_LockSurface(WIDGET_SURFACE(tbox));
			for (j = 2; j < WIDGET(tbox)->h - 4; j++) {
				Uint32 curs_col;

				curs_col = SDL_MapRGB(
				    WIDGET_SURFACE(tbox)->format, 0, 0, 0);

				WIDGET_PUT_ALPHAPIXEL(tbox, x, y+j,
				    curs_col, 100);
				WIDGET_PUT_ALPHAPIXEL(tbox, x+1, y+j,
				    curs_col, 100);
			}
			SDL_UnlockSurface(WIDGET_SURFACE(tbox));
			x++;
		}
		if (i < tw && tbox->text[i] != '\0') {
			SDL_Surface *text_s;
			char str;

			str = tbox->text[i];

			/* Character */
			/* XXX cache! */
			text_s = TTF_RenderText_Solid(font, &str, white);
			if (text_s == NULL) {
				fatal("TTF_RenderTextSolid: %s\n",
				    SDL_GetError());
			}
			WIDGET_DRAW(tbox, text_s, x, y);
			x += text_s->w;
			SDL_FreeSurface(text_s);
		}
	}
}

void
textbox_event(void *p, SDL_Event *ev, int flags)
{
	struct textbox *tbox = p;
	int textlen, i;

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		WIDGET(tbox)->win->focus = WIDGET(tbox);
		WIDGET(tbox)->win->redraw++;
		/* XXX position cursor.. */
		break;
	case SDL_KEYDOWN:
		textlen = strlen(tbox->text);
	
		for (i = 0; textbox_keycodes[i].key != SDLK_LAST; i++) {
			const struct keycode *kcode;

			kcode = &textbox_keycodes[i];
			if (kcode->key != ev->key.keysym.sym ||
			    kcode->callback == NULL) {
				continue;
			}
			if ((int)kcode->modmask == 0 ||
			    (int)ev->key.keysym.mod & (int)kcode->modmask) {
				textbox_keycodes[i].callback(tbox, ev);
				WIDGET(tbox)->win->redraw++;
				return;
			}
		}
		break;
	}
}


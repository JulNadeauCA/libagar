/*	$Csoft: textbox.c,v 1.3 2002/05/25 08:47:51 vedge Exp $	*/

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
		textbox_onattach, /* onattach */
		textbox_ondetach, /* ondetach */
		NULL,		/* attach */
		NULL		/* detach */
	},
	textbox_draw,
	textbox_event
};

struct textbox *
textbox_new(struct region *reg, char *label, int flags, int rw)
{
	struct textbox *textbox;

	textbox = emalloc(sizeof(struct textbox));
	textbox_init(textbox, label, flags, rw);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, textbox);
	pthread_mutex_unlock(&reg->win->lock);

	return (textbox);
}

void
textbox_init(struct textbox *b, char *label, int flags, int rw)
{
	int h;
	SDL_Surface *s;

	s = TTF_RenderText_Solid(font, "ABCD1234", white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}
	
	b->xmargin = 4;
	b->ymargin = 3;

	h = s->h + b->ymargin*4;

	widget_init(&b->wid, "textbox", "widget", &textbox_ops, rw, h);
	b->flags = flags;
	b->flags |= TEXTBOX_CURSOR;
	b->text = strdup("");
	b->label = label != NULL ? strdup(label) : NULL;
	b->textpos = -1;
	b->typed = NULL;
}

void
textbox_onattach(void *parent, void *child)
{
	if (SDL_EnableKeyRepeat(250, 30) != 0) {	/* XXX pref */
		warning("SDL_EnableKeyRepeat: %s\n", SDL_GetError());
	}
}

void
textbox_ondetach(void *parent, void *child)
{
	if (SDL_EnableKeyRepeat(0, 0) != 0) {		/* XXX pref */
		fatal("SDL_EnableKeyRepeat: %s\n", SDL_GetError());
	}
}

void
textbox_destroy(void *ob)
{
	struct textbox *b = ob;

	free(b->text);
	free(b->label);
}

void
textbox_draw(void *p)
{
	struct textbox *tbox = p;
	int i, j, x, y, tw;
	int bx, by;
	Uint32 lcol, rcol, bcol, curscol;

	if (WIDGET_FOCUSED(tbox)) {
		lcol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 50, 50, 50);
		rcol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 200, 200, 200);
		bcol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 100, 100, 100);
		curscol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 0, 0, 0);
	} else {
		lcol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 200, 200, 200);
		rcol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 50, 50, 50);
		bcol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 80, 80, 80);
		curscol = bcol;
	}

	/* Background */
	SDL_LockSurface(WIDGET_SURFACE(tbox));
	for (by = 0; by < WIDGET(tbox)->h; by++) {
		for (bx = 0; bx < WIDGET(tbox)->w; bx++) {
			if (by < 1)
				WIDGET_PUT_PIXEL(tbox, bx, by, lcol);
			else if (bx < 1)
				WIDGET_PUT_PIXEL(tbox, bx, by, lcol);
			else if (by >= WIDGET(tbox)->h - 1)
				WIDGET_PUT_PIXEL(tbox, bx, by, rcol);
			else if (bx >= WIDGET(tbox)->w - 1)
				WIDGET_PUT_PIXEL(tbox, bx, by, rcol);
			else
				WIDGET_PUT_PIXEL(tbox, bx, by, bcol);
		}
	}
	SDL_UnlockSurface(WIDGET_SURFACE(tbox));

	tw = strlen(tbox->text);
	if (tbox->textpos < 0) {
		tbox->textpos = tw;
	}
#if 0
	dprintf("textpos = %d/%d\n", tbox->textpos, tw);
#endif

	for (i = 0, x = tbox->xmargin, y = tbox->ymargin; (i < tw + 1); i++) {
		if (x > (WIDGET(tbox)->w - tbox->xmargin*2)) {
			/* scroll .. */
			return;
		}
		if (i == tbox->textpos && tbox->flags & TEXTBOX_CURSOR) {
			SDL_LockSurface(WIDGET_SURFACE(tbox));
			for (j = 2; j < WIDGET(tbox)->h - 4; j++) {
				WIDGET_PUT_PIXEL(tbox, x, y+j, curscol);
				WIDGET_PUT_PIXEL(tbox, x+1, y+j, curscol);
			}
			SDL_UnlockSurface(WIDGET_SURFACE(tbox));
		}
		if (i < tw && tbox->text[i] != '\0') {
			SDL_Surface *text_s;
			char str[2];

			str[0] = tbox->text[i];
			str[1] = '\0';
#if 0
	     		dprintf("render '%s'\n", str);
#endif

			/* Character */
			/* XXX cache! */
			text_s = TTF_RenderText_Solid(font, str, white);
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
			if (kcode->modmask == 0 ||
			    (int)ev->key.keysym.mod & kcode->modmask) {
				textbox_keycodes[i].callback(tbox, ev);
				textbox_draw(tbox);
				SDL_UpdateRect(WIDGET_SURFACE(tbox),
				    WIDGET(tbox)->x + WIDGET(tbox)->win->x,
				    WIDGET(tbox)->y + WIDGET(tbox)->win->y,
				    WIDGET(tbox)->w, WIDGET(tbox)->h);
				return;
			}
		}
		break;
	}
}


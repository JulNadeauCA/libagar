/*	$Csoft: textbox.c,v 1.7 2002/05/26 06:07:23 vedge Exp $	*/

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

#include "primitive.h"
#include "text.h"
#include "window.h"
#include "widget.h"
#include "textbox.h"
#include "keycodes.h"

extern TTF_Font *font;		/* XXX pref */
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
textbox_init(struct textbox *tbox, char *label, int flags, int rw)
{
	SDL_Surface *s;

	s = TTF_RenderText_Solid(font, "ABCD1234", white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}
	tbox->xmargin = 4;
	tbox->ymargin = 3;

	widget_init(&tbox->wid, "textbox", "widget", &textbox_ops, rw, -1);
	WIDGET(tbox)->h = (s->h * 2) + tbox->ymargin;

	SDL_FreeSurface(s);

	tbox->flags = flags;
	tbox->flags |= TEXTBOX_CURSOR;
	tbox->text = strdup("");
	tbox->label = label != NULL ? strdup(label) : NULL;
	tbox->textpos = -1;
	tbox->textoffs = 0;

	tbox->typed = NULL;
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
	int i, cursdrawn, j, x, y, tw;
	Uint32 curscol;
	SDL_Surface *box_s;

	curscol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 0, 0, 0);

	box_s = primitive_box(WIDGET(tbox)->w, WIDGET(tbox)->h,
	    WIDGET_FOCUSED(tbox) ? -1 : 1);
	WIDGET_DRAW(tbox, box_s, 0, 0);

	tw = strlen(tbox->text);
	if (tbox->textpos < 0) {
		tbox->textpos = tw;
	}

	cursdrawn = 0;

	for (i = tbox->textoffs, x = tbox->xmargin, y = tbox->ymargin;
	    (i < tw+1); i++) {
		if (x >= WIDGET(tbox)->w) {
			if (tbox->textpos >= tw-4) {
				tbox->textoffs++;	/* Scroll */
				cursdrawn++;
			}
			goto after;
		}
		if (i == tbox->textpos && tbox->flags & TEXTBOX_CURSOR &&
		    WIDGET_FOCUSED(tbox)) {
			SDL_LockSurface(WIDGET_SURFACE(tbox));
			for (j = 2; j < WIDGET(tbox)->h - 4; j++) {
				WIDGET_PUT_PIXEL(tbox, x, y+j, curscol);
				WIDGET_PUT_PIXEL(tbox, x+1, y+j, curscol);
			}
			SDL_UnlockSurface(WIDGET_SURFACE(tbox));
			cursdrawn++;
		}
		if (i < tw && tbox->text[i] != '\0') {
			SDL_Surface *text_s;
			char c, str[2];

			c = tbox->text[i];

			if (c >= (char)KEYCODES_CACHE_START &&
			    c <= (char)KEYCODES_CACHE_END &&
			    keycodes_cache[(int)c-(int)KEYCODES_CACHE_START]
			    != NULL) {
				text_s = keycodes_cache[(int)c -
				    (int)KEYCODES_CACHE_START];
				WIDGET_DRAW(tbox, text_s, x, y);
				x += text_s->w;
			} else {
				str[0] = (char)c;
				str[1] = '\0';
				text_s = TTF_RenderText_Solid(font, str, white);
				if (text_s == NULL) {
					warning("TTF_RenderTextSolid: %s\n",
					    SDL_GetError());
				} else {
					WIDGET_DRAW(tbox, text_s, x, y);
					x += text_s->w;
					SDL_FreeSurface(text_s);
				}
			}
		}
	}
after:
	if (!cursdrawn) {
		tbox->textoffs = tbox->textpos;
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
	
		for (i = 0; keycodes[i].key != SDLK_LAST; i++) {
			const struct keycode *kcode;

			kcode = &keycodes[i];
			if (kcode->key != ev->key.keysym.sym ||
			    kcode->callback == NULL) {
				continue;
			}
			if (kcode->modmask == 0 ||
			    (int)ev->key.keysym.mod & kcode->modmask) {
				keycodes[i].callback(tbox, ev, keycodes[i].arg);
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


/*	$Csoft: textbox.c,v 1.11 2002/06/03 02:30:59 vedge Exp $	*/

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
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
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
		NULL,		/* attach */
		NULL		/* detach */
	},
	textbox_draw
};

static void	textbox_event(int, union evarg *);

struct textbox *
textbox_new(struct region *reg, const char *label, int flags, int rw)
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
textbox_init(struct textbox *tbox, const char *label, int flags, int rw)
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
	tbox->label_s = TTF_RenderText_Solid(font, label, white);
	if (tbox->label_s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", TTF_GetError());
	}
	tbox->textpos = -1;
	tbox->textoffs = 0;

	event_new(tbox, "shown", 0, textbox_shown, NULL);
	event_new(tbox, "hidden", 0, textbox_hidden, NULL);

	event_new(tbox, "window-mousebuttonup", 0,
	    textbox_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(tbox, "window-mousebuttondown", 0,
	    textbox_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(tbox, "window-keyup", 0,
	    textbox_event, "%i", WINDOW_KEYUP);
	event_new(tbox, "window-keydown", 0,
	    textbox_event, "%i", WINDOW_KEYDOWN);
}

void
textbox_shown(int argc, union evarg *argv)
{
	if (SDL_EnableKeyRepeat(250, 30) != 0) {	/* XXX pref */
		warning("SDL_EnableKeyRepeat: %s\n", SDL_GetError());
	}
}

void
textbox_hidden(int argc, union evarg *argv)
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
	SDL_Surface *label_s = tbox->label_s;

	curscol = SDL_MapRGB(WIDGET_SURFACE(tbox)->format, 0, 0, 0);
	
	x = label_s->w;
	y = tbox->ymargin;

	WIDGET_DRAW(tbox, label_s, 0, y/2);
	
	box_s = primitive_box(tbox,
	    WIDGET(tbox)->w - (tbox->xmargin*2) - label_s->w, WIDGET(tbox)->h,
	    WIDGET_FOCUSED(tbox) ? -1 : 1);
	WIDGET_DRAW(tbox, box_s, x, 0);
	x += tbox->xmargin;

	tw = strlen(tbox->text);
	if (tbox->textpos < 0) {
		tbox->textpos = tw;
	}

	cursdrawn = 0;

	for (i = tbox->textoffs; i < (tw + 1); i++) {
		if (x >= WIDGET(tbox)->w) {
			if (tbox->textpos >= tw-4) {
				tbox->textoffs++;	/* Scroll */
				cursdrawn++;
			}
			goto after;
		}
		if (i == tbox->textpos && tbox->flags & TEXTBOX_CURSOR &&
		    WIDGET_FOCUSED(tbox) &&
		    x < WIDGET(tbox)->w - (tbox->xmargin*4)) {
			SDL_LockSurface(WIDGET_SURFACE(tbox));
			for (j = 2; j < WIDGET(tbox)->h - 4; j++) {
				WIDGET_PUT_PIXEL(tbox, x, y+j, curscol);
				WIDGET_PUT_PIXEL(tbox, x+1, y+j, curscol);
			}
			SDL_UnlockSurface(WIDGET_SURFACE(tbox));
			cursdrawn++;
		}
		if (i < tw && tbox->text[i] != '\0' &&
		    x < WIDGET(tbox)->w - (tbox->xmargin*4)) {
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
#if 0
	if (!cursdrawn) {
		tbox->textoffs = tbox->textpos / 2;
	}
#endif
}

static void
textbox_event(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int keysym, keymod;
	int textlen, i;

	OBJECT_ASSERT(argv[0].p, "widget");

	switch (argv[1].i) {
	case WINDOW_MOUSEBUTTONDOWN:
		WIDGET(tbox)->win->focus = WIDGET(tbox);
		WIDGET(tbox)->win->redraw++;
		/* XXX position cursor.. */
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		keymod = argv[3].i;
	
		textlen = strlen(tbox->text);

		if (keysym == SDLK_RETURN) {
			event_post(tbox, "textbox-return", "%s", tbox->text);
			return;
		}

		for (i = 0; keycodes[i].key != SDLK_LAST; i++) {
			const struct keycode *kcode;

			kcode = &keycodes[i];
			if (kcode->key != (SDLKey)keysym ||
			    kcode->callback == NULL) {
				continue;
			}
			if (kcode->modmask == 0 || keymod & kcode->modmask) {
				keycodes[i].callback(tbox, (SDLKey)keysym,
				    keymod, keycodes[i].arg);
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

/* Window must be locked */
void
textbox_printf(struct textbox *tbox, char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
	va_end(args);

	free(tbox->text);
	tbox->text = buf;

	WIDGET(tbox)->win->redraw++;
	tbox->textpos = 0;
	tbox->textoffs = 0;
}


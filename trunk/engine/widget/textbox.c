/*	$Csoft: textbox.c,v 1.23 2002/09/05 03:51:32 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
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
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/version.h>

#include <engine/compat/vasprintf.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "textbox.h"
#include "keycodes.h"

extern TTF_Font *font;		/* XXX pref */

static const struct widget_ops textbox_ops = {
	{
		textbox_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	textbox_draw,
	NULL		/* animate */
};

enum {
	FRAME_COLOR,
	TEXT_COLOR,
	CURSOR_COLOR1,
	CURSOR_COLOR2
};

static void	textbox_mouse(int, union evarg *);
static void	textbox_key(int, union evarg *);

struct textbox *
textbox_new(struct region *reg, const char *label, int flags, int rw, int rh)
{
	struct textbox *textbox;

	textbox = emalloc(sizeof(struct textbox));
	textbox_init(textbox, label, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, textbox);
	pthread_mutex_unlock(&reg->win->lock);

	return (textbox);
}

void
textbox_init(struct textbox *tbox, const char *label, int flags, int rw, int rh)
{
	SDL_Surface *s;

	s = text_render(NULL, -1, 0, "ABCD1234");
	tbox->xmargin = 4;
	tbox->ymargin = 3;

	widget_init(&tbox->wid, "textbox", "widget", &textbox_ops, rw, rh);
	WIDGET(tbox)->h = (s->h * 2) + (tbox->ymargin * 2);

	widget_map_color(tbox, FRAME_COLOR, "textbox-frame", 100, 100, 100);
	widget_map_color(tbox, TEXT_COLOR, "textbox-text", 250, 250, 250);
	widget_map_color(tbox, CURSOR_COLOR1, "textbox-cursor1", 50, 50, 50);
	widget_map_color(tbox, CURSOR_COLOR2, "textbox-cursor2", 0, 0, 0);

	SDL_FreeSurface(s);

	tbox->flags = flags;
	tbox->text = strdup("");
	tbox->label = label != NULL ? strdup(label) : NULL;
	tbox->label_s = text_render(NULL, -1,
	    WIDGET_COLOR(tbox, TEXT_COLOR), (char *)label);
	tbox->textpos = -1;
	tbox->textoffs = 0;
	tbox->newx = -1;

	event_new(tbox, "widget-shown", 0, textbox_shown, NULL);
	event_new(tbox, "widget-hidden", 0, textbox_hidden, NULL);
	event_new(tbox, "window-keydown", 0, textbox_key, NULL);
	event_new(tbox, "window-mousebuttondown", 0,
	    textbox_mouse, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(tbox, "window-mousemotion", 0,
	    textbox_mouse, "%i", WINDOW_MOUSEMOTION);
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
	int i, j, x, y, tw, lx;
	SDL_Surface *label_s = tbox->label_s;

	x = label_s->w;
	y = tbox->ymargin;

	/* Label */
	WIDGET_DRAW(tbox, label_s, 0, y/2);

	/* Frame */
	if (WIDGET_FOCUSED(tbox)) {
		x++;
		y++;
	}
	primitives.box(tbox, x, 0,
	    WIDGET(tbox)->w - tbox->xmargin*2 - label_s->w,
	    label_s->h + tbox->ymargin*2,
	    WIDGET_FOCUSED(tbox) ? -1 : 1,
	    WIDGET_COLOR(tbox, FRAME_COLOR));

	/*
	 * Text
	 */
	x += tbox->xmargin;

	tw = strlen(tbox->text);
	if (tbox->textpos < 0) {
		tbox->textpos = tw;
	}

	if (tbox->newx >= 0 && tbox->newx <= tbox->label_s->w) {
		dprintf("begin\n");
		tbox->textpos = tbox->textoffs;
		tbox->newx = -1;
	}

	for (i = tbox->textoffs, lx = -1; i < (tw + 1); i++) {
		if (x >= WIDGET(tbox)->w) {
			if (tbox->textpos >= tw-4) {
				tbox->textoffs++;	/* Scroll */
			}
			return;
		}

		/* Effect mouse cursor moves. */
		if (tbox->newx >= 0) {
			dprintf("newx %d lx %d x %d\n", tbox->newx, lx, x);
			if (tbox->newx >= lx && tbox->newx < x) {
				tbox->textpos = i - 1;
				tbox->newx = -1;
			}
		}
		lx = x;

		/* Draw the characters. */
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
				if (c == '\n') {
					y += label_s->h + 2;
				} else {
					str[0] = (char)c;
					str[1] = '\0';
					text_s = text_render(NULL, -1,
					    WIDGET_COLOR(tbox, TEXT_COLOR),
					    str);
					WIDGET_DRAW(tbox, text_s, x, y);
					x += text_s->w;
					SDL_FreeSurface(text_s);
				}
			}
		}
	
		/* Draw the text cursor. */
		if (i == tbox->textpos && WIDGET_FOCUSED(tbox) &&
		    x < WIDGET(tbox)->w - tbox->xmargin*4) {
			SDL_LockSurface(WIDGET_SURFACE(tbox));
			for (j = 1; j < label_s->h; j++) {
				WIDGET_PUT_PIXEL(tbox, x, y+j,
				    WIDGET_COLOR(tbox, CURSOR_COLOR1));
				WIDGET_PUT_PIXEL(tbox, x+1, y+j,
				    WIDGET_COLOR(tbox, CURSOR_COLOR2));
			}
			SDL_UnlockSurface(WIDGET_SURFACE(tbox));
		}
	}

	if (tbox->newx >= 0) {
		tbox->textpos = i - 1;
		tbox->newx = -1;
	}
}
	
static void
textbox_key(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;
	int textlen, i;
	
	textlen = strlen(tbox->text);

	if (keysym == SDLK_RETURN) {
		event_post(tbox, "textbox-return", "%s", tbox->text);
		return;
	}

	for (i = 0; keycodes[i].key != SDLK_LAST; i++) {
		const struct keycode *kcode;

		kcode = &keycodes[i];
		if (kcode->key != (SDLKey)keysym || kcode->callback == NULL) {
			continue;
		}
		if (kcode->modmask == 0 || keymod & kcode->modmask) {
			keycodes[i].callback(tbox, (SDLKey)keysym,
			    keymod, keycodes[i].arg);
			return;
		}
	}
}

static void
textbox_mouse(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;

	OBJECT_ASSERT(argv[0].p, "widget");

	switch (argv[1].i) {
	case WINDOW_MOUSEBUTTONDOWN: {
			int button = argv[2].i, x = argv[3].i;
	
			WIDGET(tbox)->win->focus = WIDGET(tbox);
			WIDGET(tbox)->win->redraw++;
			tbox->newx = x;
			break;
		}
	case WINDOW_MOUSEMOTION: {
			int x = argv[2].i, y = argv[3].i;
			Uint8 ms;

			ms = SDL_GetMouseState(NULL, NULL);
			if (ms & SDL_BUTTON_LEFT && x > tbox->label_s->w) {
				tbox->newx = x;
			}
			break;
		}
	}
}

/* Window must be locked */
void
textbox_printf(struct textbox *tbox, const char *fmt, ...)
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


/*	$Csoft: textbox.c,v 1.32 2002/11/22 05:41:45 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/compat/vasprintf.h>

#include <engine/view.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "textbox.h"
#include "keycodes.h"

static const struct widget_ops textbox_ops = {
	{
		textbox_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	textbox_draw,
	NULL		/* update */
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

	region_attach(reg, textbox);

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
	tbox->label = label != NULL ? strdup(label) : NULL;
	tbox->label_s = text_render(NULL, -1,
	    WIDGET_COLOR(tbox, TEXT_COLOR), (char *)label);
	tbox->text.s = strdup("");
	tbox->text.pos = -1;
	tbox->text.offs = 0;
	pthread_mutex_init(&tbox->text.lock, NULL);
	tbox->newx = -1;

#if 0
	event_new(tbox, "widget-shown", textbox_shown, NULL);
	event_new(tbox, "widget-hidden", textbox_hidden, NULL);
#endif
	event_new(tbox, "window-keydown", textbox_key, NULL);
	event_new(tbox, "window-mousebuttondown",
	    textbox_mouse, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(tbox, "window-mousemotion",
	    textbox_mouse, "%i", WINDOW_MOUSEMOTION);
}

#if 0
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
#endif

void
textbox_destroy(void *ob)
{
	struct textbox *tb = ob;

	free(tb->label);
	free(tb->text.s);
	pthread_mutex_destroy(&tb->text.lock);

	widget_destroy(tb);
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

	pthread_mutex_lock(&tbox->text.lock);

	tw = strlen(tbox->text.s);
	if (tbox->text.pos < 0) {
		tbox->text.pos = tw;
	}

	if (tbox->newx >= 0 && tbox->newx <= tbox->label_s->w) {
		tbox->text.pos = tbox->text.offs;
		tbox->newx = -1;
	}

	for (i = tbox->text.offs, lx = -1; i < (tw + 1); i++) {
		if (x >= WIDGET(tbox)->w) {
			if (tbox->text.pos >= tw-4) {
				tbox->text.offs++;	/* Scroll */
			}
			goto out;
		}

		/* Effect mouse cursor moves. */
		if (tbox->newx >= 0) {
			if (tbox->newx >= lx && tbox->newx < x) {
				tbox->text.pos = i - 1;
				tbox->newx = -1;
			}
		}
		lx = x;

		/* Draw the characters. */
		if (i < tw && tbox->text.s[i] != '\0' &&
		    x < WIDGET(tbox)->w - (tbox->xmargin*4)) {
			SDL_Surface *text_s;
			char c, str[2];

			c = tbox->text.s[i];

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
		if (i == tbox->text.pos && WIDGET_FOCUSED(tbox) &&
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
		tbox->text.pos = i - 1;
		tbox->newx = -1;
	}
out:
	pthread_mutex_unlock(&tbox->text.lock);
}
	
static void
textbox_key(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;
	size_t textlen;
	int i, modified = 0;

	if (keysym == SDLK_RETURN) {
		event_post(tbox, "textbox-return", NULL);
		return;
	}

	pthread_mutex_lock(&tbox->text.lock);
	textlen = strlen(tbox->text.s);
	for (i = 0; keycodes[i].key != SDLK_LAST; i++) {
		const struct keycode *kcode;

		kcode = &keycodes[i];
		if (kcode->key != (SDLKey)keysym || kcode->callback == NULL) {
			continue;
		}
		if (kcode->modmask == 0 || keymod & kcode->modmask) {
			keycodes[i].callback(tbox, (SDLKey)keysym,
			    keymod, keycodes[i].arg);
			modified++;
			goto out;
		}
	}
out:
	pthread_mutex_unlock(&tbox->text.lock);
	
	if (modified) {
		event_post(tbox, "textbox-changed", NULL);
	}
}

static void
textbox_mouse(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;

	OBJECT_ASSERT(argv[0].p, "widget");

	switch (argv[1].i) {
	case WINDOW_MOUSEBUTTONDOWN: {
			int button = argv[2].i;
			int x = argv[3].i;
	
			WIDGET(tbox)->win->focus = WIDGET(tbox);
			tbox->newx = x;
		}
		break;
	case WINDOW_MOUSEMOTION: {
			int x = argv[2].i;
			int y = argv[3].i;
			Uint8 ms;

			ms = SDL_GetMouseState(NULL, NULL);
			if (ms & SDL_BUTTON_LEFT && x > tbox->label_s->w) {
				tbox->newx = x;
			}
		}
		break;
	}
}

void
textbox_printf(struct textbox *tbox, const char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
	va_end(args);

	pthread_mutex_lock(&tbox->text.lock);
	free(tbox->text.s);
	tbox->text.s = buf;
	tbox->text.pos = 0;
	tbox->text.offs = 0;
	pthread_mutex_unlock(&tbox->text.lock);
}

char *
textbox_string(struct textbox *tb)
{
	char *s;

	pthread_mutex_lock(&tb->text.lock);
	s = strdup(tb->text.s);
	pthread_mutex_unlock(&tb->text.lock);

	return (s);
}

int
textbox_int(struct textbox *tb)
{
	int i;

	pthread_mutex_lock(&tb->text.lock);
	i = atoi(tb->text.s);
	pthread_mutex_unlock(&tb->text.lock);

	return (i);
}


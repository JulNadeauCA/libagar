/*	$Csoft: textbox.c,v 1.53 2003/05/24 00:29:00 vedge Exp $	*/

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

#include <engine/compat/vasprintf.h>
#include <engine/compat/strlcpy.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/primitive.h>
#include <engine/widget/text.h>
#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/region.h>

#include <string.h>
#include <stdarg.h>
#include <strings.h>
#include <errno.h>

const struct widget_ops textbox_ops = {
	{
		NULL,		/* init */
		textbox_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	textbox_draw,
	NULL		/* update */
};

enum {
	FRAME_COLOR,
	FRAME_READONLY_COLOR,
	TEXT_COLOR,
	CURSOR_COLOR1,
	CURSOR_COLOR2
};

static void	textbox_scaled(int, union evarg *);
static void	textbox_mousemotion(int, union evarg *);
static void	textbox_mousebuttondown(int, union evarg *);
static void	textbox_key(int, union evarg *);

struct textbox *
textbox_new(struct region *reg, const char *label, int flags, int rw, int rh)
{
	struct textbox *textbox;

	textbox = Malloc(sizeof(struct textbox));
	textbox_init(textbox, label, flags, rw, rh);
	region_attach(reg, textbox);
	return (textbox);
}

void
textbox_init(struct textbox *tbox, const char *label, int flags, int rw, int rh)
{
	tbox->xpadding = 4;
	tbox->ypadding = 3;

	widget_init(&tbox->wid, "textbox", &textbox_ops, rw, rh);

	widget_map_color(tbox, FRAME_COLOR, "frame", 100, 100, 100);
	widget_map_color(tbox, FRAME_READONLY_COLOR, "frame-readonly",
	    40, 40, 40);
	widget_map_color(tbox, TEXT_COLOR, "text", 250, 250, 250);
	widget_map_color(tbox, CURSOR_COLOR1, "cursor1", 50, 50, 50);
	widget_map_color(tbox, CURSOR_COLOR2, "cursor2", 0, 0, 0);

	tbox->flags = flags;
	if (label != NULL) {
		tbox->label = text_render(NULL, -1,
		    WIDGET_COLOR(tbox, TEXT_COLOR), (char *)label);
	} else {
		tbox->label = NULL;
	}
	tbox->text.s = Strdup("");
	tbox->text.pos = -1;
	tbox->text.offs = 0;
	pthread_mutex_init(&tbox->text.lock, NULL);
	tbox->newx = -1;

	event_new(tbox, "window-keydown", textbox_key, NULL);
	event_new(tbox, "window-mousebuttondown", textbox_mousebuttondown,
	    NULL);
	event_new(tbox, "window-mousemotion", textbox_mousemotion, NULL);
	event_new(tbox, "widget-scaled", textbox_scaled, NULL);
}

void
textbox_destroy(void *ob)
{
	struct textbox *tbox = ob;

	if (tbox->label != NULL) {
		SDL_FreeSurface(tbox->label);
	}
	free(tbox->text.s);
	pthread_mutex_destroy(&tbox->text.lock);

	widget_destroy(tbox);
}

void
textbox_draw(void *p)
{
	struct textbox *tbox = p;
	int i, lx, tw, th;
	int x = 0;
	int y = tbox->ypadding;

	if (tbox->label != NULL) {
		widget_blit(tbox, tbox->label,
		    0,
		    WIDGET(tbox)->h/2 - tbox->label->h/2);
		x += tbox->label->w;
	}

	if (WIDGET_FOCUSED(tbox)) {
		x++;
		y++;
	}
	primitives.box(tbox, x, 0,
	    WIDGET(tbox)->w - x,
	    WIDGET(tbox)->h,
	    WIDGET_FOCUSED(tbox) ? -1 : 1,
	    tbox->flags & TEXTBOX_READONLY ?
	        WIDGET_COLOR(tbox, FRAME_READONLY_COLOR) :
		WIDGET_COLOR(tbox, FRAME_COLOR));

	pthread_mutex_lock(&tbox->text.lock);
	tw = (int)strlen(tbox->text.s);
	th = text_font_height(font);

	/* Default to the end of the string. */
	if (tbox->text.pos < 0)
		tbox->text.pos = tw;

	/* Move to the beginning of the string? */
	if (tbox->newx >= 0 && tbox->newx <= tbox->label->w) {
		tbox->newx = -1;
		tbox->text.pos = tbox->text.offs;
		dprintf("move to start (%d, offs=%d)\n", tbox->text.pos,
		    tbox->text.offs);
	}

	x += tbox->xpadding;
	for (i = tbox->text.offs, lx = -1;
	     i < tw+1;
	     i++) {
		/* Move to a position inside the string? */
		if (tbox->newx >= 0 &&
		    tbox->newx >= lx && tbox->newx < x) {
			tbox->newx = -1;
			tbox->text.pos = i;
			dprintf("move to %d\n", i);
		}
		lx = x;

		/* Draw the text cursor. */
		if (i == tbox->text.pos && WIDGET_FOCUSED(tbox)) {
			primitives.line(tbox,
			    x, y,
			    x, y + tbox->label->h - 2,
			    WIDGET_COLOR(tbox, CURSOR_COLOR1));
			primitives.line(tbox,
			    x+1, y,
			    x+1, y + tbox->label->h - 2,
			    WIDGET_COLOR(tbox, CURSOR_COLOR2));
		}

		/* Draw the characters. */
		if (i < tw && tbox->text.s[i] != '\0' &&
		    x < WIDGET(tbox)->w - (tbox->xpadding*4)) {
			SDL_Surface *glyph;
			char ch = tbox->text.s[i];

			if (ch == '\n') {
				y += tbox->label->h + 2;
			} else {
				char cs[2];

				cs[0] = ch;
				cs[1] = '\0';
				glyph = text_render(NULL, -1,
				    WIDGET_COLOR(tbox, TEXT_COLOR), cs);
				widget_blit(tbox, glyph, x, y);
				x += glyph->w;
				SDL_FreeSurface(glyph);
			}
		}
	}

	/* Move to the visible end of the string? */
	if (tbox->newx >= 0) {
		tbox->newx = -1;
		tbox->text.pos = i-1;
		dprintf("move to end\n");
	}
	pthread_mutex_unlock(&tbox->text.lock);
}

static void
textbox_scaled(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	
	if (WIDGET(tbox)->rh == -1)
		WIDGET(tbox)->h = tbox->label->h + tbox->ypadding*2;
}

static void
textbox_key(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;
	int i, modified = 0;

	if (tbox->flags & TEXTBOX_READONLY) {
		return;
	}

	if (keysym == SDLK_RETURN) {
		event_post(tbox, "textbox-return", NULL);
		return;
	}

	pthread_mutex_lock(&tbox->text.lock);
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
textbox_mousemotion(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int x = argv[1].i;
	Uint8 ms;

	ms = SDL_GetMouseState(NULL, NULL);
	if (ms & SDL_BUTTON_LEFT) {
		if (x > tbox->label->w) {
			tbox->newx = x;
		} else if (x <= tbox->label->w) {
			tbox->newx = 0;
		}
	}
}

static void
textbox_mousebuttondown(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int x = argv[2].i;
	
	WIDGET(tbox)->win->focus = WIDGET(tbox);
	tbox->newx = x;
}

void
textbox_printf(struct textbox *tbox, const char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start(args, fmt);
	Vasprintf(&buf, fmt, args);
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
	s = Strdup(tb->text.s);
	pthread_mutex_unlock(&tb->text.lock);

	return (s);
}

size_t
textbox_copy_string(struct textbox *tb, char *dst, size_t dst_size)
{
	size_t rv;

	pthread_mutex_lock(&tb->text.lock);
	rv = strlcpy(dst, tb->text.s, dst_size);
	pthread_mutex_unlock(&tb->text.lock);

	return (rv);
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


/*	$Csoft: textbox.c,v 1.61 2003/06/06 09:03:54 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/primitive.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

const struct widget_ops textbox_ops = {
	{
		NULL,		/* init */
		textbox_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	textbox_draw,
	textbox_scale
};

enum {
	MOUSE_SCROLL_INCR =	4,
	KBD_SCROLL_INCR =	4,
};

enum {
	READWRITE_COLOR,
	READONLY_COLOR,
	TEXT_COLOR,
	CURSOR_COLOR1,
	CURSOR_COLOR2
};

static void	textbox_mousemotion(int, union evarg *);
static void	textbox_mousebuttondown(int, union evarg *);
static void	textbox_key(int, union evarg *);

struct textbox *
textbox_new(void *parent, const char *label)
{
	struct textbox *textbox;

	textbox = Malloc(sizeof(struct textbox));
	textbox_init(textbox, label);
	object_attach(parent, textbox);
	return (textbox);
}

void
textbox_init(struct textbox *tbox, const char *label)
{
	widget_init(tbox, "textbox", &textbox_ops, WIDGET_WFILL);
	widget_bind(tbox, "string", WIDGET_STRING, NULL, tbox->string,
	    sizeof(tbox->string));

	widget_map_color(tbox, READWRITE_COLOR, "edition", 100, 100, 100, 255);
	widget_map_color(tbox, READONLY_COLOR, "read-only", 40, 40, 40, 255);
	widget_map_color(tbox, TEXT_COLOR, "text", 250, 250, 250, 255);
	widget_map_color(tbox, CURSOR_COLOR1, "cursor1", 50, 50, 50, 255);
	widget_map_color(tbox, CURSOR_COLOR2, "cursor2", 0, 0, 0, 255);

	tbox->string[0] = '\0';
	tbox->xpadding = 4;
	tbox->ypadding = 3;
	tbox->writeable = 1;
	if (label != NULL) {
		tbox->label = text_render(NULL, -1,
		    WIDGET_COLOR(tbox, TEXT_COLOR), (char *)label);
	} else {
		tbox->label = NULL;
	}
	tbox->pos = -1;
	tbox->offs = 0;
	pthread_mutex_init(&tbox->lock, &recursive_mutexattr);
	tbox->newx = -1;

	event_new(tbox, "window-keydown", textbox_key, NULL);
	event_new(tbox, "window-mousebuttondown", textbox_mousebuttondown,
	    NULL);
	event_new(tbox, "window-mousemotion", textbox_mousemotion, NULL);
}

void
textbox_destroy(void *ob)
{
	struct textbox *tbox = ob;

	if (tbox->label != NULL)
		SDL_FreeSurface(tbox->label);

	pthread_mutex_destroy(&tbox->lock);
	widget_destroy(tbox);
}

void
textbox_draw(void *p)
{
	struct textbox *tbox = p;
	struct widget_binding *stringb;
	int i, lx, cursdrawn = 0;
	int x = 0, y;
	size_t tlen;
	char *s;

	if (tbox->label != NULL) {
		widget_blit(tbox, tbox->label, 0,
		    WIDGET(tbox)->h/2 - tbox->label->h/2);
		x = tbox->label->w;
	}

	pthread_mutex_lock(&tbox->lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	tlen = strlen(s);

	/* Default to the end of the string. */
	if (tbox->pos < 0)
		tbox->pos = tlen;

	/* Move to the beginning of the string? */
	if (tbox->newx >= 0 && tbox->newx <= tbox->label->w) {
		if (tbox->newx < tbox->label->w - tbox->xpadding) {
			if ((tbox->offs -= MOUSE_SCROLL_INCR) < 0)
				tbox->offs = 0;
		}
		tbox->pos = tbox->offs;
		tbox->newx = -1;
	}
drawtext:
	x = tbox->label->w + tbox->xpadding;
	y = tbox->ypadding;

	primitives.box(tbox,
	    x,
	    0,
	    WIDGET(tbox)->w - x - 1,
	    WIDGET(tbox)->h,
	    widget_holds_focus(tbox) ? -1 : 1,
	    tbox->writeable ? READWRITE_COLOR : READONLY_COLOR);

	x += tbox->xpadding;
	if (widget_holds_focus(tbox)) {
		x++;
		y++;
	}
	for (i = tbox->offs, lx = -1;
	     i <= tlen;
	     i++) {
		SDL_Surface *glyph;
		char ch = s[i];

		/* Move to a position inside the string? */
		if (tbox->newx >= 0 &&
		    tbox->newx >= lx && tbox->newx < x) {
			tbox->newx = -1;
			tbox->pos = i;
		}
		lx = x;

		if (i == tbox->pos && widget_holds_focus(tbox)) {
#if 1
			primitives.line(tbox,
			    x,
			    y,
			    x,
			    y + tbox->label->h - 2,
			    CURSOR_COLOR1);
			primitives.line(tbox,
			    x + 1,
			    y,
			    x + 1,
			    y + tbox->label->h - 2,
			    CURSOR_COLOR2);
#else
			primitives.line2(tbox,
			    x,
			    y,
			    x,
			    y + tbox->label->h - 2,
			    CURSOR_COLOR1);
#endif
			cursdrawn++;
		}
		
		if (x > WIDGET(tbox)->w - tbox->xpadding*4)
			break;

		if (ch == '\n') {
			y += tbox->label->h + 2;
		} else {
			char cs[2];

			/* XXX use text_render_glyph when it is fixed */
			cs[0] = ch;
			cs[1] = '\0';
			glyph = text_render(NULL, -1,
			    WIDGET_COLOR(tbox, TEXT_COLOR), cs);
			widget_blit(tbox, glyph, x, y);
			x += glyph->w;
			SDL_FreeSurface(glyph);
		}
	}
	if (widget_holds_focus(tbox) && !cursdrawn) {
		if (tbox->pos > i)
			tbox->offs += KBD_SCROLL_INCR;
		goto drawtext;
	}

	/* Move beyond the visible end of the string? */
	if (tbox->newx >= 0) {
		tbox->newx = -1;
		tbox->pos = i-1;
		if (i < tlen) {
			tbox->offs += MOUSE_SCROLL_INCR;
			tbox->pos++;
		}
	}
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->lock);
}

void
textbox_scale(void *p, int rw, int rh)
{
	struct textbox *tbox = p;

	if (rw == -1 && rh == -1) {
		/* XXX more sensible default */
		WIDGET(tbox)->w = tbox->label->w + tbox->xpadding*2 + 50;
		WIDGET(tbox)->h = tbox->label->h + tbox->ypadding*2;
	}
}

static void
textbox_key(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;
	int i;

	if (!tbox->writeable)
		return;

	if (keysym == SDLK_RETURN) {
		event_post(tbox, "textbox-return", NULL);
		return;
	}

	pthread_mutex_lock(&tbox->lock);
	for (i = 0; keycodes[i].key != SDLK_LAST; i++) {
		const struct keycode *kcode = &keycodes[i];

		if (kcode->key != (SDLKey)keysym || kcode->func== NULL)
			continue;

		if (kcode->modmask == 0 || keymod & kcode->modmask) {
			kcode->func(tbox, (SDLKey)keysym, keymod, kcode->arg);
			event_post(tbox, "textbox-changed", NULL);
			break;
		}
	}
	pthread_mutex_unlock(&tbox->lock);
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
	
	widget_focus(tbox);
	tbox->newx = x;
}

void
textbox_printf(struct textbox *tbox, const char *fmt, ...)
{
	struct widget_binding *stringb;
	va_list args;
	char *s;

	pthread_mutex_lock(&tbox->lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);

	va_start(args, fmt);
	vsnprintf(s, stringb->size, fmt, args);
	va_end(args);

	tbox->pos = 0;
	tbox->offs = 0;

	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->lock);
}

char *
textbox_string(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *s, *sd;

	pthread_mutex_lock(&tbox->lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	sd = Strdup(s);
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->lock);
	return (sd);
}

size_t
textbox_copy_string(struct textbox *tbox, char *dst, size_t dst_size)
{
	struct widget_binding *stringb;
	size_t rv;
	char *s;

	pthread_mutex_lock(&tbox->lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	rv = strlcpy(dst, s, dst_size);
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->lock);

	return (rv);
}

int
textbox_int(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *s;
	int i;

	pthread_mutex_lock(&tbox->lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	i = atoi(s);
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->lock);

	return (i);
}

void
textbox_set_writeable(struct textbox *tbox, int wr)
{
	tbox->writeable = wr;
}


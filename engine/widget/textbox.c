/*	$Csoft: textbox.c,v 1.58 2003/05/25 04:22:12 vedge Exp $	*/

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

#include <engine/compat/strlcpy.h>
#include <engine/compat/vsnprintf.h>

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
	NULL		/* update */
};

enum {
	MOUSE_SCROLL_INCR =	4,
	KBD_SCROLL_INCR =	4,
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
textbox_new(struct region *reg, const char *label)
{
	struct textbox *textbox;

	textbox = Malloc(sizeof(struct textbox));
	textbox_init(textbox, label);
	region_attach(reg, textbox);
	return (textbox);
}

void
textbox_init(struct textbox *tbox, const char *label)
{
	widget_init(&tbox->wid, "textbox", &textbox_ops, 100, -1);

	widget_bind(tbox, "string", WIDGET_STRING, NULL, tbox->def.string,
	    sizeof(tbox->def.string));
	tbox->def.string[0] = '\0';

	widget_map_color(tbox, FRAME_COLOR, "frame", 100, 100, 100);
	widget_map_color(tbox, FRAME_READONLY_COLOR, "frame-readonly",
	    40, 40, 40);
	widget_map_color(tbox, TEXT_COLOR, "text", 250, 250, 250);
	widget_map_color(tbox, CURSOR_COLOR1, "cursor1", 50, 50, 50);
	widget_map_color(tbox, CURSOR_COLOR2, "cursor2", 0, 0, 0);

	tbox->xpadding = 4;
	tbox->ypadding = 3;
	tbox->writeable = 1;
	if (label != NULL) {
		tbox->label = text_render(NULL, -1,
		    WIDGET_COLOR(tbox, TEXT_COLOR), (char *)label);
	} else {
		tbox->label = NULL;
	}
	tbox->text.pos = -1;
	tbox->text.offs = 0;
	pthread_mutex_init(&tbox->text.lock, &recursive_mutexattr);
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

	if (tbox->label != NULL)
		SDL_FreeSurface(tbox->label);

	pthread_mutex_destroy(&tbox->text.lock);
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

	pthread_mutex_lock(&tbox->text.lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	tlen = strlen(s);

	/* Default to the end of the string. */
	if (tbox->text.pos < 0)
		tbox->text.pos = tlen;

	/* Move to the beginning of the string? */
	if (tbox->newx >= 0 && tbox->newx <= tbox->label->w) {
		if (tbox->newx < tbox->label->w - tbox->xpadding) {
			if ((tbox->text.offs -= MOUSE_SCROLL_INCR) < 0)
				tbox->text.offs = 0;
		}
		tbox->text.pos = tbox->text.offs;
		tbox->newx = -1;
	}
drawtext:
	x = tbox->label->w + tbox->xpadding;
	y = tbox->ypadding;
	primitives.box(tbox, x, 0,
	    WIDGET(tbox)->w - x - 1,
	    WIDGET(tbox)->h,
	    WIDGET_FOCUSED(tbox) ? -1 : 1,
	    tbox->writeable ?
	        WIDGET_COLOR(tbox, FRAME_COLOR) :
		WIDGET_COLOR(tbox, FRAME_READONLY_COLOR));
	x += tbox->xpadding;
	if (WIDGET_FOCUSED(tbox)) {
		x++;
		y++;
	}
	for (i = tbox->text.offs, lx = -1;
	     i <= tlen;
	     i++) {
		SDL_Surface *glyph;
		char ch = s[i];

		/* Move to a position inside the string? */
		if (tbox->newx >= 0 &&
		    tbox->newx >= lx && tbox->newx < x) {
			tbox->newx = -1;
			tbox->text.pos = i;
		}
		lx = x;

		if (i == tbox->text.pos && WIDGET_FOCUSED(tbox)) {
			primitives.line(tbox,
			    x, y,
			    x, y + tbox->label->h - 2,
			    WIDGET_COLOR(tbox, CURSOR_COLOR1));
			primitives.line(tbox,
			    x+1, y,
			    x+1, y + tbox->label->h - 2,
			    WIDGET_COLOR(tbox, CURSOR_COLOR2));
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
	if (WIDGET_FOCUSED(tbox) && !cursdrawn) {
		if (tbox->text.pos > i)
			tbox->text.offs += KBD_SCROLL_INCR;
		goto drawtext;
	}

	/* Move beyond the visible end of the string? */
	if (tbox->newx >= 0) {
		tbox->newx = -1;
		tbox->text.pos = i-1;
		if (i < tlen) {
			tbox->text.offs += MOUSE_SCROLL_INCR;
			tbox->text.pos++;
		}
	}
	widget_binding_unlock(stringb);
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
	int i;

	if (!tbox->writeable)
		return;

	if (keysym == SDLK_RETURN) {
		event_post(tbox, "textbox-return", NULL);
		return;
	}

	pthread_mutex_lock(&tbox->text.lock);
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
	pthread_mutex_unlock(&tbox->text.lock);
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
	struct widget_binding *stringb;
	va_list args;
	char *s;

	pthread_mutex_lock(&tbox->text.lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);

	va_start(args, fmt);
	vsnprintf(s, stringb->size, fmt, args);
	va_end(args);

	tbox->text.pos = 0;
	tbox->text.offs = 0;

	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->text.lock);
}

char *
textbox_string(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *s, *sd;

	pthread_mutex_lock(&tbox->text.lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	sd = Strdup(s);
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->text.lock);
	return (sd);
}

size_t
textbox_copy_string(struct textbox *tbox, char *dst, size_t dst_size)
{
	struct widget_binding *stringb;
	size_t rv;
	char *s;

	pthread_mutex_lock(&tbox->text.lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	rv = strlcpy(dst, s, dst_size);
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->text.lock);

	return (rv);
}

int
textbox_int(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *s;
	int i;

	pthread_mutex_lock(&tbox->text.lock);
	stringb = widget_binding_get_locked(tbox, "string", &s);
	i = atoi(s);
	widget_binding_unlock(stringb);
	pthread_mutex_unlock(&tbox->text.lock);

	return (i);
}

void
textbox_set_writeable(struct textbox *tbox, int wr)
{
	tbox->writeable = wr;
}


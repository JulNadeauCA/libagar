/*	$Csoft: textbox.c,v 1.65 2003/06/15 08:54:19 vedge Exp $	*/

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
	CURSOR_COLOR
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
	widget_init(tbox, "textbox", &textbox_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	widget_bind(tbox, "string", WIDGET_STRING, NULL, tbox->string,
	    sizeof(tbox->string));

	widget_map_color(tbox, READWRITE_COLOR, "edition", 100, 100, 100, 255);
	widget_map_color(tbox, READONLY_COLOR, "read-only", 40, 40, 40, 255);
	widget_map_color(tbox, TEXT_COLOR, "text", 250, 250, 250, 255);
	widget_map_color(tbox, CURSOR_COLOR, "cursor", 40, 40, 40, 255);

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

	widget_destroy(tbox);
}

void
textbox_draw(void *p)
{
	struct textbox *tbox = p;
	struct widget_binding *stringb;
	int i, lx, cursdrawn = 0;
	int x = 0, y;
	size_t textlen = 0;
	void *text;
	char *s;
	Uint16 *ucs;

	if (tbox->label != NULL) {
		widget_blit(tbox, tbox->label, 0,
		    WIDGET(tbox)->h/2 - tbox->label->h/2);
		x = tbox->label->w;
	}

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		textlen = strlen((char *)text);
		break;
	case WIDGET_UNICODE:
		textlen = ucslen((Uint16 *)text);
		break;
	}

	if (tbox->pos < 0) {					/* Default */
		tbox->pos = textlen;
	} else if (tbox->pos > textlen) {			/* Past end */
		tbox->pos = textlen;
	}

	/* Move to the beginning of the string? */
	if (tbox->newx >= 0 && tbox->newx <= tbox->label->w) {
		if (tbox->newx < tbox->label->w - tbox->xpadding) {
			if ((tbox->offs -= MOUSE_SCROLL_INCR) < 0)
				tbox->offs = 0;
		}
		tbox->pos = tbox->offs;
		if (tbox->pos > textlen) {
			tbox->pos = textlen;
		}
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
	     i <= textlen;
	     i++) {
		SDL_Surface *glyph;
		Uint16 uch = 0;

		switch (stringb->type) {
		case WIDGET_STRING:
			s = text;
			uch = (unsigned char)s[i];
			break;
		case WIDGET_UNICODE:
			ucs = text;
			uch = ucs[i];
			break;
		}

		/* Move to a position inside the string? */
		if (tbox->newx >= 0 &&
		    tbox->newx >= lx && tbox->newx < x) {
			tbox->newx = -1;
			tbox->pos = i;
			if (tbox->pos > textlen)
				tbox->pos = textlen;
		}
		lx = x;

		if (i == tbox->pos && widget_holds_focus(tbox)) {
			primitives.line2(tbox, x, y, x, y+tbox->label->h-2,
			    CURSOR_COLOR);
			cursdrawn++;
		}
		
		if (x > WIDGET(tbox)->w - tbox->xpadding*4)
			break;

		if (uch == '\n') {
			y += tbox->label->h + 2;
		} else {
			Uint16 tcs[2];

			/* XXX use text_render_glyph when it is fixed */
			tcs[0] = uch;
			tcs[1] = '\0';
			glyph = text_render_unicode(NULL, -1,
			    WIDGET_COLOR(tbox, TEXT_COLOR), tcs);
			widget_blit(tbox, glyph, x, y);
			x += glyph->w;
			SDL_FreeSurface(glyph);
		}
	}
	if (widget_holds_focus(tbox) && !cursdrawn) {
		if (tbox->pos > i) {
			tbox->offs += KBD_SCROLL_INCR;
		}
		goto drawtext;
	}

	/* Move beyond the visible end of the string? */
	if (tbox->newx >= 0) {
		tbox->newx = -1;
		tbox->pos = i-1;
		if (i < textlen) {
			tbox->offs += MOUSE_SCROLL_INCR;
			tbox->pos++;
		}
		if (tbox->pos > textlen)
			tbox->pos = textlen;
	}
	widget_binding_unlock(stringb);
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
	SDLKey keysym = (SDLKey)argv[1].i;
	int keymod = argv[2].i;
	Uint16 unicode = (Uint16)argv[3].i;
	int i;

	if (!tbox->writeable)
		return;

	if (keysym == SDLK_TAB) {
		return;
	}
	if (keysym == SDLK_RETURN) {
		event_post(tbox, "textbox-return", NULL);
		WIDGET(tbox)->flags &= ~(WIDGET_FOCUSED);
		return;
	}

	for (i = 0;; i++) {
		const struct keycode *kcode = &keycodes[i];

		if (kcode->key != SDLK_LAST &&
		   (kcode->key != keysym || kcode->func == NULL))
			continue;

		if (kcode->key == SDLK_LAST ||
		    kcode->modmask == 0 || (keymod & kcode->modmask)) {
			kcode->func(tbox, keysym, keymod, kcode->arg, unicode);
			event_post(tbox, "textbox-changed", NULL);
			break;
		}
	}
#if 0
	dprintf("offs=%d, pos=%d\n", tbox->offs, tbox->pos);
#endif
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

/* Format Latin-1 text. */
void
textbox_printf(struct textbox *tbox, const char *fmt, ...)
{
	struct widget_binding *stringb;
	va_list args;
	void *text;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		va_start(args, fmt);
		vsnprintf((char *)text, stringb->size, fmt, args);
		va_end(args);
		break;
	case WIDGET_UNICODE:
		{
			char *latin1;

			latin1 = Malloc(stringb->size / sizeof(Uint16));
			va_start(args, fmt);
			vsnprintf(latin1, stringb->size, fmt, args);
			va_end(args);
			unicode_convert(UNICODE_FROM_LATIN1, (Uint16 *)text,
			    latin1, stringb->size);
			free(latin1);
		}
		break;
	}
	tbox->pos = 0;
	tbox->offs = 0;
	widget_binding_unlock(stringb);
}

/* Return a copy of Latin-1 text. */
char *
textbox_string(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *s, *sd;

	stringb = widget_get_binding(tbox, "string", &s);
	switch (stringb->type) {
	case WIDGET_STRING:
		sd = Strdup(s);
		break;
	default:
		fatal("not Latin-1");
		break;
	}
	widget_binding_unlock(stringb);
	return (sd);
}

/* Return a copy of Unicode/Latin-1 text. */
Uint16 *
textbox_unicode(struct textbox *tbox)
{
	struct widget_binding *stringb;
	void *text;
	Uint16 *ucs = NULL;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_UNICODE:
		ucs = ucsdup((Uint16 *)text);
		break;
	case WIDGET_STRING:
		ucs = unicode_import(UNICODE_FROM_LATIN1, (char *)text);
		break;
	}
	widget_binding_unlock(stringb);
	return (ucs);
}

/* Copy Latin-1 text to a fixed-size buffer and NUL-terminate. */
size_t
textbox_copy_string(struct textbox *tbox, char *dst, size_t dst_size)
{
	struct widget_binding *stringb;
	size_t rv;
	void *text;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		rv = strlcpy(dst, (char *)text, dst_size);
		break;
	default:
		fatal("not Latin-1");
		break;
	}
	widget_binding_unlock(stringb);
	return (rv);
}

/* Copy Latin-1/Unicode text to a fixed-size buffer and NUL-terminate. */
size_t
textbox_copy_unicode(struct textbox *tbox, Uint16 *dst, size_t dst_size)
{
	struct widget_binding *stringb;
	size_t rv = 0;
	void *text;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_UNICODE:
		rv = ucslcpy(dst, (Uint16 *)text, dst_size);
		break;
	case WIDGET_STRING:
		{
			Uint16 *ucs;

			ucs = unicode_import(UNICODE_FROM_LATIN1, (char *)text);
			rv = ucslcpy(dst, ucs, dst_size);
			free(ucs);
		}
		break;
	}
	widget_binding_unlock(stringb);
	return (rv);
}

/* Perform trivial conversion from Latin-1 text to int. */
int
textbox_int(struct textbox *tbox)
{
	struct widget_binding *stringb;
	void *text;
	int i;

	stringb = widget_get_binding(tbox, "string", &text);
	switch (stringb->type) {
	case WIDGET_STRING:
		i = atoi((char *)text);
		break;
	default:
		fatal("not Latin-1");
		break;
	}
	widget_binding_unlock(stringb);
	return (i);
}

void
textbox_set_writeable(struct textbox *tbox, int wr)
{
	tbox->writeable = wr;
}


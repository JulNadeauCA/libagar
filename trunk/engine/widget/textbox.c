/*	$Csoft: textbox.c,v 1.85 2005/02/08 15:45:38 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/config.h>

#include <config/have_freetype.h>

#ifdef HAVE_FREETYPE
#include <engine/loader/ttf.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif

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
		NULL,		/* reinit */
		textbox_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	textbox_draw,
	textbox_scale
};

enum {
	READWRITE_COLOR,
	READONLY_COLOR,
	TEXT_COLOR,
	CURSOR_COLOR
};

static void mousebuttondown(int, union evarg *);
static void mousemotion(int, union evarg *);
static void keydown(int, union evarg *);
static void keyup(int, union evarg *);

struct textbox *
textbox_new(void *parent, const char *label)
{
	struct textbox *textbox;

	textbox = Malloc(sizeof(struct textbox), M_OBJECT);
	textbox_init(textbox, label);
	object_attach(parent, textbox);
	return (textbox);
}

static int
process_key(struct textbox *tb, SDLKey keysym, SDLMod keymod, Uint32 unicode)
{
	int i;
	int rv = 0;

	for (i = 0;; i++) {
		const struct keycode *kcode = &keycodes[i];
		
		if (kcode->key != SDLK_LAST &&
		   (kcode->key != keysym || kcode->func == NULL))
			continue;
		
		if (kcode->key == SDLK_LAST ||
		    kcode->modmask == 0 || (keymod & kcode->modmask)) {
		  	if (kcode->clr_compo) {
				tb->compose = 0;
			}
			event_post(NULL, tb, "textbox-prechg", NULL);
			rv = kcode->func(tb, keysym, keymod, kcode->arg,
			    unicode);
			event_post(NULL, tb, "textbox-postchg", NULL);
			break;
		}
	}
	return (rv);
}

static Uint32
repeat_expire(void *obj, Uint32 ival, void *arg)
{
	struct textbox *tb = obj;

	if (process_key(tb, tb->repeat.key, tb->repeat.mod,
	    tb->repeat.unicode) == 0) {
		return (0);
	}
	return (kbd_repeat);
}

static Uint32
delay_expire(void *obj, Uint32 ival, void *arg)
{
	struct textbox *tb = obj;

	timeout_replace(tb, &tb->repeat_to, kbd_repeat);
	timeout_del(tb, &tb->cblink_to);
	tb->blink_state = 1;
	return (0);
}

static Uint32
blink_expire(void *obj, Uint32 ival, void *arg)
{
	struct textbox *tb = obj;

	tb->blink_state = !tb->blink_state;
	return (ival);
}

static void
gained_focus(int argc, union evarg *argv)
{
	struct textbox *tb = argv[0].p;

	timeout_del(tb, &tb->delay_to);
	timeout_del(tb, &tb->repeat_to);
	timeout_replace(tb, &tb->cblink_to, text_blink_rate);
}

static void
lost_focus(int argc, union evarg *argv)
{
	struct textbox *tb = argv[0].p;

	lock_timeout(tb);
	timeout_del(tb, &tb->delay_to);
	timeout_del(tb, &tb->repeat_to);
	timeout_del(tb, &tb->cblink_to);
	unlock_timeout(tb);
}

void
textbox_init(struct textbox *tbox, const char *label)
{
	widget_init(tbox, "textbox", &textbox_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	widget_bind(tbox, "string", WIDGET_STRING, tbox->string,
	    sizeof(tbox->string));

	widget_map_color(tbox, READWRITE_COLOR, "edition", 100, 100, 100, 255);
	widget_map_color(tbox, READONLY_COLOR, "read-only", 40, 40, 40, 255);
	widget_map_color(tbox, TEXT_COLOR, "text", 250, 250, 250, 255);
	widget_map_color(tbox, CURSOR_COLOR, "cursor", 40, 40, 40, 255);

	tbox->string[0] = '\0';
	tbox->xpadding = 4;
	tbox->ypadding = 3;
	tbox->writeable = 1;
	tbox->prew = tbox->xpadding*2 + 90;			/* XXX */
	tbox->preh = tbox->ypadding*2;
	tbox->pos = 0;
	tbox->offs = 0;
	tbox->compose = 0;
	tbox->blink_state = 1;
	tbox->sel_x1 = 0;
	tbox->sel_x2 = 0;
	tbox->sel_edit = 0;

	if (label != NULL) {
		tbox->label = text_render(NULL, -1,
		    WIDGET_COLOR(tbox, TEXT_COLOR), (char *)label);
		tbox->prew += tbox->label->w;
		tbox->preh += tbox->label->h;
	} else {
		tbox->label = NULL;
	}
	
	timeout_set(&tbox->repeat_to, repeat_expire, NULL, 0);
	timeout_set(&tbox->delay_to, delay_expire, NULL, 0);
	timeout_set(&tbox->cblink_to, blink_expire, NULL, 0);

	event_new(tbox, "window-keydown", keydown, NULL);
	event_new(tbox, "window-keyup", keyup, NULL);
	event_new(tbox, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(tbox, "window-mousemotion", mousemotion, NULL);
	event_new(tbox, "widget-gainfocus", gained_focus, NULL);
	event_new(tbox, "widget-lostfocus", lost_focus, NULL);
	event_new(tbox, "widget-hidden", lost_focus, NULL);
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
	struct text_font *font;
	int i, x, y;
	size_t len;
	char *s;
	
	if (tbox->label != NULL) {
		widget_blit(tbox, tbox->label, 0,
		    WIDGET(tbox)->h/2 - tbox->label->h/2);
	}

	font = text_fetch_font(
	    prop_get_string(config, "font-engine.default-font"),
	    prop_get_int(config, "font-engine.default-size"),
	    0);

	stringb = widget_get_binding(tbox, "string", &s);
	len = strlen(s);

	x = ((tbox->label!=NULL) ? tbox->label->w : 0) + tbox->xpadding;
	y = tbox->ypadding;

	primitives.box(tbox,
	    x,
	    0,
	    WIDGET(tbox)->w - x - 1,
	    WIDGET(tbox)->h,
	    (WIDGET(tbox)->flags & WIDGET_FOCUSED) ? -1 : 1,
	    tbox->writeable ? READWRITE_COLOR : READONLY_COLOR);

	x += tbox->xpadding;
	if (WIDGET(tbox)->flags & WIDGET_FOCUSED) {
		x++;
		y++;
	}
	for (i = 0; i <= len; i++) {
		int invert = 0;

		if ((WIDGET(tbox)->flags & WIDGET_FOCUSED) &&
		    tbox->blink_state) {
			if (i == tbox->pos) {
				primitives.line(tbox,
				    x,
				    y + 1,
				    x,
				    y + text_font_height - 2,
				    CURSOR_COLOR);
			}
		}
		if (i == len)
			break;

		if (s[i] == '\n') {
			y += text_font_line_skip;
			continue;
		} else if (s[i] == '\t') {
			x += text_tab_width;
			continue;
		}
		{
			FT_Bitmap *ftbmp;
			Uint32 ch = (Uint32)s[i];
			struct ttf_font *ttf = font->p;
			struct ttf_glyph *glyph;
			int xglyph, yglyph;
			Uint8 *src;

			if (ttf_find_glyph(ttf, ch,
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
				continue;
			}
			glyph = ttf->current;
			ftbmp = &glyph->bitmap;
			src = ftbmp->buffer;

			if (i == 0 && glyph->minx < 0) {
				x -= glyph->minx;
			}
			if ((x + glyph->minx + ftbmp->width + glyph->advance)
			    >= WIDGET(tbox)->w)
				continue;

			for (yglyph = 0; yglyph < ftbmp->rows; yglyph++) {
				/* Work around FreeType 9.3.3 bug. */
				if (glyph->yoffset < 0)
					glyph->yoffset = 0;

				for (xglyph = 0; xglyph < ftbmp->width;
				     xglyph++) {
					if ((invert && src[xglyph]) ||
					   (!invert && !src[xglyph])) {
						continue;
					}
					widget_put_pixel(tbox,
					    x + glyph->minx + xglyph,
					    y + glyph->yoffset + yglyph,
					    WIDGET_COLOR(tbox, TEXT_COLOR));
				}
				src += ftbmp->pitch;
			}
			x += glyph->advance;
		}
	}
out:
	widget_binding_unlock(stringb);
}

void
textbox_prescale(struct textbox *tbox, const char *text)
{
	text_prescale(text, &tbox->prew, NULL);
	tbox->prew += tbox->label->w + tbox->xpadding*2;
}

void
textbox_scale(void *p, int rw, int rh)
{
	struct textbox *tbox = p;

	if (rw == -1 && rh == -1) {
		WIDGET(tbox)->w = tbox->prew;
		WIDGET(tbox)->h = tbox->preh;
	}
}

static void
keydown(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	int keymod = argv[2].i;
	Uint32 unicode = (Uint32)argv[3].i;

	if (!tbox->writeable)
		return;

	if (keysym == SDLK_ESCAPE || keysym == SDLK_TAB) {
		return;
	}
	if (keysym == SDLK_RETURN) {
		widget_unset_focus(tbox);
		event_post(NULL, tbox, "textbox-return", NULL);
		return;
	}
	
	tbox->repeat.key = keysym;
	tbox->repeat.mod = keymod;
	tbox->repeat.unicode = unicode;
	tbox->blink_state = 1;

	lock_timeout(tbox);
	timeout_del(tbox, &tbox->repeat_to);
	if (process_key(tbox, keysym, keymod, unicode) == 1) {
		timeout_replace(tbox, &tbox->delay_to, kbd_delay);
	} else {
		timeout_del(tbox, &tbox->delay_to);
	}
	unlock_timeout(tbox);
}

static void
keyup(int argc, union evarg *argv)
{
	struct textbox *tb = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	int keymod = argv[2].i;
	Uint32 unicode = (Uint32)argv[3].i;

	if ((keysym == tb->repeat.key && unicode == tb->repeat.unicode) ||
	    (keysym == SDLK_RETURN)) {
		lock_timeout(tb);
		timeout_del(tb, &tb->repeat_to);
		timeout_del(tb, &tb->delay_to);
		timeout_replace(tb, &tb->cblink_to, text_blink_rate);
		unlock_timeout(tb);
	}
}

/* Map mouse coordinates to a position within the string. */
static int
cursor_position(struct textbox *tbox, int mx, int my, int *pos)
{
	struct widget_binding *stringb;
	struct text_font *font;
	int tstart = 0;
	int i, x, y;
	size_t len;
	char *s;

	x = ((tbox->label!=NULL) ? tbox->label->w : 0) + tbox->xpadding;
	if (mx <= x) {
		return (-1);
	}
	x += tbox->xpadding + (WIDGET(tbox)->flags & WIDGET_FOCUSED) ? 1 : 0;
	y = tbox->ypadding;

	stringb = widget_get_binding(tbox, "string", &s);
	len = strlen(s);
	font = text_fetch_font(
	    prop_get_string(config, "font-engine.default-font"),
	    prop_get_int(config, "font-engine.default-size"), 0);

	for (i = tstart; i < len; i++) {
		if (s[i] == '\n') {
			y += text_font_line_skip;
			continue;
		} else if (s[i] == '\t') {
			x += text_tab_width;
			continue;
		}
		{
			Uint32 ch = (Uint32)s[i];
			struct ttf_font *ttf = font->p;
			FT_Bitmap *ftbmp;
			struct ttf_glyph *glyph;

			if (ttf_find_glyph(ttf, ch,
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
				continue;
			}
			glyph = ttf->current;
			ftbmp = &glyph->bitmap;

			if (i == 0 && glyph->minx < 0)
				x -= glyph->minx;
			if ((x + glyph->minx+ftbmp->width)
			    >= WIDGET(tbox)->w)
				continue;
		
			if (mx >= x &&
			    mx < x+glyph->minx+ftbmp->width) {
				*pos = i;
				goto in;
			}
			x += glyph->advance;
		}
	}
	widget_binding_unlock(stringb);
	return (1);
in:
	widget_binding_unlock(stringb);
	return (0);
}

static void
move_cursor(struct textbox *tbox, int mx, int my)
{
	int rv;

	rv = cursor_position(tbox, mx, my, &tbox->pos);
	if (rv == -1) {
		tbox->pos = 0;
	} else if (rv == 1) {
		struct widget_binding *stringb;
		char *s;
		
		stringb = widget_get_binding(tbox, "string", &s);
		tbox->pos = strlen(s);
		widget_binding_unlock(stringb);
	}
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int btn = argv[1].i;
	int mx = argv[2].i;
	int my = argv[3].i;
	int rv;

	if (tbox->label == NULL ||
	    mx > tbox->label->w)
		widget_focus(tbox);

	if (btn == SDL_BUTTON_LEFT)
		move_cursor(tbox, mx, my);
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct textbox *tbox = argv[0].p;
	int mx = argv[1].i;
	int my = argv[2].i;
	int state = argv[5].i;

	if (state & SDL_BUTTON_LEFT)
		move_cursor(tbox, mx, my);
}

void
textbox_printf(struct textbox *tbox, const char *fmt, ...)
{
	struct widget_binding *stringb;
	va_list args;
	char *text;

	stringb = widget_get_binding(tbox, "string", &text);
	if (fmt != NULL) {
		va_start(args, fmt);
		vsnprintf(text, stringb->size, fmt, args);
		va_end(args);
	} else {
		text[0] = '\0';
	}
	/* XXX */
	tbox->pos = 0;
	widget_binding_unlock(stringb);
}

char *
textbox_string(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *s, *sd;

	stringb = widget_get_binding(tbox, "string", &s);
	sd = Strdup(s);
	widget_binding_unlock(stringb);
	return (sd);
}

/* Copy text to a fixed-size buffer and always NUL-terminate. */
size_t
textbox_copy_string(struct textbox *tbox, char *dst, size_t dst_size)
{
	struct widget_binding *stringb;
	size_t rv;
	char *text;

	stringb = widget_get_binding(tbox, "string", &text);
	rv = strlcpy(dst, text, dst_size);
	widget_binding_unlock(stringb);
	return (rv);
}

/* Perform trivial conversion from string to int. */
int
textbox_int(struct textbox *tbox)
{
	struct widget_binding *stringb;
	char *text;
	int i;

	stringb = widget_get_binding(tbox, "string", &text);
	i = atoi(text);
	widget_binding_unlock(stringb);
	return (i);
}

void
textbox_set_writeable(struct textbox *tbox, int wr)
{
	tbox->writeable = wr;
}


/*	$Csoft: text.c,v 1.55 2003/03/04 00:32:09 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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
#include <engine/compat/strsep.h>
#include <engine/engine.h>

#include <engine/view.h>
#include <engine/config.h>

#include "widget.h"
#include "window.h"
#include "label.h"
#include "button.h"
#include "text.h"
#include "textbox.h"
#include "keycodes.h"

ttf_font *font = NULL;		/* Default font */

/* Cached fonts */
struct text_font {
	char	 *name;
	int	  size;
	int	  style;
	ttf_font *font;
	SLIST_ENTRY(text_font) fonts;
};
static SLIST_HEAD(text_fontq, text_font) fonts = SLIST_HEAD_INITIALIZER(&fonts);
static pthread_mutex_t fonts_lock = PTHREAD_MUTEX_INITIALIZER;

static ttf_font *
text_load_font(char *name, int size, int style)
{
	char *path;
	ttf_font *nfont;
	struct text_font *fon;

	if (name == NULL) {		/* Default font */
		return (font);
	}

	pthread_mutex_lock(&fonts_lock);

	SLIST_FOREACH(fon, &fonts, fonts) {
		if (strcmp(fon->name, name) == 0 &&
		    fon->size == size &&
		    fon->style == style) {
			pthread_mutex_unlock(&fonts_lock);
			return (fon->font);
		}
	}

	path = object_path(name, "ttf");
	if (path == NULL) {
		fatal("%s.ttf: %s", name, error_get());
	}
	nfont = ttf_open_font(path, size);
	if (nfont == NULL) {
		fatal("%s: %s", path, error_get());
	}
	free(path);

	ttf_set_font_style(nfont, style);

	fon = emalloc(sizeof(struct text_font));
	fon->name = Strdup(name);
	fon->size = size;
	fon->style = style;
	fon->font = nfont;

	SLIST_INSERT_HEAD(&fonts, fon, fonts);

	pthread_mutex_unlock(&fonts_lock);

	return (nfont);
}

int
text_init(void)
{
	if (ttf_init() == -1) {
		error_set("ttf_init: %s", SDL_GetError());
		/* TODO revert to a bitmap font! */
		return (-1);
	}
	return (0);
}

void
text_set_default_font(char *name, int size, int style)
{
	if (font != NULL) {
		/* Replacing the default font would be thread-unsafe. */
		fatal("default font already set");
	}

	dprintf("%s:%d,%d\n",
	    prop_get_string(config, "font-engine.default-font"),
	    prop_get_int(config, "font-engine.default-size"),
	    prop_get_int(config, "font-engine.default-style"));

	font = text_load_font(
	    prop_get_string(config, "font-engine.default-font"),
	    prop_get_int(config, "font-engine.default-size"),
	    prop_get_int(config, "font-engine.default-style"));
}

static void
text_destroy_font(struct text_font *fon)
{
	free(fon->name);
	ttf_close_font(fon->font);
}

void
text_destroy(void)
{
	struct text_font *fon, *nextfon;
	
	for (fon = SLIST_FIRST(&fonts);
	     fon != SLIST_END(&fonts);
	     fon = nextfon) {
		nextfon = SLIST_NEXT(fon, fonts);
		text_destroy_font(fon);
		free(fon);
	}
	ttf_destroy();
}

int
text_font_height(ttf_font *fon)
{
	return (ttf_font_height((fon != NULL) ? fon : font));
}

int
text_font_ascent(ttf_font *fon)
{
	return (ttf_font_ascent((fon != NULL) ? fon : font));
}

int
text_font_descent(ttf_font *fon)
{
	return (ttf_font_descent((fon != NULL) ? fon : font));
}

int
text_font_line_skip(ttf_font *fon)
{
	return (ttf_font_line_skip((fon != NULL) ? fon : font));
}

SDL_Surface *
text_render(char *fontname, int fontsize, Uint32 color, char *s)
{
	SDL_Surface *su;
	SDL_Color col;
	Uint8 r, g, b;
	ttf_font *fon;
	SDL_Rect rd;
	int nlines, maxw;
	char *sd, *sp;

	if (s == NULL || strcmp("", s) == 0 ||
	    !prop_get_bool(config, "font-engine")) {
		/* Return an empty surface. */
		dprintf("empty surface\n");
		return (view_surface(SDL_SWSURFACE, 0, 0));
	}

	/* Get a font handle. */
	fon = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));
	
	/* Decompose the color. */
	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	/* Find out the line count. */
	sd = Strdup(s);
	for (sp = sd, nlines = 0; *sp != '\0'; sp++) {
		if (*sp == '\n') {
			nlines++;
		}
	}

	if (nlines == 0) {
		/* Render a single line. */
		su = ttf_render_text_solid(fon, sd, col);
		if (su == NULL) {
			fatal("ttf_render_text_solid: %s", error_get());
		}
	} else {
		SDL_Surface **lines, **lp;
		int i;

		/* XXX inefficient */

		/*
		 * Render the text to an array of surfaces, since we cannot
		 * predict the width of the final surface.
		 */
		lines = emalloc(sizeof(SDL_Surface *) * nlines);
		for (lp = lines, maxw = 0;
		    (sp = strsep(&sd, "\n")) != NULL;
		    lp++) {
		    	if (strcmp(sp, "") == 0) {
				*lp = NULL;
				continue;
			}
			*lp = ttf_render_text_solid(fon, sp, col);
			if (*lp == NULL) {
				fatal("ttf_render_text_solid: %s", error_get());
			}
			if ((*lp)->w > maxw) {
				maxw = (*lp)->w;	/* Grow width */
			}
		}

		rd.x = 0;
		rd.y = 0;
		rd.w = 0;
		rd.h = ttf_font_height(fon);

		/* Render the final surface. */
		su = view_surface(SDL_SWSURFACE, maxw, rd.h * nlines);
		for (i = 0;
		     i < nlines;
		     i++, rd.y += rd.h) {	/* XXX respect line skip */
			if (lines[i] == NULL) {
				continue;
			}
			rd.w = lines[i]->w;
			SDL_BlitSurface(lines[i], NULL, su, &rd);
			SDL_FreeSurface(lines[i]);
		}
		free(lines);
	}

	free(sd);
	return (su);
}

void
text_msg(const char *caption, const char *format, ...)
{
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *button;
	va_list args;
	char *msg;
	SDL_Surface *msg_eval;
	Uint16 w, h;

	va_start(args, format);
	Vasprintf(&msg, format, args);
	va_end(args);

	/* Auto-size the window. XXX waste */
	msg_eval = text_render(NULL, -1, 0, msg);
	w = msg_eval->w;
	h = msg_eval->h;
	SDL_FreeSurface(msg_eval);

	win = window_generic_new(w + 20, h + 100, NULL);
	window_set_caption(win, "%s", caption);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		lab = label_new(reg, 100, 60, msg);
		button = button_new(reg, "Ok", NULL, 0, 99, 40);
		WIDGET_FOCUS(button);

		event_new(button, "button-pushed",
		    window_generic_detach, "%p", win);
		window_show(win);
	}

	free(msg);
}


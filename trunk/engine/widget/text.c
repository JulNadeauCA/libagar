/*	$Csoft: text.c,v 1.60 2003/04/25 09:38:49 vedge Exp $	*/

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

#include <engine/compat/vsnprintf.h>
#include <engine/compat/strsep.h>

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/config.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/button.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

ttf_font *font = NULL;		/* Default font */

/* Cached fonts */
struct text_font {
	char	 *name;
	int	  size;
	int	  style;
	ttf_font *font;
	SLIST_ENTRY(text_font) fonts;
};

static SLIST_HEAD(text_fontq, text_font) text_fonts =
    SLIST_HEAD_INITIALIZER(&text_fonts);
pthread_mutex_t text_lock = PTHREAD_MUTEX_INITIALIZER;

static ttf_font *
text_load_font(char *name, int size, int style)
{
	char path[FILENAME_MAX];
	ttf_font *nfont;
	struct text_font *fon;

	if (name == NULL) 		/* Default font */
		return (font);

	pthread_mutex_lock(&text_lock);

	SLIST_FOREACH(fon, &text_fonts, fonts) {
		if (strcmp(fon->name, name) == 0 &&
		    fon->size == size &&
		    fon->style == style) {
			pthread_mutex_unlock(&text_lock);
			return (fon->font);
		}
	}

	if (object_path(name, "ttf", path, sizeof(path)) == -1)
		fatal("%s.ttf: %s", name, error_get());

	if ((nfont = ttf_open_font(path, size)) == NULL)
		fatal("%s: %s", path, error_get());

	ttf_set_font_style(nfont, style);

	fon = Malloc(sizeof(struct text_font));
	fon->name = Strdup(name);
	fon->size = size;
	fon->style = style;
	fon->font = nfont;

	SLIST_INSERT_HEAD(&text_fonts, fon, fonts);		/* Cache */
	pthread_mutex_unlock(&text_lock);
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
	
	for (fon = SLIST_FIRST(&text_fonts);
	     fon != SLIST_END(&text_fonts);
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

/* Render a single glyph onto a new surface. */
SDL_Surface *
text_render_glyph(char *fontname, int fontsize, Uint32 color, char ch)
{
	ttf_font *fon;
	SDL_Color col;
	SDL_Surface *su;
	Uint8 r, g, b;

	fon = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));

	/* Decompose the color. */
	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	su = ttf_render_glyph_solid(fon, (unsigned char)ch, col);
	if (su == NULL)
		fatal("rendering glyph: %s", error_get());
	return (su);
}

/* Render a (possibly multi-line) string onto a new surface. */
SDL_Surface *
text_render(char *fontname, int fontsize, Uint32 color, char *s)
{
	SDL_Surface *su;
	SDL_Color col;
	Uint8 r, g, b;
	ttf_font *fon;
	SDL_Rect rd;
	int nlines, maxw, fon_h;
	char *sd, *sp;

	if (s == NULL || s[0] == '\0' ||
	    !prop_get_bool(config, "font-engine")) {	    /* Null surface */
		return (view_surface(SDL_SWSURFACE, 0, 0));
	}

	fon = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));
	fon_h = text_font_height(fon);

	/* Decompose the color. */
	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	/* Find out the line count. */
	sd = Strdup(s);
	for (sp = sd, nlines = 0; *sp != '\0'; sp++) {
		if (*sp == '\n')
			nlines++;
	}

	if (nlines == 0) {					/* One line */
		su = ttf_render_text_solid(fon, sd, col);
		if (su == NULL) {
			fatal("ttf_render_text_solid: %s", error_get());
		}
	} else {						/* Multiline */
		SDL_Surface **lines, **lp;
		int i;

		/*
		 * Render the text to an array of surfaces, since we cannot
		 * predict the width of the final surface.
		 */
		lines = Malloc(sizeof(SDL_Surface *) * nlines);
		for (lp = lines, maxw = 0;
		    (sp = strsep(&sd, "\n")) != NULL;
		    lp++) {
		    	if (sp[0] == '\0') {
				*lp = NULL;
				continue;
			}
			*lp = ttf_render_text_solid(fon, sp, col);
			if (*lp == NULL) {
				fatal("ttf_render_text_solid: %s", error_get());
			}
			if ((*lp)->w > maxw)
				maxw = (*lp)->w;	/* Grow width */
		}

		rd.x = 0;
		rd.y = 0;
		rd.w = 0;
		rd.h = fon_h;

		/* Render the final surface. */
		su = view_surface(SDL_SWSURFACE, maxw, rd.h*nlines);
		for (i = 0;
		     i < nlines;
		     i++, rd.y += rd.h) {	/* XXX respect line skip */
			if (lines[i] == NULL)
				continue;

			rd.w = lines[i]->w;
			SDL_BlitSurface(lines[i], NULL, su, &rd);
			SDL_FreeSurface(lines[i]);
		}
		free(lines);
	}

	free(sd);
	return (su);
}

/* Display a message. */
void
text_msg(const char *caption, const char *format, ...)
{
	char msg[LABEL_MAX_LENGTH];
	struct window *win;
	struct region *reg;
	va_list args;
	SDL_Surface *msgsu;
	int w, h;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	/* Auto-size the window. XXX waste! */
	msgsu = text_render(NULL, -1, 0, msg);
	w = (int)msgsu->w;
	h = (int)msgsu->h;
	SDL_FreeSurface(msgsu);

	win = window_generic_new(msgsu->w + 20,
	    msgsu->h + ttf_font_height(font) + 60,
	    NULL);
	window_set_caption(win, "%s", caption);
	
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		label_new(reg, -1, -1, msg);
	}

	reg = region_new(win, REGION_VALIGN, 0, -1, 100, 0);
	{
		struct button *button;

		button = button_new(reg, "Ok", NULL, 0, 100, 0);
		WIDGET_FOCUS(button);
		event_new(button, "button-pushed", window_generic_detach,
		    "%p", win);
		window_show(win);
	}
}


/*	$Csoft: text.c,v 1.64 2003/06/06 09:03:54 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/config.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

ttf_font *font = NULL;		/* Default font */

#define FONT_NAME_MAX	32

struct text_font {
	char	  name[FONT_NAME_MAX];
	int	  size;
	int	  style;
	ttf_font *font;
	SLIST_ENTRY(text_font) fonts;
};

static SLIST_HEAD(text_fontq, text_font) text_fonts =	/* Cached fonts */
    SLIST_HEAD_INITIALIZER(&text_fonts);
pthread_mutex_t text_lock = PTHREAD_MUTEX_INITIALIZER;

static ttf_font *
text_load_font(const char *name, int size, int style)
{
	char fname[FILENAME_MAX];
	char *path = NULL;
	ttf_font *nfont;
	struct text_font *fon;

	if (name == NULL) 		/* Default font */
		return (font);

	pthread_mutex_lock(&text_lock);

	SLIST_FOREACH(fon, &text_fonts, fonts) {
		if (strcmp(fon->name, name) == 0 &&
		    fon->size == size &&
		    fon->style == style) {
			nfont = fon->font;
			goto out;
		}
	}

	snprintf(fname, sizeof(fname), "/engine/widget/fonts/%s", name);
	if ((path = config_search_file("load-path", fname, "ttf")) == NULL)
		fatal("%s", error_get());

	if ((nfont = ttf_open_font(path, size)) == NULL) {
		fatal("%s: %s", path, error_get());
	}
	ttf_set_font_style(nfont, style);

	fon = Malloc(sizeof(struct text_font));
	strlcpy(fon->name, name, sizeof(fon->name));
	fon->size = size;
	fon->style = style;
	fon->font = nfont;

	SLIST_INSERT_HEAD(&text_fonts, fon, fonts);		/* Cache */
out:
	pthread_mutex_unlock(&text_lock);
	Free(path);
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
#ifdef DEBUG
	/* Replacing the default font would be thread-unsafe. */
	if (font != NULL)
		fatal("default font already set");
#endif
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
text_render_glyph(const char *fontname, int fontsize, Uint32 color, char ch)
{
	ttf_font *fon;
	SDL_Color col;
	SDL_Surface *su;
	Uint8 r, g, b;

	fon = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));

	/* Decompose the color. */
	SDL_GetRGB(color, vfmt, &r, &g, &b);
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
text_render(const char *fontname, int fontsize, Uint32 color, const char *s)
{
	SDL_Surface *su;
	SDL_Color col;
	Uint8 r, g, b;
	ttf_font *fon;
	SDL_Rect rd;
	int nlines, maxw, fon_h;
	char *sd, *sp;

	if (s == NULL || s[0] == '\0' ||
	    !prop_get_bool(config, "font-engine")) 	    /* Null surface */
		return (view_surface(SDL_SWSURFACE, 0, 0));

	fon = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));
	fon_h = text_font_height(fon);

	/* Decompose the color. */
	SDL_GetRGB(color, vfmt, &r, &g, &b);
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
		int lineskip, i;

		/*
		 * Render the text to an array of surfaces, since we cannot
		 * predict the width of the final surface.
		 */
		lineskip = ttf_font_line_skip(fon);
		lines = Malloc(sizeof(SDL_Surface *) * nlines);
		for (lp = lines, maxw = 0;
		    (sp = strsep(&sd, "\n")) != NULL;
		    lp++) {
		    	if (sp[0] == '\0') {
				*lp = NULL;
				continue;
			}

			if ((*lp = ttf_render_text_solid(fon, sp, col)) == NULL)
				fatal("ttf_render_text_solid: %s", error_get());

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

/* Return the expected size of a text surface. */
void
text_prescale(const char *text, int *w, int *h)
{
	SDL_Surface *su;

	su = text_render(NULL, -1, 0, text);
	if (w != NULL)
		*w = (int)su->w;
	if (h != NULL)
		*h = (int)su->h;
	SDL_FreeSurface(su);
}

/* Display a message. */
void
text_msg(const char *caption, const char *format, ...)
{
	char msg[LABEL_MAX];
	struct window *win;
	struct vbox *vb;
	va_list args;
	struct button *button;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = window_new(NULL);
	window_set_caption(win, "%s", caption);
	window_set_position(win, WINDOW_CENTER, 1);

	vb = vbox_new(win, 0);
	label_new(vb, msg);

	vb = vbox_new(win, VBOX_HOMOGENOUS|VBOX_WFILL|VBOX_HFILL);
	button = button_new(vb, "Ok");
	event_new(button, "button-pushed", window_generic_detach, "%p", win);

	widget_focus(button);
	window_show(win);
}


/*	$Csoft: text.c,v 1.81 2004/04/20 09:20:31 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

static const char *text_msg_titles[] = {
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

static struct text_font *default_font = NULL;
int text_composition = 1;			/* Built-in input composition */
int text_rightleft = 0;				/* Right-to-left text display */

static SLIST_HEAD(text_fontq, text_font) text_fonts =	/* Cached fonts */
    SLIST_HEAD_INITIALIZER(&text_fonts);
pthread_mutex_t text_lock = PTHREAD_MUTEX_INITIALIZER;

/* Load and cache the named font. */
static struct text_font *
text_load_font(const char *name, int size, int style)
{
	char path[MAXPATHLEN];
	struct text_font *font;

	if (name == NULL)
		return (default_font);

	pthread_mutex_lock(&text_lock);

	SLIST_FOREACH(font, &text_fonts, fonts) {
		if (strcmp(font->name, name) == 0 &&
		    font->size == size &&
		    font->style == style)
			break;
	}
	if (font != NULL)
		goto out;

	if (config_search_file("font-path", name, "ttf", path, sizeof(path))
	    == -1)
		fatal("%s", error_get());
	
	font = Malloc(sizeof(struct text_font), M_TEXT);
	strlcpy(font->name, name, sizeof(font->name));
	font->size = size;
	font->style = style;

	if ((font->p = ttf_open_font(path, size)) == NULL) {
		fatal("%s: %s", path, error_get());
	}
	ttf_set_font_style(font->p, style);

	SLIST_INSERT_HEAD(&text_fonts, font, fonts);
out:
	pthread_mutex_unlock(&text_lock);
	return (font);
}

/* Initialize the text rendering engine and set the default font. */
int
text_init(int flags)
{
	if (prop_get_bool(config, "font-engine") == 0)
		return (0);

	if (flags & TEXT_TTF) {
		if (ttf_init() == -1) {
			error_set("ttf_init: %s", SDL_GetError());
			return (-1);
		}
		dprintf("face %s size %d style 0x%x\n",
		    prop_get_string(config, "font-engine.default-font"),
		    prop_get_int(config, "font-engine.default-size"),
		    prop_get_int(config, "font-engine.default-style"));
		default_font = text_load_font(
		    prop_get_string(config, "font-engine.default-font"),
		    prop_get_int(config, "font-engine.default-size"),
		    prop_get_int(config, "font-engine.default-style"));
	}
	return (0);
}

void
text_destroy(void)
{
	struct text_font *fon, *nextfon;
	
	for (fon = SLIST_FIRST(&text_fonts);
	     fon != SLIST_END(&text_fonts);
	     fon = nextfon) {
		nextfon = SLIST_NEXT(fon, fonts);
		ttf_close_font(fon->p);
		Free(fon, M_TEXT);
	}
	ttf_destroy();
}

int
text_font_height(struct text_font *font)
{
	return (ttf_font_height((font != NULL) ? font->p : default_font->p));
}

int
text_font_ascent(struct text_font *font)
{
	return (ttf_font_ascent((font != NULL) ? font->p : default_font->p));
}

int
text_font_descent(struct text_font *font)
{
	return (ttf_font_descent((font != NULL) ? font->p : default_font->p));
}

int
text_font_line_skip(struct text_font *font)
{
	return (ttf_font_line_skip((font != NULL) ? font->p : default_font->p));
}

/* Render an Unicode character onto a new surface. */
SDL_Surface *
text_render_glyph(const char *fontname, int fontsize, Uint32 color, Uint32 ch)
{
	struct text_font *font;
	SDL_Color col;
	SDL_Surface *su;
	Uint8 r, g, b;

	font = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));

	/* Decompose the color. */
	SDL_GetRGB(color, vfmt, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	su = ttf_render_glyph_solid(font->p, ch, col);
	if (su == NULL) {
		fatal("rendering glyph: %s", error_get());
	}
	return (su);
}

/* Render UTF-8 text onto a new surface. */
SDL_Surface *
text_render(const char *fontname, int fontsize, Uint32 color, const char *text)
{
	Uint32 *ucs;
	SDL_Surface *su;

	ucs = unicode_import(UNICODE_FROM_UTF8, text);
	su = text_render_unicode(fontname, fontsize, color, ucs);
	free(ucs);
	return (su);
}

/* Render (possibly multi-line) UCS-4 text onto a new surface. */
SDL_Surface *
text_render_unicode(const char *fontname, int fontsize, Uint32 color,
    const Uint32 *text)
{
	SDL_Rect rd;
	SDL_Color col;
	SDL_Surface *su;
	struct text_font *font;
	Uint32 *ucs, *ucsd, *ucsp;
	int nlines, maxw, font_h;
	Uint8 r, g, b;

	if (text == NULL || text[0] == '\0') {
		SDL_Surface *su;
	
		su = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 8, 0, 0, 0, 0);
		if (su == NULL) {
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		}
		return (su);
	}

	font = text_load_font(fontname, fontsize,
	    prop_get_int(config, "font-engine.default-style"));
	font_h = text_font_height(font);

	/* Decompose the color. */
	SDL_GetRGB(color, vfmt, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	/* Find out the line count. */
	ucsd = ucs = ucs4_dup(text);
	for (ucsp = ucs, nlines = 0; *ucsp != '\0'; ucsp++) {
		if (*ucsp == '\n')
			nlines++;
	}

	if (nlines == 0) {					/* One line */
		su = ttf_render_unicode_solid(font->p, ucs, col);
		if (su == NULL) {
			fatal("ttf_render_text_solid: %s", error_get());
		}
	} else {						/* Multiline */
		SDL_Surface **lines;
		int lineskip, i;
		const Uint32 sep[2] = { '\n', '\0' };
		Uint32 colorkey;

		/*
		 * Render the text to an array of surfaces, since we cannot
		 * predict the width of the final surface.
		 * XXX move to ttf_render_unicode_solid().
		 */
		lineskip = ttf_font_line_skip(font->p);
		lines = Malloc(sizeof(SDL_Surface *) * nlines, M_TEXT);
		for (i = 0, maxw = 0;
		    (ucsp = ucs4_sep(&ucs, sep)) != NULL && ucsp[0] != '\0';
		    i++) {
			lines[i] = ttf_render_unicode_solid(font->p, ucsp, col);
			if (lines[i] == NULL) {
				fatal("ttf_render_unicode_solid: %s",
				    error_get());
			}
			if (lines[i]->w > maxw)
				maxw = lines[i]->w;	/* Grow width */
		}

		rd.x = 0;
		rd.y = 0;
		rd.w = 0;
		rd.h = font_h;

		/* Generate the final surface. */
		su = SDL_CreateRGBSurface(SDL_SWSURFACE, maxw, lineskip*nlines,
		    vfmt->BitsPerPixel,
		    vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, 0);
		if (su == NULL)
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	
		colorkey = SDL_MapRGB(su->format, 15, 15, 15);
		SDL_FillRect(su, NULL, colorkey);

		for (i = 0;
		     i < nlines;
		     i++, rd.y += lineskip) {
			rd.w = lines[i]->w;
			SDL_SetColorKey(lines[i], 0, 0);
			SDL_BlitSurface(lines[i], NULL, su, &rd);
			SDL_FreeSurface(lines[i]);
		}
		Free(lines, M_TEXT);

		SDL_SetColorKey(su, SDL_SRCCOLORKEY, colorkey);
	}

	free(ucsd);
	return (su);
}

/* Return the expected size of an Unicode text element. */
void
text_prescale_unicode(const Uint32 *ucs, int *w, int *h)
{
	SDL_Surface *su;

	su = text_render_unicode(NULL, -1, 0, ucs);
	if (w != NULL)
		*w = (int)su->w;
	if (h != NULL)
		*h = (int)su->h;
	SDL_FreeSurface(su);
}

/* Return the expected surface size for a UTF-8 string. */
void
text_prescale(const char *text, int *w, int *h)
{
	Uint32 *ucs;

	ucs = unicode_import(UNICODE_FROM_UTF8, text);
	text_prescale_unicode(ucs, w, h);
	free(ucs);
}

/* Display a message. */
void
text_msg(enum text_msg_title title, const char *format, ...)
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
	window_set_caption(win, "%s", _(text_msg_titles[title]));
	window_set_position(win, WINDOW_CENTER, 1);

	vb = vbox_new(win, 0);
	label_new(vb, LABEL_STATIC, msg);

	vb = vbox_new(win, VBOX_HOMOGENOUS|VBOX_WFILL|VBOX_HFILL);
	button = button_new(vb, _("Ok"));
	event_new(button, "button-pushed", window_generic_detach, "%p", win);

	widget_focus(button);
	window_show(win);
}

/*
 * Parse a command-line font specification and set the default font.
 * The format is <face>,<size>,<style>.
 */
void
text_parse_fontspec(char *fontspec)
{
	char *s;

	if ((s = strsep(&fontspec, ":,./")) != NULL &&
	    s[0] != '\0') {
		prop_set_string(config, "font-engine.default-font", s);
	}
	if ((s = strsep(&fontspec, ":,./")) != NULL &&
	    s[0] != '\0') {
		prop_set_int(config, "font-engine.default-size", atoi(s));
	}
	if ((s = strsep(&fontspec, ":,./")) != NULL &&
	    s[0] != '\0') {
		prop_set_int(config, "font-engine.default-style", atoi(s));
	}
}

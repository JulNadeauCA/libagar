/*	$Csoft: text.c,v 1.99 2005/02/08 15:45:38 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/have_freetype.h>

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/config.h>

#ifdef HAVE_FREETYPE
#include <engine/loader/ttf.h>
#endif

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/button.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

int text_composition = 1;		/* Built-in input composition */
int text_rightleft = 0;			/* Right-to-left text display */
int text_font_height = 0;		/* Default font height (px) */
int text_font_ascent = 0;		/* Default font ascent (px) */
int text_font_descent = 0;		/* Default font descent (px) */
int text_font_line_skip = 0;		/* Default font line skip (px) */
int text_tab_width = 40;		/* Tab width (px) */
int text_blink_rate = 250;		/* Cursor blink rate (ms) */

#define TEXT_NBUCKETS 1024

static const char *text_msg_titles[] = {
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

pthread_mutex_t text_lock = PTHREAD_MUTEX_INITIALIZER;
static struct text_font *default_font = NULL;		/* Default font */
static SLIST_HEAD(text_fontq, text_font) text_fonts =	/* Cached fonts */
    SLIST_HEAD_INITIALIZER(&text_fonts);
static struct {
	SLIST_HEAD(, text) texts;
} text_cache[TEXT_NBUCKETS];
static struct timeout text_timeout;		/* Timer for text_tmsg() */

struct text_font *
text_fetch_font(const char *name, int size, int style)
{
	char path[MAXPATHLEN];
	struct text_font *font;
	
	pthread_mutex_lock(&text_lock);
	SLIST_FOREACH(font, &text_fonts, fonts) {
		if (font->size == size &&
		    font->style == style &&
		    strcmp(font->name, name) == 0)
			break;
	}
	if (font != NULL)
		goto out;

	if (config_search_file("font-path", name, NULL, path, sizeof(path))
	    == -1)
		fatal("%s", error_get());
	
	font = Malloc(sizeof(struct text_font), M_TEXT);
	strlcpy(font->name, name, sizeof(font->name));
	font->size = size;
	font->style = style;

#ifdef HAVE_FREETYPE
	dprintf("%s (%d pts)\n", path, size);
	if ((font->p = ttf_open_font(path, size)) == NULL) {
		fatal("%s: %s", path, error_get());
	}
	ttf_set_font_style(font->p, style);
#else
	/* TODO */
#endif

	SLIST_INSERT_HEAD(&text_fonts, font, fonts);
out:
	pthread_mutex_unlock(&text_lock);
	return (font);
}

static Uint32
expire_tmsg(void *obj, Uint32 ival, void *arg)
{
	struct window *win = arg;

	view_detach(win);
	return (0);
}

/* Initialize the font engine and configure the default font. */
int
text_init(void)
{
	int i;

	if (prop_get_bool(config, "font-engine") == 0)
		return (0);

#ifdef HAVE_FREETYPE
	if (ttf_init() == -1) {
		error_set("ttf_init: %s", SDL_GetError());
		return (-1);
	}
	default_font = text_fetch_font(
	    prop_get_string(config, "font-engine.default-font"),
	    prop_get_int(config, "font-engine.default-size"),
	    prop_get_int(config, "font-engine.default-style"));
	text_font_height = ttf_font_height(default_font->p);
	text_font_ascent = ttf_font_ascent(default_font->p);
	text_font_descent = ttf_font_descent(default_font->p);
	text_font_line_skip = ttf_font_line_skip(default_font->p);
#endif

	for (i = 0; i < TEXT_NBUCKETS; i++) {
		SLIST_INIT(&text_cache[i].texts);
	}
	return (0);
}

static void
free_text(struct text *txt)
{
	Free(txt->s, 0);
	SDL_FreeSurface(txt->su);
	Free(txt, M_TEXT);
}

void
text_destroy(void)
{
	struct text_font *fon, *nextfon;
	int i;
	
	for (i = 0; i < TEXT_NBUCKETS; i++) {
		struct text *txt, *ntxt;

		for (txt = SLIST_FIRST(&text_cache[i].texts);
		     txt != SLIST_END(&text_cache[i].texts);
		     txt = ntxt) {
			ntxt = SLIST_NEXT(txt, texts);
			free_text(txt);
		}
		SLIST_INIT(&text_cache[i].texts);
	}
	
	for (fon = SLIST_FIRST(&text_fonts);
	     fon != SLIST_END(&text_fonts);
	     fon = nextfon) {
		nextfon = SLIST_NEXT(fon, fonts);
#ifdef HAVE_FREETYPE
		ttf_close_font(fon->p);
#endif
		Free(fon, M_TEXT);
	}
#ifdef HAVE_FREETYPE
	ttf_destroy();
#endif
}

static __inline__ int
text_hash(const char *s)
{
	unsigned long h;
	const unsigned char *p;

	p = (const unsigned char *)s;
	for (h = 0; *p != '\0'; p++) {
		h = 37*h + *p;
	}
	return (h % TEXT_NBUCKETS);
}

/* Look up the text surface cache. */
struct text *
text_render2(const char *fontname, int fontsize, Uint32 color, const char *s)
{
	struct text *txt;
	int h;

	h = text_hash(s);
	SLIST_FOREACH(txt, &text_cache[h].texts, texts) {
		if (fontsize == txt->fontsize &&
		    color == txt->color &&
		    strcmp(fontname, txt->fontname) == 0 &&
		    strcmp(s, txt->s) == 0)
			break;
	}
	if (txt == NULL) {
		Uint32 *ucs;

		txt = Malloc(sizeof(struct text), M_TEXT);
		strlcpy(txt->fontname, fontname, sizeof(txt->fontname));
		txt->fontsize = fontsize;
		txt->color = color;
		txt->s = Strdup(s);
		txt->nrefs = 1;

		ucs = unicode_import(UNICODE_FROM_UTF8, s);
		txt->su = text_render_unicode(fontname, fontsize, color, ucs);
		free(ucs);
	} else {
		txt->nrefs++;
	}
	return (txt);
}

void
text_unused2(struct text *txt)
{
	if (--txt->nrefs == 0) {
		int h;

		h = text_hash(txt->s);
		SLIST_REMOVE(&text_cache[h].texts, txt, text, texts);
		free_text(txt);
	}
}

/* Render UTF-8 text onto a new surface. */
/* XXX use state variables for font spec */
/* XXX inefficient */
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

#ifdef HAVE_FREETYPE

/* Render (possibly multi-line) UCS-4 text onto a new surface. */
/* TODO use pools to get rid of all the insane allocations */
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

	font = text_fetch_font(fontname != NULL ? fontname :
	    prop_get_string(config, "font-engine.default-font"),
	    fontsize >= 0 ? fontsize :
	    prop_get_int(config, "font-engine.default-size"), 0);
	font_h = ttf_font_height(font->p);

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
		su = SDL_CreateRGBSurface(SDL_SWSURFACE, maxw,
		    lineskip*(nlines+1),
		    vfmt->BitsPerPixel,
		    vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, 0);
		if (su == NULL)
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	
		colorkey = SDL_MapRGB(su->format, 15, 15, 15);
		SDL_FillRect(su, NULL, colorkey);

		for (i = 0, rd.y = lineskip/2;
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

#else /* !HAVE_FREETYPE */

SDL_Surface *
text_render_unicode(const char *fontname, int fontsize, Uint32 color,
    const Uint32 *text)
{
	/* TODO bitmap version */
	return (NULL);
}

#endif /* HAVE_FREETYPE */

/* Return the expected size of an Unicode text element. */
void
text_prescale_unicode(const Uint32 *ucs, int *w, int *h)
{
	SDL_Surface *su;

	/* XXX get the bounding box instead */
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
	struct button *bu;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = window_new(WINDOW_NO_RESIZE|WINDOW_NO_CLOSE|WINDOW_NO_MINIMIZE|
	                 WINDOW_NO_MAXIMIZE|WINDOW_NO_DECORATIONS, NULL);
	window_set_caption(win, "%s", _(text_msg_titles[title]));
	window_set_position(win, WINDOW_CENTER, 1);

	vb = vbox_new(win, 0);
	label_new(vb, LABEL_STATIC, msg);

	vb = vbox_new(win, VBOX_HOMOGENOUS|VBOX_WFILL|VBOX_HFILL);
	bu = button_new(vb, _("Ok"));
	event_new(bu, "button-pushed", window_generic_detach, "%p", win);

	widget_focus(bu);
	window_show(win);
}

/* Display a message for a given period of time. */
void
text_tmsg(enum text_msg_title title, Uint32 expire, const char *format, ...)
{
	char msg[LABEL_MAX];
	struct window *win;
	struct vbox *vb;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = window_new(WINDOW_NO_RESIZE|WINDOW_NO_CLOSE|WINDOW_NO_MINIMIZE|
	                 WINDOW_NO_MAXIMIZE|WINDOW_NO_DECORATIONS, NULL);
	window_set_caption(win, "%s", _(text_msg_titles[title]));
	window_set_position(win, WINDOW_CENTER, 1);

	vb = vbox_new(win, 0);
	label_new(vb, LABEL_STATIC, msg);
	window_show(win);

	lock_timeout(NULL);
	if (timeout_scheduled(NULL, &text_timeout)) {
		view_detach((struct window *)text_timeout.arg);
		timeout_del(NULL, &text_timeout);
	}
	unlock_timeout(NULL);

	timeout_set(&text_timeout, expire_tmsg, win, TIMEOUT_LOADABLE);
	timeout_add(NULL, &text_timeout, expire);
}

/* Prompt the user for a floating-point value. */
void
text_prompt_float(double *fp, double min, double max, const char *unit,
    const char *format, ...)
{
	char msg[LABEL_MAX];
	struct window *win;
	struct vbox *vb;
	va_list args;
	struct button *button;
	struct fspinbutton *fsb;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = window_new(WINDOW_MODAL|WINDOW_NO_VRESIZE, NULL);
	window_set_caption(win, "%s", _("Enter real number"));
	window_set_position(win, WINDOW_CENTER, 1);

	vb = vbox_new(win, VBOX_WFILL);
	label_new(vb, LABEL_STATIC, msg);
	
	vb = vbox_new(win, VBOX_WFILL);
	fsb = fspinbutton_new(vb, unit, _("Number: "));
	WIDGET(fsb)->flags |= WIDGET_WFILL;
	widget_bind(fsb, "value", WIDGET_DOUBLE, fp);
	fspinbutton_set_range(fsb, min, max);
	event_new(fsb, "fspinbutton-return", window_generic_detach, "%p", win);
	
	vb = vbox_new(win, VBOX_HOMOGENOUS|VBOX_WFILL|VBOX_HFILL);
	button = button_new(vb, _("Ok"));
	event_new(button, "button-pushed", window_generic_detach, "%p", win);

	/* TODO test type */

	window_show(win);
	widget_focus(fsb->input);
}

/* Prompt the user for a string. */
void
text_prompt_string(char **sp, size_t len, const char *msgfmt, ...)
{
	char msg[LABEL_MAX];
	struct window *win;
	struct vbox *vb;
	va_list args;
	struct button *button;
	struct textbox *tb;

	va_start(args, msgfmt);
	vsnprintf(msg, sizeof(msg), msgfmt, args);
	va_end(args);

	win = window_new(WINDOW_MODAL|WINDOW_NO_VRESIZE, NULL);
	window_set_caption(win, "%s", _("Enter string"));
	window_set_position(win, WINDOW_CENTER, 1);

	vb = vbox_new(win, VBOX_WFILL);
	label_new(vb, LABEL_STATIC, msg);
	
	vb = vbox_new(win, VBOX_WFILL);

	tb = textbox_new(vb, _("String: "));
	WIDGET(tb)->flags |= WIDGET_WFILL;
	widget_bind(tb, "string", WIDGET_STRING, sp, len);
	event_new(tb, "textbox-return", window_generic_detach, "%p", win);
	
	vb = vbox_new(win, VBOX_HOMOGENOUS|VBOX_WFILL|VBOX_HFILL);
	button = button_new(vb, _("Ok"));
	event_new(button, "button-pushed", window_generic_detach, "%p", win);

	window_show(win);
	widget_focus(tb);
}

/*
 * Parse a command-line font specification and set the default font.
 * The format is <face>,<size>,<style>.
 */
void
text_parse_fontspec(char *fontspec)
{
	char *s;

	if ((s = strsep(&fontspec, ":,/")) != NULL &&
	    s[0] != '\0') {
		prop_set_string(config, "font-engine.default-font", s);
	}
	if ((s = strsep(&fontspec, ":,/")) != NULL &&
	    s[0] != '\0') {
		prop_set_int(config, "font-engine.default-size", atoi(s));
	}
	if ((s = strsep(&fontspec, ":,/")) != NULL &&
	    s[0] != '\0') {
		prop_set_int(config, "font-engine.default-style", atoi(s));
	}
}

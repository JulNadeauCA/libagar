/*	$Csoft: text.c,v 1.38 2002/11/15 00:49:54 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <engine/engine.h>

#include "widget.h"
#include "window.h"
#include "label.h"
#include "button.h"
#include "text.h"
#include "textbox.h"
#include "keycodes.h"

/* XXX prefs */
#define DEFAULT_FONT_NAME	"larabie"
#define DEFAULT_FONT_SIZE	16

TTF_Font *font;		/* Default font */
int font_h;		/* Default font height */

struct text_font {
	char	 *name;
	int	 size;
	int	 style;
	TTF_Font *font;
	SLIST_ENTRY(text_font) fonts;
};

static SLIST_HEAD(text_fontq, text_font) fonts = SLIST_HEAD_INITIALIZER(&fonts);
static pthread_mutex_t fonts_lock = { PTHREAD_MUTEX_INITIALIZER };

static TTF_Font *
get_font(char *name, int size, int style)
{
	char *path;
	TTF_Font *nfont;
	struct text_font *fon;

	if (name == NULL) {
		/* Default font */
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
		fatal("%s.ttf: %s\n", name, error_get());
	}
	nfont = TTF_OpenFont(path, size);
	if (nfont == NULL) {
		fatal("%s: %s\n", path, error_get());
	}
	free(path);

	TTF_SetFontStyle(nfont, style);

	fon = emalloc(sizeof(struct text_font));
	fon->name = strdup(name);
	fon->size = size;
	fon->style = style;
	fon->font = nfont;

	SLIST_INSERT_HEAD(&fonts, fon, fonts);

	pthread_mutex_unlock(&fonts_lock);

	return (nfont);
}

int
text_engine_init(void)
{
	if (TTF_Init() < 0) {
		fatal("TTF_Init: %s\n", SDL_GetError());
		return (-1);
	}

	/* Load the default font. */
	font = get_font(DEFAULT_FONT_NAME, DEFAULT_FONT_SIZE, 0);
	font_h = TTF_FontHeight(font);

	return (0);
}

void
text_engine_destroy(void)
{
	struct text_font *fon, *nextfon;
	
	/* Free glyph cache. */
	keycodes_freeglyphs();

	/* Close the opened fonts. */
	for (fon = SLIST_FIRST(&fonts);
	     fon != SLIST_END(&fonts);
	     fon = nextfon) {
		nextfon = SLIST_NEXT(fon, fonts);
		free(fon->name);
		TTF_CloseFont(fon->font);
		free(nextfon);
	}
	TTF_Quit();
}

SDL_Surface *
text_render(char *fontname, int fontsize, Uint32 color, char *s)
{
	SDL_Surface *su;
	SDL_Color col;
	Uint8 r, g, b;
	TTF_Font *fon;
	SDL_Rect rd;
	int nlines, maxw;
	char *sd, *sp;

#ifdef DEBUG
	if (s == NULL || strcmp("", s) == 0) {
		fatal("empty string\n");
	}
#endif
	/* Get a font handle. */
	fon = get_font(fontname, fontsize, 0);
	
	/* Decompose the color. */
	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	/* Find out the line count. */
	sd = strdup(s);
	for (sp = sd, nlines = 0; *sp != '\0'; sp++) {
		if (*sp == '\n') {
			nlines++;
		}
	}

	if (nlines == 0) {
		/* Render a single line. */
		su = TTF_RenderText_Solid(fon, sd, col);
		if (su == NULL) {
			fatal("TTF_RenderText_Solid: %s\n", error_get());
		}
	} else {
		SDL_Surface **lines, **lp;
		int i;
	
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
			*lp = TTF_RenderText_Solid(fon, sp, col);
			if (*lp == NULL) {
				fatal("TTF_RenderText_Solid: %s\n",
				    error_get());
			}
			if ((*lp)->w > maxw) {
				maxw = (*lp)->w;	/* Grow width */
			}
		}

		rd.x = 0;
		rd.y = 0;
		rd.w = 0;
		rd.h = TTF_FontHeight(fon);

		/* Render the final surface. */
		su = view_surface(SDL_SWSURFACE, maxw, rd.h * nlines);
		for (i = 0;
		     i < nlines;
		     i++, rd.y += rd.h) {
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
text_msg(char *title, char *fmt, ...)
{
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *button;
	va_list args;
	char *msg;

	va_start(args, fmt);
	if (vasprintf(&msg, fmt, args) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
	va_end(args);

	win = window_generic_new(253, 140, "%s", title);
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	lab = label_new(reg, 100, 60, msg);
	button = button_new(reg, "Ok", NULL, 0, 99, 40);
	WIDGET_FOCUS(button);

	event_new(button, "button-pushed", window_generic_detached, "%p", win);
	window_show(win);

	free(msg);
}


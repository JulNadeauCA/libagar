/*	$Csoft: text.c,v 1.25 2002/08/12 06:57:54 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/map.h>

#include "text.h"
#include "widget.h"
#include "window.h"
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
		fatal("%s: %s\n", path, TTF_GetError());
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

/* Called at engine initialization. */
int
text_engine_init(void)
{
	static SDL_Color white = { 255, 255, 255 };
	SDL_Surface *stext;

	if (TTF_Init() < 0) {
		fatal("TTF_Init: %s\n", SDL_GetError());
		return (-1);
	}

	/* Load the default font. */
	font = get_font(DEFAULT_FONT_NAME, DEFAULT_FONT_SIZE, 0);

	/* Stupid hack to obtain the maximum height of the default font. */
	stext = TTF_RenderText_Solid(font, " ", white);
	if (stext == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}
	font_h = stext->h;
	SDL_FreeSurface(stext);

	return (0);
}

/* Called at engine shutdown. */
void
text_engine_destroy(void)
{
	struct text_font *fon, *nextfon;

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

#ifdef DEBUG
	if (s == NULL || strcmp("", s) == 0) {
		fatal("empty string\n");
	}
#endif

	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	col.r = r;
	col.g = g;
	col.b = b;

	fon = get_font(fontname, fontsize, 0);
	su = TTF_RenderText_Solid(fon, s, col);
	if (su == NULL) {
		fatal("TTF_RenderText_Solid: %s\n", TTF_GetError());
	}
	return (su);
}

void
text_msg(char *fmt, ...)
{
	
}


/*	$Csoft: text.c,v 1.18 2002/06/06 10:18:02 vedge Exp $	*/

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

/* XXX obsolete interface, move to widgets. */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/map.h>

#include "text.h"
#include "window.h"
#include "widget.h"
#include "textbox.h"
#include "keycodes.h"

static const struct object_ops text_ops = {
	text_destroy,
	NULL,		/* load */
	NULL		/* save */
};

static TAILQ_HEAD(, text) textsh = TAILQ_HEAD_INITIALIZER(textsh);
static pthread_mutex_t textslock = PTHREAD_MUTEX_INITIALIZER;

static int gx = 0, gy = 0;	/* Current cascading coordinates */
static int ntexts = 0;		/* Concurrent text window count */
static int maxfonth;		/* Maximum font height */

/* XXX prefs */
#define FONTNAME	"larabie"
#define FONTSIZE	16
#define XMARGIN		8
#define YMARGIN		8
#define LINESPACE	0
#define TIMEGRANUL	1000

TTF_Font *font;
static SDL_Color white = { 255, 255, 255 };

static Uint32	 text_tick(Uint32, void *);
static void	 text_renderbg(SDL_Surface *, SDL_Rect *);

/* Called at engine initialization. */
int
text_engine_init(void)
{
	char *path;
	SDL_Surface *stext;

	if (TTF_Init() < 0) {
		fatal("TTF_Init: %s\n", SDL_GetError());
		return (-1);
	}

	path = object_path(FONTNAME, "ttf");
	if (path == NULL) {
		fatal("%s.ttf: %s\n", FONTNAME, AGAR_GetError());
	}

	font = TTF_OpenFont(path, FONTSIZE);	/* XXX pref */
	if (font == NULL) {
		fatal("%s: %s\n", path, TTF_GetError());
		return (-1);
	}
	
	TTF_SetFontStyle(font, 0);
	
	/* Stupid hack to obtain the maximum line height. */
	stext = TTF_RenderText_Solid(font, " ", white);
	if (stext == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}
	maxfonth = stext->h;
	SDL_FreeSurface(stext);

	SDL_AddTimer(TIMEGRANUL, text_tick, NULL);

	return (0);
}

/* Called at engine shutdown. */
void
text_engine_destroy(void)
{
	if (font != NULL) {
		TTF_CloseFont(font);
		font = NULL;
	}
	TTF_Quit();
}

void
text_init(struct text *te, Sint16 x, Sint16 y, Uint16 w, Uint16 h,
    Uint32 flags, Uint8 sleepms)
{
	static int textid = 0;
	char textname[128];

#ifdef DEBUG
	if ((flags & TEXT_DEBUG) && !engine_debug) {
		return;
	}
#endif

	sprintf(textname, "text%d\n", textid++);
	object_init(&te->obj, "text-dialog", textname, NULL, 0, &text_ops);
	te->flags = flags;

	te->sleepms = sleepms;
	te->w = w;
	te->h = h;
	te->x = x > 0 ? x : gx;
	te->y = y > 0 ? y : gy;
	te->tx = XMARGIN;
	te->ty = YMARGIN;
	te->nlines = te->h / maxfonth;
	te->bgcolor = SDL_MapRGB(mainview->v->format, 30, 90, 180);
	te->fgcolor = &white;
	te->view = mainview;
	te->v = mainview->v;
	te->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
	    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if (te->surface == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
	}

	/* Prevent overlapping blits. */
	te->mvmask.x = (te->x / TILEW) - te->view->mapxoffs;
	te->mvmask.y = (te->y / TILEH) - te->view->mapyoffs;
	te->mvmask.w = (te->w / TILEW);
	te->mvmask.h = (te->h / TILEH);
}

void
text_destroy(void *p)
{
	struct text *te = (struct text *)p;

	if (te->surface != NULL) {
		SDL_FreeSurface(te->surface);
	}
	
}

void
text_destroyall(void)
{
	struct text *te;

	pthread_mutex_lock(&textslock);
	TAILQ_FOREACH(te, &textsh, texts) {
		te->sleepms = -1;
		/* XXX what about event dialogs? */
	}
	pthread_mutex_unlock(&textslock);

	text_tick(0, te);
}

static void
text_attached(struct text *te)
{
	pthread_mutex_lock(&textslock);
	TAILQ_INSERT_TAIL(&textsh, te, texts);
	pthread_mutex_unlock(&textslock);
	
	ntexts++;

	view_maskfill(te->view, &te->mvmask, 1);
}

static void
text_detached(struct text *te)
{
	ntexts--;

	pthread_mutex_lock(&textslock);
	TAILQ_REMOVE(&textsh, te, texts);
	pthread_mutex_unlock(&textslock);
	
	view_maskfill(te->view, &te->mvmask, -1);
	if (te->view->map != NULL) {
		te->view->map->redraw++;
	}
}

static void
text_renderbg(SDL_Surface *v, SDL_Rect *rd)
{
	static Uint32 col;
	static Uint32 border[5];
	Uint8 *dst = v->pixels;
	Uint32 xo, yo;

	/* XXX waste */
	border[0] = SDL_MapRGB(v->format, 50, 50, 50);
	border[1] = SDL_MapRGB(v->format, 100, 100, 160);
	border[2] = SDL_MapRGB(v->format, 192, 192, 192);
	border[3] = SDL_MapRGB(v->format, 100, 100, 160);
	border[4] = SDL_MapRGB(v->format, 50, 50, 50);

	SDL_LockSurface(v);
	for (yo = 0; yo < rd->h; yo++) {
		for (xo = 0; xo < rd->w; xo++) {
			static Uint32 y, x;
		
			y = rd->y + yo;
			x = rd->x + xo;

			if (xo > rd->w - 4) {
				col = border[rd->w - xo];
			} else if (yo < 4) {
				col = border[yo+1];
			} else if (xo < 4) {
				col = border[xo+1];
			} else if (yo > rd->h - 4) {
				col = border[rd->h - yo];
			} else {
				/* XXX pref */
				col = SDL_MapRGBA(v->format, yo, 0, xo >> 2,
				    200);
			}
		
			switch (v->format->BytesPerPixel) {
			case 1:
				dst[x] = col;
				break;
			case 2:
				((Uint16 *)dst)[x] = col;
				break;
			case 3:
				if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
					dst[x*3] = col;
					dst[x*3 + 1] = col>>8;
					dst[x*3 + 2] = col>>16;
				} else {
					dst[x*3] = col>>16;
					dst[x*3 + 1] = col>>8;
					dst[x*3 + 2] = col;
				}
				break;
			case 4:
				((Uint32 *)dst)[x] = col;
				break;
			}
		}
		dst += v->pitch;
	}
	SDL_UnlockSurface(v);
}

/* Render the window background. */
void
text_clear(struct text *te)
{
	SDL_Rect rd = { 0, 0, te->w, te->h };

	if ((te->flags & TEXT_TRANSPARENT) == 0) {
		text_renderbg(te->surface, &rd);
	}
}

/* Draw all text windows. */
void
text_drawall(void)
{
	struct text *te;

	pthread_mutex_lock(&textslock);
	TAILQ_FOREACH(te, &textsh, texts) {
		SDL_Rect rd;

		rd.x = te->x;
		rd.y = te->y;
		rd.w = te->surface->w;
		rd.h = te->surface->h;

		SDL_BlitSurface(te->surface, NULL, te->v, &rd);
		SDL_UpdateRect(te->v, rd.x, rd.y, rd.w, rd.h);
	}
	pthread_mutex_unlock(&textslock);
}

static Uint32
text_tick(Uint32 ival, void *p)
{
	struct text *textgc[ntexts];
	struct text *te;
	int i, ntextgc = 0;

	pthread_mutex_lock(&textslock);
	TAILQ_FOREACH(te, &textsh, texts) {
		if (te->flags & TEXT_SLEEP && --te->sleepms < 1) {
			textgc[ntextgc++] = te;
		}
	}
	pthread_mutex_unlock(&textslock);

	for (i = 0; i < ntextgc; i++) {
		te = textgc[i];
		text_detached(te);
	}

	return (ival);
}

void
text_render(struct text *te, char *text)
{
	SDL_Rect rd;
	SDL_Surface *stext;
	char *s, *sp, *textp;

	s = strdup(text);
	sp = s;
	textp = s;
	for (sp = s; *sp != '\0'; sp++) {
		if (*sp == '\n') {
			*sp = '\0';
			stext = TTF_RenderText_Solid(font, textp, *te->fgcolor);
			if (stext == NULL) {
				fatal("TTF_RenderTextSolid: %s\n",
				    SDL_GetError());
			}
			rd.x = te->tx;
			rd.y = te->ty;
			rd.w = stext->w;
			rd.h = stext->h;
			te->tx += rd.w;

			SDL_BlitSurface(stext, NULL, te->surface, &rd);
			SDL_FreeSurface(stext);

			textp = sp + 1;
			te->ty += maxfonth + LINESPACE;
			te->tx = XMARGIN;
		}
	}
	free(s);
}

void
text_printf(struct text *te, char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
	va_end(args);

	text_render(te, buf);
	free(buf);

	text_drawall();
}

/*
 * Display a single message in an auto-sized window at position
 * given by the cascading algorithm.
 */
void
text_msg(Uint8 delay, Uint32 flags, char *fmt, ...)
{
	va_list args;
	char *buf, *bufc, *s, *last;
	struct text *te;
	int w = 0, h = 0, nlines, longlinew, cycle;
	
	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
	va_end(args);

	bufc = strdup(buf);
	for (s = strtok_r(bufc, "\n", &last), nlines = 0, longlinew = 0;
	     s != NULL;
	     s = strtok_r(NULL, "\n", &last), nlines++) {
		SDL_Surface *sline;

		sline = TTF_RenderText_Solid(font, s, white);
		if (sline == NULL) {
			fatal("TTF_RenderTextSolid: %s\n", TTF_GetError());
		}
		if (sline->w > longlinew) {
			longlinew = sline->w;
		}
		SDL_FreeSurface(sline);
	}
	free(bufc);

	/* Adjust the geometry with the map, for optimization purposes. */
	while (h - YMARGIN < (nlines * maxfonth))
		h += TILEH;
	while (w - XMARGIN < longlinew + XMARGIN)
		w += TILEW;

	cycle = 1;
	TAILQ_FOREACH(te, &textsh, texts) {
		if (te->y <= TILEH << 1) {
			cycle = 0;
		}
	}
	if (cycle) {
		gx = TILEW * 2;
		gy = TILEH << 1;
	} else {
		gx += TILEW;
		gy += TILEH;
	}
	if ((gy + h) >= mainview->h)
		gy = TILEH;
	if ((gx + w) >= mainview->w)
		gx = TILEW;

	te = emalloc(sizeof(struct text));
	text_init(te, gx, gy, w, h, flags, delay);
	if (te != NULL) {
		text_attached(te);

		text_clear(te);
		text_render(te, buf);
		text_drawall();
	}

	free(buf);
}

/*	$Csoft: window.c,v 1.2 2002/04/18 04:03:59 vedge Exp $	*/

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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/map.h>
#include <engine/version.h>

#include "widget.h"
#include "window.h"

static struct obvec window_vec = {
	window_destroy,
	window_load,
	window_save,
	window_link,
	window_unlink
};

static TAILQ_HEAD(, window) windowsh = TAILQ_HEAD_INITIALIZER(windowsh);
static pthread_mutex_t windowslock = PTHREAD_MUTEX_INITIALIZER;

static SDL_TimerID timer = NULL;
static Uint32 delta = 0;

static Uint32	window_time(Uint32, void *);

struct window *
window_create(struct viewport *view, char *name, char *caption, Uint32 flags,
    enum window_bg bgtype, SDL_Rect rect, Uint32 *bgcolor, Uint32 *fgcolor)
{
	struct window *w;

	w = (struct window *)emalloc(sizeof(struct window));
	object_init(&w->obj, name, 0, &window_vec);

	w->caption = strdup(caption);

	w->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h,
	    view->depth,
	    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if (w->surface == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
	}

	w->view = view;
	w->flags = flags;
	w->bgtype = bgtype;
	w->rect = rect;
	w->bgcolor = bgcolor;
	w->fgcolor = fgcolor;

	w->vmask.x = (rect.x / view->map->tilew) - view->mapxoffs;
	w->vmask.y = (rect.y / view->map->tileh) - view->mapyoffs;
	w->vmask.w = (rect.w / view->map->tilew);
	w->vmask.h = (rect.h / view->map->tilew);

	view_maskfill(view, &w->vmask, 1);

	/* XXX pref */
	w->border[0] = SDL_MapRGB(view->v->format, 50, 50, 50);
	w->border[1] = SDL_MapRGB(view->v->format, 100, 100, 160);
	w->border[2] = SDL_MapRGB(view->v->format, 192, 192, 192);
	w->border[3] = SDL_MapRGB(view->v->format, 100, 100, 160);
	w->border[4] = SDL_MapRGB(view->v->format, 50, 50, 50);
	
	TAILQ_INIT(&w->widgetsh);

	if (pthread_mutex_init(&w->widgetslock, NULL) != 0) {
		perror("widgetslock");
		return (NULL);
	}

	return (w);
}

int
window_init(void)
{
	return (0);
}

void
window_quit(void)
{
	if (timer != NULL) {
		SDL_RemoveTimer(timer);
	}
}

static Uint32
window_time(Uint32 ival, void *wp)
{
	if (delta++ > 32767) {
		delta= 0;
	}

	if (wp != NULL) {
		WINDOW(wp)->view->map->redraw++;
	}
	
	return (ival);
}

void
window_drawall(void)
{
	struct window *w;

	pthread_mutex_lock(&windowslock);
	TAILQ_FOREACH(w, &windowsh, windows) {
		window_draw(w);
	}
	pthread_mutex_unlock(&windowslock);
}

void
window_draw(struct window *w)
{
	SDL_Surface *v = w->view->v;
	Uint32 xo, yo, col;
	Uint8 *dst;
	struct widget *wid;

	/* Render the background. */
	if (w->flags & WINDOW_PLAIN) {
		SDL_FillRect(v, &w->rect, *w->bgcolor);
	} else {
		SDL_LockSurface(v);
		for (yo = 0; yo < w->rect.h; yo++) {
			for (xo = 0; xo < w->rect.w; xo++) {
				dst = (Uint8 *)v->pixels + (w->rect.y+yo) *
				    v->pitch + (w->rect.x+xo) *
				    v->format->BytesPerPixel;

				if (xo > w->rect.w - 4) {
					col = w->border[w->rect.w - xo];
				} else if (yo < 4) {
					col = w->border[yo+1];
				} else if (xo < 4) {
					col = w->border[xo+1];
				} else if (yo > w->rect.h - 4) {
					col = w->border[w->rect.h - yo];
				} else {
					static Uint8 expcom;
			
					switch (w->bgtype) {
					case WINDOW_SOLID:
						col = *w->bgcolor;
						break;
					case WINDOW_GRADIENT:
						col = SDL_MapRGBA(v->format,
						    yo >> 2, 0, xo >> 2, 200);
						break;
					case WINDOW_CUBIC:
						expcom =
						    ((yo+delta)^(xo-delta))+
						    delta;
						if (expcom > 150)
							expcom -= 140;
						col = SDL_MapRGBA(v->format,
						    0, expcom, 0, 200);
						break;
					default:
						break;
					}
				}
				switch (v->format->BytesPerPixel) {
				case 1:
					*dst = col;
					break;
				case 2:
					*(Uint16 *)dst = col;
					break;
				case 3:
					if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
						dst[0] = (col>>16) & 0xff;
						dst[1] = (col>>8) & 0xff;
						dst[2] = col & 0xff;
					} else {
						dst[0] = col & 0xff;
						dst[1] = (col>>8) & 0xff;
						dst[2] = (col>>16) & 0xff;
					}
					break;
				case 4:
					*((Uint32 *)dst) = col;
					break;
				}
			}
		}
		SDL_UnlockSurface(v);
	}

	/* Render the widgets. */
	pthread_mutex_lock(&w->widgetslock);
	TAILQ_FOREACH(wid, &w->widgetsh, widgets) {
		widget_draw(wid);
	}
	pthread_mutex_unlock(&w->widgetslock);

	SDL_UpdateRect(v, w->rect.x, w->rect.y, w->rect.w, w->rect.h);
}

int
window_load(void *p, int fd)
{
	struct window *w = (struct window *)p;

	if (version_read(fd, "agar window", 1, 0) != 0) {
		return (-1);
	}
	
	w->flags = fobj_read_uint32(fd);
	w->caption = fobj_read_string(fd);
	*w->bgcolor = fobj_read_uint32(fd);
	*w->fgcolor = fobj_read_uint32(fd);
	w->rect = fobj_read_rect(fd);

	return (0);
}

int
window_save(void *p, int fd)
{
	struct window *w = (struct window *)p;

	version_write(fd, "agar window", 1, 0);

	fobj_write_uint32(fd, w->flags);
	fobj_write_string(fd, w->caption);
	fobj_write_uint32(fd, *w->bgcolor);
	fobj_write_uint32(fd, *w->fgcolor);
	fobj_write_rect(fd, w->rect);

	return (0);
}

int
window_link(void *ob)
{
	struct window *w = (struct window *)ob;

	pthread_mutex_lock(&windowslock);

	TAILQ_INSERT_HEAD(&windowsh, w, windows);

	if (timer != NULL) {
		SDL_RemoveTimer(timer);	
	}
	timer = SDL_AddTimer(100, window_time, w);
	
	pthread_mutex_unlock(&windowslock);

	return (0);
}

int
window_unlink(void *ob)
{
	struct window *w = (struct window *)ob;
	
	pthread_mutex_lock(&windowslock);
	TAILQ_REMOVE(&windowsh, w, windows);
	pthread_mutex_unlock(&windowslock);

	return (0);
}

int
window_destroy(void *ob)
{
	struct window *w = (struct window *)ob;
	struct widget *wid;
	
	pthread_mutex_lock(&w->widgetslock);
	TAILQ_FOREACH(wid, &w->widgetsh, widgets) {
		dprintf("destroy widget %s\n", OBJECT(wid)->name);
		object_unlink(wid);
		object_destroy(wid);
	}
	pthread_mutex_unlock(&w->widgetslock);

	SDL_FreeSurface(w->surface);
	free(w->caption);
	
	view_maskfill(w->view, &w->vmask, -1);
	w->view->map->redraw++;
	
	pthread_mutex_destroy(&w->widgetslock);

	return (0);
}


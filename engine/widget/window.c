/*	$Csoft: window.c,v 1.13 2002/04/28 14:11:23 vedge Exp $	*/

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

static const struct obvec window_ops = {
	window_destroy,
	NULL,
	NULL,
	window_link,
	window_unlink
};

/* This list shares world->lock. */
TAILQ_HEAD(, window) windowsh = TAILQ_HEAD_INITIALIZER(windowsh);

/* Specific garbage collector for widgets. Used by widget.c. */
TAILQ_HEAD(, widget) uwidgetsh = TAILQ_HEAD_INITIALIZER(uwidgetsh);
pthread_mutex_t uwidgets_lock = PTHREAD_MUTEX_INITIALIZER;

static Uint32 delta = 0, delta2 = 256;

static void	 window_unlink_queued(void);

/* XXX fucking insane */
struct window *
window_new(struct viewport *view, char *caption, Uint32 flags, Uint32 bgtype,
    Sint16 x, Sint16 y, Uint16 w, Uint16 h) {
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, view, caption, flags, bgtype, x, y, w, h);

	pthread_mutex_lock(&world->lock);
	object_link(win);
	pthread_mutex_unlock(&world->lock);
	return (win);
}

/* XXX fucking insane */
void
window_init(struct window *win, struct viewport *view, char *caption,
    Uint32 flags, Uint32 bgtype, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	object_init(&win->obj, "window", NULL, 0, &window_ops);

	win->caption = strdup(caption);
	win->view = view;
	win->flags = flags;
	win->bgtype = bgtype;

	switch (win->bgtype) {
	case WINDOW_CUBIC:
	case WINDOW_CUBIC2:
		win->flags |= WINDOW_ANIMATE;
		break;
	}

	win->x = x;
	win->y = y;
	win->w = w;
	win->h = h;
	win->redraw = 0;
	/* XXX pref */
	win->bgcolor = SDL_MapRGBA(view->v->format, 0, 50, 30, 250);
	win->fgcolor = SDL_MapRGBA(view->v->format, 200, 200, 200, 100);

	win->vmask.x = (x / view->map->tilew) - view->mapxoffs;
	win->vmask.y = (y / view->map->tileh) - view->mapyoffs;
	win->vmask.w = (w / view->map->tilew);
	win->vmask.h = (h / view->map->tilew);

	/* XXX pref */
	win->border[0] = SDL_MapRGB(view->v->format, 50, 50, 50);
	win->border[1] = SDL_MapRGB(view->v->format, 100, 100, 160);
	win->border[2] = SDL_MapRGB(view->v->format, 192, 192, 192);
	win->border[3] = SDL_MapRGB(view->v->format, 100, 100, 160);
	win->border[4] = SDL_MapRGB(view->v->format, 50, 50, 50);
	
	TAILQ_INIT(&win->widgetsh);

	pthread_mutex_init(&win->lock, NULL);
}

void
window_mouse_motion(SDL_Event *ev)
{
}

void
window_key(SDL_Event *ev)
{
}

/*
 * Called by event handlers, once the widget list traversals are
 * complete and widgets can be freed.
 */
static void
window_unlink_queued(void)
{
	struct widget *w, *nextw;

	/* Perform deferred unlink operations. */
	pthread_mutex_lock(&uwidgets_lock);
	for (w = TAILQ_FIRST(&uwidgetsh);
	     w != TAILQ_END(&uwidgetsh);
	     w = nextw) {
		nextw = TAILQ_NEXT(w, uwidgets);
		TAILQ_REMOVE(&uwidgetsh, w, widgets);
		free(w);
	}
	TAILQ_INIT(&uwidgetsh);
	pthread_mutex_unlock(&uwidgets_lock);
}

/* Must be called when win->lock is held. */
void
window_mouse_button(SDL_Event *ev)
{
	struct window *win;
	struct widget *w = NULL;

	pthread_mutex_lock(&world->lock);
	TAILQ_FOREACH(win, &windowsh, windows) {
		if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
			continue;
		}

		TAILQ_FOREACH(w, &win->widgetsh, widgets) {
			pthread_mutex_lock(&win->lock);
			if (WIDGET_INSIDE(w, ev->button.x, ev->button.y)) {
				widget_event(w, ev, 0);
			}
			pthread_mutex_unlock(&win->lock);
		}
		break;
	}
	pthread_mutex_unlock(&world->lock);

	/* Garbage collect any unlinked widget. */
	window_unlink_queued();
}

void
window_draw(struct window *win)
{
	SDL_Surface *v = win->view->v;
	Uint32 xo, yo, col = 0;
	Uint8 *dst;
	struct widget *wid;

	/* Render the background. */
	if (win->flags & WINDOW_PLAIN) {
		SDL_Rect rd;

		rd.x = win->x;
		rd.y = win->y;
		rd.w = win->w;
		rd.h = win->h;
		
		SDL_FillRect(v, &rd, win->bgcolor);
	} else {
		SDL_LockSurface(v);
		for (yo = 0; yo < win->h; yo++) {
			for (xo = 0; xo < win->w; xo++) {
				dst = (Uint8 *)v->pixels + (win->y + yo) *
				    v->pitch + (win->x+xo) *
				    v->format->BytesPerPixel;

				if (xo > win->w - 4) {
					col = win->border[win->w - xo];
				} else if (yo < 4) {
					col = win->border[yo+1];
				} else if (xo < 4) {
					col = win->border[xo+1];
				} else if (yo > win->h - 4) {
					col = win->border[win->h - yo];
				} else {
					static Uint8 expcom;
			
					switch (win->bgtype) {
					case WINDOW_SOLID:
						col = win->bgcolor;
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
					case WINDOW_CUBIC2:
						expcom =
						    ((yo+delta)^(xo-delta))+
						    delta;
						if (expcom > 150) {
							expcom -= 140;
						}
						col = SDL_MapRGBA(v->format,
						    0, 0, expcom, 0);
						col *= (yo*xo);
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
	pthread_mutex_lock(&win->lock);
	TAILQ_FOREACH(wid, &win->widgetsh, widgets) {
		widget_draw(wid);
	}
	pthread_mutex_unlock(&win->lock);

	SDL_UpdateRect(v, win->x, win->y, win->w, win->h);

	if (win->flags & WINDOW_ANIMATE) {
		if (delta++ > 256) {
			delta = 0;
		}
		if (delta2-- < 1) {
			delta2 = 256;
		}
		win->redraw++;
	} else {
		win->redraw = 0;
	}
}

/* Must be called when win->lock is held. */
int
window_link(void *ob)
{
	struct window *win = (struct window *)ob;

	/* Increment the view mask for this area. */
	view_maskfill(win->view, &win->vmask, 1);

	/* Shares world->lock, which we assume is held. */
	TAILQ_INSERT_HEAD(&windowsh, win, windows);

	win->redraw++;
	
	return (0);
}

/*
 * Must be called when win->lock is held.
 * Assuming we are called in event context, queued unlink
 * operations will be performed later.
 */
int
window_unlink(void *ob)
{
	struct window *win = (struct window *)ob;
	struct widget *wid;

	/*
	 * Unlink widgets, they are not freed until the event loop is
	 * done traversing the window list.
	 */
	TAILQ_FOREACH(wid, &win->widgetsh, widgets) {
		widget_unlink(wid);
	}

	/* Remove from the window list. */
	TAILQ_REMOVE(&windowsh, win, windows);
	
	/* Decrement the view mask for this area. */
	view_maskfill(win->view, &win->vmask, -1);
	if (win->view->map != NULL) {
		win->view->map->redraw++;
	}
	
	return (0);
}

void
window_destroy(void *p)
{
	struct window *win = (struct window *)p;

	free(win->caption);
	pthread_mutex_destroy(&win->lock);
}


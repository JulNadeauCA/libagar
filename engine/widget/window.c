/*	$Csoft: window.c,v 1.16 2002/04/30 08:19:42 vedge Exp $	*/

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

#include "text.h"
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
TAILQ_HEAD(windows_head, window) windowsh = TAILQ_HEAD_INITIALIZER(windowsh);

/* Specific garbage collector for widgets. Used by widget.c. */
TAILQ_HEAD(, widget) uwidgetsh = TAILQ_HEAD_INITIALIZER(uwidgetsh);
pthread_mutex_t uwidgets_lock = PTHREAD_MUTEX_INITIALIZER;

static Uint32 delta = 0, delta2 = 256;

static void	 window_unlink_queued(void);

/* XXX fucking insane */
struct window *
window_new(struct viewport *view, char *caption, Uint32 flags,
    window_type_t type, Sint16 x, Sint16 y, Uint16 w, Uint16 h) {
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, view, caption, flags, type, x, y, w, h);

	pthread_mutex_lock(&world->lock);
	object_link(win);
	pthread_mutex_unlock(&world->lock);
	return (win);
}

/* XXX fucking insane */
void
window_init(struct window *win, struct viewport *view, char *caption,
    Uint32 flags, window_type_t type, Sint16 x, Sint16 y, Uint16 w, Uint16 h)
{
	static int nwindow = 0;
	struct winseg *nseg;
	char *name;

	name = object_name("window", nwindow++);
	object_init(&win->obj, name, NULL, 0, &window_ops);
	free(name);

	win->caption = strdup(caption);
	win->view = view;
	win->flags = flags;
	win->type = type;

	switch (win->type) {
	case WINDOW_CUBIC:
	case WINDOW_CUBIC2:
		win->flags |= WINDOW_ANIMATE;
		break;
	default:
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
	dprintrect("vmask", &win->vmask);

	/* XXX pref */
	win->border[0] = SDL_MapRGB(view->v->format, 50, 50, 50);
	win->border[1] = SDL_MapRGB(view->v->format, 100, 100, 160);
	win->border[2] = SDL_MapRGB(view->v->format, 192, 192, 192);
	win->border[3] = SDL_MapRGB(view->v->format, 100, 100, 160);
	win->border[4] = SDL_MapRGB(view->v->format, 50, 50, 50);
	
	TAILQ_INIT(&win->widgetsh);
	SLIST_INIT(&win->winsegsh);

	pthread_mutex_init(&win->lock, NULL);

	/* Allocate a segment covering the whole window. */
	pthread_mutex_lock(&win->lock);
	nseg = winseg_new(win, NULL, WINSEG_HORIZ, 0);
	pthread_mutex_unlock(&win->lock);
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
	TAILQ_FOREACH_REVERSE(win, &windowsh, windows, windows_head) {
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

#define WINDOW_CORNER(xo, yo) do {			\
	int z;						\
	z = ((xo) * (yo))/2;				\
	if (z < 2) {					\
		return (-1);				\
	} else {					\
		*col = win->border[((xo) * (yo))/2];	\
		return (1);				\
	}						\
} while (/*CONSTCOND*/0)

static __inline__ int
window_decoration(struct window *win, int xo, int yo, Uint32 *col)
{
	if (xo < 4 && yo < 4) {
		/* Upper left */
		WINDOW_CORNER(xo, yo);
	} else if (yo < 4 && xo > win->w - 4) {
		/* Upper right */
		WINDOW_CORNER(win->w - xo, yo);
	} else if (xo > win->w - 4 && yo > win->h - 4) {
		/* Lower right */
		WINDOW_CORNER(win->w - xo, win->h - yo);
	} else if (xo < 4 && yo > win->h - 4) {
		/* Lower left */
		WINDOW_CORNER(xo, win->h - yo);
	}

	if (win->flags & WINDOW_TITLEBAR) {
		if (yo < 24 && xo > 4 && xo < win->w - 4) {
			if (yo < 4) {
				*col = win->border[yo+1];
			} else if (yo > 20) {
				*col = win->border[yo-20];
			} else {
				switch (win->type) {
				case WINDOW_CUBIC:
				case WINDOW_CUBIC2:
					*col = SDL_MapRGB(win->view->v->format,
					    0, xo>>3, 0);
					break;
				case WINDOW_GRADIENT:
					*col = SDL_MapRGB(win->view->v->format,
					    xo>>5, 0, xo>>2);
					break;
				case WINDOW_SOLID:
					break;
				}
			}
			return (1);
		}
	}
	
	if (xo > win->w - 4) {
		*col = win->border[win->w - xo];
		return (1);
	} else if (yo < 4) {
		*col = win->border[yo+1];
		return (1);
	} else if (xo < 4) {
		*col = win->border[xo+1];
		return (1);
	} else if (yo > win->h - 4) {
		*col = win->border[win->h - yo];
		return (1);
	}
	return (0);
}

void
window_draw(struct window *win)
{
	SDL_Surface *v = win->view->v;
	Uint32 xo, yo, col = 0;
	Uint8 *dst, expcom;
	struct widget *wid;
	int rv;

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

				rv = window_decoration(win, xo, yo, &col);
				if (rv < 0) {
					continue;
				} else if (rv == 0) {
					switch (win->type) {
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

	/* Render the title bar. */
	/* XXX ridiculously inefficient with animated windows. */
	if (win->flags & WINDOW_TITLEBAR) {
		static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */
		SDL_Surface *s;
		SDL_Rect rd;

		s = TTF_RenderText_Solid(font, win->caption, white);
		if (s == NULL) {
			fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
		}
		rd.x = win->x + (win->w - s->w - 6); /* XXX border width */
		rd.y = win->y + 4;
		rd.w = s->w;
		rd.h = s->h;
		SDL_BlitSurface(s, NULL, win->view->v, &rd);
		SDL_FreeSurface(s);
	}

	/* Render the widgets. */
	pthread_mutex_lock(&win->lock);
	TAILQ_FOREACH(wid, &win->widgetsh, widgets) {
		widget_draw(wid);
	}
	pthread_mutex_unlock(&win->lock);

	SDL_UpdateRect(v, win->x, win->y, win->w, win->h);
	
	/* Always redraw if this is an animated window. */
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
	struct winseg *seg, *nextseg = NULL;

	/* Free segments. */
	pthread_mutex_lock(&win->lock);
	for (seg = SLIST_FIRST(&win->winsegsh);
	     seg != SLIST_END(&win->winsegsh);
	     seg = nextseg) {
		nextseg = SLIST_NEXT(seg, winsegs);
		dprintf("free seg\n");
		free(seg);
	}
	pthread_mutex_unlock(&win->lock);
	pthread_mutex_destroy(&win->lock);
	
	free(win->caption);
}

struct winseg *
winseg_new(struct window *pwin, struct winseg *pseg, window_seg_t type, int req)
{
	struct winseg *seg;

	seg = emalloc(sizeof(struct winseg));
	seg->pwin = pwin != NULL ? pwin : pseg->pwin;
	seg->pseg = pseg;
	seg->req = req;
	seg->type = type;
	seg->x = -1;
	seg->y = -1;
	seg->w = -1;
	seg->h = -1;

	dprintf("parent %s\n", OBJECT(pwin)->name);

#if 0
	if (req == 0) {		/* Root segment */
		dprintf("root seg\n");
		seg->x = 0;
		seg->y = 0;
		seg->w = pwin->w;
		seg->h = pwin->h;
	} else {
		seg->x = pseg->x;
		seg->y = pseg->y;
	}
#endif

	return (seg);
}

void
winseg_destroy(struct winseg *seg)
{
	free(seg);
}

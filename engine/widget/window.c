/*	$Csoft: window.c,v 1.29 2002/05/25 08:48:20 vedge Exp $	*/

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

static const struct object_ops window_ops = {
	window_destroy,
	NULL,		/* load */
	NULL,		/* save */
	window_onattach,
	window_ondetach,
	window_attach,
	window_detach
};

/* XXX struct */
#include "borders/grey10.h"

static Uint32 delta = 0, delta2 = 256;

static void	post_widget_ev(void *, void *, void *);

/* XXX fucking insane */
struct window *
window_new(char *caption, int flags, enum window_type type, int x, int y,
    int w, int h) {
	struct window *win;

	flags |= WINDOW_ROUNDEDGES;	/* XXX pref */

	win = emalloc(sizeof(struct window));
	window_init(win, mainview, caption, flags, type, x, y, w, h);

	/* Attach window to main view. */
	view_attach(mainview, win);

	return (win);
}

/* XXX fucking insane */
void
window_init(struct window *win, struct viewport *view, char *caption,
    int flags, enum window_type type, int rx, int ry, int rw, int rh)
{
	static int nwindow = 0;
	char *name;
	int i;

	name = object_name("window", nwindow++);
	object_init(&win->obj, "window", name, NULL, 0, &window_ops);
	free(name);
	
	/* XXX pref */
	win->borderw = default_nborder;
	win->border = emalloc(win->borderw * sizeof(Uint32));
	for (i = 0; i < win->borderw; i++) {
		win->border[i] = SDL_MapRGB(view->v->format,
		    default_border[i].r, default_border[i].g,
		    default_border[i].b);
	}
	
	win->titleh = 18 + win->borderw;	/* XXX font */

	win->caption = strdup(caption);
	win->view = view;
	win->flags = flags;
	win->type = type;
	win->spacing = 4;
	win->redraw = 0;
	win->focus = NULL;

	for (win->x = 0; win->x < (rx * view->w / 100);
	     win->x += view->map->tilew) ;;
	for (win->y = 0; win->y < (ry * view->h / 100);
	     win->y += view->map->tileh) ;;
	for (win->w = 0; win->w < (rw * view->w / 100);
	     win->w += view->map->tilew) ;;
	for (win->h = 0; win->h < (rh * view->h / 100);
	     win->h += view->map->tileh) ;;

	win->body.x = win->x + win->borderw;
	win->body.y = win->y + win->borderw*2 + win->titleh;
	win->body.w = win->w - win->borderw*2;
	win->body.h = win->h - win->borderw*2 - win->titleh;
	
	switch (win->type) {
	case WINDOW_CUBIC:
	case WINDOW_CUBIC2:
		win->flags |= WINDOW_ANIMATE;
		break;
	default:
		break;
	}

	/* XXX pref */
	win->bgcolor = SDL_MapRGBA(view->v->format, 0, 50, 30, 250);
	win->fgcolor = SDL_MapRGBA(view->v->format, 200, 200, 200, 100);

	win->vmask.x = (win->x / view->map->tilew) - view->mapxoffs;
	win->vmask.y = (win->y / view->map->tileh) - view->mapyoffs;
	win->vmask.w = (win->w / view->map->tilew);
	win->vmask.h = (win->h / view->map->tilew);

	SLIST_INIT(&win->regionsh);
	pthread_mutex_init(&win->lock, NULL);
}

/* View must be locked. */
static void *
dispatch_widget_event(void *p)
{
	struct window_event *wev = p;

	if (WIDGET_OPS(wev->w)->widget_event != NULL) {
		WIDGET_OPS(wev->w)->widget_event(wev->w, &wev->ev, wev->flags);
	}
	free(wev);
	return (NULL);
}

static void
post_widget_ev(void *parent, void *child, void *arg)
{
	pthread_t cbthread;
	struct widget *wid = child;
	struct window_event *wev;
	SDL_Event *ev = arg;
	
	OBJECT_ASSERT(child, "widget");
	
	if (WIDGET_OPS(wid)->widget_event == NULL) {
		return;
	}
	
	wev = emalloc(sizeof(struct window_event));
	wev->w = wid;
	wev->flags = 0;
	wev->ev = *ev;

	pthread_create(&cbthread, NULL, dispatch_widget_event, wev);
}

#define WINDOW_CORNER(win, xo, yo) do {				\
	if (win->flags & WINDOW_ROUNDEDGES) {			\
		int z;						\
		z = ((xo) * (yo)) + ((win)->borderw/2);		\
		if (z < (win)->borderw) {			\
			return (-1);				\
		} else {					\
			*col = (win)->border[((xo)*(yo)) /	\
			    ((win)->borderw)+1];		\
			return (1);				\
		}						\
	} else {						\
		*col = SDL_MapRGB((win)->view->v->format,	\
		    180, 180, 180);				\
		return (1);					\
	}							\
} while (/*CONSTCOND*/0)

static __inline__ int
window_decoration(struct window *win, int xo, int yo, Uint32 *col)
{
	if (xo < win->borderw && yo < win->borderw) {
		/* Upper left */
		WINDOW_CORNER(win, xo, yo);
	} else if (yo < win->borderw && xo > (win->w - win->borderw)) {
		/* Upper right */
		WINDOW_CORNER(win, win->w - xo, yo);
	} else if (xo > (win->w - win->borderw) &&
	           yo > (win->h - win->borderw)) {
		/* Lower right */
		WINDOW_CORNER(win, win->w - xo, win->h - yo);
	} else if (xo < win->borderw && yo > win->h - win->borderw) {
		/* Lower left */
		WINDOW_CORNER(win, xo, win->h - yo);
	}

	if (win->flags & WINDOW_TITLEBAR) {
		if (yo < (win->titleh + win->borderw) && xo > win->borderw &&
		    xo < win->w - win->borderw) {
			if (yo < win->borderw) {
				*col = win->border[yo+1];
			} else if (yo > win->titleh) {
				*col = win->border[yo-win->titleh];
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
	
	if (xo > (win->w - win->borderw)) {
		*col = win->border[win->w - xo];
		return (1);
	} else if (yo < win->borderw) {
		*col = win->border[yo+1];
		return (1);
	} else if (xo < win->borderw) {
		*col = win->border[xo+1];
		return (1);
	} else if (yo > (win->h - win->borderw)) {
		*col = win->border[win->h - yo];
		return (1);
	}
	return (0);
}

/*
 * Render a window.
 * Window must be locked.
 */
void
window_draw(struct window *win)
{
	struct viewport *view = win->view;
	SDL_Surface *v = view->v;
	Uint32 xo, yo, col = 0;
	Uint8 expcom;
	struct region *reg;
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
				WINDOW_PUT_PIXEL(win, xo, yo, col);
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
		rd.x = win->x + (win->w - s->w - win->borderw);
		rd.y = win->y + win->borderw;
		rd.w = s->w;
		rd.h = s->h;
		SDL_BlitSurface(s, NULL, v, &rd);
		SDL_FreeSurface(s);
	}

	/* Render the widgets. */
	SLIST_FOREACH(reg, &win->regionsh, regions) {
		if (reg->flags & REGION_BORDER) {
			Uint32 c;
			int x, y;

			SDL_LockSurface(win->view->v);
			for (y = 0; y < reg->h; y++) {
				for (x = 0; x < reg->w; x++) {
					if (x < 1 || y < 1 ||
					    x >= reg->w-1 ||
					    y >= reg->h-1) {
						c = SDL_MapRGB(v->format,
						    255, 255, 255);

						REGION_PUT_PIXEL(reg, x, y, c);
					}
				}
			}
			SDL_UnlockSurface(win->view->v);
		}

		/* Draw the widgets. */
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			if (!(wid->flags & WIDGET_HIDE)) {
				WIDGET_OPS(wid)->widget_draw(wid);
			}
		}
	}
	SDL_UpdateRect(v, win->x, win->y, win->w, win->h);
	
	/* Always redraw if this is an animated window. */
	/* XXX use real time */
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

/* Called when window is attached to view. */
void
window_onattach(void *parent, void *child)
{
	struct viewport *view = parent;
	struct window *win = child;

	/* Increment the view mask for this area. */
	view_maskfill(view, &win->vmask, 1);
	win->redraw++;
}

/*
 * Called when window is detached from view.
 * Will lock window and view.
 */
void
window_ondetach(void *parent, void *child)
{
	struct viewport *view = parent;
	struct window *win = child;

	/* XXX ... */

	/* Decrement the view mask for this area. */
	view_maskfill(view, &win->vmask, -1);
	if (view->map != NULL) {
		view->map->redraw++;
	}

	/* Other windows may become visible. */
	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH(win, &view->windowsh, windows)
		win->redraw++;
	pthread_mutex_unlock(&view->lock);
}

/*
 * Attach a region to this window.
 * Window must be locked.
 */
void
window_attach(void *parent, void *child)
{
	struct window *win = parent;
	struct region *reg = child;

	OBJECT_ASSERT(parent, "window");
	OBJECT_ASSERT(child, "window-region");
	
	reg->win = win;

	SLIST_INSERT_HEAD(&win->regionsh, reg, regions);
}

/*
 * Detach a segment from this window.
 * Window must be locked.
 */
void
window_detach(void *parent, void *child)
{
	struct window *win = parent;
	struct region *reg = child;

	OBJECT_ASSERT(parent, "window");
	OBJECT_ASSERT(child, "window-region");

	SLIST_REMOVE(&win->regionsh, reg, region, regions);
}

void
window_destroy(void *p)
{
	struct window *win = p;
	struct region *reg, *nextreg = NULL;

	for (reg = SLIST_FIRST(&win->regionsh);
	     reg != SLIST_END(&win->regionsh);
	     reg = nextreg) {
		nextreg = SLIST_NEXT(reg, regions);
		window_detach(win, reg);
		object_destroy(reg);
	}
	pthread_mutex_destroy(&win->lock);
	free(win->caption);
	free(win->border);
}

/* Window list must be locked. */
void
window_draw_all(void)
{
	struct window *win;

	TAILQ_FOREACH(win, &mainview->windowsh, windows) {
		pthread_mutex_lock(&win->lock);
		if (win->redraw) {
			window_draw(win);
		}
		pthread_mutex_unlock(&win->lock);
	}
}

/*
 * Dispatch events to widgets and windows.
 * View must be locked.
 */
int
window_event_all(struct viewport *view, SDL_Event *ev)
{
	struct region *reg;
	struct window *win;
	struct widget *wid;

	switch (ev->type) {
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows,
		    windows_head) {
			pthread_mutex_lock(&win->lock);
			if (WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				SLIST_FOREACH(reg, &win->regionsh, regions) {
					TAILQ_FOREACH(wid, &reg->widgetsh,
					    widgets) {
						if (WIDGET_INSIDE(wid,
						    ev->button.x,
						    ev->button.y)) {
							post_widget_ev(reg,
							    wid, ev);
						}
					}
				}
				pthread_mutex_unlock(&win->lock);
				return (1);
			}
			pthread_mutex_unlock(&win->lock);
		}
		break;
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		win = TAILQ_LAST(&view->windowsh, windows_head);
		pthread_mutex_lock(&win->lock);
		SLIST_FOREACH(reg, &win->regionsh, regions) {
			TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
				if (wid == win->focus) {
					post_widget_ev(reg, wid, ev);
					pthread_mutex_unlock(&win->lock);
					return (1);
				}
			}
		}
		pthread_mutex_unlock(&win->lock);
		break;
	}
	return (0);
}

void
window_resize(struct window *win)
{
	struct region *reg;

	SLIST_FOREACH(reg, &win->regionsh, regions) {
		struct widget *wid;
		int x, y, nwidgets;
		
		/* Scale/position the region. */
		reg->x = (reg->rx * win->body.w / 100) + (win->body.x - win->x);
		reg->y = (reg->ry * win->body.h / 100) + (win->body.y - win->y);
		reg->w = (reg->rw * win->body.w / 100);
		reg->h = (reg->rh * win->body.h / 100);
		
		nwidgets = 0;
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			nwidgets++;
		}

		reg->x += reg->spacing;
		reg->y += reg->spacing;
		reg->w -= reg->spacing * 2;
		reg->h -= reg->spacing * 2;

		x = reg->x;
		y = reg->y;
	
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			/* Scale/position the widget. */
			wid->x = x;
			wid->y = y;
			if (wid->rw > 0) {
				wid->w = wid->rw * reg->w / 100;
			}
			if (wid->rh > 0) {
				wid->h = wid->rh * reg->h / 100;
			}

			if (wid->w >= reg->w)
				wid->w = reg->w;
			if (wid->h >= reg->h)
				wid->h = reg->h;

#ifdef WIDGET_DEBUG
			dprintf("%s: %dx%d at %d,%d\n", OBJECT(wid)->name,
			    wid->w, wid->h, wid->x, wid->y);
#endif

			if (reg->flags & REGION_VALIGN) {
				y += wid->h + reg->spacing;
			} else if (reg->flags & REGION_HALIGN) {
				x += wid->w + reg->spacing;
			}
		}
	}
}


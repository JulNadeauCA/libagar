/*	$Csoft: window.c,v 1.22 2002/05/19 14:30:24 vedge Exp $	*/

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

static Uint32 delta = 0, delta2 = 256;

/* XXX fucking insane */
struct window *
window_new(char *caption, int flags, enum window_type type, int x, int y,
    int w, int h) {
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, mainview, caption, flags, type, x, y, w, h);

	/* Attach window to main view. */
	view_attach(mainview, win);

	dprintf("new window\n");
	return (win);
}

/* XXX fucking insane */
void
window_init(struct window *win, struct viewport *view, char *caption,
    int flags, enum window_type type, int x, int y, int w, int h)
{
	static int nwindow = 0;
	char *name;

	name = object_name("window", nwindow++);
	object_init(&win->obj, "window", name, NULL, 0, &window_ops);
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

	/* XXX pref */
	win->border[0] = SDL_MapRGB(view->v->format, 50, 50, 50);
	win->border[1] = SDL_MapRGB(view->v->format, 100, 100, 160);
	win->border[2] = SDL_MapRGB(view->v->format, 192, 192, 192);
	win->border[3] = SDL_MapRGB(view->v->format, 100, 100, 160);
	win->border[4] = SDL_MapRGB(view->v->format, 50, 50, 50);
	
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

void
window_widget_event(void *parent, void *child, void *arg)
{
	pthread_t cbthread;
	struct widget *wid = child;
	struct window_event *wev;
	SDL_Event *ev = arg;

	dprintf("parent = %s\n", OBJECT(parent)->type);

	OBJECT_ASSERT(child, "widget");

	if (WIDGET_OPS(wid)->widget_event == NULL) {
		return;
	}

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		if (WIDGET_INSIDE(wid, ev->button.x, ev->button.y)) {
			wev = emalloc(sizeof(struct window_event));
			wev->w = wid;
			wev->flags = 0;
			wev->ev = *ev;
			pthread_create(&cbthread, NULL, dispatch_widget_event,
			    wev);
		}
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		dprintf("key\n");
		break;
	default:
		fatal("cannot handle event\n");
	}
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

/*
 * Render a window.
 * Window must be locked.
 */
void
window_draw(struct window *win)
{
	SDL_Surface *v = win->view->v;
	Uint32 xo, yo, col = 0;
	Uint8 *dst, expcom;
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
	SLIST_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			if (!(wid->flags & WIDGET_HIDE)) {
				WIDGET_OPS(wid)->widget_draw(wid);
			}
		}
	}
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

/* Called when window is attached to view. */
void
window_onattach(void *parent, void *child)
{
	struct viewport *view = parent;
	struct window *win = child;

	dprintf("%s is being attached to %s\n", OBJECT(win)->name,
	    OBJECT(view)->name);

	/* Increment the view mask for this area. */
	view_maskfill(view, &win->vmask, 1);
	win->redraw++;
}

/*
 * Called when window is detached from view.
 * Will lock window.
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
	reg->x = reg->rx * win->w / 100;
	reg->y = reg->ry * win->h / 100;
	reg->w = reg->rw * win->w / 100;
	reg->h = reg->rh * win->h / 100;

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
	OBJECT_ASSERT(child, "window-segment");

	SLIST_REMOVE(&win->regionsh, reg, region, regions);
}

void
window_destroy(void *p)
{
	struct window *win = p;
#if 0
	struct segment *seg, *nextseg = NULL;

	/* Free segments. */
	for (seg = SLIST_FIRST(&win->winsegsh);
	     seg != SLIST_END(&win->winsegsh);
	     seg = nextseg) {
		nextseg = SLIST_NEXT(seg, winsegs);
		free(seg);
	}
#endif
	pthread_mutex_destroy(&win->lock);
	free(win->caption);
}

/* Window list must be locked. */
void
window_draw_all(void)
{
	struct window *win;

	TAILQ_FOREACH_REVERSE(win, &mainview->windowsh, windows, windows_head) {
		pthread_mutex_lock(&win->lock);
		if (win->redraw) {
			window_draw(win);
		}
		pthread_mutex_unlock(&win->lock);
	}
}

void
window_resize(struct window *win)
{
	struct region *reg;

	SLIST_FOREACH(reg, &win->regionsh, regions) {
		struct widget *wid;
		int x, y;

		x = reg->x;
		y = reg->y;
	
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			wid->x = x;
			wid->y = y;

			if (!(wid->flags & WIDGET_HIDE)) {
				WIDGET_OPS(wid)->widget_draw(wid);
			}

			switch (reg->walign) {
			case WIDGET_HALIGN:
				x += wid->w + reg->spacing;
				break;
			case WIDGET_VALIGN:
				y += wid->h + reg->spacing;
				break;
			}
		}
	}
}


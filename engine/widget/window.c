/*	$Csoft: window.c,v 1.37 2002/06/06 10:18:02 vedge Exp $	*/

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
	window_attach,
	window_detach
};

/* XXX struct */
#include "borders/grey8.h"

static Uint32 delta = 0, delta2 = 256;
static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */

struct window *
window_new(char *caption, int flags, enum window_type type, int x, int y,
    int w, int h) {
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, mainview, caption, flags, type, x, y, w, h);

	/* Attach window to main view, make it visible. */
	view_attach(mainview, win);

	return (win);
}

void
window_init(struct window *win, struct viewport *view, char *caption,
    int flags, enum window_type type, int rx, int ry, int rw, int rh)
{
	static int nwindow = 0;
	SDL_Surface *s;
	char *name;
	int i;
	
	flags |= WINDOW_ROUNDEDGES;	/* XXX pref */

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

	s = TTF_RenderText_Solid(font, "ABC1234", white);
	win->titleh = s->h + win->borderw;	/* XXX ridiculous */
	SDL_FreeSurface(s);

	win->caption = strdup(caption);
	win->view = view;
	win->flags = flags;
	win->type = type;
	win->spacing = 4;
	win->redraw = 0;
	win->focus = NULL;

	if (flags & WINDOW_ABSOLUTE) {
		for (win->x = 0; win->x < rx; win->x += TILEW) ;;
		for (win->y = 0; win->y < ry; win->y += TILEH) ;;
		for (win->w = 0; win->w < rw; win->w += TILEW) ;;
		for (win->h = 0; win->h < rh; win->h += TILEH) ;;
	} else {
		for (win->x = 0; win->x < (rx * view->w / 100);
		     win->x += TILEW) ;;
		for (win->y = 0; win->y < (ry * view->h / 100);
		     win->y += TILEH) ;;
		for (win->w = 0; win->w < (rw * view->w / 100);
		     win->w += TILEW) ;;
		for (win->h = 0; win->h < (rh * view->h / 100);
		     win->h += TILEH) ;;
	}

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

	TAILQ_INIT(&win->regionsh);
	pthread_mutex_init(&win->lock, NULL);
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
					*col = SDL_MapRGB(
					    win->view->v->format,
					    0, xo<<3, 0);
					break;
				case WINDOW_GRADIENT:
					if (win == TAILQ_LAST(
					     &win->view->windowsh, windowq)) {
						*col = SDL_MapRGB(
						    win->view->v->format,
						    0, 100 + (xo / 4), yo * 3);
					} else {
						*col = SDL_MapRGB(
						    win->view->v->format,
						    0, xo / 2, yo);
					}
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
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
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

	TAILQ_INSERT_HEAD(&win->regionsh, reg, regions);
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

	TAILQ_REMOVE(&win->regionsh, reg, regions);
}

/*
 * This cannot be used when the events are being processed. Events
 * referencing windows which no longer exist cannot be dealt with
 * in a sane, portable manner.
 */
void
window_destroy(void *p)
{
	struct window *win = p;
	struct region *reg, *nextreg = NULL;

	for (reg = TAILQ_FIRST(&win->regionsh);
	     reg != TAILQ_END(&win->regionsh);
	     reg = nextreg) {
		nextreg = TAILQ_NEXT(reg, regions);
		window_detach(win, reg);
		object_destroy(reg);
	}
	free(win->caption);
	free(win->border);
	pthread_mutex_destroy(&win->lock);
}

/* Window/view must be locked. */
int
window_show(struct window *win)
{
	struct viewport *view = win->view;
	struct region *reg;
	struct widget *wid;
	struct window *owin;
	int prev;

	prev = (win->flags & WINDOW_SHOW);
	if (prev) {
		/* Already visible. */
		return (prev);
	}
	
	win->flags |= WINDOW_SHOW;

	/* Calculate and increment the view mask for this area. */
	win->vmask.x = (win->x / TILEW) - view->mapxoffs;
	win->vmask.y = (win->y / TILEH) - view->mapyoffs;
	win->vmask.w = (win->w / TILEW);
	win->vmask.h = (win->h / TILEW);
	view_maskfill(win->view, &win->vmask, 1);
	win->redraw++;

	TAILQ_REMOVE(&view->windowsh, win, windows);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
	
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "shown", "%p", win);
		}
	}
	
	/* Other windows titlebars should be redrawn. */
	TAILQ_FOREACH(owin, &win->view->windowsh, windows) {
		if (win == owin) {
			continue;
		}
		pthread_mutex_lock(&owin->lock);
		if (owin->flags & WINDOW_SHOW) {
			owin->redraw++;
		}
		pthread_mutex_unlock(&owin->lock);
	}

	return (prev);
}

/* Window/view must be locked. */
int
window_hide(struct window *win)
{
	struct window *owin, *newwin;
	struct viewport *view = win->view;
	struct region *reg;
	struct widget *wid;
	int prev;
	
	prev = (win->flags & WINDOW_SHOW);
	if (!prev) {
		/* Already hidden. */
		return (prev);
	}

	/* Notify child widgets */
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "hidden", "%p", win);
		}
	}
	
	win->flags &= ~(WINDOW_SHOW);
	view_maskfill(view, &win->vmask, -1);

	/* Map may become visible. */
	if (view->map != NULL) {
		view->map->redraw++;
	}

	/* Other windows may become visible. */
	newwin = NULL;
	TAILQ_FOREACH(owin, &view->windowsh, windows) {
		if (win == owin) {
			continue;
		}
		pthread_mutex_lock(&owin->lock);
		if (owin->flags & WINDOW_SHOW) {
			newwin = owin;
			owin->redraw++;
		}
		pthread_mutex_unlock(&owin->lock);
	}
	if (newwin != NULL) {
		TAILQ_REMOVE(&view->windowsh, newwin, windows);
		TAILQ_INSERT_TAIL(&view->windowsh, newwin, windows);
	}
	
	return (prev);
}

/* View must be locked. */
void
window_draw_all(void)
{
	struct window *win;

	TAILQ_FOREACH(win, &mainview->windowsh, windows) {
		if (pthread_mutex_trylock(&win->lock) == 0) {
			if (win->redraw && win->flags & WINDOW_SHOW) {
				window_draw(win);
			}
			pthread_mutex_unlock(&win->lock);
		}
	}
}

/*
 * Cycle focus throughout widgets.
 * Window must be locked.
 */
static void
cycle_widgets(struct window *win, int reverse)
{
	struct widget *wid = win->focus;
	struct region *nreg, *rreg;
	struct widget *owid, *rwid;
	struct widget *nwid;

	if (wid == NULL) {
		dprintf("%s: no focus\n", OBJECT(win)->name);
		return;
	}

	if (reverse) {
		nwid = TAILQ_PREV(wid, widgetsq, widgets);
	} else {
		nwid = TAILQ_NEXT(wid, widgets);
	}

	if (nwid != NULL) {
		WIDGET_FOCUS(nwid);
		return;
	}

	TAILQ_FOREACH(nreg, &win->regionsh, regions) {
		TAILQ_FOREACH(owid, &nreg->widgetsh, widgets) {
			if (owid != wid) {
				continue;
			}
			if (reverse) {
				rreg = TAILQ_PREV(nreg, regionsq, regions);
				if (rreg != NULL) {
					rwid = TAILQ_LAST(&rreg->widgetsh,
					    widgetsq);
					if (rwid != NULL) {
						WIDGET_FOCUS(rwid);
						return;
					}
				} else {
					rreg = TAILQ_LAST(&win->regionsh,
					    regionsq);
					if (rreg != NULL) {
						rwid = TAILQ_LAST(
						    &rreg->widgetsh,
						    widgetsq);
						if (rwid != NULL) {
							WIDGET_FOCUS(rwid);
							return;
						}
					}
				}
			} else {
				rreg = TAILQ_NEXT(nreg, regions);
				if (rreg != NULL) {
					rwid = TAILQ_FIRST(&rreg->widgetsh);
					if (rwid != NULL) {
						WIDGET_FOCUS(rwid);
						return;
					}
				} else {
					rreg = TAILQ_FIRST(&win->regionsh);
					if (rreg != NULL) {
						rwid = TAILQ_FIRST(
						    &rreg->widgetsh);
						if (rwid != NULL) {
							WIDGET_FOCUS(rwid);
							return;
						}
					}
				}
			}
		}
	}
}

/*
 * Dispatch events to widgets and windows.
 * View must be locked, window list must not be empty.
 */
int
window_event_all(struct viewport *view, SDL_Event *ev)
{
	struct region *reg;
	struct window *win;
	struct widget *wid;

	TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if ((win->flags & WINDOW_SHOW) == 0) {
			goto nextwin;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			switch (view->winop) {
			case VIEW_WINOP_MOVE:
				view_maskfill(view, &win->vmask, -1);
				if (MAP_COORD(ev->motion.x, view) <
				    view->wop_mapx)
					win->x -= TILEW;
				else if (MAP_COORD(ev->motion.x, view) >
				    view->wop_mapx)
					win->x += TILEW;
				if (MAP_COORD(ev->motion.y, view) <
				    view->wop_mapy)
					win->y -= TILEH;
				else if (MAP_COORD(ev->motion.y, view) >
				    view->wop_mapy)
					win->y += TILEH;

				if ((win->x + win->w) > view->w - 32)
					win->x = view->w - win->w - 32;
				if ((win->y + win->h) > view->h - 32)
					win->y = view->h - win->h - 32;
				if (win->x < 32)
					win->x = 32;
				if (win->y < 32)
					win->y = 32;
			
				win->vmask.x = (win->x / TILEW) -
				    view->mapxoffs;
				win->vmask.y = (win->y / TILEH) -
				    view->mapyoffs;
				win->vmask.w = (win->w / TILEW);
				win->vmask.h = (win->h / TILEW);
				view_maskfill(view, &win->vmask, 1);

				/* XXX incomplete */
				win->redraw++;
				mainview->map->redraw++;	/* XXX */
			
				view->wop_mapx = MAP_COORD(ev->motion.x, view);
				view->wop_mapy = MAP_COORD(ev->motion.y, view);
	
				goto posted;
			case VIEW_WINOP_RESIZE:
			case VIEW_WINOP_NONE:
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			view->winop = VIEW_WINOP_NONE;
			/* FALLTHROUGH */
		case SDL_MOUSEBUTTONDOWN:
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto nextwin;
			}
			if (ev->type && SDL_MOUSEBUTTONDOWN &&
			    ev->button.y - win->y <= win->titleh) {
				view->winop = VIEW_WINOP_MOVE;
			}
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
					if (!WIDGET_INSIDE(wid, ev->button.x,
					    ev->button.y)) {
						continue;
					}
					event_post(wid,
					    (ev->type == SDL_MOUSEBUTTONUP) ?
					    "window-mousebuttonup" :
					    "window-mousebuttondown",
					    "%i, %i, %i",
					    ev->button.button,
					    ev->button.x -
					    (wid->x - wid->win->x),
					    ev->button.y -
					    (wid->y - wid->win->y));
					goto posted;
				}
			}
			break;
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			switch (ev->key.keysym.sym) {	/* Always ignore */
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				pthread_mutex_unlock(&win->lock);
				return (0);
			default:
				break;
			}
			if (ev->key.keysym.sym == SDLK_TAB &&
			    ev->type == SDL_KEYUP) {
				cycle_widgets(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				win->redraw++;
				goto posted;
			}
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
					if (!WIDGET_FOCUSED(wid)) {
						continue;
					}
					event_post(wid,
					    (ev->type == SDL_KEYUP) ?
					    "window-keyup" :
					    "window-keydown",
					    "%i, %i",
					    (int)ev->key.keysym.sym,
					    (int)ev->key.keysym.mod);
					goto posted;
				}
			}
		}
nextwin:
		pthread_mutex_unlock(&win->lock);
	}

	return (0);
posted:
	pthread_mutex_unlock(&win->lock);
	return (1);
}

void
window_resize(struct window *win)
{
	struct region *reg;

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		struct widget *wid;
		int x, y;
		int nwidgets = 0;

		TAILQ_FOREACH(wid, &reg->widgetsh, widgets)
			nwidgets++;

		/* Scale/position the region. */
		reg->x = (reg->rx * win->body.w / 100) +
		    (win->body.x - win->x);
		reg->y = (reg->ry * win->body.h / 100) +
		    (win->body.y - win->y);
		reg->w = (reg->rw * win->body.w / 100);
		reg->h = (reg->rh * win->body.h / 100);
	
		reg->x += reg->spacing / 2;
		reg->y += reg->spacing / 2;
		reg->w -= reg->spacing;
		reg->h -= reg->spacing;

		x = reg->x;
		y = reg->y;
	
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			wid->x = x;
			wid->y = y;

			if (wid->rw > 0) {
				wid->w = wid->rw * reg->w / 100;
			}
			if (wid->rh > 0)
				wid->h = wid->rh * reg->h / 100;

			if (wid->w > reg->w)
				wid->w = reg->w;
			if (wid->h > reg->h)
				wid->h = reg->h;

#if 0
			dprintf("%s: %dx%d at %d,%d\n", OBJECT(wid)->name,
			    wid->w, wid->h, wid->x, wid->y);
#endif

			/* Space widgets */
			if (reg->flags & REGION_VALIGN) {
				y += wid->h + (reg->spacing / nwidgets) / 2;
				wid->h -= reg->spacing/nwidgets;
			} else if (reg->flags & REGION_HALIGN) {
				x += wid->w + (reg->spacing / nwidgets) / 2;
				wid->w -= reg->spacing / nwidgets;
			}
		}
	}
}


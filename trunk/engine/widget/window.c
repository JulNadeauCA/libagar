/*	$Csoft: window.c,v 1.46 2002/07/08 05:24:50 vedge Exp $	*/

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

#include <engine/physics.h>
#include <engine/mapedit/mapedit.h>

#include "text.h"
#include "widget.h"
#include "window.h"

static const struct object_ops window_ops = {
	window_destroy,
	NULL,		/* load */
	NULL		/* save */
};

/* XXX struct */
#include "borders/grey8.h"

static Uint32 delta = 0, delta2 = 256;
static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */

static void	window_move(struct window *, Uint16, Uint16);

struct window *
window_new(char *caption, int flags, int x, int y, int w, int h) {
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, caption, flags, x, y, w, h);

	/* Attach window to main view, make it visible. */
	pthread_mutex_lock(&view->lock);
	view_attach(win);
	pthread_mutex_unlock(&view->lock);

	return (win);
}

void
window_init(struct window *win, char *caption, int flags,
    int rx, int ry, int rw, int rh)
{
	static int nwindow = 0;
	char *name;
	int i;

	flags |= WINDOW_TITLEBAR|WINDOW_ROUNDEDGES;

	name = object_name("window", nwindow++);
	object_init(&win->obj, "window", name, "window", OBJ_ART, &window_ops);
	free(name);
	
	/* XXX pref */
	win->borderw = default_nborder;
	win->border = emalloc(win->borderw * sizeof(Uint32));
	for (i = 0; i < win->borderw; i++) {
		win->border[i] = SDL_MapRGB(view->v->format,
		    default_border[i].r, default_border[i].g,
		    default_border[i].b);
	}

	win->titleh = font_h + win->borderw;

	win->caption = strdup(caption);
	win->flags = flags;
	win->type = flags & WINDOW_TYPE;
	if (win->type == 0) {
		win->type = WINDOW_DEFAULT_TYPE;
	}
	win->spacing = 4;
	win->redraw = 0;
	win->focus = NULL;

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		if (flags & WINDOW_SCALE) {
			for (win->x = 0; win->x < (rx * view->w / 100);
			     win->x += TILEW) ;;
			for (win->y = 0; win->y < (ry * view->h / 100);
			     win->y += TILEH) ;;
			for (win->w = 0; win->w < (rw * view->w / 100);
			     win->w += TILEW) ;;
			for (win->h = 0; win->h < (rh * view->h / 100);
			     win->h += TILEH) ;;
		} else {
			for (win->x = 0; win->x < rx; win->x += TILEW) ;;
			for (win->y = 0; win->y < ry; win->y += TILEH) ;;
			for (win->w = 0; win->w < rw; win->w += TILEW) ;;
			for (win->h = 0; win->h < rh; win->h += TILEH) ;;
		}
		break;
	case GFX_ENGINE_GUI:
		if (flags & WINDOW_SCALE) {
			win->x = (rx * view->w / 100);
			win->y = (ry * view->h / 100);
			win->w = (rw * view->w / 100);
			win->h = (rh * view->h / 100);
		} else {
			win->x = rx;
			win->y = ry;
			win->w = rw;
			win->h = rh;
		}
		break;
	}

	if (flags & WINDOW_CENTER) {
		win->x = view->w/2 - win->w/2;
		win->y = view->h/2 - win->h/2;
	} else {
		win->x = (win->x < view->w) ? win->x : 0;
		win->y = (win->y < view->h) ? win->y : 0;
	}
	win->w = (win->x + win->w < view->w) ? win->w : view->w;
	win->h = (win->y + win->h < view->h) ? win->h : view->h;

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

#define WINDOW_CORNER(win, xo, yo) do {					\
	if (win->flags & WINDOW_ROUNDEDGES) {				\
		int z;							\
		z = ((xo) * (yo)) + ((win)->borderw/2);			\
		if (z < (win)->borderw) {				\
			return (-1);					\
		} else {						\
			*col = (win)->border[((xo)*(yo)) /		\
			    ((win)->borderw)+1];			\
			return (1);					\
		}							\
	} else {							\
		*col = SDL_MapRGB(view->v->format, 180, 180, 180);	\
		return (1);						\
	}								\
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
		/* XXX inefficient */
		if (yo < (win->titleh + win->borderw) - 1 &&
		    xo > win->borderw &&
		    xo < win->w - win->borderw) {
			if (yo < win->borderw - 1) {
				*col = win->border[yo+1];
			} else if (yo > win->titleh) {
				*col = win->border[yo-win->titleh];
			} else {
				*col = SDL_MapRGB(view->v->format, 0, 0, 0);
			}
			return (1);
		}
	}
	
	if (xo > (win->w - win->borderw)) {		/* Right */
		*col = win->border[win->w - xo];
		return (1);
	} else if (yo < win->borderw - 1) {		/* Top */
		*col = win->border[yo+1];
		return (1);
	} else if (xo < win->borderw - 1) {		/* Left */
		*col = win->border[xo+1];
		return (1);
	} else if (yo > (win->h - win->borderw)) {	/* Bottom */
		if (xo == (win->w - 16) || xo == 16) {
			*col = SDL_MapRGB(view->v->format, 0, 0, 0);
		} else if (xo == (win->w - 17) || xo == 17) {
			*col = SDL_MapRGB(view->v->format, 250, 250, 250);
		} else {
			*col = win->border[win->h - yo];
		}
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
	SDL_Surface *v = view->v;
	struct region *reg;
	struct widget *wid;
	int rv;
	Uint32 xo, yo, col = 0;
	Uint8 expcom;

	/* Render the background. */
	if (win->flags & WINDOW_PLAIN) {
		SDL_Rect rd;

		rd.x = win->x;
		rd.y = win->y;
		rd.w = win->w;
		rd.h = win->h;
		
		SDL_FillRect(v, &rd, win->bgcolor);
	} else {
		/* XXX inefficient */
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

		rd.x = win->x + win->borderw;
		rd.y = win->y + win->borderw;
		SDL_BlitSurface(SPRITE(win, 0), NULL, view->v, &rd);
	}

	/* Render the widgets. */
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		if (reg->flags & REGION_BORDER) {
			Uint32 c;
			int x, y;

			SDL_LockSurface(view->v);
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
			SDL_UnlockSurface(view->v);
		}

		/* Draw the widgets. */
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			if (!(wid->flags & WIDGET_HIDE)) {
				WIDGET_OPS(wid)->widget_draw(wid);
			}
		}
	}

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		SDL_UpdateRect(v, win->x, win->y, win->w, win->h);
		break;
	case GFX_ENGINE_GUI:
		/* The screen will be redrawn entirely. */
		break;
	}
	
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
 * Update animations.
 * Window must be locked.
 */
void
window_animate(struct window *win)
{
	struct region *reg;
	struct widget *wid;

	/* Render animated widgets. */
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			if (!(wid->flags & WIDGET_HIDE) &&
			    WIDGET_OPS(wid)->widget_animate != NULL) {
				WIDGET_OPS(wid)->widget_animate(wid);
			}
		}
	}

	if (view->gfx_engine == GFX_ENGINE_TILEBASED) { /* XXX special */
		SDL_UpdateRect(view->v, win->x, win->y, win->w, win->h);
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

/* View and window must not be locked. */
int
window_show(struct window *win)
{
	int rv;

	if (view->gfx_engine == GFX_ENGINE_TILEBASED)
		pthread_mutex_lock(&view->lock);

	pthread_mutex_lock(&win->lock);
	rv = window_show_locked(win);
	pthread_mutex_unlock(&win->lock);

	if (view->gfx_engine == GFX_ENGINE_TILEBASED)
		pthread_mutex_unlock(&view->lock);

	return (rv);
}

/* View and window must not be locked. */
int
window_hide(struct window *win)
{
	int rv;

	if (view->gfx_engine == GFX_ENGINE_TILEBASED)
		pthread_mutex_lock(&view->lock);

	pthread_mutex_lock(&win->lock);
	rv = window_hide_locked(win);
	pthread_mutex_unlock(&win->lock);

	if (view->gfx_engine == GFX_ENGINE_TILEBASED)
		pthread_mutex_unlock(&view->lock);

	return (rv);
}

/* View and window must be locked. */
int
window_show_locked(struct window *win)
{
	struct region *reg;
	struct widget *wid;
	int prev;

	prev = (win->flags & WINDOW_SHOWN);
	if (prev) {
		/* Already visible. */
		return (prev);
	}
	
	win->flags |= WINDOW_SHOWN;

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		/* Calculate and increment the tile mask for this area. */
		win->vmask.x = (win->x / TILEW);
		win->vmask.y = (win->y / TILEH);
		win->vmask.w = (win->w / TILEW);
		win->vmask.h = (win->h / TILEW);
		view_maskfill(&win->vmask, 1);
		break;
	default:
		break;
	}

	view->focus_win = win;
	
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "shown", "%p", win);
		}
	}

#if 0
	VIEW_REDRAW();
#endif
	return (prev);
}

/*
 * Window must be locked.
 * View must be locked in tile-based mode. XXX
 */
int
window_hide_locked(struct window *win)
{
	struct region *reg;
	struct widget *wid;
	int prev;

	prev = (win->flags & WINDOW_SHOWN);
	if (!prev) {
		/* Already hidden. */
		return (prev);
	}
	
	view->focus_win = NULL;

	/* Notify child widgets */
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "hidden", "%p", win);
		}
	}
	
	win->flags &= ~(WINDOW_SHOWN);

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		view_maskfill(&win->vmask, -1);
		break;
	case GFX_ENGINE_GUI:
		{ 
			SDL_Rect rd;

			rd.x = win->x;
			rd.y = win->y;
			rd.w = win->w;
			rd.h = win->h;

			SDL_FillRect(view->v, &rd, NULL);
		}
		break;
	}

#if 0
	/* The highest window must be at the tail of the queue. */
	WINDOW_CYCLE(win);
	VIEW_REDRAW();
#endif
	return (prev);
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

/* View and window must be locked. */
static void
window_move(struct window *win, Uint16 x, Uint16 y)
{
	int moved = 0;
	int tilew = TILEW, tileh = TILEH;

	switch (view->gfx_engine) {
	case GFX_ENGINE_GUI:
		tilew = 16;	/* XXX pref */
		tileh = 16;
		break;
	default:
		break;
	}

	if (x/tilew < view->wop_mapx && win->x > tilew) {
		win->x -= tilew;
		moved++;
	} else if (x/tilew > view->wop_mapx &&
	    (win->x + win->w) < (view->w - tilew)) {
		win->x += tilew;
		moved++;
	}
	if (y/tilew < view->wop_mapy && win->y > tileh) {
		win->y -= tileh;
		moved++;
	} else if (y/tileh > view->wop_mapy &&
	    (win->y + win->h) < (view->h - tileh)) {
		win->y += tileh;
		moved++;
	}

	if (moved) {
		switch (view->gfx_engine) {
		case GFX_ENGINE_TILEBASED:
			/* Move the tile mask over to the new position. */
			view_maskfill(&win->vmask, -1);
			win->vmask.x = (win->x / TILEW);
			win->vmask.y = (win->y / TILEH);
			win->vmask.w = (win->w / TILEW);
			win->vmask.h = (win->h / TILEW);
			view_maskfill(&win->vmask, 1);
			break;
		case GFX_ENGINE_GUI:
			/* XXX cosmetic */
			SDL_FillRect(view->v, NULL,
			    SDL_MapRGB(view->v->format, 0, 0, 0));
			break;
		}
	}

	view->wop_mapx = x / tilew;
	view->wop_mapy = y / tileh;
}

/*
 * Dispatch events to widgets and windows.
 * View must be locked, window list must not be empty.
 */
int
window_event_all(SDL_Event *ev)
{
	struct region *reg;
	struct window *win;
	struct widget *wid;
	int gavefocus = 0;

	if (ev->type == SDL_MOUSEBUTTONUP && view->winop != VIEW_WINOP_NONE) {
		view->winop = VIEW_WINOP_NONE;
		return (1);
	}

	/* Dispatch relevant events to relevant windows. */
	TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if ((win->flags & WINDOW_SHOWN) == 0) {
			goto nextwin;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			/*
			 * Window operation
			 */
			switch (view->winop) {
			case VIEW_WINOP_MOVE:
				if (view->wop_win != win) {
					goto nextwin;
				}
				window_move(win, ev->motion.x, ev->motion.y);
				goto posted;
			case VIEW_WINOP_LRESIZE:
			case VIEW_WINOP_RRESIZE:
			case VIEW_WINOP_HRESIZE:
				do {
					SDL_Rect rd;
					rd.x = win->x;
					rd.y = win->y;
					rd.w = win->w;
					rd.h = win->h;
					SDL_FillRect(view->v, &rd, 0);
				} while (/*CONSTCOND*/ 0);
				if (view->wop_win != win) {
					goto nextwin;
				}
				switch (view->winop) {
				case VIEW_WINOP_LRESIZE:
					if (ev->motion.xrel < 0) {
						win->w -= ev->motion.xrel;
						win->x += ev->motion.xrel;
					} else if (ev->motion.xrel > 0) {
						win->w -= ev->motion.xrel;
						win->x += ev->motion.xrel;
					}
					if (ev->motion.yrel < 0) {
						win->h += ev->motion.yrel;
					} else if (ev->motion.yrel > 0) {
						win->h += ev->motion.yrel;
					}
					break;
				case VIEW_WINOP_RRESIZE:
					if (ev->motion.xrel < 0) {
						win->w += ev->motion.xrel;
					} else if (ev->motion.xrel > 0) {
						win->w += ev->motion.xrel;
					}
					if (ev->motion.yrel < 0) {
						win->h += ev->motion.yrel;
					} else if (ev->motion.yrel > 0) {
						win->h += ev->motion.yrel;
					}
					break;
				case VIEW_WINOP_HRESIZE:
					if (ev->motion.yrel < 0) {
						win->h += ev->motion.yrel;
					} else if (ev->motion.yrel > 0) {
						win->h += ev->motion.yrel;
					}
				default:
				}
				window_resize(win);
				break;
			case VIEW_WINOP_NONE:
				break;
			}
			/*
			 * Widget event
			 */
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
					int widx, widy;

					if (!WIDGET_FOCUSED(wid)) {
						continue;
					}
					widx = wid->x + wid->win->x;
					widy = wid->y + wid->win->y;

					/* XXX inefficient */
					if (wid->flags & WIDGET_MOUSEOUT &&
					    ((int)ev->motion.x < widx ||
					    (int)ev->motion.y < widy ||
					    (int)ev->motion.x > widx+wid->w ||
					    (int)ev->motion.y > widy+wid->h)) {
						event_post(wid,
						    "window-mouseout", NULL);
						goto posted;
					}
					
					event_post(wid, "window-mousemotion",
					    "%i, %i",
					    (int)ev->motion.x -
					     (wid->x + wid->win->x),
					    (int)ev->motion.y -
					     (wid->y + wid->win->y));
					goto posted;
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto nextwin;
			}
			if (!WINDOW_FOCUSED(win)) {
				view->focus_win = win;
				gavefocus++;
			}
			/* FALLTHROUGH */
		case SDL_MOUSEBUTTONUP:
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto nextwin;
			}
			/*
			 * Window operation.
			 */
			if (ev->type == SDL_MOUSEBUTTONDOWN) {
				if (ev->button.y - win->y <= win->titleh) {
				    	if (ev->button.x - win->x < 20) {
						window_hide_locked(win);
					}
					view->winop = VIEW_WINOP_MOVE;
					view->wop_win = win;
				} else if (ev->button.y - win->y >
				    win->h - win->borderw) {
				    	if (ev->button.x - win->x < 17) {
						view->winop =
						    VIEW_WINOP_LRESIZE;
					} else if (ev->button.x - win->x >
					    win->h - 17) {
						view->winop =
						    VIEW_WINOP_RRESIZE;
					} else {
						view->winop = 
						    VIEW_WINOP_HRESIZE;
					}
					view->wop_win = win;
				}	
			}
			/*
			 * Widget event.
			 */
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
					    (wid->x + wid->win->x),
					    ev->button.y -
					    (wid->y + wid->win->y));
					goto posted;
				}
			}
			if (gavefocus) {
				goto posted;
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
			/*
			 * Tab cycling.
			 */
			if (ev->key.keysym.sym == SDLK_TAB &&
			    ev->type == SDL_KEYUP) {
				cycle_widgets(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				win->redraw++;
				goto posted;
			}
			/*
			 * Widget event.
			 */
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

	/* If the focus was given to another window, update the list. */
	if (view->focus_win != NULL) {
		dprintf("gave focus to %s\n", OBJECT(view->focus_win)->name);
		WINDOW_FOCUS(view->focus_win);
	}
	return (1);
}

/* Window must be locked. */
void
window_resize(struct window *win)
{
	struct region *reg;
	
	win->body.x = win->x + win->borderw;
	win->body.y = win->y + win->borderw*2 + win->titleh;
	win->body.w = win->w - win->borderw*2;
	win->body.h = win->h - win->borderw*2 - win->titleh;

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		struct widget *wid;
		int x = win->borderw + 4, y = win->titleh + win->borderw + 4; 
		int nwidgets = 0;

		TAILQ_FOREACH(wid, &reg->widgetsh, widgets)
			nwidgets++;

		/* Region coordinates */
		if (reg->rx > 0) {
			reg->x = (reg->rx * win->body.w / 100) +
			    (win->body.x - win->x) + 1;
		} else if (reg->rx == 0) {
			reg->x = x;
		} else {
			reg->x = abs(reg->rx);
		}
		if (reg->ry > 0) {
			reg->y = (reg->ry * win->body.h / 100) +
			    (win->body.y - win->y);
		} else if (reg->ry == 0) {
			reg->y = y;
		} else {
			reg->y = abs(reg->ry);
		}

		/* Region geometry */
		if (reg->rw > 0) {
			reg->w = (reg->rw * win->body.w / 100);
		} else if (reg->rw == 0) {
			reg->w = win->w - x;
		} else {
			reg->w = abs(reg->rw);
		}
		
		if (reg->rh >= 0) {
			reg->h = (reg->rh * win->body.h / 100) - 4; /* XXX */
		} else if (reg->rh == 0) {
			reg->h = win->h - y;
		} else {
			reg->h = abs(reg->rh);
		}
		
		reg->x += reg->spacing / 2;
		reg->y += reg->spacing / 2;
		reg->w -= reg->spacing;
		reg->h -= reg->spacing;

		x = reg->x;
		y = reg->y;
	
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			wid->x = x;
			wid->y = y;

			if (wid->rw > 0)
				wid->w = wid->rw * reg->w / 100;
			if (wid->rh > 0)
				wid->h = wid->rh * reg->h / 100;

			if (wid->w > reg->w)
				wid->w = reg->w;
			if (wid->h > reg->h)
				wid->h = reg->h;

			event_post(wid, "window-widget-scaled", "%i, %i",
			    reg->w, reg->h);

#if 0
			dprintf("%s: %dx%d at %d,%d\n", OBJECT(wid)->name,
			    wid->w, wid->h, wid->x, wid->y);
#endif

			/* Space widgets */
			if (reg->flags & REGION_VALIGN) {
				y += wid->h + reg->spacing;
				if (wid->rw > 0) {
					wid->h -= reg->spacing / nwidgets;
				}
			} else if (reg->flags & REGION_HALIGN) {
				x += wid->w + reg->spacing;
				if (wid->rw > 0) {
					wid->w -= reg->spacing / nwidgets;
				}
			}
		}
	}
}


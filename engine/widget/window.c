/*	$Csoft: window.c,v 1.66 2002/08/26 07:19:12 vedge Exp $	*/

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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/config.h>

#include <engine/compat/vasprintf.h>
#include <engine/mapedit/mapedit.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "primitive.h"

static const struct object_ops window_ops = {
	window_destroy,
	NULL,		/* load */
	NULL		/* save */
};

Uint32 bg_color = 0;

/* XXX struct */
#include "borders/grey8.h"

static void	window_move(struct window *, SDL_MouseMotionEvent *);
static void	window_update_mask(struct window *);
static void	window_clamp(struct window *, int, int);
static void	window_round(struct window *, int, int, int, int);
static void	winop_resize(int, struct window *, SDL_MouseMotionEvent *);

/* View must not be locked. */
struct window *
window_new(char *caption, int flags, int x, int y, int w, int h, int minw,
    int minh)
{
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, caption, flags, x, y, w, h, minw, minh);

	/* Attach window to main view, make it visible. */
	pthread_mutex_lock(&view->lock);
	view_attach(win);
	pthread_mutex_unlock(&view->lock);

	return (win);
}

/* Adjust window to tile granularity. */
static void
window_round(struct window *win, int x, int y, int w, int h)
{
	win->x = x - (x % TILEW);
	win->y = y - (y % TILEH);
	win->w = w - (w % TILEW);
	win->h = h - (h % TILEH);
}

void
window_init(struct window *win, char *caption, int flags,
    int rx, int ry, int rw, int rh, int minw, int minh)
{
	static int nwindow = 0;
	char *name;
	int i;

	if (bg_color == 0) {
		/* Set the background color. */
		bg_color = SDL_MapRGB(view->v->format, 0, 0, 0);
	}

	/* XXX pref */
	flags |= WINDOW_TITLEBAR;

	name = object_name("window", nwindow++);
	object_init(&win->wid.obj, "window", name, "window",
	    OBJECT_ART, &window_ops);
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
	win->flags = flags;
	win->type = (flags & WINDOW_TYPE) == 0 ? WINDOW_DEFAULT_TYPE :
	    (flags & WINDOW_TYPE);
	win->spacing = 4;
	win->redraw = 0;
	win->focus = NULL;
	win->caption = NULL;
	win->caption_s = NULL;
	win->caption_color = SDL_MapRGB(view->v->format, 255, 255, 255);
	win->minw = minw;
	win->minh = minh;

	window_titlebar_printf(win, "%s", caption);

	/* Set the initial window position/geometry. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		if (flags & WINDOW_SCALE) {
			window_round(win,
			     rx * view->w / 100,
			     ry * view->h / 100,
			     rw * view->w / 100,
			     rh * view->h / 100);
		} else {
			window_round(win, rx, ry, rw, rh);
		}
		if (flags & WINDOW_CENTER) {
			window_round(win,
			    view->w/2 - win->w/2,
			    view->h/2 - win->h/2,
			    win->w, win->h);
		}
		/* Clamp to view area and leave a margin. */
		window_clamp(win, TILEW, TILEH);
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
		if (flags & WINDOW_CENTER) {
			win->x = view->w/2 - win->w/2;
			win->y = view->h/2 - win->h/2;
		}
		/* Clamp to view area and leave a margin. */
		window_clamp(win, 16, 16);
		break;
	}

	/* Primitive operations will need this. */
	win->wid.win = win;
	win->wid.x = 0;
	win->wid.y = 0;
	win->wid.w = 0;
	win->wid.h = 0;

	/* Force animation for relevant backgrounds. */
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

/*
 * Render a window.
 * Window must be locked.
 */
void
window_draw(struct window *win)
{
	SDL_Rect rd;
	SDL_Surface *v = view->v;
	struct region *reg;
	struct widget *wid;
	int i;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = win->w;
	rd.h = win->h;

	switch (win->type) {
	default:
		SDL_FillRect(v, &rd, win->bgcolor);
		break;
	}

	for (i = 1; i < win->borderw; i++) {
		primitives.line(win,		/* Top */
		    i, i,
		    win->w - i, i,
		    win->border[i]);
		primitives.line(win,		/* Bottom */
		    i, win->h - i,
		    win->w - i, win->h - i,
		    win->border[i]);
		primitives.line(win,		/* Left */
		    i, i,
		    i, win->h - i,
		    win->border[i]);
		primitives.line(win,		/* Right */
		    win->w - i, i,
		    win->w - i, win->h - i,
		    win->border[i]);
	}

	/* Render the title bar. */
	if (win->flags & WINDOW_TITLEBAR) {
		SDL_Rect rd;

		/* Caption */
		rd.x = win->x + (win->w - win->caption_s->w - win->borderw);
		rd.y = win->y + win->borderw;
		SDL_BlitSurface(win->caption_s, NULL, v, &rd);

		/* Close button */
		rd.x = win->x + win->borderw;
		rd.y = win->y + win->borderw;
		SDL_BlitSurface(SPRITE(win, 0), NULL, view->v, &rd);
		
		/* Border */
		primitives.line(win,
		    win->borderw, win->titleh+2,
		    win->w-win->borderw, win->titleh+2,
		    win->border[3]);
		primitives.line(win,
		    win->borderw, win->titleh+3,
		    win->w-win->borderw, win->titleh+3,
		    win->border[1]);
	}

	/* Render the widgets. */
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		if (config->widget_flags & CONFIG_REGION_BORDERS) {
			primitives.square(win,
			    reg->x, reg->y,
			    reg->w, reg->h,
			    SDL_MapRGB(view->v->format, 255, 255, 255));
		}

		/* Draw the widgets. */
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			WIDGET_OPS(wid)->widget_draw(wid);
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

#if 0
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
#else
	win->redraw = 0;
#endif
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
			if (WIDGET_OPS(wid)->widget_animate != NULL) {
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

	if (view->gfx_engine != GFX_ENGINE_GUI)		/* XXX */
		pthread_mutex_lock(&view->lock);

	pthread_mutex_lock(&win->lock);
	rv = window_show_locked(win);
	pthread_mutex_unlock(&win->lock);

	if (view->gfx_engine != GFX_ENGINE_GUI)		/* XXX */
		pthread_mutex_unlock(&view->lock);

	return (rv);
}

/* View and window must not be locked. */
int
window_hide(struct window *win)
{
	int rv;

	if (view->gfx_engine != GFX_ENGINE_GUI)		/* XXX */
		pthread_mutex_lock(&view->lock);

	pthread_mutex_lock(&win->lock);
	rv = window_hide_locked(win);
	pthread_mutex_unlock(&win->lock);

	if (view->gfx_engine != GFX_ENGINE_GUI)		/* XXX */
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

	if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
		window_update_mask(win);
	}

	view->focus_win = win;
	
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "widget-shown", "%p", win);
		}
	}
	window_resize(win);
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
			event_post(wid, "widget-hidden", "%p", win);
		}
	}
	
	win->flags &= ~(WINDOW_SHOWN);

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		rootmap_maskfill(&win->vmask, -1);
		break;
	case GFX_ENGINE_GUI:
		{ 
			SDL_Rect rd;

			rd.x = win->x;
			rd.y = win->y;
			rd.w = win->w;
			rd.h = win->h;

			SDL_FillRect(view->v, &rd, NULL);
			SDL_UpdateRect(view->v, rd.x, rd.y, rd.w, rd.h);
		}
		break;
	}

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

/* Update the map view mask in tile-based mode. */
static void
window_update_mask(struct window *win)
{
	win->vmask.x = (win->x / TILEW);
	win->vmask.y = (win->y / TILEH);
	win->vmask.w = (win->w / TILEW);
	win->vmask.h = (win->h / TILEW);
	rootmap_maskfill(&win->vmask, 1);
}

/* View and window must be locked. */
static void
window_move(struct window *win, SDL_MouseMotionEvent *motion)
{
	int moved = 0;
	int tilew, tileh;
	SDL_Rect oldpos;

	oldpos.x = win->x;
	oldpos.y = win->y;
	oldpos.w = win->w;
	oldpos.h = win->h;

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		tilew = TILEW;
		tileh = TILEH;
		break;
	default:
		tilew = 16;	/* XXX pref */
		tileh = 16;
		break;
	}

	switch (view->gfx_engine) {
	case GFX_ENGINE_GUI:
		if (motion->xrel != 0 || motion->yrel != 0) {
			win->x += motion->xrel;
			win->y += motion->yrel;
			moved++;
		}
		break;
	case GFX_ENGINE_TILEBASED:
		{
			static Sint16 oldx = 0, oldy = 0;
			Sint16 nx, ny;

			nx = motion->x / tilew;
			ny = motion->y / tileh;

			if (oldx != 0 || oldy != 0) {
				if (nx > oldx) {
					win->x += tilew;
					moved++;
				}
				if (ny > oldy) {
					win->y += tileh;
					moved++;
				}
				if (nx < oldx) {
					win->x -= tilew;
					moved++;
				}
				if (ny < oldy) {
					win->y -= tilew;
					moved++;
				}
			}
			oldx = nx;
			oldy = ny;
		}
		break;
	}

	/* Clamp to view area, leave a margin. */
	window_clamp(win, tilew, tileh);

	if (moved) {
		switch (view->gfx_engine) {
		case GFX_ENGINE_TILEBASED:
			/* Move the tile mask over to the new position. */
			rootmap_maskfill(&win->vmask, -1);
			window_update_mask(win);

			/* Redraw the map. View is already locked. */
			view->rootmap->map->redraw++;
			break;
		case GFX_ENGINE_GUI:
			SDL_FillRect(view->v, &oldpos, bg_color);
			window_draw(win);
			SDL_UpdateRect(view->v, oldpos.x, oldpos.y,
			    oldpos.w, oldpos.h);
			break;
		}
	}
}

/*
 * Give focus to a window.
 * View must be locked.
 */
static void
window_focus(struct window *win)
{
	struct window *lastwin;
	struct region *reg;
	struct widget *wid;

	lastwin = TAILQ_LAST(&view->windowsh, windowq);
	if (win != NULL && lastwin == win) {
		/* The window already holds focus. */
		return;
	}

	if (lastwin != NULL) {
		/* Notify the previous window of the focus change. */
		event_post(lastwin, "window-lostfocus", NULL);

		/* Notify the previous window's widgets of the focus change. */
		TAILQ_FOREACH(reg, &lastwin->regionsh, regions) {
			TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
				event_post(wid, "widget-lostfocus", NULL);
			}
		}
	}

	if (win != NULL) {
		/*
		 * Move the new window at the list tail (so the rendering
		 * functions don't have to traverse the list backwards).
		 */
		TAILQ_REMOVE(&view->windowsh, win, windows);
		TAILQ_INSERT_TAIL(&view->windowsh, win, windows);

		/* Notify the new window of the focus change. */
		event_post(win, "window-gainfocus", NULL);
	}
}

/*
 * Focus windows if necessary.
 * View must be locked, window list must not be empty.
 */
static void
window_mousefocus(Uint16 x, Uint16 y)
{
	struct window *win;

	TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows, windowq) {
		if (WINDOW_INSIDE(win, x, y) && win->flags & WINDOW_SHOWN) {
			view->focus_win = win;
			return;
		}
	}
	view->focus_win = NULL;
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
	static int ox = 0, oy = 0;
	int nx, ny;
	int focus_changed = 0;

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		window_mousefocus(ev->button.x, ev->button.y);
		focus_changed++;
		break;
	case SDL_MOUSEBUTTONUP:
		view->winop = VIEW_WINOP_NONE;
		break;
	}

	TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if ((win->flags & WINDOW_SHOWN) == 0) {
			goto nextwin;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			/* Window mouse motion operation */
			if (view->winop != VIEW_WINOP_NONE &&
			    view->wop_win != win) {
				goto nextwin;
			}
			switch (view->winop) {
			case VIEW_WINOP_MOVE:
				window_move(win, &ev->motion);
				goto posted;
			case VIEW_WINOP_LRESIZE:
			case VIEW_WINOP_RRESIZE:
			case VIEW_WINOP_HRESIZE:
				/* Resize the window. */
				winop_resize(view->winop, win, &ev->motion);
				goto posted;
			case VIEW_WINOP_NONE:
			}

			/* Widget mouse motion event */
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
					    "%i, %i, %i, %i",
					    (int)ev->motion.x -
					     (wid->x + wid->win->x),
					    (int)ev->motion.y -
					     (wid->y + wid->win->y),
					    (int)ev->motion.xrel,
					    (int)ev->motion.yrel);
					goto posted;
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto nextwin;
			}

			/* Window mouse button operation */
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
					    win->w - 17) {
						view->winop =
						    VIEW_WINOP_RRESIZE;
					} else {
						view->winop = 
						    VIEW_WINOP_HRESIZE;
					}
					view->wop_win = win;
				}	
			}

			/* Widget mouse button event */
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

			if (focus_changed) {
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

			/* Tab cycling */
			if (ev->key.keysym.sym == SDLK_TAB &&
			    ev->type == SDL_KEYUP) {
				cycle_widgets(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				win->redraw++;
				goto posted;
			}

			/* Widget event */
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

	/*
	 * The focus_changed flag is set if there was a focus change
	 * in reaction to a window operation. The focus_win variable
	 * may also be changed by window show/hide functions.
	 */
	if (focus_changed || view->focus_win != NULL) {
		/* Reorder the window list. */
		window_focus(view->focus_win);
		view->focus_win = NULL;
	}
	return (1);
}

static void
window_clamp(struct window *win, int minw, int minh)
{
	if (win->x < minw)
		win->x = minw;
	if (win->y < minh)
		win->y = minh;
	if (win->x+win->w > view->w-minw)
		win->x = view->w - win->w - minw;
	if (win->y+win->h > view->h-minh)
		win->y = view->h - win->h - minh;
}

/*
 * Resize a window with the mouse.
 * Window must be locked, config must not be locked by the caller thread.
 */
static void
winop_resize(int op, struct window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect ro;
	int nx, ny;

	ro.x = win->x;
	ro.y = win->y;
	ro.w = win->w;
	ro.h = win->h;
	SDL_FillRect(view->v, &ro, 0);

	nx = win->x;
	ny = win->y;

	/* Resize the window accordingly. */
	switch (op) {
	case VIEW_WINOP_LRESIZE:
		if (motion->xrel < 0) {
			win->w -= motion->xrel;
			nx = win->x + motion->xrel;
		} else if (motion->xrel > 0) {
			win->w -= motion->xrel;
			nx = win->x + motion->xrel;
		}
		if (motion->yrel < 0 || motion->yrel > 0) {
			win->h += motion->yrel;
		}
		break;
	case VIEW_WINOP_RRESIZE:
		if (motion->xrel < 0 || motion->xrel > 0) {
			win->w += motion->xrel;
		}
		if (motion->yrel < 0 || motion->yrel > 0) {
			win->h += motion->yrel;
		}
		break;
	case VIEW_WINOP_HRESIZE:
		if (motion->yrel < 0 || motion->yrel > 0) {
			win->h += motion->yrel;
		}
		default:
	}

	/* Clamp to minimum window geometry. */
	pthread_mutex_lock(&config->lock);
	if (win->w < win->minw &&
	   (config->widget_flags & CONFIG_WINDOW_ANYSIZE) == 0) {
		win->w = win->minw;
	} else {
		win->x = nx;
	}
	if (win->h < win->minh &&
	   (config->widget_flags & CONFIG_WINDOW_ANYSIZE) == 0) {
		win->h = win->minh;
	} else {
		win->y = ny;
	}
	pthread_mutex_unlock(&config->lock);

	/* Clamp to maximum window geometry. */
	if (win->x+win->w > view->w - 16) {
		win->x = ro.x;
		win->w = ro.w;
	}
	if (win->y+win->h > view->h - 16) {
		win->y = ro.y;
		win->h = ro.h;
	}

	/* Effect the change. */
	window_resize(win);
	window_draw(win);
	SDL_UpdateRect(view->v, ro.x, ro.y, ro.w, ro.h);
}

/* Window must be locked. */
void
window_resize(struct window *win)
{
	struct region *reg;
	int minw, minh;

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		minw = TILEW;
		minh = TILEH;
		rootmap_maskfill(&win->vmask, -1);
		break;
	default:
		minw = 16;
		minh = 16;
		break;
	}

	/* Clamp to view area, leave a margin. */
	window_clamp(win, minw, minh);

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
			int rw, rh;

			wid->x = x;
			wid->y = y;

			rw = wid->rw;
			rh = wid->rh;

			if (rw == 0) {
				rw = (reg->flags & REGION_HALIGN) ?
				    100 / (nwidgets + 2) : 100;
			}
			if (rh == 0) {
				rh = (reg->flags & REGION_VALIGN) ?
				    100 / (nwidgets + 2) : 100;
			}

			if (rw >= 0)
				wid->w = rw * reg->w/100;
			if (rh >= 0)
				wid->h = rh * reg->h/100;

			if (wid->w > reg->w)
				wid->w = reg->w;
			if (wid->h > reg->h)
				wid->h = reg->h;

			event_post(wid, "widget-scaled", "%i, %i",
			    reg->w, reg->h);
			
			/* Space widgets */
			if (reg->flags & REGION_VALIGN) {
				if (rw > 0) {
					if (TAILQ_LAST(&reg->widgetsh,
					    widgetsq) == wid) {
						wid->h -= reg->spacing/2;
					}
					wid->h -= reg->spacing/nwidgets;
				}
				y += wid->h + reg->spacing;
			} else {
				if (rw > 0) {
					if (TAILQ_LAST(&reg->widgetsh,
					    widgetsq) == wid) {
						wid->w -= reg->spacing/2;
					}
					wid->w -= reg->spacing/nwidgets;
				}
				x += wid->w + reg->spacing;
			}
		}
	}
	if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
		window_update_mask(win);
	}

#ifdef DEBUG
	if (config->widget_flags & CONFIG_WINDOW_ANYSIZE) {
		dprintf("%s: %d, %d\n", OBJECT(win)->name, win->w, win->h);
	}
#endif
}

/* Window must be locked. */
void
window_titlebar_printf(struct window *win, const char *fmt, ...)
{
	va_list args;

	/* XXX */
	if (win->caption != NULL) {
		free(win->caption);
		win->caption = NULL;
	}
	va_start(args, fmt);
	vasprintf(&win->caption, fmt, args);
	va_end(args);

	if (win->caption_s != NULL) {
		SDL_FreeSurface(win->caption_s);
	}
	win->caption_s = text_render(NULL, -1, win->caption_color,
	    win->caption);
}


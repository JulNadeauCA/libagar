/*	$Csoft: window.c,v 1.91 2002/11/09 07:39:03 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
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

#include <engine/compat/asprintf.h>
#include <engine/compat/vasprintf.h>

#include <sys/types.h>

#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libfobj/fobj.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/config.h>
#include <engine/version.h>

#include <engine/mapedit/mapedit.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "primitive.h"

static const struct version window_ver = {
	"agar window",
	1, 0
};

static const struct object_ops window_ops = {
	window_destroy,
	window_load,
	window_save
};

enum {
	BACKGROUND_COLOR,
	TITLEBAR_FOCUSED_COLOR,
	TITLEBAR_UNFOCUSED_COLOR,
	TITLEBAR_TEXT_UNFOCUSED_COLOR,
	TITLEBAR_TEXT_FOCUSED_COLOR
};

static Uint32 bg_color = 0;

/* XXX struct */
#include "borders/green1.h"

static void	window_move(struct window *, SDL_MouseMotionEvent *);
static void	window_update_mask(struct window *);
static void	window_clamp(struct window *);
static void	window_round(struct window *, int, int, int, int);
static void	window_update_titlebar(struct window *);
static void	winop_resize(int, struct window *, SDL_MouseMotionEvent *);
static void	window_focus(struct window *);

struct window *
window_new(char *name, char *caption, int flags, int x, int y, int w, int h,
    int minw, int minh)
{
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, name, caption, flags, x, y, w, h, minw, minh);

	/* Attach window to main view, make it visible. */
	pthread_mutex_lock(&view->lock);
	view_attach(win);
	pthread_mutex_unlock(&view->lock);

	return (win);
}

/*
 * Adjust window to tile granularity.
 * Window must be locked.
 */
static void
window_round(struct window *win, int x, int y, int w, int h)
{
	win->x = x - (x % TILEW);
	win->y = y - (y % TILEH);
	win->w = w - (w % TILEW);
	win->h = h - (h % TILEH);
}

void
window_init(struct window *win, char *name, char *caption, int flags,
    int rx, int ry, int rw, int rh, int minw, int minh)
{
	char *wname;
	int i;
	int fl = flags;

	if (bg_color == 0) {
		/* Set the background color. */
		bg_color = SDL_MapRGB(view->v->format, 0, 0, 0);
	}
	
	if (name != NULL) {					/* Unique */
		asprintf(&wname, "win-%s", name);
		fl |= WINDOW_SAVE_POSITION;
	} else {						/* Generic */
		static int curwindow = 0;
		static pthread_mutex_t curwindow_lock =
		    PTHREAD_MUTEX_INITIALIZER;

		pthread_mutex_lock(&curwindow_lock);
		curwindow++;
		pthread_mutex_unlock(&curwindow_lock);

		asprintf(&wname, "win-generic%d", curwindow++);
	}

	object_init(&win->wid.obj, "window", wname, "window",
	    OBJECT_ART|OBJECT_KEEP_MEDIA, &window_ops);
	free(wname);
	
	widget_map_color(&win->wid, BACKGROUND_COLOR,
	    "window-background",
	    0, 40, 20);

	widget_map_color(&win->wid, TITLEBAR_UNFOCUSED_COLOR,
	    "window-title-bar-background-unfocused",
	    0, 60, 40);
	widget_map_color(&win->wid, TITLEBAR_TEXT_UNFOCUSED_COLOR,
	    "window-title-bar-text-unfocused",
	    20, 100, 100);

	widget_map_color(&win->wid, TITLEBAR_FOCUSED_COLOR,
	    "window-title-bar-background-focused",
	    0, 90, 90);
	widget_map_color(&win->wid, TITLEBAR_TEXT_FOCUSED_COLOR,
	    "window-title-bar-text-focused",
	    80, 200, 200);

	/* XXX pref */
	win->borderw = default_nborder;
	win->border = emalloc(win->borderw * sizeof(Uint32));
	for (i = 0; i < win->borderw; i++) {
		win->border[i] = SDL_MapRGB(view->v->format,
		    default_border[i].r, default_border[i].g,
		    default_border[i].b);
	}

	fl |= WINDOW_TITLEBAR;

	win->titleh = font_h + win->borderw;
	win->flags = fl;
	win->type = ((fl & WINDOW_TYPE) == 0) ?
	    WINDOW_DEFAULT_TYPE : (fl & WINDOW_TYPE);
	win->spacing = 4;
	win->redraw = 0;
	win->focus = NULL;
	win->caption = NULL;
	win->minw = minw;
	win->minh = minh;

	window_titlebar_printf(win, "%s", caption);

	/* Set the initial window position/geometry. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		if (win->flags & WINDOW_SCALE) {
			window_round(win,
			     rx * view->w / 100,
			     ry * view->h / 100,
			     rw * view->w / 100,
			     rh * view->h / 100);
		} else {
			window_round(win, rx, ry, rw, rh);
		}
		if (win->flags & WINDOW_CENTER) {
			window_round(win,
			    view->w/2 - win->w/2,
			    view->h/2 - win->h/2,
			    win->w, win->h);
		}
		/* Clamp to view area and leave a margin. */
		window_clamp(win);
		break;
	case GFX_ENGINE_GUI:
		if (win->flags & WINDOW_SCALE) {
			win->x = rx * view->w / 100;
			win->y = ry * view->h / 100;
			win->w = rw * view->w / 100;
			win->h = rh * view->h / 100;
		} else {
			win->x = rx;
			win->y = ry;
			win->w = rw;
			win->h = rh;
		}
		if (win->flags & WINDOW_CENTER) {
			win->x = view->w/2 - win->w/2;
			win->y = view->h/2 - win->h/2;
		}
		break;
	}
	
	/* Clamp to view area and leave a margin. */
	window_clamp(win);

	/* Primitive operations will need this. */
	win->wid.win = win;
	win->wid.x = 0;
	win->wid.y = 0;
	win->wid.w = 0;
	win->wid.h = 0;

	TAILQ_INIT(&win->regionsh);
	pthread_mutexattr_init(&win->lockattr);
	pthread_mutexattr_settype(&win->lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&win->lock, &win->lockattr);
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

	SDL_FillRect(v, &rd, WIDGET_COLOR(win, BACKGROUND_COLOR));

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
		SDL_Rect bgrd;
		SDL_Surface *caption;

		/* XXX yuck */
		bgrd.x = win->x + win->borderw;
		bgrd.y = win->y + win->borderw;
		bgrd.w = win->w - win->borderw*2+1;
		bgrd.h = win->titleh - win->borderw/2;

		/* Titlebar background */
		SDL_FillRect(view->v, &bgrd,
		    WIDGET_COLOR(win, VIEW_FOCUSED(win) ?
		    TITLEBAR_FOCUSED_COLOR : TITLEBAR_UNFOCUSED_COLOR));
	
		/* Caption */
		caption = text_render(NULL, -1,
		    WIDGET_COLOR(win, VIEW_FOCUSED(win) ?
		    TITLEBAR_TEXT_FOCUSED_COLOR :
		    TITLEBAR_TEXT_UNFOCUSED_COLOR),
		    win->caption);
		rd.x = win->x + (win->w - caption->w - win->borderw);
		rd.y = win->y + win->borderw;
		SDL_BlitSurface(caption, NULL, v, &rd);
		SDL_FreeSurface(caption);

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

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
#if 0
			if (WIDGET_OPS(wid)->widget_update != NULL) {
				/* Indirect rendering method */
				pthread_mutex_lock(&wid->surface.lock);

				if (wid->surface.redraw != 0) {
					wid->surface.redraw = 0;
					WIDGET_OPS(wid)->widget_update(wid);
				}

				/* XXX move rect to widget structure */
				rd.x = WIDGET_ABSX(wid);
				rd.y = WIDGET_ABSY(wid);
				rd.w = wid->w;
				rd.h = wid->h;

				SDL_BlitSurface(wid->surface.source, NULL,
				    view->v, &rd);
				pthread_mutex_unlock(&wid->surface.lock);
			} else if (WIDGET_OPS(wid)->widget_draw != NULL) {
#endif
				/* Direct rendering method */
				WIDGET_OPS(wid)->widget_draw(wid);
#if 0
			}
#endif
		}

#ifdef DEBUG
		if (prop_uint32(config, "widgets.flags") &
		    CONFIG_REGION_BORDERS) {
			primitives.square(win,
			    reg->x, reg->y,
			    reg->w, reg->h,
			    SDL_MapRGB(view->v->format, 255, 255, 255));
		}
#endif
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
	win->redraw = 0;
#endif
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
 * Detach a region from this window.
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
	pthread_mutexattr_destroy(&win->lockattr);
}

int
window_show(struct window *win)
{
	struct region *reg;
	struct widget *wid;

	pthread_mutex_lock(&view->lock);
	pthread_mutex_lock(&win->lock);

	if (win->flags & WINDOW_SHOWN) {		/* Already visible? */
		pthread_mutex_unlock(&win->lock);
		pthread_mutex_unlock(&view->lock);
		return (1);
	}
	win->flags |= WINDOW_SHOWN;

	if (win->flags & WINDOW_SAVE_POSITION) {
		/* Try to load previously saved window geometry/coordinates. */
		object_load(win);
	}

	if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
		window_update_mask(win);
	}

	view->focus_win = win;		/* Focus */

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "widget-shown", "%p", win);
		}
	}

	window_focus(win);
	window_resize(win);		/* In case the window is new. */

	pthread_mutex_unlock(&win->lock);
	pthread_mutex_unlock(&view->lock);
	return (0);
}

int
window_hide(struct window *win)
{
	struct region *reg;
	struct widget *wid;

	pthread_mutex_lock(&view->lock);
	pthread_mutex_lock(&win->lock);

	if ((win->flags & WINDOW_SHOWN) == 0) {		/* Already hidden? */
		pthread_mutex_unlock(&win->lock);
		pthread_mutex_unlock(&view->lock);
		return (0);
	}

	/* XXX cycle focus */
	view->focus_win = NULL;

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

			SDL_FillRect(view->v, &rd, 0);
			SDL_UpdateRect(view->v, rd.x, rd.y, rd.w, rd.h);
		}
		break;
	}

	if (win->flags & WINDOW_SAVE_POSITION) {
		/* Save the window state. */
		object_save(win);
	}

	pthread_mutex_unlock(&win->lock);
	pthread_mutex_unlock(&view->lock);
	return (1);
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
 * Update the map view mask in tile-based mode.
 * Window must be locked.
 */
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
	SDL_Rect oldpos;
	int moved = 0;

	oldpos.x = win->x;
	oldpos.y = win->y;
	oldpos.w = win->w;
	oldpos.h = win->h;

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

			nx = motion->x / TILEW;
			ny = motion->y / TILEH;

			if (oldx != 0 || oldy != 0) {
				if (nx > oldx) {
					win->x += TILEW;
					moved++;
				}
				if (ny > oldy) {
					win->y += TILEH;
					moved++;
				}
				if (nx < oldx) {
					win->x -= TILEW;
					moved++;
				}
				if (ny < oldy) {
					win->y -= TILEW;
					moved++;
				}
			}
			oldx = nx;
			oldy = ny;
		}
		break;
	}

	/* Clamp to view area, leave a margin. */
	window_clamp(win);

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
			{
				SDL_Rect nrd;

				if (win->x > oldpos.x) {	/* Right */
					nrd.x = oldpos.x;
					nrd.y = oldpos.y;
					nrd.w = win->x - oldpos.x;
					nrd.h = win->h;
					SDL_FillRect(view->v, &nrd, bg_color);
					SDL_UpdateRect(view->v, nrd.x,
					    nrd.y, nrd.w, nrd.h);
				}
				if (win->y > oldpos.y) {	/* Down */
					nrd.x = oldpos.x;
					nrd.y = oldpos.y;
					nrd.w = win->w;
					nrd.h = win->y - oldpos.y;
					SDL_FillRect(view->v, &nrd, bg_color);
					SDL_UpdateRect(view->v, nrd.x,
					    nrd.y, nrd.w, nrd.h);
				}
				if (win->x < oldpos.x) {	/* Left */
					nrd.x = win->x + win->w;
					nrd.y = win->y;
					nrd.w = oldpos.x - win->x;
					nrd.h = oldpos.h;
					SDL_FillRect(view->v, &nrd, bg_color);
					SDL_UpdateRect(view->v, nrd.x,
					    nrd.y, nrd.w, nrd.h);
				}
				if (win->y < oldpos.y) {	/* Up */
					nrd.x = oldpos.x;
					nrd.y = win->y + win->h;
					nrd.w = oldpos.w;
					nrd.h = oldpos.y - win->y;
					SDL_FillRect(view->v, &nrd, bg_color);
					SDL_UpdateRect(view->v, nrd.x,
					    nrd.y, nrd.w, nrd.h);
				}
			}
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
	struct window *lastwin, *ow;
	struct region *reg;
	struct widget *wid;

	lastwin = TAILQ_LAST(&view->windows, windowq);
	if (win != NULL && lastwin == win) {		/* Already focused? */
		return;
	}

	if (lastwin != NULL) {
#if 0
		if (lastwin->focus != NULL) {
			/* Take the focus off the widget. */
			event_post(lastwin->focus, "widget-lostfocus", NULL);
			lastwin->focus = NULL;
		}
#endif
		/* Notify the previous window of the focus change. */
		event_post(lastwin, "window-lostfocus", NULL);
	}

	if (win != NULL) {
		/*
		 * Move the new window at the list tail (so the rendering
		 * functions don't have to traverse the list backwards).
		 */
		TAILQ_REMOVE(&view->windows, win, windows);
		TAILQ_INSERT_TAIL(&view->windows, win, windows);
		
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

	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
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
window_event(SDL_Event *ev)
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

	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if ((win->flags & WINDOW_SHOWN) == 0) {
			goto nextwin;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			/*
			 * Window mouse motion operation
			 */
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
			/*
			 * Widget mouse motion event
			 */
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
					if ((VIEW_FOCUSED(win) &&
					     WIDGET_FOCUSED(wid)) ||
					    (wid->flags &
					     WIDGET_UNFOCUSED_MOTION)) {
						event_post(wid,
						    "window-mousemotion",
						    "%i, %i, %i, %i",
						    (int)ev->motion.x -
						     (wid->x + wid->win->x),
						    (int)ev->motion.y -
						     (wid->y + wid->win->y),
						    (int)ev->motion.xrel,
						    (int)ev->motion.yrel);
					}
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			/* Cancel window operations. */ 
			view->winop = VIEW_WINOP_NONE;
			view->wop_win = NULL;
			/*
			 * Widget mouse button up event
			 */
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
					if ((VIEW_FOCUSED(win) &&
					     WIDGET_FOCUSED(wid)) ||
					    (wid->flags &
					     WIDGET_UNFOCUSED_BUTTONUP)) {
						event_post(wid,
						    "window-mousebuttonup",
						    "%i, %i, %i",
						    ev->button.button,
						    ev->button.x -
						    (wid->x+wid->win->x),
						    ev->button.y -
						    (wid->y+wid->win->y));
					}
				}
			}
			if (focus_changed) {
				goto posted;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto nextwin;
			}
			/*
			 * Window mouse button down operation
			 */
			if (ev->button.y - win->y <= win->titleh) {
			    	if (ev->button.x - win->x < 20) { /* XXX */
					window_hide(win);
					event_post(win, "window-close", NULL);
				}
				view->winop = VIEW_WINOP_MOVE;
				view->wop_win = win;
			} else if (ev->button.y-win->y > win->h-win->borderw) {
			    	if (ev->button.x-win->x < 17) {
					view->winop = VIEW_WINOP_LRESIZE;
				} else if (ev->button.x-win->x > win->w-17) {
					view->winop = VIEW_WINOP_RRESIZE;
				} else {
					view->winop = VIEW_WINOP_HRESIZE;
				}
				view->wop_win = win;
			}
			/*
			 * Widget mouse button down event
			 */
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
					if (!WIDGET_INSIDE(wid, ev->button.x,
					    ev->button.y)) {
						continue;
					}
					event_post(wid,
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
			switch (ev->key.keysym.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				/* Always ignore modifiers */
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
			if (VIEW_FOCUSED(win) && win->focus != NULL) {
				event_post(win->focus,
				    (ev->type == SDL_KEYUP) ?
				    "window-keyup" :
				    "window-keydown",
				    "%i, %i",
				    (int)ev->key.keysym.sym,
				    (int)ev->key.keysym.mod);
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

/* Window must be locked. */
static void
window_clamp(struct window *win)
{
	if (win->x < view->margin.w)
		win->x = view->margin.w;
	if (win->y < view->margin.h)
		win->y = view->margin.h;
	if (win->x+win->w > view->w - view->margin.w)
		win->x = view->w - win->w - view->margin.w;
	if (win->y+win->h > view->h - view->margin.h)
		win->y = view->h - win->h - view->margin.h;
}

/*
 * Resize a window with the mouse.
 * Window must be locked.
 */
static void
winop_resize(int op, struct window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect ro, rn;
	int nx, ny;
	int tw = 16, th = 16;

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
	if (win->w < win->minw &&
	   (prop_uint32(config, "widgets.flags") & CONFIG_WINDOW_ANYSIZE)
	    == 0) {
		win->w = win->minw;
	} else {
		win->x = nx;
	}
	if (win->h < win->minh &&
	   (prop_uint32(config, "widgets.flags") & CONFIG_WINDOW_ANYSIZE)
	    == 0) {
		win->h = win->minh;
	} else {
		win->y = ny;
	}
	
	if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
		tw = TILEW;
		th = TILEH;
	}
	if (win->x < tw) {
		win->x = tw;
	}
	if (win->y < th) {
		win->y = th;
	}

	/* Clamp to view boundaries. */
	if (win->x+win->w > view->w - tw) {
		win->x = ro.x;
		win->w = ro.w;
	}
	if (win->y+win->h > view->h - th) {
		win->y = ro.y;
		win->h = ro.h;
	}

	/* Effect the change. */
	window_resize(win);

	/* Rectangle at the left (lresize operation). */
	if (win->x > ro.x) {
		SDL_UpdateRect(view->v,
		    ro.x, ro.y,
		    win->x-ro.x, win->h);
	}
	/* Rectangle at the right (rresize and hresize ops). */
	if (win->w < ro.w) {
		SDL_UpdateRect(view->v,
		    win->x + win->w, win->y,
		    ro.w - win->w, ro.h);
	}
	/* Rectangle at the bottom (rresize and hresize ops). */
	if (win->h < ro.h) {
		SDL_UpdateRect(view->v,
		    win->x, win->y + win->h,
		    ro.w, ro.h - win->h);
	}
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
	window_clamp(win);

	win->body.x = win->x + win->borderw;
	win->body.y = win->y + win->borderw*2 + win->titleh;
	win->body.w = win->w - win->borderw*2;
	win->body.h = win->h - win->borderw*2 - win->titleh;

	win->wid.w = win->w;
	win->wid.h = win->h;

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
	if (prop_uint32(config, "widgets.flags") & CONFIG_WINDOW_ANYSIZE) {
		dprintf("%s: %d, %d\n", OBJECT(win)->name, win->w, win->h);
	}
#endif
}

static void
window_update_titlebar(struct window *win)
{
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

	window_update_titlebar(win);
}

int
window_load(void *p, int fd)
{
	struct window *win = p;

	if (version_read(fd, &window_ver) != 0) {
		return (-1);
	}

	win->x = read_uint32(fd);
	win->y = read_uint32(fd);
	win->w = read_uint32(fd);
	win->h = read_uint32(fd);

	/* XXX scale */

	/* Ensure the window fits inside the view area. */
	window_resize(win);

	return (0);
}

int
window_save(void *p, int fd)
{
	struct window *win = p;

	version_write(fd, &window_ver);
	write_uint32(fd, win->x);
	write_uint32(fd, win->y);
	write_uint32(fd, win->w);
	write_uint32(fd, win->h);
	
	return (0);
}

void
window_detach_generic(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	OBJECT_ASSERT(win, "window");
	view_detach(win);
}

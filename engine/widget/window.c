/*	$Csoft: window.c,v 1.108 2002/11/22 05:41:45 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/compat/asprintf.h>
#include <engine/compat/vasprintf.h>

#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/config.h>
#include <engine/version.h>
#include <engine/view.h>
#include <engine/mapedit/mapedit.h>

#include <libfobj/fobj.h>

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

static void	window_clamp(struct window *);
static void	window_round(struct window *, int, int, int, int);
static void	window_focus(struct window *);
static void	winop_move(struct window *, SDL_MouseMotionEvent *);
static void	winop_resize(int, struct window *, SDL_MouseMotionEvent *);

struct window *
window_new(char *name, int flags, int x, int y, int w, int h,
    int minw, int minh)
{
	struct window *win;

	win = emalloc(sizeof(struct window));
	window_init(win, name, flags, x, y, w, h, minw, minh);

	view_attach(win);			/* Attach to view */
	return (win);
}

struct window *
window_generic_new(int w, int h, const char *name_fmt, ...)
{
	struct window *win;
	va_list args;
	char *name;
	static pthread_mutex_t genlock = PTHREAD_MUTEX_INITIALIZER;
	static int genxoffs = -1, genyoffs = -1;

	if (name_fmt != NULL) {
		va_start(args, name_fmt);
		if (vasprintf(&name, name_fmt, args) == -1) {
			fatal("vasprintf: %s\n", strerror(errno));
		}
		va_end(args);
		
		pthread_mutex_lock(&view->lock);
		TAILQ_FOREACH(win, &view->windows, windows) {
			if (strlen(OBJECT(win)->name) > 4 &&
			    strcmp(OBJECT(win)->name+4, name) == 0) {
				view_focus(win);
				pthread_mutex_unlock(&view->lock);
				return (NULL);
			}
		}
		pthread_mutex_unlock(&view->lock);
	
	} else {
		name = NULL;
	}

	win = emalloc(sizeof(struct window));

	pthread_mutex_lock(&genlock);

	window_init(win, name, 0,
	    view->w/2 - w/2 + genxoffs,
	    view->h/2 - h/2 + genyoffs,
	    w, h, w, h);

	if ((genxoffs += 3) + w > view->w/2)
		genxoffs = 0;
	if ((genyoffs += 3) + h > view->h/2)
		genyoffs = 0;

	pthread_mutex_unlock(&genlock);
	
	free(name);

	/* Destroy the window instead of hiding it on close. */
	event_new(win, "window-close", window_generic_detached, "%p", win);

	view_attach(win);			/* Attach to view */
	return (win);
}

/*
 * Adjust window to tile granularity.
 * Window must be locked.
 */
static void
window_round(struct window *win, int x, int y, int w, int h)
{
	win->rd.x = x - (x % TILEW);
	win->rd.y = y - (y % TILEH);
	win->rd.w = w - (w % TILEW);
	win->rd.h = h - (h % TILEH);
}

void
window_init(struct window *win, char *name, int flags, int rx, int ry,
    int rw, int rh, int minw, int minh)
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

	fl |= (WINDOW_TITLEBAR|WINDOW_MATERIALIZE|WINDOW_DEMATERIALIZE);

	win->titleh = font_h + win->borderw;
	win->flags = fl;
	win->spacing = 4;
	win->focus = NULL;
	win->caption = strdup("Untitled");
	win->minw = minw;
	win->minh = minh;

	/* Set the initial window position/geometry. */
	if (win->flags & WINDOW_SCALE) {
		win->rd.x = rx * view->w / 100;
		win->rd.y = ry * view->h / 100;
		win->rd.w = rw * view->w / 100;
		win->rd.h = rh * view->h / 100;
	} else {
		win->rd.x = rx;
		win->rd.y = ry;
		win->rd.w = rw;
		win->rd.h = rh;
	}
	if (win->flags & WINDOW_CENTER) {
		win->rd.x = view->w/2 - win->rd.w/2;
		win->rd.y = view->h/2 - win->rd.h/2;
	}
	
	/* Clamp down to view area and leave a margin. */
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
	SDL_Surface *v = view->v;
	struct region *reg;
	struct widget *wid;
	int i;

	SDL_FillRect(v, &win->rd, WIDGET_COLOR(win, BACKGROUND_COLOR));

	for (i = 1; i < win->borderw; i++) {
		primitives.line(win,		/* Top */
		    i, i,
		    win->rd.w - i, i,
		    win->border[i]);
		primitives.line(win,		/* Bottom */
		    i, win->rd.h - i,
		    win->rd.w - i, win->rd.h - i,
		    win->border[i]);
		primitives.line(win,		/* Left */
		    i, i,
		    i, win->rd.h - i,
		    win->border[i]);
		primitives.line(win,		/* Right */
		    win->rd.w - i, i,
		    win->rd.w - i, win->rd.h - i,
		    win->border[i]);
	}

	/* Render the title bar. */
	if (win->flags & WINDOW_TITLEBAR) {
		SDL_Surface *caption;
		SDL_Rect rd;

		/* XXX yuck */
		rd.x = win->rd.x + win->borderw;
		rd.y = win->rd.y + win->borderw;
		rd.w = win->rd.w - win->borderw*2+1;
		rd.h = win->titleh - win->borderw/2;

		/* Titlebar background */
		SDL_FillRect(view->v, &rd,
		    WIDGET_COLOR(win, VIEW_FOCUSED(win) ?
		    TITLEBAR_FOCUSED_COLOR : TITLEBAR_UNFOCUSED_COLOR));
		
		rd.w = win->rd.w;
		rd.h = win->rd.h;
		
		/* Caption */
		caption = text_render(NULL, -1,
		    WIDGET_COLOR(win, VIEW_FOCUSED(win) ?
		    TITLEBAR_TEXT_FOCUSED_COLOR :
		    TITLEBAR_TEXT_UNFOCUSED_COLOR),
		    win->caption);
		rd.x = win->rd.x + (win->rd.w - caption->w - win->borderw);
		rd.y = win->rd.y + win->borderw;
		SDL_BlitSurface(caption, NULL, v, &rd);
		SDL_FreeSurface(caption);

		/* Close button */
		rd.x = win->rd.x + win->borderw;
		rd.y = win->rd.y + win->borderw;
		SDL_BlitSurface(SPRITE(win, 0), NULL, view->v, &rd);
		
		/* Border */
		primitives.line(win,
		    win->borderw, win->titleh+2,
		    win->rd.w-win->borderw, win->titleh+2,
		    win->border[3]);
		primitives.line(win,
		    win->borderw, win->titleh+3,
		    win->rd.w-win->borderw, win->titleh+3,
		    win->border[1]);
	}

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			WIDGET_OPS(wid)->widget_draw(wid);
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

	/* Queue the video update. */
	VIEW_UPDATE(win->rd);
}

/* Attach a region to this window. */
void
window_attach(void *parent, void *child)
{
	struct window *win = parent;
	struct region *reg = child;

	OBJECT_ASSERT(parent, "window");
	OBJECT_ASSERT(child, "window-region");

	reg->win = win;

	pthread_mutex_lock(&win->lock);
	TAILQ_INSERT_HEAD(&win->regionsh, reg, regions);
	pthread_mutex_unlock(&win->lock);
}

/* Detach a region from this window. */
void
window_detach(void *parent, void *child)
{
	struct window *win = parent;
	struct region *reg = child;

	OBJECT_ASSERT(parent, "window");
	OBJECT_ASSERT(child, "window-region");

	pthread_mutex_lock(&win->lock);
	TAILQ_REMOVE(&win->regionsh, reg, regions);
	pthread_mutex_unlock(&win->lock);
}

void
window_destroy(void *p)
{
	struct window *win = p;
	struct region *reg, *nextreg = NULL;

	OBJECT_ASSERT(win, "window");

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

	view->focus_win = win;		/* Focus */

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "widget-shown", "%p", win);
		}
	}
	
	window_focus(win);
	window_resize(win);		/* In case the window is new. */

	if (win->flags & WINDOW_MATERIALIZE) {
		primitive_sequence(win, PRIMITIVE_SEQ_MATERIALIZE);
	}

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

	if (win->flags & WINDOW_DEMATERIALIZE) {
		primitive_sequence(win, PRIMITIVE_SEQ_DEMATERIALIZE);
	}

	/* XXX cycle focus */
	view->focus_win = NULL;

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
			event_post(wid, "widget-hidden", "%p", win);
		}
	}
	
	win->flags &= ~(WINDOW_SHOWN);

	/* Update the background. */
	if (view->gfx_engine == GFX_ENGINE_GUI) {
		SDL_FillRect(view->v, &win->rd, bg_color);
		SDL_UpdateRect(view->v, win->rd.x, win->rd.y,
		    win->rd.w, win->rd.h);
	} else {
		if (view->rootmap != NULL) {		/* Redraw the map. */
			view->rootmap->map->redraw++;
		}
	}

	if (win->flags & WINDOW_SAVE_POSITION) {
		/* Save the window position and geometry. */
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

/* View and window must be locked. */
static void
winop_move(struct window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect oldpos;

	/* Save the old window position in GUI mode. */
	if (view->gfx_engine == GFX_ENGINE_GUI) {
		oldpos = win->rd;		/* Structure copy */
	}

	/* Update the window coordinates, adjust to view area. */
	win->rd.x += motion->xrel;
	win->rd.y += motion->yrel;
	window_clamp(win);

	/* Update around the window in GUI mode. */
	if (view->gfx_engine == GFX_ENGINE_GUI) {
		SDL_Rect nrd;

		if (win->rd.x > oldpos.x) {	/* Right */
			nrd.x = oldpos.x;
			nrd.y = oldpos.y;
			nrd.w = win->rd.x - oldpos.x;
			nrd.h = win->rd.h;
			SDL_FillRect(view->v, &nrd, bg_color);
			SDL_UpdateRect(view->v, nrd.x,
			    nrd.y, nrd.w, nrd.h);
		}
		if (win->rd.y > oldpos.y) {	/* Down */
			nrd.x = oldpos.x;
			nrd.y = oldpos.y;
			nrd.w = win->rd.w;
			nrd.h = win->rd.y - oldpos.y;
			SDL_FillRect(view->v, &nrd, bg_color);
			SDL_UpdateRect(view->v, nrd.x,
			    nrd.y, nrd.w, nrd.h);
		}
		if (win->rd.x < oldpos.x) {	/* Left */
			nrd.x = win->rd.x + win->rd.w;
			nrd.y = win->rd.y;
			nrd.w = oldpos.x - win->rd.x;
			nrd.h = oldpos.h;
			SDL_FillRect(view->v, &nrd, bg_color);
			SDL_UpdateRect(view->v, nrd.x,
			    nrd.y, nrd.w, nrd.h);
		}
		if (win->rd.y < oldpos.y) {	/* Up */
			nrd.x = oldpos.x;
			nrd.y = win->rd.y + win->rd.h;
			nrd.w = oldpos.w;
			nrd.h = oldpos.y - win->rd.y;
			SDL_FillRect(view->v, &nrd, bg_color);
			SDL_UpdateRect(view->v, nrd.x,
			    nrd.y, nrd.w, nrd.h);
		}
	} else if (view->rootmap != NULL) {		/* Tile-based */
		/* Redraw the map. */
		view->rootmap->map->redraw++;
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
		/* XXX */
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
	static struct window *keydown_win = NULL;	/* XXX hack */

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
			if (WINDOW_INSIDE(win, ev->button.x, ev->button.y) &&
			    win->flags & WINDOW_SHOWN) {
				view->focus_win = win;
				focus_changed++;
				goto scan_wins;
			}
		}
		view->focus_win = NULL;
		focus_changed++;
		break;
	case SDL_MOUSEBUTTONUP:
		view->winop = VIEW_WINOP_NONE;
		break;
	}

scan_wins:
	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if ((win->flags & WINDOW_SHOWN) == 0) {
			goto next_win;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			if (view->winop != VIEW_WINOP_NONE &&
			    view->wop_win != win) {
				goto next_win;
			}
			switch (view->winop) {
			case VIEW_WINOP_MOVE:
				winop_move(win, &ev->motion);
				goto posted;
			case VIEW_WINOP_LRESIZE:
			case VIEW_WINOP_RRESIZE:
			case VIEW_WINOP_HRESIZE:
				winop_resize(view->winop, win, &ev->motion);
				goto posted;
			case VIEW_WINOP_NONE:
				break;
			}
			/*
			 * Post the mouse motion event to the widget that
			 * holds the focus inside the focused window, and
			 * to any widget with the WIDGET_UNFOCUSED_MOTION
			 * flag set.
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
						     (wid->x + wid->win->rd.x),
						    (int)ev->motion.y -
						     (wid->y + wid->win->rd.y),
						    (int)ev->motion.xrel,
						    (int)ev->motion.yrel);
					}
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			/* Cancel any current window operation. */ 
			view->winop = VIEW_WINOP_NONE;
			view->wop_win = NULL;
			/*
			 * Send the mouse button release event to the widget
			 * that holds focus inside the focused window, and
			 * any widget with the WIDGET_UNFOCUSED_BUTTONUP flag.
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
						    (wid->x+wid->win->rd.x),
						    ev->button.y -
						    (wid->y+wid->win->rd.y));
					}
				}
			}
			if (focus_changed) {
				goto posted;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto next_win;
			}
			if (ev->button.y - win->rd.y <= win->titleh) {
				/* Close the window. */
			    	if (ev->button.x - win->rd.x < 20) { /* XXX */
					window_hide(win);
					event_post(win, "window-close", NULL);
				}
				view->winop = VIEW_WINOP_MOVE;
				view->wop_win = win;
			} else if (ev->button.y-win->rd.y >
			    win->rd.h-win->borderw) {
				/* Resize the window. */
			    	if (ev->button.x-win->rd.x < 17) {
					view->winop = VIEW_WINOP_LRESIZE;
				} else if (ev->button.x-win->rd.x >
				    win->rd.w-17) {
					view->winop = VIEW_WINOP_RRESIZE;
				} else {
					view->winop = VIEW_WINOP_HRESIZE;
				}
				view->wop_win = win;
			}
			/*
			 * Send the mouse button press event to the
			 * widget under the cursor.
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
					    (wid->x + wid->win->rd.x),
					    ev->button.y -
					    (wid->y + wid->win->rd.y));
					goto posted;
				}
			}
			if (focus_changed) {
				goto posted;
			}
			break;
		case SDL_KEYUP:
			if (keydown_win != NULL && keydown_win != win) {
				/*
				 * Key was initially pressed while another
				 * window was holding focus, ignore.
				 */
				keydown_win = NULL;
				break;
			}
			/* FALLTHROUGH */
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
				/*
				 * Ensure the keyup event is posted to
				 * this window when the key is released,
				 * in case a keydown event handler changes
				 * the window focus.
				 */
				keydown_win = win; 
			}
		}
next_win:
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
	if (win->rd.x < 0)
		win->rd.x = 0;
	if (win->rd.y < 0)
		win->rd.y = 0;
	if (win->rd.x+win->rd.w > view->w)
		win->rd.x = view->w - win->rd.w;
	if (win->rd.y+win->rd.h > view->h)
		win->rd.y = view->h - win->rd.h;
}

/*
 * Resize a window with the mouse.
 * Window must be locked.
 */
static void
winop_resize(int op, struct window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect ro;
	int nx, ny;

	ro = win->rd;		/* Structure copy */

	nx = win->rd.x;
	ny = win->rd.y;

	/* Resize the window accordingly. */
	switch (op) {
	case VIEW_WINOP_LRESIZE:
		if (motion->xrel < 0) {
			win->rd.w -= motion->xrel;
			nx = win->rd.x + motion->xrel;
		} else if (motion->xrel > 0) {
			win->rd.w -= motion->xrel;
			nx = win->rd.x + motion->xrel;
		}
		if (motion->yrel < 0 || motion->yrel > 0) {
			win->rd.h += motion->yrel;
		}
		break;
	case VIEW_WINOP_RRESIZE:
		if (motion->xrel < 0 || motion->xrel > 0) {
			win->rd.w += motion->xrel;
		}
		if (motion->yrel < 0 || motion->yrel > 0) {
			win->rd.h += motion->yrel;
		}
		break;
	case VIEW_WINOP_HRESIZE:
		if (motion->yrel < 0 || motion->yrel > 0) {
			win->rd.h += motion->yrel;
		}
	default:
		break;
	}

	/* Clamp to minimum window geometry. */
	if (win->rd.w < win->minw &&
	   (prop_uint32(config, "widgets.flags") & CONFIG_WINDOW_ANYSIZE)
	    == 0) {
		win->rd.w = win->minw;
	} else {
		win->rd.x = nx;
	}
	if (win->rd.h < win->minh &&
	   (prop_uint32(config, "widgets.flags") & CONFIG_WINDOW_ANYSIZE)
	    == 0) {
		win->rd.h = win->minh;
	} else {
		win->rd.y = ny;
	}
	
	if (win->rd.x < 0)
		win->rd.x = 0;
	if (win->rd.y < 0)
		win->rd.y = 0;

	/* Clamp to view boundaries. */
	if (win->rd.x + win->rd.w > view->w) {
		win->rd.x = ro.x;
		win->rd.w = ro.w;
	}
	if (win->rd.y + win->rd.h > view->h) {
		win->rd.y = ro.y;
		win->rd.h = ro.h;
	}

	/* Effect the change. */
	window_resize(win);

	/* Update the background. */
	if (view->gfx_engine == GFX_ENGINE_GUI) {
		/* Rectangle at the left (lresize operation). */
		if (win->rd.x > ro.x) {
			SDL_UpdateRect(view->v,
			    ro.x, ro.y,
			    win->rd.x-ro.x, win->rd.h);
		}
		/* Rectangle at the right (rresize and hresize ops). */
		if (win->rd.w < ro.w) {
			SDL_UpdateRect(view->v,
			    win->rd.x + win->rd.w, win->rd.y,
			    ro.w - win->rd.w, ro.h);
		}
		/* Rectangle at the bottom (rresize and hresize ops). */
		if (win->rd.h < ro.h) {
			SDL_UpdateRect(view->v,
			    win->rd.x, win->rd.y + win->rd.h,
			    ro.w, ro.h - win->rd.h);
		}
	} else if (view->rootmap != NULL) {
		/* Redraw the map. */
		view->rootmap->map->redraw++;
	}
}

/* Window must be locked. */
void
window_resize(struct window *win)
{
	struct region *reg;

	/* Clamp to view area, leave a margin. */
	window_clamp(win);

	win->body.x = win->rd.x + win->borderw;
	win->body.y = win->rd.y + win->borderw*2 + win->titleh;
	win->body.w = win->rd.w - win->borderw*2;
	win->body.h = win->rd.h - win->borderw*2 - win->titleh;

	win->wid.w = win->rd.w;
	win->wid.h = win->rd.h;

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		struct widget *wid;
		int x = win->borderw + 4, y = win->titleh + win->borderw + 4; 
		int nwidgets = 0;

		/* XXX */
		TAILQ_FOREACH(wid, &reg->widgetsh, widgets)
			nwidgets++;

		/* Region coordinates */
		if (reg->rx > 0) {
			reg->x = (reg->rx * win->body.w / 100) +
			    (win->body.x - win->rd.x) + 1;
		} else if (reg->rx == 0) {
			reg->x = x;
		} else {
			reg->x = abs(reg->rx);
		}
		if (reg->ry > 0) {
			reg->y = (reg->ry * win->body.h / 100) +
			    (win->body.y - win->rd.y);
		} else if (reg->ry == 0) {
			reg->y = y;
		} else {
			reg->y = abs(reg->ry);
		}

		/* Region geometry */
		if (reg->rw > 0) {
			reg->w = (reg->rw * win->body.w / 100);
			if (reg->w > win->rd.w - 16) {
				reg->w = win->rd.w - 16;
			}
		} else if (reg->rw == 0) {
			reg->w = win->rd.w - x;
		} else {
			reg->w = abs(reg->rw);
		}
		
		if (reg->rh >= 0) {
			reg->h = (reg->rh * win->body.h / 100) - 4;
			if (reg->h > win->rd.h - 32) {
				reg->h = win->rd.h - 32;
			}
		} else if (reg->rh == 0) {
			reg->h = win->rd.h - y;
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

#ifdef DEBUG
	if (prop_uint32(config, "widgets.flags") & CONFIG_WINDOW_ANYSIZE) {
		dprintf("%s: %d, %d\n", OBJECT(win)->name, win->rd.w, win->rd.h);
	}
#endif
}

void
window_set_caption(struct window *win, const char *fmt, ...)
{
	va_list args;

	pthread_mutex_lock(&win->lock);

	/* XXX */
	if (win->caption != NULL) {
		free(win->caption);
	}
	va_start(args, fmt);
	if (vasprintf(&win->caption, fmt, args) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
	va_end(args);
	
	pthread_mutex_unlock(&win->lock);
}

int
window_load(void *p, int fd)
{
	struct window *win = p;

	if (version_read(fd, &window_ver) != 0) {
		return (-1);
	}

	win->rd.x = read_uint32(fd);
	win->rd.y = read_uint32(fd);
	win->rd.w = read_uint32(fd);
	win->rd.h = read_uint32(fd);

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
	write_uint32(fd, win->rd.x);
	write_uint32(fd, win->rd.y);
	write_uint32(fd, win->rd.w);
	write_uint32(fd, win->rd.h);
	
	return (0);
}

void
window_generic_detached(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	OBJECT_ASSERT(win, "window");

	view_detach(win);
}

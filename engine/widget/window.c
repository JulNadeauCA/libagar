/*	$Csoft: window.c,v 1.224 2004/05/06 06:24:27 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/config.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/widget.h>
#include <engine/widget/primitive.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

const struct version window_ver = {
	"agar window",
	3, 0
};

const struct widget_ops window_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		window_destroy,
		window_load,
		window_save,
		NULL			/* edit */
	},
	window_draw,
	window_scale
};

enum {
	BGFILL_COLOR,
	BG_COLOR,
	CAPTION_COLOR,
	HIGHLIGHT_COLOR
};

/* XXX */
#include "borders/blue1.h"

static void	window_shown(int, union evarg *);
static void	window_hidden(int, union evarg *);
static void	window_clamp(struct window *);
static void	window_focus(struct window *);
static void	window_apply_alignment(struct window *);
static void	winop_move(struct window *, SDL_MouseMotionEvent *);
static void	winop_resize(int, struct window *, SDL_MouseMotionEvent *);

#ifdef DEBUG
#define DEBUG_STATE		0x01
#define DEBUG_RESIZE		0x02
#define DEBUG_RESIZE_GEO	0x04

int	window_debug = 0;
#define	engine_debug window_debug
#endif

pthread_mutex_t	window_lock = PTHREAD_MUTEX_INITIALIZER;
int		window_xoffs = 0;
int		window_yoffs = 0;
int		window_freescale = 0;

struct window *
window_new(const char *fmt, ...)
{
	struct window *win = NULL;

	pthread_mutex_lock(&view->lock);
	if (fmt != NULL) {				/* Unique instance */
		struct event *ev;
		char name[OBJECT_NAME_MAX];
		struct window *owin;
		va_list ap;
		char *c;

		va_start(ap, fmt);
		vsnprintf(name, sizeof(name), fmt, ap);
		va_end(ap);

		for (c = &name[0]; *c != '\0'; c++) {
			if (*c == '/')
				*c = '_';
		}

		TAILQ_FOREACH(owin, &view->windows, windows) {
			if (strlen(OBJECT(owin)->name) < 4 ||
			    strcmp(OBJECT(owin)->name+4, name) != 0) {
				continue;
			}
			window_show(owin);
			window_focus(owin);
			goto out;
		}

		win = Malloc(sizeof(struct window), M_OBJECT);
		window_init(win, name);
		event_new(win, "window-close", window_generic_hide, "%p", win);
	} else {						/* Generic */
		win = Malloc(sizeof(struct window), M_OBJECT);
		window_init(win, NULL);
		event_new(win, "window-close", window_generic_detach, "%p",
		    win);
	}
	view_attach(win);
out:
	pthread_mutex_unlock(&view->lock);
	return (win);
}

void
window_init(void *p, const char *name)
{
	char wname[OBJECT_NAME_MAX];
	struct window *win = p;
	struct event *ev;
	int i;

	strlcpy(wname, "win-", sizeof(wname));
	strlcat(wname, (name != NULL) ? name : "generic", sizeof(wname));

	widget_init(win, "window", &window_ops, 0);
	object_set_type(win, "window");
	object_set_name(win, wname);

	widget_map_color(win, BGFILL_COLOR, "background-filling", 0, 0, 0, 255);
	widget_map_color(win, BG_COLOR, "background", 0, 0, 0, 255);
	widget_map_color(win, CAPTION_COLOR, "caption", 245, 245, 245, 255);
#if 0
	/* XXX issues */
	win->flags = (name != NULL) ? WINDOW_PERSISTENT : 0;
#else
	win->flags = 0;
#endif
	win->visible = 0;

	win->borderw = default_nborder;
	win->border = Malloc(win->borderw * sizeof(Uint32), M_WIDGET);
	for (i = 0; i < win->borderw; i++) {
		win->border[i] = SDL_MapRGB(vfmt,
		    default_border[i].r,
		    default_border[i].g,
		    default_border[i].b);

		if (i == win->borderw/2) {
			widget_map_color(win, HIGHLIGHT_COLOR, "highlight",
			    default_border[i].r,
			    default_border[i].g,
			    default_border[i].b,
			    255);
		}
	}

	pthread_mutex_init(&win->lock, &recursive_mutexattr);
	win->alignment = WINDOW_CENTER;
	win->spacing = 2;
	win->padding = win->borderw + 2;
	win->minw = 0;
	win->minh = 0;
	win->tbar = titlebar_new(win, 0);
	TAILQ_INIT(&win->subwins);

	/* Automatically notify children of visibility changes. */
	ev = event_new(win, "widget-shown", window_shown, NULL);
	ev->flags |= EVENT_PROPAGATE;
	ev = event_new(win, "widget-hidden", window_hidden, NULL);
	ev->flags |= EVENT_PROPAGATE;
	ev = event_new(win, "widget-lostfocus", NULL, NULL);
	ev->flags |= EVENT_PROPAGATE;

	/* Notify children prior to destruction. */
	ev = event_new(win, "detached", NULL, NULL);
	ev->flags |= EVENT_PROPAGATE;
}

/* Attach a sub-window. */
void
window_attach(struct window *win, struct window *subwin)
{
	pthread_mutex_lock(&view->lock);
	TAILQ_INSERT_HEAD(&win->subwins, subwin, swins);
	pthread_mutex_unlock(&view->lock);
}

/* Detach a sub-window. */
void
window_detach(struct window *win, struct window *subwin)
{
	pthread_mutex_lock(&view->lock);
	TAILQ_REMOVE(&win->subwins, subwin, swins);
	view_detach(subwin);
	pthread_mutex_unlock(&view->lock);
}

void
window_draw(void *p)
{
	struct window *win = p;
	int i, ncol;

	primitives.rect_filled(win,
	    0,
	    0,
	    WIDGET(win)->w,
	    WIDGET(win)->h,
	    BG_COLOR);

	/* Draw the window frame (expected to fit inside padding). */
	for (i = 1; i < win->borderw; i++) {
		ncol = widget_push_color(WIDGET(win), win->border[i]);

		primitives.line(win,
		    i,
		    i,
		    WIDGET(win)->w - i,
		    i,
		    ncol);
		primitives.line(win,
		    i, 
		    WIDGET(win)->h - i,
		    WIDGET(win)->w - i,
		    WIDGET(win)->h - i,
		    ncol);
		primitives.line(win,
		    i,
		    i,
		    i,
		    WIDGET(win)->h - i,
		    ncol);
		primitives.line(win,
		    WIDGET(win)->w - i,
		    i,
		    WIDGET(win)->w - i,
		    WIDGET(win)->h - i,
		    ncol);
	
		widget_pop_color(WIDGET(win));
	}
	
	/* Draw the resize decorations. */
	ncol = widget_push_color(WIDGET(win), win->border[0]);
	primitives.line(win,				/* Lower left */
	    18,
	    WIDGET(win)->h - win->borderw,
	    18,
	    WIDGET(win)->h - 2,
	    ncol);
	primitives.line(win,
	    19,
	    WIDGET(win)->h - win->borderw,
	    19,
	    WIDGET(win)->h - 2,
	    HIGHLIGHT_COLOR);
	primitives.line(win,
	    2,
	    WIDGET(win)->h - 20,
	    win->borderw,
	    WIDGET(win)->h - 20,
	    ncol);
	primitives.line(win,
	    2,
	    WIDGET(win)->h - 19,
	    win->borderw,
	    WIDGET(win)->h - 19,
	    HIGHLIGHT_COLOR);
	
	primitives.line(win,			       /* Lower right */
	    WIDGET(win)->w - 19,
	    WIDGET(win)->h - win->borderw,
	    WIDGET(win)->w - 19,
	    WIDGET(win)->h - 2,
	    ncol);
	primitives.line(win,
	    WIDGET(win)->w - 18,
	    WIDGET(win)->h - win->borderw,
	    WIDGET(win)->w - 18,
	    WIDGET(win)->h - 2,
	    HIGHLIGHT_COLOR);
	primitives.line(win,
	    WIDGET(win)->w - win->borderw,
	    WIDGET(win)->h - 20,
	    WIDGET(win)->w - 2,
	    WIDGET(win)->h - 20,
	    ncol);
	primitives.line(win,
	    WIDGET(win)->w - win->borderw,
	    WIDGET(win)->h - 19,
	    WIDGET(win)->w - 2,
	    WIDGET(win)->h - 19,
	    HIGHLIGHT_COLOR);

	widget_pop_color(WIDGET(win));
}

void
window_destroy(void *p)
{
	struct window *win = p;

	Free(win->border, M_WIDGET);
	pthread_mutex_destroy(&win->lock);
	/* view_detach_queued() frees the subwindow list. */

	widget_destroy(win);
}

static void
window_shown(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	int init = (WIDGET(win)->x == -1 && WIDGET(win)->y == -1);

	view->focus_win = win;
	window_focus(win);

	if (init) {
		/* First pass: initial sizing. */
		WIDGET_OPS(win)->scale(win, WIDGET(win)->w, WIDGET(win)->h);

		/* Second pass: [wh]fill and homogenous box divisions. */
		WIDGET_OPS(win)->scale(win, WIDGET(win)->w, WIDGET(win)->h);
	
		/* Position the window and cache the absolute widget coords. */
		window_apply_alignment(win);
		widget_update_coords(win, WIDGET(win)->x, WIDGET(win)->y);
	}

	if (win->flags & WINDOW_PERSISTENT)
		object_load(win);
	
	event_post(NULL, win, "window-shown", NULL);
}

static void
window_hidden(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	
	/* Remove the focus. XXX cycle */
	view->focus_win = NULL;

	/* Update the background. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_GUI:
		primitives.rect_filled(win, 0, 0,
		    WIDGET(win)->w,
		    WIDGET(win)->h,
		    BGFILL_COLOR);
		
		if (!view->opengl) {
			SDL_UpdateRect(view->v,
			    WIDGET(win)->x, WIDGET(win)->y,
			    WIDGET(win)->w, WIDGET(win)->h);
		}
		break;
	case GFX_ENGINE_TILEBASED:
		rootmap_redraw();
		break;
	}

	if (win->flags & WINDOW_PERSISTENT)
		object_save(win);

	event_post(NULL, win, "window-hidden", NULL);
}

/* Toggle the visibility of a window. */
int
window_toggle_visibility(struct window *win)
{
	int oldvis;

	pthread_mutex_lock(&win->lock);
	oldvis = win->visible;
	if (win->visible) {
		window_hide(win);
	} else {
		window_show(win);
	}
	pthread_mutex_unlock(&win->lock);
	return (oldvis);
}

/* Set the visibility bit of a window. */
void
window_show(struct window *win)
{
	pthread_mutex_lock(&win->lock);
	if (!win->visible) {
		win->visible++;
		event_post(NULL, win, "widget-shown", NULL);
	}
	pthread_mutex_unlock(&win->lock);
}

/* Clear the visibility bit of a window. */
void
window_hide(struct window *win)
{
	pthread_mutex_lock(&win->lock);
	if (win->visible) {
		win->visible = 0;
		event_post(NULL, win, "widget-hidden", NULL);
	}
	pthread_mutex_unlock(&win->lock);
}

static void
count_widgets(struct widget *wid, unsigned int *nwidgets)
{
	struct widget *cwid;

	if (wid->flags & WIDGET_FOCUSABLE)
		(*nwidgets)++;

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		count_widgets(cwid, nwidgets);
}

static void
map_widgets(struct widget *wid, struct widget **widgets, unsigned int *i)
{
	struct widget *cwid;

	if (wid->flags & WIDGET_FOCUSABLE)
		widgets[(*i)++] = wid;

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		map_widgets(cwid, widgets, i);
}

/*
 * Move the widget focus inside a window.
 * The window must be locked.
 */
static void
cycle_focus(struct window *win, int reverse)
{
	struct widget **widgets;
	struct widget *olfocus;
	unsigned int nwidgets = 0;
	int i = 0;

	if ((olfocus = widget_find_focus(win)) == NULL) {
		dprintf("no focus!\n");
		return;
	}

	count_widgets(WIDGET(win), &nwidgets);
	widgets = Malloc(nwidgets * sizeof(struct widget *), M_WIDGET);
	map_widgets(WIDGET(win), widgets, &i);

	for (i = 0; i < nwidgets; i++) {
		if (widgets[i] == olfocus) {
			if (reverse) {
				if (i-1 >= 0) {
					widget_focus(widgets[i-1]);
				} else if (i-1 < 0) {
					widget_focus(widgets[nwidgets-1]);
				}
			} else {
				if (i+1 < nwidgets) {
					widget_focus(widgets[i+1]);
				} else if (i+1 == nwidgets) {
					widget_focus(widgets[0]);
				}
			}
			break;
		}
	}
	Free(widgets, M_WIDGET);
}

/*
 * Move a window using the mouse.
 * The view and window must be locked.
 */
static void
winop_move(struct window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect oldpos, newpos, rfill1, rfill2;
	Uint32 fillcolor = WIDGET_COLOR(win, BGFILL_COLOR);

	if (view->gfx_engine == GFX_ENGINE_GUI) {
		oldpos.x = WIDGET(win)->x;
		oldpos.y = WIDGET(win)->y;
		oldpos.w = WIDGET(win)->w;
		oldpos.h = WIDGET(win)->h;
	}

	WIDGET(win)->x += motion->xrel;
	WIDGET(win)->y += motion->yrel;
	window_clamp(win);

	widget_update_coords(win, WIDGET(win)->x, WIDGET(win)->y);

	/* Update the background. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		rootmap_redraw();
		break;
	case GFX_ENGINE_GUI:
		newpos.x = WIDGET(win)->x;
		newpos.y = WIDGET(win)->y;
		newpos.w = WIDGET(win)->w;
		newpos.h = WIDGET(win)->h;
		rfill1.w = 0;
		rfill2.w = 0;
		if (newpos.x > oldpos.x) {		/* Right */
			rfill1.x = oldpos.x;
			rfill1.y = oldpos.y;
			rfill1.w = newpos.x - oldpos.x;
			rfill1.h = newpos.h;
		} else if (newpos.x < oldpos.x) {	/* Left */
			rfill1.x = newpos.x + newpos.w;
			rfill1.y = newpos.y;
			rfill1.w = oldpos.x - newpos.x;
			rfill1.h = oldpos.h;
		}
		if (newpos.y > oldpos.y) {		/* Downward */
			rfill2.x = oldpos.x;
			rfill2.y = oldpos.y;
			rfill2.w = newpos.w;
			rfill2.h = newpos.y - oldpos.y;
		} else if (newpos.y < oldpos.y) {	/* Upward */
			rfill2.x = oldpos.x;
			rfill2.y = newpos.y + newpos.h;
			rfill2.w = oldpos.w;
			rfill2.h = oldpos.y - newpos.y;
		}
#ifdef HAVE_OPENGL
		if (view->opengl) {
			Uint8 r, g, b;

			SDL_GetRGB(fillcolor, vfmt, &r, &g, &b);
			glColor3ub(r, g, b);

			if (rfill1.w > 0) {
				glRecti(
				    rfill1.x,
				    rfill1.y,
				    rfill1.x + rfill1.w,
				    rfill1.y + rfill1.h);
			}
			if (rfill2.w > 0) {
				glRecti(
				    rfill2.x,
				    rfill2.y,
				    rfill2.x + rfill2.w,
				    rfill2.y + rfill2.h);
			}
		} else
#endif /* HAVE_OPENGL */
		{
			if (rfill1.w > 0) {
				SDL_FillRect(view->v, &rfill1, fillcolor);
				SDL_UpdateRects(view->v, 1, &rfill1);
			}
			if (rfill2.w > 0) {
				SDL_FillRect(view->v, &rfill2, fillcolor);
				SDL_UpdateRects(view->v, 1, &rfill2);
			}
		}
		break;
	}
}

/*
 * Give focus to a window.
 * The view and window must be locked.
 */
static void
window_focus(struct window *win)
{
	struct window *lastwin;

	view->focus_win != NULL;

	lastwin = TAILQ_LAST(&view->windows, windowq);
	if (win != NULL && lastwin == win) 		/* Already focused? */
		return;

	if (lastwin != NULL)
		event_post(NULL, lastwin, "widget-lostfocus", NULL);

	if (win != NULL) {
		/*
		 * Move the new window to tail, so the rendering functions
		 * don't have to traverse the list backwards.
		 */
		TAILQ_REMOVE(&view->windows, win, windows);
		TAILQ_INSERT_TAIL(&view->windows, win, windows);
	}
}

/*
 * Dispatch events to widgets.
 * The view must be locked, and the window list must be nonempty.
 */
int
window_event(SDL_Event *ev)
{
	static struct window *keydown_win = NULL;	/* XXX hack */
	struct window *win;
	struct widget *wid;
	int focus_changed = 0;

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		/* Focus on the highest overlapping window. */
		view->focus_win = NULL;
		TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
			pthread_mutex_lock(&win->lock);
			if (!win->visible ||
			    !widget_area(win, ev->button.x, ev->button.y)) {
				pthread_mutex_unlock(&win->lock);
				continue;
			}
			view->focus_win = win;
			pthread_mutex_unlock(&win->lock);
			break;
		}
		focus_changed++;
		break;
	case SDL_MOUSEBUTTONUP:
		view->winop = VIEW_WINOP_NONE;
		break;
	}

	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if (!win->visible) {
			pthread_mutex_unlock(&win->lock);
			continue;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			if (view->winop != VIEW_WINOP_NONE &&
			    view->wop_win != win) {
				pthread_mutex_unlock(&win->lock);
				continue;
			}
			switch (view->winop) {
			case VIEW_WINOP_NONE:
				break;
			case VIEW_WINOP_MOVE:
				winop_move(win, &ev->motion);
				goto out;
			case VIEW_WINOP_LRESIZE:
			case VIEW_WINOP_RRESIZE:
			case VIEW_WINOP_HRESIZE:
				winop_resize(view->winop, win, &ev->motion);
				goto out;
			}
			/*
			 * Forward to all widgets that either hold focus or have
			 * the WIDGET_UNFOCUSED_MOTION flag set.
			 */
			OBJECT_FOREACH_CHILD(wid, win, widget) {
				widget_mousemotion(win, wid,
				    ev->motion.x, ev->motion.y,
				    ev->motion.xrel, ev->motion.yrel,
				    ev->motion.state);
			}
			break;
		case SDL_MOUSEBUTTONUP:
			view->winop = VIEW_WINOP_NONE;
			view->wop_win = NULL;
			/*
			 * Forward to all widgets that either hold focus or have
			 * the WIDGET_UNFOCUSED_BUTTONUP flag set.
			 */
			OBJECT_FOREACH_CHILD(wid, win, widget) {
				widget_mousebuttonup(win, wid,
				    ev->button.button,
				    ev->button.x, ev->button.y);
			}
			if (focus_changed) {
				goto out;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!widget_area(win, ev->button.x, ev->button.y)) {
				pthread_mutex_unlock(&win->lock);
				continue;
			}
			if ((ev->button.y - WIDGET(win)->y) >
			    (WIDGET(win)->h - win->borderw)) {
				/* Start a resize operation. */
			    	if ((ev->button.x - WIDGET(win)->x) < 17) {
					view->winop = VIEW_WINOP_LRESIZE;
				} else if ((ev->button.x - WIDGET(win)->x) >
				           (WIDGET(win)->w - 17)) {
					view->winop = VIEW_WINOP_RRESIZE;
				} else {
					view->winop = VIEW_WINOP_HRESIZE;
				}
				view->wop_win = win;
			}
			/* Forward to overlapping widgets. */
			OBJECT_FOREACH_CHILD(wid, win, widget) {
				if (widget_mousebuttondown(win, wid,
				    ev->button.button, ev->button.x,
				    ev->button.y)) {
					goto out;
				}
			}
			if (focus_changed) {
				goto out;
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
			if (ev->key.keysym.sym == SDLK_TAB &&	/* Move focus */
			    ev->type == SDL_KEYUP) {
				cycle_focus(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				goto out;
			}
			if (WINDOW_FOCUSED(win)) {
				struct widget *fwid;

				if ((fwid = widget_find_focus(win)) != NULL) {
					event_post(NULL, fwid,
					    (ev->type == SDL_KEYUP) ?
					    "window-keyup" : "window-keydown",
					    "%i, %i, %i",
					    (int)ev->key.keysym.sym,
					    (int)ev->key.keysym.mod,
					    (int)ev->key.keysym.unicode);
					/*
					 * Ensure the keyup event is posted to
					 * this window when the key is released,
					 * in case a keydown event handler
					 * changes the window focus.
					 */
					keydown_win = win;
				} else {
					dprintf("no focused widget in %s\n",
					    OBJECT(win)->name);
				}
			}
		}
		pthread_mutex_unlock(&win->lock);
	}
	return (0);
out:
	pthread_mutex_unlock(&win->lock);

	/*
	 * The focus_changed flag is set if there was a focus change
	 * in reaction to a window operation. The focus_win variable
	 * may also be changed by window show/hide functions.
	 */
	if (focus_changed || view->focus_win != NULL) {
		/* Reorder the window list. */
		window_focus(view->focus_win);
	}
	return (1);
}

/*
 * Clamp the window geometry/coordinates down to the view area.
 * The window must be locked.
 */
static void
window_clamp(struct window *win)
{
	if (WIDGET(win)->x < 0)
		WIDGET(win)->x = 0;
	if (WIDGET(win)->y < 0)
		WIDGET(win)->y = 0;

	if (WIDGET(win)->w > view->w)
		WIDGET(win)->w = view->w;
	if (WIDGET(win)->h > view->h)
		WIDGET(win)->h = view->h;

	if ((WIDGET(win)->x + WIDGET(win)->w) > view->w)
		WIDGET(win)->x = view->w - WIDGET(win)->w;
	if ((WIDGET(win)->y + WIDGET(win)->h) > view->h)
		WIDGET(win)->y = view->h - WIDGET(win)->h;
}

/*
 * Resize a window with the mouse.
 * The window must be locked.
 */
static void
winop_resize(int op, struct window *win, SDL_MouseMotionEvent *motion)
{
	Uint32 fillcolor = WIDGET_COLOR(win, BGFILL_COLOR);
	SDL_Rect rfill1, rfill2;
	int ox = WIDGET(win)->x;
	int oy = WIDGET(win)->y;
	int ow = WIDGET(win)->w;
	int oh = WIDGET(win)->h;
	int nx = WIDGET(win)->x;

	/* XXX contorted */
	switch (op) {
	case VIEW_WINOP_LRESIZE:
		if (motion->xrel < 0) {
			WIDGET(win)->w -= motion->xrel;
			nx += motion->xrel;
		} else if (motion->xrel > 0) {
			WIDGET(win)->w -= motion->xrel;
			nx += motion->xrel;
		}
		if (motion->yrel < 0 || motion->yrel > 0)
			WIDGET(win)->h += motion->yrel;
		break;
	case VIEW_WINOP_RRESIZE:
		if (motion->xrel < 0 || motion->xrel > 0)
			WIDGET(win)->w += motion->xrel;
		if (motion->yrel < 0 || motion->yrel > 0)
			WIDGET(win)->h += motion->yrel;
		break;
	case VIEW_WINOP_HRESIZE:
		if (motion->yrel < 0 || motion->yrel > 0)
			WIDGET(win)->h += motion->yrel;
		break;
	default:
		break;
	}

	if (!window_freescale && WIDGET(win)->w < win->minw) {
		WIDGET(win)->w = win->minw;
	} else {
		WIDGET(win)->x = nx;
	}
	if (!window_freescale && WIDGET(win)->h < win->minh)
		WIDGET(win)->h = win->minh;

	if (WIDGET(win)->x < 0)
		WIDGET(win)->x = 0;
	if (WIDGET(win)->y < 0)
		WIDGET(win)->y = 0;

	if ((WIDGET(win)->x + WIDGET(win)->w) > view->w) {
		WIDGET(win)->x = ox;
		WIDGET(win)->w = ow;
	}
	if ((WIDGET(win)->y + WIDGET(win)->h) > view->h) {
		WIDGET(win)->y = oy;
		WIDGET(win)->h = oh;
	}

	/* Effect the possible changes in geometry. */
	WIDGET_OPS(win)->scale(win, WIDGET(win)->w, WIDGET(win)->h);
	widget_update_coords(win, WIDGET(win)->x, WIDGET(win)->y);

	/* Update the background. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_GUI:
		rfill1.w = 0;
		rfill2.w = 0;

		if (WIDGET(win)->x > ox) {			/* L-resize */
			rfill1.x = ox;
			rfill1.y = oy;
			rfill1.w = WIDGET(win)->x - ox;
			rfill1.h = WIDGET(win)->h;
		} else if (WIDGET(win)->w < ow) {		/* R-resize */
			rfill1.x = WIDGET(win)->x + WIDGET(win)->w;
			rfill1.y = WIDGET(win)->y;
			rfill1.w = ow - WIDGET(win)->w;
			rfill1.h = oh;
		}
		if (WIDGET(win)->h < oh) {			/* H-resize */
			rfill2.x = ox;
			rfill2.y = WIDGET(win)->y + WIDGET(win)->h;
			rfill2.w = ow;
			rfill2.h = oh - WIDGET(win)->h;
		}
#ifdef HAVE_OPENGL
		if (view->opengl) {
			Uint8 r, g, b;

			SDL_GetRGB(fillcolor, vfmt, &r, &g, &b);
			glColor3ub(r, g, b);

			if (rfill1.w > 0) {
				glRecti(
				    rfill1.x,
				    rfill1.y,
				    rfill1.x + rfill1.w,
				    rfill1.y + rfill1.h);
			}
			if (rfill2.w > 0) {
				glRecti(
				    rfill2.x,
				    rfill2.y,
				    rfill2.x + rfill2.w,
				    rfill2.y + rfill2.h);
			}
		} else
#endif /* HAVE_OPENGL */
		{
			if (rfill1.w > 0) {
				SDL_FillRect(view->v, &rfill1, fillcolor);
				SDL_UpdateRects(view->v, 1, &rfill1);
			}
			if (rfill2.w > 0) {
				SDL_FillRect(view->v, &rfill2, fillcolor);
				SDL_UpdateRects(view->v, 1, &rfill2);
			}
		}
		break;
	case GFX_ENGINE_TILEBASED:
		rootmap_redraw();
		break;
	}
}

int
window_load(void *p, struct netbuf *buf)
{
	struct window *win = p;
	int view_w, view_h;
	struct version ver;

	if (version_read(buf, &window_ver, &ver) != 0)
		return (-1);

	win->flags = (int)read_uint32(buf);
	view_w = (int)read_uint16(buf);
	view_h = (int)read_uint16(buf);
	WIDGET(win)->x = (int)read_sint16(buf) * view->v->w / view_w;
	WIDGET(win)->y = (int)read_sint16(buf) * view->v->h / view_h;
	WIDGET(win)->w = (int)read_uint16(buf) * view->v->w / view_w;
	WIDGET(win)->h = (int)read_uint16(buf) * view->v->h / view_h;

	/* Adjust to the minimum cosmetic size. */
	if (WIDGET(win)->w < win->minw)
		WIDGET(win)->w = win->minw;
	if (WIDGET(win)->h < win->minh)
		WIDGET(win)->w = win->minh;

	/* Effect the possible changes in geometry. */
	WIDGET_OPS(win)->scale(win, WIDGET(win)->w, WIDGET(win)->h);
	widget_update_coords(win, WIDGET(win)->x, WIDGET(win)->y);
	return (0);
}

int
window_save(void *p, struct netbuf *buf)
{
	struct window *win = p;

	version_write(buf, &window_ver);
	write_uint32(buf, (Uint32)win->flags);
	write_uint16(buf, view->v->w);
	write_uint16(buf, view->v->h);
	write_sint16(buf, (Sint16)WIDGET(win)->x);
	write_sint16(buf, (Sint16)WIDGET(win)->y);
	write_uint16(buf, (Uint16)WIDGET(win)->w);
	write_uint16(buf, (Uint16)WIDGET(win)->h);
	return (0);
}

void
window_generic_detach(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	view_detach(win);
}

void
window_generic_show(int argc, union evarg *argv)
{
	window_show(argv[1].p);
}

void
window_generic_hide(int argc, union evarg *argv)
{
	window_hide(argv[1].p);
}

void
window_scale(void *p, int w, int h)
{
	struct window *win = p;
	struct widget *wid;
	int nfixed = 0, nhfill = 0, totfixed = 0;
	int x = win->padding;
	int y = win->padding;

	pthread_mutex_lock(&win->lock);

	if (w == -1 && h == -1) {
		int maxw = 0;

		WIDGET(win)->w = win->padding*2;
		WIDGET(win)->h = win->padding*2;

		OBJECT_FOREACH_CHILD(wid, win, widget) {
			wid->x = x;
			wid->y = y;

			WIDGET_OPS(wid)->scale(wid, -1, -1);	/* Hint */

			if (maxw < wid->w) {
				maxw = wid->w;
			}
			if ((maxw + win->padding*2) > WIDGET(win)->w) {
				WIDGET(win)->w = maxw + win->padding*2;
			}
			WIDGET(win)->h += wid->h + win->spacing;
			y += wid->h + win->spacing;
		}

		/* Prevent further scaling of widgets to unaesthetic sizes. */
		win->minw = WIDGET(win)->w;
		win->minh = WIDGET(win)->h;
		goto out;
	}

	/*
	 * Calculate the total height requested by fixed-size widgets
	 * and allow one widget to use the remaining space.
	 */
	OBJECT_FOREACH_CHILD(wid, win, widget) {
		WIDGET_OPS(wid)->scale(wid, -1, -1);		/* Hint */

		if (wid->flags & WIDGET_HFILL) {
			nhfill++;
		} else {
			totfixed += wid->h;
			nfixed++;
		}
	}

#ifdef DEBUG
	/* XXX divide instead */
	if (nhfill > 1)
		fatal(">1 WIDGET_HFILL widget is nonsense in a window");
#endif

	/* Mix fixed-size and possibly hfill widgets. */
	OBJECT_FOREACH_CHILD(wid, win, widget) {
		wid->x = x;
		wid->y = y;

		if (wid->flags & WIDGET_WFILL) {
			wid->w = w - win->padding*2; 
		}
		if (wid->flags & WIDGET_HFILL) {
			wid->h = h - totfixed -
			    (nfixed*win->spacing + win->padding*2) -
			    win->spacing*2;
		}
		WIDGET_OPS(wid)->scale(wid, wid->w, wid->h);

		y += wid->h + win->spacing;
	}

	/* Don't effect padding on the titlebar. */
	WIDGET(win->tbar)->x = 2;
	WIDGET(win->tbar)->y = 1;
	WIDGET(win->tbar)->w = WIDGET(win)->w - 2;
out:
	pthread_mutex_unlock(&win->lock);
}

/* Change the spacing between child widgets. */
void
window_set_spacing(struct window *win, int spacing)
{
	pthread_mutex_lock(&win->lock);
	win->spacing = spacing;
	pthread_mutex_unlock(&win->lock);
}

/* Change the padding around child widgets. */
void
window_set_padding(struct window *win, int padding)
{
	pthread_mutex_lock(&win->lock);
	win->padding = padding;
	pthread_mutex_unlock(&win->lock);
}

/* Request a specific initial window position. */
void
window_set_position(struct window *win, enum window_alignment alignment,
    int cascade)
{
	pthread_mutex_lock(&win->lock);
	win->alignment = alignment;
	if (cascade) {
		win->flags |= WINDOW_CASCADE;
	} else {
		win->flags &= ~WINDOW_CASCADE;
	}
	pthread_mutex_unlock(&win->lock);
}

/* Set a default window-close handler. */
void
window_set_closure(struct window *win, enum window_close_mode mode)
{
	switch (mode) {
	case WINDOW_HIDE:
		event_new(win, "window-close", window_generic_hide, "%p", win);
		break;
	case WINDOW_DETACH:
		event_new(win, "window-close", window_generic_detach, "%p",
		    win);
		break;
	case WINDOW_IGNORE:
		event_remove(win, "window-close");
		break;
	}
}

/*
 * Set the position of a window assuming its size is known.
 * The window must be locked.
 */
static void
window_apply_alignment(struct window *win)
{
	switch (win->alignment) {
	case WINDOW_UPPER_LEFT:
		WIDGET(win)->x = 0;
		WIDGET(win)->y = 0;
		break;
	case WINDOW_MIDDLE_LEFT:
		WIDGET(win)->x = 0;
		WIDGET(win)->y = view->h/2 - WIDGET(win)->h/2;
		break;
	case WINDOW_LOWER_LEFT:
		WIDGET(win)->x = 0;
		WIDGET(win)->y = view->h - WIDGET(win)->h;
		break;
	case WINDOW_UPPER_RIGHT:
		WIDGET(win)->x = view->w - WIDGET(win)->w;
		WIDGET(win)->y = 0;
		break;
	case WINDOW_MIDDLE_RIGHT:
		WIDGET(win)->x = view->w - WIDGET(win)->w;
		WIDGET(win)->y = view->h/2 - WIDGET(win)->h/2;
		break;
	case WINDOW_LOWER_RIGHT:
		WIDGET(win)->x = view->w - WIDGET(win)->w;
		WIDGET(win)->y = view->h - WIDGET(win)->h;
		break;
	case WINDOW_CENTER:
		WIDGET(win)->x = view->w/2 - WIDGET(win)->w/2;
		WIDGET(win)->y = view->h/2 - WIDGET(win)->h/2;
		break;
	case WINDOW_LOWER_CENTER:
		WIDGET(win)->x = view->w/2 - WIDGET(win)->w/2;
		WIDGET(win)->y = view->h - WIDGET(win)->h;
		break;
	case WINDOW_UPPER_CENTER:
		WIDGET(win)->x = view->w/2 - WIDGET(win)->w/2;
		WIDGET(win)->y = 0;
		break;
	}
	if (win->flags & WINDOW_CASCADE) {
		pthread_mutex_lock(&window_lock);
		if ((window_xoffs += text_font_height(NULL))+WIDGET(win)->w >
		    view->w/2) {
			window_xoffs = 0;
		}
		if ((window_yoffs += win->borderw)+WIDGET(win)->h > view->h/2) {
			window_yoffs = 0;
		}
		WIDGET(win)->x += window_xoffs;
		WIDGET(win)->y += window_yoffs;
		pthread_mutex_unlock(&window_lock);
	}
	window_clamp(win);
}

/* Set the text to show inside a window's titlebar. */
void
window_set_caption(struct window *win, const char *fmt, ...)
{
	char s[LABEL_MAX];
	va_list ap;
	SDL_Surface *su;

	if (win->tbar == NULL)
		return;

	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);

#ifdef DEBUG
	strlcpy(win->caption, s, sizeof(win->caption));
#endif
	su = text_render(NULL, -1, WIDGET_COLOR(win, CAPTION_COLOR), s);
	label_set_surface(win->tbar->label, su);
}

/*	$Csoft: window.c,v 1.233 2004/09/29 05:48:34 vedge Exp $	*/

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
	HIGHLIGHT_COLOR,
	BORDER_BASE_COLOR
};

static const SDL_Color default_border[] = {
	{ 70, 70, 65, 255 },
	{ 70, 70, 65, 255 },
	{ 75, 75, 70, 255 },
	{ 90, 90, 85, 255 },
	{ 75, 75, 70, 255 },
	{ 70, 70, 65, 255 }
};

static const struct style_colormod default_colormods[] = {
	{ "button",	"frame",	{ 90, 90, 90, 255 } },
	{ "button",	"disabled",	{ 106, 106, 106, 255 } },
	{ "button",	"text",		{ 0, 0, 0, 255 } },
	{ NULL,		NULL,		{ 0, 0, 0, 0 } }
};

const struct style default_style = {
	"default",
	{
		default_border, 6,
		default_border, 6,
		{ 90, 90, 85, 255 },			/* highlight */
		{ 236, 236, 236, 255 },			/* bgcolor */
		NULL,					/* bggradient */
		NULL,					/* bgtexture */
		NOTCH_RESIZE_STYLE
	},
	{
		default_colormods,
		NULL					/* texmods */
	},
	NULL
};

static void	window_shown(int, union evarg *);
static void	window_hidden(int, union evarg *);
static void	clamp_to_view(struct window *);
static void	apply_alignment(struct window *);
static void	move_window(struct window *, SDL_MouseMotionEvent *);
static void	resize_window(int, struct window *, SDL_MouseMotionEvent *);

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

static __inline__ int
focus_existing(const char *name)
{
	struct window *owin;

	TAILQ_FOREACH(owin, &view->windows, windows) {
		if (strlen(OBJECT(owin)->name) >= 4 &&
		    strcmp(OBJECT(owin)->name+4, name) == 0) {
			window_show(owin);
			window_focus(owin);
		    	return (1);
		}
	}
	return (0);
}

struct window *
window_new(int flags, const char *fmt, ...)
{
	char name[OBJECT_NAME_MAX], *c;
	struct window *win;
	va_list ap;

	pthread_mutex_lock(&view->lock);
	if (fmt != NULL) {
		va_start(ap, fmt);
		vsnprintf(name, sizeof(name), fmt, ap);
		va_end(ap);

		for (c = &name[0]; *c != '\0'; c++) {
			if (*c == '/')
				*c = '_';
		}
		if (focus_existing(name)) {
			win = NULL;
			goto out;
		}

		win = Malloc(sizeof(struct window), M_OBJECT);
		window_init(win, name, flags);
		event_new(win, "window-close", window_generic_hide, "%p", win);
	} else {
		win = Malloc(sizeof(struct window), M_OBJECT);
		window_init(win, NULL, flags);
		event_new(win, "window-close", window_generic_detach, "%p",
		    win);
	}
	if (flags & WINDOW_HIDE) {
		event_new(win, "window-close", window_generic_hide, "%p", win);
	} else if (flags & WINDOW_DETACH) {
		event_new(win, "window-close", window_generic_detach, "%p",
		    win);
	}
	
	view_attach(win);
out:
	pthread_mutex_unlock(&view->lock);
	return (win);
}

void
window_init(void *p, const char *name, int flags)
{
	char wname[OBJECT_NAME_MAX];
	struct window *win = p;
	int titlebar_flags = 0;
	struct event *ev;
	int i;

	strlcpy(wname, "win-", sizeof(wname));
	strlcat(wname, (name != NULL) ? name : "generic", sizeof(wname));

	widget_init(win, "window", &window_ops, 0);
	object_set_type(win, "window");
	object_set_name(win, wname);

	widget_map_color(win, BGFILL_COLOR, "background-filling", 0, 0, 0, 255);
	widget_map_color(win, BG_COLOR, "background", 0, 0, 0, 255);
	
	win->flags = flags;
	win->visible = 0;

	pthread_mutex_init(&win->lock, &recursive_mutexattr);
	win->alignment = WINDOW_CENTER;
	win->spacing = 2;
	TAILQ_INIT(&win->subwins);
	win->h_border = NULL;
	win->v_border = NULL;
	win->h_border_w = 0;
	win->v_border_w = 0;
	win->xpadding = 0;
	win->ypadding = 0;
	win->minw = 0;
	win->minh = 0;
	
	/* Select the default window style. */
	window_set_style(win, &default_style);
	
	/* Create the titlebar unless disabled. */
	if (flags & WINDOW_NO_CLOSE)
		titlebar_flags |= TITLEBAR_NO_CLOSE;
	if (flags & WINDOW_NO_MINIMIZE)
		titlebar_flags |= TITLEBAR_NO_MINIMIZE;
	if ((flags & WINDOW_MAXIMIZE) == 0)
		titlebar_flags |= TITLEBAR_NO_MAXIMIZE;

	win->tbar = (flags & WINDOW_NO_TITLEBAR) ? NULL :
	    titlebar_new(win, titlebar_flags);

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

/* Apply style settings to a window, which will be inherited by children. */
void
window_set_style(struct window *win, const struct style *style)
{
	int i;

	WIDGET(win)->style = style;
	
	widget_map_color(win, HIGHLIGHT_COLOR, "highlight",
	    style->win.highlight.r,
	    style->win.highlight.g,
	    style->win.highlight.b, 255);

	if ((win->flags & WINDOW_NO_DECORATIONS) == 0) {
		for (i = 0; i < win->h_border_w+win->v_border_w; i++) {
			widget_pop_color(win);
		}
		win->h_border_w = style->win.h_border_w;
		win->v_border_w = style->win.v_border_w;
	} else {
		win->h_border_w = 0;
		win->v_border_w = 0;
	}

	for (i = 0; i < win->h_border_w; i++) {
		widget_push_color(win, SDL_MapRGB(vfmt,
		    style->win.h_border[i].r,
		    style->win.h_border[i].g,
		    style->win.h_border[i].b));
	}
	for (i = 0; i < win->v_border_w; i++) {
		widget_push_color(win, SDL_MapRGB(vfmt,
		    style->win.v_border[i].r,
		    style->win.v_border[i].g,
		    style->win.v_border[i].b));
	}
	
	win->xpadding = win->h_border_w+4;
	win->ypadding = win->v_border_w+4;
	win->minh = win->ypadding*2 + text_font_height;
	win->minw = win->xpadding*2;
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
	int i, ncolor = BORDER_BASE_COLOR;

	primitives.rect_filled(win, 0, 0, WIDGET(win)->w, WIDGET(win)->h,
	    BG_COLOR);

	if (win->flags & WINDOW_NO_DECORATIONS)
		return;

	/* Draw the window frame (expected to fit inside padding). */
	for (i = 1; i < win->h_border_w-1; i++, ncolor++) {
		primitives.line(win,
		    i,
		    i,
		    WIDGET(win)->w - i,
		    i,
		    ncolor);
		primitives.line(win,
		    i, 
		    WIDGET(win)->h - i,
		    WIDGET(win)->w - i,
		    WIDGET(win)->h - i,
		    ncolor);
	}
	for (i = 1; i < win->v_border_w-1; i++, ncolor++) {
		primitives.line(win,
		    i,
		    i,
		    i,
		    WIDGET(win)->h - i,
		    ncolor);
		primitives.line(win,
		    WIDGET(win)->w - i,
		    i,
		    WIDGET(win)->w - i,
		    WIDGET(win)->h - i,
		    ncolor);
	}

	if ((win->flags & WINDOW_NO_HRESIZE) == 0) {
		primitives.line(win,
		    18,
		    WIDGET(win)->h - win->h_border_w,
		    18,
		    WIDGET(win)->h - 2,
		    ncolor-1);
		primitives.line(win,
		    19,
		    WIDGET(win)->h - win->h_border_w,
		    19,
		    WIDGET(win)->h - 2,
		    HIGHLIGHT_COLOR);
		
		primitives.line(win,
		    WIDGET(win)->w - 19,
		    WIDGET(win)->h - win->h_border_w,
		    WIDGET(win)->w - 19,
		    WIDGET(win)->h - 2,
		    ncolor-1);
		primitives.line(win,
		    WIDGET(win)->w - 18,
		    WIDGET(win)->h - win->h_border_w,
		    WIDGET(win)->w - 18,
		    WIDGET(win)->h - 2,
		    HIGHLIGHT_COLOR);
	}
	
	if ((win->flags & WINDOW_NO_VRESIZE) == 0) {
		primitives.line(win,
		    2,
		    WIDGET(win)->h - 20,
		    win->v_border_w,
		    WIDGET(win)->h - 20,
		    ncolor-1);
		primitives.line(win,
		    2,
		    WIDGET(win)->h - 19,
		    win->v_border_w,
		    WIDGET(win)->h - 19,
		    HIGHLIGHT_COLOR);
		
		primitives.line(win,
		    WIDGET(win)->w - win->v_border_w,
		    WIDGET(win)->h - 20,
		    WIDGET(win)->w - 2,
		    WIDGET(win)->h - 20,
		    ncolor-1);
		primitives.line(win,
		    WIDGET(win)->w - win->v_border_w,
		    WIDGET(win)->h - 19,
		    WIDGET(win)->w - 2,
		    WIDGET(win)->h - 19,
		    HIGHLIGHT_COLOR);
	}
}

void
window_destroy(void *p)
{
	struct window *win = p;

	pthread_mutex_destroy(&win->lock);
	/* view_detach_queued() will free the sub-windows */
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

		/* Second pass: [wh]fill and homogenous divisions. */
		WIDGET_OPS(win)->scale(win, WIDGET(win)->w, WIDGET(win)->h);
	
		/* Position the window and cache the absolute widget coords. */
		apply_alignment(win);
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

	/* Update the background if necessary. */
	if (!window_surrounded(win)) {
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
	}

	if (win->flags & WINDOW_PERSISTENT)
		object_save(win);

	event_post(NULL, win, "window-hidden", NULL);
}

/*
 * Verify whether a given window resides inside the area of some other
 * window, to see whether is it necessary to update the background.
 */
int
window_surrounded(struct window *win)
{
	struct window *owin;

	TAILQ_FOREACH(owin, &view->windows, windows) {
		pthread_mutex_lock(&owin->lock);
		if (owin->visible &&
		    widget_area(owin, WIDGET(win)->x, WIDGET(win)->y) &&
		    widget_area(owin,
		     WIDGET(win)->x + WIDGET(win)->w,
		     WIDGET(win)->y + WIDGET(win)->h)) {
			pthread_mutex_unlock(&owin->lock);
			return (1);
		}
		pthread_mutex_unlock(&owin->lock);
	}
	return (0);
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
move_window(struct window *win, SDL_MouseMotionEvent *motion)
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
	clamp_to_view(win);

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
		if (!view->opengl) {
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
void
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

	/* Process the input events. */
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
				move_window(win, &ev->motion);
				goto out;
			case VIEW_WINOP_LRESIZE:
			case VIEW_WINOP_RRESIZE:
			case VIEW_WINOP_HRESIZE:
				resize_window(view->winop, win, &ev->motion);
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
			    (WIDGET(win)->h - win->ypadding)) {
				/* Initiate a resize operation. */
				/* XXX don't hardcode notch position */
			    	if ((ev->button.x - WIDGET(win)->x) < 17) {
					view->winop = VIEW_WINOP_LRESIZE;
					view->wop_win = win;
				} else if ((ev->button.x - WIDGET(win)->x) >
				           (WIDGET(win)->w - 17)) {
					view->winop = VIEW_WINOP_RRESIZE;
					view->wop_win = win;
				} else if (!(win->flags & WINDOW_NO_VRESIZE)) {
					view->winop = VIEW_WINOP_HRESIZE;
					view->wop_win = win;
				}
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
clamp_to_view(struct window *win)
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
resize_window(int op, struct window *win, SDL_MouseMotionEvent *motion)
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
		if (!(win->flags & WINDOW_NO_HRESIZE)) {
			if (motion->xrel < 0) {
				WIDGET(win)->w -= motion->xrel;
				nx += motion->xrel;
			} else if (motion->xrel > 0) {
				WIDGET(win)->w -= motion->xrel;
				nx += motion->xrel;
			}
		}
		if (!(win->flags & WINDOW_NO_VRESIZE)) {
			if (motion->yrel < 0 || motion->yrel > 0)
				WIDGET(win)->h += motion->yrel;
		}
		break;
	case VIEW_WINOP_RRESIZE:
		if (!(win->flags & WINDOW_NO_HRESIZE)) {
			if (motion->xrel < 0 || motion->xrel > 0)
				WIDGET(win)->w += motion->xrel;
		}
		if (!(win->flags & WINDOW_NO_VRESIZE)) {
			if (motion->yrel < 0 || motion->yrel > 0)
				WIDGET(win)->h += motion->yrel;
		}
		break;
	case VIEW_WINOP_HRESIZE:
		if (!(win->flags & WINDOW_NO_HRESIZE)) {
			if (motion->yrel < 0 || motion->yrel > 0)
				WIDGET(win)->h += motion->yrel;
		}
		break;
	default:
		break;
	}

	/* XXX check minimum */
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
		if (!view->opengl) {
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
	int x = win->xpadding;
	int y = win->ypadding;

	pthread_mutex_lock(&win->lock);

	if (w == -1 && h == -1) {
		int maxw = 0;

		WIDGET(win)->w = win->xpadding*2;
		WIDGET(win)->h = win->ypadding*2;

		OBJECT_FOREACH_CHILD(wid, win, widget) {
			wid->x = x;
			wid->y = y;

			WIDGET_OPS(wid)->scale(wid, -1, -1);	/* Hint */

			if (maxw < wid->w) {
				maxw = wid->w;
			}
			if ((maxw + win->xpadding*2) > WIDGET(win)->w) {
				WIDGET(win)->w = maxw + win->xpadding*2;
			}
			WIDGET(win)->h += wid->h + win->spacing;
			y += wid->h + win->spacing;
		}
#if 1
		/* Prevent further scaling of widgets to unaesthetic sizes. */
		win->minw = WIDGET(win)->w;
		win->minh = WIDGET(win)->h;
#endif
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
			wid->w = w - win->xpadding*2; 
		}
		if (wid->flags & WIDGET_HFILL) {
			wid->h = h - totfixed -
			    (nfixed*win->spacing + win->ypadding*2) -
			    win->spacing*2;
		}
		WIDGET_OPS(wid)->scale(wid, wid->w, wid->h);

		y += wid->h + win->spacing;
	}

	if (win->tbar != NULL) {
		WIDGET(win->tbar)->x = 1;
		WIDGET(win->tbar)->y = 1;
		WIDGET(win->tbar)->w = WIDGET(win)->w - 1;
	}
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
window_set_padding(struct window *win, int xpadding, int ypadding)
{
	pthread_mutex_lock(&win->lock);
	if (xpadding >= 0) {
		win->xpadding = xpadding;
	}
	if (ypadding >= 0) {
		win->ypadding = ypadding;
	}
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
window_set_closure(struct window *win, int mode)
{
	switch (mode) {
	case WINDOW_HIDE:
		event_new(win, "window-close", window_generic_hide, "%p", win);
		break;
	case WINDOW_DETACH:
		event_new(win, "window-close", window_generic_detach, "%p",
		    win);
		break;
	default:
		event_remove(win, "window-close");
		break;
	}
}

/*
 * Set the position of a window assuming its size is known.
 * The window must be locked.
 */
static void
apply_alignment(struct window *win)
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
		if ((window_xoffs += text_font_height)+WIDGET(win)->w >
		    view->w/2) {
			window_xoffs = 0;
		}
		if ((window_yoffs += win->v_border_w+4)+WIDGET(win)->h >
		    view->h/2) {
			window_yoffs = 0;
		}
		WIDGET(win)->x += window_xoffs;
		WIDGET(win)->y += window_yoffs;
		pthread_mutex_unlock(&window_lock);
	}
	clamp_to_view(win);
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
	titlebar_set_caption(win->tbar, s);
}

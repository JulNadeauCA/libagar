/*	$Csoft: window.c,v 1.180 2003/04/29 01:31:55 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/snprintf.h>
#include <engine/compat/vasprintf.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/config.h>
#include <engine/version.h>
#include <engine/view.h>

#include <engine/mapedit/mapedit.h>

#include <engine/widget/text.h>
#include <engine/widget/primitive.h>
#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

static const struct version window_ver = {
	"agar window",
	2, 0
};

static const struct object_ops window_ops = {
	window_destroy,
	window_load,
	window_save
};

enum {
	BACKGROUND_COLOR,
	BACKGROUND_FILL_COLOR,
	TITLEBAR_FOCUSED_COLOR,
	TITLEBAR_UNFOCUSED_COLOR,
	TITLEBAR_TEXT_UNFOCUSED_COLOR,
	TITLEBAR_TEXT_FOCUSED_COLOR,
	TITLEBAR_BUTTONS_UNFOCUSED_COLOR,
	TITLEBAR_BUTTONS_FOCUSED_COLOR
};

/* XXX struct */
#include "borders/green2.h"

static void	window_clamp(struct window *);
static void	window_focus(struct window *);
static void	winop_move(struct window *, SDL_MouseMotionEvent *);
static void	winop_resize(int, struct window *, SDL_MouseMotionEvent *);
static void	winop_close(struct window *);
static void	winop_hide_body(struct window *);
#if 0
static void	resize_reg(int, struct window *, struct region *);
#endif

#ifdef DEBUG
#define DEBUG_STATE		0x01
#define DEBUG_RESIZE		0x02
#define DEBUG_DRAW		0x04
#define DEBUG_RESIZE_GEO	0x08

int	window_debug = 0;
#define	engine_debug window_debug
#endif

pthread_mutex_t window_lock = PTHREAD_MUTEX_INITIALIZER;
Uint32 window_ngeneric = 0;
int window_xoffs = -1, window_yoffs = -1;
static SDL_Surface *close_icon, *hidden_body_icon, *hide_body_icon;
static int icons_inited = 0;

struct window *
window_new(char *name, int flags, int x, int y, int w, int h,
    int minw, int minh)
{
	struct window *win;

	win = Malloc(sizeof(struct window));
	window_init(win, name, flags, x, y, w, h, minw, minh);
	window_clamp(win);
	view_attach(win);			/* Attach to view */
	return (win);
}

/* Create a centered window, unique if name_fmt is non-NULL. */
struct window *
window_generic_new(int w, int h, const char *name_fmt, ...)
{
	struct window *win;
	va_list args;
	char *name;
	int xoffs, yoffs;

	if (name_fmt != NULL) {				/* Single instance */
		va_start(args, name_fmt);
		Vasprintf(&name, name_fmt, args);
		va_end(args);
		
		pthread_mutex_lock(&view->lock);
		TAILQ_FOREACH(win, &view->windows, windows) {
			if (strlen(OBJECT(win)->name) > 4 &&
			    strcmp(OBJECT(win)->name+4, name) == 0) {
			    	if ((win->flags & WINDOW_SHOWN) == 0) {
					window_show(win);
				}
				window_focus(win);
				pthread_mutex_unlock(&view->lock);
				error_set("window exists");
				return (NULL);
			}
		}
		pthread_mutex_unlock(&view->lock);
	} else {					/* Multiple instances */
		name = NULL;
	}

	win = Malloc(sizeof(struct window));

	pthread_mutex_lock(&window_lock);
	xoffs = window_xoffs;
	yoffs = window_yoffs;
	pthread_mutex_unlock(&window_lock);

	/* Initialize the window, figure out the initial coordinates. */
	window_init(win, name, 0,
	    view->w/2 - w/2 + xoffs,
	    view->h/2 - h/2 + yoffs,
	    w, h, w, h);

	pthread_mutex_lock(&window_lock);
	if ((window_xoffs += 3) + w > view->w/2)
		window_xoffs = 0;
	if ((window_yoffs += 3) + h > view->h/2)
		window_yoffs = 0;
	pthread_mutex_unlock(&window_lock);

	window_clamp(win);
	
	free(name);

	/* Detach by default. */
	event_new(win, "window-close", window_generic_detach, "%p", win);
	view_attach(win);
	return (win);
}

static void
window_init_icons(struct window *win)
{
	int i;

	dprintf("initing icons\n");

	i = win->titleh - win->borderw;
	close_icon = view_scale_surface(SPRITE(win, 0), i, i);
	hidden_body_icon = view_scale_surface(SPRITE(win, 1), i, i);
	hide_body_icon = view_scale_surface(SPRITE(win, 2), i, i);
	
	icons_inited = 1;
}

void
window_init(struct window *win, char *name, int flags, int rx, int ry,
    int rw, int rh, int minw, int minh)
{
	char wname[OBJECT_NAME_MAX];
	int i, fl = flags;

	if (name != NULL) {					/* Unique */
		snprintf(wname, sizeof(wname), "win-%s", name);
		fl |= WINDOW_SAVE_POSITION;
	} else {						/* Generic */
		pthread_mutex_lock(&window_lock);
		window_ngeneric++;
		pthread_mutex_unlock(&window_lock);
		snprintf(wname, sizeof(wname), "win-generic%u",
		    window_ngeneric++);
	}

	for (i = 0; i < sizeof(wname); i++) {	
		if (wname[i] == '\0')
			break;
		if (wname[i] == '/') 		/* XXX restrict further */
			wname[i] = '_';
	}
	object_init(OBJECT(win), "window", wname, 0, &window_ops);
	if (object_load_art(win, "window", 1) == -1)
		fatal("window: %s", error_get());
	
	widget_map_color(&win->wid, BACKGROUND_COLOR,
	    "background",
	    0, 40, 20);
	widget_map_color(&win->wid, BACKGROUND_FILL_COLOR,
	    "background-fill",
	    0, 0, 0);

	widget_map_color(&win->wid, TITLEBAR_UNFOCUSED_COLOR,
	    "titlebar-background-unfocused",
	    0, 60, 40);
	widget_map_color(&win->wid, TITLEBAR_TEXT_UNFOCUSED_COLOR,
	    "titlebar-text-unfocused",
	    20, 100, 100);
	widget_map_color(&win->wid, TITLEBAR_BUTTONS_UNFOCUSED_COLOR,
	    "titlebar-buttons-unfocused",
	    20, 100, 100);

	widget_map_color(&win->wid, TITLEBAR_FOCUSED_COLOR,
	    "titlebar-background-focused",
	    0, 90, 90);
	widget_map_color(&win->wid, TITLEBAR_TEXT_FOCUSED_COLOR,
	    "titlebar-text-focused",
	    80, 200, 200);
	widget_map_color(&win->wid, TITLEBAR_BUTTONS_FOCUSED_COLOR,
	    "titlebar-buttons-focused",
	    70, 120, 120);

	/* XXX pref */
	win->borderw = default_nborder;
	win->border = Malloc(win->borderw * sizeof(Uint32));
	for (i = 0; i < win->borderw; i++) {
		win->border[i] = SDL_MapRGB(view->v->format,
		    default_border[i].r, default_border[i].g,
		    default_border[i].b);
	}

	fl |= (WINDOW_TITLEBAR);
	win->flags = fl;
	win->clicked_button = WINDOW_NO_BUTTON;

	win->titleh = text_font_height(font) + win->borderw;
	win->focus = NULL;
	win->caption = Strdup("Untitled");
	win->minw = minw;
	win->minh = minh;
	win->xspacing = 3;
	win->yspacing = 3;

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
	
	window_clamp(win);			/* Clamp to view area */

	/* Fictitious widget geometry, for primitive operations and colors. */
	win->wid.win = win;
	win->wid.x = 0;
	win->wid.y = 0;
	win->wid.w = 0;
	win->wid.h = 0;

	TAILQ_INIT(&win->regionsh);
	pthread_mutex_init(&win->lock, &recursive_mutexattr);
	
	/* Prescale the titlebar icons. */
	pthread_mutex_lock(&window_lock);
	if (!icons_inited)
		window_init_icons(win);
	pthread_mutex_unlock(&window_lock);
}

/*
 * Render the window frame and decorations.
 * The window must be locked.
 */
static void
window_draw_frame(struct window *win)
{
	int i;

	/* Draw the border. */
	for (i = 1; i < win->borderw; i++) {
		primitives.line(win,		/* Top */
		    i,			i,
		    win->rd.w - i,	i,
		    win->border[i]);
		if ((win->flags & WINDOW_HIDDEN_BODY) == 0) {
			primitives.line(win,		/* Bottom */
			    i,			win->rd.h - i,
			    win->rd.w - i,	win->rd.h - i,
			    win->border[i]);
			primitives.line(win,		/* Left */
			    i,			i,
			    i,			win->rd.h - i,
			    win->border[i]);
			primitives.line(win,		/* Right */
			    win->rd.w - i,	i,
			    win->rd.w - i,	win->rd.h - i,
			    win->border[i]);
		} else {
			primitives.line(win,		/* Left */
			    i,			i,
			    i,			win->titleh + win->borderw - i,
			    win->border[i]);
			primitives.line(win,		/* Right */
			    win->rd.w - i,	i,
			    win->rd.w - i, 	win->titleh + win->borderw - i,
			    win->border[i]);
			primitives.line(win,		/* Bottom */
			    i,			win->titleh + win->borderw - i,
			    win->rd.w - i,	win->titleh + win->borderw - i,
			    win->border[i]);
		}
	}
	
	/* Draw the resize decorations. */
	if ((win->flags & WINDOW_HIDDEN_BODY) == 0) {
		primitives.line(win,				/* Lower left */
		    18,			win->rd.h - win->borderw,
		    18,			win->rd.h - 2,
		    win->border[0]);
		primitives.line(win,
		    19,			win->rd.h - win->borderw,
		    19,			win->rd.h - 2,
		    win->border[win->borderw/2]);
		primitives.line(win,
		    2,		  	win->rd.h - 20,
		    win->borderw,	win->rd.h - 20,
		    win->border[0]);
		primitives.line(win,
		    2,			win->rd.h - 19,
		    win->borderw,	win->rd.h - 19,
		    win->border[win->borderw/2]);
	
		primitives.line(win,			       /* Lower right */
		    win->rd.w - 19,	win->rd.h - win->borderw,
		    win->rd.w - 19,	win->rd.h - 2,
		    win->border[0]);
		primitives.line(win,
		    win->rd.w - 18,	win->rd.h - win->borderw,
		    win->rd.w - 18,	win->rd.h - 2,
		    win->border[win->borderw/2]);
		primitives.line(win,
		    win->rd.w - win->borderw,	win->rd.h - 20,
		    win->rd.w - 2,		win->rd.h - 20,
		    win->border[0]);
		primitives.line(win,
		    win->rd.w - win->borderw,	win->rd.h - 19,
		    win->rd.w - 2,		win->rd.h - 19,
		    win->border[win->borderw/2]);
	}
}

/*
 * Render the window's titlebar.
 * The window must be locked.
 */
static void
window_draw_titlebar(struct window *win)
{
	int bw = win->borderw + 2;
	int th = win->titleh - 2;
	SDL_Surface *caption;
	SDL_Rect rd, rclip, rclip_save;
	Uint32 bcolor;

	/* XXX yuck */
	rd.x = win->borderw;
	rd.y = win->borderw;
	rd.w = win->rd.w - win->borderw*2+1;
	rd.h = win->titleh - win->borderw/2;

	/* Draw the titlebar background. */
	primitives.rect_filled(win, &rd,
	    WIDGET_COLOR(win, WINDOW_FOCUSED(win) ?
	    TITLEBAR_FOCUSED_COLOR : TITLEBAR_UNFOCUSED_COLOR));

	rd.x += win->rd.x;
	rd.y += win->rd.y;
	rd.w = win->rd.w;
	rd.h = win->rd.h;
	
	/* Draw the window caption. XXX inefficient */
	rclip = rd;
	rclip.x += th*2;			/* Buttons */
	SDL_GetClipRect(view->v, &rclip_save);	/* Save clipping rectangle */
	SDL_SetClipRect(view->v, &rclip);
	caption = text_render(NULL, -1, WINDOW_FOCUSED(win) ?
	    WIDGET_COLOR(win, TITLEBAR_TEXT_FOCUSED_COLOR) :
	    WIDGET_COLOR(win, TITLEBAR_TEXT_UNFOCUSED_COLOR), win->caption);
	widget_blit(win, caption,
	    win->rd.w - caption->w - win->borderw,
	    win->borderw);
	SDL_FreeSurface(caption);
	SDL_SetClipRect(view->v, &rclip_save);	/* Restore clipping rectangle */

	/* Buttons */
	bcolor = WINDOW_FOCUSED(win) ?
	    WIDGET_COLOR(win, TITLEBAR_BUTTONS_FOCUSED_COLOR) :
	    WIDGET_COLOR(win, TITLEBAR_BUTTONS_UNFOCUSED_COLOR);
	primitives.box(win, bw-1, bw-1, th-1, th-4,		/* Close */
	    (win->clicked_button == WINDOW_CLOSE_BUTTON) ? -1 : 1,
	    bcolor);
	widget_blit(win, close_icon, bw, bw-1);

	primitives.box(win, th+bw-1, bw-1, th-1, th-4,		/* Hide */
	    (win->clicked_button == WINDOW_HIDE_BUTTON) ? -1 : 1,
	    bcolor);
	if (win->flags & WINDOW_HIDDEN_BODY) {
		widget_blit(win, hidden_body_icon, th+bw, bw-1);
	} else {
		widget_blit(win, hide_body_icon, th+bw, bw-1);
	}

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

/*
 * Render a window.
 * The window must be locked.
 */
void
window_draw(struct window *win)
{
	struct region *reg;
	struct widget *wid;
	SDL_Rect rd = win->rd;
	
	debug_n(DEBUG_DRAW, "drawing %s (%ux%u):\n", OBJECT(win)->name,
	    win->rd.w, win->rd.h);

	/* Fill the background. */
	if ((win->flags & WINDOW_HIDDEN_BODY) == 0) {
		rd.x = 0;
		rd.y = 0;
		primitives.rect_filled(win, &rd,
		    WIDGET_COLOR(win, BACKGROUND_FILL_COLOR));
	}
	
	/* Draw the title bar. */
	if (win->flags & WINDOW_TITLEBAR) {
		window_draw_titlebar(win);
	}

	/* Render the widgets. */
	if ((win->flags & WINDOW_HIDDEN_BODY) == 0) {
		TAILQ_FOREACH(reg, &win->regionsh, regions) {
			SDL_Rect regclip_save;

			debug_n(DEBUG_DRAW, " %s(%d,%d)\n", OBJECT(reg)->name,
			    reg->x, reg->y);

			if (reg->flags & REGION_CLIPPING) {
				SDL_Rect regclip;

				regclip.x = win->rd.x+reg->x;
				regclip.y = win->rd.y+reg->y;
				regclip.w = reg->w;
				regclip.h = reg->h;
				SDL_GetClipRect(view->v, &regclip_save);
				SDL_SetClipRect(view->v, &regclip);
			}

			TAILQ_FOREACH(wid, &reg->widgets, widgets) {
				SDL_Rect widclip_save;
			
				debug_n(DEBUG_DRAW, "  %s(%d,%d)\n",
				    OBJECT(wid)->name,
				    wid->x, wid->y);

				if (wid->flags & WIDGET_CLIPPING) {
					SDL_Rect widclip;

					widclip.x = WIDGET_ABSX(wid);
					widclip.y = WIDGET_ABSY(wid);
					widclip.w = wid->w;
					widclip.h = wid->h;
					SDL_GetClipRect(view->v, &widclip_save);
					SDL_SetClipRect(view->v, &widclip);
				}

				if (wid->w > 0 && wid->h > 0) {
					if (wid->x < 0 || wid->y < 0) {
						fatal("neg widget coords");
					}
					WIDGET_OPS(wid)->widget_draw(wid);
				}

				if (wid->flags & WIDGET_CLIPPING) {
					SDL_SetClipRect(view->v, &widclip_save);
				}
			}
			
			if (reg->flags & REGION_CLIPPING) {
				SDL_SetClipRect(view->v, &regclip_save);
			}

#ifdef DEBUG
			if (prop_get_bool(config, "widget.reg-borders")) {
				primitives.rect_outlined(win,
				    reg->x, reg->y,
				    reg->w, reg->h,
				    SDL_MapRGB(view->v->format, 255, 255, 255));
			}
#endif
		}
	}

	/* Draw the window border and decorations. */
	window_draw_frame(win);

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

	object_destroy(reg);
	free(reg);
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
		object_destroy(reg);
		free(reg);
	}

	free(win->caption);
	free(win->border);
	pthread_mutex_destroy(&win->lock);
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
		object_load(win, NULL);
	}

	view->focus_win = win;		/* Focus */

	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgets, widgets) {
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

	view->focus_win = NULL;

	/* Notify the widgets. */
	TAILQ_FOREACH(reg, &win->regionsh, regions) {
		TAILQ_FOREACH(wid, &reg->widgets, widgets) {
			event_post(wid, "widget-hidden", "%p", win);
		}
	}
	
	win->flags &= ~(WINDOW_SHOWN);

	/* Update the background. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_GUI:
		{
			SDL_Rect rfill = win->rd;

			rfill.x = 0;
			rfill.y = 0;
			primitives.rect_filled(win, &rfill,
			    WIDGET_COLOR(win, BACKGROUND_FILL_COLOR));
		
			if (!view->opengl) {
				SDL_UpdateRect(view->v, win->rd.x, win->rd.y,
				    win->rd.w, win->rd.h);
			}
		}
		break;
	case GFX_ENGINE_TILEBASED:
		if (view->rootmap != NULL) {
			view->rootmap->map->redraw++;
		}
		break;
	}

	/* Save the window position and geometry. */
	if (win->flags & WINDOW_SAVE_POSITION) {
		object_save(win, NULL);
	}

	pthread_mutex_unlock(&win->lock);
	pthread_mutex_unlock(&view->lock);
	return (1);
}

/*
 * Cycle focus throughout widgets.
 * The window must be locked.
 */
static void
cycle_widgets(struct window *win, int reverse)
{
	struct widget *wid = win->focus;
	struct region *nreg, *rreg;
	struct widget *owid, *rwid;
	struct widget *nwid;

	if (wid == NULL) {
		/* No focus */
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
		TAILQ_FOREACH(owid, &nreg->widgets, widgets) {
			if (owid != wid) {
				continue;
			}
			if (reverse) {
				rreg = TAILQ_PREV(nreg, regionsq, regions);
				if (rreg != NULL) {
					rwid = TAILQ_LAST(&rreg->widgets,
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
						    &rreg->widgets, widgetsq);
						if (rwid != NULL) {
							WIDGET_FOCUS(rwid);
							return;
						}
					}
				}
			} else {
				rreg = TAILQ_NEXT(nreg, regions);
				if (rreg != NULL) {
					rwid = TAILQ_FIRST(&rreg->widgets);
					if (rwid != NULL) {
						WIDGET_FOCUS(rwid);
						return;
					}
				} else {
					rreg = TAILQ_FIRST(&win->regionsh);
					if (rreg != NULL) {
						rwid = TAILQ_FIRST(
						    &rreg->widgets);
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
 * Move a window using the mouse.
 * The view and window must be locked.
 */
static void
winop_move(struct window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect oldpos, newpos, rfill1, rfill2;
	Uint32 fillcolor = WIDGET_COLOR(win, BACKGROUND_FILL_COLOR);

	if (view->gfx_engine == GFX_ENGINE_GUI) {
		oldpos = win->rd;
	}

	/* Update the window coordinates, adjust to view area. */
	win->rd.x += motion->xrel;
	win->rd.y += motion->yrel;
	window_clamp(win);

	/* Update the background. */
	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		view->rootmap->map->redraw++;
		break;
	case GFX_ENGINE_GUI:
		newpos = win->rd;
		if (win->flags & WINDOW_HIDDEN_BODY) {
			oldpos.h = win->titleh + win->borderw;
			newpos.h = oldpos.h;
		}
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

			SDL_GetRGB(fillcolor, view->v->format, &r, &g, &b);
			glColor3ub(r, g, b);

			if (rfill1.w > 0) {
				glRecti(rfill1.x, rfill1.y,
				    rfill1.x + rfill1.w,
				    rfill1.y + rfill1.h);
			}
			if (rfill2.w > 0) {
				glRecti(rfill2.x, rfill2.y,
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
 * React to the close button.
 * The window must be locked.
 */
static void
winop_close(struct window *win)
{
	window_hide(win);
	event_post(win, "window-close", NULL);
}

/*
 * React to the hide button.
 * The window must be locked.
 */
static void
winop_hide_body(struct window *win)
{
	if (win->flags & WINDOW_HIDDEN_BODY) {		/* Restore */
		win->flags &= ~(WINDOW_HIDDEN_BODY);
		win->rd.w = win->saved_rd.w;
		win->rd.h = win->saved_rd.h;
	} else {					/* Hide */
		int titleh = win->titleh + win->borderw;

		win->flags |= WINDOW_HIDDEN_BODY;
		win->saved_rd = win->rd;
		win->rd.h = titleh;

		/* Update the background. */
		switch (view->gfx_engine) {
		case GFX_ENGINE_GUI:
			{
				SDL_Rect rbody = win->saved_rd;

				rbody.x = 0;
				rbody.y = titleh;
				rbody.h -= titleh;

				primitives.rect_filled(win, &rbody,
				    WIDGET_COLOR(win, BACKGROUND_FILL_COLOR));

				if (!view->opengl) {
					rbody.x = win->saved_rd.x;
					rbody.y = win->saved_rd.y + titleh;
					SDL_UpdateRects(view->v, 1, &rbody);
				}
			}
			break;
		case GFX_ENGINE_TILEBASED:
			view->rootmap->map->redraw++;
			break;
		}
	}

	window_resize(win);				/* Clamp */
}

/*
 * Give focus to a window.
 * The view and window must be locked.
 */
static void
window_focus(struct window *win)
{
	struct window *lastwin;

	view->focus_win = NULL;

	lastwin = TAILQ_LAST(&view->windows, windowq);
	if (win != NULL && lastwin == win) {		/* Already focused? */
		return;
	}

	if (lastwin != NULL) {
#if 0
		/* XXX */
		if (lastwin->focus != NULL) {
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
 * The view must be locked, and the window list must not be empty.
 */
int
window_event(SDL_Event *ev)
{
	struct region *reg;
	struct window *win;
	struct widget *wid;
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
			default:
				break;
			}
			if (win->flags & WINDOW_HIDDEN_BODY) {
				/* Don't catch events. */
				goto next_win;
			}
			/*
			 * Post the mouse motion event to the widget that
			 * holds the focus inside the focused window, and
			 * to any widget with the WIDGET_UNFOCUSED_MOTION
			 * flag set.
			 */
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
#if 0
				if (reg->flags & REGION_RESIZING) {
					switch (view->winop) {
					case VIEW_WINOP_REGRESIZE_LEFT:
					case VIEW_WINOP_REGRESIZE_RIGHT:
					default:
						break;
					}
					window_resize(win);
				}
#endif
				TAILQ_FOREACH(wid, &reg->widgets, widgets) {
					if ((WINDOW_FOCUSED(win) &&
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

			switch (win->clicked_button) {
			case WINDOW_NO_BUTTON:
				break;
			case WINDOW_CLOSE_BUTTON:
				winop_close(win);
				break;
			case WINDOW_HIDE_BUTTON:
				winop_hide_body(win);
				break;
			}
			win->clicked_button = WINDOW_NO_BUTTON;

			if (win->flags & WINDOW_HIDDEN_BODY) {
				/* Don't catch events. */
				goto next_win;
			}
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
				/* Clear the resize operation flag. */
				reg->flags &= ~(REGION_RESIZING);

				/* Send the mousebuttonup event to widgets. */
				TAILQ_FOREACH(wid, &reg->widgets, widgets) {
					if ((WINDOW_FOCUSED(win) &&
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
			win->clicked_button = WINDOW_NO_BUTTON;
			if (!WINDOW_INSIDE(win, ev->button.x, ev->button.y)) {
				goto next_win;
			}
			if (ev->button.y - win->rd.y <= win->titleh) {
			    	if (ev->button.x - win->rd.x <
				    win->titleh + win->borderw) {
					win->clicked_button =
					    WINDOW_CLOSE_BUTTON;
					goto posted;
				} else if (ev->button.x - win->rd.x <
				    win->titleh*2 + win->borderw) {
					win->clicked_button =
					    WINDOW_HIDE_BUTTON;
					goto posted;
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
			if (win->flags & WINDOW_HIDDEN_BODY) {
				/* Don't catch events. */
				goto next_win;
			}
			TAILQ_FOREACH(reg, &win->regionsh, regions) {
#if 0
				/* Select this region for resize? */
				if (ev->button.x == win->rd.x+reg->x) {
					resize_reg(VIEW_WINOP_REGRESIZE_LEFT,
					    win, reg);
					goto posted;
				} else if (ev->button.x == win->rd.x+reg->x+
				    reg->w) {
					resize_reg(VIEW_WINOP_REGRESIZE_RIGHT,
					    win, reg);
					goto posted;
				} else if (ev->button.y == win->rd.x+reg->y) {
					resize_reg(VIEW_WINOP_REGRESIZE_UP,
					    win, reg);
					goto posted;
				} else if (ev->button.y == win->rd.y+reg->y+
				    reg->h) {
					resize_reg(VIEW_WINOP_REGRESIZE_DOWN,
					    win, reg);
					goto posted;
				}
#endif
				/* Post mousebuttondown event to widget. */
				TAILQ_FOREACH(wid, &reg->widgets, widgets) {
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
			if (win->flags & WINDOW_HIDDEN_BODY) {
				/* Don't catch events. */
				goto next_win;
			}
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
			if (WINDOW_FOCUSED(win) && win->focus != NULL) {
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
	if (win->rd.x < 0)
		win->rd.x = 0;
	if (win->rd.y < 0)
		win->rd.y = 0;
	if (win->rd.w > view->w)
		win->rd.w = view->w;
	if (win->rd.h > view->h)
		win->rd.h = view->h;
	if (win->rd.x+win->rd.w > view->w)
		win->rd.x = view->w - win->rd.w;
	if (win->rd.y+win->rd.h > view->h)
		win->rd.y = view->h - win->rd.h;
}

/*
 * Resize a window with the mouse.
 * The window must be locked.
 */
static void
winop_resize(int op, struct window *win, SDL_MouseMotionEvent *motion)
{
	Uint32 fillcolor = WIDGET_COLOR(win, BACKGROUND_FILL_COLOR);
	SDL_Rect ro, rfill1, rfill2;
	int nx, ny;

	ro = win->rd;
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
	    !prop_get_bool(config, "widget.any-size")) {
		win->rd.w = win->minw;
	} else {
		win->rd.x = nx;
	}
	if (win->rd.h < win->minh &&
	    !prop_get_bool(config, "widget.any-size")) {
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
	switch (view->gfx_engine) {
	case GFX_ENGINE_GUI:
		rfill1.w = 0;
		rfill2.w = 0;

		if (win->rd.x > ro.x) {			/* L-resize */
			rfill1.x = ro.x;
			rfill1.y = ro.y;
			rfill1.w = win->rd.x - ro.x;
			rfill1.h = win->rd.h;
		} else if (win->rd.w < ro.w) {		/* R-resize */
			rfill1.x = win->rd.x + win->rd.w;
			rfill1.y = win->rd.y;
			rfill1.w = ro.w - win->rd.w;
			rfill1.h = ro.h;
		}
		if (win->rd.h < ro.h) {			/* H-resize */
			rfill2.x = ro.x;
			rfill2.y = win->rd.y + win->rd.h;
			rfill2.w = ro.w;
			rfill2.h = ro.h - win->rd.h;
		}
#ifdef HAVE_OPENGL
		if (view->opengl) {
			Uint8 r, g, b;

			SDL_GetRGB(fillcolor, view->v->format, &r, &g, &b);
			glColor3ub(r, g, b);

			if (rfill1.w > 0) {
				glRecti(rfill1.x, rfill1.y,
				    rfill1.x + rfill1.w,
				    rfill1.y + rfill1.h);
			}
			if (rfill2.w > 0) {
				glRecti(rfill2.x, rfill2.y,
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
		if (view->rootmap != NULL) {
			view->rootmap->map->redraw++;
		}
		break;
	}
}

#if 0
/*
 * Resize a region with the mouse.
 * The window must be locked.
 */
static void
resize_reg(int op, struct window *win, struct region *reg)
{
	if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) == 0) {
		return;
	}
	view->winop = op;
	view->wop_win = win;
	reg->flags |= REGION_RESIZING;
}
#endif

/* Update the window's caption. */
void
window_set_caption(struct window *win, const char *fmt, ...)
{
	va_list args;

	pthread_mutex_lock(&win->lock);

	/* XXX */
	Free(win->caption);
	va_start(args, fmt);
	Vasprintf(&win->caption, fmt, args);
	va_end(args);
	
	pthread_mutex_unlock(&win->lock);
}

int
window_load(void *p, struct netbuf *buf)
{
	struct window *win = p;
	Uint16 view_w, view_h;

	if (version_read(buf, &window_ver, NULL) != 0)
		return (-1);

	win->flags |= read_uint32(buf);
	
	view_w = read_uint16(buf);
	view_h = read_uint16(buf);

	win->rd.x = read_sint16(buf) * view->v->w / view_w;
	win->rd.y = read_sint16(buf) * view->v->h / view_h;
	win->rd.w = read_uint16(buf) * view->v->w / view_w;
	win->rd.h = read_uint16(buf) * view->v->h / view_h;

	win->saved_rd.x = read_sint16(buf) * view->v->w / view_w;
	win->saved_rd.y = read_sint16(buf) * view->v->h / view_h;
	win->saved_rd.w = read_uint16(buf) * view->v->w / view_w;
	win->saved_rd.h = read_uint16(buf) * view->v->h / view_h;
	
	debug(DEBUG_STATE, "%s: %dx%d for %dx%d at [%d,%d]\n",
	    OBJECT(win)->name, win->rd.w, win->rd.h, view_w, view_h,
	    win->rd.x, win->rd.y);

	/* Effect the change, ensure the window fits inside the view area. */
	window_resize(win);

	return (0);
}

int
window_save(void *p, struct netbuf *buf)
{
	struct window *win = p;

	debug(DEBUG_STATE, "saving %s: %dx%d at [%d,%d]\n", OBJECT(win)->name,
	    win->rd.w, win->rd.h, win->rd.x, win->rd.y);

	version_write(buf, &window_ver);

	write_uint32(buf, win->flags & WINDOW_PERSISTENT);
	
	write_uint16(buf, view->v->w);
	write_uint16(buf, view->v->h);

	write_sint16(buf, win->rd.x);
	write_sint16(buf, win->rd.y);
	write_uint16(buf, win->rd.w);
	write_uint16(buf, win->rd.h);

	write_sint16(buf, win->saved_rd.x);
	write_sint16(buf, win->saved_rd.y);
	write_uint16(buf, win->saved_rd.w);
	write_uint16(buf, win->saved_rd.h);

	return (0);
}

void
window_generic_detach(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	OBJECT_ASSERT(win, "window");

	view_detach(win);
}

void
window_generic_show(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	OBJECT_ASSERT(win, "window");

	if ((win->flags & WINDOW_SHOWN) == 0) {
		window_show(win);
	}
}

void
window_generic_hide(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	OBJECT_ASSERT(win, "window");

	if (win->flags & WINDOW_SHOWN) {
		window_hide(win);
	}
}

void
window_resize(struct window *win)
{
	struct region *reg;
	int regx, regy;
	int xmar, ymar;

	debug_n(DEBUG_RESIZE, "resizing %s (%dx%d):\n", OBJECT(win)->name,
	    win->rd.w, win->rd.h);

	if (!prop_get_bool(config, "widget.any-size") &&
	    (win->flags & WINDOW_HIDDEN_BODY) == 0) {
		if (win->rd.w < win->minw)
			win->rd.w = win->minw;
		if (win->rd.h < win->minh)
			win->rd.h = win->minh;
	}

	window_clamp(win);

	win->body.x = win->rd.x + win->borderw;
	win->body.y = win->rd.y + win->borderw*2 + win->titleh;
	win->body.w = win->rd.w - win->borderw*2;
	win->body.h = win->rd.h - win->borderw*2 - win->titleh;

	win->wid.w = win->rd.w;
	win->wid.h = win->rd.h;

	xmar = win->borderw + win->xspacing + 1;
	ymar = win->borderw + win->titleh + 1;
	regx = xmar;
	regy = ymar;

	TAILQ_FOREACH_REVERSE(reg, &win->regionsh, regions, regionsq) {
		struct widget *wid;
		int x, y;

		debug_n(DEBUG_RESIZE, " %s(%d,%d)\n", OBJECT(reg)->name,
		    reg->x, reg->y);

		/* Set the region's effective coordinates. */
		if (reg->rx > 0) {			/* % of width */
			reg->x = reg->rx * win->body.w / 100 +
			    (win->body.x - win->rd.x);
			reg->x += win->xspacing;
		} else {				/* auto position */
			reg->x = regx;
		}
		if (reg->ry > 0) {			/* % of height */
			reg->y = (reg->ry * win->body.h / 100) +
			    (win->body.y - win->rd.y);
			reg->y += win->yspacing;
		} else {				/* auto position */
			reg->y = regy;
		}

		/* Set the region's effective width. */
		if (reg->rw < 0) {			/* auto size width */
			reg->w = 0;
			reg->x += win->xspacing;
			TAILQ_FOREACH(wid, &reg->widgets, widgets) {
				if (wid->rw != -1) {
					dprintf("%s has scaled width\n",
					    OBJECT(wid)->name);
					continue;
				}
				event_post(wid, "widget-scaled",
				    "%i, %i", -1, -1);
				if (reg->flags & REGION_HALIGN)
					reg->w += wid->w + reg->xspacing;
				else if (wid->h > reg->h)
					reg->w = wid->w;
			}
		} else if (reg->rw > 0) {		/* % of window width */
			if (reg->nwidgets > 1 && reg->flags & REGION_HALIGN) {
				reg->w = reg->rw *
				    (win->body.w -
				     win->xspacing*(reg->nwidgets-1)) / 100;
			} else {
				reg->w = reg->rw *
				    (win->body.w - win->xspacing) / 100;
			}
			reg->w -= win->xspacing;
		} else if (reg->rw == 0) {		/* remaining space */
			reg->x += win->xspacing;
			reg->w = win->body.w - (regx - xmar) - win->xspacing;
		}
			
		/* Set the region's effective height. */
		if (reg->rh < 0) {			/* auto size height */
			reg->h = 0;
			reg->y += win->yspacing;
			TAILQ_FOREACH(wid, &reg->widgets, widgets) {
				if (wid->rh != -1) {
					dprintf("%s has scaled height\n",
					    OBJECT(wid)->name);
					continue;
				}
				event_post(wid, "widget-scaled", "%i, %i",
				    -1, -1);
				if (reg->flags & REGION_VALIGN)
					reg->h += wid->h + reg->yspacing;
				else if (wid->h > reg->h)
					reg->h = wid->h;
			}
		} else if (reg->rh > 0) {		/* % of window height */
			if (reg->nwidgets > 1 && reg->flags & REGION_VALIGN) {
				reg->h = reg->rh *
				    (win->body.h -
				     win->yspacing*(reg->nwidgets-1)) / 100;
				reg->h -= win->yspacing;
			} else {
				reg->h = reg->rh *
				    (win->body.h - win->yspacing*2) / 100;
			}
		} else if (reg->rh == 0) {		/* remaining space */
			reg->y += win->yspacing;
			reg->h = win->body.h - (regy - ymar) - win->yspacing;
		}

		/* Resize the widgets. */
		x = reg->x;
		y = reg->y;

		TAILQ_FOREACH(wid, &reg->widgets, widgets) {
			debug_n(DEBUG_RESIZE, "  %s(%d,%d)\n",
			    OBJECT(wid)->name, x, y);

			/* Set the widget's effective coordinates. */
			wid->x = x;
			wid->y = y;

			/* Set the widget's effective width. */
			if (wid->rw > 0) {		/* % of region */
				if (reg->flags & REGION_HALIGN) {
					wid->w = (wid->rw *
					    (reg->w - ((reg->nwidgets-1) *
					     reg->xspacing))) / 100;
				} else {
					wid->w = wid->rw * reg->w / 100;
				}
			} else if (wid->rw == 0) {	/* auto size width */
				if (reg->flags & REGION_HALIGN) {
					wid->w = reg->w/reg->nwidgets -
					    reg->xspacing*(reg->nwidgets-1);
				} else {
					wid->w = reg->w;
				}
			}
			
			/* Set the widget's effective height. */
			if (wid->rh > 0) {		/* % of region */
				if (reg->flags & REGION_VALIGN) {
					wid->h = (wid->rh *
					    (reg->h - ((reg->nwidgets-1) *
					     reg->yspacing))) / 100;
				} else {
					wid->h = wid->rh * reg->h / 100;
				}
			} else if (wid->rh == 0) {	/* auto size height */
				if (reg->flags & REGION_VALIGN) {
					wid->h = reg->h/reg->nwidgets -
					   reg->yspacing*(reg->nwidgets-1);
				} else {
					wid->h = reg->h;
				}
			}
			if (wid->w > reg->w)
				wid->w = reg->w - x;
			if (wid->h > reg->h)
				wid->h = reg->h - y;

			event_post(wid, "widget-scaled", "%i, %i",
			    reg->w, reg->h);

			/* Move */
			if (reg->flags & REGION_VALIGN) {
				y += wid->h + reg->yspacing;
			} else {
				x += wid->w + reg->xspacing;
			}
		}
		
		if (reg->rw == -1)
			regx += reg->w + win->xspacing;
		if (reg->rh == -1)
			regy += reg->h + win->yspacing;
 	}

	debug_n(DEBUG_RESIZE_GEO, "%s: %dx%d\n", OBJECT(win)->name,
	    win->rd.w, win->rd.h);
}

void
window_set_spacing(struct window *win, Uint8 xsp, Uint8 ysp)
{
	pthread_mutex_lock(&win->lock);
	win->xspacing = xsp;
	win->yspacing = ysp;
	pthread_mutex_unlock(&win->lock);
}

void
window_set_geo(struct window *win, Uint16 w, Uint16 h)
{
	pthread_mutex_lock(&win->lock);
	win->rd.w = w;
	win->rd.h = h;
	pthread_mutex_unlock(&win->lock);
}

void
window_set_position(struct window *win, Sint16 x, Sint16 y)
{
	pthread_mutex_lock(&win->lock);
	win->rd.x = x;
	win->rd.y = y;
	pthread_mutex_unlock(&win->lock);
}

void
window_set_min_geo(struct window *win, Uint16 minw, Uint16 minh)
{
	pthread_mutex_lock(&win->lock);
	win->minw = minw;
	win->minh = minh;
	pthread_mutex_unlock(&win->lock);
}

void
window_set_titleh(struct window *win, Uint8 h)
{
	pthread_mutex_lock(&win->lock);
	win->titleh = h;
	pthread_mutex_unlock(&win->lock);
}

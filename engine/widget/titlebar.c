/*	$Csoft: titlebar.c,v 1.23 2005/04/04 01:05:41 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <string.h>

#include "titlebar.h"

const struct widget_ops titlebar_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		box_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	titlebar_draw,
	box_scale
};

static void titlebar_mousebuttondown(int, union evarg *);
static void titlebar_mousebuttonup(int, union evarg *);

struct titlebar *
titlebar_new(void *parent, int flags)
{
	struct titlebar *tbar;

	tbar = Malloc(sizeof(struct titlebar), M_OBJECT);
	titlebar_init(tbar, flags);
	object_attach(parent, tbar);
	tbar->win = (struct window *)parent;
	return (tbar);
}

static void
maximize_window(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[1].p;
	struct window *win = tbar->win;

	if (win->flags & WINDOW_MAXIMIZED) {
		window_set_geometry(win, win->savx, win->savy, win->savw,
		    win->savh);
		win->flags &= ~(WINDOW_MAXIMIZED);
		if (!view->opengl) {
			SDL_FillRect(view->v, NULL, COLOR(BG_COLOR));
			SDL_UpdateRect(view->v, 0, 0, view->v->w, view->v->h);
		}
	} else {
		win->savx = WIDGET(win)->x;
		win->savy = WIDGET(win)->y;
		win->savw = WIDGET(win)->w;
		win->savh = WIDGET(win)->h;
		window_set_geometry(win, 0, 0, view->w, view->h);
		win->flags |= WINDOW_MAXIMIZED;
	}
}

static void
minimize_window(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[1].p;
	struct window *win = tbar->win;

	win->flags |= WINDOW_ICONIFIED;
	window_hide(win);
}

static void
close_window(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[1].p;

	event_post(NULL, tbar->win, "window-close", NULL);
}

void
titlebar_init(struct titlebar *tbar, int flags)
{
	box_init(&tbar->hb, BOX_HORIZ, BOX_WFILL);
	object_set_ops(tbar, &titlebar_ops);
	object_wire_gfx(tbar, "/engine/widget/pixmaps");

	box_set_padding(&tbar->hb, 5);
	box_set_spacing(&tbar->hb, 0);

	widget_set_type(tbar, "titlebar");
	WIDGET(tbar)->flags |= WIDGET_UNFOCUSED_BUTTONUP;

	tbar->flags = flags;
	tbar->pressed = 0;
	tbar->win = NULL;
	tbar->label = label_new(tbar, LABEL_STATIC, _("Untitled"));
	WIDGET(tbar->label)->flags |= WIDGET_WFILL;
	
	if ((flags & TITLEBAR_NO_MAXIMIZE) == 0) {
		tbar->maximize_btn = button_new(tbar, NULL);
		button_set_focusable(tbar->maximize_btn, 0);
		button_set_label(tbar->maximize_btn,
		    SPRITE(tbar,TITLEBAR_MAXIMIZE_ICON).su);
		button_set_padding(tbar->maximize_btn, 1);
		event_new(tbar->maximize_btn, "button-pushed", maximize_window,
		    "%p", tbar);
	} else {
		tbar->maximize_btn = NULL;
	}

	if ((flags & TITLEBAR_NO_MINIMIZE) == 0) {
		tbar->minimize_btn = button_new(tbar, NULL);
		button_set_focusable(tbar->minimize_btn, 0);
		button_set_label(tbar->minimize_btn,
		    SPRITE(tbar,TITLEBAR_MINIMIZE_ICON).su);
		button_set_padding(tbar->minimize_btn, 1);
		event_new(tbar->minimize_btn, "button-pushed", minimize_window,
		    "%p", tbar);
	} else {
		tbar->minimize_btn = NULL;
	}

	if ((flags & TITLEBAR_NO_CLOSE) == 0) {
		tbar->close_btn = button_new(tbar, NULL);
		button_set_focusable(tbar->close_btn, 0);
		button_set_label(tbar->close_btn,
		    SPRITE(tbar,TITLEBAR_CLOSE_ICON).su);
		button_set_padding(tbar->close_btn, 1);
		event_new(tbar->close_btn, "button-pushed", close_window,
		    "%p", tbar);
	} else {
		tbar->close_btn = NULL;
	}

	event_new(tbar, "window-mousebuttondown", titlebar_mousebuttondown,
	    NULL);
	event_new(tbar, "window-mousebuttonup", titlebar_mousebuttonup, NULL);
}

void
titlebar_draw(void *p)
{
	struct titlebar *tbar = p;

	primitives.box(tbar,
	    0,
	    0,
	    WIDGET(tbar)->w,
	    WIDGET(tbar)->h,
	    tbar->pressed ? -1 : 1,
	    WINDOW_FOCUSED(tbar->win) ? COLOR(TITLEBAR_FOCUSED_COLOR) :
	                                COLOR(TITLEBAR_UNFOCUSED_COLOR));
}

static void
titlebar_mousebuttondown(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[0].p;

	tbar->pressed = 1;

	pthread_mutex_lock(&view->lock);
	view->winop = VIEW_WINOP_MOVE;
	view->focus_win = tbar->win;
	view->wop_win = tbar->win;
	pthread_mutex_unlock(&view->lock);
}

static void
titlebar_mousebuttonup(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[0].p;
	
	tbar->pressed = 0;
	
	pthread_mutex_lock(&view->lock);
	view->winop = VIEW_WINOP_NONE;
	view->wop_win = NULL;
	pthread_mutex_unlock(&view->lock);
}

void
titlebar_set_caption(struct titlebar *tbar, const char *caption)
{
	label_set_surface(tbar->label, (caption == NULL) ? NULL :
	    text_render(NULL, -1, COLOR(TITLEBAR_CAPTION_COLOR), caption));
}

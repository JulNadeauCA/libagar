/*	$Csoft: titlebar.c,v 1.13 2004/03/18 21:27:48 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

enum {
	UNFOCUSED_COLOR,
	FOCUSED_COLOR
};

static void	titlebar_mousebuttondown(int, union evarg *);
static void	titlebar_mousebuttonup(int, union evarg *);
static void	titlebar_hide_win(int, union evarg *);
static void	titlebar_close_win(int, union evarg *);

struct titlebar *
titlebar_new(void *parent, int flags)
{
	struct titlebar *tbar;

#ifdef DEBUG
	if (strcmp(OBJECT(parent)->type, "window") != 0)
		fatal("titlebars only attach to windows");
#endif

	tbar = Malloc(sizeof(struct titlebar), M_OBJECT);
	titlebar_init(tbar, flags);
	object_attach(parent, tbar);
	tbar->win = (struct window *)parent;
	return (tbar);
}

void
titlebar_init(struct titlebar *tbar, int flags)
{
	box_init(&tbar->hb, BOX_HORIZ, BOX_WFILL);
	object_set_ops(tbar, &titlebar_ops);
	object_wire_gfx(tbar, "/engine/widget/window");

	box_set_padding(&tbar->hb, 5);
	box_set_spacing(&tbar->hb, 0);

	widget_set_type(tbar, "titlebar");
	widget_map_color(tbar, UNFOCUSED_COLOR, "unfocused", 35, 35, 35, 255);
	widget_map_color(tbar, FOCUSED_COLOR, "focused", 40, 60, 73, 255);
	WIDGET(tbar)->flags |= WIDGET_UNFOCUSED_BUTTONUP;

	tbar->flags = flags;
	tbar->pressed = 0;
	tbar->win = NULL;
	
	tbar->label = label_new(tbar, LABEL_STATIC, _("Untitled"));
	WIDGET(tbar->label)->flags |= WIDGET_WFILL;

	tbar->hide_bu = button_new(tbar, NULL);
	button_set_focusable(tbar->hide_bu, 0);
	event_new(tbar->hide_bu, "button-pushed", titlebar_hide_win, "%p",
	    tbar);

	tbar->close_bu = button_new(tbar, NULL);
	button_set_focusable(tbar->close_bu, 0);
	event_new(tbar->close_bu, "button-pushed", titlebar_close_win, "%p",
	    tbar);

	button_set_label(tbar->close_bu, SPRITE(tbar, TITLEBAR_CLOSE_ICON));
	button_set_label(tbar->hide_bu, SPRITE(tbar, TITLEBAR_HIDE_ICON));
	button_set_padding(tbar->close_bu, 0);
	button_set_padding(tbar->hide_bu, 0);

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
	    WINDOW_FOCUSED(tbar->win) ? FOCUSED_COLOR : UNFOCUSED_COLOR);
}

static void
titlebar_mousebuttondown(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[0].p;

	tbar->pressed = 1;

	pthread_mutex_lock(&view->lock);
	view->winop = VIEW_WINOP_MOVE;
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

static void
titlebar_hide_win(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[1].p;

	/* TODO */
//	tbar->win->visible = 0;
}

static void
titlebar_close_win(int argc, union evarg *argv)
{
	struct titlebar *tbar = argv[1].p;

	event_post(NULL, tbar->win, "window-close", NULL);
}


/*	$Csoft: combo.c,v 1.19 2004/05/15 02:12:00 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include "combo.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static struct widget_ops combo_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		combo_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	combo_scale
};

struct combo *
combo_new(void *parent, int flags, const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct combo *com;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	com = Malloc(sizeof(struct combo), M_OBJECT);
	combo_init(com, label, flags);
	object_attach(parent, com);
	return (com);
}

static void
combo_collapse(struct combo *com)
{
	struct widget_binding *stateb;
	int *state;

	if (com->win == NULL)
		return;

	com->saved_h = WIDGET(com->win)->h;
	window_hide(com->win);
	object_detach(com->list);
	view_detach(com->win);
	com->win = NULL;
	
	stateb = widget_get_binding(com->button, "state", &state);
	*state = 0;
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);
}

static void
combo_expand(int argc, union evarg *argv)
{
	struct combo *com = argv[1].p;
	int expand = argv[2].i;

	if (expand) {						/* Expand */
		struct widget *win;

		com->win = window_new(NULL);
		win = WIDGET(com->win);
		object_detach(com->win->tbar);
		object_destroy(com->win->tbar);
		Free(com->win->tbar, M_OBJECT);

		object_attach(com->win, com->list);
	
		win->w = WIDGET(com)->w - WIDGET(com->button)->w;
		win->h = com->saved_h > 0 ? com->saved_h : WIDGET(com)->h*5;
		win->x = WIDGET(com)->cx;
		win->y = WIDGET(com)->cy;
		if (win->x+win->w > view->w)
			win->w = view->w - win->x;
		if (win->y+win->h > view->h)
			win->h = view->h - win->y;

		WIDGET_SCALE(win, win->w, win->h);
		widget_update_coords(com->win, win->x, win->y);
		window_show(com->win);
	} else {
		combo_collapse(com);
	}
}

void
combo_select(struct combo *com, struct tlist_item *it)
{
	pthread_mutex_lock(&com->list->lock);
	textbox_printf(com->tbox, "%s", it->text);
	tlist_select(com->list, it);
	pthread_mutex_unlock(&com->list->lock);
}

static void
combo_selected(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct combo *com = argv[1].p;
	struct tlist_item *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = tlist_item_selected(tl)) != NULL) {
		textbox_printf(com->tbox, "%s", ti->text);
		event_post(NULL, com, "combo-selected", "%p", ti);
	}
	pthread_mutex_unlock(&tl->lock);
	combo_collapse(com);
}

static void
combo_return(int argc, union evarg *argv)
{
	char s[TEXTBOX_STRING_MAX];
	struct textbox *tbox = argv[0].p;
	struct combo *com = argv[1].p;

	textbox_copy_string(tbox, s, sizeof(s));
	event_post(NULL, com, "combo-selected", "%s", s);
}

#if 0
static void
combo_mousebuttonup(int argc, union evarg *argv)
{
	struct combo *com = argv[0].p;
/*	int button = argv[1].i; */
	int x = WIDGET(com)->cx + argv[2].i;
	int y = WIDGET(com)->cy + argv[3].i;

	if (com->win != NULL && !widget_area(com->win, x, y)) {
		combo_collapse(com);
	}
}
#endif

void
combo_init(struct combo *com, const char *label, int flags)
{
	widget_init(com, "combo", &combo_ops, WIDGET_FOCUSABLE|WIDGET_WFILL|
	    WIDGET_UNFOCUSED_BUTTONUP);
	com->win = NULL;
	com->flags = flags;
	com->saved_h = 0;

	com->tbox = textbox_new(com, label);
	com->button = button_new(com, " ... ");
	button_set_sticky(com->button, 1);
	button_set_padding(com->button, 1);

	com->list = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(com->list, 0);
	
	if (flags & COMBO_TREE)	
		com->list->flags |= TLIST_TREE;
	if (flags & COMBO_POLL)
		com->list->flags |= TLIST_POLL;

	event_new(com->button, "button-pushed", combo_expand, "%p", com);
	event_new(com->list, "tlist-changed", combo_selected, "%p", com);
	event_new(com->tbox, "textbox-return", combo_return, "%p", com);
#if 0
	event_new(com, "window-mousebuttonup", combo_mousebuttonup, NULL);
#endif
}

void
combo_destroy(void *p)
{
	struct combo *com = p;

	if (com->win != NULL) {
		window_hide(com->win);
		object_detach(com->list);
		view_detach(com->win);
	}
	object_destroy(com->list);
	Free(com->list, M_OBJECT);
	widget_destroy(com);
}

void
combo_scale(void *p, int w, int h)
{
	struct combo *com = p;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(com->tbox, -1, -1);
		WIDGET(com)->w = WIDGET(com->tbox)->w;
		WIDGET(com)->h = WIDGET(com->tbox)->h;

		WIDGET_SCALE(com->button, -1, -1);
		WIDGET(com)->w += WIDGET(com->button)->w;
		if (WIDGET(com->button)->h > WIDGET(com)->h) {
			WIDGET(com)->h = WIDGET(com->button)->h;
		}
		return;
	}
	
	widget_scale(com->button, -1, -1);
	widget_scale(com->tbox, w - WIDGET(com->button)->w - 1, h);
	widget_scale(com->button, WIDGET(com->button)->w, h);

	WIDGET(com->tbox)->x = 0;
	WIDGET(com->tbox)->y = 0;
	WIDGET(com->button)->x = w - WIDGET(com->button)->w - 1;
	WIDGET(com->button)->y = 0;
}


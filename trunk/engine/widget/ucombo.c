/*	$Csoft: ucombo.c,v 1.9 2004/05/17 07:08:48 vedge Exp $	*/

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

#include "ucombo.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

static struct widget_ops ucombo_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		ucombo_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	ucombo_scale
};

struct ucombo *
ucombo_new(void *parent)
{
	struct ucombo *com;

	com = Malloc(sizeof(struct ucombo), M_OBJECT);
	ucombo_init(com);
	object_attach(parent, com);
	return (com);
}

static void
ucombo_collapse(struct ucombo *com)
{
	struct widget_binding *stateb;
	int *state;

	if (com->panel == NULL)
		return;

	com->saved_w = WIDGET(com->panel)->w;
	com->saved_h = WIDGET(com->panel)->h;
	window_hide(com->panel);
	object_detach(com->list);
	view_detach(com->panel);
	com->panel = NULL;
	
	stateb = widget_get_binding(com->button, "state", &state);
	*state = 0;
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);
}

static void
ucombo_expand(int argc, union evarg *argv)
{
	struct ucombo *com = argv[1].p;
	int expand = argv[2].i;
	struct widget *pan;

	if (expand) {
		com->panel = window_new(WINDOW_NO_TITLEBAR|
		                        WINDOW_NO_DECORATIONS, NULL);
		pan = WIDGET(com->panel);

		object_attach(com->panel, com->list);
	
		pan->w = com->saved_w > 0 ? com->saved_w : WIDGET(com)->w*4;
		pan->h = com->saved_h > 0 ? com->saved_h : WIDGET(com)->h*5;
		pan->x = WIDGET(com)->cx;
		pan->y = WIDGET(com)->cy;
		if (pan->x+pan->w > view->w)
			pan->w = view->w - pan->x;
		if (pan->y+pan->h > view->h)
			pan->h = view->h - pan->y;
		
		tlist_prescale(com->list, "XXXXXXXXXXXXXXXXXX", 6);
		WIDGET_SCALE(com->list, -1, -1);
		WIDGET_SCALE(pan, -1, -1);
		WIDGET_SCALE(pan, pan->w, pan->h);
		widget_update_coords(pan, pan->x, pan->y);

		window_show(com->panel);
	} else {
		ucombo_collapse(com);
	}
}

/* Effect a user item selection. */
static void
ucombo_selected(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct ucombo *com = argv[1].p;
	struct tlist_item *it;

	pthread_mutex_lock(&tl->lock);
	if ((it = tlist_item_selected(tl)) != NULL) {
		it->selected++;
		event_post(NULL, com, "ucombo-selected", "%p", it);
	}
	pthread_mutex_unlock(&tl->lock);

	ucombo_collapse(com);
}

void
ucombo_init(struct ucombo *com)
{
	widget_init(com, "ucombo", &ucombo_ops, WIDGET_FOCUSABLE|WIDGET_WFILL|
	    WIDGET_UNFOCUSED_BUTTONUP);
	com->panel = NULL;
	com->saved_h = 0;

	com->button = button_new(com, "...");
	button_set_sticky(com->button, 1);
	button_set_padding(com->button, 1);
	event_new(com->button, "button-pushed", ucombo_expand, "%p", com);
	
	com->list = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(com->list, 0);
	event_new(com->list, "tlist-changed", ucombo_selected, "%p", com);
}

void
ucombo_destroy(void *p)
{
	struct ucombo *com = p;

	if (com->panel != NULL) {
		window_hide(com->panel);
		object_detach(com->list);
		view_detach(com->panel);
	}
	object_destroy(com->list);
	Free(com->list, M_OBJECT);
	widget_destroy(com);
}

void
ucombo_scale(void *p, int w, int h)
{
	struct ucombo *com = p;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(com->button, -1, -1);
		WIDGET(com)->w = WIDGET(com->button)->w;
		WIDGET(com)->h = WIDGET(com->button)->h;
		return;
	}
	
	widget_scale(com->button, w, h);
	WIDGET(com->button)->x = 0;
	WIDGET(com->button)->y = 0;
}


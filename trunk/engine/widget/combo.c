/*	$Csoft: combo.c,v 1.44 2003/06/08 23:53:17 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static struct widget_ops combo_ops = {
	{
		NULL,			/* init */
		combo_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	combo_scale
};

struct combo *
combo_new(void *parent, const char *fmt, ...)
{
	char label[COMBO_LABEL_MAX];
	struct combo *com;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	com = Malloc(sizeof(struct combo));
	combo_init(com, label);
	object_attach(parent, com);
	return (com);
}

static void
combo_collapse(struct combo *com)
{
	struct widget_binding *stateb;
	int *state;

	window_hide(com->win);
	object_detach(com->win, com->list);
	view_detach(com->win);
	com->win = NULL;
	
	stateb = widget_binding_get_locked(com->button, "state", &state);
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
		com->win = window_new(NULL);
		object_detach(com->win, com->win->tbar);
		object_destroy(com->win->tbar);
		free(com->win->tbar);

		object_attach(com->win, com->list);
	
		WIDGET(com->win)->w = WIDGET(com)->w - WIDGET(com->button)->w;
		WIDGET(com->win)->h = WIDGET(com)->h*5;
		WIDGET(com->win)->x = WIDGET(com)->cx;
		WIDGET(com->win)->y = WIDGET(com)->cy;
		WIDGET_SCALE(com->win,
		    WIDGET(com->win)->w,
		    WIDGET(com->win)->h);
		window_remap_widgets(com->win,
		    WIDGET(com->win)->x,
		    WIDGET(com->win)->y);
		window_show(com->win);
	} else {
		combo_collapse(com);
	}
}

static void
combo_select(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct combo *com = argv[1].p;
	struct tlist_item *ti;

	if ((ti = tlist_item_selected(tl)) != NULL) {
		struct widget_binding *stringb;
		char *s;

		stringb = widget_binding_get_locked(com->tbox, "string", &s);
		strlcpy(s, ti->text, stringb->size);
		widget_binding_unlock(stringb);
	}

	combo_collapse(com);
}

void
combo_init(struct combo *com, const char *label)
{
	widget_init(com, "combo", &combo_ops, WIDGET_FOCUSABLE|WIDGET_WFILL);

	com->tbox = textbox_new(com, label);
	com->button = button_new(com, " ... ");
	button_set_sticky(com->button, 1);
	button_set_padding(com->button, 1);
	com->win = NULL;
	com->list = Malloc(sizeof(struct tlist));
	tlist_init(com->list, 0);

	event_new(com->button, "button-pushed", combo_expand, "%p", com);
	event_new(com->list, "tlist-changed", combo_select, "%p", com);
}

void
combo_destroy(void *p)
{
	struct combo *com = p;

	if (com->win != NULL) {
		object_destroy(com->win);
	}
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
		
		if (WIDGET(com->button)->h > WIDGET(com)->h)
			WIDGET(com)->h = WIDGET(com->button)->h;
		return;
	}
	
	widget_scale(com->button, -1, -1);
	widget_scale(com->tbox, w - WIDGET(com->button)->w, h);
	widget_scale(com->button, WIDGET(com->button)->w, h);

	WIDGET(com->tbox)->x = 0;
	WIDGET(com->tbox)->y = 0;
	WIDGET(com->button)->x = w - WIDGET(com->button)->w;
	WIDGET(com->button)->y = 0;
}


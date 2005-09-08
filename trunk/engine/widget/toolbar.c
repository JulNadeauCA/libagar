/*	$Csoft: toolbar.c,v 1.8 2005/06/10 02:02:01 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#include "toolbar.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/separator.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static struct widget_ops toolbar_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		box_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,
	toolbar_scale
};

struct toolbar *
toolbar_new(void *parent, enum toolbar_type type, int nrows, int flags)
{
	struct toolbar *tbar;

	tbar = Malloc(sizeof(struct toolbar), M_OBJECT);
	toolbar_init(tbar, type, nrows, flags);
	object_attach(parent, tbar);
	return (tbar);
}

void
toolbar_init(struct toolbar *tbar, enum toolbar_type type, int nrows, int flags)
{
	int i;
	
	switch (type) {
	case TOOLBAR_HORIZ:
		box_init(&tbar->box, BOX_VERT, BOX_WFILL);
		break;
	case TOOLBAR_VERT:
		box_init(&tbar->box, BOX_HORIZ, BOX_HFILL);
		break;
	}
	box_set_padding(&tbar->box, 1);
	box_set_spacing(&tbar->box, 1);
	object_set_ops(tbar, &toolbar_ops);

	tbar->type = type;
	tbar->nrows = 0;
	for (i = 0; i < nrows && i < TOOLBAR_MAX_ROWS; i++) {
		int bflags = 0;

		if (flags & TOOLBAR_HOMOGENOUS) {
			bflags = BOX_HOMOGENOUS;
		}
		switch (type) {
		case TOOLBAR_HORIZ:
			tbar->rows[i] = box_new(&tbar->box, BOX_HORIZ,
			    BOX_WFILL|bflags);
			break;
		case TOOLBAR_VERT:
			tbar->rows[i] = box_new(&tbar->box, BOX_VERT, 
			    BOX_HFILL|bflags);
			break;
		}
		box_set_padding(tbar->rows[i], 1);
		box_set_spacing(tbar->rows[i], 1);
		tbar->nrows++;
	}
}

struct button *
toolbar_add_button(struct toolbar *tbar, int row, SDL_Surface *icon,
    int sticky, int def, void (*handler)(int, union evarg *),
    const char *fmt, ...)
{
	struct button *bu;
	struct event *ev;
	va_list ap;

#ifdef DEBUG
	if (row < 0 || row > tbar->nrows)
		fatal("no such row %d", row);
#endif
	bu = button_new(tbar->rows[row], NULL);
	button_set_label(bu, icon);
	button_set_focusable(bu, 0);
	button_set_sticky(bu, sticky);
	widget_set_bool(bu, "state", def);
	
	ev = event_new(bu, "button-pushed", handler, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, ev);
		}
		va_end(ap);
	}
	return (bu);
}

void
toolbar_add_separator(struct toolbar *tbar, int nrow)
{
	separator_new(tbar->rows[nrow], tbar->type == TOOLBAR_HORIZ ?
	    SEPARATOR_HORIZ : SEPARATOR_VERT);
}

void
toolbar_select_unique(struct toolbar *tbar, struct button *ubu)
{
	struct widget_binding *stateb;
	struct button *bu;
	int i;

	for (i = 0; i < tbar->nrows; i++) {
		OBJECT_FOREACH_CHILD(bu, tbar->rows[i], button) {
			int *state;

			if (bu == ubu) {
				continue;
			}
			stateb = widget_get_binding(bu, "state", &state);
			*state = 0;
			widget_binding_modified(stateb);
			widget_binding_unlock(stateb);
		}
	}
}

void
toolbar_scale(void *p, int w, int h)
{
	struct box *bo = p;
	struct widget *wid;
	int x = bo->padding;
	int y = bo->padding;

	if (w == -1 && h == -1) {
		int maxw = 0, maxh = 0;
		int dw, dh;

		WIDGET(bo)->w = bo->padding*2 - bo->spacing;
		WIDGET(bo)->h = bo->padding*2;

		/* Reserve enough space to hold widgets and spacing/padding. */
		OBJECT_FOREACH_CHILD(wid, bo, widget) {
			WIDGET_OPS(wid)->scale(wid, -1, -1);
			if (wid->w > maxw) maxw = wid->w;
			if (wid->h > maxh) maxh = wid->h;

			if ((dh = maxh + bo->padding*2) > WIDGET(bo)->h) {
				WIDGET(bo)->h = dh;
			}
			WIDGET(bo)->w += wid->w + bo->spacing;
		}
		WIDGET(bo)->w -= bo->spacing;
		return;
	}

	OBJECT_FOREACH_CHILD(wid, bo, widget) {
		wid->x = x;
		wid->y = y;

		x += wid->w + bo->spacing;
		WIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
	}
}

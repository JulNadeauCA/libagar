/*	$Csoft: toolbar.c,v 1.3 2004/03/18 21:27:48 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
	box_scale
};

struct toolbar *
toolbar_new(void *parent, enum toolbar_type type, int nrows)
{
	struct toolbar *tbar;

	tbar = Malloc(sizeof(struct toolbar), M_OBJECT);
	toolbar_init(tbar, type, nrows);
	object_attach(parent, tbar);
	return (tbar);
}

void
toolbar_init(struct toolbar *tbar, enum toolbar_type type, int nrows)
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
	box_set_padding(&tbar->box, 2);
	box_set_spacing(&tbar->box, 1);
	object_set_ops(tbar, &toolbar_ops);

	tbar->type = type;
	tbar->nrows = 0;
	for (i = 0; i < nrows && i < TOOLBAR_MAX_ROWS; i++) {
		switch (type) {
		case TOOLBAR_HORIZ:
			tbar->rows[i] = box_new(&tbar->box, BOX_HORIZ,
			    BOX_WFILL|BOX_HOMOGENOUS);
			break;
		case TOOLBAR_VERT:
			tbar->rows[i] = box_new(&tbar->box, BOX_VERT, 
			    BOX_HFILL|BOX_HOMOGENOUS);
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

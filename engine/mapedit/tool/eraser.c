/*	$Csoft: eraser.c,v 1.25 2003/03/08 00:55:41 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/version.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include <libfobj/fobj.h>

#include "tool.h"
#include "eraser.h"

static const struct version eraser_ver = {
	"agar eraser tool",
	1, 0
};

static const struct tool_ops eraser_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	eraser_window,
	NULL,			/* cursor */
	eraser_effect,
	NULL			/* mouse */
};

void
eraser_init(void *p)
{
	struct eraser *eraser = p;

	tool_init(&eraser->tool, "eraser", &eraser_ops);

	eraser->mode = ERASER_ALL;
}

struct window *
eraser_window(void *p)
{
	struct eraser *er = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-eraser", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    91, 71,
	    91, 71);
	window_set_caption(win, "Eraser");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		static const char *mode_items[] = {
			"All",
			"Highest",
			NULL
		};
		struct radio *rad;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &er->mode);
		win->focus = WIDGET(rad);
	}
	return (win);
}

void
eraser_effect(void *p, struct mapview *mv, struct node *node)
{
	struct eraser *er = p;
	struct noderef *nref, *nnref;
	
	if (TAILQ_EMPTY(&node->nrefs)) {
		return;
	}
	
	for (nref = TAILQ_FIRST(&node->nrefs);
	     nref != TAILQ_END(&node->nrefs);
	     nref = nnref) {
		nnref = TAILQ_NEXT(nref, nrefs);
		if (nref->layer == mv->cur_layer) {
			TAILQ_REMOVE(&node->nrefs, nref, nrefs);
			noderef_destroy(nref);
			free(nref);

			if (er->mode == ERASER_HIGHEST) {
				break;
			}
		}
	}
	node->flags = 0;
}


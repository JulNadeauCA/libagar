/*	$Csoft: eraser.c,v 1.39 2003/08/29 04:55:43 vedge Exp $	*/

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

#include "eraser.h"

#include <engine/widget/radio.h>

static void	 eraser_effect(void *, struct mapview *, struct map *,
		               struct node *);

const struct tool_ops eraser_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* cursor */
	eraser_effect,
	NULL			/* mouse */
};

void
eraser_init(void *p)
{
	struct eraser *er = p;
	static const char *mode_items[] = {
		N_("All"),
		N_("Highest"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	tool_init(&er->tool, "eraser", &eraser_ops, MAPEDIT_TOOL_ERASER);
	er->mode = ERASER_ALL;

	win = TOOL(er)->win = window_new("mapedit-tool-eraser");
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	window_set_caption(win, _("Eraser"));
	event_new(win, "window-close", tool_window_close, "%p", er);

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, NULL, &er->mode);
}

static void
eraser_effect(void *p, struct mapview *mv, struct map *m, struct node *dn)
{
	struct eraser *er = p;
	struct noderef *nref, *nnref;
	
	if (TAILQ_EMPTY(&dn->nrefs))
		return;
	
	for (nref = TAILQ_FIRST(&dn->nrefs);
	     nref != TAILQ_END(&dn->nrefs);
	     nref = nnref) {
		nnref = TAILQ_NEXT(nref, nrefs);
		if (nref->layer == m->cur_layer) {
			TAILQ_REMOVE(&dn->nrefs, nref, nrefs);
			noderef_destroy(m, nref);

			if (er->mode == ERASER_HIGHEST)
				break;
		}
	}
}


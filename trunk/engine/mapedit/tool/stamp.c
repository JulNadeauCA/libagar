/*	$Csoft: stamp.c,v 1.24 2003/02/02 23:05:59 vedge Exp $	*/

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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "stamp.h"

static const struct tool_ops stamp_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	stamp_window,
	stamp_cursor,
	stamp_effect
};

void
stamp_init(void *p)
{
	struct stamp *stamp = p;

	tool_init(&stamp->tool, "stamp", &stamp_ops);

	stamp->mode = STAMP_REPLACE;
}

struct window *
stamp_window(void *p)
{
	struct stamp *st = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-stamp", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    157, 76,
	    157, 76);
	window_set_caption(win, "Stamp");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		struct radio *rad;
		static const char *mode_items[] = {
			"Replace",
			"Insert highest",
			NULL
		};

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &st->mode);
		win->focus = WIDGET(rad);
	}
	return (win);
}

void
stamp_effect(void *p, struct mapview *mv, struct node *dstnode)
{
	struct stamp *st = p;
	struct map *m = mv->map;
	struct node *srcnode = mapedit.src_node;
	struct noderef *nref, *nnref;

	if (srcnode == NULL) {
		text_msg("Error", "No source node");
		return;
	}
	if (srcnode == dstnode) {
		text_msg("Error", "Source node == destination node");
		return;
	}

	if (st->mode == STAMP_REPLACE) {
		for (nref = TAILQ_FIRST(&dstnode->nrefs);
		     nref != TAILQ_END(&dstnode->nrefs);
		     nref = nnref) {
			nnref = TAILQ_NEXT(nref, nrefs);
			free(nref);
		}
		TAILQ_INIT(&dstnode->nrefs);
	}

	TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
		node_copy_ref(nref, dstnode);
	}
	dstnode->flags = srcnode->flags & ~NODE_ORIGIN;
}

int
stamp_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	struct stamp *st = p;
	SDL_Surface *srcsu;
	struct noderef *nref;

	if (mapedit.src_node == NULL) {
		return (-1);
	}

	TAILQ_FOREACH(nref, &mapedit.src_node->nrefs, nrefs) {
		noderef_draw(mv->map, nref, rd->x, rd->y);
	}
	return (0);
}


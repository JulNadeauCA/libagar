/*	$Csoft: fill.c,v 1.15 2003/03/26 10:04:18 vedge Exp $	*/

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

#include "fill.h"

#include <engine/widget/radio.h>
#include <engine/widget/text.h>

static const struct tool_ops fill_ops = {
	{
		tool_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	fill_window,
	NULL,			/* cursor */
	fill_effect,
	NULL			/* mouse */
};

void
fill_init(void *p)
{
	struct fill *fill = p;

	tool_init(&fill->tool, "fill", &fill_ops);
	fill->mode = FILL_FILL_MAP;
}

struct window *
fill_window(void *p)
{
	struct fill *fi = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-fill", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    126, 128,
	    126, 128);
	window_set_caption(win, "Fill");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	{
		static const char *mode_items[] = {
			"Fill",
			"Clear",
			NULL
		};
		struct radio *rad;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &fi->mode);
		win->focus = WIDGET(rad);
	}
	return (win);
}

void
fill_effect(void *p, struct mapview *mv, struct node *dstnode)
{
	struct fill *fi = p;
	struct map *m = mv->map;
	struct node *srcnode = mapedit.src_node;
	int dx = 0, dy = 0;
	int w = m->mapw, h = m->maph;
	int x, y;

	if (srcnode == NULL && fi->mode == FILL_FILL_MAP) {
		text_msg("Error", "No source node");
		return;
	}

	mapview_get_selection(mv, &dx, &dy, &w, &h);

	for (y = dy; y < dy+h; y++) {
		for (x = dx; x < dx+w; x++) {
			struct node *dstnode = &m->map[y][x];
			struct noderef *nref;

			if (fi->mode == FILL_FILL_MAP &&
			    srcnode == dstnode) {
				/* Avoid circular reference */
				continue;
			}

			/* Remove all refs on this layer. */
			node_clear_layer(dstnode, mv->map->cur_layer);

			switch (fi->mode) {
			case FILL_FILL_MAP:
				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
					struct noderef *nnref;

					nnref = node_copy_ref(nref, dstnode);
					nnref->layer = mv->map->cur_layer;
				}
				break;
			case FILL_CLEAR_MAP:
				break;
			}

			dstnode->flags = srcnode->flags;
		}
	}
}


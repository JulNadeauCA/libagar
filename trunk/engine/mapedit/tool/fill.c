/*	$Csoft: fill.c,v 1.25 2003/07/28 15:29:58 vedge Exp $	*/

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

#include <engine/widget/vbox.h>
#include <engine/widget/radio.h>

const struct tool_ops fill_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
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

	tool_init(&fill->tool, "fill", &fill_ops, MAPEDIT_TOOL_FILL);
	TOOL(fill)->cursor = SPRITE(fill, TOOL_FILL_CURSOR);
	fill->mode = FILL_FILL_MAP;
}

struct window *
fill_window(void *p)
{
	static const char *mode_items[] = {
		N_("Fill"),
		N_("Clear"),
		NULL
	};
	struct fill *fi = p;
	struct window *win;
	struct vbox *vb;
	struct radio *rad;

	win = window_new("mapedit-tool-fill");
	window_set_caption(win, _("Fill"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	vb = vbox_new(win, 0);
	rad = radio_new(vb, mode_items);
	widget_bind(rad, "value", WIDGET_INT, NULL, &fi->mode);
	return (win);
}

void
fill_effect(void *p, struct mapview *mv, struct map *m, struct node *node)
{
	struct fill *fi = p;
	struct node *srcnode = mapedit.src_node;
	int sx, sy, dx, dy;
	int w, h;

	if (srcnode == NULL && fi->mode == FILL_FILL_MAP) {
		text_msg(MSG_ERROR, _("No source node."));
		return;
	}

	dx = 0;
	dy = 0;
	w = m->mapw;
	h = m->maph;
	mapview_get_selection(mv, &dx, &dy, &w, &h);

	for (sy = dy; sy < dy+h; sy++) {
		for (sx = dx; sx < dx+w; sx++) {
			struct node *dn = &m->map[sy][sx];

			if (fi->mode == FILL_FILL_MAP &&
			    srcnode == dn) {
				/* Avoid circular reference */
				continue;
			}

			node_clear(m, dn, m->cur_layer);

			if (fi->mode == FILL_FILL_MAP) {
				struct noderef *r;

				TAILQ_FOREACH(r, &srcnode->nrefs, nrefs)
					node_copy_ref(r, m, dn, m->cur_layer);
			}
		}
	}
}


/*	$Csoft: fill.c,v 1.27 2003/08/29 04:56:18 vedge Exp $	*/

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

static void	 fill_effect(void *, struct mapview *, struct map *,
		             struct node *);

const struct tool_ops fill_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* cursor */
	fill_effect,
	NULL			/* mouse */
};

void
fill_init(void *p)
{
	struct fill *fi = p;
	static const char *mode_items[] = {
		N_("Pattern fill"),
		N_("Clear"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	tool_init(&fi->tool, "fill", &fill_ops, MAPEDIT_TOOL_FILL);
	TOOL(fi)->cursor = SPRITE(fi, TOOL_FILL_CURSOR);
	fi->mode = FILL_PATTERN;

	win = TOOL(fi)->win = window_new("mapedit-tool-fill");
	window_set_caption(win, _("Fill"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	event_new(win, "window-close", tool_window_close, "%p", fi);

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, NULL, &fi->mode);
}

static void
fill_effect(void *p, struct mapview *mv, struct map *m, struct node *node)
{
	struct fill *fi = p;
	struct map *copybuf = &mapedit.copybuf;
	int sx = 0, sy = 0, dx = 0, dy = 0;
	int dw = m->mapw, dh = m->maph;
	int x, y;

	if (copybuf->mapw == 0 || copybuf->maph == 0)
		return;

	mapview_get_selection(mv, &dx, &dy, &dw, &dh);

	for (y = dy; y < dy+dh; y++) {
		for (x = dx; x < dx+dw; x++) {
			struct node *sn = &copybuf->map[sy][sx];
			struct node *dn = &m->map[y][x];

			node_clear(m, dn, m->cur_layer);

			if (fi->mode == FILL_PATTERN) {
				struct noderef *r;

				TAILQ_FOREACH(r, &sn->nrefs, nrefs)
					node_copy_ref(r, m, dn, m->cur_layer);
			}

			if (++sx >= copybuf->mapw)
				sx = 0;
		}
		if (++sy >= copybuf->maph)
			sy = 0;
	}
}


/*	$Csoft: fill.c,v 1.29 2003/10/13 23:49:00 vedge Exp $	*/

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
#include <engine/mapedit/mapedit.h>

#include <engine/widget/radio.h>

static void fill_init(void);
static void fill_effect(struct mapview *, struct map *, struct node *);

struct tool fill_tool = {
	N_("Clear/Fill"),
	N_("Clear or fill the whole layer/selection."),
	MAPEDIT_TOOL_FILL,
	MAPEDIT_FILL_CURSOR,
	fill_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	fill_effect,
	NULL,			/* cursor */
	NULL			/* mouse */
};

static enum fill_mode {
	FILL_PATTERN,			/* Pattern fill */
	FILL_CLEAR			/* Erase whole layer */
} mode = FILL_PATTERN;

static void
fill_init(void)
{
	static const char *mode_items[] = {
		N_("Pattern fill"),
		N_("Clear"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	win = tool_window_new(&fill_tool, "mapedit-tool-fill");

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &mode);
}

static void
fill_effect(struct mapview *mv, struct map *m, struct node *node)
{
	struct map *copybuf = &mapedit.copybuf;
	int sx = 0, sy = 0, dx = 0, dy = 0;
	int dw = m->mapw, dh = m->maph;
	int x, y;

	if (mode == FILL_PATTERN &&
	    (copybuf->mapw == 0 || copybuf->maph == 0)) {
	    	text_msg(MSG_ERROR, _("The copy/paste buffer is empty."));
		return;
	}

	mapview_get_selection(mv, &dx, &dy, &dw, &dh);

	for (y = dy; y < dy+dh; y++) {
		for (x = dx; x < dx+dw; x++) {
			struct node *dn = &m->map[y][x];
			struct noderef *r;

			node_clear(m, dn, m->cur_layer);

			if (mode == FILL_PATTERN) {
				struct node *sn = &copybuf->map[sy][sx];

				TAILQ_FOREACH(r, &sn->nrefs, nrefs)
					node_copy_ref(r, m, dn, m->cur_layer);
				if (++sx >= copybuf->mapw)
					sx = 0;
			}
		}
		if (mode == FILL_PATTERN) {
			if (++sy >= copybuf->maph)
				sy = 0;
		}
	}
}


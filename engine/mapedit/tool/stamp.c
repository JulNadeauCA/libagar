/*	$Csoft: stamp.c,v 1.46 2003/06/17 23:30:46 vedge Exp $	*/

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

#include "stamp.h"

#include <engine/widget/vbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

const struct tool_ops stamp_ops = {
	{
		NULL,		/* init */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	stamp_window,
	stamp_cursor,
	stamp_effect,
	NULL			/* mouse */
};

void
stamp_init(void *p)
{
	struct stamp *stamp = p;

	tool_init(&stamp->tool, "stamp", &stamp_ops);
	TOOL(stamp)->icon = SPRITE(&mapedit, MAPEDIT_TOOL_STAMP);
	stamp->mode = STAMP_REPLACE;
}

struct window *
stamp_window(void *p)
{
	static const char *mode_items[] = {
		"Replace",
		"Insert",
		NULL
	};
	struct stamp *st = p;
	struct window *win;
	struct radio *rad;

	win = window_new("mapedit-tool-stamp");
	window_set_caption(win, _("Stamp"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, NULL, &st->mode);
	widget_focus(rad);
	return (win);
}

void
stamp_effect(void *p, struct mapview *mv, struct map *m, struct node *node)
{
	struct node *sn = mapedit.src_node;
	struct noderef *r;

	if (sn == NULL) {
		text_msg(MSG_ERROR, _("No source node is selected"));
		return;
	}
	if (sn == node)					/* Circular reference */
		return;

	node_clear(m, node, m->cur_layer);

	TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
		node_copy_ref(r, m, node, m->cur_layer);
	}
}

int
stamp_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	struct noderef *r;

	if (mapedit.src_node == NULL)
		return (-1);

	TAILQ_FOREACH(r, &mapedit.src_node->nrefs, nrefs) {
		noderef_draw(mv->map, r,
		    WIDGET(mv)->cx + rd->x,
		    WIDGET(mv)->cy + rd->y);
	}
	return (0);
}


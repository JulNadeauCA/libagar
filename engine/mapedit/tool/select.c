/*	$Csoft: select.c,v 1.17 2003/08/26 07:55:02 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include "select.h"

const struct tool_ops select_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL			/* mouse */
};

/* Copy the selection to the copy buffer. */
static void
select_copy(void *p, struct mapview *mv)
{
	struct map *copybuf = &mapedit.copybuf;
	struct map *m = mv->map;
	int sx, sy, dx, dy;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no selection to copy."));
		return;
	}

	dprintf("copy [%d,%d]+[%dx%d]\n", mv->esel.x, mv->esel.y, mv->esel.w,
	    mv->esel.h);

	if (copybuf->map != NULL)
		map_free_nodes(copybuf);
	if (map_alloc_nodes(copybuf, mv->esel.w, mv->esel.h) == -1) {
		text_msg(MSG_ERROR, "%s", error_get());
		return;
	}

	for (sy = mv->esel.y, dy = 0;
	     sy < mv->esel.y + mv->esel.h;
	     sy++, dy++) {
		for (sx = mv->esel.x, dx = 0;
		     sx < mv->esel.x + mv->esel.w;
		     sx++, dx++) {
			node_copy(m, &m->map[sy][sx], m->cur_layer, copybuf,
			    &copybuf->map[dy][dx], 0);
		}
	}
}

static void
select_paste(void *p, struct mapview *mv)
{
	struct map *copybuf = &mapedit.copybuf;
	struct map *m = mv->map;
	int sx, sy, dx, dy;
	
	if (copybuf->map == NULL) {
		text_msg(MSG_ERROR, _("The copy buffer is empty!"));
		return;
	}

	if (mv->esel.set) {
		dx = mv->esel.x;
		dy = mv->esel.y;
	} else {
		if (mv->cx != -1 && mv->cy != -1) {
			dx = mv->cx;
			dy = mv->cy;
		} else {
			dx = 0;
			dy = 0;
		}
	}

	dprintf("[%dx%d] at [%d,%d]\n", copybuf->mapw, copybuf->maph, dx, dy);

	for (sy = 0, dy = mv->esel.y;
	     sy < copybuf->maph && dy < m->maph;
	     sy++, dy++) {
		for (sx = 0, dx = mv->esel.x;
		     sx < copybuf->mapw && dx < m->mapw;
		     sx++, dx++) {
			node_copy(copybuf, &copybuf->map[sy][sx], 0,
			    m, &m->map[dy][dx], m->cur_layer);
		}
	}
}

static void
select_kill(void *p, struct mapview *mv)
{
	struct map *m = mv->map;
	int x, y;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no selection to kill."));
		return;
	}
	
	dprintf("[%d,%d]+[%d,%d]\n", mv->esel.x, mv->esel.y, mv->esel.w,
	    mv->esel.h);

	for (y = mv->esel.y; y < mv->esel.y + mv->esel.h; y++) {
		for (x = mv->esel.x; x < mv->esel.x + mv->esel.w; x++) {
			node_clear(m, &m->map[y][x], m->cur_layer);
		}
	}
}

static void
select_cut(void *p, struct mapview *mv)
{
	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no selection to cut."));
		return;
	}
	select_copy(p, mv);
	select_kill(p, mv);
}

void
select_init(void *p)
{
	struct select *sel = p;

	tool_init(&sel->tool, "select", &select_ops, MAPEDIT_TOOL_SELECT);
	TOOL(sel)->cursor = SPRITE(sel, TOOL_SELECT_CURSOR);

	tool_bind_key(sel, KMOD_CTRL, SDLK_c, select_copy, 0);
	tool_bind_key(sel, KMOD_CTRL, SDLK_v, select_paste, 1);
	tool_bind_key(sel, KMOD_CTRL, SDLK_x, select_cut, 1);
	tool_bind_key(sel, KMOD_CTRL, SDLK_k, select_kill, 1);
}


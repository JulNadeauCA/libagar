/*	$Csoft: stamp.c,v 1.51 2003/09/07 04:17:37 vedge Exp $	*/

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

#include <engine/widget/radio.h>

static int	 stamp_cursor(void *, struct mapview *, SDL_Rect *);
static void	 stamp_effect(void *, struct mapview *, struct map *,
		              struct node *);

const struct tool_ops stamp_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	stamp_cursor,
	stamp_effect,
	NULL			/* mouse */
};

void
stamp_init(void *p)
{
	struct stamp *st = p;
	static const char *mode_items[] = {
		N_("Replace"),
		N_("Insert"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	tool_init(&st->tool, "stamp", &stamp_ops, MAPEDIT_TOOL_STAMP);
	st->mode = STAMP_REPLACE;

	win = TOOL(st)->win = window_new("mapedit-tool-stamp");
	window_set_caption(win, _("Stamp"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	event_new(win, "window-close", tool_window_close, "%p", st);

	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &st->mode);
}

static void
stamp_effect(void *p, struct mapview *mv, struct map *m, struct node *node)
{
	struct stamp *st = p;
	struct map *copybuf = &mapedit.copybuf;
	int sx, sy, dx, dy;
	
	for (sy = 0, dy = mv->cy - copybuf->maph/2;
	     sy < copybuf->maph && dy < m->maph;
	     sy++, dy++) {
		for (sx = 0, dx = mv->cx - copybuf->mapw/2;
		     sx < copybuf->mapw && dx < m->mapw;
		     sx++, dx++) {
			struct node *sn = &copybuf->map[sy][sx];
			struct node *dn = &m->map[dy][dx];
			struct noderef *r;

			if (st->mode == STAMP_REPLACE)
				node_clear(m, dn, m->cur_layer);

			TAILQ_FOREACH(r, &sn->nrefs, nrefs)
				node_copy_ref(r, m, dn, m->cur_layer);
		}
	}
}

static int
stamp_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	struct map *copybuf = &mapedit.copybuf;
	struct noderef *r;
	int sx, sy, dx, dy;
	int rv = -1;
	
	/* Avoid circular references when viewing the copy buffer. */
	if (mv->map == copybuf)
		return (-1);

	for (sy = 0, dy = rd->y - (copybuf->maph * mv->map->tileh)/2;
	     sy < copybuf->maph;
	     sy++, dy += mv->map->tileh) {
		for (sx = 0, dx = rd->x - (copybuf->mapw * mv->map->tilew)/2;
		     sx < copybuf->mapw;
		     sx++, dx += mv->map->tilew) {
			struct node *sn = &copybuf->map[sy][sx];

			TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
				noderef_draw(mv->map, r,
				    WIDGET(mv)->cx+dx,
				    WIDGET(mv)->cy+dy);
				rv = 0;
			}
			if (mv->flags & MAPVIEW_PROPS)
				mapview_draw_props(mv, sn, dx, dy, -1, -1);
		}
	}
	return (rv);
}


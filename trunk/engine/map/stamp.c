/*	$Csoft: stamp.c,v 1.64 2005/04/14 02:43:48 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/rg/tileset.h>

#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>

#include "map.h"
#include "mapedit.h"

static enum {
	STAMP_SRC_ARTWORK,	/* From artwork list */
	STAMP_SRC_COPYBUF	/* From copy/paste buffer */
} source = STAMP_SRC_ARTWORK;

static int replace = 1;

static void
init(struct tool *t)
{
	static const char *source_items[] = {
		N_("Artwork list"),
		N_("Map buffer"),
		NULL
	};
	struct window *win;
	struct radio *rad;
	struct checkbox *cb;

	win = tool_window(t, "mapedit-tool-stamp");

	label_new(win, LABEL_STATIC, _("Source: "));
	rad = radio_new(win, source_items);
	widget_bind(rad, "value", WIDGET_INT, &source);

	cb = checkbox_new(win, _("Replace mode"));
	widget_bind(cb, "state", WIDGET_INT, &replace);

	tool_push_status(t, _("Specify the destination node."));
}

static void
effect(struct tool *t, struct node *n)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;

	if (source == STAMP_SRC_COPYBUF) {
		struct map *copybuf = &mapedit.copybuf;
		int sx, sy, dx, dy;

		for (sy = 0, dy = mv->cy;
		     sy < copybuf->maph && dy < m->maph;
		     sy++, dy++) {
			for (sx = 0, dx = mv->cx;
			     sx < copybuf->mapw && dx < m->mapw;
			     sx++, dx++) {
				struct node *sn = &copybuf->map[sy][sx];
				struct node *dn = &m->map[dy][dx];
				struct noderef *r;

				if (replace) {
					node_clear(m, dn, m->cur_layer);
				}
				TAILQ_FOREACH(r, &sn->nrefs, nrefs)
					node_copy_ref(r, m, dn, m->cur_layer);
			}
		}
	} else {
		struct tlist_item *it;

		if (mv->art_tl != NULL &&
		    (it = tlist_item_selected(mv->art_tl)) != 0 &&
		    strcmp(it->class, "tile") == 0) {
			struct tile *t = it->p1;
			struct noderef *r;
			
			if (replace) {
				node_clear(m, n, m->cur_layer);
			}
			r = node_add_sprite(m, n, t->ts, t->sprite);
			r->layer = m->cur_layer;
		}
	}
}

static int
cursor(struct tool *t, SDL_Rect *rd)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	struct tlist_item *it;
	int rv = -1;

	if (source == STAMP_SRC_COPYBUF) {
		struct map *copybuf = &mapedit.copybuf;
		struct noderef *r;
		int sx, sy, dx, dy;
	
		/* Avoid circular references when viewing the copy buffer. */
		if (mv->map == copybuf) {
			return (-1);
		}
		for (sy = 0, dy = rd->y;
		     sy < copybuf->maph;
		     sy++, dy += mv->tilesz) {
			for (sx = 0, dx = rd->x;
			     sx < copybuf->mapw;
			     sx++, dx += mv->tilesz) {
				struct node *sn = &copybuf->map[sy][sx];
	
				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					noderef_draw(m, r,
					    WIDGET(mv)->cx+dx,
					    WIDGET(mv)->cy+dy,
					    mv->tilesz);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS)
					mapview_draw_props(mv, sn, dx, dy,
					    -1, -1);
			}
		}
	} else if (mv->art_tl != NULL &&
	   (it = tlist_item_selected(mv->art_tl)) != NULL &&
	   strcmp(it->class, "tile") == 0) {
		struct tile *tile = it->p1;
		int dx, dy;

		if (tile->su != NULL) {
			struct noderef rtmp;

			noderef_init(&rtmp, NODEREF_SPRITE);
			noderef_set_sprite(&rtmp, m, tile->ts, tile->sprite);
			noderef_draw(m, &rtmp,
			    WIDGET(mv)->cx + rd->x,
			    WIDGET(mv)->cy + rd->y,
			    mv->tilesz);
			noderef_destroy(m, &rtmp);
			rv = -1;
		}
	}
	return (rv);
}

const struct tool stamp_tool = {
	N_("Stamp"),
	N_("Insert the contents of the copy/paste buffer."),
	STAMP_TOOL_ICON,
	-1,
	init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	cursor,
	effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

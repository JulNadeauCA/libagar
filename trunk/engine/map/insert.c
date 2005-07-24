/*	$Csoft: insert.c,v 1.19 2005/07/19 04:24:14 vedge Exp $	*/

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

#ifdef MAP

#include <engine/rg/tileset.h>

#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/primitive.h>

#include "map.h"
#include "mapedit.h"

static enum {
	STAMP_SRC_ARTWORK,	/* From artwork list */
	STAMP_SRC_COPYBUF	/* From copy/paste buffer */
} source = STAMP_SRC_ARTWORK;

enum gfx_snap_mode insert_snap_mode = GFX_SNAP_NOT;

static int replace = 1;
static int angle = 0;

static void
insert_pane(struct tool *t, void *con)
{
	static const char *source_items[] = {
		N_("Artwork"),
		N_("Map buffer"),
		NULL
	};
	struct radio *rad;
	struct checkbox *cb;
	struct spinbutton *sb;
	struct combo *com;

	label_new(con, LABEL_STATIC, _("Source: "));
	rad = radio_new(con, source_items);
	widget_bind(rad, "value", WIDGET_INT, &source);
	
	label_new(con, LABEL_STATIC, _("Snap to: "));
	rad = radio_new(con, gfx_snap_names);
	widget_bind(rad, "value", WIDGET_INT, &insert_snap_mode);

	cb = checkbox_new(con, _("Replace mode"));
	widget_bind(cb, "state", WIDGET_INT, &replace);

	sb = spinbutton_new(con, _("Rotation: "));
	widget_bind(sb, "value", WIDGET_INT, &angle);
	spinbutton_set_range(sb, 0, 360);
	spinbutton_set_increment(sb, 90);
}

static void
init_gfx_ref(struct mapview *mv, struct noderef *r, struct sprite *spr)
{
	struct map *m = mv->map;
	int sm;

	noderef_init(r, NODEREF_SPRITE);
	noderef_set_sprite(r, m, spr->pgfx->pobj, spr->index);
	r->layer = m->cur_layer;
	r->r_gfx.xcenter = 0;
	r->r_gfx.ycenter = 0;
	r->r_gfx.xorigin = spr->xOrig;
	r->r_gfx.yorigin = spr->yOrig;

	switch (insert_snap_mode) {
	case GFX_SNAP_NOT:
		r->r_gfx.xcenter += mv->cxoffs*TILESZ/MV_TILESZ(mv);
		r->r_gfx.ycenter += mv->cyoffs*TILESZ/MV_TILESZ(mv);
		break;
	case GFX_SNAP_TO_GRID:
		r->r_gfx.xcenter += TILESZ/2;
		r->r_gfx.ycenter += TILESZ/2;
		break;
	}

	transform_rotate(r, angle);
}

static void
insert_init(struct tool *t)
{
	tool_push_status(t,
	    _("Select position on map ($(L)=Insert, $(M)=Rotate)"));
}

static int
insert_effect(struct tool *t, struct node *n)
{
	struct mapview *mv = t->mv;
	struct map *m = mv->map;
	struct noderef *r;

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
		struct sprite *spr;
		SDL_Surface *su;
		int sx, sy, dx, dy;

		if (mv->art_tl == NULL ||
		    (it = tlist_selected_item(mv->art_tl)) == 0 ||
		    strcmp(it->class, "tile") != 0) {
			return (1);
		}
		spr = it->p1;
		su = spr->su;

		for (sy = 0, dy = mv->cy;
		     sy < su->h && dy < m->maph;
		     sy += TILESZ, dy++) {
			for (sx = 0, dx = mv->cx;
			     sx < su->w && dx < m->mapw;
			     sx += TILESZ, dx++) {
				struct node *dn = &m->map[dy][dx];
			
				r = Malloc(sizeof(struct noderef),
				    M_MAP_NODEREF);
				init_gfx_ref(mv, r, spr);
				r->r_gfx.rs.x = sx;
				r->r_gfx.rs.y = sy;
				/* XXX may be smaller */
				r->r_gfx.rs.w = TILESZ;
				r->r_gfx.rs.h = TILESZ;
			
				TAILQ_INSERT_TAIL(&dn->nrefs, r, nrefs);
			}
		}
	}
	return (1);
}

static int
insert_cursor(struct tool *t, SDL_Rect *rd)
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
		     sy++, dy += MV_TILESZ(mv)) {
			for (sx = 0, dx = rd->x;
			     sx < copybuf->mapw;
			     sx++, dx += MV_TILESZ(mv)) {
				struct node *sn = &copybuf->map[sy][sx];
	
				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					noderef_draw(m, r,
					    WIDGET(mv)->cx+dx,
					    WIDGET(mv)->cy+dy,
					    mv->cam);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS)
					mapview_draw_props(mv, sn, dx, dy,
					    -1, -1);
			}
		}
	} else if (mv->art_tl != NULL &&
	   (it = tlist_selected_item(mv->art_tl)) != NULL &&
	   strcmp(it->class, "tile") == 0) {
		struct sprite *spr = it->p1;
		int dx, dy;

		if (spr->su != NULL) {
			struct noderef rtmp;
			struct transform *trans;

			init_gfx_ref(mv, &rtmp, spr);
			primitives.rect_outlined(mv, rd->x+1, rd->y+1,
			    MV_TILESZ(mv)-1, MV_TILESZ(mv)-1,
			    COLOR(MAPVIEW_GRID_COLOR));
			noderef_draw(m, &rtmp,
			    WIDGET(mv)->cx + rd->x,
			    WIDGET(mv)->cy + rd->y,
			    mv->cam);
			noderef_destroy(m, &rtmp);
			rv = 0;
		}
	}
	return (rv);
}

static int
insert_mousebuttondown(struct tool *t, int x, int y, int btn)
{
	if (btn == SDL_BUTTON_MIDDLE) {
		angle = (angle + 90) % 360;
		return (1);
	}
	return (0);
}

const struct tool insert_tool = {
	"insert",
	N_("Insert map element."),
	STAMP_TOOL_ICON, -1,
	TOOL_HIDDEN,
	insert_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	insert_cursor,
	insert_effect,
	NULL,			/* mousemotion */
	insert_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL,			/* keyup */
	insert_pane
};

#endif /* MAP */

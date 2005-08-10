/*	$Csoft: fill.c,v 1.12 2005/07/30 05:01:34 vedge Exp $	*/

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

#include <compat/arc4random.h>

#include <engine/engine.h>

#ifdef MAP

#include <engine/rg/tileset.h>

#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/tlist.h>

#include "map.h"
#include "mapedit.h"

struct fill_tool {
	struct tool tool;
	enum fill_mode {
		FILL_FROM_CLIPBRD,
		FILL_FROM_ART,
		FILL_CLEAR
	} mode;
	int randomize_angle;
};

static void
fill_init(void *p)
{
	struct fill_tool *fi = p;

	fi->mode = FILL_FROM_ART;
	fi->randomize_angle = 0;
	
	tool_push_status(fi, _("Select a node and fill with $(L)."));
}

static void
fill_pane(void *p, void *con)
{
	struct fill_tool *fi = p;
	static const char *mode_items[] = {
		N_("Clipboard pattern"),
		N_("Source artwork"),
		N_("Clear"),
		NULL
	};
	struct radio *rad;
	struct checkbox *cb;

	rad = radio_new(con, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &fi->mode);

	cb = checkbox_new(con, _("Randomize angle"));
	widget_bind(cb, "state", WIDGET_BOOL, &fi->randomize_angle);
}

static int
fill_effect(void *p, struct node *n)
{
	struct fill_tool *fi = p;
	struct mapview *mv = TOOL(fi)->mv;
	struct map *m = mv->map;
	struct map *copybuf = &mapedit.copybuf;
	int sx = 0, sy = 0, dx = 0, dy = 0;
	int dw = m->mapw, dh = m->maph;
	int x, y, angle = 0, i = 0;
	struct tlist_item *it;
	struct sprite *spr;
	Uint32 rand = 0;
	Uint8 byte = 0;

	mapview_get_selection(mv, &dx, &dy, &dw, &dh);

	switch (fi->mode) {
	case FILL_FROM_CLIPBRD:
		if (copybuf->mapw == 0 || copybuf->maph == 0) {
			text_msg(MSG_ERROR, _("The clipboard is empty."));
			return (1);
		}
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				struct node *sn = &copybuf->map[sy][sx];
				struct node *dn = &m->map[y][x];
				struct noderef *r;

				node_clear(m, dn, m->cur_layer);
				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					node_copy_ref(r, m, dn, m->cur_layer);
				}
				if (++sx >= copybuf->mapw)
					sx = 0;
			}
		}
		if (++sy >= copybuf->maph) {
			sy = 0;
		}
		break;
	case FILL_FROM_ART:
		if (mv->lib_tl == NULL ||
		    (it = tlist_selected_item(mv->lib_tl)) == NULL ||
		    strcmp(it->class, "tile") != 0) {
			break;
		}
		spr = it->p1;
		
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				struct node *n = &m->map[y][x];
				struct noderef *r;

				node_clear(m, n, m->cur_layer);
				r = Malloc(sizeof(struct noderef),
				    M_MAP_NODEREF);
				noderef_init(r, NODEREF_SPRITE);
				noderef_set_sprite(r, m, spr->pgfx->pobj,
				    spr->index);
				noderef_set_layer(r, m->cur_layer);
				r->r_gfx.xorigin = spr->xOrig;
				r->r_gfx.yorigin = spr->yOrig;
				r->r_gfx.xcenter = TILESZ/2;
				r->r_gfx.ycenter = TILESZ/2;
				TAILQ_INSERT_TAIL(&n->nrefs, r, nrefs);
	
				if (fi->randomize_angle) {
					switch (i++) {
					case 0:
						rand = arc4random();
						byte = (rand&0xff000000) >> 24;
						break;
					case 1:
						byte = (rand&0x00ff0000) >> 16;
						break;
					case 2:
						byte = (rand&0x0000ff00) >> 8;
						break;
					case 3:
						byte = (rand&0x000000ff);
						i = 0;
						break;
					}
					if (byte < 60) {
						transform_rotate(r, 0);
					} else if (byte < 120) {
						transform_rotate(r, 90);
					} else if (byte < 180) {
						transform_rotate(r, 180);
					} else if (byte < 240) {
						transform_rotate(r, 270);
					}
				}
			}
		}
		break;
	case FILL_CLEAR:
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++)
				node_clear(m, &m->map[y][x], m->cur_layer);
		}
		break;
	}
	return (1);
}

const struct tool_ops fill_tool_ops = {
	"Fill", N_("Clear/fill layer"),
	FILL_TOOL_ICON,
	sizeof(struct fill_tool),
	0,
	fill_init,
	NULL,			/* destroy */
	fill_pane,
	NULL,			/* edit */
	NULL,			/* cursor */
	fill_effect,
	
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL,			/* keyup */
};

#endif /* MAP */

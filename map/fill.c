/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/tlist.h>
#include <agar/gui/radio.h>
#include <agar/gui/checkbox.h>

#include <agar/map/map.h>

#include <string.h>

struct rg_fill_tool {
	MAP_Tool tool;
	enum fill_mode {
		FILL_FROM_CLIPBRD,
		FILL_FROM_ART,
		FILL_CLEAR
	} mode;
	int randomize_angle;
};

static void
Init(void *_Nonnull p)
{
	struct rg_fill_tool *fi = p;

	fi->mode = FILL_FROM_ART;
	fi->randomize_angle = 0;
	
	MAP_ToolPushStatus(fi, _("Select a node and fill with $(L)."));
}

static void
EditPane(void *_Nonnull p, void *_Nonnull con)
{
	struct rg_fill_tool *fi = p;
	static const char *mode_items[] = {
		N_("Clipboard pattern"),
		N_("Source artwork"),
		N_("Clear"),
		NULL
	};

	AG_RadioNewUint(con, AG_RADIO_HFILL, mode_items, &fi->mode);
	AG_CheckboxNewInt(con, 0, _("Randomize angle"), &fi->randomize_angle);
}

static int
Effect(void *_Nonnull p, MAP_Node *_Nonnull n)
{
	struct rg_fill_tool *fi = p;
	MAP_View *mv = TOOL(fi)->mv;
	MAP *m = mv->map;
	MAP *copybuf = &mapEditor.copybuf;
	int sx = 0, sy = 0, dx = 0, dy = 0;
	int dw = m->mapw, dh = m->maph;
	int x, y;
	AG_TlistItem *it;
	RG_Tile *tile;
#if 0
	int i = 0;
#endif

	MAP_ViewGetSelection(mv, &dx, &dy, &dw, &dh);
	MAP_ModBegin(m);

	switch (fi->mode) {
	case FILL_FROM_CLIPBRD:
		if (copybuf->mapw == 0 || copybuf->maph == 0) {
			AG_TextMsg(AG_MSG_ERROR, _("The clipboard is empty."));
			return (1);
		}
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				MAP_Node *sn = &copybuf->map[sy][sx];
				MAP_Node *dn = &m->map[y][x];
				MAP_Item *r;

				MAP_ModNodeChg(m, x, y);
				MAP_NodeRemoveAll(m, dn, m->cur_layer);
				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					MAP_NodeCopyItem(r, m, dn,
					    m->cur_layer);
				}
				if (++sx >= (int)copybuf->mapw)
					sx = 0;
			}
		}
		if (++sy >= (int)copybuf->maph) {
			sy = 0;
		}
		break;
	case FILL_FROM_ART:
		if (mv->lib_tl == NULL ||
		    (it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
		    strcmp(it->cat, "tile") != 0) {
			break;
		}
		tile = it->p1;
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				MAP_Node *n = &m->map[y][x];
				MAP_Item *r;

				MAP_ModNodeChg(m, x, y);
				MAP_NodeRemoveAll(m, n, m->cur_layer);
				r = Malloc(sizeof(MAP_Item));
				MAP_ItemInit(r, MAP_ITEM_TILE);
				MAP_ItemSetTile(r, m, tile->ts, tile->main_id);
				MAP_ItemSetLayer(r, m->cur_layer);
				r->r_gfx.xorigin = tile->xOrig;
				r->r_gfx.yorigin = tile->yOrig;
				r->r_gfx.xcenter = MAPTILESZ/2;
				r->r_gfx.ycenter = MAPTILESZ/2;
				TAILQ_INSERT_TAIL(&n->nrefs, r, nrefs);
#if 0
				if (fi->randomize_angle) {
					Uint32 rand = 0;
					Uint8 byte = 0;

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
						RG_TransformRotate(r, 0);
					} else if (byte < 120) {
						RG_TransformRotate(r, 90);
					} else if (byte < 180) {
						RG_TransformRotate(r, 180);
					} else if (byte < 240) {
						RG_TransformRotate(r, 270);
					}
				}
#endif
			}
		}
		break;
	case FILL_CLEAR:
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				MAP_ModNodeChg(m, x, y);
				MAP_NodeRemoveAll(m, &m->map[y][x],
				    m->cur_layer);
			}
		}
		break;
	}
	MAP_ModEnd(m);
	return (1);
}

const MAP_ToolOps mapFillOps = {
	"Fill", N_("Clear/fill layer"),
	&mapIconFill,
	sizeof(struct rg_fill_tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	EditPane,
	NULL,			/* edit */
	NULL,			/* cursor */
	Effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL,			/* keyup */
};

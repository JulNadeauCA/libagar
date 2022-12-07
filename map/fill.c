/*
 * Copyright (c) 2002-2022 Julien Nadeau Carriere <vedge@csoft.net>
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

typedef struct fill_tool {
	MAP_Tool tool;                             /* MAP_Tool -> FILL_Tool */
	enum fill_mode {
		FILL_FROM_CLIPBRD,
		FILL_FROM_ART,
		FILL_CLEAR
	} mode;
	int randomize_angle;
} FILL_Tool;

static void
Init(void *_Nonnull obj)
{
	FILL_Tool *ft = obj;

	ft->mode = FILL_FROM_ART;
	ft->randomize_angle = 0;
	
	MAP_ToolPushStatus(ft,
	    _("Select a node and Left-Click to Fill / Clear similar nodes."));
}

static void
EditPane(void *_Nonnull obj, void *_Nonnull box)
{
	FILL_Tool *ft = obj;
	static const char *mode_items[] = {
		N_("Clipboard pattern"),
		N_("Source artwork"),
		N_("Clear"),
		NULL
	};

	AG_RadioNewUint(box, AG_RADIO_HFILL, mode_items, &ft->mode);
	AG_CheckboxNewInt(box, 0, _("Randomize angle"), &ft->randomize_angle);
}

static int
Effect(void *_Nonnull obj, MAP_Node *_Nonnull node)
{
	FILL_Tool *ft = obj;
	MAP_View *mv = TOOL(ft)->mv;
	MAP *map = mv->map, *mapCopy;
	int sx = 0, sy = 0, dx = 0, dy = 0;
	int dw = map->w, dh = map->h;
	int x,y, count;
	AG_TlistItem *it;
	RG_Tile *tile;
	MAP_NodeselTool *selTool;
#if 0
	int i = 0;
#endif

	MAP_ViewGetSelection(mv, &dx, &dy, &dw, &dh);
	MAP_BeginRevision(map);

	switch (ft->mode) {
	case FILL_FROM_CLIPBRD:
		if ((selTool = (MAP_NodeselTool *)MAP_ViewFindTool(mv, "Nodesel")) == NULL) {
			MAP_ViewStatus(mv, _("No clipboard is available."));
			return (0);
		}
		mapCopy = &selTool->mapCopy;
		if (mapCopy->w == 0 || mapCopy->h == 0) {
			MAP_ViewStatus(mv, _("The clipboard is empty."));
			return (0);
		}
		count = 0;
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				MAP_Node *nodeSrc = &mapCopy->map[sy][sx];
				MAP_Node *node = &map->map[y][x];
				MAP_Item *mi;

				MAP_NodeRevision(map, x,y, map->undo, map->nUndo);

				MAP_NodeClear(map, node, map->layerCur);

				TAILQ_FOREACH(mi, &nodeSrc->items, items) {
					MAP_DuplicateItem(map, node,
					    map->layerCur, mi);
				}
				if (++sx >= (int)mapCopy->w) {
					sx = 0;
				}
				count++;
			}
		}
		if (++sy >= (int)mapCopy->h) {
			sy = 0;
		}
		MAP_ViewStatus(mv, _("Filled %d nodes from clipboard (%dx%d) "
		                     "at [" AGSI_BOLD "%d,%d" AGSI_RST "]."),
		    count, dw,dh, dx,dy);
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
				MAP_Node *node = &map->map[y][x];
				MAP_Tile *mt;

				MAP_NodeRevision(map, x,y, map->undo, map->nUndo);

				MAP_NodeClear(map, node, map->layerCur);

				mt = MAP_TileNew(map, node, tile->ts,
				                 tile->main_id);
				MAPITEM(mt)->layer = map->layerCur;

				mt->xCenter = MAP_TILESZ_DEF / 2;
				mt->yCenter = MAP_TILESZ_DEF / 2;
/*				mt->xOrigin = tile->xOrig; */
/*				mt->yOrigin = tile->yOrig; */
#if 0
				if (ft->randomize_angle) {
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
						RG_TransformRotate(mt, 0);
					} else if (byte < 120) {
						RG_TransformRotate(mt, 90);
					} else if (byte < 180) {
						RG_TransformRotate(mt, 180);
					} else if (byte < 240) {
						RG_TransformRotate(mt, 270);
					}
				}
#endif
			}
		}
		break;
	case FILL_CLEAR:
		for (y = dy; y < dy+dh; y++) {
			for (x = dx; x < dx+dw; x++) {
				MAP_NodeRevision(map, x,y, map->undo, map->nUndo);
				MAP_NodeClear(map, &map->map[y][x], map->layerCur);
			}
		}
		MAP_ViewStatus(mv, _("Cleared (%dx%d) nodes at "
		                     "[" AGSI_BOLD "%d,%d" AGSI_RST "]."),
		    dw,dh, dx,dy);
		break;
	}
	MAP_CommitRevision(map);
	return (1);
}

const MAP_ToolOps mapFillOps = {
	"Fill", N_("Clear/fill layer"),
	&mapIconFill,
	sizeof(FILL_Tool),
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

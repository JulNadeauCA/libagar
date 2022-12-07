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
#include <agar/gui/checkbox.h>

#include <agar/map/map.h>

static int all_layers = 0;

static int
EraseSelection(MAP_Tool *_Nonnull tool, AG_KeySym key, int state, void *_Nullable arg)
{
	MAP_View *mv = tool->mv;
	MAP *map = mv->map;
	Uint x, y;

	for (y = 0; y < map->h; y++) {
		for (x = 0; x < map->w; x++) {
			MAP_Node *node = &map->map[y][x];
			MAP_Item *mi;
			Uint i;

			TAILQ_FOREACH(mi, &node->items, items) {
				if (mi->layer != map->layerCur) {
					continue;
				}
				if (mi->flags & MAP_ITEM_SELECTED)
					MAP_NodeDelItem(map, node, mi);
			}
			for (i = 0; i < node->nLocs; i++) {
				MAP_Location *loc = &node->locs[i];
				MAP_Object *mo = loc->obj;

				if ((loc->flags & MAP_OBJECT_LOCATION_SELECTED) ||
				     (mo->flags & MAP_OBJECT_SELECTED)) {
					MAP_NodeDelLocation(map, node, loc);
				}
			}
		}
	}
	return (0);
}

static void
Init(void *_Nonnull obj)
{
	MAP_ToolBindKey(obj, 0, AG_KEY_DELETE, EraseSelection, NULL);
	MAP_ToolPushStatus(obj, _("Left-Click to Delete item under cursor. "
	                          "Press DEL to Delete selection."));
}

static void
EditPane(void *_Nonnull tool, void *_Nonnull box)
{
	AG_CheckboxNewInt(box, 0, _("All layers"), &all_layers);
}

static int
MouseButtonDown(void *_Nonnull obj, int x, int y, int btn)
{
	if (btn == AG_MOUSE_LEFT) {
		MAP_BeginRevision(TOOL(obj)->mv->map);
	}
	return (0);
}

static int
MouseButtonUp(void *_Nonnull obj, int x, int y, int btn)
{
	MAP *map = TOOL(obj)->mv->map;

	if (btn == AG_MOUSE_LEFT) {
		if (map->nChanges == 0) {
			MAP_AbortRevision(map);
		}
		MAP_CommitRevision(map);
	}
	return (0);
}

static int
Effect(void *_Nonnull obj, MAP_Node *_Nonnull node)
{
	MAP_View *mv = TOOL(obj)->mv;
	MAP *map = mv->map;
	MAP_Item *mi;
	int nChanges = 0;
	Uint i;

	MAP_NodeRevision(map, mv->cx, mv->cy, map->undo, map->nUndo);

	TAILQ_FOREACH(mi, &node->items, items) {
		if (!all_layers &&
		    mi->layer != map->layerCur)
			continue;

		TAILQ_REMOVE(&node->items, mi, items);
		MAP_ItemDestroy(map, mi);
		free(mi);

		nChanges++;
	}

del_locations:
	for (i = 0; i < node->nLocs; i++) {
		MAP_Location *loc = &node->locs[i];

		if (!all_layers &&
		    loc->layer != map->layerCur)
			continue;

		if (MAP_NodeDelLocationAtIndex(map, node, i)) {
			nChanges++;
			goto del_locations;	/* Operation changes locs[] */
		}
	}

	return (nChanges);
}

static int
Cursor(void *_Nonnull obj, AG_Rect *_Nonnull rd)
{
	AG_Color c;

	AG_ColorRGBA_8(&c, 255,0,0,64);
	AG_DrawRectBlended(TOOL(obj)->mv, rd, &c,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	return (1);
}

const MAP_ToolOps mapEraserOps = {
	"Eraser", N_("Remove node elements."),
	&mapIconEraser,
	sizeof(MAP_Tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	EditPane,
	NULL,			/* edit */
	Cursor,
	Effect,
	NULL,			/* mousemotion */
	MouseButtonDown,
	MouseButtonUp,
	NULL,			/* keydown */
	NULL			/* keyup */
};

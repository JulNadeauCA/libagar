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
#include <agar/gui/box.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/notebook.h>
#include <agar/gui/numerical.h>
#include <agar/gui/radio.h>
#include <agar/gui/tlist.h>

#include <agar/map/map.h>
#include <agar/map/insert.h>

#include <string.h>

/*
 * Insert a static map item (MAP_Item).
 */

static void
Init(void *_Nonnull obj)
{
	MAP_InsertTool *tool = obj;

	tool->snap_mode = RG_SNAP_NONE;
	tool->replace_mode = 0;
	tool->angle = 0;

//	AG_ObjectInitStatic(&tool->mTmp, &mapClass);
	tool->mvTmp = NULL;

	MAP_ToolPushStatus(tool,
	    _("Select a position and Left-Click to Insert. "
	      "Use Middle-Click to Rotate."));
}

static void
Destroy(void *_Nonnull obj)
{
//	MAP_InsertTool *tool = obj;

//	AG_ObjectDestroy(&tool->mTmp);
}

#if 0
static void
SnapTile(MAP_InsertTool *_Nonnull tool, MAP_Item *_Nonnull mi,
    RG_Tile *_Nonnull tile)
{
	MAP_View *mv = TOOL(tool)->mv;

	switch (tool->snap_mode) {
	case RG_SNAP_NONE:
		mi->data.tile.xCenter += mv->cxoffs * MAP_TILESZ_DEF / MAP_TILESZ(mv);
		mi->data.tile.yCenter += mv->cyoffs * MAP_TILESZ_DEF / MAP_TILESZ(mv);
		break;
	default:
		break;
	}
}
#endif

#if 0
static void
GenerateTileMap(MAP_InsertTool *_Nonnull tool, RG_Tile *_Nonnull tile)
{
	MAP *mapTmp = &tool->mTmp;
	int sy,sx, dx,dy;
	int sw = tile->su->w / MAP_TILESZ_DEF;
	int sh = tile->su->h / MAP_TILESZ_DEF;

	if (tile->su->w % MAP_TILESZ_DEF > 0) sw++;
	if (tile->su->h % MAP_TILESZ_DEF > 0) sh++;

	MAP_AllocNodes(mapTmp, sw, sh);

	mapTmp->xOrigin = tile->xOrig / MAP_TILESZ_DEF;
	mapTmp->yOrigin = tile->yOrig / MAP_TILESZ_DEF;

	for (sy=0, dy=0; sy < tile->su->h; sy += MAP_TILESZ_DEF, dy++) {
		for (sx=0, dx=0; sx < tile->su->w; sx += MAP_TILESZ_DEF, dx++) {
			MAP_Tile *mt;
			int dw,dh, nlayer;

			if (dx >= (int)mapTmp->w ||
			    dy >= (int)mapTmp->h)
				continue;

			dw = tile->su->w - sx;
			dh = tile->su->h - sy;

			mt = MAP_TileNew(mapTmp, &mapTmp->map[dy][dx],
			                 tile->ts, tile->main_id);

			mt->rs.x = dx * MAP_TILESZ_DEF;
			mt->rs.y = dy * MAP_TILESZ_DEF;
			mt->rs.w = (dw >= MAP_TILESZ_DEF) ? MAP_TILESZ_DEF : dw;
			mt->rs.h = (dh >= MAP_TILESZ_DEF) ? MAP_TILESZ_DEF : dh;

			MAPITEM(mt)->flags |= RG_TILE_ATTR2(tile,dx,dy);

			nlayer = RG_TILE_LAYER2(tile,dx,dy);
			if (nlayer < 0) {
				nlayer = 0;
			} else {
				if (nlayer >= (int)mapTmp->nLayers)
					MAP_PushLayer(mapTmp, "");
			}
			MAPITEM(mt)->layer = nlayer;
			
			/* XXX also need to rotate the whole map */
	//		RG_TransformRotate(mt, tool->angle);
		}
	}
}
#endif

static void
EditPane(void *_Nonnull obj, void *_Nonnull con)
{
#if 0
	MAP_InsertTool *tool = obj;
	AG_TlistItem *it;
	MAP_View *mvMain = TOOL(tool)->mv;
	MAP_View *mv;
	RG_Tile *tile;
	AG_Notebook *nb;
	AG_NotebookTab *nt;

	if ((it = AG_TlistSelectedItem(mvMain->lib_tl)) == NULL ||
	     strcmp(it->cat, "tile") != 0) {
		return;
	}
	tile = it->p1;
	
	nb = AG_NotebookNew(con, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Tiles"), AG_BOX_VERT);
	{
		mv = tool->mvTmp = MAP_ViewNew(nt, &tool->mTmp,
		                               MAP_VIEW_EDIT | MAP_VIEW_GRID,
		                               NULL, NULL);
		MAP_ViewSizeHint(mv, 7,7);

		MAP_ViewSelectTool(mv,
		    MAP_ViewRegTool(mv, &mapNodeselOps, &tool->mTmp),
		    &tool->mTmp);

		GenerateTileMap(tool, tile);
	}

	nt = AG_NotebookAdd(nb, _("Settings"), AG_BOX_VERT);
	{
		AG_Numerical *num;

		AG_LabelNewS(nt, 0, _("Snap to: "));
		AG_RadioNewUint(nt, AG_RADIO_HFILL, rgTileSnapModes,
		    &tool->snap_mode);

		AG_CheckboxNewInt(nt, 0, _("Replace mode"), &tool->replace_mode);

		num = AG_NumericalNewIntR(nt, 0, "deg", _("Rotation: "),
		    &tool->angle, 0, 360);
		AG_SetInt(num, "inc", 90);
	}
#endif
}

static int
Effect(void *_Nonnull obj, MAP_Node *_Nonnull node)
{
#if 0
	MAP_InsertTool *tool = obj;
	MAP_View *mv = TOOL(tool)->mv;
	MAP *mapSrc = &tool->mTmp;
	MAP *mapDst = mv->map;
	AG_TlistItem *it;
	int sx, sy, sx0, sy0, sx1, sy1;
	int dx, dy, dx0, dy0;
	const int tileSz = MAP_TILESZ(mv);
	
	if (mv->lib_tl == NULL ||
	    (it = AG_TlistSelectedItem(mv->lib_tl)) == 0 ||
	    strcmp(it->cat, "tile") != 0) {
		return (1);
	}
	/* tile = it->p1; */

	if (tool->mvTmp->esel.set) {
		sx0 = tool->mvTmp->esel.x;
		sy0 = tool->mvTmp->esel.y;
		sx1 = sx0 + tool->mvTmp->esel.w - 1;
		sy1 = sy0 + tool->mvTmp->esel.h - 1;
		dx0 = mv->cx;
		dy0 = mv->cy;
	} else {
		sx0 = 0;
		sy0 = 0;
		sx1 = mapSrc->w - 1;
		sy1 = mapSrc->h - 1;
		dx0 = mv->cx - mapSrc->xOrigin;
		dy0 = mv->cy - mapSrc->yOrigin;
	}

	MAP_BeginRevision(mapDst);
	for (sy=sy0, dy=dy0;
	     sy <= sy1 && dy < (int)mapDst->h;
	     sy++, dy++) {
		for (sx = sx0, dx = dx0;
		     sx <= sx1 && dx < (int)mapDst->w;
		     sx++, dx++) {
			MAP_Node *sn, *dn;
			MAP_Item *miSrc, *mi;

			if (dx < 0 || dx >= (int)mapDst->w ||
			    dy < 0 || dy >= (int)mapDst->h) {
				continue;
			}
			sn = &mapSrc->map[sy][sx]; 
			dn = &mapDst->map[dy][dx];
			
			MAP_NodeRevision(mapDst, dx,dy, map->undo, map->nUndo);

			if (tool->replace_mode) {
				MAP_NodeClear(mapDst, dn, mapDst->layerCur);
			}
			TAILQ_FOREACH(miSrc, &sn->items, items) {
				mi = MAP_DuplicateItem(miSrc, mapDst, dn, -1);
				mi->layer += mapDst->layerCur;
				while (mi->layer >= mapDst->nLayers) {
					if (MAP_PushLayer(mapDst, "") == 0)
						MAP_ChangeLayerAdd(mapDst,
						    mapDst->nLayers - 1);
				}
				if (mi->type == MAP_ITEM_TILE &&
				    tool->snap_mode == RG_SNAP_NONE) {
					MAP_Tile *mt = MAPTILE(mi);

					mt->xCenter += mv->cxoffs *
					               MAP_TILESZ_DEF / tileSz;
					mt->yCenter += mv->cyoffs *
					               MAP_TILESZ_DEF / tileSz;
				}
			}
		}
	}
	MAP_CommitRevision(mapDst);
#endif
	return (1);
}

static int
Cursor(void *_Nonnull obj, AG_Rect *_Nonnull rd)
{
#if 0
	MAP_InsertTool *tool = obj;
	MAP_View *mv = TOOL(tool)->mv;
	MAP *mapSrc = &tool->mTmp;
	AG_TlistItem *it;
	RG_Tile *tile;
	AG_Rect r;
	AG_Color c;
	const int tileSz = MAP_TILESZ(mv);
	int sx0, sy0, sx1, sy1;
	int dx0, dy0;
	int dx, dy, sx, sy;

	if (mv->lib_tl == NULL ||
	   (it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	   strcmp(it->cat, "tile") != 0 ||
	   (tile = it->p1) == NULL || tile->su == NULL) {
		return (-1);
	}

	if (tool->snap_mode == RG_SNAP_NONE) {
		r.x = rd->x + 1;
		r.y = rd->y + 1;
		r.w = tileSz - 1;
		r.h = tileSz - 1;

		AG_ColorRGB_8(&c, 200,200,200);
		AG_DrawRectOutline(mv, &r, &c);
	}
	
	if (tool->mvTmp->esel.set) {
		sx0 = tool->mvTmp->esel.x;
		sy0 = tool->mvTmp->esel.y;
		sx1 = sx0 + tool->mvTmp->esel.w - 1;
		sy1 = sy0 + tool->mvTmp->esel.h - 1;
		dx0 = WIDGET(mv)->rView.x1 + rd->x;
		dy0 = WIDGET(mv)->rView.y1 + rd->y;
	} else {
		sx0 = 0;
		sy0 = 0;
		sx1 = mapSrc->w - 1;
		sy1 = mapSrc->h - 1;
		dx0 = WIDGET(mv)->rView.x1 + rd->x - mapSrc->xOrigin * tileSz;
		dy0 = WIDGET(mv)->rView.y1 + rd->y - mapSrc->yOrigin * tileSz;
	}
	if (tool->snap_mode == RG_SNAP_NONE) {
		dx0 += mv->cxoffs * MAP_TILESZ_DEF / tileSz;
		dy0 += mv->cyoffs * MAP_TILESZ_DEF / tileSz;
	}
	for (sy = sy0, dy = dy0; sy <= sy1; sy++, dy += tileSz) {
		for (sx = sx0, dx = dx0; sx <= sx1; sx++, dx += tileSz) {
			MAP_Node *sn = &mapSrc->map[sy][sx];
			MAP_Item *mi;

			TAILQ_FOREACH(mi, &sn->items, items)
				MAP_ItemDraw(mv->map, mi, dx,dy, mv->cam);
		}
	}
#endif
	return (0);
}

static int
MouseButtonDown(void *_Nonnull obj, int x, int y, int btn)
{
	MAP_InsertTool *tool = obj;

	if (btn == AG_MOUSE_MIDDLE) {
		tool->angle = (tool->angle + 90) % 360;
		return (1);
	}
	return (0);
}

static int
MouseMotion(void *_Nonnull obj, int x, int y, int xrel, int yrel)
{
	MAP_InsertTool *tool = obj;
	MAP_View *mv = TOOL(tool)->mv;
	const int tileSz = MAP_TILESZ(mv);
	const int nx = x / tileSz;
	const int ny = y / tileSz;
	AG_TlistItem *it;

	if (nx == mv->mouse.x && ny == mv->mouse.y)
		return (0);

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	if (strcmp(it->cat, "tile") == 0) {
		MAP_ToolSetStatus(tool,
		    _("Insert %s tile at [%d,%d] (Click Left = Confirm | Middle = Rotate)."),
		    it->text, mv->cx, mv->cy);
	}
	return (0);
}

const MAP_ToolOps mapInsertOps = {
	"Insert",
	N_("Insert static map item"),
	&mapIconStamp,
	sizeof(MAP_InsertTool),
	0,
	1,
	Init,
	Destroy,
	EditPane,
	NULL,				/* edit */
	Cursor,
	Effect,
	MouseMotion,
	MouseButtonDown,
	NULL,				/* mousebuttonup */
	NULL,				/* keydown */
	NULL				/* keyup */
};

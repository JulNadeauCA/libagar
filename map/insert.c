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
#include <agar/gui/box.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/notebook.h>
#include <agar/gui/numerical.h>
#include <agar/gui/radio.h>
#include <agar/gui/tlist.h>

#include <agar/map/map.h>
#include <agar/map/insert.h>

#include <string.h>

static void
Init(void *_Nonnull p)
{
	struct map_insert_tool *ins = p;

	ins->snap_mode = RG_SNAP_NONE;
	ins->replace_mode = 0;
	ins->angle = 0;

	AG_ObjectInit(&ins->mTmp, &mapClass);
	OBJECT(&ins->mTmp)->flags |= AG_OBJECT_STATIC;
	ins->mvTmp = NULL;

	MAP_ToolPushStatus(ins,
	    _("Select position on map ($(L)=Insert, $(M)=Rotate)"));
}

static void
Destroy(void *_Nonnull p)
{
	struct map_insert_tool *ins = p;

	AG_ObjectDestroy(&ins->mTmp);
}

#if 0
static void
SnapTile(struct map_insert_tool *_Nonnull ins, MAP_Item *_Nonnull r,
    RG_Tile *_Nonnull tile)
{
	MAP_View *mv = TOOL(ins)->mv;

	switch (ins->snap_mode) {
	case RG_SNAP_NONE:
		r->r_gfx.xcenter += mv->cxoffs*MAPTILESZ/AGMTILESZ(mv);
		r->r_gfx.ycenter += mv->cyoffs*MAPTILESZ/AGMTILESZ(mv);
		break;
	default:
		break;
	}
}
#endif

static void
GenerateTileMap(struct map_insert_tool *_Nonnull ins, RG_Tile *_Nonnull tile)
{
	int sy, sx, dx, dy;
	int sw = tile->su->w/MAPTILESZ;
	int sh = tile->su->h/MAPTILESZ;

	if (tile->su->w%MAPTILESZ > 0) sw++;
	if (tile->su->h%MAPTILESZ > 0) sh++;

	MAP_AllocNodes(&ins->mTmp, sw, sh);
	ins->mTmp.origin.x = tile->xOrig/MAPTILESZ;
	ins->mTmp.origin.y = tile->yOrig/MAPTILESZ;
	for (sy = 0, dy = 0;
	     sy < tile->su->h;
	     sy += MAPTILESZ, dy++) {
		for (sx = 0, dx = 0;
		     sx < tile->su->w;
		     sx += MAPTILESZ, dx++) {
			MAP_Node *dn;
			MAP_Item *r;
			int dw, dh, nlayer;

			if (dx >= (int)ins->mTmp.mapw ||
			    dy >= (int)ins->mTmp.maph)
				continue;

			dn = &ins->mTmp.map[dy][dx];
			dw = tile->su->w - sx;
			dh = tile->su->h - sy;

			r = Malloc(sizeof(MAP_Item));
			MAP_ItemInit(r, MAP_ITEM_TILE);
			MAP_ItemSetTile(r, &ins->mTmp, tile->ts, tile->main_id);

			r->r_gfx.rs.x = dx*MAPTILESZ;
			r->r_gfx.rs.y = dy*MAPTILESZ;
			r->r_gfx.rs.w = (dw >= MAPTILESZ) ? MAPTILESZ : dw;
			r->r_gfx.rs.h = (dh >= MAPTILESZ) ? MAPTILESZ : dh;
			r->flags |= RG_TILE_ATTR2(tile,dx,dy);

			nlayer = RG_TILE_LAYER2(tile,dx,dy);
			if (nlayer < 0) {
				nlayer = 0;
			} else {
				if (nlayer >= (int)ins->mTmp.nLayers)
					MAP_PushLayer(&ins->mTmp, "");
			}
			r->layer = nlayer;
			
			/* XXX also need to rotate the whole map */
	//		RG_TransformRotate(r, ins->angle);

			TAILQ_INSERT_TAIL(&dn->nrefs, r, nrefs);
		}
	}
}

static void
EditPane(void *_Nonnull p, void *_Nonnull con)
{
	struct map_insert_tool *ins = p;
	AG_TlistItem *it;
	MAP_View *mvMain = TOOL(ins)->mv;
	MAP_View *mv;
	RG_Tile *tile;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	if ((it = AG_TlistSelectedItem(mvMain->lib_tl)) == NULL ||
	     strcmp(it->cat, "tile") != 0) {
		return;
	}
	tile = it->p1;
	
	nb = AG_NotebookNew(con, AG_NOTEBOOK_VFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAdd(nb, _("Tiles"), AG_BOX_VERT);
	mv = ins->mvTmp = MAP_ViewNew(ntab, &ins->mTmp,
	    MAP_VIEW_EDIT|MAP_VIEW_GRID, NULL, NULL);
	MAP_ViewSizeHint(mv, 7, 7);
	MAP_ViewSelectTool(mv,
	    MAP_ViewRegTool(mv, &mapNodeselOps, &ins->mTmp),
	    &ins->mTmp);
	GenerateTileMap(ins, tile);

	ntab = AG_NotebookAdd(nb, _("Settings"), AG_BOX_VERT);
	{
		AG_Numerical *num;

		AG_LabelNewS(ntab, 0, _("Snap to: "));
		AG_RadioNewUint(ntab, AG_RADIO_HFILL,
		    rgTileSnapModes, &ins->snap_mode);
		AG_CheckboxNewInt(ntab, 0, _("Replace mode"),
		    &ins->replace_mode);
		num = AG_NumericalNewIntR(ntab, 0, "deg", _("Rotation: "),
		    &ins->angle, 0, 360);
		AG_SetInt(num, "inc", 90);
	}
}

static int
Effect(void *_Nonnull p, MAP_Node *_Nonnull node)
{
	struct map_insert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	MAP *mSrc = &ins->mTmp;
	MAP *mDst = mv->map;
	int sx, sy, sx0, sy0, sx1, sy1;
	int dx, dy, dx0, dy0;
	AG_TlistItem *it;
	
	if (mv->lib_tl == NULL ||
	    (it = AG_TlistSelectedItem(mv->lib_tl)) == 0 ||
	    strcmp(it->cat, "tile") != 0) {
		return (1);
	}
	/* tile = it->p1; */

	if (ins->mvTmp->esel.set) {
		sx0 = ins->mvTmp->esel.x;
		sy0 = ins->mvTmp->esel.y;
		sx1 = sx0 + ins->mvTmp->esel.w - 1;
		sy1 = sy0 + ins->mvTmp->esel.h - 1;
		dx0 = mv->cx;
		dy0 = mv->cy;
	} else {
		sx0 = 0;
		sy0 = 0;
		sx1 = mSrc->mapw-1;
		sy1 = mSrc->maph-1;
		dx0 = mv->cx - mSrc->origin.x;
		dy0 = mv->cy - mSrc->origin.y;
	}

	MAP_ModBegin(mDst);
	for (sy = sy0, dy = dy0;
	     sy <= sy1 && dy < (int)mDst->maph;
	     sy++, dy++) {
		for (sx = sx0, dx = dx0;
		     sx <= sx1 && dx < (int)mDst->mapw;
		     sx++, dx++) {
			MAP_Node *sn, *dn;
			MAP_Item *r1, *r2;

			if (dx < 0 || dx >= (int)mDst->mapw ||
			    dy < 0 || dy >= (int)mDst->maph) {
				continue;
			}
			sn = &mSrc->map[sy][sx]; 
			dn = &mDst->map[dy][dx];
			
			MAP_ModNodeChg(mDst, dx, dy);

			if (ins->replace_mode) {
				MAP_NodeRemoveAll(mDst, dn, mDst->cur_layer);
			}
			TAILQ_FOREACH(r1, &sn->nrefs, nrefs) {
				r2 = MAP_NodeCopyItem(r1, mDst, dn, -1);
				r2->layer += mDst->cur_layer;
				while (r2->layer >= mDst->nLayers) {
					if (MAP_PushLayer(mDst, "") == 0)
						MAP_ModLayerAdd(mDst,
						    mDst->nLayers - 1);
				}
				if (ins->snap_mode == RG_SNAP_NONE) {
					r2->r_gfx.xcenter +=
					    mv->cxoffs*MAPTILESZ/AGMTILESZ(mv);
					r2->r_gfx.ycenter +=
					    mv->cyoffs*MAPTILESZ/AGMTILESZ(mv);
				}
			}
		}
	}
	MAP_ModEnd(mDst);
	return (1);
}

static int
Cursor(void *_Nonnull p, AG_Rect *_Nonnull rd)
{
	struct map_insert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	MAP *mSrc = &ins->mTmp;
	AG_TlistItem *it;
	RG_Tile *tile;
	AG_Rect r;
	AG_Color c;
	int sx0, sy0, sx1, sy1;
	int dx0, dy0;
	int dx, dy, sx, sy;

	if (mv->lib_tl == NULL ||
	   (it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	   strcmp(it->cat, "tile") != 0 ||
	   (tile = it->p1) == NULL || tile->su == NULL) {
		return (-1);
	}

	if (ins->snap_mode == RG_SNAP_NONE) {
		r.x = rd->x + 1;
		r.y = rd->y + 1;
		r.w = AGMTILESZ(mv) - 1;
		r.h = AGMTILESZ(mv) - 1;

		AG_ColorRGB_8(&c, 200,200,200);
		AG_DrawRectOutline(mv, &r, &c);
	}
	
	if (ins->mvTmp->esel.set) {
		sx0 = ins->mvTmp->esel.x;
		sy0 = ins->mvTmp->esel.y;
		sx1 = sx0 + ins->mvTmp->esel.w - 1;
		sy1 = sy0 + ins->mvTmp->esel.h - 1;
		dx0 = WIDGET(mv)->rView.x1 + rd->x;
		dy0 = WIDGET(mv)->rView.y1 + rd->y;
	} else {
		sx0 = 0;
		sy0 = 0;
		sx1 = mSrc->mapw-1;
		sy1 = mSrc->maph-1;
		dx0 = WIDGET(mv)->rView.x1+rd->x - mSrc->origin.x*AGMTILESZ(mv);
		dy0 = WIDGET(mv)->rView.y1+rd->y - mSrc->origin.y*AGMTILESZ(mv);
	}
	if (ins->snap_mode == RG_SNAP_NONE) {
		dx0 += mv->cxoffs*MAPTILESZ/AGMTILESZ(mv);
		dy0 += mv->cyoffs*MAPTILESZ/AGMTILESZ(mv);
	}
	for (sy = sy0, dy = dy0; sy <= sy1; sy++, dy += AGMTILESZ(mv)) {
		for (sx = sx0, dx = dx0; sx <= sx1; sx++, dx += AGMTILESZ(mv)) {
			MAP_Node *sn = &mSrc->map[sy][sx];
			MAP_Item *r;

			TAILQ_FOREACH(r, &sn->nrefs, nrefs)
				MAP_ItemDraw(mv->map, r, dx, dy, mv->cam);
		}
	}
	return (0);
}

static int
MouseButtonDown(void *_Nonnull p, int x, int y, int btn)
{
	struct map_insert_tool *ins = p;

	if (btn == AG_MOUSE_MIDDLE) {
		ins->angle = (ins->angle + 90) % 360;
		return (1);
	}
	return (0);
}

static int
MouseMotion(void *_Nonnull p, int x, int y, int xrel, int yrel, int btn)
{
	struct map_insert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	int nx = x/AGMTILESZ(mv);
	int ny = y/AGMTILESZ(mv);
	AG_TlistItem *it;

	if (nx == mv->mouse.x && ny == mv->mouse.y)
		return (0);

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	if (strcmp(it->cat, "tile") == 0) {
		MAP_ToolSetStatus(ins,
		    _("Insert %s tile at [%d,%d] ($(L)=Confirm, $(M)=Rotate)."),
		    it->text, mv->cx, mv->cy);
	}
	return (0);
}

const MAP_ToolOps mapInsertOps = {
	"Insert", N_("Insert node element"),
	&mapIconStamp,
	sizeof(struct map_insert_tool),
	TOOL_HIDDEN,
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

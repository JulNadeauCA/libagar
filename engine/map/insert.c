/*	$Csoft: insert.c,v 1.13 2005/08/29 05:27:28 vedge Exp $	*/

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
#include <engine/widget/separator.h>
#include <engine/widget/notebook.h>
#include <engine/widget/combo.h>

#include "map.h"
#include "mapedit.h"
#include "insert.h"
#include "tools.h"

static void
insert_init(void *p)
{
	struct ag_map_insert_tool *ins = p;

	ins->snap_mode = AG_GFX_SNAP_NOT;
	ins->replace_mode = 0;
	ins->angle = 0;
	AG_MapInit(&ins->mTmp, "tmp");
	ins->mvTmp = NULL;

	AG_MaptoolPushStatus(ins,
	    _("Select position on map ($(L)=Insert, $(M)=Rotate)"));
}

static void
insert_destroy(void *p)
{
	struct ag_map_insert_tool *ins = p;

	map_destroy(&ins->mTmp);
}

static void
snap_sprite(struct ag_map_insert_tool *ins, AG_Nitem *r, AG_Sprite *spr)
{
	AG_Mapview *mv = TOOL(ins)->mv;

	switch (ins->snap_mode) {
	case AG_GFX_SNAP_NOT:
		r->r_gfx.xcenter += mv->cxoffs*AGTILESZ/AGMTILESZ(mv);
		r->r_gfx.ycenter += mv->cyoffs*AGTILESZ/AGMTILESZ(mv);
		break;
	default:
		break;
	}

}

static void
generate_map(struct ag_map_insert_tool *ins, AG_Sprite *spr)
{
	int sy, sx, dx, dy;
	int sw = spr->su->w/AGTILESZ;
	int sh = spr->su->h/AGTILESZ;
	int nw, nh;

	if (spr->su->w%AGTILESZ > 0) sw++;
	if (spr->su->h%AGTILESZ > 0) sh++;

	AG_MapAllocNodes(&ins->mTmp, sw, sh);
	ins->mTmp.origin.x = spr->xOrig/AGTILESZ;
	ins->mTmp.origin.y = spr->yOrig/AGTILESZ;
	for (sy = 0, dy = 0;
	     sy < spr->su->h;
	     sy += AGTILESZ, dy++) {
		for (sx = 0, dx = 0;
		     sx < spr->su->w;
		     sx += AGTILESZ, dx++) {
			AG_Node *dn;
			AG_Nitem *r;
			int dw, dh, nlayer;

			if (dx >= ins->mTmp.mapw ||
			    dy >= ins->mTmp.maph)
				continue;

			dn = &ins->mTmp.map[dy][dx];
			dw = spr->su->w - sx;
			dh = spr->su->h - sy;

			r = Malloc(sizeof(AG_Nitem), M_MAP_NITEM);
			AG_NitemInit(r, AG_NITEM_SPRITE);
			AG_NitemSetSprite(r, &ins->mTmp, spr->pgfx->pobj,
			    spr->index);

			r->r_gfx.rs.x = dx*AGTILESZ;
			r->r_gfx.rs.y = dy*AGTILESZ;
			r->r_gfx.rs.w = (dw >= AGTILESZ) ? AGTILESZ : dw;
			r->r_gfx.rs.h = (dh >= AGTILESZ) ? AGTILESZ : dh;
			r->flags |= AG_SPRITE_ATTR2(spr,dx,dy);

			nlayer = AG_SPRITE_LAYER2(spr,dx,dy);
			if (nlayer < 0) {
				nlayer = 0;
			} else {
				if (nlayer >= ins->mTmp.nlayers)
					AG_MapPushLayer(&ins->mTmp, "");
			}
			r->layer = nlayer;
			
			/* XXX also need to rotate the whole map */
	//		AG_TransformRotate(r, ins->angle);

			TAILQ_INSERT_TAIL(&dn->nrefs, r, nrefs);
		}
	}
}

static void
insert_pane(void *p, void *con)
{
	struct ag_map_insert_tool *ins = p;
	AG_Radio *rad;
	AG_Checkbox *cb;
	AG_Spinbutton *sb;
	AG_Combo *com;
	AG_TlistItem *it;
	AG_Mapview *mvMain = TOOL(ins)->mv;
	AG_Mapview *mv;
	AG_Sprite *spr;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	if ((it = AG_TlistSelectedItem(mvMain->lib_tl)) == NULL ||
	     strcmp(it->class, "tile") != 0) {
		return;
	}
	spr = it->p1;
	
	nb = AG_NotebookNew(con, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_WFILL);
	ntab = AG_NotebookAddTab(nb, _("Tiles"), AG_BOX_VERT);
	mv = ins->mvTmp = AG_MapviewNew(ntab, &ins->mTmp,
	    AG_MAPVIEW_EDIT|AG_MAPVIEW_GRID, NULL, NULL);
	AG_MapviewPrescale(mv, 7, 7);
	AG_MapviewSelectTool(mv,
	    AG_MapviewRegTool(mv, &agMapNodeselOps, &ins->mTmp),
	    &ins->mTmp);
	generate_map(ins, spr);
	
	ntab = AG_NotebookAddTab(nb, _("Settings"), AG_BOX_VERT);
	{
		AG_LabelNew(ntab, AG_LABEL_STATIC, _("Snap to: "));
		rad = AG_RadioNew(ntab, agGfxSnapNames);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &ins->snap_mode);

		cb = AG_CheckboxNew(ntab, _("Replace mode"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &ins->replace_mode);

		sb = AG_SpinbuttonNew(ntab, _("Rotation: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &ins->angle);
		AG_SpinbuttonSetRange(sb, 0, 360);
		AG_SpinbuttonSetIncrement(sb, 90);
	}
}

static int
insert_effect(void *p, AG_Node *node)
{
	struct ag_map_insert_tool *ins = p;
	AG_Mapview *mv = TOOL(ins)->mv;
	AG_Map *mSrc = &ins->mTmp;
	AG_Map *mDst = mv->map;
	int sx, sy, sx0, sy0, sx1, sy1;
	int dx, dy, dx0, dy0;
	int l;
	AG_TlistItem *it;
	AG_Sprite *spr;
	
	if (mv->lib_tl == NULL ||
	    (it = AG_TlistSelectedItem(mv->lib_tl)) == 0 ||
	    strcmp(it->class, "tile") != 0) {
		return (1);
	}
	spr = it->p1;

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

	AG_MapmodBegin(mDst);
	for (sy = sy0, dy = dy0;
	     sy <= sy1 && dy < mDst->maph;
	     sy++, dy++) {
		for (sx = sx0, dx = dx0;
		     sx <= sx1 && dx < mDst->mapw;
		     sx++, dx++) {
			AG_Node *sn, *dn;
			AG_Nitem *r1, *r2;

			if (dx < 0 || dx >= mDst->mapw ||
			    dy < 0 || dy >= mDst->maph) {
				continue;
			}
			sn = &mSrc->map[sy][sx]; 
			dn = &mDst->map[dy][dx];
			
			AG_MapmodNodeChg(mDst, dx, dy);

			if (ins->replace_mode) {
				AG_NodeRemoveAll(mDst, dn, mDst->cur_layer);
			}
			TAILQ_FOREACH(r1, &sn->nrefs, nrefs) {
				r2 = AG_NodeCopyItem(r1, mDst, dn, -1);
				r2->layer += mDst->cur_layer;
				while (r2->layer >= mDst->nlayers) {
					if (AG_MapPushLayer(mDst, "") == 0)
						AG_MapmodLayerAdd(mDst,
						    mDst->nlayers - 1);
				}
				if (ins->snap_mode == AG_GFX_SNAP_NOT) {
					r2->r_gfx.xcenter +=
					    mv->cxoffs*AGTILESZ/AGMTILESZ(mv);
					r2->r_gfx.ycenter +=
					    mv->cyoffs*AGTILESZ/AGMTILESZ(mv);
				}
			}
		}
	}
	AG_MapmodEnd(mDst);
	return (1);
}

static int
insert_cursor(void *p, SDL_Rect *rd)
{
	struct ag_map_insert_tool *ins = p;
	AG_Mapview *mv = TOOL(ins)->mv;
	AG_Map *mSrc = &ins->mTmp;
	AG_TlistItem *it;
	AG_Sprite *spr;
	int sx0, sy0, sx1, sy1;
	int dx0, dy0, dx1, dy1;
	int dx, dy, sx, sy;

	if (mv->lib_tl == NULL ||
	   (it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	   strcmp(it->class, "tile") != 0 ||
	   (spr = it->p1) == NULL || spr->su == NULL) {
		return (-1);
	}

	if (ins->snap_mode == AG_GFX_SNAP_NOT) {
		agPrim.rect_outlined(mv, rd->x+1, rd->y+1,
		    AGMTILESZ(mv)-1, AGMTILESZ(mv)-1,
		    AG_COLOR(MAPVIEW_GRID_COLOR));
	}
	
	if (ins->mvTmp->esel.set) {
		sx0 = ins->mvTmp->esel.x;
		sy0 = ins->mvTmp->esel.y;
		sx1 = sx0 + ins->mvTmp->esel.w - 1;
		sy1 = sy0 + ins->mvTmp->esel.h - 1;
		dx0 = AGWIDGET(mv)->cx + rd->x;
		dy0 = AGWIDGET(mv)->cy + rd->y;
	} else {
		sx0 = 0;
		sy0 = 0;
		sx1 = mSrc->mapw-1;
		sy1 = mSrc->maph-1;
		dx0 = AGWIDGET(mv)->cx + rd->x - mSrc->origin.x*AGMTILESZ(mv);
		dy0 = AGWIDGET(mv)->cy + rd->y - mSrc->origin.y*AGMTILESZ(mv);
	}
	if (ins->snap_mode == AG_GFX_SNAP_NOT) {
		dx0 += mv->cxoffs*AGTILESZ/AGMTILESZ(mv);
		dy0 += mv->cyoffs*AGTILESZ/AGMTILESZ(mv);
	}
	for (sy = sy0, dy = dy0; sy <= sy1; sy++, dy += AGMTILESZ(mv)) {
		for (sx = sx0, dx = dx0; sx <= sx1; sx++, dx += AGMTILESZ(mv)) {
			AG_Node *sn = &mSrc->map[sy][sx];
			AG_Nitem *r;

			TAILQ_FOREACH(r, &sn->nrefs, nrefs)
				AG_NitemDraw(mv->map, r, dx, dy, mv->cam);
		}
	}
	return (0);
}

static int
insert_mousebuttondown(void *p, int x, int y, int btn)
{
	struct ag_map_insert_tool *ins = p;

	if (btn == SDL_BUTTON_MIDDLE) {
		ins->angle = (ins->angle + 90) % 360;
		return (1);
	}
	return (0);
}

static int
insert_mousemotion(void *p, int x, int y, int xrel, int yrel, int btn)
{
	struct ag_map_insert_tool *ins = p;
	AG_Mapview *mv = TOOL(ins)->mv;
	int nx = x/AGMTILESZ(mv);
	int ny = y/AGMTILESZ(mv);
	AG_TlistItem *it;
	AG_Object *ob;

	if (nx == mv->mouse.x && ny == mv->mouse.y)
		return (0);

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	if (strcmp(it->class, "tile") == 0) {
		AG_MaptoolSetStatus(ins,
		    _("Insert %s tile at [%d,%d] ($(L)=Confirm, $(M)=Rotate)."),
		    it->text, mv->cx, mv->cy);
	}
	return (0);
}

const AG_MaptoolOps agMapInsertOps = {
	"Insert", N_("Insert node element"),
	STAMP_TOOL_ICON,
	sizeof(struct ag_map_insert_tool),
	TOOL_HIDDEN,
	insert_init,
	insert_destroy,
	insert_pane,
	NULL,				/* edit */
	insert_cursor,
	insert_effect,

	insert_mousemotion,
	insert_mousebuttondown,
	NULL,				/* mousebuttonup */
	NULL,				/* keydown */
	NULL				/* keyup */
};

#endif /* MAP */

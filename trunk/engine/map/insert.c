/*	$Csoft: insert.c,v 1.11 2005/08/27 04:34:05 vedge Exp $	*/

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

#include "map.h"
#include "mapedit.h"
#include "insert.h"
#include "tools.h"

static void
insert_init(void *p)
{
	struct insert_tool *ins = p;

	ins->snap_mode = GFX_SNAP_NOT;
	ins->replace_mode = 0;
	ins->angle = 0;
	map_init(&ins->mTmp, "tmp");
	ins->mvTmp = NULL;

	tool_push_status(ins,
	    _("Select position on map ($(L)=Insert, $(M)=Rotate)"));
}

static void
insert_destroy(void *p)
{
	struct insert_tool *ins = p;

	map_destroy(&ins->mTmp);
}

static void
snap_sprite(struct insert_tool *ins, struct noderef *r, struct sprite *spr)
{
	struct mapview *mv = TOOL(ins)->mv;

	switch (ins->snap_mode) {
	case GFX_SNAP_NOT:
		r->r_gfx.xcenter += mv->cxoffs*TILESZ/MV_TILESZ(mv);
		r->r_gfx.ycenter += mv->cyoffs*TILESZ/MV_TILESZ(mv);
		break;
	default:
		break;
	}

}

static void
generate_map(struct insert_tool *ins, struct sprite *spr)
{
	int sy, sx, dx, dy;
	int xorig, yorig;
	int sw = spr->su->w/TILESZ;
	int sh = spr->su->h/TILESZ;
	int nw, nh;

	if (spr->su->w%TILESZ > 0) sw++;
	if (spr->su->h%TILESZ > 0) sh++;

	map_alloc_nodes(&ins->mTmp, sw, sh);
	ins->mTmp.origin.x = spr->xOrig/TILESZ;
	ins->mTmp.origin.y = spr->yOrig/TILESZ;
	xorig = spr->xOrig%TILESZ;
	yorig = spr->yOrig%TILESZ;
	for (sy = 0, dy = 0;
	     sy < spr->su->h;
	     sy += TILESZ, dy++) {
		for (sx = 0, dx = 0;
		     sx < spr->su->w;
		     sx += TILESZ, dx++) {
			struct node *dn;
			struct noderef *r;
			int dw, dh, nlayer;

			if (dx >= ins->mTmp.mapw ||
			    dy >= ins->mTmp.maph)
				continue;

			dn = &ins->mTmp.map[dy][dx];
			dw = spr->su->w - sx;
			dh = spr->su->h - sy;

			r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
			noderef_init(r, NODEREF_SPRITE);
			noderef_set_sprite(r, &ins->mTmp, spr->pgfx->pobj,
			    spr->index);

			r->r_gfx.rs.x = dx*TILESZ;
			r->r_gfx.rs.y = dy*TILESZ;
			r->r_gfx.rs.w = (dw >= TILESZ) ? TILESZ : dw;
			r->r_gfx.rs.h = (dh >= TILESZ) ? TILESZ : dh;
			r->r_gfx.xorigin = xorig;
			r->r_gfx.yorigin = yorig;
			r->flags |= SPRITE_ATTR2(spr,dx,dy);

			nlayer = SPRITE_LAYER2(spr,dx,dy);
			if (nlayer < 0) {
				nlayer = 0;
			} else {
				if (nlayer >= ins->mTmp.nlayers)
					map_push_layer(&ins->mTmp, "");
			}
			r->layer = nlayer;
			
			/* XXX also need to rotate the whole map */
	//		transform_rotate(r, ins->angle);

			TAILQ_INSERT_TAIL(&dn->nrefs, r, nrefs);
		}
	}
}

static void
insert_pane(void *p, void *con)
{
	struct insert_tool *ins = p;
	struct radio *rad;
	struct checkbox *cb;
	struct spinbutton *sb;
	struct combo *com;
	struct tlist_item *it;
	struct mapview *mvMain = TOOL(ins)->mv;
	struct mapview *mv;
	struct sprite *spr;
	struct notebook *nb;
	struct notebook_tab *ntab;

	if ((it = tlist_selected_item(mvMain->lib_tl)) == NULL ||
	     strcmp(it->class, "tile") != 0) {
		return;
	}
	spr = it->p1;
	
	nb = notebook_new(con, NOTEBOOK_HFILL|NOTEBOOK_WFILL);
	ntab = notebook_add_tab(nb, _("Tiles"), BOX_VERT);
	mv = ins->mvTmp = mapview_new(ntab, &ins->mTmp,
	    MAPVIEW_EDIT|MAPVIEW_GRID, NULL, NULL);
	mapview_prescale(mv, 7, 7);
	mapview_select_tool(mv,
	    mapview_reg_tool(mv, &nodesel_ops, &ins->mTmp),
	    &ins->mTmp);
	generate_map(ins, spr);
	
	ntab = notebook_add_tab(nb, _("Settings"), BOX_VERT);
	{
		label_new(ntab, LABEL_STATIC, _("Snap to: "));
		rad = radio_new(ntab, gfx_snap_names);
		widget_bind(rad, "value", WIDGET_INT, &ins->snap_mode);

		cb = checkbox_new(ntab, _("Replace mode"));
		widget_bind(cb, "state", WIDGET_INT, &ins->replace_mode);

		sb = spinbutton_new(ntab, _("Rotation: "));
		widget_bind(sb, "value", WIDGET_INT, &ins->angle);
		spinbutton_set_range(sb, 0, 360);
		spinbutton_set_increment(sb, 90);
	}
}

static int
insert_effect(void *p, struct node *node)
{
	struct insert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	struct map *mSrc = &ins->mTmp;
	struct map *mDst = mv->map;
	int sx, sy, sx0, sy0, sx1, sy1;
	int dx, dy, dx0, dy0;
	int l;
	struct tlist_item *it;
	struct sprite *spr;
	
	if (mv->lib_tl == NULL ||
	    (it = tlist_selected_item(mv->lib_tl)) == 0 ||
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

	mapmod_begin(mDst);
	for (sy = sy0, dy = dy0;
	     sy <= sy1 && dy < mDst->maph;
	     sy++, dy++) {
		for (sx = sx0, dx = dx0;
		     sx <= sx1 && dx < mDst->mapw;
		     sx++, dx++) {
			struct node *sn, *dn;
			struct noderef *r1, *r2;

			if (dx < 0 || dx >= mDst->mapw ||
			    dy < 0 || dy >= mDst->maph) {
				continue;
			}
			sn = &mSrc->map[sy][sx]; 
			dn = &mDst->map[dy][dx];
			
			mapmod_nodechg(mDst, dx, dy);

			if (ins->replace_mode) {
				node_clear(mDst, dn, mDst->cur_layer);
			}
			TAILQ_FOREACH(r1, &sn->nrefs, nrefs) {
				r2 = node_copy_ref(r1, mDst, dn, -1);
				r2->layer += mDst->cur_layer;
				while (r2->layer >= mDst->nlayers) {
					if (map_push_layer(mDst, "") == 0)
						mapmod_layeradd(mDst,
						    mDst->nlayers - 1);
				}
				if (ins->snap_mode == GFX_SNAP_NOT) {
					r2->r_gfx.xcenter +=
					    mv->cxoffs*TILESZ/MV_TILESZ(mv);
					r2->r_gfx.ycenter +=
					    mv->cyoffs*TILESZ/MV_TILESZ(mv);
				}
			}
		}
	}
	mapmod_end(mDst);
	return (1);
}

static int
insert_cursor(void *p, SDL_Rect *rd)
{
	struct insert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	struct map *mSrc = &ins->mTmp;
	struct tlist_item *it;
	struct sprite *spr;
	int sx0, sy0, sx1, sy1;
	int dx0, dy0, dx1, dy1;
	int dx, dy, sx, sy;

	if (mv->lib_tl == NULL ||
	   (it = tlist_selected_item(mv->lib_tl)) == NULL ||
	   strcmp(it->class, "tile") != 0 ||
	   (spr = it->p1) == NULL || spr->su == NULL) {
		return (-1);
	}

	if (ins->snap_mode == GFX_SNAP_NOT) {
		primitives.rect_outlined(mv, rd->x+1, rd->y+1,
		    MV_TILESZ(mv)-1, MV_TILESZ(mv)-1,
		    COLOR(MAPVIEW_GRID_COLOR));
	}
	
	if (ins->mvTmp->esel.set) {
		sx0 = ins->mvTmp->esel.x;
		sy0 = ins->mvTmp->esel.y;
		sx1 = sx0 + ins->mvTmp->esel.w - 1;
		sy1 = sy0 + ins->mvTmp->esel.h - 1;
		dx0 = WIDGET(mv)->cx + rd->x;
		dy0 = WIDGET(mv)->cy + rd->y;
	} else {
		sx0 = 0;
		sy0 = 0;
		sx1 = mSrc->mapw-1;
		sy1 = mSrc->maph-1;
		dx0 = WIDGET(mv)->cx + rd->x - mSrc->origin.x*MV_TILESZ(mv);
		dy0 = WIDGET(mv)->cy + rd->y - mSrc->origin.y*MV_TILESZ(mv);
	}
	if (ins->snap_mode == GFX_SNAP_NOT) {
		dx0 += mv->cxoffs*TILESZ/MV_TILESZ(mv);
		dy0 += mv->cyoffs*TILESZ/MV_TILESZ(mv);
	}
	for (sy = sy0, dy = dy0; sy <= sy1; sy++, dy += MV_TILESZ(mv)) {
		for (sx = sx0, dx = dx0; sx <= sx1; sx++, dx += MV_TILESZ(mv)) {
			struct node *sn = &mSrc->map[sy][sx];
			struct noderef *r;

			TAILQ_FOREACH(r, &sn->nrefs, nrefs)
				noderef_draw(mv->map, r, dx, dy, mv->cam);
		}
	}
	return (0);
}

static int
insert_mousebuttondown(void *p, int x, int y, int btn)
{
	struct insert_tool *ins = p;

	if (btn == SDL_BUTTON_MIDDLE) {
		ins->angle = (ins->angle + 90) % 360;
		return (1);
	}
	return (0);
}

static int
insert_mousemotion(void *p, int x, int y, int xrel, int yrel, int btn)
{
	struct insert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	int nx = x/MV_TILESZ(mv);
	int ny = y/MV_TILESZ(mv);
	struct tlist_item *it;
	struct object *ob;

	if (nx == mv->mouse.x && ny == mv->mouse.y)
		return (0);

	if ((it = tlist_selected_item(mv->lib_tl)) == NULL ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	if (strcmp(it->class, "tile") == 0) {
		tool_set_status(ins,
		    _("Insert %s tile at [%d,%d] ($(L)=Confirm, $(M)=Rotate)."),
		    it->text, mv->cx, mv->cy);
	}
	return (0);
}

const struct tool_ops insert_ops = {
	"Insert", N_("Insert node element"),
	STAMP_TOOL_ICON,
	sizeof(struct insert_tool),
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

/*	$Csoft: vg_block.c,v 1.16 2005/09/27 00:25:20 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/textbox.h>

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

/* Create a new block and select it for edition. */
VG_Block *
VG_BeginBlock(VG *vg, const char *name, int flags)
{
	VG_Block *vgb;

	vgb = Malloc(sizeof(VG_Block), M_VG);
	strlcpy(vgb->name, name, sizeof(vgb->name));
	vgb->flags = flags;
	vgb->pos.x = 0;
	vgb->pos.y = 0;
	vgb->pos.z = 0;
	vgb->origin.x = 0;
	vgb->origin.y = 0;
	vgb->origin.z = 0;
	vgb->theta = 0;
	vgb->selected = 0;
	TAILQ_INIT(&vgb->vges);

	vg->cur_block = vgb;
	TAILQ_INSERT_HEAD(&vg->blocks, vgb, vgbs);
	return (vgb);
}

void
VG_SelectBlock(VG *vg, VG_Block *vgb)
{
	vg->cur_block = vgb;
}

/* Finish the current block. */
void
VG_EndBlock(VG *vg)
{
	vg->cur_block = NULL;
}

/* Look up the named block. */
VG_Block *
VG_GetBlock(VG *vg, const char *name)
{
	VG_Block *vgb;

	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		if (strcmp(vgb->name, name) == 0)
			break;
	}
	return (vgb);
}

/* Displace the elements associated with a block. */
void
VG_MoveBlock(VG *vg, VG_Block *vgb, double x, double y,
    int layer)
{
	VG_Element *vge;
	int i;

	TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
		for (i = 0; i < vge->nvtx; i++) {
			vge->vtx[i].x -= vgb->pos.x - x;
			vge->vtx[i].y -= vgb->pos.y - y;
		}
		if (layer != -1)
			vge->layer = layer;
	}
	
	vgb->pos.x = x;
	vgb->pos.y = y;
	vg->redraw++;
}

/* Modify a block's angle of rotation. */
void
VG_BlockTheta(VG *vg, VG_Block *vgb, double theta)
{
	vgb->theta = theta;
}

/* Apply a rotation transformation on a block. */
void
VG_RotateBlock(VG *vg, VG_Block *vgb, double theta)
{
	VG_Block *block_save;
	VG_Element *vge;
	Uint32 i;

	block_save = vg->cur_block;
	VG_SelectBlock(vg, vgb);

	TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
		for (i = 0; i < vge->nvtx; i++) {
			double r, theta;
			double x, y;

			VG_Abs2Rel(vg, &vge->vtx[i], &x, &y);
			VG_Car2Pol(vg, x, y, &r, &theta);
			theta += vgb->theta;
			VG_Pol2Car(vg, r, theta, &x, &y);
			VG_Rel2Abs(vg, x, y, &vge->vtx[i]);
		}
	}
	VG_SelectBlock(vg, block_save);
	vg->redraw++;
}

/* Calculate the collective extent of the elements in a block. */
void
VG_BlockExtent(VG *vg, VG_Block *vgb, VG_Rect *ext)
{
	double xmin = vgb->pos.x, xmax = vgb->pos.x;
	double ymin = vgb->pos.y, ymax = vgb->pos.y;
	VG_Element *vge;
	VG_Rect r;

	TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
		if (vge->ops->bbox == NULL)
			continue;

		vge->ops->bbox(vg, vge, &r);

		if (r.x < xmin) { xmin = r.x; }
		if (r.y < ymin) { ymin = r.y; }
		if (r.x+r.w > xmax) { xmax = r.x+r.w; }
		if (r.y+r.h > ymax) { ymax = r.y+r.h; }
	}
	ext->x = xmin;
	ext->y = ymin;
	ext->w = xmax-xmin;
	ext->h = ymax-ymin;
}

/* Convert absolute coordinates to block relative coordinates. */
void
VG_Abs2Rel(VG *vg, const VG_Vtx *vtx, double *x, double *y)
{
	*x = vtx->x;
	*y = vtx->y;

	if (vg->cur_block != NULL) {
		*x -= vg->cur_block->pos.x;
		*y -= vg->cur_block->pos.y;
	}
}

/* Convert block relative coordinates to absolute coordinates. */
void
VG_Rel2Abs(VG *vg, double x, double y, VG_Vtx *vtx)
{
	vtx->x = x;
	vtx->y = y;

	if (vg->cur_block != NULL) {
		vtx->x += vg->cur_block->pos.x;
		vtx->y += vg->cur_block->pos.y;
	}
}

/* Destroy a block as well as the elements associated with it. */
void
VG_DestroyBlock(VG *vg, VG_Block *vgb)
{
	VG_Element *vge, *nvge;

	for (vge = TAILQ_FIRST(&vgb->vges);
	     vge != TAILQ_END(&vgb->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vgbmbs);
		TAILQ_REMOVE(&vg->vges, vge, vges);
		VG_FreeElement(vg, vge);
	}
	TAILQ_REMOVE(&vg->blocks, vgb, vgbs);
	Free(vgb, M_VG);
	vg->redraw = 1;
}

/* Destroy all elements associated with a block. */
void
VG_ClearBlock(VG *vg, VG_Block *vgb)
{
	VG_Element *vge, *nvge;

	for (vge = TAILQ_FIRST(&vgb->vges);
	     vge != TAILQ_END(&vgb->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vgbmbs);
		TAILQ_REMOVE(&vg->vges, vge, vges);
		VG_FreeElement(vg, vge);
	}
	TAILQ_INIT(&vgb->vges);
	vg->redraw = 1;
}

/* Generate absolute vg coordinates for a vertex that's part of a block. */
void
VG_BlockOffset(VG *vg, VG_Vtx *vtx)
{
	if (vg->cur_block != NULL) {
		vtx->x += vg->cur_block->pos.x;
		vtx->y += vg->cur_block->pos.y;
	}
}

#ifdef EDITION

static void
destroy_block(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;
	AG_Tlist *tl = argv[2].p;
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		if (it->iconsrc == AGICON(VGBLOCK_ICON)) {
			VG_DestroyBlock(vg, (VG_Block *)it->p1);
		} else if (it->iconsrc == AGICON(DRAWING_ICON)) {
			VG_DestroyElement(vg, (VG_Element *)it->p1);
		}
	}
}

static void
poll_blocks(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	VG *vg = argv[1].p;
	VG_Block *vgb;
	VG_Element *vge;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	pthread_mutex_lock(&vg->lock);

	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		char name[VG_BLOCK_NAME_MAX];
		VG_Rect rext;

		VG_BlockExtent(vg, vgb, &rext);
		snprintf(name, sizeof(name),
		    "%s (%.2f,%.2f; \xce\xb8=%.2f; ext=%.2f,%.2f %.2fx%.2f)",
		    vgb->name, vgb->pos.x, vgb->pos.y, vgb->theta,
		    rext.x, rext.y, rext.w, rext.h);
		it = AG_TlistAddPtr(tl, AGICON(VGBLOCK_ICON), name, vgb);
		it->depth = 0;
		TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
			it = AG_TlistAddPtr(tl, AGICON(DRAWING_ICON),
			    _(vge->ops->name), vge);
			it->depth = 1;
		}
	}
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		it = AG_TlistAddPtr(tl, AGICON(DRAWING_ICON),
		    _(vge->ops->name), vge);
		it->depth = 1;
	}

	pthread_mutex_unlock(&vg->lock);
	AG_TlistRestore(tl);
}

AG_Window *
VG_BlockEditor(VG *vg)
{
	AG_Window *win;
	AG_Box *bo;
	AG_Tlist *tl;

	win = AG_WindowNew(AG_WINDOW_HIDE, NULL);
	AG_WindowSetCaption(win, _("Blocks"));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 0);
	
	tl = AG_TlistNew(win, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE);
	AG_SetEvent(tl, "tlist-poll", poll_blocks, "%p", vg);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonAct(bo, _("Destroy"), 0,
		    destroy_block, "%p,%p", vg, tl);
	}
	return (win);
}
#endif /* EDITION */

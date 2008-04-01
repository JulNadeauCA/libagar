/*
 * Copyright (c) 2004-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Block element. Associated with blocks are groups of entities that remain
 * rigid under translation and rotation.
 */

#include <core/limits.h>
#include <core/core.h>

#include <gui/window.h>
#include <gui/tlist.h>
#include <gui/toolbar.h>
#include <gui/textbox.h>

#include "vg.h"
#include "vg_math.h"
#include "vg_view.h"
#include "icons.h"

#include <string.h>

/* Create a new block and select it for edition. */
VG_Block *
VG_BeginBlock(VG *vg, const char *name, int flags)
{
	VG_Block *vgb;

	vgb = Malloc(sizeof(VG_Block));
	Strlcpy(vgb->name, name, sizeof(vgb->name));
	vgb->flags = flags;
	vgb->pos.x = 0.0f;
	vgb->pos.y = 0.0f;
	vgb->theta = 0.0f;
	vgb->selected = 0;
	TAILQ_INIT(&vgb->nodes);

	VG_Lock(vg);
	vg->curBlock = vgb;
	TAILQ_INSERT_HEAD(&vg->blocks, vgb, vgbs);
	return (vgb);
}

void
VG_SelectBlock(VG *vg, VG_Block *vgb)
{
	VG_Lock(vg);
	vg->curBlock = vgb;
}

/* Finish the current block. */
void
VG_EndBlock(VG *vg)
{
	vg->curBlock = NULL;
	VG_Unlock(vg);
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
VG_MoveBlock(VG *vg, VG_Block *vgb, float x, float y, int layer)
{
	VG_Node *vge;
	int i;

	TAILQ_FOREACH(vge, &vgb->nodes, vgbmbs) {
		for (i = 0; i < vge->nvtx; i++) {
			vge->vtx[i].x -= vgb->pos.x - x;
			vge->vtx[i].y -= vgb->pos.y - y;
		}
		if (layer != -1)
			vge->layer = layer;
	}
	
	vgb->pos.x = x;
	vgb->pos.y = y;
}

/* Modify a block's angle of rotation. */
void
VG_BlockTheta(VG *vg, VG_Block *vgb, float theta)
{
	vgb->theta = theta;
}

/* Apply a rotation transformation on a block. */
void
VG_RotateBlock(VG *vg, VG_Block *vgb, float theta)
{
	VG_Block *block_save;
	VG_Node *vge;
	Uint32 i;

	block_save = vg->curBlock;
	VG_SelectBlock(vg, vgb);

	TAILQ_FOREACH(vge, &vgb->nodes, vgbmbs) {
		for (i = 0; i < vge->nvtx; i++) {
			float x, y, r, theta;

			VG_Abs2Rel(vg, &vge->vtx[i], &x, &y);
			r = Sqrt(x*x + y*y);
			theta = Atan2(y,x) + vgb->theta;
			x = r*Cos(theta);
			y = r*Sin(theta);
			VG_Rel2Abs(vg, x, y, &vge->vtx[i]);
		}
	}
	VG_SelectBlock(vg, block_save);
}

/* Calculate the collective extent of the elements in a block. */
void
VG_BlockExtent(VG_View *vv, VG_Block *vgb, VG_Rect *ext)
{
	float xmax = vgb->pos.x;
	float ymax = vgb->pos.y;
	VG_Node *vn;
	VG_Rect r;

	ext->x = vgb->pos.x;
	ext->y = vgb->pos.y;
	TAILQ_FOREACH(vn, &vgb->nodes, vgbmbs) {
		if (vn->ops->extent == NULL) {
			continue;
		}
		vn->ops->extent(vv, vn, &r);
		if (r.x < ext->x) { ext->x = r.x; }
		if (r.y < ext->y) { ext->y = r.y; }
		if (r.x+r.w > xmax) { xmax = r.x+r.w; }
		if (r.y+r.h > ymax) { ymax = r.y+r.h; }
	}
	ext->w = xmax - ext->x;
	ext->h = ymax - ext->y;
}

/* Convert absolute coordinates to block relative coordinates. */
void
VG_Abs2Rel(VG *vg, const VG_Vtx *vtx, float *x, float *y)
{
	*x = vtx->x;
	*y = vtx->y;

	if (vg->curBlock != NULL) {
		*x -= vg->curBlock->pos.x;
		*y -= vg->curBlock->pos.y;
	}
}

/* Convert block relative coordinates to absolute coordinates. */
void
VG_Rel2Abs(VG *vg, float x, float y, VG_Vtx *vtx)
{
	vtx->x = x;
	vtx->y = y;

	if (vg->curBlock != NULL) {
		vtx->x += vg->curBlock->pos.x;
		vtx->y += vg->curBlock->pos.y;
	}
}

/* Destroy a block as well as the elements associated with it. */
void
VG_DestroyBlock(VG *vg, VG_Block *vgb)
{
	VG_Node *vge, *nvge;

	for (vge = TAILQ_FIRST(&vgb->nodes);
	     vge != TAILQ_END(&vgb->nodes);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vgbmbs);
		TAILQ_REMOVE(&vg->nodes, vge, nodes);
		VG_FreeNode(vg, vge);
	}
	TAILQ_REMOVE(&vg->blocks, vgb, vgbs);
	Free(vgb);
}

/* Destroy all elements associated with a block. */
void
VG_ClearBlock(VG *vg, VG_Block *vgb)
{
	VG_Node *vge, *nvge;

	for (vge = TAILQ_FIRST(&vgb->nodes);
	     vge != TAILQ_END(&vgb->nodes);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vgbmbs);
		TAILQ_REMOVE(&vg->nodes, vge, nodes);
		VG_FreeNode(vg, vge);
	}
	TAILQ_INIT(&vgb->nodes);
}

/* Return the block closest to the given coordinates. */
VG_Block *
VG_BlockClosest(VG *vg, float x, float y)
{
	VG_Node *vge;
	float closest_idx = AG_FLT_MAX, idx;
	VG_Node *closest_vge = NULL;
	float ix, iy;

	TAILQ_FOREACH(vge, &vg->nodes, nodes) {
		if (vge->ops->intsect == NULL || vge->block == NULL) {
			continue;
		}
		ix = x;
		iy = y;
		idx = vge->ops->intsect(vg, vge, &ix, &iy);
		if (idx < closest_idx) {
			closest_idx = idx;
			closest_vge = vge;
		}
	}
	return (closest_vge != NULL ? closest_vge->block : NULL);
}

static void
DestroyBlock(AG_Event *event)
{
	VG *vg = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		if (it->iconsrc == vgIconBlock.s) {
			VG_DestroyBlock(vg, (VG_Block *)it->p1);
		} else if (it->iconsrc == vgIconDrawing.s) {
			VG_DestroyNode(vg, (VG_Node *)it->p1);
		}
	}
}

static void
PollBlocks(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	VG *vg = AG_PTR(1);
	VG_Block *vgb;
	VG_Node *vge;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	VG_Lock(vg);

	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		char name[VG_BLOCK_NAME_MAX];

		Snprintf(name, sizeof(name),
		    "%s (%.2f,%.2f; \xce\xb8=%.2f)",
		    vgb->name, vgb->pos.x, vgb->pos.y, vgb->theta);
		it = AG_TlistAddPtr(tl, vgIconBlock.s, name, vgb);
		it->depth = 0;
		TAILQ_FOREACH(vge, &vgb->nodes, vgbmbs) {
			it = AG_TlistAddPtr(tl, vgIconDrawing.s,
			    _(vge->ops->name), vge);
			it->depth = 1;
		}
	}
	TAILQ_FOREACH(vge, &vg->nodes, nodes) {
		it = AG_TlistAddPtr(tl, vgIconDrawing.s, _(vge->ops->name),
		    vge);
		it->depth = 1;
	}

	VG_Unlock(vg);
	AG_TlistRestore(tl);
}

AG_Window *
VG_BlockEditor(VG *vg)
{
	AG_Window *win;
	AG_Box *bo;
	AG_Tlist *tl;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Blocks"));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_RIGHT, 0);
	AG_WindowSetCloseAction(win, AG_WINDOW_HIDE);
	
	tl = AG_TlistNew(win, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE|
	                      AG_TLIST_EXPAND);
	AG_SetEvent(tl, "tlist-poll", PollBlocks, "%p", vg);
	AG_WidgetFocus(tl);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(bo, 0, _("Destroy"), DestroyBlock, "%p,%p",
		    vg, tl);
	}
	return (win);
}

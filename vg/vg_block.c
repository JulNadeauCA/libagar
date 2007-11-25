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

#include <core/limits.h>
#include <core/core.h>

#include <gui/window.h>
#include <gui/tlist.h>
#include <gui/toolbar.h>
#include <gui/textbox.h>

#include "vg.h"
#include "vg_primitive.h"
#include "vg_math.h"
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
	vgb->pos.x = 0;
	vgb->pos.y = 0;
	vgb->origin.x = 0;
	vgb->origin.y = 0;
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
VG_MoveBlock(VG *vg, VG_Block *vgb, float x, float y, int layer)
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
	VG_Element *vge;
	Uint32 i;

	block_save = vg->cur_block;
	VG_SelectBlock(vg, vgb);

	TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
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
VG_BlockExtent(VG *vg, VG_Block *vgb, VG_Rect *ext)
{
	float xmin = vgb->pos.x, xmax = vgb->pos.x;
	float ymin = vgb->pos.y, ymax = vgb->pos.y;
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
VG_Abs2Rel(VG *vg, const VG_Vtx *vtx, float *x, float *y)
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
VG_Rel2Abs(VG *vg, float x, float y, VG_Vtx *vtx)
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
	Free(vgb);
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

/* Return the block closest to the given coordinates. */
VG_Block *
VG_BlockClosest(VG *vg, float x, float y)
{
	VG_Element *vge;
	float closest_idx = AG_FLT_MAX, idx;
	VG_Element *closest_vge = NULL;
	float ix, iy;
#if 0
	int o = VG_NORIGINS;
#endif

	TAILQ_FOREACH(vge, &vg->vges, vges) {
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
#if 0
		if (o >= vg->norigin) {
			VG_AddOrigin(vg, 0.0, 0.0, 0.125,
			    SDL_MapRGB(vg->fmt, 200, 0, 0));
		}
		VG_Origin(vg, o, vg->origin[0].x+ix, vg->origin[0].y+iy);
		o++;
#endif
	}
	return (closest_vge != NULL ? closest_vge->block : NULL);
}

static void
destroy_block(AG_Event *event)
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
			VG_DestroyElement(vg, (VG_Element *)it->p1);
		}
	}
}

static void
poll_blocks(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	VG *vg = AG_PTR(1);
	VG_Block *vgb;
	VG_Element *vge;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_MutexLock(&vg->lock);

	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		char name[VG_BLOCK_NAME_MAX];
		VG_Rect rext;

		VG_BlockExtent(vg, vgb, &rext);
		Snprintf(name, sizeof(name),
		    "%s (%.2f,%.2f; \xce\xb8=%.2f; ext=%.2f,%.2f %.2fx%.2f)",
		    vgb->name, vgb->pos.x, vgb->pos.y, vgb->theta,
		    rext.x, rext.y, rext.w, rext.h);
		it = AG_TlistAddPtr(tl, vgIconBlock.s, name, vgb);
		it->depth = 0;
		TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
			it = AG_TlistAddPtr(tl, vgIconDrawing.s,
			    _(vge->ops->name), vge);
			it->depth = 1;
		}
	}
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		it = AG_TlistAddPtr(tl, vgIconDrawing.s, _(vge->ops->name),
		    vge);
		it->depth = 1;
	}

	AG_MutexUnlock(&vg->lock);
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
	AG_SetEvent(tl, "tlist-poll", poll_blocks, "%p", vg);
	AG_WidgetFocus(tl);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(bo, 0, _("Destroy"), destroy_block, "%p,%p",
		    vg, tl);
	}
	return (win);
}

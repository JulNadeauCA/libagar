/*	$Csoft: vg_block.c,v 1.1 2004/04/30 05:24:26 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
struct vg_block *
vg_begin_block(struct vg *vg, const char *name)
{
	struct vg_block *vgb;

	vgb = Malloc(sizeof(struct vg_block), M_VG);
	strlcpy(vgb->name, name, sizeof(vgb->name));
	vgb->pos.x = 0;
	vgb->pos.y = 0;
	vgb->pos.z = 0;
	vgb->origin.x = 0;
	vgb->origin.y = 0;
	vgb->origin.z = 0;
	TAILQ_INIT(&vgb->vges);

	vg->cur_block = vgb;
	TAILQ_INSERT_HEAD(&vg->blocks, vgb, vgbs);
	return (vgb);
}

void
vg_end_block(struct vg *vg)
{
	vg->cur_block = NULL;
}

void
vg_move_block(struct vg *vg, struct vg_block *vgb, double x, double y,
    int layer)
{
	struct vg_element *vge;
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

void
vg_destroy_block(struct vg *vg, struct vg_block *vgb)
{
	struct vg_element *vge, *nvge;

	for (vge = TAILQ_FIRST(&vgb->vges);
	     vge != TAILQ_END(&vgb->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vgbmbs);
		vg_destroy_element(vg, vge);
	}
	TAILQ_REMOVE(&vg->blocks, vgb, vgbs);
	Free(vgb, M_VG);
}

static void
destroy_block(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct tlist_item *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		switch (it->text[0]) {
		case 'B':
			vg_destroy_block(vg, (struct vg_block *)it->p1);
			break;
		case 'E':
			vg_destroy_element(vg, (struct vg_element *)it->p1);
			break;
		}
	}
}

static void
poll_blocks(int argc, union evarg *argv)
{
	char name[TLIST_LABEL_MAX];
	struct tlist *tl = argv[0].p;
	struct vg *vg = argv[1].p;
	struct vg_block *vgb;
	struct vg_element *vge;
	struct tlist_item *it;

	tlist_clear_items(tl);
	pthread_mutex_lock(&vg->lock);

	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		strlcpy(name, "B ", sizeof(name));
		strlcat(name, _(vgb->name), sizeof(name));
		it = tlist_insert_item(tl, ICON(VGBLOCK_ICON), name, vgb);
		it->depth = 0;
		TAILQ_FOREACH(vge, &vgb->vges, vgbmbs) {
			strlcpy(name, "E ", sizeof(name));
			strlcat(name, _(vge->ops.name), sizeof(name));
			it = tlist_insert_item(tl, ICON(VGOBJ_ICON), name, vge);
			it->depth = 1;
		}
	}

	pthread_mutex_unlock(&vg->lock);
	tlist_restore_selections(tl);
}

/* Generate absolute vg coordinates for a vertex that's part of a block. */
void
vg_block_offset(struct vg *vg, struct vg_vertex *vtx)
{
	if (vg->cur_block != NULL) {
		vtx->x += vg->cur_block->pos.x;
		vtx->y += vg->cur_block->pos.y;
	}
}

struct window *
vg_block_editor(struct vg *vg)
{
	struct window *win;
	struct box *bo;
	struct tlist *tl;

	win = window_new(NULL);
	window_set_caption(win, _("Blocks"));
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 0);
	window_set_closure(win, WINDOW_HIDE);
	
	tl = tlist_new(win, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
	event_new(tl, "tlist-poll", poll_blocks, "%p", vg);

	bo = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	{
		struct button *bu;

		bu = button_new(bo, _("Destroy"));
		event_new(bu, "button-pushed", destroy_block, "%p,%p", vg, tl);
	}
	return (win);
}

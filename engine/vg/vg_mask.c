/*	$Csoft: vg_mask.c,v 1.2 2004/05/12 04:53:13 vedge Exp $	*/

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

#include "vg.h"
#include "vg_primitive.h"

const struct vg_element_ops vg_mask_ops = {
	N_("Polygonal mask"),
	vg_mask_init,
	NULL,
	vg_draw_mask,
	NULL
};

void
vg_mask_init(struct vg *vg, struct vg_element *vge)
{
	vge->vg_mask.scale = 1.0;
	vge->vg_mask.visible = 0;
	vge->vg_mask.p = NULL;
}

void
vg_draw_mask(struct vg *vg, struct vg_element *vge)
{
	int x1, y1, x2, y2;
	int x0, y0;
	int i;

	if (!vge->vg_mask.visible)
		return;

	if (vge->nvtx < 1) {
		/* TODO particle */
		return;
	}
	vg_rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &x1, &y1);
	x0 = x1;
	y0 = y1;
	for (i = 1; i < vge->nvtx; i++) {
		vg_rcoords2(vg, vge->vtx[i].x, vge->vtx[i].y, &x2, &y2);
		vg_line_primitive(vg, x1, y1, x2, y2, vge->color);
		x1 = x2;
		y1 = y2;
	}
	vg_line_primitive(vg, x0, y0, x1, y1, vge->color);
}

void
vg_mask_scale(struct vg *vg, float scale)
{
	vg->cur_vge->vg_mask.scale = scale;
}

void
vg_mask_visible(struct vg *vg, int vis)
{
	vg->cur_vge->vg_mask.visible = vis;
}

void
vg_mask_pointer(struct vg *vg, void *p)
{
	vg->cur_vge->vg_mask.p = p;
}

void
vg_mask_mousebutton(struct vg *vg, void (*func)(void *, Uint8), void *p)
{
	vg->cur_vge->vg_mask.mousebutton = func;
	vg->cur_vge->vg_mask.p = p;
}

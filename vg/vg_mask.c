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
 * Polygonal mask element.
 */

#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"

static void
Init(VG *vg, VG_Element *vge)
{
	vge->vg_mask.scale = 1.0f;
	vge->vg_mask.visible = 0;
	vge->vg_mask.p = NULL;
}

static void
Draw(VG_View *vv, VG_Element *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	int x1, y1, x2, y2;
	int x0, y0;
	int i;

	if (!vge->vg_mask.visible)
		return;

	if (vge->nvtx < 1) {
		/* TODO particle */
		return;
	}
	VG_GetViewCoords(vv, vge->vtx[0].x, vge->vtx[0].y, &x1, &y1);
	x0 = x1;
	y0 = y1;
	for (i = 1; i < vge->nvtx; i++) {
		VG_GetViewCoords(vv, vge->vtx[i].x, vge->vtx[i].y, &x2, &y2);
		AG_DrawLine(vv, x1, y1, x2, y2, c32);
		x1 = x2;
		y1 = y2;
	}
	AG_DrawLine(vv, x0, y0, x1, y1, c32);
}

void
VG_MaskScale(VG *vg, float scale)
{
	vg->cur_vge->vg_mask.scale = scale;
}

void
VG_MaskIsVisible(VG *vg, int vis)
{
	vg->cur_vge->vg_mask.visible = vis;
}

void
VG_MaskPtr(VG *vg, void *p)
{
	vg->cur_vge->vg_mask.p = p;
}

void
VG_MaskMouseButton(VG *vg, void (*func)(void *, Uint8), void *p)
{
	vg->cur_vge->vg_mask.mousebutton = func;
	vg->cur_vge->vg_mask.p = p;
}

const VG_ElementOps vgMaskOps = {
	N_("Polygonal mask"),
	NULL,
	Init,
	NULL,		/* destroy */
	Draw,
	NULL,		/* extent */
	NULL		/* intsect */
};

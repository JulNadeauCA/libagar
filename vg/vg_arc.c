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
 * Circular arc element.
 */

#include <core/limits.h>
#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"
#include "vg_math.h"
#include "icons.h"

static void
Init(VG *vg, VG_Element *vge)
{
	vge->vg_arc.w = 1.0f;
	vge->vg_arc.h = 1.0f;
	vge->vg_arc.s = 0.0f;
	vge->vg_arc.e = 360.0f;
}

void
VG_ArcBox(VG *vg, float w, float h)
{
	VG_Element *vge = vg->cur_vge;

	vge->vg_arc.w = w;
	vge->vg_arc.h = h;
}

void
VG_ArcRange(VG *vg, float s, float e)
{
	VG_Element *vge = vg->cur_vge;

	vge->vg_arc.s = s;
	vge->vg_arc.e = e;
}

void
VG_Arc3Points(VG *vg, VG_Vtx v[3])
{
}

static void
Draw(VG_View *vv, VG_Element *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	int a1 = (int)vge->vg_arc.s;
	int a2 = (int)vge->vg_arc.e;
	int x, y, w, h;
	int xPrev = 0, yPrev = 0;
	int w2 = w/2, h2 = h/2;
	int a;

	VG_GetViewCoords(vv, vge->vtx[0].x, vge->vtx[0].y, &x, &y);
	w = (int)(vge->vg_arc.w*vv->scale);
	h = (int)(vge->vg_arc.h*vv->scale);

	while (a2 < a1)
		a2 += 360;

	for (a = a1; a <= a2; a++) {
		int x, y;

		x = ((long)vg_cos_tbl[a % 360]*(long)w2/1024) + x; 
		y = ((long)vg_sin_tbl[a % 360]*(long)h2/1024) + y;
		if (a != a1) {
			AG_DrawLine(vv, xPrev, yPrev, x, y, c32);
		}
		xPrev = x;
		yPrev = y;
	}
}

static void
Extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_arc.w/2.0f;
	r->y = vge->vtx[0].y - vge->vg_arc.h/2.0f;
	r->w = vge->vg_arc.w;
	r->h = vge->vg_arc.h;
}

static float
Intersect(VG *vg, VG_Element *vge, float *x, float *y)
{
	/* TODO */
	return (AG_FLT_MAX);
}

const VG_ElementOps vgArcOps = {
	N_("Arc"),
	&vgIconCircle,
	Init,
	NULL,
	Draw,
	Extent,
	Intersect
};

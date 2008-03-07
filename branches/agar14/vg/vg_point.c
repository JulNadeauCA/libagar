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

#include "vg.h"
#include "vg_primitive.h"
#include "vg_math.h"
#include "icons.h"

static void
VG_PointRender(VG *vg, VG_Element *vge)
{
	VG_Vtx *vtx;
	int rx, ry;

	if (vge->nvtx >= 1) {
		vtx = &vge->vtx[0];
		VG_Rcoords2(vg, vtx->x, vtx->y, &rx, &ry);
		VG_PutPixel(vg, rx, ry-1, vge->color);
		VG_PutPixel(vg, rx-1, ry, vge->color);
		VG_PutPixel(vg, rx, ry, vge->color);
		VG_PutPixel(vg, rx+0, ry, vge->color);
		VG_PutPixel(vg, rx, ry+1, vge->color);
	}
}

static void
VG_PointExtent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	if (vge->nvtx >= 1) {
		VG_Vtx *vtx = &vge->vtx[0];
		
		r->x = vtx->x-1;
		r->y = vtx->y-1;
		r->w = vtx->x+1;
		r->h = vtx->x+1;
	} else {
		r->x = 0;
		r->y = 0;
		r->w = 0;
		r->h = 0;
	}
}

static float
VG_PointIntersect(VG *vg, VG_Element *vge, float *x, float *y)
{
	if (vge->nvtx >= 1) {
		float d = Distance2(*x, *y, vge->vtx[0].x, vge->vtx[0].y);
		*x = vge->vtx[0].x;
		*y = vge->vtx[0].y;
		return (d);
	} else {
		return (AG_FLT_MAX);
	}
}

const VG_ElementOps vgPointsOps = {
	N_("Point"),
	&vgIconPoints,
	NULL,
	NULL,
	VG_PointRender,
	VG_PointExtent,
	VG_PointIntersect
};

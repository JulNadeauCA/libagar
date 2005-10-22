/*	$Csoft: vg_circle.c,v 1.25 2005/09/27 00:25:20 vedge Exp $	*/

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

#include <core/core.h>

#include "vg.h"
#include "vg_primitive.h"
#include "vg_math.h"

static void
init(VG *vg, VG_Element *vge)
{
	vge->vg_circle.radius = 0.025;
}

void
VG_CircleRadius(VG *vg, double radius)
{
	vg->cur_vge->vg_circle.radius = radius;
}

void
VG_CircleDiameter(VG *vg, double diameter)
{
	vg->cur_vge->vg_circle.radius = diameter/2;
}

static void
render(VG *vg, VG_Element *vge)
{
	int rx, ry, radius;

	VG_Rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &rx, &ry);
	VG_RLength(vg, vge->vg_circle.radius, &radius);
	VG_CirclePrimitive(vg, rx, ry, radius, vge->color);
}

static void
extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_circle.radius;
	r->y = vge->vtx[0].y - vge->vg_circle.radius;
	r->w = vge->vg_circle.radius*2;
	r->h = vge->vg_circle.radius*2;
}

static float
intsect(VG *vg, VG_Element *vge, double x, double y)
{
	double rho, theta;
	VG_Vtx *vtx;

	if (vge->nvtx < 1) {
		return (FLT_MAX);
	}
	vtx = &vge->vtx[0];
	VG_Car2Pol(vg, x - vtx->x, y - vtx->y, &rho, &theta);

	return ((float)fabs(rho - vge->vg_circle.radius));
}

const VG_ElementOps vgCircleOps = {
	N_("Circle"),
	VGCIRCLES_ICON,
	init,
	NULL,
	render,
	extent,
	intsect
};

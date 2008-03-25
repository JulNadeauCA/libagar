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
 * Circle element.
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
	vge->vg_circle.radius = 0.025f;
}

void
VG_CircleRadius(VG *vg, float radius)
{
	vg->cur_vge->vg_circle.radius = radius;
}

void
VG_CircleDiameter(VG *vg, float diameter)
{
	vg->cur_vge->vg_circle.radius = diameter/2.0f;
}

static void
Draw(VG_View *vv, VG_Element *vge)
{
	int x, y, r;

	VG_GetViewCoords(vv, vge->vtx[0].x, vge->vtx[0].y, &x, &y);
	r = (int)(vge->vg_circle.radius*vv->scale);
	AG_DrawCircle(vv, x, y, r, VG_MapColorRGB(vge->color));
}

static void
Extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_circle.radius;
	r->y = vge->vtx[0].y - vge->vg_circle.radius;
	r->w = vge->vg_circle.radius*2.0f;
	r->h = vge->vg_circle.radius*2.0f;
}

static float
Intersect(VG *vg, VG_Element *vge, float *x, float *y)
{
	float d;

	if (vge->nvtx < 1) {
		return (AG_FLT_MAX);
	}
	d = Distance2(vge->vtx[0].x, vge->vtx[0].y, *x, *y) -
	    vge->vg_circle.radius;
	return (d);
}

const VG_ElementOps vgCircleOps = {
	N_("Circle"),
	&vgIconCircle,
	Init,
	NULL,
	Draw,
	Extent,
	Intersect	
};

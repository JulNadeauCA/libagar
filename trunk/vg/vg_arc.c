/*	$Csoft: vg_ellipse.c,v 1.20 2005/09/27 00:25:20 vedge Exp $	*/

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
VG_ArcInit(VG *vg, VG_Element *vge)
{
	vge->vg_arc.w = 1.0;
	vge->vg_arc.h = 1.0;
	vge->vg_arc.s = 0.0;
	vge->vg_arc.e = 360.0;
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
VG_ArcRender(VG *vg, VG_Element *vge)
{
	int x, y;
	int w, h;

	VG_Rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &x, &y);
	VG_RLength(vg, vge->vg_arc.w, &w);
	VG_RLength(vg, vge->vg_arc.h, &h);
	VG_ArcPrimitive(vg, x, y, w, h, (int)vge->vg_arc.s, (int)vge->vg_arc.e,
	    vge->color);
}

static void
VG_ArcExtent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_arc.w/2;
	r->y = vge->vtx[0].y - vge->vg_arc.h/2;
	r->w = vge->vg_arc.w;
	r->h = vge->vg_arc.h;
}

static float
VG_ArcIntersect(VG *vg, VG_Element *vge, float *x, float *y)
{
	/* TODO */
	return (FLT_MAX);
}

const VG_ElementOps vgArcOps = {
	N_("Arc"),
	VGCIRCLES_ICON,
	VG_ArcInit,
	NULL,
	VG_ArcRender,
	VG_ArcExtent,
	VG_ArcIntersect
};

const VG_ElementOps vgEllipseOps = {
	N_("Ellipse"),
	VGCIRCLES_ICON,
	VG_ArcInit,
	NULL,
	VG_ArcRender,
	VG_ArcExtent,
	VG_ArcIntersect
};


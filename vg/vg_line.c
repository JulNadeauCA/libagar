/*	$Csoft: vg_line.c,v 1.29 2005/09/27 00:25:20 vedge Exp $	*/

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

void
VG_DrawLineSegments(VG *vg, VG_Element *vge)
{
	int i;

	for (i = 0; i < vge->nvtx-1; i += 2) {
		if (vg->flags & VG_ANTIALIAS) {
			float Ax, Ay, Bx, By;

			VG_VtxCoords2d(vg, vge, i, &Ax, &Ay);
			VG_VtxCoords2d(vg, vge, i+1, &Bx, &By);
			VG_WuLinePrimitive(vg, Ax, Ay, Bx, By,
			    vge->line_st.thickness, vge->color);
		} else {
			int Ax, Ay, Bx, By;

			VG_VtxCoords2i(vg, vge, i, &Ax, &Ay);
			VG_VtxCoords2i(vg, vge, i+1, &Bx, &By);
			VG_LinePrimitive(vg, Ax, Ay, Bx, By, vge->color);
		}
	}
}

void
VG_DrawLineStrip(VG *vg, VG_Element *vge)
{
	int i;

	if (vg->flags & VG_ANTIALIAS) {
		float Ax, Ay, Bx, By;

		VG_VtxCoords2d(vg, vge, 0, &Ax, &Ay);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, vge, i, &Bx, &By);
			VG_WuLinePrimitive(vg, Ax, Ay, Bx, By,
			    vge->line_st.thickness, vge->color);
			Ax = Bx;
			Ay = By;
		}
	} else {
		int Ax, Ay, Bx, By;

		VG_VtxCoords2i(vg, vge, 0, &Ax, &Ay);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, vge, i, &Bx, &By);
			VG_LinePrimitive(vg, Ax, Ay, Bx, By, vge->color);
			Ax = Bx;
			Ay = By;
		}
	}
}

void
VG_DrawLineLoop(VG *vg, VG_Element *vge)
{
	if (vg->flags & VG_ANTIALIAS) {
		float Ax, Ay, Bx, By;
		float Cx, Cy;
		int i;

		VG_VtxCoords2d(vg, vge, 0, &Ax, &Ay);
		Cx = Ax;
		Cy = Ay;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, vge, i, &Bx, &By);
			VG_WuLinePrimitive(vg, Ax, Ay, Bx, By,
			    vge->line_st.thickness, vge->color);
			Ax = Bx;
			Ay = By;
		}
		VG_WuLinePrimitive(vg, Cx, Cy, Ax, Ay,
		    vge->line_st.thickness, vge->color);
	} else {
		int Ax, Ay, Bx, By;
		int Cx, Cy;
		int i;

		VG_VtxCoords2i(vg, vge, 0, &Ax,&Ay);
		Cx = Ax;
		Cy = Ay;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, vge, i, &Bx,&By);
			VG_LinePrimitive(vg, Ax,Ay, Bx,By, vge->color);
			Ax = Bx;
			Ay = By;
		}
		VG_LinePrimitive(vg, Cx,Cy, Ax,Ay, vge->color);
	}
}

static void
VG_LineExtent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	float xmin, xmax;
	float ymin, ymax;
	int i;

	xmin = xmax = vge->vtx[0].x;
	ymin = ymax = vge->vtx[0].y;
	for (i = 0; i < vge->nvtx; i++) {
		if (vge->vtx[i].x < xmin) { xmin = vge->vtx[i].x; }
		if (vge->vtx[i].y < ymin) { ymin = vge->vtx[i].y; }
		if (vge->vtx[i].x > xmax) { xmax = vge->vtx[i].x; }
		if (vge->vtx[i].y > ymax) { ymax = vge->vtx[i].y; }
	}
	r->x = xmin;
	r->y = ymin;
	r->w = xmax-xmin;
	r->h = ymax-ymin;
}

float
VG_ClosestLine2PointLen(VG *vg, int Ax, int Ay, int Bx, int By, int Px, int Py)
{
	float Vx = (float)(Bx - Ax);
	float Vy = (float)(By - Ay);
	float Wx = (float)(Px - Ax);
	float Wy = (float)(Py - Ay);
	float Ux, Uy;
	float c1, c2, b;

	c1 = VG_DotProd2(Wx,Wy, Vx,Vy);
	if (c1 <= 0.0) { return (VG_Distance2(Px,Py, Ax,Ay)); }
	c2 = VG_DotProd2(Vx,Vy, Vx,Vy);
	if (c2 <= 0.0) { return (VG_Distance2(Px,Py, Bx,By)); }
	b = c1/c2;
	Ux = Ax + b*Vx;
	Uy = Ay + b*Vy;
	return (VG_Distance2(Px,Py, Ux,Uy));
}

float
VG_LineIntersect(VG *vg, VG_Element *vge, float x, float y)
{
	float d, dMin = FLT_MAX;
	VG_Vtx v1, v2;
	float Ax, Ay, Bx, By, Cx, Cy;
	int i;

	switch (vge->type) {
	case VG_LINE_STRIP:
		VG_VtxCoords2d(vg, vge, 0, &Ax,&Ay);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, vge, i, &Bx,&By);

			d = VG_ClosestLine2PointLen(vg, Ax,Ay, Bx,By, x,y);
			if (d < dMin) { dMin = d; }

			Ax = Bx;
			Ay = By;
		}
		break;
	case VG_LINES:
		for (i = 0; i < vge->nvtx-1; i+=2) {
			VG_VtxCoords2d(vg, vge, i, &Ax,&Ay);
			VG_VtxCoords2d(vg, vge, i+1, &Bx,&By);

			d = VG_ClosestLine2PointLen(vg, Ax,Ay, Bx,By, x,y);
			if (d < dMin ) { dMin = d; }
		}
		break;
	case VG_LINE_LOOP:
	case VG_POLYGON:
		VG_VtxCoords2d(vg, vge, 0, &Ax,&Ay);
		Cx = Ax;
		Cy = Ay;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, vge, i, &Bx,&By);

			d = VG_ClosestLine2PointLen(vg, Ax,Ay, Bx,By, x,y);
			if (d < dMin ) { dMin = d; }
			
			Ax = Bx;
			Ay = By;
		}
		d = VG_ClosestLine2PointLen(vg, Cx,Cy, Ax,Ay, x,y);
		if (d < dMin) { dMin = d; }
		break;
	default:
		break;
	}
	return (dMin/vg->scale);
}

const VG_ElementOps vgLinesOps = {
	N_("Line"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineSegments,
	VG_LineExtent,
	VG_LineIntersect
};
const VG_ElementOps vgLineStripOps = {
	N_("Line strip"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineStrip,
	VG_LineExtent,
	VG_LineIntersect
};
const VG_ElementOps vgLineLoopOps = {
	N_("Line loop"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineLoop,
	VG_LineExtent,
	VG_LineIntersect
};

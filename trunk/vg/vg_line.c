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
 * Line, line strip and line loop elements.
 */

#include <core/limits.h>
#include <core/core.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"
#include "vg_math.h"
#include "icons.h"

void
VG_DrawLineSegments(VG_View *vv, VG_Node *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	int Ax, Ay, Bx, By;
	int i;

	for (i = 0; i < vge->nvtx-1; i += 2) {
		VG_GetViewCoordsVtx(vv, vge, i,   &Ax,&Ay);
		VG_GetViewCoordsVtx(vv, vge, i+1, &Bx,&By);
		AG_DrawLine(vv, Ax,Ay, Bx,By, c32);
	}
}

void
VG_DrawLineStrip(VG_View *vv, VG_Node *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	int Ax, Ay, Bx, By;
	int i;

	VG_GetViewCoordsVtx(vv, vge, 0, &Ax,&Ay);
	for (i = 1; i < vge->nvtx; i++) {
		VG_GetViewCoordsVtx(vv, vge, i, &Bx,&By);
		AG_DrawLine(vv, Ax,Ay, Bx,By, c32);
		Ax = Bx;
		Ay = By;
	}
}

void
VG_DrawLineLoop(VG_View *vv, VG_Node *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	int Ax, Ay, Bx, By, Cx, Cy;
	int i;

	VG_GetViewCoordsVtx(vv, vge, 0, &Ax,&Ay);
	Cx = Ax;
	Cy = Ay;
	for (i = 1; i < vge->nvtx; i++) {
		VG_GetViewCoordsVtx(vv, vge, i, &Bx,&By);
		AG_DrawLine(vv, Ax,Ay, Bx,By, c32);
		Ax = Bx;
		Ay = By;
	}
	AG_DrawLine(vv, Cx,Cy, Ax,Ay, c32);
}

void
VG_LineExtent(VG *vg, VG_Node *vge, VG_Rect *r)
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

static __inline__ float
Magnitude(float Ax, float Ay, float Bx, float By)
{
	float vx = Bx - Ax;
	float vy = By - Ay;

	return (Sqrt(vx*vx + vy*vy));
}

float
VG_ClosestLinePoint(VG *vg, float Ax, float Ay, float Bx, float By,
    float *Px, float *Py)
{
	float mag, u;
	float xInt, yInt;

	mag = Magnitude(Bx, By, Ax, Ay);
	u = ((*Px - Ax)*(Bx - Ax) + (*Py - Ay)*(By - Ay))/(mag*mag);
	if (u < 0.0f) {
		xInt = Ax;
		yInt = Ay;
	} else if (u > 1.0f) {
		xInt = Bx;
		yInt = By;
	} else {
		xInt = Ax + u*(Bx - Ax);
		yInt = Ay + u*(By - Ay);
	}
	mag = Magnitude(*Px, *Py, xInt, yInt);
	*Px = xInt;
	*Py = yInt;
	return (mag);
}

float
VG_LineIntersect(VG *vg, VG_Node *vge, float *x, float *y)
{
	float d, dMin = AG_FLT_MAX;
	float Ax, Ay, Bx, By, Cx, Cy;
	float ix, iy, mx = 0.0f, my = 0.0f;
	int i;

	switch (vge->type) {
	case VG_LINE_STRIP:
		Ax = vge->vtx[0].x;
		Ay = vge->vtx[0].y;
		for (i = 1; i < vge->nvtx; i++) {
			Bx = vge->vtx[i].x;
			By = vge->vtx[i].y;
			ix = *x;
			iy = *y;
			d = VG_ClosestLinePoint(vg, Ax,Ay, Bx,By, &ix,&iy);
			if (d < dMin) {
				dMin = d;
				mx = ix;
				my = iy;
			}
			Ax = Bx;
			Ay = By;
		}
		if (dMin < AG_FLT_MAX) {
			*x = mx;
			*y = my;
		}
		break;
	case VG_LINES:
		for (i = 0; i < vge->nvtx-1; i+=2) {
			Ax = vge->vtx[i].x;
			Ay = vge->vtx[i].y;
			Bx = vge->vtx[i+1].x;
			By = vge->vtx[i+1].y;
			ix = *x;
			iy = *y;
			d = VG_ClosestLinePoint(vg, Ax,Ay, Bx,By, &ix,&iy);
			if (d < dMin) {
				dMin = d;
				mx = ix;
				my = iy;
			}
		}
		if (dMin < AG_FLT_MAX) {
			*x = mx;
			*y = my;
		}
		break;
	case VG_LINE_LOOP:
	case VG_POLYGON:
		Cx = Ax = vge->vtx[0].x;
		Cy = Ay = vge->vtx[0].y;
		for (i = 1; i < vge->nvtx; i++) {
			Bx = vge->vtx[i].x;
			By = vge->vtx[i].y;

			ix = *x;
			iy = *y;
			d = VG_ClosestLinePoint(vg, Ax,Ay, Bx,By, &ix,&iy);
			if (d < dMin) {
				dMin = d;
				mx = ix;
				my = iy;
			}
			Ax = Bx;
			Ay = By;
		}

		ix = *x;
		iy = *y;
		d = VG_ClosestLinePoint(vg, Cx,Cy, Ax,Ay, &ix,&iy);
		if (d < dMin) {
			dMin = d;
			mx = ix;
			my = iy;
		}
		if (dMin < AG_FLT_MAX) {
			*x = mx;
			*y = my;
		}
		break;
	default:
		break;
	}
	return (dMin);
}

const VG_NodeOps vgLinesOps = {
	N_("Line"),
	&vgIconLine,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineSegments,
	VG_LineExtent,
	VG_LineIntersect
};
const VG_NodeOps vgLineStripOps = {
	N_("Line strip"),
	&vgIconLine,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineStrip,
	VG_LineExtent,
	VG_LineIntersect
};
const VG_NodeOps vgLineLoopOps = {
	N_("Line loop"),
	&vgIconLine,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineLoop,
	VG_LineExtent,
	VG_LineIntersect
};

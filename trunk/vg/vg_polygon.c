/*
 * Copyright (c) 2005-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Polygon element.
 */

#include <core/core.h>
#include <core/limits.h>

#include <gui/widget.h>
#include <gui/primitive.h>

#include "vg.h"
#include "vg_view.h"
#include "icons.h"

static void
Init(VG *vg, VG_Node *vge)
{
	vge->vg_polygon.outline = 0;
}

static int
CompareInts(const void *p1, const void *p2)
{
	return (*(const int *)p1 - *(const int *)p2);
}

static void
DrawOutline(VG_View *vv, VG_Node *vge, Uint32 c32)
{
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

static void
Draw(VG_View *vv, VG_Node *vge)
{
	Uint32 c32 = VG_MapColorRGB(vge->color);
	VG *vg = vv->vg;
	int i;
	int y, x1, y1, x2, y2;
	int ign, miny, maxy;
	int ind1, ind2;
	int ints;

	if (vge->nvtx < 3 || vge->vg_polygon.outline) {
		DrawOutline(vv, vge, c32);
		return;
	}
	
	if (vg->ints == NULL) {
		vg->ints = Malloc(vge->nvtx*sizeof(int));
		vg->nints = vge->nvtx;
	} else {
		if (vge->nvtx > vg->nints) {
			vg->ints = Realloc(vg->ints, vge->nvtx*sizeof(int));
			vg->nints = vge->nvtx;
		}
	}

	/* Find Y maxima */
	VG_GetViewCoordsVtx(vv, vge, 0, &ign, &miny);
	maxy = miny;
	for (i = 1; i < vge->nvtx; i++) {
		int vy;
	
		VG_GetViewCoordsVtx(vv, vge, i, &ign, &vy);
		if (vy < miny) {
			miny = vy;
		} else if (vy > maxy) {
			maxy = vy;
		}
	}

	/* Find the intersections. */
	for (y = miny; y <= maxy; y++) {
		ints = 0;
		for (i = 0; i < vge->nvtx; i++) {
			if (i == 0) {
				ind1 = vge->nvtx - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			VG_GetViewCoordsVtx(vv, vge, ind1, &ign, &y1);
			VG_GetViewCoordsVtx(vv, vge, ind2, &ign, &y2);
			if (y1 < y2) {
				VG_GetViewCoordsVtx(vv, vge, ind1, &x1, &ign);
				VG_GetViewCoordsVtx(vv, vge, ind2, &x2, &ign);
			} else if (y1 > y2) {
				VG_GetViewCoordsVtx(vv, vge, ind1, &x2, &y2);
				VG_GetViewCoordsVtx(vv, vge, ind2, &x1, &y1);
			} else {
				continue;
			}
			if (((y >= y1) && (y < y2)) ||
			    ((y == maxy) && (y > y1) && (y <= y2))) {
				vg->ints[ints++] =
				    (((y-y1)<<16) / (y2-y1)) *
				    (x2-x1) + (x1<<16);
			} 
		}
		qsort(vg->ints, ints, sizeof(int), CompareInts);

		for (i = 0; i < ints; i += 2) {
			int xa, xb;

			xa = vg->ints[i] + 1;
			xa = (xa>>16) + ((xa&0x8000) >> 15);
			xb = vg->ints[i+1] - 1;
			xb = (xb>>16) + ((xb&0x8000) >> 15);

			AG_DrawLineH(vv, xa, xb, y, c32);
		}
	}
}

static void
Extent(VG_View *vv, VG_Node *vge, VG_Rect *r)
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

static float
Proximity(VG *vg, VG_Node *vge, float *x, float *y)
{
	float d, dMin = AG_FLT_MAX;
	float Ax, Ay, Bx, By, Cx, Cy;
	float ix, iy, mx = 0.0f, my = 0.0f;
	int i;

	Cx = Ax = vge->vtx[0].x;
	Cy = Ay = vge->vtx[0].y;
	for (i = 1; i < vge->nvtx; i++) {
		Bx = vge->vtx[i].x;
		By = vge->vtx[i].y;

		ix = *x;
		iy = *y;
		d = VG_PointLineDistance(vg, Ax,Ay, Bx,By, &ix,&iy);
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
	d = VG_PointLineDistance(vg, Cx,Cy, Ax,Ay, &ix,&iy);
	if (d < dMin) {
		dMin = d;
		mx = ix;
		my = iy;
	}
	if (dMin < AG_FLT_MAX) {
		*x = mx;
		*y = my;
	}
	return (dMin);
}

const VG_NodeOps vgPolygonOps = {
	N_("Polygon"),
	&vgIconPolygon,
	Init,
	NULL,				/* destroy */
	Draw,
	Extent,
	Proximity
};

/*	$Csoft: vg_polygon.c,v 1.8 2005/09/27 00:25:21 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

static void
init(VG *vg, VG_Element *vge)
{
	vge->vg_polygon.outline = 0;
}

static int
compare_ints(const void *p1, const void *p2)
{
	return (*(const int *)p1 - *(const int *)p2);
}

static void
render(VG *vg, VG_Element *vge)
{
	int i;
	int x, y, x1, y1, x2, y2;
	int miny, maxy;
	int ind1, ind2;
	int ints;

	if (vge->nvtx < 3 || vge->vg_polygon.outline) {	/* Draw outline */
		VG_DrawLineLoop(vg, vge);
		return;
	}
	
	if (vg->ints == NULL) {
		vg->ints = Malloc(vge->nvtx*sizeof(int), M_VG);
		vg->nints = vge->nvtx;
	} else {
		if (vge->nvtx > vg->nints) {
			vg->ints = Realloc(vg->ints, vge->nvtx*sizeof(int));
			vg->nints = vge->nvtx;
		}
	}

	/* Find Y maxima */
	VG_VtxCoords2i(vg, vge, 0, NULL, &miny);
	maxy = miny;
	for (i = 1; i < vge->nvtx; i++) {
		int vy;
	
		VG_VtxCoords2i(vg, vge, i, NULL, &vy);
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
			VG_VtxCoords2i(vg, vge, ind1, NULL, &y1);
			VG_VtxCoords2i(vg, vge, ind2, NULL, &y2);
			if (y1 < y2) {
				VG_VtxCoords2i(vg, vge, ind1, &x1, NULL);
				VG_VtxCoords2i(vg, vge, ind2, &x2, NULL);
//				x1 = VG_RASX(vg, vge->vtx[ind1].x);
//				x2 = VG_RASX(vg, vge->vtx[ind2].x);
			} else if (y1 > y2) {
				VG_VtxCoords2i(vg, vge, ind1, &x2, &y2);
				VG_VtxCoords2i(vg, vge, ind2, &x1, &y1);
//				y2 = VG_RASY(vg, vge->vtx[ind1].y);
//				y1 = VG_RASY(vg, vge->vtx[ind2].y);
//				x2 = VG_RASX(vg, vge->vtx[ind1].x);
//				x1 = VG_RASX(vg, vge->vtx[ind2].x);
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
		qsort(vg->ints, ints, sizeof(int), compare_ints);

		for (i = 0; i < ints; i += 2) {
			int xa, xb, xi;
			Uint8 r, g, b;

			xa = vg->ints[i] + 1;
			xa = (xa>>16) + ((xa&0x8000) >> 15);
			xb = vg->ints[i+1] - 1;
			xb = (xb>>16) + ((xb&0x8000) >> 15);

			VG_HLinePrimitive(vg, xa, xb, y, vge->color);
		}
	}
}

static void
extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	double xmin = 0, xmax = 0;
	double ymin = 0, ymax = 0;
	int i;

	xmin = vge->vtx[0].x;
	ymin = vge->vtx[0].y;
	for (i = 0; i < vge->nvtx; i++) {
		if (vge->vtx[i].x < xmin)
			xmin = vge->vtx[i].x;
		if (vge->vtx[i].y < ymin)
			ymin = vge->vtx[i].y;
		if (vge->vtx[i].x > xmax)
			xmax = vge->vtx[i].x;
		if (vge->vtx[i].y > ymax)
			ymax = vge->vtx[i].y;
	}
	r->x = xmin;
	r->y = ymin;
	r->w = xmax-xmin;
	r->h = ymax-ymin;
}

const VG_ElementOps vgPolygonOps = {
	N_("Polygon"),
	RG_POLYGON_ICON,
	init,
	NULL,				/* destroy */
	render,
	extent,
	VG_LineIntersect
};

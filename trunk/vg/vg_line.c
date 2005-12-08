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
			float x1, y1, x2, y2;

			VG_VtxCoords2d(vg, vge, i, &x1, &y1);
			VG_VtxCoords2d(vg, vge, i+1, &x2, &y2);
			VG_WuLinePrimitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
		} else {
			int x1, y1, x2, y2;

			VG_VtxCoords2i(vg, vge, i, &x1, &y1);
			VG_VtxCoords2i(vg, vge, i+1, &x2, &y2);
			VG_LinePrimitive(vg, x1, y1, x2, y2, vge->color);
		}
	}
}

void
VG_DrawLineStrip(VG *vg, VG_Element *vge)
{
	int i;

	if (vg->flags & VG_ANTIALIAS) {
		float x1, y1, x2, y2;

		VG_VtxCoords2d(vg, vge, 0, &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, vge, i, &x2, &y2);
			VG_WuLinePrimitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
			x1 = x2;
			y1 = y2;
		}
	} else {
		int x1, y1, x2, y2;

		VG_VtxCoords2i(vg, vge, 0, &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, vge, i, &x2, &y2);
			VG_LinePrimitive(vg, x1, y1, x2, y2, vge->color);
			x1 = x2;
			y1 = y2;
		}
	}
}

void
VG_DrawLineLoop(VG *vg, VG_Element *vge)
{
	if (vg->flags & VG_ANTIALIAS) {
		float x1, y1, x2, y2;
		float x0, y0;
		int i;

		VG_VtxCoords2d(vg, vge, 0, &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, vge, i, &x2, &y2);
			VG_WuLinePrimitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
			x1 = x2;
			y1 = y2;
		}
		VG_WuLinePrimitive(vg, x0, y0, x1, y1,
		    vge->line_st.thickness, vge->color);
	} else {
		int x1, y1, x2, y2;
		int x0, y0;
		int i;

		VG_VtxCoords2i(vg, vge, 0, &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, vge, i, &x2, &y2);
			VG_LinePrimitive(vg, x1, y1, x2, y2, vge->color);
			x1 = x2;
			y1 = y2;
		}
		VG_LinePrimitive(vg, x0, y0, x1, y1, vge->color);
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

/*
 * Calculate the distance of the shortest path from a given point to any
 * point on a line.
 */
static float
VG_ClosestLinePoint(VG *vg, int x1, int y1, int x2, int y2, int mx, int my)
{
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;
	float closest = DBL_MAX;
	float theta, rho;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if (dy <= dx) {
		d = dy*2 - dx;
		inc1 = dy*2;
		inc2 = (dy-dx)*2;
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydir = -1;
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydir = 1;
			xend = x2;
		}
		VG_Car2Pol(vg, mx-x, my-y, &rho, &theta);
		if (rho < closest) { closest = rho; }

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				VG_Car2Pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		} else {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y--;
					d += inc2;
				}
				VG_Car2Pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		}		
	} else {
		d = dx*2 - dy;
		inc1 = dx*2;
		inc2 = (dx-dy)*2;
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdir = -1;
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdir = 1;
		}
		VG_Car2Pol(vg, mx-x, my-y, &rho, &theta);
		if (rho < closest) { closest = rho; }

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				VG_Car2Pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		} else {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x--;
					d += inc2;
				}
				VG_Car2Pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		}
	}
	return (closest);
}

float
VG_LineIntersect(VG *vg, VG_Element *vge, float x, float y)
{
	int mx = VG_RASX(vg,x);
	int my = VG_RASX(vg,y);
	float d, min_distance = FLT_MAX;
	VG_Vtx v1, v2;
	int x1, y1, x2, y2, x0, y0;
	int i;

	switch (vge->type) {
	case VG_LINE_STRIP:
		VG_VtxCoords2i(vg, vge, 0, &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, vge, i, &x2, &y2);

			d = VG_ClosestLinePoint(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }

			x1 = x2;
			y1 = y2;
		}
		break;
	case VG_LINES:
		for (i = 0; i < vge->nvtx-1; i+=2) {
			VG_VtxCoords2i(vg, vge, i, &x1, &y1);
			VG_VtxCoords2i(vg, vge, i+1, &x2, &y2);

			d = VG_ClosestLinePoint(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }
		}
		break;
	case VG_LINE_LOOP:
	case VG_POLYGON:
		VG_VtxCoords2i(vg, vge, 0, &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, vge, i, &x2, &y2);

			d = VG_ClosestLinePoint(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }
			
			x1 = x2;
			y1 = y2;
		}
		d = VG_ClosestLinePoint(vg, x0, y0, x1, y1, mx, my);
		if (d < min_distance) { min_distance = d; }
		break;
	default:
		break;
	}
	return (VG_VECLENF(vg,min_distance));
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

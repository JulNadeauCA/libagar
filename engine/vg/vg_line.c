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

#include <engine/engine.h>

#ifdef EDITION
#include <engine/map/mapview.h>
#include <engine/map/tool.h>

#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#endif

#include "vg.h"
#include "vg_primitive.h"

void
VG_DrawLineSegments(VG *vg, VG_Element *vge)
{
	int i;

	for (i = 0; i < vge->nvtx-1; i += 2) {
		if (vg->flags & VG_ANTIALIAS) {
			double x1, y1, x2, y2;

			VG_VtxCoords2d(vg, &vge->vtx[i], &x1, &y1);
			VG_VtxCoords2d(vg, &vge->vtx[i+1], &x2, &y2);
			VG_WuLinePrimitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
		} else {
			int x1, y1, x2, y2;

			VG_VtxCoords2i(vg, &vge->vtx[i], &x1, &y1);
			VG_VtxCoords2i(vg, &vge->vtx[i+1], &x2, &y2);
			VG_LinePrimitive(vg, x1, y1, x2, y2, vge->color);
		}
	}
}

void
VG_DrawLineStrip(VG *vg, VG_Element *vge)
{
	int i;

	if (vg->flags & VG_ANTIALIAS) {
		double x1, y1, x2, y2;

		VG_VtxCoords2d(vg, &vge->vtx[0], &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, &vge->vtx[i], &x2, &y2);
			VG_WuLinePrimitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
			x1 = x2;
			y1 = y2;
		}
	} else {
		int x1, y1, x2, y2;

		VG_VtxCoords2i(vg, &vge->vtx[0], &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, &vge->vtx[i], &x2, &y2);
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
		double x1, y1, x2, y2;
		double x0, y0;
		int i;

		VG_VtxCoords2d(vg, &vge->vtx[0], &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2d(vg, &vge->vtx[i], &x2, &y2);
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

		VG_VtxCoords2i(vg, &vge->vtx[0], &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, &vge->vtx[i], &x2, &y2);
			VG_LinePrimitive(vg, x1, y1, x2, y2, vge->color);
			x1 = x2;
			y1 = y2;
		}
		VG_LinePrimitive(vg, x0, y0, x1, y1, vge->color);
	}
}

static void
extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	double xmin, xmax;
	double ymin, ymax;
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
intsect_line(VG *vg, int x1, int y1, int x2, int y2, int mx, int my)
{
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;
	float closest = DBL_MAX;
	double theta, rho;

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
VG_LineIntersect(VG *vg, VG_Element *vge, double x, double y)
{
	int mx = VG_RASX(vg,x);
	int my = VG_RASX(vg,y);
	double rho, theta;
	float d, min_distance = FLT_MAX;
	VG_Vtx v1, v2;
	int x1, y1, x2, y2, x0, y0;
	int i;

	switch (vge->type) {
	case VG_LINE_STRIP:
		VG_VtxCoords2i(vg, &vge->vtx[0], &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, &vge->vtx[i], &x2, &y2);

			d = intsect_line(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }

			x1 = x2;
			y1 = y2;
		}
		break;
	case VG_LINES:
		for (i = 0; i < vge->nvtx-1; i+=2) {
			VG_VtxCoords2i(vg, &vge->vtx[i], &x1, &y1);
			VG_VtxCoords2i(vg, &vge->vtx[i+1], &x2, &y2);

			d = intsect_line(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }
		}
		break;
	case VG_LINE_LOOP:
	case VG_POLYGON:
		VG_VtxCoords2i(vg, &vge->vtx[0], &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			VG_VtxCoords2i(vg, &vge->vtx[i], &x2, &y2);

			d = intsect_line(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }
			
			x1 = x2;
			y1 = y2;
		}
		d = intsect_line(vg, x0, y0, x1, y1, mx, my);
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
	extent,
	VG_LineIntersect
};
const VG_ElementOps vgLineStripOps = {
	N_("Line strip"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineStrip,
	extent,
	VG_LineIntersect
};
const VG_ElementOps vgLineLoopOps = {
	N_("Line loop"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	VG_DrawLineLoop,
	extent,
	VG_LineIntersect
};

#ifdef EDITION
static enum {
	MODE_SEGMENTS,
	MODE_STRIP,
	MODE_LOOP
} mode = MODE_STRIP;

static int seq;
static VG_Element *cur_line;
static VG_Vtx *cur_vtx;

static void
line_AG_MaptoolInit(void *t)
{
	AG_MaptoolPushStatus(t, _("Specify first point."));
	seq = 0;
	cur_line = NULL;
	cur_vtx = NULL;
}

static void
line_tool_pane(void *t, void *con)
{
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	AG_Radio *rad;

	rad = AG_RadioNew(con, mode_items);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &mode);
}

static int
line_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel,
    int btn)
{
	VG *vg = TOOL(t)->p;
	double x, y;
	
	VG_Map2Vec(vg, xmap, ymap, &x, &y);
	vg->origin[1].x = x;
	vg->origin[1].y = y;

	if (cur_vtx != NULL) {
		cur_vtx->x = x;
		cur_vtx->y = y;
		vg->redraw++;
	}
	return (1);
}

static int
line_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	VG *vg = TOOL(t)->p;
	double vx, vy;

	switch (mode) {
	case MODE_SEGMENTS:
		if (btn == 1) {
			if (seq++ == 0) {
				cur_line = VG_Begin(vg, VG_LINES);
				VG_Map2Vec(vg, xmap, ymap, &vx, &vy);
				VG_Vertex2(vg, vx, vy);
				cur_vtx = VG_Vertex2(vg, vx, vy);

				AG_MaptoolPushStatus(t,
				    _("Specify second point or [undo line]."));
			} else {
				goto finish;
			}
		} else {
			if (cur_line != NULL) {
				VG_DestroyElement(vg, cur_line);
			}
			goto finish;
		}
		break;
	case MODE_STRIP:
	case MODE_LOOP:
		if (btn == 1) {
			if (seq++ == 0) {
#ifdef DEBUG
				if (vg->cur_block != NULL)
					fatal("block");
#endif
				cur_line = VG_Begin(vg,
				    mode == MODE_STRIP ? VG_LINE_STRIP :
							 VG_LINE_LOOP);
				VG_Map2Vec(vg, xmap, ymap, &vx, &vy);
				VG_Vertex2(vg, vx, vy);
			} else {
				AG_MaptoolPopStatus(t);
			}
			VG_Map2Vec(vg, xmap, ymap, &vx, &vy);
			cur_vtx = VG_Vertex2(vg, vx, vy);
			vg->redraw++;

			AG_MaptoolPushStatus(t, _("Specify point %d or "
			                      "[close/undo vertex]."), seq+1);
		} else {
			if (cur_vtx != NULL) {
				if (cur_line->nvtx <= 2) {
					VG_DestroyElement(vg, cur_line);
					cur_line = NULL;
				} else {
					VG_PopVertex(vg);
				}
				vg->redraw++;
			}
			goto finish;
		}
		break;
	default:
		break;
	}
	return (1);
finish:
	cur_line = NULL;
	cur_vtx = NULL;
	seq = 0;
	AG_MaptoolPopStatus(t);
	return (1);
}

const AG_MaptoolOps vgLineTool = {
	"Lines", N_("Line segments, strips and loops."),
	VGLINES_ICON,
	sizeof(AG_Maptool),
	0,
	line_AG_MaptoolInit,
	NULL,			/* destroy */
	line_tool_pane,
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	line_mousemotion,
	line_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

/*	$Csoft: vg_polygon.c,v 1.7 2005/07/30 05:01:34 vedge Exp $	*/

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

#include <engine/engine.h>

#ifdef EDITION
#include <engine/map/mapview.h>
#include <engine/map/tool.h>

#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#endif

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
	maxy = miny = VG_RASY(vg, vge->vtx[0].y);
	for (i = 1; i < vge->nvtx; i++) {
		int vy = VG_RASY(vg, vge->vtx[i].y);

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
			y1 = VG_RASY(vg, vge->vtx[ind1].y);
			y2 = VG_RASY(vg, vge->vtx[ind2].y);
			if (y1 < y2) {
				x1 = VG_RASX(vg, vge->vtx[ind1].x);
				x2 = VG_RASX(vg, vge->vtx[ind2].x);
			} else if (y1 > y2) {
				y2 = VG_RASY(vg, vge->vtx[ind1].y);
				y1 = VG_RASY(vg, vge->vtx[ind2].y);
				x2 = VG_RASX(vg, vge->vtx[ind1].x);
				x1 = VG_RASX(vg, vge->vtx[ind2].x);
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

#ifdef EDITION
static int seq;
static VG_Element *cur_polygon;
static VG_Vtx *cur_vtx;

static void
init_tool(void *t)
{
	AG_MaptoolPushStatus(t, _("Specify first point."));
	seq = 0;
	cur_polygon = NULL;
	cur_vtx = NULL;
}

static int
polygon_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel,
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
polygon_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	VG *vg = TOOL(t)->p;
	double vx, vy;

	if (btn == SDL_BUTTON_LEFT) {
		if (seq++ == 0) {
#ifdef DEBUG
			if (vg->cur_block != NULL)
				fatal("block");
#endif
			cur_polygon = VG_Begin(vg, VG_POLYGON);
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
			if (cur_polygon->nvtx <= 2) {
				VG_DestroyElement(vg, cur_polygon);
				cur_polygon = NULL;
			} else {
				VG_PopVertex(vg);
			}
			vg->redraw++;
		}
	}
	return (1);
finish:
	cur_polygon = NULL;
	cur_vtx = NULL;
	seq = 0;
	AG_MaptoolPopStatus(t);
	return (1);
}

const AG_MaptoolOps vg_polygon_tool = {
	"Polygon", N_("Draw filled polygons."),
	RG_POLYGON_ICON,
	sizeof(AG_Maptool),
	0,
	init_tool,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	polygon_mousemotion,
	polygon_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

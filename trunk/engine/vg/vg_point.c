/*	$Csoft: vg_point.c,v 1.22 2005/07/30 05:01:34 vedge Exp $	*/

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
#endif

#include "vg.h"
#include "vg_primitive.h"

static void
render(VG *vg, VG_Element *vge)
{
	VG_Vtx *vtx;
	int rx, ry;

	if (vge->nvtx >= 1) {
		vtx = &vge->vtx[0];
		VG_Rcoords2(vg, vtx->x, vtx->y, &rx, &ry);
		VG_PutPixel(vg, rx, ry-1, vge->color);
		VG_PutPixel(vg, rx-1, ry, vge->color);
		VG_PutPixel(vg, rx, ry, vge->color);
		VG_PutPixel(vg, rx+0, ry, vge->color);
		VG_PutPixel(vg, rx, ry+1, vge->color);
	}
}

static void
extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	if (vge->nvtx >= 1) {
		VG_Vtx *vtx = &vge->vtx[0];
		
		r->x = vtx->x-1;
		r->y = vtx->y-1;
		r->w = vtx->x+1;
		r->h = vtx->x+1;
	} else {
		r->x = 0;
		r->y = 0;
		r->w = 0;
		r->h = 0;
	}
}

static float
intsect(VG *vg, VG_Element *vge, double x, double y)
{
	if (vge->nvtx >= 1) {
		VG_Vtx *vtx = &vge->vtx[0];
		return (vtx->x - x) + (vtx->y - y);
	} else {
		return (FLT_MAX);
	}
}

const VG_ElementOps vgPointsOps = {
	N_("Point"),
	VGPOINTS_ICON,
	NULL,
	NULL,
	render,
	extent,
	intsect
};

#ifdef EDITION
static void
point_AG_MaptoolInit(void *t)
{
	AG_MaptoolPushStatus(t, _("Specify the point location."));
}

static int
point_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel,
    int btn)
{
	VG *vg = TOOL(t)->p;
	
	VG_Map2Vec(vg, xmap, ymap, &vg->origin[1].x, &vg->origin[1].y);
	vg->redraw++;
	return (1);
}

static int
point_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	VG *vg = TOOL(t)->p;
	double vx, vy;

	VG_Begin(vg, VG_POINTS);
	VG_Map2Vec(vg, xmap, ymap, &vx, &vy);
	VG_Vertex2(vg, vx, vy);
	VG_End(vg);
	return (1);
}

const AG_MaptoolOps vg_point_tool = {
	N_("Point"), N_("Trace an individual point."),
	VGPOINTS_ICON,
	sizeof(AG_Maptool),
	0,
	point_AG_MaptoolInit,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	point_mousemotion,
	point_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

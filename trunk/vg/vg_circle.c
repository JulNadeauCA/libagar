/*	$Csoft: vg_circle.c,v 1.25 2005/09/27 00:25:20 vedge Exp $	*/

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
#include "vg_math.h"

static void
init(VG *vg, VG_Element *vge)
{
	vge->vg_circle.radius = 0.025;
}

void
VG_CircleRadius(VG *vg, double radius)
{
	vg->cur_vge->vg_circle.radius = radius;
}

void
VG_CircleDiameter(VG *vg, double diameter)
{
	vg->cur_vge->vg_circle.radius = diameter/2;
}

static void
render(VG *vg, VG_Element *vge)
{
	int rx, ry, radius;

	VG_Rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &rx, &ry);
	VG_RLength(vg, vge->vg_circle.radius, &radius);
	VG_CirclePrimitive(vg, rx, ry, radius, vge->color);
}

static void
extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_circle.radius;
	r->y = vge->vtx[0].y - vge->vg_circle.radius;
	r->w = vge->vg_circle.radius*2;
	r->h = vge->vg_circle.radius*2;
}

static float
intsect(VG *vg, VG_Element *vge, double x, double y)
{
	double rho, theta;
	VG_Vtx *vtx;

	if (vge->nvtx < 1) {
		return (FLT_MAX);
	}
	vtx = &vge->vtx[0];
	VG_Car2Pol(vg, x - vtx->x, y - vtx->y, &rho, &theta);

	return (fabsf(rho - vge->vg_circle.radius));
}

const VG_ElementOps vgCircleOps = {
	N_("Circle"),
	VGCIRCLES_ICON,
	init,
	NULL,
	render,
	extent,
	intsect
};

#ifdef EDITION
static VG_Element *cur_circle;
static VG_Vtx *cur_radius;
static int seq;

static void
circle_AG_MaptoolInit(void *p)
{
	AG_MaptoolPushStatus(p, _("Specify the circle's center point."));
	seq = 0;
	cur_circle = NULL;
	cur_radius = NULL;
}

static int
circle_mousemotion(void *p, int xmap, int ymap, int xrel, int yrel, int b)
{
	VG *vg = TOOL(p)->p;
	double x, y;
	
	VG_Map2Vec(vg, xmap, ymap, &x, &y);

	if (cur_radius != NULL) {
		cur_radius->x = x;
		cur_radius->y = y;
		cur_circle->vg_circle.radius =
		    sqrt(pow(x-vg->origin[2].x,2) +
		         pow(y-vg->origin[2].y,2));
	} else {
		vg->origin[2].x = x;
		vg->origin[2].y = y;
		vg->redraw++;
	}
	vg->origin[1].x = x;
	vg->origin[1].y = y;
	return (1);
}

static int
circle_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	VG *vg = TOOL(t)->p;
	double vx, vy;

	switch (btn) {
	case 1:
		if (seq++ == 0) {
			cur_circle = VG_Begin(vg, VG_CIRCLE);
			VG_Map2Vec(vg, xmap, ymap, &vx, &vy);
			VG_Vertex2(vg, vx, vy);
			cur_radius = VG_Vertex2(vg, vx, vy);
			VG_End(vg);

			AG_MaptoolPushStatus(t, _("Specify the circle's radius "
			                      "or [undo circle]."));
		} else {
			goto finish;
		}
		break;
	default:
		if (cur_circle != NULL) {
			VG_DestroyElement(vg, cur_circle);
		}
		goto finish;
	}
finish:
	cur_circle = NULL;
	cur_radius = NULL;
	seq = 0;
	AG_MaptoolPopStatus(t);
	return (1);
}

const AG_MaptoolOps vgCircleTool = {
	"Circles", N_("Draw circles."),
	VGCIRCLES_ICON,
	sizeof(AG_Maptool),
	0,
	circle_AG_MaptoolInit,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	circle_mousemotion,
	circle_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

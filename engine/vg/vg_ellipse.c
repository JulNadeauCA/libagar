/*	$Csoft: vg_ellipse.c,v 1.19 2005/07/30 05:01:34 vedge Exp $	*/

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
	vge->vg_arc.w = 1;
	vge->vg_arc.h = 1;
	vge->vg_arc.s = 0;
	vge->vg_arc.e = 360;
}

void
VG_EllipseExtent(VG *vg, double w, double h)
{
	VG_Element *vge = vg->cur_vge;

	vge->vg_arc.w = w;
	vge->vg_arc.h = h;
}

void
VG_EllipseArc(VG *vg, double s, double e)
{
	VG_Element *vge = vg->cur_vge;

	vge->vg_arc.s = s;
	vge->vg_arc.e = e;
}

static void
render(VG *vg, VG_Element *vge)
{
	int x, y;
	int w, h;
	int s, e;

	VG_Rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &x, &y);
	VG_RLength(vg, vge->vg_arc.w, &w);
	VG_RLength(vg, vge->vg_arc.h, &h);
	VG_RLength(vg, vge->vg_arc.s, &s);
	VG_RLength(vg, vge->vg_arc.e, &e);
	VG_ArcPrimitive(vg, x, y, w, h, s, e, vge->color);
}

static void
extent(VG *vg, VG_Element *vge, VG_Rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_arc.w/2;
	r->y = vge->vtx[0].y - vge->vg_arc.h/2;
	r->w = vge->vg_arc.w;
	r->h = vge->vg_arc.h;
}

static float
intsect(VG *vg, VG_Element *vge, double x, double y)
{
	return (FLT_MAX);
}

const VG_ElementOps vgEllipseOps = {
	N_("Ellipse"),
	VGCIRCLES_ICON,
	init,
	NULL,
	render,
	extent,
	intsect
};

const VG_ElementOps vgArcOps = {
	N_("Arc"),
	VGCIRCLES_ICON,
	init,
	NULL,
	render,
	extent,
	intsect
};

#ifdef EDITION
static VG_Element *cur_ellipse;
static int seq;

static void
ellipse_AG_MaptoolInit(void *t)
{
	seq = 0;
	cur_ellipse = NULL;
	AG_MaptoolPushStatus(t, _("Specify the ellipse's center point."));
}

static int
ellipse_mousemotion(void *p, int xmap, int ymap, int xrel, int yrel, int b)
{
	VG *vg = TOOL(p)->p;
	double x, y;
	
	VG_Map2Vec(vg, xmap, ymap, &x, &y);

	if (cur_ellipse != NULL) {
		if (seq == 1) {
			double rx, ry;
			double theta, m;

			rx = fabs(x - vg->origin[2].x);
			ry = fabs(y - vg->origin[2].y);

			theta = rx>0 ? atan(ry/rx) : 0;
			m = sqrt(pow(rx,2) + pow(ry,2));
			cur_ellipse->vg_arc.w = cos(theta)*m*2;
			cur_ellipse->vg_arc.h = sin(theta)*m*2;
		} 
	} else {
		vg->origin[2].x = x;
		vg->origin[2].y = y;
	}
	vg->origin[1].x = x;
	vg->origin[1].y = y;
	vg->redraw++;
	return (1);
}

static int
ellipse_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	VG *vg = TOOL(t)->p;
	double vx, vy;

	switch (btn) {
	case 1:
		switch (seq++) {
		case 0:
			cur_ellipse = VG_Begin(vg, VG_ELLIPSE);
			VG_Map2Vec(vg, xmap, ymap, &vx, &vy);
			VG_Vertex2(vg, vx, vy);
			VG_End(vg);

			AG_MaptoolPushStatus(t,
			    _("Specify the ellipse's geometry or "
			      "[undo ellipse]."));
			break;
		default:
			goto finish;
		}
		break;
	default:
		if (cur_ellipse != NULL) {
			VG_DestroyElement(vg, cur_ellipse);
		}
		goto finish;
	}
finish:
	cur_ellipse = NULL;
	seq = 0;
	AG_MaptoolPopStatus(t);
	return (1);
}

const AG_MaptoolOps vg_ellipse_tool = {
	N_("Ellipses"), N_("Draw ellipses."),
	VGCIRCLES_ICON,
	sizeof(AG_Maptool),
	0,
	ellipse_AG_MaptoolInit,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	ellipse_mousemotion,
	ellipse_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

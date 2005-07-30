/*	$Csoft: vg_point.c,v 1.21 2005/06/30 06:26:23 vedge Exp $	*/

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
render(struct vg *vg, struct vg_element *vge)
{
	struct vg_vertex *vtx;
	int rx, ry;

	if (vge->nvtx >= 1) {
		vtx = &vge->vtx[0];
		vg_rcoords2(vg, vtx->x, vtx->y, &rx, &ry);
		vg_put_pixel(vg, rx, ry-1, vge->color);
		vg_put_pixel(vg, rx-1, ry, vge->color);
		vg_put_pixel(vg, rx, ry, vge->color);
		vg_put_pixel(vg, rx+0, ry, vge->color);
		vg_put_pixel(vg, rx, ry+1, vge->color);
	}
}

static void
extent(struct vg *vg, struct vg_element *vge, struct vg_rect *r)
{
	if (vge->nvtx >= 1) {
		struct vg_vertex *vtx = &vge->vtx[0];
		
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
intsect(struct vg *vg, struct vg_element *vge, double x, double y)
{
	if (vge->nvtx >= 1) {
		struct vg_vertex *vtx = &vge->vtx[0];
		return (vtx->x - x) + (vtx->y - y);
	} else {
		return (FLT_MAX);
	}
}

const struct vg_element_ops vg_points_ops = {
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
point_tool_init(void *t)
{
	tool_push_status(t, _("Specify the point location."));
}

static int
point_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel,
    int btn)
{
	struct vg *vg = TOOL(t)->p;
	
	vg_map2vec(vg, xmap, ymap, &vg->origin[1].x, &vg->origin[1].y);
	vg->redraw++;
	return (1);
}

static int
point_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	struct vg *vg = TOOL(t)->p;
	double vx, vy;

	vg_begin_element(vg, VG_POINTS);
	vg_map2vec(vg, xmap, ymap, &vx, &vy);
	vg_vertex2(vg, vx, vy);
	vg_end_element(vg);
	return (1);
}

const struct tool_ops vg_point_tool = {
	N_("Point"), N_("Trace an individual point."),
	VGPOINTS_ICON,
	sizeof(struct tool),
	0,
	point_tool_init,
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

/*	$Csoft: vg_point.c,v 1.3 2004/04/10 04:55:17 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#include <engine/mapedit/mapview.h>
#include <engine/mapedit/tool.h>
#endif

#include "vg.h"
#include "vg_primitive.h"

void
vg_point_radius(struct vg *vg, double radius)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_point.radius = radius;
}

void
vg_point_diameter(struct vg *vg, double diameter)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_point.radius = diameter/2;
}

void
vg_draw_points(struct vg *vg, struct vg_element *vge)
{
	struct vg_vertex *vtx;
	int rx, ry;

	for (vtx = &vge->vtx[0]; vtx < &vge->vtx[vge->nvtx]; vtx++) {
		vg_rcoords2(vg, vtx->x, vtx->y, &rx, &ry);
		vg_put_pixel(vg, rx, ry, vge->color);
	}
}

#ifdef EDITION
static void
point_tool_init(struct tool *t)
{
	tool_push_status(t, _("Specify point.\n"));
}

static void
point_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txorel, int tyorel, int b)
{
	struct vg *vg = t->p;
	
	vg_vcoords2(vg, tx, ty, txoff, tyoff, &vg->origin[1].x,
	    &vg->origin[1].y);
	vg_rasterize(vg);
}

static void
point_mousebuttondown(struct tool *t, int tx, int ty, int txoff, int tyoff,
    int b)
{
	struct vg *vg = t->p;
	double vx, vy;

	vg_begin(vg, VG_POINTS);
	vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
	vg_vertex2(vg, vx, vy);
	vg_rasterize(vg);
}

const struct tool point_tool = {
	N_("Points"),
	N_("Trace individual points."),
	VGPOINTS_ICON,
	-1,
	point_tool_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	point_mousemotion,
	point_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

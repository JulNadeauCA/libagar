/*	$Csoft: vg_circle.c,v 1.4 2004/04/19 02:07:37 vedge Exp $	*/

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
#include "vg_math.h"

void
vg_circle_radius(struct vg *vg, double radius)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_circle.radius = radius;
}

void
vg_circle_diameter(struct vg *vg, double diameter)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_circle.radius = diameter/2;
}

void
vg_draw_circle(struct vg *vg, struct vg_element *vge)
{
	int rx, ry, radius;

	vg_rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &rx, &ry);
	vg_rlength(vg, vge->vg_circle.radius, &radius);
	vg_circle_primitive(vg, rx, ry, radius, vge->color);
}

void
vg_circle_bbox(struct vg *vg, struct vg_element *vge, struct vg_rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_circle.radius;
	r->y = vge->vtx[0].y - vge->vg_circle.radius;
	r->w = vge->vg_circle.radius*2;
	r->h = vge->vg_circle.radius*2;
}

#ifdef EDITION
static struct vg_element *cur_circle;
static struct vg_vertex *cur_radius;
static int seq;

static void
circle_tool_init(struct tool *t)
{
	tool_push_status(t, _("Specify the circle's center point.\n"));
	seq = 0;
	cur_circle = NULL;
	cur_radius = NULL;
}

static void
circle_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txorel, int tyorel, int b)
{
	struct vg *vg = t->p;
	double x, y;
	
	vg_vcoords2(vg, tx, ty, txoff, tyoff, &x, &y);

	if (cur_radius != NULL) {
		cur_radius->x = x;
		cur_radius->y = y;
		cur_circle->vg_circle.radius =
		    sqrt(pow(x-vg->origin[2].x,2) +
		         pow(y-vg->origin[2].y,2));
	} else {
		vg->origin[2].x = x;
		vg->origin[2].y = y;
	}
	if (cur_circle != NULL) {
		cur_circle->redraw++;
		vg_rasterize(vg);
	}
	vg->origin[1].x = x;
	vg->origin[1].y = y;
}

static void
circle_mousebuttondown(struct tool *t, int tx, int ty, int txoff, int tyoff,
    int b)
{
	struct vg *vg = t->p;
	double vx, vy;

	switch (b) {
	case 1:
		if (seq++ == 0) {
			cur_circle = vg_begin(vg, VG_CIRCLE);
			vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
			vg_vertex2(vg, vx, vy);
			cur_radius = vg_vertex2(vg, vx, vy);
			tool_push_status(t, _("Specify the circle's radius "
			                      "or [undo circle].\n"));
		} else {
			goto finish;
		}
		break;
	default:
		if (cur_circle != NULL) {
			vg_undo_element(vg, cur_circle);
			cur_circle->redraw++;
			vg_rasterize(vg);
		}
		goto finish;
	}
	return;
finish:
	cur_circle = NULL;
	cur_radius = NULL;
	seq = 0;
	tool_pop_status(t);
}

const struct tool circle_tool = {
	N_("Circles"),
	N_("Draw circles."),
	VGCIRCLES_ICON,
	-1,
	circle_tool_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	circle_mousemotion,
	circle_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

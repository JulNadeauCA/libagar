/*	$Csoft: vg_ellipse.c,v 1.2 2004/04/20 01:05:43 vedge Exp $	*/

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
vg_ellipse_diameter2(struct vg *vg, double w, double h)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_arc.w = w;
	vge->vg_arc.h = h;
}

void
vg_draw_ellipse(struct vg *vg, struct vg_element *vge)
{
	int x, y;
	int w, h;
	int s, e;

	vg_rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &x, &y);
	vg_rlength(vg, vge->vg_arc.w, &w);
	vg_rlength(vg, vge->vg_arc.h, &h);
	vg_rlength(vg, vge->vg_arc.s, &s);
	vg_rlength(vg, vge->vg_arc.e, &e);
	vg_arc_primitive(vg, x, y, w, h, s, e, vge->color);
}

void
vg_ellipse_bbox(struct vg *vg, struct vg_element *vge, struct vg_rect *r)
{
	r->x = vge->vtx[0].x - vge->vg_arc.w/2;
	r->y = vge->vtx[0].y - vge->vg_arc.h/2;
	r->w = vge->vg_arc.w;
	r->h = vge->vg_arc.h;
}

#ifdef EDITION
static struct vg_element *cur_ellipse;
static int seq;

static void
ellipse_tool_init(struct tool *t)
{
	seq = 0;
	cur_ellipse = NULL;
	tool_push_status(t, _("Specify the ellipse's center point.\n"));
}

static void
ellipse_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txorel, int tyorel, int b)
{
	struct vg *vg = t->p;
	double x, y;
	
	vg_vcoords2(vg, tx, ty, txoff, tyoff, &x, &y);

	if (cur_ellipse != NULL) {
		if (seq == 1) {
			cur_ellipse->vg_arc.w = x - vg->origin[2].x;
			cur_ellipse->vg_arc.h = y - vg->origin[2].y;
		} 
		cur_ellipse->redraw++;
		vg_rasterize(vg);
	} else {
		vg->origin[2].x = x;
		vg->origin[2].y = y;
	}
	vg->origin[1].x = x;
	vg->origin[1].y = y;
}

static void
ellipse_mousebuttondown(struct tool *t, int tx, int ty, int txoff, int tyoff,
    int b)
{
	struct vg *vg = t->p;
	double vx, vy;

	switch (b) {
	case 1:
		switch (seq++) {
		case 0:
			cur_ellipse = vg_begin(vg, VG_ELLIPSE);
			vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
			vg_vertex2(vg, vx, vy);
			tool_push_status(t, _("Specify the ellipse's geometry "
			                      "or [undo ellipse].\n"));
			break;
		default:
			goto finish;
		}
		break;
	default:
		if (cur_ellipse != NULL) {
			vg_undo_element(vg, cur_ellipse);
			cur_ellipse->redraw++;
			vg_rasterize(vg);
		}
		goto finish;
	}
	return;
finish:
	cur_ellipse = NULL;
	seq = 0;
	tool_pop_status(t);
}

const struct tool ellipse_tool = {
	N_("Ellipses"),
	N_("Draw ellipses."),
	VGCIRCLES_ICON,
	-1,
	ellipse_tool_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	ellipse_mousemotion,
	ellipse_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

/*	$Csoft: vg_origin.c,v 1.1 2004/04/17 00:43:39 vedge Exp $	*/

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

#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/spinbutton.h>
#endif

#include "vg.h"
#include "vg_primitive.h"

void
vg_origin2(struct vg *vg, int o, double ox, double oy)
{
	vg_origin3(vg, o, ox, oy, 0);
}

void
vg_origin3(struct vg *vg, int o, double ox, double oy, double oz)
{
	struct vg_element *vge;
	struct vg_vertex *vtx;
	int i;

	if (o == 0) {
		TAILQ_FOREACH(vge, &vg->vges, vges) {
			for (i = 0; i < vge->nvtx; i++) {
				struct vg_vertex *vtx = &vge->vtx[i];

				vtx->x -= ox - vg->origin[o].x;
				vtx->y -= oy - vg->origin[o].y;
				vtx->z -= oz - vg->origin[o].z;
			}
		}
	}
	vg->origin[o].x = ox;
	vg->origin[o].y = oy;
	vg->origin[o].z = oz;
}

void
vg_draw_origin(struct vg *vg)
{
	int rx, ry, radius;
	int o;

	for (o = 0; o < vg->norigin; o++) {
		if (o == 0) {
			vg_arcoords2(vg, vg->origin[o].x, vg->origin[o].y,
			    &rx, &ry);
		} else {
			vg_rcoords2(vg, vg->origin[o].x, vg->origin[o].y,
			    &rx, &ry);
		}
		vg_rlength(vg, vg->origin_radius[o], &radius);
		vg_circle_primitive(vg, rx, ry, radius,
		    vg->origin_color[o]);
		vg_line_primitive(vg, rx, ry, rx+radius, ry,
		    vg->origin_color[o]);
		vg_line_primitive(vg, rx, ry, rx, ry+radius,
		    vg->origin_color[o]);
	}
}

#ifdef EDITION
static int norigin = 0;

static void
origin_tool_init(struct tool *t)
{
	struct window *win;
	struct spinbutton *sbu;

	win = tool_window(t, "vg-tool-origin");
	sbu = spinbutton_new(win, _("Origin#: "));
	widget_bind(sbu, "value", WIDGET_INT, &norigin);
	spinbutton_set_range(sbu, 0, VG_NORIGINS-1);

	tool_push_status(t, _("Specify origin point.\n"));
}

static void
origin_mousebuttondown(struct tool *t, int tx, int ty, int txoff,
    int tyoff, int b)
{
	struct vg *vg = t->p;
	double x, y;

	if (b == 1) {
		vg_avcoords2(vg, tx, ty, txoff, tyoff, &x, &y);
		vg_origin2(vg, norigin, x, y);
		vg_rasterize(vg);
	}
}

static void
origin_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txorel, int tyorel, int b)
{
	struct vg *vg = t->p;
	double x, y;

	if (b & SDL_BUTTON(1)) {
		vg_avcoords2(vg, tx, ty, txoff, tyoff, &x, &y);
		vg_origin2(vg, norigin, x, y);
		vg_rasterize(vg);
	}
}

const struct tool origin_tool = {
	N_("Origin"),
	N_("Displace the origin point."),
	VGORIGIN_ICON,
	-1,
	origin_tool_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	origin_mousemotion,
	origin_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

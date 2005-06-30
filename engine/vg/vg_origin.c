/*	$Csoft: vg_origin.c,v 1.14 2005/06/16 05:20:03 vedge Exp $	*/

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
	struct vg_block *vgb;
	struct vg_vertex *vtx;
	int i;

	if (o == 0) {
		if (vg->cur_block != NULL) {
			TAILQ_FOREACH(vge, &vg->cur_block->vges, vgbmbs) {
				for (i = 0; i < vge->nvtx; i++) {
					struct vg_vertex *vtx = &vge->vtx[i];

					vtx->x -= ox - vg->origin[0].x;
					vtx->y -= oy - vg->origin[0].y;
					vtx->z -= oz - vg->origin[0].z;
				}
			}
			vg->cur_block->origin.x = ox;
			vg->cur_block->origin.y = oy;
			vg->cur_block->origin.z = oz;
			return;
		}

		TAILQ_FOREACH(vge, &vg->vges, vges) {
			for (i = 0; i < vge->nvtx; i++) {
				struct vg_vertex *vtx = &vge->vtx[i];

				vtx->x -= ox - vg->origin[0].x;
				vtx->y -= oy - vg->origin[0].y;
				vtx->z -= oz - vg->origin[0].z;
			}
		}
		TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
			vgb->pos.x -= ox - vg->origin[0].x;
			vgb->pos.y -= oy - vg->origin[0].y;
			vgb->pos.z -= oz - vg->origin[0].z;
		}
		for (i = 1; i < vg->norigin; i++) {
			vg->origin[i].x -= ox - vg->origin[0].x;
			vg->origin[i].y -= oy - vg->origin[0].y;
			vg->origin[i].z -= oz - vg->origin[0].z;
		}
		vg->origin[0].x = ox;
		vg->origin[0].y = oy;
		vg->origin[0].z = oz;
	} else {
		vg->origin[o].x = ox - vg->origin[0].x;
		vg->origin[o].y = oy - vg->origin[0].y;
		vg->origin[o].z = oz - vg->origin[0].z;
	}
}

void
vg_origin_color(struct vg *vg, int o, int r, int g, int b)
{
	vg->origin_color[o] = SDL_MapRGB(vg->fmt, r, g, b);
}

void
vg_origin_radius(struct vg *vg, int o, float r)
{
	vg->origin_radius[o] = r;
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

	tool_push_status(t, _("Specify origin point."));
}

static int
origin_mousebuttondown(struct tool *t, int xmap, int ymap, int btn)
{
	struct vg *vg = t->p;
	double x, y;

	if (btn == 1) {
		vg_map2veca(vg, xmap, ymap, &x, &y);
		vg_origin2(vg, norigin, x, y);
		vg->redraw++;
	}
	return (1);
}

static int
origin_mousemotion(struct tool *t, int xmap, int ymap, int xrel, int yrel,
    int btn)
{
	struct vg *vg = t->p;
	double x, y;

	if (btn & SDL_BUTTON(1)) {
		vg_map2veca(vg, xmap, ymap, &x, &y);
		vg_origin2(vg, norigin, x, y);
		vg->redraw++;
	}
	return (1);
}

struct tool vg_origin_tool = {
	N_("Origin"),
	N_("Displace the origin point."),
	VGORIGIN_ICON, -1,
	0,
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

/*	$Csoft: vg_origin.c,v 1.16 2005/07/30 05:01:34 vedge Exp $	*/

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
VG_Origin2(VG *vg, int o, double ox, double oy)
{
	VG_Origin3(vg, o, ox, oy, 0);
}

void
VG_Origin3(VG *vg, int o, double ox, double oy, double oz)
{
	VG_Element *vge;
	VG_Block *vgb;
	VG_Vtx *vtx;
	int i;

	if (o == 0) {
		if (vg->cur_block != NULL) {
			TAILQ_FOREACH(vge, &vg->cur_block->vges, vgbmbs) {
				for (i = 0; i < vge->nvtx; i++) {
					VG_Vtx *vtx = &vge->vtx[i];

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
				VG_Vtx *vtx = &vge->vtx[i];

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
VG_OriginColor(VG *vg, int o, int r, int g, int b)
{
	vg->origin_color[o] = SDL_MapRGB(vg->fmt, r, g, b);
}

void
VG_OriginRadius(VG *vg, int o, float r)
{
	vg->origin_radius[o] = r;
}

void
VG_DrawOrigin(VG *vg)
{
	int rx, ry, radius;
	int o;

	for (o = 0; o < vg->norigin; o++) {
		if (o == 0) {
			VG_AbsRcoords2(vg, vg->origin[o].x, vg->origin[o].y,
			    &rx, &ry);
		} else {
			VG_Rcoords2(vg, vg->origin[o].x, vg->origin[o].y,
			    &rx, &ry);
		}
		VG_RLength(vg, vg->origin_radius[o], &radius);
		VG_CirclePrimitive(vg, rx, ry, radius,
		    vg->origin_color[o]);
		VG_LinePrimitive(vg, rx, ry, rx+radius, ry,
		    vg->origin_color[o]);
		VG_LinePrimitive(vg, rx, ry, rx, ry+radius,
		    vg->origin_color[o]);
	}
}

#ifdef EDITION
static int norigin = 0;

static void
origin_AG_MaptoolInit(void *t)
{
	AG_MaptoolPushStatus(t, _("Specify origin point."));
}

static void
origin_tool_pane(void *t, void *con)
{
	AG_Spinbutton *sbu;

	sbu = AG_SpinbuttonNew(con, _("Origin#: "));
	AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &norigin);
	AG_SpinbuttonSetRange(sbu, 0, VG_NORIGINS-1);
}

static int
origin_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	VG *vg = TOOL(t)->p;
	double x, y;

	if (btn == 1) {
		VG_Map2VecAbs(vg, xmap, ymap, &x, &y);
		VG_Origin2(vg, norigin, x, y);
		vg->redraw++;
	}
	return (1);
}

static int
origin_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel,
    int btn)
{
	VG *vg = TOOL(t)->p;
	double x, y;

	if (btn & SDL_BUTTON(1)) {
		VG_Map2VecAbs(vg, xmap, ymap, &x, &y);
		VG_Origin2(vg, norigin, x, y);
		vg->redraw++;
	}
	return (1);
}

const AG_MaptoolOps vgOriginTool = {
	"Origin", N_("Displace the origin point."),
	VGORIGIN_ICON,
	sizeof(AG_Maptool),
	0,
	origin_AG_MaptoolInit,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */
	
	origin_mousemotion,
	origin_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

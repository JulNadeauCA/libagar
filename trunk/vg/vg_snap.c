/*	$Csoft: vg_snap.c,v 1.11 2005/09/27 00:25:21 vedge Exp $	*/

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

#include <core/core.h>

#include <gui/toolbar.h>
#include <gui/button.h>
#ifdef EDITION
#include <gui/menu.h>
#endif

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

static void
snap_to_grid(VG *vg, float *x, float *y)
{
	float gx, gy;
	float xoff, yoff;
	int xsign = *x >= 0 ? 1 : -1;
	int ysign = *y >= 0 ? 1 : -1;

	/* XXX bletcherous */
	for (gx = 0; gx <= fabsf(*x)-vg->grid_gap; gx += vg->grid_gap)
	    ;;
	for (gy = 0; gy <= fabsf(*y)-vg->grid_gap; gy += vg->grid_gap)
	    ;;

	xoff = fabsf(*x) - gx;
	yoff = fabsf(*y) - gy;

	if (xsign == 1) {
		xoff = *x - gx;
		*x = gx;
		if (xoff > vg->grid_gap/2)
			*x += vg->grid_gap;
	} else {
		xoff = fabsf(*x) - gx;
		*x = -gx;
		if (xoff > vg->grid_gap/2)
			*x -= vg->grid_gap;
	}

	if (ysign == 1) {
		yoff = *y - gy;
		*y = gy;
		if (yoff > vg->grid_gap/2)
			*y += vg->grid_gap;
	} else {
		yoff = fabsf(*y) - gy;
		*y = -gy;
		if (yoff > vg->grid_gap/2)
			*y -= vg->grid_gap;
	}
}

static void
snap_to_endpoint(VG *vg, float *x, float *y)
{
	VG_Element *vge;
	VG_Vtx *vtx;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vtx = &vge->vtx[i];

			/* XXX */
			if (*x > vtx->x - 0.5 && *x < vtx->x + 0.5 &&
			    *y > vtx->y - 0.5 && *y < vtx->y + 0.5) {
				*x = vtx->x;
				*y = vtx->y;
				break;
			}
		}
	}
}

void
VG_SnapPoint(VG *vg, float *x, float *y)
{
	switch (vg->snap_mode) {
	case VG_NEAREST_INTEGER:
		*x = rint(*x);
		*y = rint(*y);
		break;
	case VG_GRID:
		snap_to_grid(vg, x, y);
		break;
	case VG_ENDPOINT:
		snap_to_endpoint(vg, x, y);
		break;
	case VG_FREE_POSITIONING:
		break;
	default:
		break;
	}
}

void
VG_SetSnapMode(VG *vg, enum vg_snap_mode mode)
{
	vg->snap_mode = mode;
}

/* XXX cache inefficient */
void
VG_DrawGrid(VG *vg)
{
	int x, y, xoffs, yoffs;
	int rlen;

	VG_RLength(vg, vg->grid_gap, &rlen);
	xoffs = vg->rDst.x % rlen;
	yoffs = vg->rDst.y % rlen;
	
	if (vg->flags & VG_DIRECT) {
		for (y = yoffs; (y+rlen) < vg->su->h; y += rlen) {
			for (x = xoffs; (x+rlen) < vg->su->w; x += rlen)
				VG_PutPixel(vg, x, y, vg->grid_color);
		}
	} else {
		int x2 = vg->rDst.x+vg->rDst.w;
		int y2 = vg->rDst.y+vg->rDst.h;

		if (vg->rDst.w <= rlen || vg->rDst.h <= rlen) {
			return;
		}
		for (y = vg->rDst.y; (y+rlen) < y2; y += rlen) {
			for (x = vg->rDst.x; (x+rlen) < x2; x += rlen)
				VG_PutPixel(vg, x, y, vg->grid_color);
		}
	}
}

static void
snap_to(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_Toolbar *tbar = AG_PTR(1);
	VG *vg = AG_PTR(2);
	int snap_mode = AG_INT(3);

	AG_ToolbarSelectUnique(tbar, bu);
	VG_SetSnapMode(vg, snap_mode);
}

static void
snap_to_m(AG_Event *event)
{
	VG *vg = AG_PTR(1);
	int snap_mode = AG_INT(2);

	VG_SetSnapMode(vg, snap_mode);
}

AG_Toolbar *
VG_SnapToolbar(void *parent, VG *vg, enum ag_toolbar_type ttype)
{
	AG_Toolbar *snbar;
	AG_Button *bu;

	snbar = AG_ToolbarNew(parent, ttype, 1, AG_TOOLBAR_HOMOGENOUS);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_FREE_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_FREE_POSITIONING);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_RINT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_NEAREST_INTEGER);
	bu = AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_GRID_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_GRID);
	AG_WidgetSetInt(bu, "state", 1);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_ENDPOINT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_ENDPOINT);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_ENDPOINT_D_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_ENDPOINT_DISTANCE);

	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_CLOSEST_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_CLOSEST_POINT);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_CENTERPT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_CENTER_POINT);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_MIDDLEPT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_MIDDLE_POINT);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_INTSECT_AUTO_ICON),
	    1, 0, snap_to, "%p,%p,%i", snbar, vg,
	    VG_INTERSECTIONS_AUTO);
	AG_ToolbarAddButton(snbar, 0, AGICON(SNAP_INTSECT_MANUAL_ICON),
	    1, 0, snap_to, "%p,%p,%i", snbar, vg,
	    VG_INTERSECTIONS_MANUAL);
	return (snbar);
}

void
VG_SnapMenu(AG_Menu *m, AG_MenuItem *mi, VG *vg)
{
	AG_MenuAction(mi, _("Free positioning"), SNAP_FREE_ICON,
	    snap_to_m, "%p,%i", vg, VG_FREE_POSITIONING);
	AG_MenuAction(mi, _("Grid"), SNAP_GRID_ICON,
	    snap_to_m, "%p,%i", vg, VG_GRID);
	AG_MenuAction(mi, _("Nearest integer"), SNAP_RINT_ICON,
	    snap_to_m, "%p,%i", vg, VG_NEAREST_INTEGER);
	
	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Endpoint"), SNAP_ENDPOINT_ICON,
	    snap_to_m, "%p,%i", vg, VG_ENDPOINT);
	AG_MenuAction(mi, _("Distance from endpoint"), SNAP_ENDPOINT_D_ICON,
	    snap_to_m, "%p,%i", vg, VG_ENDPOINT_DISTANCE);
	AG_MenuAction(mi, _("Closest point"), SNAP_CLOSEST_ICON,
	    snap_to_m, "%p,%i", vg, VG_CLOSEST_POINT);
	AG_MenuAction(mi, _("Center point"), SNAP_CENTERPT_ICON,
	    snap_to_m, "%p,%i", vg, VG_CENTER_POINT);
	AG_MenuAction(mi, _("Middle point"), SNAP_MIDDLEPT_ICON,
	    snap_to_m, "%p,%i", vg, VG_MIDDLE_POINT);

	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Intersections (auto)"), SNAP_INTSECT_AUTO_ICON,
	    snap_to_m, "%p,%i", vg, VG_INTERSECTIONS_AUTO);
	AG_MenuAction(mi, _("Intersections (manual)"),SNAP_INTSECT_MANUAL_ICON,
	    snap_to_m, "%p,%i", vg, VG_INTERSECTIONS_MANUAL);
}

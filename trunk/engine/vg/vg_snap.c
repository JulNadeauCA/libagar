/*	$Csoft: vg_snap.c,v 1.10 2005/03/05 12:14:04 vedge Exp $	*/

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

#include <engine/widget/toolbar.h>
#include <engine/widget/button.h>
#ifdef EDITION
#include <engine/widget/menu.h>
#endif

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

static void
snap_to_grid(VG *vg, double *x, double *y)
{
	double gx, gy;
	double xoff, yoff;
	int xsign = *x >= 0 ? 1 : -1;
	int ysign = *y >= 0 ? 1 : -1;

	/* XXX bletcherous */
	for (gx = 0; gx <= fabs(*x)-vg->grid_gap; gx += vg->grid_gap)
	    ;;
	for (gy = 0; gy <= fabs(*y)-vg->grid_gap; gy += vg->grid_gap)
	    ;;

	xoff = fabs(*x) - gx;
	yoff = fabs(*y) - gy;

	if (xsign == 1) {
		xoff = *x - gx;
		*x = gx;
		if (xoff > vg->grid_gap/2)
			*x += vg->grid_gap;
	} else {
		xoff = fabs(*x) - gx;
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
		yoff = fabs(*y) - gy;
		*y = -gy;
		if (yoff > vg->grid_gap/2)
			*y -= vg->grid_gap;
	}
}

static void
snap_to_endpoint(VG *vg, double *x, double *y)
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
VG_SnapPoint(VG *vg, double *x, double *y)
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

void
VG_DrawGrid(VG *vg)
{
	int x, y;
	int rlen;

	VG_RLength(vg, vg->grid_gap, &rlen);

	for (y = 0; y < vg->su->h; y += rlen) {
		for (x = 0; x < vg->su->w; x += rlen) {
			VG_PutPixel(vg, x, y, vg->grid_color);
		}
	}
}

static void
snap_to(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;
	AG_Toolbar *tbar = argv[1].p;
	VG *vg = argv[2].p;
	int snap_mode = argv[3].i;

	AG_ToolbarSelectUnique(tbar, bu);
	VG_SetSnapMode(vg, snap_mode);
}

static void
snap_to_m(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;
	int snap_mode = argv[2].i;

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

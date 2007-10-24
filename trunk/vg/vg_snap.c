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
#include "icons.h"

static void
VG_SnapToGrid(VG *vg, float *x, float *y)
{
	float r;

	if (x != NULL) {
		r = remainderf(*x, vg->grid_gap);
		*x -= r;
		if (r > 0.5) { *x += vg->grid_gap; }
	}
	if (y != NULL) {
		r = remainderf(*y, vg->grid_gap);
		*y -= r;
		if (r > 0.5) { *y += vg->grid_gap; }
	}
}

static void
VG_SnapToEndpoint(VG *vg, float *x, float *y)
{
	VG_Element *vge;
	VG_Vtx *vtx;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vtx = &vge->vtx[i];
			/* TODO */
		}
	}
}

void
VG_SnapPoint(VG *vg, float *x, float *y)
{
	switch (vg->snap_mode) {
	case VG_NEAREST_INTEGER:
		if (x != NULL) *x = rint(*x);
		if (y != NULL) *y = rint(*y);
		break;
	case VG_GRID:
		VG_SnapToGrid(vg, x, y);
		break;
	case VG_ENDPOINT:
		VG_SnapToEndpoint(vg, x, y);
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

	AG_ToolbarSelectOnly(tbar, bu);
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

	snbar = AG_ToolbarNew(parent, ttype, 1, AG_TOOLBAR_HOMOGENOUS|
	                                        AG_TOOLBAR_STICKY);
	AG_ToolbarButtonIcon(snbar, vgIconSnapFree.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_FREE_POSITIONING);
	AG_ToolbarButtonIcon(snbar, vgIconSnapRint.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_NEAREST_INTEGER);
	AG_ToolbarButtonIcon(snbar, vgIconSnapGrid.s, 1,
	    snap_to, "%p,%p,%i", snbar, vg, VG_GRID);
	AG_ToolbarButtonIcon(snbar, vgIconSnapEndpt.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_ENDPOINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapEndptDist.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_ENDPOINT_DISTANCE);

	AG_ToolbarButtonIcon(snbar, vgIconSnapClosest.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_CLOSEST_POINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapCenterPt.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_CENTER_POINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapMiddlePt.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_MIDDLE_POINT);
	AG_ToolbarButtonIcon(snbar, vgIconSnapIntsectAuto.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_INTERSECTIONS_AUTO);
	AG_ToolbarButtonIcon(snbar, vgIconSnapIntsectManual.s, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_INTERSECTIONS_MANUAL);
	return (snbar);
}

void
VG_SnapMenu(AG_Menu *m, AG_MenuItem *mi, VG *vg)
{
	AG_MenuAction(mi, _("Free positioning"), vgIconSnapFree.s,
	    snap_to_m, "%p,%i", vg, VG_FREE_POSITIONING);
	AG_MenuAction(mi, _("Grid"), vgIconSnapGrid.s,
	    snap_to_m, "%p,%i", vg, VG_GRID);
	AG_MenuAction(mi, _("Nearest integer"), vgIconSnapRint.s,
	    snap_to_m, "%p,%i", vg, VG_NEAREST_INTEGER);
	
	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Endpoint"), vgIconSnapEndpt.s,
	    snap_to_m, "%p,%i", vg, VG_ENDPOINT);
	AG_MenuAction(mi, _("Distance from endpoint"), vgIconSnapEndptDist.s,
	    snap_to_m, "%p,%i", vg, VG_ENDPOINT_DISTANCE);
	AG_MenuAction(mi, _("Closest point"), vgIconSnapClosest.s,
	    snap_to_m, "%p,%i", vg, VG_CLOSEST_POINT);
	AG_MenuAction(mi, _("Center point"), vgIconSnapCenterPt.s,
	    snap_to_m, "%p,%i", vg, VG_CENTER_POINT);
	AG_MenuAction(mi, _("Middle point"), vgIconSnapMiddlePt.s,
	    snap_to_m, "%p,%i", vg, VG_MIDDLE_POINT);

	AG_MenuSeparator(mi);

	AG_MenuAction(mi, _("Intersections (auto)"), vgIconSnapIntsectAuto.s,
	    snap_to_m, "%p,%i", vg, VG_INTERSECTIONS_AUTO);
	AG_MenuAction(mi, _("Intersections (manual)"),vgIconSnapIntsectManual.s,
	    snap_to_m, "%p,%i", vg, VG_INTERSECTIONS_MANUAL);
}

/*	$Csoft: vg_snap.c,v 1.3 2004/04/26 07:03:46 vedge Exp $	*/

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

#include <engine/widget/toolbar.h>
#include <engine/widget/button.h>

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

static void
snap_to_grid(struct vg *vg, double *x, double *y)
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
snap_to_endpoint(struct vg *vg, double *x, double *y)
{
	struct vg_element *vge;
	struct vg_vertex *vtx;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vtx = &vge->vtx[i];

			if (vg_near_vertex2(vg, vtx, *x, *y, 0.25)) {
				*x = vtx->x;
				*y = vtx->y;
				break;
			}
		}
	}
}

void
vg_snap_to(struct vg *vg, double *x, double *y)
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
vg_snap_mode(struct vg *vg, enum vg_snap_mode mode)
{
	vg->snap_mode = mode;
}

void
vg_draw_grid(struct vg *vg)
{
	int x, y;
	int rlen;

	vg_rlength(vg, vg->grid_gap, &rlen);

	for (y = 0; y < vg->su->h; y += rlen) {
		for (x = 0; x < vg->su->w; x += rlen) {
			vg_put_pixel(vg, x, y, vg->grid_color);
		}
	}
}

static void
snap_to(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct toolbar *tbar = argv[1].p;
	struct vg *vg = argv[2].p;
	int snap_mode = argv[3].i;

	toolbar_select_unique(tbar, bu);
	vg_snap_mode(vg, snap_mode);
}

struct toolbar *
vg_snap_toolbar(void *parent, struct vg *vg, enum toolbar_type ttype)
{
	struct toolbar *snbar;
	struct button *bu;

	snbar = toolbar_new(parent, ttype, 1);
	toolbar_add_button(snbar, 0, ICON(SNAP_FREE_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_FREE_POSITIONING);
	toolbar_add_button(snbar, 0, ICON(SNAP_RINT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_NEAREST_INTEGER);
	bu = toolbar_add_button(snbar, 0, ICON(SNAP_GRID_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_GRID);
	widget_set_int(bu, "state", 1);
	toolbar_add_button(snbar, 0, ICON(SNAP_ENDPOINT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_ENDPOINT);
	toolbar_add_button(snbar, 0, ICON(SNAP_ENDPOINT_D_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_ENDPOINT_DISTANCE);

	toolbar_add_button(snbar, 0, ICON(SNAP_CLOSEST_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_CLOSEST_POINT);
	toolbar_add_button(snbar, 0, ICON(SNAP_CENTERPT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_CENTER_POINT);
	toolbar_add_button(snbar, 0, ICON(SNAP_MIDDLEPT_ICON), 1, 0,
	    snap_to, "%p,%p,%i", snbar, vg, VG_MIDDLE_POINT);
	toolbar_add_button(snbar, 0, ICON(SNAP_INTSECT_AUTO_ICON),
	    1, 0, snap_to, "%p,%p,%i", snbar, vg,
	    VG_INTERSECTIONS_AUTO);
	toolbar_add_button(snbar, 0, ICON(SNAP_INTSECT_MANUAL_ICON),
	    1, 0, snap_to, "%p,%p,%i", snbar, vg,
	    VG_INTERSECTIONS_MANUAL);
	return (snbar);
}

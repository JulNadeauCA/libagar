/*	$Csoft: vg_line.c,v 1.1 2004/03/30 16:03:58 vedge Exp $	*/

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
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/mapedit/mapview.h>
#include <engine/mapedit/tool.h>
#endif

#include "vg.h"
#include "vg_primitive.h"
#include "vgedit.h"

void
vg_draw_lines(struct vg *vg, struct vg_element *vge)
{
	int x1, y1, x2, y2;
	int i;

	for (i = 0; i < vge->nvtx-1; i += 2) {
		vg_rcoords(vg, vge->vtx[i].x, vge->vtx[i].y, &x1, &y1);
		vg_rcoords(vg, vge->vtx[i+1].x, vge->vtx[i+1].y, &x2, &y2);

		vg_line_primitive(vg, x1, y1, x2, y2, vge->color);
	}
}

void
vg_draw_line_strip(struct vg *vg, struct vg_element *vge)
{
	double vx = vge->vtx[0].x;
	double vy = vge->vtx[0].y;
	double px = vx;
	double py = vy;
	int x1, y1, x2, y2;
	int i;

	for (i = 1; i < vge->nvtx; i++) {
		vx += vge->vtx[i].x;
		vy += vge->vtx[i].y;

		vg_rcoords(vg, px, py, &x1, &y1);
		vg_rcoords(vg, vx, vy, &x2, &y2);
		vg_line_primitive(vg, x1, y1, x2, y2, vge->color);

		px = vx;
		py = vy;
	}
}

void
vg_draw_line_loop(struct vg *vg, struct vg_element *vge)
{
}

#ifdef EDITION
static int mode = 0;
static int seq = 0;

static void
line_init(struct tool *t)
{
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	win = tool_window(t, "vgedit-tool-line");
	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &mode);
}

static void
line_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txoffrel, int tyoffrel, int b)
{
	struct vg *vg = t->p;
	double vx, vy;
	
	vg_vcoords(vg, tx, ty, txoff, tyoff, &vx, &vy);
	mapview_status(t->mv, "%d+%d,%d+%d -> %.2f,%.2f", tx, txoff, ty, tyoff,
	    vx, vy);
}

static void
line_mousebuttondown(struct tool *t, int tx, int ty, int txoff, int tyoff,
    int b)
{
	struct vg *vg = t->p;
	double vx, vy;

	switch (seq++) {
	case 0:
		vg_begin(vg, VG_LINES);
		vg_vcoords(vg, tx, ty, txoff, tyoff, &vx, &vy);
		vg_vertex2(vg, vx, vy);
		mapview_status(t->mv, "%.2f,%.2f ->", vx, vy);
		break;
	case 1:
		vg_vcoords(vg, tx, ty, txoff, tyoff, &vx, &vy);
		vg_vertex2(vg, vx, vy);
		vg_end(vg);
		vg_rasterize(vg);
		seq = 0;
		mapview_status(t->mv, "-> %.2f,%.2f", vx, vy);
		break;
	}
}

const struct tool line_tool = {
	N_("Line segments"),
	N_("Draw line segments, strips and loops."),
	VGLINES_ICON,
	-1,
	line_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	line_mousemotion,
	line_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

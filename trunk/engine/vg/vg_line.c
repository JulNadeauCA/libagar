/*	$Csoft: vg_line.c,v 1.4 2004/04/11 03:28:43 vedge Exp $	*/

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
#include "vgobj.h"

void
vg_draw_lines(struct vg *vg, struct vg_element *vge)
{
	int x1, y1, x2, y2;
	int i;

	for (i = 0; i < vge->nvtx-1; i += 2) {
		vg_rcoords2(vg, vge->vtx[i].x, vge->vtx[i].y, &x1, &y1);
		vg_rcoords2(vg, vge->vtx[i+1].x, vge->vtx[i+1].y, &x2, &y2);
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
		vx = vge->vtx[i].x;
		vy = vge->vtx[i].y;

		vg_rcoords2(vg, px, py, &x1, &y1);
		vg_rcoords2(vg, vx, vy, &x2, &y2);
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
static enum {
	MODE_SEGMENTS,
	MODE_STRIP,
	MODE_LOOP
} mode = MODE_STRIP;

static int seq;
static struct vg_element *cur_line;
static struct vg_vertex *cur_vtx;

static void
line_tool_init(struct tool *t)
{
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	struct window *win;
	struct radio *rad;

	win = tool_window(t, "vg-tool-line");
	rad = radio_new(win, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &mode);
	
	tool_push_status(t, _("Specify first point.\n"));
	seq = 0;
	cur_line = NULL;
	cur_vtx = NULL;
}

static void
line_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txorel, int tyorel, int b)
{
	struct vg *vg = t->p;
	double x, y;
	
	vg_vcoords2(vg, tx, ty, txoff, tyoff, &x, &y);
	vg->origin[1].x = x;
	vg->origin[1].y = y;

	if (cur_vtx != NULL) {
		cur_vtx->x = x;
		cur_vtx->y = y;
	}
	vg_rasterize(vg);
}

static void
line_mousebuttondown(struct tool *t, int tx, int ty, int txoff, int tyoff,
    int b)
{
	struct vg *vg = t->p;
	double vx, vy;

	switch (mode) {
	case MODE_SEGMENTS:
		if (b == 1) {
			if (seq++ == 0) {
				cur_line = vg_begin(vg, VG_LINES);
				vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
				vg_vertex2(vg, vx, vy);
				cur_vtx = vg_vertex2(vg, vx, vy);
				vg_rasterize(vg);
				tool_push_status(t, _("Specify second point "
				                      "or [undo line].\n"));
			} else {
				goto finish;
			}
		} else {
			if (cur_line != NULL) {
				vg_undo_element(vg, cur_line);
				vg_rasterize(vg);
			}
			goto finish;
		}
		break;
	case MODE_STRIP:
		if (b == 1) {
			if (seq++ == 0) {
				cur_line = vg_begin(vg, VG_LINE_STRIP);
				vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
				vg_vertex2(vg, vx, vy);
			} else {
				tool_pop_status(t);
			}
			vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
			cur_vtx = vg_vertex2(vg, vx, vy);
			vg_rasterize(vg);

			tool_push_status(t, _("Specify point %d or "
			                      "[close/undo vertex].\n"),
					      seq+1);
		} else {
			if (cur_vtx != NULL) {
				vg_pop_vertex(vg);
				vg_rasterize(vg);
			}
			goto finish;
		}
		break;
	default:
		break;
	}
	return;
finish:
	cur_line = NULL;
	cur_vtx = NULL;
	seq = 0;
	tool_pop_status(t);
}

const struct tool line_tool = {
	N_("Line segments"),
	N_("Draw line segments, strips and loops."),
	VGLINES_ICON,
	-1,
	line_tool_init,
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

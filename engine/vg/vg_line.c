/*	$Csoft: vg_line.c,v 1.27 2005/06/30 06:26:23 vedge Exp $	*/

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
#endif

#include "vg.h"
#include "vg_primitive.h"

void
vg_draw_line_segments(struct vg *vg, struct vg_element *vge)
{
	int i;

	for (i = 0; i < vge->nvtx-1; i += 2) {
		if (vg->flags & VG_ANTIALIAS) {
			double x1, y1, x2, y2;

			vg_vtxcoords2d(vg, &vge->vtx[i], &x1, &y1);
			vg_vtxcoords2d(vg, &vge->vtx[i+1], &x2, &y2);
			vg_wuline_primitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
		} else {
			int x1, y1, x2, y2;

			vg_vtxcoords2i(vg, &vge->vtx[i], &x1, &y1);
			vg_vtxcoords2i(vg, &vge->vtx[i+1], &x2, &y2);
			vg_line_primitive(vg, x1, y1, x2, y2, vge->color);
		}
	}
}

void
vg_draw_line_strip(struct vg *vg, struct vg_element *vge)
{
	int i;

	if (vg->flags & VG_ANTIALIAS) {
		double x1, y1, x2, y2;

		vg_vtxcoords2d(vg, &vge->vtx[0], &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			vg_vtxcoords2d(vg, &vge->vtx[i], &x2, &y2);
			vg_wuline_primitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
			x1 = x2;
			y1 = y2;
		}
	} else {
		int x1, y1, x2, y2;

		vg_vtxcoords2i(vg, &vge->vtx[0], &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			vg_vtxcoords2i(vg, &vge->vtx[i], &x2, &y2);
			vg_line_primitive(vg, x1, y1, x2, y2, vge->color);
			x1 = x2;
			y1 = y2;
		}
	}
}

void
vg_draw_line_loop(struct vg *vg, struct vg_element *vge)
{
	if (vg->flags & VG_ANTIALIAS) {
		double x1, y1, x2, y2;
		double x0, y0;
		int i;

		vg_vtxcoords2d(vg, &vge->vtx[0], &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			vg_vtxcoords2d(vg, &vge->vtx[i], &x2, &y2);
			vg_wuline_primitive(vg, x1, y1, x2, y2,
			    vge->line_st.thickness, vge->color);
			x1 = x2;
			y1 = y2;
		}
		vg_wuline_primitive(vg, x0, y0, x1, y1,
		    vge->line_st.thickness, vge->color);
	} else {
		int x1, y1, x2, y2;
		int x0, y0;
		int i;

		vg_vtxcoords2i(vg, &vge->vtx[0], &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			vg_vtxcoords2i(vg, &vge->vtx[i], &x2, &y2);
			vg_line_primitive(vg, x1, y1, x2, y2, vge->color);
			x1 = x2;
			y1 = y2;
		}
		vg_line_primitive(vg, x0, y0, x1, y1, vge->color);
	}
}

static void
extent(struct vg *vg, struct vg_element *vge, struct vg_rect *r)
{
	double xmin, xmax;
	double ymin, ymax;
	int i;

	xmin = xmax = vge->vtx[0].x;
	ymin = ymax = vge->vtx[0].y;
	for (i = 0; i < vge->nvtx; i++) {
		if (vge->vtx[i].x < xmin)
			xmin = vge->vtx[i].x;
		if (vge->vtx[i].y < ymin)
			ymin = vge->vtx[i].y;
		if (vge->vtx[i].x > xmax)
			xmax = vge->vtx[i].x;
		if (vge->vtx[i].y > ymax)
			ymax = vge->vtx[i].y;
	}
	r->x = xmin;
	r->y = ymin;
	r->w = xmax-xmin;
	r->h = ymax-ymin;
}

/*
 * Calculate the distance of the shortest path from a given point to any
 * point on a line.
 */
static float
intsect_line(struct vg *vg, int x1, int y1, int x2, int y2, int mx, int my)
{
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;
	float closest = DBL_MAX;
	double theta, rho;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if (dy <= dx) {
		d = dy*2 - dx;
		inc1 = dy*2;
		inc2 = (dy-dx)*2;
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydir = -1;
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydir = 1;
			xend = x2;
		}
		vg_car2pol(vg, mx-x, my-y, &rho, &theta);
		if (rho < closest) { closest = rho; }

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				vg_car2pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		} else {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y--;
					d += inc2;
				}
				vg_car2pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		}		
	} else {
		d = dx*2 - dy;
		inc1 = dx*2;
		inc2 = (dx-dy)*2;
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdir = -1;
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdir = 1;
		}
		vg_car2pol(vg, mx-x, my-y, &rho, &theta);
		if (rho < closest) { closest = rho; }

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				vg_car2pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		} else {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x--;
					d += inc2;
				}
				vg_car2pol(vg, mx-x, my-y, &rho, &theta);
				if (rho <= closest) {
					closest = rho;
				} else {
//					break;
				}
			}
		}
	}
	return (closest);
}

float
vg_line_intsect(struct vg *vg, struct vg_element *vge, double x, double y)
{
	int mx = VG_RASX(vg,x);
	int my = VG_RASX(vg,y);
	double rho, theta;
	float d, min_distance = FLT_MAX;
	struct vg_vertex v1, v2;
	int x1, y1, x2, y2, x0, y0;
	int i;

	switch (vge->type) {
	case VG_LINE_STRIP:
		vg_vtxcoords2i(vg, &vge->vtx[0], &x1, &y1);
		for (i = 1; i < vge->nvtx; i++) {
			vg_vtxcoords2i(vg, &vge->vtx[i], &x2, &y2);

			d = intsect_line(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }

			x1 = x2;
			y1 = y2;
		}
		break;
	case VG_LINES:
		for (i = 0; i < vge->nvtx-1; i+=2) {
			vg_vtxcoords2i(vg, &vge->vtx[i], &x1, &y1);
			vg_vtxcoords2i(vg, &vge->vtx[i+1], &x2, &y2);

			d = intsect_line(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }
		}
		break;
	case VG_LINE_LOOP:
	case VG_POLYGON:
		vg_vtxcoords2i(vg, &vge->vtx[0], &x1, &y1);
		x0 = x1;
		y0 = y1;
		for (i = 1; i < vge->nvtx; i++) {
			vg_vtxcoords2i(vg, &vge->vtx[i], &x2, &y2);

			d = intsect_line(vg, x1, y1, x2, y2, mx, my);
			if (d < min_distance) { min_distance = d; }
			
			x1 = x2;
			y1 = y2;
		}
		d = intsect_line(vg, x0, y0, x1, y1, mx, my);
		if (d < min_distance) { min_distance = d; }
		break;
	default:
		break;
	}
	return (VG_VECLENF(vg,min_distance));
}

const struct vg_element_ops vg_lines_ops = {
	N_("Line"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	vg_draw_line_segments,
	extent,
	vg_line_intsect
};
const struct vg_element_ops vg_line_strip_ops = {
	N_("Line strip"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	vg_draw_line_strip,
	extent,
	vg_line_intsect
};
const struct vg_element_ops vg_line_loop_ops = {
	N_("Line loop"),
	VGLINES_ICON,
	NULL,				/* init */
	NULL,				/* destroy */
	vg_draw_line_loop,
	extent,
	vg_line_intsect
};

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
line_tool_init(void *t)
{
	tool_push_status(t, _("Specify first point."));
	seq = 0;
	cur_line = NULL;
	cur_vtx = NULL;
}

static void
line_tool_pane(void *t, void *con)
{
	static const char *mode_items[] = {
		N_("Line segments"),
		N_("Line strip"),
		N_("Line loop"),
		NULL
	};
	struct radio *rad;

	rad = radio_new(con, mode_items);
	widget_bind(rad, "value", WIDGET_INT, &mode);
}

static int
line_mousemotion(void *t, int xmap, int ymap, int xrel, int yrel,
    int btn)
{
	struct vg *vg = TOOL(t)->p;
	double x, y;
	
	vg_map2vec(vg, xmap, ymap, &x, &y);
	vg->origin[1].x = x;
	vg->origin[1].y = y;

	if (cur_vtx != NULL) {
		cur_vtx->x = x;
		cur_vtx->y = y;
		vg->redraw++;
	}
	return (1);
}

static int
line_mousebuttondown(void *t, int xmap, int ymap, int btn)
{
	struct vg *vg = TOOL(t)->p;
	double vx, vy;

	switch (mode) {
	case MODE_SEGMENTS:
		if (btn == 1) {
			if (seq++ == 0) {
				cur_line = vg_begin_element(vg, VG_LINES);
				vg_map2vec(vg, xmap, ymap, &vx, &vy);
				vg_vertex2(vg, vx, vy);
				cur_vtx = vg_vertex2(vg, vx, vy);

				tool_push_status(t, _("Specify second point "
				                      "or [undo line]."));
			} else {
				goto finish;
			}
		} else {
			if (cur_line != NULL) {
				vg_destroy_element(vg, cur_line);
			}
			goto finish;
		}
		break;
	case MODE_STRIP:
	case MODE_LOOP:
		if (btn == 1) {
			if (seq++ == 0) {
#ifdef DEBUG
				if (vg->cur_block != NULL)
					fatal("block");
#endif
				cur_line = vg_begin_element(vg,
				    mode == MODE_STRIP ? VG_LINE_STRIP :
							 VG_LINE_LOOP);
				vg_map2vec(vg, xmap, ymap, &vx, &vy);
				vg_vertex2(vg, vx, vy);
			} else {
				tool_pop_status(t);
			}
			vg_map2vec(vg, xmap, ymap, &vx, &vy);
			cur_vtx = vg_vertex2(vg, vx, vy);
			vg->redraw++;

			tool_push_status(t, _("Specify point %d or "
			                      "[close/undo vertex]."), seq+1);
		} else {
			if (cur_vtx != NULL) {
				if (cur_line->nvtx <= 2) {
					vg_destroy_element(vg, cur_line);
					cur_line = NULL;
				} else {
					vg_pop_vertex(vg);
				}
				vg->redraw++;
			}
			goto finish;
		}
		break;
	default:
		break;
	}
	return (1);
finish:
	cur_line = NULL;
	cur_vtx = NULL;
	seq = 0;
	tool_pop_status(t);
	return (1);
}

const struct tool_ops vg_line_tool = {
	"Lines", N_("Line segments, strips and loops."),
	VGLINES_ICON,
	sizeof(struct tool),
	0,
	line_tool_init,
	NULL,			/* destroy */
	line_tool_pane,
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	line_mousemotion,
	line_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

/*	$Csoft: sketch_polygon.c,v 1.1 2005/06/01 09:08:45 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/vg/vg.h>
#include <engine/vg/vg_primitive.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/label.h>

#include "tileset.h"
#include "tileview.h"

struct polygon_tool {
	struct tileview_tool tool;
	enum {
		BEGIN_POLYGON,
		CONTINUE_POLYGON
	} seq;
	struct vg_element *polygon;
	struct vg_vertex *vtx;
	int preview;
};

static int
compare_ints(const void *p1, const void *p2)
{
	return (*(const int *)p1 - *(const int *)p2);
}

/* Special rendering function for sketch polygons with texturing. */
void
sketch_polygon_render(struct tile *t, struct vg *vg, struct vg_element *vge)
{
	struct texture *tex = NULL;
	struct vg_vertex *vtx = vge->vtx;
	u_int i, nvtx = vge->nvtx;
	int x, y, x1, y1, x2, y2;
	int miny, maxy;
	int ind1, ind2;
	int ints;

	if (vge->nvtx < 3 || vge->vg_polygon.outline) {	/* Draw outline */
		vg_draw_line_loop(vg, vge);
		return;
	}
	if ((vge->fill_st.style == VG_TEXTURED) &&
	    (tex = texture_find(t->ts, vge->fill_st.texture)) == NULL)
		return;

	if (vg->ints == NULL) {
		vg->ints = Malloc(nvtx*sizeof(int), M_RG);
		vg->nints = nvtx;
	} else {
		if (nvtx > vg->nints) {
			vg->ints = Realloc(vg->ints, nvtx*sizeof(int));
			vg->nints = nvtx;
		}
	}

	/* Find Y maxima */
	maxy = miny = VG_RASY(vg,vtx[0].y);
	for (i = 1; i < nvtx; i++) {
		int vy = VG_RASY(vg,vtx[i].y);

		if (vy < miny) {
			miny = vy;
		} else if (vy > maxy) {
			maxy = vy;
		}
	}

	/* Find the intersections. */
	for (y = miny; y <= maxy; y++) {
		ints = 0;
		for (i = 0; i < nvtx; i++) {
			if (i == 0) {
				ind1 = nvtx - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			y1 = VG_RASY(vg,vtx[ind1].y);
			y2 = VG_RASY(vg,vtx[ind2].y);
			if (y1 < y2) {
				x1 = VG_RASX(vg,vtx[ind1].x);
				x2 = VG_RASX(vg,vtx[ind2].x);
			} else if (y1 > y2) {
				y2 = VG_RASY(vg,vtx[ind1].y);
				y1 = VG_RASY(vg,vtx[ind2].y);
				x2 = VG_RASX(vg,vtx[ind1].x);
				x1 = VG_RASX(vg,vtx[ind2].x);
			} else {
				continue;
			}
			if (((y >= y1) && (y < y2)) ||
			    ((y == maxy) && (y > y1) && (y <= y2))) {
				vg->ints[ints++] =
				    (((y-y1)<<16) / (y2-y1)) *
				    (x2-x1) + (x1<<16);
			} 
		}
		qsort(vg->ints, ints, sizeof(int), compare_ints);

		for (i = 0; i < ints; i += 2) {
			int xa, xb, xi;
			Uint8 r, g, b;

			xa = vg->ints[i] + 1;
			xa = (xa>>16) + ((xa&0x8000) >> 15);
			xb = vg->ints[i+1] - 1;
			xb = (xb>>16) + ((xb&0x8000) >> 15);

			switch (vge->fill_st.style) {
			case VG_NOFILL:
				break;
			case VG_SOLID:
				vg_hline_primitive(vg, xa, xb, y, vge->color);
				break;
			case VG_TEXTURED:
				for (xi = xa; xi < xb; xi++) {
					SDL_GetRGB(GET_PIXEL2(tex->t->su,
					    (xi % tex->t->su->w),
					    (y % tex->t->su->h)),
					    tex->t->su->format, &r, &g, &b);
					PUT_PIXEL2_CLIPPED(vg->su, xi, y,
					    SDL_MapRGB(vg->fmt, r, g, b));
				}
				break;
			}
		}
	}
}

static void
init(void *p)
{
	struct polygon_tool *pt = p;

	pt->polygon = NULL;
	pt->vtx = NULL;
	pt->seq = BEGIN_POLYGON;
	pt->preview = 0;
}

static struct window *
edit(void *p)
{
	struct polygon_tool *pt = p;
	struct window *win;
	struct checkbox *cb;

	win = window_new(0, NULL);
	cb = checkbox_new(win, _("Preview"));
	widget_bind(cb, "state", WIDGET_INT, &pt->preview);
	return (win);
}

static void
mousebuttondown(void *p, struct sketch *sk, double x, double y, int button)
{
	struct polygon_tool *pt = p;
	struct vg *vg = sk->vg;
	struct tileview *tv = TILEVIEW_TOOL(pt)->tv;
	Uint8 r, g, b;

	switch (pt->seq) {
	case BEGIN_POLYGON:
		if (button == SDL_BUTTON_LEFT) {
			pt->polygon = vg_begin_element(vg, VG_POLYGON);
			vg_vertex2(vg, x, y);
			pt->vtx = vg_vertex2(vg, x, y);
			pt->seq = CONTINUE_POLYGON;
			tv->flags |= TILEVIEW_NO_SCROLLING;
	
			prim_hsv2rgb(sk->h, sk->s, sk->v, &r, &g, &b);
			vg_color4(vg, r, g, b, (int)(sk->a*255.0));

			if (!pt->preview)
				pt->polygon->vg_polygon.outline = 1;
		}
		break;
	case CONTINUE_POLYGON:
		if (button == SDL_BUTTON_LEFT) {
			pt->vtx = vg_vertex2(vg, x, y);
		} else {
			if (pt->polygon->nvtx < 3) {
				vg_destroy_element(vg, pt->polygon);
			} else {
				pt->polygon->vg_polygon.outline = 0;
				vg_pop_vertex(vg);
			}
			pt->vtx = NULL;
			pt->polygon = NULL;
			pt->seq = BEGIN_POLYGON;
			tv->flags &= ~TILEVIEW_NO_SCROLLING;
		}
		break;
	}
}

static void
mousemotion(void *p, struct sketch *sk, double x, double y, double xrel,
    double yrel)
{
	struct polygon_tool *pt = p;
	struct tileview *tv = TILEVIEW_TOOL(pt)->tv;

	if (pt->vtx != NULL) {
		pt->vtx->x = x;
		pt->vtx->y = y;
		sk->vg->redraw++;
	}
}

struct tileview_sketch_tool_ops sketch_polygon_ops = {
	{
		_("Polygon"),
		_("Sketch filled polygons."),
		sizeof(struct polygon_tool),
		TILEVIEW_SKETCH_TOOL,
		RG_POLYGON_ICON, -1,
		init,
		NULL,		/* destroy */
		edit,
		NULL,		/* selected */
		NULL		/* unselected */
	},
	mousebuttondown,
	NULL,			/* mousebuttonup */
	mousemotion,
	NULL,			/* mousewheel */
	NULL,			/* keydown */
	NULL,			/* keyup */
};

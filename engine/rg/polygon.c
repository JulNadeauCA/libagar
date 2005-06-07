/*	$Csoft: polygon.c,v 1.15 2005/06/04 04:49:38 vedge Exp $	*/

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
#include <engine/view.h>

#include <engine/vg/vg.h>

#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/radio.h>
#include <engine/widget/box.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/combo.h>

#include "tileset.h"
#include "tileview.h"
#include "polygon.h"
#include "texsel.h"

const struct version polygon_ver = {
	"agar rg polygon feature",
	2, 0
};

const struct feature_ops polygon_ops = {
	"polygon",
	sizeof(struct polygon),
	N_("Filled/textured polygon."),
	FEATURE_AUTOREDRAW,
	polygon_init,
	polygon_load,
	polygon_save,
	polygon_destroy,
	polygon_render,
	NULL,
	NULL,
	polygon_edit
};

void
polygon_init(void *p, struct tileset *ts, int flags)
{
	struct polygon *poly = p;
	struct sketch *sk, *osk;
	u_int skno = 0;

	feature_init(poly, ts, flags, &polygon_ops);
	poly->sketch[0] = '\0';
	poly->type = POLYGON_SOLID;
	poly->cSolid = SDL_MapRGBA(ts->fmt, 0, 0, 0, 255);
	poly->texture[0] = '\0';
	poly->texture_alpha = 255;
	poly->ints = NULL;
	poly->nints = 0;
}

void
polygon_destroy(void *p)
{
	struct polygon *poly = p;

	Free(poly->ints, M_RG);
}

int
polygon_load(void *p, struct netbuf *buf)
{
	struct polygon *poly = p;
	struct tileset *ts = FEATURE(poly)->ts;

	if (version_read(buf, &polygon_ver, NULL) == -1)
		return (-1);

	poly->type = (enum polygon_type)read_uint8(buf);
	copy_string(poly->sketch, buf, sizeof(poly->sketch));
	poly->cSolid = read_color(buf, ts->fmt);
	copy_string(poly->texture, buf, sizeof(poly->texture));
	poly->texture_alpha = (int)read_uint8(buf);
	return (0);
}

void
polygon_save(void *p, struct netbuf *buf)
{
	struct polygon *poly = p;
	struct tileset *ts = FEATURE(poly)->ts;

	version_write(buf, &polygon_ver);

	write_uint8(buf, (Uint8)poly->type);
	write_string(buf, poly->sketch);
	write_color(buf, ts->fmt, poly->cSolid);
	write_string(buf, poly->texture);
	write_uint8(buf, (Uint8)poly->texture_alpha);
}

static void
poll_sketches(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tile *t = argv[1].p;
	struct tileset *ts = t->ts;
	struct tile_element *tel;
	struct tlist_item *it;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->type != TILE_SKETCH) {
			continue;
		}
		it = tlist_insert(tl, tel->tel_sketch.sk->vg->su, "%s",
		    tel->name);
		it->p1 = tel;
		it->class = "tile-sketch";
	}
	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
select_sketch(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct polygon *poly = argv[1].p;
	struct tile *t = argv[2].p;
	struct tlist_item *it = argv[3].p;

	strlcpy(poly->sketch, it->text, sizeof(poly->sketch));
}

struct window *
polygon_edit(void *p, struct tileview *tv)
{
	struct polygon *poly = p;
	struct window *win;
	struct radio *rad;
	static const char *modes[] = {
		N_("Solid fill"),
		N_("Texture fill"),
		NULL
	};
	struct box *box;
	struct combo *com;

	win = window_new(0, NULL);
	window_set_caption(win, _("Polygon"));

	rad = radio_new(win, modes);
	widget_bind(rad, "value", WIDGET_INT, &poly->type);

	com = combo_new(win, COMBO_POLL, _("Sketch: "));
	event_new(com->list, "tlist-poll", poll_sketches, "%p", tv->tile);
	event_new(com, "combo-selected", select_sketch, "%p,%p", poly,
	    tv->tile);
	combo_select_text(com, poly->sketch);

	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	{
		struct spinbutton *sb;
		struct notebook *nb;
		struct notebook_tab *ntab;
		struct box *hb;

		nb = notebook_new(box, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
		ntab = notebook_add_tab(nb, _("Color"), BOX_VERT);
		{
			struct hsvpal *hsv1, *hsv2;

			hsv1 = hsvpal_new(ntab);
			WIDGET(hsv1)->flags |= WIDGET_WFILL|WIDGET_HFILL;
			widget_bind(hsv1, "pixel-format", WIDGET_POINTER,
			    &tv->ts->fmt);
			widget_bind(hsv1, "pixel", WIDGET_UINT32,
			    &poly->cSolid);
		}

		ntab = notebook_add_tab(nb, _("Texture"), BOX_VERT);
		{
			struct texsel *txsel;

			txsel = texsel_new(ntab, tv->ts, 0);
			WIDGET(txsel)->flags |= WIDGET_WFILL|WIDGET_HFILL;
			widget_bind(txsel, "texture-name", WIDGET_STRING,
			    poly->texture, sizeof(poly->texture));
		
			sb = spinbutton_new(ntab, _("Texture alpha: "));
			widget_bind(sb, "value", WIDGET_INT,
			    &poly->texture_alpha);
			spinbutton_set_range(sb, 0, 255);
			spinbutton_set_increment(sb, 5);
		}
	}
	return (win);
}

static int
compare_ints(const void *a, const void *b)
{
	return (*(const int *)a) - (*(const int *)b);
}

static __inline__ void
render_polygon(struct tile *tile, struct polygon *poly,
    struct tile_element *ske, struct vg *vg, struct vg_element *vge,
    struct texture *tex)
{
	struct vg_vertex *vtx = vge->vtx;
	u_int i, nvtx = vge->nvtx;
	int x, y, x1, y1, x2, y2;
	int miny, maxy;
	int ind1, ind2;
	int ints;

	if (poly->ints == NULL) {
		poly->ints = Malloc(nvtx*sizeof(int), M_RG);
		poly->nints = nvtx;
	} else {
		if (nvtx > poly->nints) {
			poly->ints = Realloc(poly->ints, nvtx*sizeof(int));
			poly->nints = nvtx;
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
				poly->ints[ints++] =
				    (((y-y1)<<16) / (y2-y1)) *
				    (x2-x1) + (x1<<16);
			} 
		}
		qsort(poly->ints, ints, sizeof(int), compare_ints);

		for (i = 0; i < ints; i += 2) {
			int xa, xb, xi;
			Uint8 r, g, b;

			xa = poly->ints[i] + 1;
			xa = (xa>>16) + ((xa&0x8000) >> 15);
			xb = poly->ints[i+1] - 1;
			xb = (xb>>16) + ((xb&0x8000) >> 15);

			switch (poly->type) {
			case POLYGON_SOLID:
				prim_hline(tile, xa, xb, y, poly->cSolid);
				break;
			case POLYGON_TEXTURED:
				for (xi = xa; xi < xb; xi++) {
					SDL_GetRGB(GET_PIXEL2(tex->t->su,
					    (xi % tex->t->su->w),
					    (y % tex->t->su->h)),
					    tex->t->su->format, &r, &g, &b);
					PUT_PIXEL2_CLIPPED(tile->su,
					    ske->tel_sketch.x+xi,
					    ske->tel_sketch.y+y,
					    SDL_MapRGB(tile->su->format,
					    r, g, b));
				}
				break;
			}
		}
	}
}

/* Apply texturing/filling to VG_POLYGON elements of a sketch. */
void
polygon_render(void *p, struct tile *tile, int fx, int fy)
{
	struct polygon *poly = p;
	struct tile_element *ske;
	struct texture *tex = NULL;
	struct vg *vg;
	struct vg_element *vge;

	if ((ske = tile_find_element(tile, TILE_SKETCH, poly->sketch)) == NULL)
		return;

	if ((poly->type == POLYGON_TEXTURED) &&
	    (tex = texture_find(tile->ts, poly->texture)) == NULL)
		return;

	vg = ske->tel_sketch.sk->vg;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		vge->drawn = 0;
		switch (vge->type) {
		case VG_POLYGON:
			if (vge->nvtx >= 3) {
				render_polygon(tile, poly, ske, vg, vge, tex);
			}
			break;
		default:
			vg_rasterize_element(vg, vge);
			break;
		}
	}
}

#if 0
/* Rendering routine for simple polygons. */
void
polygon_render_simple(void *p, struct tile *tile, int fx, int fy)
{
	struct polygon *poly = p;
	SDL_Surface *sDst = tile->su;
	SDL_Surface *sVg;
	int x, y, d;
	int *left, *right;
	struct tile_element *ske;
	struct sketch *sk;
	struct texture *tex = NULL;

	if ((ske = tile_find_element(tile, TILE_SKETCH, poly->sketch)) == NULL)
		return;

	if ((poly->type == POLYGON_TEXTURED) &&
	    (tex = texture_find(tile->ts, poly->texture)) == NULL)
		return;

	sk = ske->tel_sketch.sk;
	sVg = sk->vg->su;

	left = Malloc(sVg->h*sizeof(int), M_RG);
	right = Malloc(sVg->h*sizeof(int), M_RG);
	memset(left, 0, sVg->h*sizeof(int));
	memset(right, 0, sVg->h*sizeof(int));

	for (y = 0; y < sVg->h; y++) {
		for (x = 0; x < sVg->w; x++) {
			if (GET_PIXEL2(sVg, x, y) != sVg->format->colorkey) {
				left[y] = x;
				break;
			}
		}
		for (x = sVg->w-1; x >= 0; x--) {
			if (GET_PIXEL2(sVg, x, y) != sVg->format->colorkey) {
				right[y] = x;
				break;
			}
		}
	}

	switch (poly->type) {
	case POLYGON_SOLID:
		for (y = 0;
		     y < sVg->h && y < sDst->h;
		     y++) {
			if ((d = right[y] - left[y]) > 0) {
				prim_hline(tile, left[y], right[y], y,
				    poly->cSolid);
			}
		}
		break;
	case POLYGON_TEXTURED:
		for (y = 0; y < sVg->h; y++) {
			Uint8 r, g, b;

			for (x = 0; x < sVg->w; x++) {
				if (y < sDst->h && x < sDst->w &&
				    (right[y] - left[y]) > 0 &&
				    x >= left[y] && x <= right[y]) {
					SDL_GetRGB(GET_PIXEL2(tex->t->su,
					    (x % tex->t->su->w),
					    (y % tex->t->su->h)),
					    tex->t->su->format, &r, &g, &b);
					PUT_PIXEL2_CLIPPED(sDst,
					    ske->tel_sketch.x+x,
					    ske->tel_sketch.y+y,
					    SDL_MapRGB(sDst->format, r, g, b));
				}
			}
		}
		break;
	}

	Free(left, M_RG);
	Free(right, M_RG);
}
#endif

/*	$Csoft: polygon.c,v 1.1 2005/05/16 10:35:14 vedge Exp $	*/

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

const struct version polygon_ver = {
	"agar rg polygon feature",
	0, 0
};

const struct feature_ops polygon_ops = {
	"polygon",
	sizeof(struct polygon),
	N_("Filled/textured polygon."),
	FEATURE_AUTOREDRAW,
	polygon_init,
	polygon_load,
	polygon_save,
	NULL,		/* destroy */
	polygon_apply,
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
	poly->type = POLYGON_SOLID;
	poly->alpha = 255;
	poly->p_solid.c = SDL_MapRGB(ts->fmt, 0, 0, 0);
	poly->sketch_tel = NULL;
	poly->sketch = NULL;
}

int
polygon_load(void *p, struct netbuf *buf)
{
	struct polygon *poly = p;
	struct tileset *ts = FEATURE(poly)->ts;

	if (version_read(buf, &polygon_ver, NULL) == -1)
		return (-1);

	poly->type = (enum polygon_type)read_uint8(buf);
	switch (poly->type) {
	case POLYGON_SOLID:
		poly->p_solid.c = read_color(buf, ts->fmt);
		break;
	case POLYGON_TEXTURED:
		poly->p_texture.texid = (int)read_uint32(buf);
		poly->p_texture.texcoords[0] = (int)read_uint32(buf);
		poly->p_texture.texcoords[1] = (int)read_uint32(buf);
		poly->p_texture.texcoords[2] = (int)read_uint32(buf);
		poly->p_texture.texcoords[3] = (int)read_uint32(buf);
		break;
	}
	return (0);
}

void
polygon_save(void *p, struct netbuf *buf)
{
	struct polygon *poly = p;
	struct tileset *ts = FEATURE(poly)->ts;

	version_write(buf, &polygon_ver);

	write_uint8(buf, (Uint8)poly->type);
	switch (poly->type) {
	case POLYGON_SOLID:
		write_color(buf, ts->fmt, poly->p_solid.c);
		break;
	case POLYGON_TEXTURED:
		write_uint32(buf, (Uint32)poly->p_texture.texid);
		write_uint32(buf, (Uint32)poly->p_texture.texcoords[0]);
		write_uint32(buf, (Uint32)poly->p_texture.texcoords[1]);
		write_uint32(buf, (Uint32)poly->p_texture.texcoords[2]);
		write_uint32(buf, (Uint32)poly->p_texture.texcoords[3]);
		break;
	}
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
		it = tlist_insert(tl, tel->tel_sketch.sk->vg->su,
		    "%s", tel->tel_sketch.sk->name);
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

	poly->sketch_tel = it->p1;
	poly->sketch = poly->sketch_tel->tel_sketch.sk;
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

	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	{
		struct hsvpal *hsv1, *hsv2;
		struct spinbutton *sb;
		struct notebook *nb;
		struct notebook_tab *ntab;
		struct box *hb;

		nb = notebook_new(box, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
		ntab = notebook_add_tab(nb, _("Color"), BOX_VERT);
		notebook_select_tab(nb, ntab);
		{
			hsv1 = hsvpal_new(ntab, tv->ts->fmt);
			WIDGET(hsv1)->flags |= WIDGET_WFILL|
			                       WIDGET_HFILL;
			widget_bind(hsv1, "pixel", WIDGET_UINT32,
			    &poly->p_solid.c);
		}

		ntab = notebook_add_tab(nb, _("Texture"), BOX_VERT);
		{
		}
		
		sb = spinbutton_new(box, _("Overall alpha: "));
		widget_bind(sb, "value", WIDGET_UINT8, &poly->alpha);
		spinbutton_set_range(sb, 0, 255);
		spinbutton_set_increment(sb, 5);
	}
	return (win);
}

void
polygon_apply(void *p, struct tile *t, int fx, int fy)
{
	struct polygon *poly = p;
	SDL_Surface *sDst = t->su;
	Uint8 *pDst, *pEnd;
	SDL_Surface *sVg;
	int x, y, d;
	int *left, *right;

	if (poly->sketch == NULL) {
		return;
	}
	sVg = poly->sketch->vg->su;

	left = Malloc(sVg->w*sizeof(int), M_RG);
	right = Malloc(sVg->w*sizeof(int), M_RG);
	memset(left, 0, sVg->w*sizeof(int));
	memset(right, 0, sVg->w*sizeof(int));

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

	for (y = 0; y < sVg->h; y++) {
		if ((d = right[y] - left[y]) > 0) {
			pDst = (Uint8 *)sDst->pixels + y*sDst->pitch +
			    left[y]*sDst->format->BytesPerPixel;
			pEnd = pDst + d*sDst->format->BytesPerPixel;

			while (pDst <= pEnd) {
				PUT_PIXEL(sDst, pDst, poly->p_solid.c);
				pDst += sDst->format->BytesPerPixel;
			}
		}
	}

	Free(left, M_RG);
	Free(right, M_RG);
}


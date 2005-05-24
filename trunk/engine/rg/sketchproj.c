/*	$Csoft: sketchproj.c,v 1.3 2005/05/23 01:30:00 vedge Exp $	*/

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
#include "sketchproj.h"

const struct version sketchproj_ver = {
	"agar rg sketch projection feature",
	0, 0
};

const struct feature_ops sketchproj_ops = {
	"sketchproj",
	sizeof(struct sketchproj),
	N_("Sketch projection."),
	FEATURE_AUTOREDRAW,
	sketchproj_init,
	sketchproj_load,
	sketchproj_save,
	NULL,		/* destroy */
	sketchproj_apply,
	NULL,
	NULL,
	sketchproj_edit
};

void
sketchproj_init(void *p, struct tileset *ts, int flags)
{
	struct sketchproj *sproj = p;

	feature_init(sproj, ts, flags, &sketchproj_ops);
	sproj->alpha = 255;
	sproj->color = SDL_MapRGB(ts->fmt, 0, 0, 0);
	sproj->sketch[0] = '\0';
}

int
sketchproj_load(void *p, struct netbuf *buf)
{
	struct sketchproj *sproj = p;
	struct tileset *ts = FEATURE(sproj)->ts;

	if (version_read(buf, &sketchproj_ver, NULL) == -1)
		return (-1);

	copy_string(sproj->sketch, buf, sizeof(sproj->sketch));
	sproj->alpha = read_uint8(buf);
	sproj->color = read_color(buf, ts->fmt);
	return (0);
}

void
sketchproj_save(void *p, struct netbuf *buf)
{
	struct sketchproj *sproj = p;
	struct tileset *ts = FEATURE(sproj)->ts;

	version_write(buf, &sketchproj_ver);

	write_string(buf, sproj->sketch);
	write_uint8(buf, sproj->alpha);
	write_color(buf, ts->fmt, sproj->color);
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
	struct sketchproj *sproj = argv[1].p;
	struct tile *t = argv[2].p;
	struct tlist_item *it = argv[3].p;

	strlcpy(sproj->sketch, it->text, sizeof(sproj->sketch));
}

struct window *
sketchproj_edit(void *p, struct tileview *tv)
{
	struct sketchproj *sproj = p;
	struct window *win;
	struct box *box;
	struct combo *com;

	win = window_new(0, NULL);
	window_set_caption(win, _("Polygon"));

	com = combo_new(win, COMBO_POLL, _("Sketch: "));
	event_new(com->list, "tlist-poll", poll_sketches, "%p", tv->tile);
	event_new(com, "combo-selected", select_sketch, "%p,%p", sproj,
	    tv->tile);
	combo_select_text(com, sproj->sketch);

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
			hsv1 = hsvpal_new(ntab);
			WIDGET(hsv1)->flags |= WIDGET_WFILL|
			                       WIDGET_HFILL;
			widget_bind(hsv1, "pixel-format", WIDGET_POINTER,
			    &tv->ts->fmt);
			widget_bind(hsv1, "pixel", WIDGET_UINT32,
			    &sproj->color);
		}

		sb = spinbutton_new(box, _("Overall alpha: "));
		widget_bind(sb, "value", WIDGET_UINT8, &sproj->alpha);
		spinbutton_set_range(sb, 0, 255);
		spinbutton_set_increment(sb, 5);
	}
	return (win);
}

void
sketchproj_apply(void *p, struct tile *t, int fx, int fy)
{
	struct sketchproj *sproj = p;
	SDL_Surface *sDst = t->su;
	double x1, y1, x2, y2;
	struct vg *vg;
	struct vg_element *vge;
	struct tile_element *ske;
	struct sketch *sk;
	int i;

	if ((ske = tile_find_element(t, TILE_SKETCH, sproj->sketch)) == NULL) {
		return;
	}
	sk = ske->tel_sketch.sk;
	vg = ske->tel_sketch.sk->vg;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		switch (vge->type) {
		case VG_LINE_STRIP:
			vg_vtxcoords2d(vg, &vge->vtx[0], &x1, &y1);
			for (i = 1; i < vge->nvtx; i++) {
				vg_vtxcoords2d(vg, &vge->vtx[i], &x2, &y2);
				prim_color_u32(t, sproj->color);
				prim_wuline(t,
				    ske->tel_sketch.x+x1,
				    ske->tel_sketch.y+y1,
				    ske->tel_sketch.x+x2,
				    ske->tel_sketch.y+y2);
				x1 = x2;
				y1 = y2;
			}
			break;
		default:
			break;
		}
	}
}


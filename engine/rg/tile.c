/*	$Csoft: tile.c,v 1.5 2005/01/30 02:34:42 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/menu.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/mspinbutton.h>

#include "tileset.h"
#include "tileview.h"

#include "fill.h"

void
tile_init(struct tile *t, const char *name)
{
	strlcpy(t->name, name, sizeof(t->name));
	t->flags = 0;
	t->used = 0;
	t->su = NULL;
	TAILQ_INIT(&t->features);
}

void
tile_scale(struct tileset *ts, struct tile *t, Uint16 w, Uint16 h, Uint8 flags)
{
	Uint32 sflags = SDL_SWSURFACE;

	if (flags & TILE_SRCCOLORKEY)	sflags |= SDL_SRCCOLORKEY;
	if (flags & TILE_SRCALPHA)	sflags |= SDL_SRCALPHA;

	if (t->su != NULL) {
		SDL_FreeSurface(t->su);
	}
	t->flags = flags;
	t->su = SDL_CreateRGBSurface(sflags, w, h, ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask,
	    ts->fmt->Gmask,
	    ts->fmt->Bmask,
	    ts->fmt->Amask);

	if (t->su == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

#if 0
	dprintf("Surface masks=%08x,%08x,%08x,%08x\n",
	    t->su->format->Rmask,
	    t->su->format->Gmask,
	    t->su->format->Bmask,
	    t->su->format->Amask);
	dprintf("Surface shifts=%08x,%08x,%08x,%08x\n",
	    t->su->format->Rshift,
	    t->su->format->Gshift,
	    t->su->format->Bshift,
	    t->su->format->Ashift);
	dprintf("Surface losses=%08x,%08x,%08x,%08x\n",
	    t->su->format->Rloss,
	    t->su->format->Gloss,
	    t->su->format->Bloss,
	    t->su->format->Aloss);
	dprintf("Surface colorkey=%08x\n", t->su->format->colorkey);
	dprintf("Surface alpha=%02x\n", t->su->format->alpha);
#endif
}

void
tile_generate(struct tile *t)
{
	struct tile_feature *tft;
	u_int i;

	SDL_FillRect(t->su, NULL, SDL_MapRGBA(t->su->format, 0, 0, 0, 0));

	TAILQ_FOREACH(tft, &t->features, features) {
		struct feature *ft = tft->ft;

		if (ft->ops->apply != NULL)
			ft->ops->apply(ft, t, tft->x, tft->y);
	}
}

struct tile_feature *
tile_add_feature(struct tile *t, void *ft)
{
	struct tile_feature *tft;

	tft = Malloc(sizeof(struct tile_feature), M_RG);
	tft->ft = ft;
	tft->x = 0;
	tft->y = 0;
	tft->visible = 1;
	TAILQ_INSERT_TAIL(&t->features, tft, features);
	return (tft);
}

void
tile_remove_feature(struct tile *t, void *pft)
{
	struct tile_feature *tft;

	TAILQ_FOREACH(tft, &t->features, features) {
		if (tft->ft == pft)
			break;
	}
	if (tft != NULL) {
		TAILQ_REMOVE(&t->features, tft, features);
		Free(tft, M_RG);
	}
}

void
tile_save(struct tile *t, struct netbuf *buf)
{
	Uint32 nfeatures = 0;
	off_t nfeatures_offs;
	struct tile_feature *tft;

	write_string(buf, t->name);
	write_uint8(buf, t->flags);
	write_uint16(buf, t->su->w);
	write_uint16(buf, t->su->h);

	nfeatures_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(tft, &t->features, features) {
		dprintf("%s: saving %s feature ref\n", t->name, tft->ft->name);
		write_string(buf, tft->ft->name);
		write_sint32(buf, (Sint32)tft->x);
		write_sint32(buf, (Sint32)tft->y);
		write_uint8(buf, (Uint8)tft->visible);
		nfeatures++;
	}
	pwrite_uint32(buf, nfeatures, nfeatures_offs);
}

int
tile_load(struct tileset *ts, struct tile *t, struct netbuf *buf)
{
	Uint32 i, nfeatures;
	Uint16 w, h;
	Uint8 flags;
	
	flags = read_uint8(buf);
	w = read_uint16(buf);
	h = read_uint16(buf);
	tile_scale(ts, t, w, h, flags);

	nfeatures = read_uint32(buf);
	dprintf("%s: %ux%u, %u features\n", t->name, w, h, nfeatures);
	for (i = 0; i < nfeatures; i++) {
		char feat_name[FEATURE_NAME_MAX];
		struct feature *ft;
		Sint32 x, y;
		Uint8 visible;

		copy_string(feat_name, buf, sizeof(feat_name));
		x = read_sint32(buf);
		y = read_sint32(buf);
		visible = read_uint8(buf);
		dprintf("%s: feat %s at %d,%d\n", t->name, feat_name, x, y);

		TAILQ_FOREACH(ft, &ts->features, features) {
			if (strcmp(ft->name, feat_name) == 0)
				break;
		}
		if (ft == NULL) {
			error_set("Nonexistent feature: `%s'", feat_name);
			return (-1);
		}
		tile_add_feature(t, ft);
	}
	tile_generate(t);
	return (0);
}

void
tile_destroy(struct tile *t)
{
	
}

static void
close_tile(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	
	pthread_mutex_lock(&ts->lock);
	t->used--;
	pthread_mutex_unlock(&ts->lock);

	view_detach(win);
}

static void
insert_fill(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *feat_tl = argv[3].p;
	struct tlist_item *feat_it;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct fill *fill;

	fill = Malloc(sizeof(struct fill), M_RG);
	fill_init(fill, ts, 0);
	TAILQ_INSERT_TAIL(&ts->features, FEATURE(fill), features);
	tile_add_feature(t, fill);
	tile_generate(t);

	if (tv->edit_mode) {
		feature_close(tv, pwin);
	}
	feature_edit(tv, FEATURE(fill), pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, feat_tl, "tlist-poll", NULL);
	tlist_unselect_all(feat_tl);
	TAILQ_FOREACH(feat_it, &feat_tl->items, items) {
		struct tile_feature *tft;

		if (strcmp(feat_it->class, "feature") != 0) {
			continue;
		}
		tft = feat_it->p1;
		if (tft->ft == FEATURE(fill)) {
			tlist_select(feat_tl, feat_it);
			break;
		}
	}
}

static void
poll_features(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	struct tile_feature *tft;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(tft, &t->features, features) {
		char label[TLIST_LABEL_MAX];
		struct feature_sketch *fsk;
		struct feature_pixmap *fpx;
		struct tlist_item *it;

		it = tlist_insert(tl, ICON(OBJ_ICON), "%s (%d,%d) %s",
		    tft->ft->name, tft->x, tft->y,
		    tft->visible ? "" : " (invisible)");
		it->class = "feature";
		it->p1 = tft;
		it->depth = 0;

		if (!TAILQ_EMPTY(&tft->ft->sketches) ||
		    !TAILQ_EMPTY(&tft->ft->pixmaps)) {
			it->flags |= TLIST_HAS_CHILDREN;
		}
		if (!tlist_visible_children(tl, it))
			continue;

		TAILQ_FOREACH(fsk, &tft->ft->sketches, sketches) {
			it = tlist_insert(tl, ICON(DRAWING_ICON),
			    "%s (%d,%d) %s", fsk->sk->name, fsk->x, fsk->y,
			    fsk->visible ? "" : " (invisible)");
			it->class = "sketch";
			it->p1 = tft;
		}
		TAILQ_FOREACH(fpx, &tft->ft->pixmaps, pixmaps) {
			it = tlist_insert(tl, ICON(DRAWING_ICON),
			    "%s (%d,%d) %s", fpx->px->name, fpx->x, fpx->y,
			    fpx->visible ? "" : " (invisible)");
			it->class = "pixmap";
			it->p1 = tft;
		}
	}

	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
edit_feature(int argc, union evarg *argv)
{
	struct widget *sndr = argv[0].p;
	struct tileview *tv = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct window *pwin = argv[3].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct tlist_item *it;

	if (strcmp(sndr->type, "tlist") == 0)
		tv->edit_mode = !tv->edit_mode;

	if (tv->edit_mode == 0) {
		feature_close(tv, pwin);
		return;
	} else {
		if ((it = tlist_item_selected(tl)) == NULL) {
			tv->edit_mode = 0;
			return;
		}
	}
	
	if (strcmp(it->class, "feature") == 0) {
		struct tile_feature *tft = it->p1;

		feature_edit(tv, tft->ft, pwin);
	} else if (strcmp(it->class, "sketch") == 0) {
		//
	} else if (strcmp(it->class, "pixmap") == 0) {
		//
	}
}

static void
delete_feature(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tileset *ts = argv[2].p;
	struct tile *t = argv[3].p;
	struct tlist *feat_tl = argv[4].p;
	struct window *pwin = argv[5].p;
	struct tlist_item *it = tlist_item_selected(feat_tl);
	struct tile_feature *tft;

	if (it == NULL)
		return;
	
	if (strcmp(it->class, "feature") == 0) {
		struct tile_feature *tft = it->p1;

		tile_remove_feature(t, tft->ft);
	} else if (strcmp(it->class, "sketch") == 0) {
		//
	} else if (strcmp(it->class, "pixmap") == 0) {
		//
	}
	
	if (tv->edit_mode)
		feature_close(tv, pwin);
}

static void
resize_tile(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	struct mspinbutton *msb = argv[3].p;
	struct window *pwin = argv[4].p;
	struct checkbox *ckey_cb = argv[5].p;
	struct checkbox *alpha_cb = argv[6].p;
	struct tileview *tv = argv[7].p;
	int w = widget_get_int(msb, "xvalue");
	int h = widget_get_int(msb, "yvalue");
	int flags = 0;

	if (widget_get_int(ckey_cb, "state") == 1)
		flags |= SDL_SRCALPHA;
	if (widget_get_int(alpha_cb, "state") == 1)
		flags |= SDL_SRCCOLORKEY;

	tile_scale(ts, t, w, h, flags);
	tileview_resize(tv, 100);
	view_detach(pwin);
}

static void
resize_tile_dlg(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	struct window *pwin = argv[3].p;
	struct tileview *tv = argv[4].p;
	struct window *win;
	struct mspinbutton *msb;
	struct box *box;
	struct button *b;
	struct checkbox *ckey_cb, *alpha_cb;

	win = window_new(WINDOW_DETACH|WINDOW_NO_RESIZE|WINDOW_NO_MINIMIZE,
	    NULL);
	window_set_caption(win, _("Resize tile `%s'"), t->name);

	msb = mspinbutton_new(win, "x", _("New size:"));
	mspinbutton_set_range(msb, TILE_SIZE_MIN, TILE_SIZE_MAX);
	widget_set_int(msb, "xvalue", t->su->w);
	widget_set_int(msb, "yvalue", t->su->h);

	ckey_cb = checkbox_new(win, _("Colorkeying"));
	widget_set_int(ckey_cb, "state", t->su->flags & SDL_SRCCOLORKEY);

	alpha_cb = checkbox_new(win, _("Alpha blending"));
	widget_set_int(alpha_cb, "state", t->su->flags & SDL_SRCALPHA);

	box = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	{
		b = button_new(box, "OK");
		event_new(b, "button-pushed", resize_tile,
		    "%p,%p,%p,%p,%p,%p,%p",
		    ts, t, msb, win, ckey_cb, alpha_cb, tv);

		b = button_new(box, "Cancel");
		event_new(b, "button-pushed", window_generic_detach, "%p", win);
	}

	window_attach(pwin, win);
	window_show(win);
}

struct window *
tile_edit(struct tileset *ts, struct tile *t)
{
	struct window *win;
	struct box *box;
	struct AGMenu *menu;
	struct AGMenuItem *item;
	struct tileview *tv;
	struct tlist *feat_tl;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, "%s <%s>", t->name, OBJECT(ts)->name);
	event_new(win, "window-close", close_tile, "%p,%p", ts, t);
	
	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, t, TILEVIEW_AUTOREGEN);
	
	feat_tl = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(feat_tl, TLIST_POLL|TLIST_TREE);
	WIDGET(feat_tl)->flags &= ~(WIDGET_WFILL);
	tlist_prescale(feat_tl, _("Feature #00 (0,0)"), 10);
	event_new(feat_tl, "tlist-poll", poll_features, "%p,%p", ts, t);

	menu = ag_menu_new(win);
	item = ag_menu_add_item(menu, _("Features"));
	{
		ag_menu_action(item, _("Fill"), NULL,
		    SDLK_f, KMOD_CTRL, insert_fill, "%p,%p,%p", tv, win,
		    feat_tl);
		    
		ag_menu_action(item, _("Sketch projection"), NULL,
		    SDLK_s, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Polygon"), NULL,
		    SDLK_p, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Extruded base"), NULL,
		    SDLK_e, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Revolved base"), NULL,
		    SDLK_r, KMOD_CTRL, NULL, "%p,%p", ts, t);
	}

	item = ag_menu_add_item(menu, _("Edit"));
	{
		ag_menu_action(item, _("Resize tile..."), NULL,
		    0, 0, resize_tile_dlg, "%p,%p,%p,%p", ts, t, win, tv);
	}

	box = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HFILL|BOX_FRAME);
	box_set_padding(box, 0);
	box_set_spacing(box, 0);
	{
		struct box *fbox;
		struct button *fbu;

		fbox = box_new(box, BOX_VERT, BOX_HFILL|BOX_FRAME);
		box_set_padding(fbox, 0);
		box_set_spacing(fbox, 0);
		{
			object_attach(fbox, feat_tl);
	
			fbu = button_new(fbox, _("Edit"));
			WIDGET(fbu)->flags |= WIDGET_WFILL;
			button_set_sticky(fbu, 1);
			widget_bind(fbu, "state", WIDGET_INT, &tv->edit_mode);

			event_new(fbu, "button-pushed", edit_feature,
			    "%p,%p,%p", tv, feat_tl, win);
			event_new(feat_tl, "tlist-dblclick", edit_feature,
			    "%p,%p,%p", tv, feat_tl, win);
			
			fbu = button_new(fbox, _("Delete"));
			event_new(fbu, "button-pushed", delete_feature,
			    "%p,%p,%p,%p,%p", tv, ts, t, feat_tl, win);
			WIDGET(fbu)->flags |= WIDGET_WFILL;
		}
		
		object_attach(box, tv);
		widget_focus(tv);
	}

	t->used++;
	return (win);
}

void
tile_put_pixel(struct tile *t, int x, int y, Uint32 color)
{
	Uint8 *dst = (Uint8 *)t->su->pixels + y*t->su->pitch +
	    x*t->su->format->BytesPerPixel;

	if (x < 0 || y < 0 ||
	    x >= t->su->w || y >= t->su->h)
		return;

	*(Uint32 *)dst = color;
}

void
tile_blend_rgb(struct tile *t, int x, int y, enum tile_blend_mode mode,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint8 dr, dg, db, da, ba;
	Uint8 *dst;

	if (x < 0 || y < 0 ||
	    x > t->su->w || y > t->su->h)
		return;

	dst = (Uint8 *)t->su->pixels + y*t->su->pitch +
	    x*t->su->format->BytesPerPixel;
	SDL_GetRGBA(*(Uint32 *)dst, t->su->format, &dr, &dg, &db, &da);

	if (mode == TILE_BLEND_SRCALPHA) {
		ba = a;
	} else if (mode == TILE_BLEND_DSTALPHA) {
		ba = da;
	} else {
		ba = (Uint8)(da+a)/2;
	}

	*(Uint32 *)dst = SDL_MapRGB(t->su->format,
	    (((r - dr) * ba) >> 8) + dr,
	    (((g - dg) * ba) >> 8) + dg,
	    (((b - db) * ba) >> 8) + db);
}

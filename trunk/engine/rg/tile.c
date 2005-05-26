/*	$Csoft: tile.c,v 1.50 2005/05/24 08:15:10 vedge Exp $	*/

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

#include <engine/map/map.h>

#include <engine/loader/surface.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/menu.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/label.h>
#include <engine/widget/separator.h>
#include <engine/widget/radio.h>

#include "tileset.h"
#include "tileview.h"
#include "fill.h"
#include "polygon.h"
#include "sketchproj.h"

/*
 * Blend a pixmap with the tile; add the source alpha to the destination
 * alpha of each pixel.
 */
static void
blend_overlay_alpha(struct tile *t, SDL_Surface *su, SDL_Rect *rd)
{
	u_int sx, sy, dx, dy;
	Uint8 *pSrc, *pDst;
	Uint8 sR, sG, sB, sA;
	Uint8 dR, dG, dB, dA;
	int alpha;

	SDL_LockSurface(su);
	SDL_LockSurface(t->su);
	for (sy = 0, dy = rd->y; sy < su->h; sy++, dy++) {
		for (sx = 0, dx = rd->x; sx < su->w; sx++, dx++) {
			if (CLIPPED_PIXEL(t->su, dx, dy))
				continue;

			pSrc = (Uint8 *)su->pixels + sy*su->pitch + (sx << 2);

			if (*(Uint32 *)pSrc == su->format->colorkey)
				continue;

			pDst = (Uint8 *)t->su->pixels + dy*t->su->pitch +
					(dx << 2);

			if (*(Uint32 *)pDst != t->su->format->colorkey) {
				SDL_GetRGBA(*(Uint32 *)pDst, t->su->format,
				    &dR, &dG, &dB, &dA);
				SDL_GetRGBA(*(Uint32 *)pSrc, su->format,
				    &sR, &sG, &sB, &sA);

				alpha = dA + sA;
				if (alpha > 255) {
					alpha = 255;
				}
				*(Uint32 *)pDst = SDL_MapRGBA(t->su->format,
				    (((sR - dR) * sA) >> 8) + dR,
				    (((sG - dG) * sA) >> 8) + dG,
				    (((sB - dB) * sA) >> 8) + dB,
				    (Uint8)alpha);
			} else {
				*(Uint32 *)pDst = *(Uint32 *)pSrc;
			}
		}
	}
	SDL_UnlockSurface(t->su);
	SDL_UnlockSurface(su);
}

static void
blend_normal(struct tile *t, SDL_Surface *su, SDL_Rect *rd)
{
	SDL_BlitSurface(su, NULL, t->su, rd);
}

void
tile_init(struct tile *t, struct tileset *ts, const char *name)
{
	strlcpy(t->name, name, sizeof(t->name));
	t->flags = 0;
	t->su = NULL;
	t->ts = ts;
	t->nrefs = 0;
	t->sprite = 0;
	t->blend_fn = blend_overlay_alpha;
	TAILQ_INIT(&t->elements);
}

void
tile_scale(struct tileset *ts, struct tile *t, Uint16 w, Uint16 h, u_int flags,
    Uint8 alpha)
{
	Uint32 sflags = SDL_SWSURFACE;

	if (flags & TILE_SRCALPHA)	sflags |= SDL_SRCALPHA;
	if (flags & TILE_SRCCOLORKEY)	sflags |= SDL_SRCCOLORKEY;

	/* Previous surface will be freed by sprite_set_surface(). */

	t->flags = flags|TILE_DIRTY;
	t->su = SDL_CreateRGBSurface(sflags, w, h, ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask, ts->fmt->Gmask, ts->fmt->Bmask, ts->fmt->Amask);
	if (t->su == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	t->su->format->alpha = alpha;
	sprite_set_surface(&SPRITE(ts,t->sprite), t->su);
}

void
tile_generate(struct tile *t)
{
	struct tile_element *tel;
	struct tile_pixmap *tpx;
	u_int i;

	/* TODO check for opaque fill features/pixmaps first */
	SDL_FillRect(t->su, NULL, SDL_MapRGBA(t->su->format, 0, 0, 0, 0));

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (!tel->visible) {
			continue;
		}
		switch (tel->type) {
		case TILE_FEATURE:
			{
				struct feature *ft = tel->tel_feature.ft;

				if (ft->ops->apply != NULL)
					ft->ops->apply(ft, t,
					    tel->tel_feature.x,
					    tel->tel_feature.y);
			}
			break;
		case TILE_PIXMAP:
			{
				struct pixmap *px = tel->tel_pixmap.px;
				SDL_Rect rd;

				rd.x = tel->tel_pixmap.x;
				rd.y = tel->tel_pixmap.y;
				rd.w = px->su->w;
				rd.h = px->su->h;
				t->blend_fn(t, px->su, &rd);
			}
			break;
		case TILE_SKETCH:
			{
				struct sketch *sk = tel->tel_sketch.sk;
				struct vg *vg = sk->vg;
				SDL_Rect rd;

				rd.x = tel->tel_sketch.x;
				rd.y = tel->tel_sketch.y;
				rd.w = vg->su->w;
				rd.h = vg->su->h;
				t->blend_fn(t, vg->su, &rd);
			}
			break;
		}
	}

	sprite_update(&SPRITE(t->ts,t->sprite));
}

static __inline__ void
gen_element_name(struct tile_element *tel, struct tile *t, const char *fname)
{
	struct tile_element *oel;
	u_int elno = 0;

tryname:
	snprintf(tel->name, sizeof(tel->name), "%s <#%u>", fname,
	    elno);
	TAILQ_FOREACH(oel, &t->elements, elements) {
		if (strcmp(oel->name, tel->name) == 0)
			break;
	}
	if (oel != NULL) {
		elno++;
		goto tryname;
	}
}

struct tile_element *
tile_find_element(struct tile *t, enum tile_element_type type, const char *name)
{
	struct tile_element *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->type == type &&
		    strcmp(tel->name, name) == 0)
			break;
	}
	return (tel);
}

struct tile_element *
tile_add_feature(struct tile *t, const char *name, void *ft, int x, int y)
{
	struct tile_element *tel;

	tel = Malloc(sizeof(struct tile_element), M_RG);
	if (name != NULL) {
		strlcpy(tel->name, name, sizeof(tel->name));
	} else {
		gen_element_name(tel, t, FEATURE(ft)->name);
	}
	tel->type = TILE_FEATURE;
	tel->visible = 1;
	tel->tel_feature.ft = ft;
	tel->tel_feature.x = x;
	tel->tel_feature.y = y;
	TAILQ_INSERT_TAIL(&t->elements, tel, elements);
	t->flags |= TILE_DIRTY;
	return (tel);
}

void
tile_remove_feature(struct tile *t, void *ftp, int destroy)
{
	struct feature *ft = ftp;
	struct tile_element *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->tel_feature.ft == ft)
			break;
	}
	if (tel != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		Free(tel, M_RG);

		if (--ft->nrefs == 0 && destroy) {
			text_tmsg(MSG_INFO, 500,
			    _("Destroying unreferenced feature `%s'."),
			    ft->name);
			TAILQ_REMOVE(&t->ts->features, ft, features);
			feature_destroy(ft);
			Free(ft, M_RG);
		}
	}
}

struct tile_element *
tile_add_pixmap(struct tile *t, const char *name, struct pixmap *px,
    int x, int y)
{
	struct tile_element *tel;

	tel = Malloc(sizeof(struct tile_element), M_RG);
	if (name != NULL) {
		strlcpy(tel->name, name, sizeof(tel->name));
	} else {
		gen_element_name(tel, t, px->name);
	}
	tel->type = TILE_PIXMAP;
	tel->visible = 1;
	tel->tel_pixmap.px = px;
	tel->tel_pixmap.x = x;
	tel->tel_pixmap.y = y;
	tel->tel_pixmap.alpha = 255;
	TAILQ_INSERT_TAIL(&t->elements, tel, elements);
	px->nrefs++;

	t->flags |= TILE_DIRTY;
	return (tel);
}

struct tile_element *
tile_add_sketch(struct tile *t, const char *name, struct sketch *sk,
    int x, int y)
{
	struct tile_element *tel;

	tel = Malloc(sizeof(struct tile_element), M_RG);
	if (name != NULL) {
		strlcpy(tel->name, name, sizeof(tel->name));
	} else {
		gen_element_name(tel, t, sk->name);
	}
	tel->type = TILE_SKETCH;
	tel->visible = 1;
	tel->tel_sketch.sk = sk;
	tel->tel_sketch.x = x;
	tel->tel_sketch.y = y;
	tel->tel_sketch.alpha = 255;
	TAILQ_INSERT_TAIL(&t->elements, tel, elements);
	sk->nrefs++;
	t->flags |= TILE_DIRTY;
	return (tel);
}

void
tile_remove_pixmap(struct tile *t, struct pixmap *px, int destroy)
{
	struct tile_element *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->tel_pixmap.px == px)
			break;
	}
	if (tel != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		Free(tel, M_RG);
		if (--px->nrefs == 0 && destroy) {
			text_tmsg(MSG_INFO, 250,
			    _("Destroying unreferenced pixmap `%s'."),
			    px->name);
			TAILQ_REMOVE(&t->ts->pixmaps, px, pixmaps);
			pixmap_destroy(px);
			Free(px, M_RG);
		}
	}
}

void
tile_remove_sketch(struct tile *t, struct sketch *sk, int destroy)
{
	struct tile_element *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->tel_sketch.sk == sk)
			break;
	}
	if (tel != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		Free(tel, M_RG);
		if (--sk->nrefs == 0 && destroy) {
			text_tmsg(MSG_INFO, 250,
			    _("Destroying unreferenced sketch `%s'."),
			    sk->name);
			TAILQ_REMOVE(&t->ts->sketches, sk, sketches);
			sketch_destroy(sk);
			Free(sk, M_RG);
		}
	}
}

void
tile_save(struct tile *t, struct netbuf *buf)
{
	Uint32 nelements = 0;
	off_t nelements_offs;
	struct tile_element *tel;

	write_string(buf, t->name);
	write_uint8(buf, t->flags);
	write_surface(buf, t->su);
	write_uint32(buf, t->sprite);
	write_sint16(buf, (Sint16)SPRITE(t->ts,t->sprite).xOrig);
	write_sint16(buf, (Sint16)SPRITE(t->ts,t->sprite).yOrig);
	write_uint8(buf, (Uint8)SPRITE(t->ts,t->sprite).snap_mode);

	nelements_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(tel, &t->elements, elements) {
		write_string(buf, tel->name);
		write_uint32(buf, (Uint32)tel->type);
		write_uint8(buf, (Uint8)tel->visible);

		switch (tel->type) {
		case TILE_FEATURE:
			{
				struct feature *ft = tel->tel_feature.ft;

				write_string(buf, ft->name);
				write_sint32(buf, (Sint32)tel->tel_feature.x);
				write_sint32(buf, (Sint32)tel->tel_feature.y);
			}
			break;
		case TILE_PIXMAP:
			{
				struct pixmap *px = tel->tel_pixmap.px;

				write_string(buf, px->name);
				write_sint32(buf, (Sint32)tel->tel_pixmap.x);
				write_sint32(buf, (Sint32)tel->tel_pixmap.y);
				write_uint8(buf, (Uint8)tel->tel_pixmap.alpha);
			}
			break;
		case TILE_SKETCH:
			{
				struct sketch *sk = tel->tel_sketch.sk;

				write_string(buf, sk->name);
				write_sint32(buf, (Sint32)tel->tel_sketch.x);
				write_sint32(buf, (Sint32)tel->tel_sketch.y);
				write_uint8(buf, (Uint8)tel->tel_sketch.alpha);
			}
			break;
		}
		nelements++;
	}
	pwrite_uint32(buf, nelements, nelements_offs);
}

int
tile_load(struct tileset *ts, struct tile *t, struct netbuf *buf)
{
	Uint32 i, nelements;
	Uint8 flags;
	
	t->flags = read_uint8(buf);
	t->su = read_surface(buf, ts->fmt);
	t->sprite = read_uint32(buf);
	sprite_set_surface(&SPRITE(ts,t->sprite), t->su);
	SPRITE(ts,t->sprite).xOrig = (int)read_sint16(buf);
	SPRITE(ts,t->sprite).yOrig = (int)read_sint16(buf);
	SPRITE(ts,t->sprite).snap_mode = (int)read_uint8(buf);

	nelements = read_uint32(buf);
	dprintf("%s: %u elements\n", t->name, nelements);
	for (i = 0; i < nelements; i++) {
		char name[TILE_ELEMENT_NAME_MAX];
		enum tile_element_type type;
		struct tile_element *tel;
		int visible;

		copy_string(name, buf, sizeof(name));
		type = (enum tile_element_type)read_uint32(buf);
		visible = (int)read_uint8(buf);

		switch (type) {
		case TILE_FEATURE:
			{
				char feat_name[FEATURE_NAME_MAX];
				struct feature *ft;
				Sint32 x, y;

				copy_string(feat_name, buf, sizeof(feat_name));
				x = read_sint32(buf);
				y = read_sint32(buf);
				dprintf("%s: feat %s at %d,%d\n", t->name,
				    feat_name, x, y);
				TAILQ_FOREACH(ft, &ts->features, features) {
					if (strcmp(ft->name, feat_name) == 0)
						break;
				}
				if (ft == NULL) {
					error_set("bad feature: %s", feat_name);
					return (-1);
				}
				tel = tile_add_feature(t, name, ft, x, y);
				tel->visible = visible;
			}
			break;
		case TILE_PIXMAP:
			{
				char pix_name[PIXMAP_NAME_MAX];
				struct pixmap *px;
				Sint32 x, y;
				int alpha;

				copy_string(pix_name, buf, sizeof(pix_name));
				x = read_sint32(buf);
				y = read_sint32(buf);
				alpha = (int)read_uint8(buf);

				dprintf("%s: pix %s at %d,%d (a=%u)\n", t->name,
				    pix_name, x, y, alpha);
				TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
					if (strcmp(px->name, pix_name) == 0)
						break;
				}
				if (px == NULL) {
					error_set("bad pixmap: %s", pix_name);
					return (-1);
				}
				tel = tile_add_pixmap(t, name, px, x, y);
				tel->tel_pixmap.alpha = alpha;
				tel->visible = visible;
			}
			break;
		case TILE_SKETCH:
			{
				char sk_name[SKETCH_NAME_MAX];
				struct sketch *sk;
				Sint32 x, y;
				Uint32 w, h;
				int alpha;

				copy_string(sk_name, buf, sizeof(sk_name));
				x = read_sint32(buf);
				y = read_sint32(buf);
				alpha = (int)read_uint8(buf);

				dprintf("%s: sk %s at %d,%d (a=%u)\n", t->name,
				    sk_name, x, y, alpha);
				TAILQ_FOREACH(sk, &ts->sketches, sketches) {
					if (strcmp(sk->name, sk_name) == 0)
						break;
				}
				if (sk == NULL) {
					error_set("bad sketch: %s", sk_name);
					return (-1);
				}
				tel = tile_add_sketch(t, name, sk, x, y);
				tel->tel_sketch.alpha = alpha;
				tel->visible = visible;
			}
			break;
		default:
			break;
		}
	}
	t->flags &= ~TILE_DIRTY;
	return (0);
}

void
tile_destroy(struct tile *t)
{
	sprite_destroy(&SPRITE(t->ts,t->sprite));
	t->su = NULL;
}

#ifdef EDITION

static void
geo_ctrl_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	struct tileview_ctrl *ctrl = argv[1].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	int w = tileview_int(ctrl, 2);
	int h = tileview_int(ctrl, 3);
	
	if (w != t->su->w || h != t->su->h)  {
		tile_scale(ts, t, w, h, t->flags, t->su->format->alpha);
		tileview_set_zoom(tv, tv->zoom, 0);
	}
}

static void
close_element(struct tileview *tv)
{
	switch (tv->state) {
	case TILEVIEW_FEATURE_EDIT:
		if (tv->tv_feature.ft->ops->flags & FEATURE_AUTOREDRAW) {
			tileview_set_autoredraw(tv, 0, 0);
		}
		if (tv->tv_feature.menu != NULL) {
			feature_close_menu(tv);
		}
		if (tv->tv_feature.win != NULL) {
			view_detach(tv->tv_feature.win);
			tv->tv_feature.win = NULL;
		}
		break;
	case TILEVIEW_PIXMAP_EDIT:
		if (tv->tv_pixmap.win != NULL) {
			tileview_remove_ctrl(tv, tv->tv_pixmap.ctrl);
			tv->tv_pixmap.ctrl = NULL;
			view_detach(tv->tv_pixmap.win);
			tv->tv_pixmap.win = NULL;
		}
		break;
	case TILEVIEW_SKETCH_EDIT:
		if (tv->tv_sketch.win != NULL) {
			tileview_remove_ctrl(tv, tv->tv_sketch.ctrl);
			tv->tv_sketch.ctrl = NULL;
			view_detach(tv->tv_sketch.win);
			tv->tv_sketch.win = NULL;
		}
		break;
	default:
		if (tv->menu != NULL) {
			tile_close_menu(tv);
		}
		break;
	}
	
	if (tv->state == TILEVIEW_TILE_EDIT) {
		if (tv->tv_tile.geo_ctrl != NULL)
			tileview_remove_ctrl(tv, tv->tv_tile.geo_ctrl);
		if (tv->tv_tile.orig_ctrl != NULL)
			tileview_remove_ctrl(tv, tv->tv_tile.orig_ctrl);
	}
	tv->state = TILEVIEW_TILE_EDIT;
	tv->edit_mode = 0;

	tv->tv_tile.geo_ctrl = tileview_insert_ctrl(tv, TILEVIEW_RDIMENSIONS,
	    "%i,%i,%u,%u", 0, 0,
	    (u_int)tv->tile->su->w,
	    (u_int)tv->tile->su->h);
	tv->tv_tile.geo_ctrl->buttonup = event_new(tv, NULL, geo_ctrl_buttonup,
	    "%p", tv->tv_tile.geo_ctrl);

	tv->tv_tile.orig_ctrl = tileview_insert_ctrl(tv, TILEVIEW_POINT,
	    "%*i,%*i",
	    &SPRITE(tv->ts,tv->tile->sprite).xOrig,
	    &SPRITE(tv->ts,tv->tile->sprite).yOrig);
	tv->tv_tile.orig_ctrl->cIna.r = 0;
	tv->tv_tile.orig_ctrl->cIna.g = 255;
	tv->tv_tile.orig_ctrl->cIna.b = 0;
	tv->tv_tile.orig_ctrl->cHigh.r = 73;
	tv->tv_tile.orig_ctrl->cHigh.g = 186;
	tv->tv_tile.orig_ctrl->cHigh.b = 51;
	tv->tv_tile.orig_ctrl->cLow.r = 86;
	tv->tv_tile.orig_ctrl->cLow.g = 161;
	tv->tv_tile.orig_ctrl->cLow.b = 71;
	tv->tv_tile.orig_ctrl->cOver.r = 191;
	tv->tv_tile.orig_ctrl->cOver.g = 170;
	tv->tv_tile.orig_ctrl->cOver.b = 47;
	tv->tv_tile.orig_ctrl->aEna = 70;
	tv->tv_tile.orig_ctrl->aIna = 30;
	tv->tv_tile.orig_ctrl->aOver= 100;

	if (tv->tel_tbar != NULL) {
		struct window *pwin = widget_parent_window(tv->tel_tbar);
	
		object_detach(tv->tel_tbar);
		object_destroy(tv->tel_tbar);
		Free(tv->tel_tbar, M_OBJECT);
		tv->tel_tbar = NULL;

		WINDOW_UPDATE(pwin);
	}
}

static void
element_closed(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct tileview *tv = argv[1].p;

	if (tv->edit_mode)
		close_element(tv);
}

static void
pixmap_ctrl_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	struct tileview_ctrl *ctrl = argv[1].p;
	struct pixmap *px = argv[2].p;
	struct tile *t = tv->tile;
	int w = tileview_int(ctrl, 2);
	int h = tileview_int(ctrl, 3);
	
	if (w != px->su->w || h != px->su->h) {
		pixmap_scale(px, w, h, ctrl->xoffs, ctrl->yoffs);
	}
	t->flags |= TILE_DIRTY;
}

static void
sketch_ctrl_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	struct tileview_ctrl *ctrl = argv[1].p;
	struct sketch *sk = argv[2].p;
	struct tile *t = tv->tile;
	int w = tileview_int(ctrl, 2);
	int h = tileview_int(ctrl, 3);
	
	if (w != sk->vg->su->w || h != sk->vg->su->h)  {
		sketch_scale(sk, w, h, 1.0, ctrl->xoffs, ctrl->yoffs);
	}
	t->flags |= TILE_DIRTY;
}

static void
open_element(struct tileview *tv, struct tile_element *tel,
    struct window *pwin)
{
	if (tv->state == TILEVIEW_TILE_EDIT) {
		tileview_remove_ctrl(tv, tv->tv_tile.geo_ctrl);
		tileview_remove_ctrl(tv, tv->tv_tile.orig_ctrl);
		tv->tv_tile.geo_ctrl = NULL;
		tv->tv_tile.orig_ctrl = NULL;
	}

	switch (tel->type) {
	case TILE_FEATURE:
		{
			struct window *win;
			struct feature *ft = tel->tel_feature.ft;

			tv->state = TILEVIEW_FEATURE_EDIT;
			tv->tv_feature.ft = ft;
			tv->tv_feature.menu = NULL;
			tv->tv_feature.menu_item = NULL;
			tv->tv_feature.menu_win = NULL;

			if (ft->ops->flags & FEATURE_AUTOREDRAW)
				tileview_set_autoredraw(tv, 1, 125);
			
			if (ft->ops->edit != NULL) {
				win = ft->ops->edit(ft, tv);
				window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
				window_attach(pwin, win);
				window_show(win);

				tv->tv_feature.win = win;
				event_new(win, "window-close", element_closed,
				    "%p", tv);
				
				view->focus_win = pwin;
				widget_focus(tv);
			} else {
				tv->tv_feature.win = NULL;
			}

			if (ft->ops->toolbar != NULL) {
				tv->tel_tbar = ft->ops->toolbar(ft, tv);
				WINDOW_UPDATE(pwin);
			}
		}
		break;
	case TILE_PIXMAP:
		{
			struct window *win;
			
			tv->state = TILEVIEW_PIXMAP_EDIT;
			tv->tv_pixmap.px = tel->tel_pixmap.px;
			tv->tv_pixmap.tel = tel;
			tv->tv_pixmap.ctrl = tileview_insert_ctrl(tv,
			    TILEVIEW_RECTANGLE, "%*i,%*i,%u,%u",
			    &tel->tel_pixmap.x,
			    &tel->tel_pixmap.y,
			    (u_int)tel->tel_pixmap.px->su->w,
			    (u_int)tel->tel_pixmap.px->su->h);
			tv->tv_pixmap.ctrl->buttonup =
			    event_new(tv, NULL, pixmap_ctrl_buttonup, "%p,%p",
			    tv->tv_pixmap.ctrl, tel->tel_pixmap.px);
			tv->tv_pixmap.state = TILEVIEW_PIXMAP_IDLE;
			tv->tv_pixmap.win = win = pixmap_edit(tv, tel);
			tv->tv_pixmap.menu = NULL;
			tv->tv_pixmap.menu_item = NULL;
			tv->tv_pixmap.menu_win = NULL;

			window_attach(pwin, win);
			window_show(win);
			event_new(win, "window-close", element_closed, "%p",tv);
			view->focus_win = pwin;
			widget_focus(tv);
			
			tv->tel_tbar = pixmap_toolbar(tv, tel);
			WINDOW_UPDATE(pwin);
		}
		break;
	case TILE_SKETCH:
		{
			struct window *win;
			
			tv->state = TILEVIEW_SKETCH_EDIT;
			tv->tv_sketch.sk = tel->tel_sketch.sk;
			tv->tv_sketch.tel = tel;
			tv->tv_sketch.ctrl = tileview_insert_ctrl(tv,
			    TILEVIEW_RECTANGLE, "%*i,%*i,%u,%u",
			    &tel->tel_sketch.x,
			    &tel->tel_sketch.y,
			    tel->tel_sketch.sk->vg->su->w,
			    tel->tel_sketch.sk->vg->su->h);
			tv->tv_sketch.ctrl->buttonup =
			    event_new(tv, NULL, sketch_ctrl_buttonup, "%p,%p",
			    tv->tv_sketch.ctrl, tel->tel_sketch.sk);
			tv->tv_sketch.win = win = sketch_edit(tv, tel);
			tv->tv_sketch.menu = NULL;
			tv->tv_sketch.menu_item = NULL;
			tv->tv_sketch.menu_win = NULL;

			window_attach(pwin, win);
			window_show(win);
			event_new(win, "window-close", element_closed, "%p",tv);
			view->focus_win = pwin;
			widget_focus(tv);

			tv->tel_tbar = sketch_toolbar(tv, tel);
			WINDOW_UPDATE(pwin);
		}
		break;
	}
	tv->edit_mode = 1;
}

static void
create_pixmap(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist_item *eit;
	struct pixmap *px;
	struct tile_element *tel;
	u_int pixno = 0;
	struct pixmap *opx;

	px = Malloc(sizeof(struct pixmap), M_RG);
	pixmap_init(px, tv->ts, 0);
tryname:
	snprintf(px->name, sizeof(px->name), "pixmap #%u", pixno);
	TAILQ_FOREACH(opx, &tv->ts->pixmaps, pixmaps) {
		if (strcmp(opx->name, px->name) == 0)
			break;
	}
	if (opx != NULL) {
		pixno++;
		goto tryname;
	}

	pixmap_scale(px, 32, 32, 0, 0);
	TAILQ_INSERT_TAIL(&tv->ts->pixmaps, px, pixmaps);

	tel = tile_add_pixmap(tv->tile, NULL, px, 0, 0);
	close_element(tv);
	open_element(tv, tel, pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, tl_feats, "tlist-poll", NULL);
	tlist_unselect_all(tl_feats);
	TAILQ_FOREACH(eit, &tl_feats->items, items) {
		struct tile_element *tel;

		if (strcmp(eit->class, "pixmap") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_pixmap.px == px) {
			tlist_select(tl_feats, eit);
			break;
		}
	}
}

static void
create_sketch(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist_item *eit;
	struct sketch *sk, *osk;
	struct tile_element *tel;
	u_int skno = 0;

	sk = Malloc(sizeof(struct sketch), M_RG);
	sketch_init(sk, tv->ts, 0);
tryname:
	snprintf(sk->name, sizeof(sk->name), "sketch #%u", skno);
	TAILQ_FOREACH(osk, &tv->ts->sketches, sketches) {
		if (strcmp(osk->name, sk->name) == 0)
			break;
	}
	if (osk != NULL) {
		skno++;
		goto tryname;
	}

	sketch_scale(sk, 32, 32, 1.0, 0, 0);
	TAILQ_INSERT_TAIL(&tv->ts->sketches, sk, sketches);
	tel = tile_add_sketch(tv->tile, NULL, sk, 0, 0);
	tv->tile->flags |= TILE_DIRTY;
	close_element(tv);
	open_element(tv, tel, pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, tl_feats, "tlist-poll", NULL);
	tlist_unselect_all(tl_feats);
	TAILQ_FOREACH(eit, &tl_feats->items, items) {
		struct tile_element *tel;

		if (strcmp(eit->class, "sketch") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_sketch.sk == sk) {
			tlist_select(tl_feats, eit);
			break;
		}
	}
}

static void
attach_pixmap(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct window *win_dlg = argv[3].p;
	struct tlist *tl_feats = argv[4].p;
	struct tlist *tl_pixmaps = argv[5].p;
	struct tlist_item *it;
	struct tile_element *tel;
	struct pixmap *px;

	if ((it = tlist_selected_item(tl_pixmaps)) == NULL) {
		return;
	}
	px = it->p1;

	tel = tile_add_pixmap(tv->tile, NULL, px, 0, 0);
	close_element(tv);
	open_element(tv, tel, pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, tl_feats, "tlist-poll", NULL);
	tlist_unselect_all(tl_feats);
	TAILQ_FOREACH(it, &tl_feats->items, items) {
		struct tile_element *tel;

		if (strcmp(it->class, "pixmap") != 0) {
			continue;
		}
		tel = it->p1;
		if (tel->tel_pixmap.px == px) {
			tlist_select(tl_feats, it);
			break;
		}
	}

	view_detach(win_dlg);
}

static void
attach_pixmap_dlg(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist *tl;
	struct pixmap *px;
	struct window *win;
	struct box *bo;

	win = window_new(WINDOW_MODAL|WINDOW_NO_MINIMIZE, NULL);
	window_set_caption(win, _("Attach existing pixmap"));

	tl = tlist_new(win, 0);
	tlist_set_item_height(tl, TILESZ);
	tlist_prescale(tl, "XXXXXXXXXXXXXXXXXXX", 5);

	TAILQ_FOREACH(px, &tv->ts->pixmaps, pixmaps) {
		struct tlist_item *it;

		it = tlist_insert(tl, px->su, "%s (%ux%u)", px->name,
		    px->su->w, px->su->h);
		it->p1 = px;
	}
	
	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	{
		struct button *bu;
	
		bu = button_new(bo, _("OK"));
		event_new(bu, "button-pushed", attach_pixmap, "%p,%p,%p,%p,%p",
		    tv, pwin, win, tl_feats, tl);
	
		bu = button_new(bo, _("Cancel"));
		event_new(bu, "button-pushed", window_generic_detach, "%p",
		    win);
	}

	window_attach(pwin, win);
	window_show(win);
}

static void
attach_sketch(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct window *win_dlg = argv[3].p;
	struct tlist *tl_feats = argv[4].p;
	struct tlist *tl_sketches = argv[5].p;
	struct tlist_item *it;
	struct tile_element *tel;
	struct sketch *sk;

	if ((it = tlist_selected_item(tl_sketches)) == NULL) {
		return;
	}
	sk = it->p1;

	tel = tile_add_sketch(tv->tile, NULL, sk, 0, 0);
	close_element(tv);
	open_element(tv, tel, pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, tl_feats, "tlist-poll", NULL);
	tlist_unselect_all(tl_feats);
	TAILQ_FOREACH(it, &tl_feats->items, items) {
		struct tile_element *tel;

		if (strcmp(it->class, "sketch") != 0) {
			continue;
		}
		tel = it->p1;
		if (tel->tel_sketch.sk == sk) {
			tlist_select(tl_feats, it);
			break;
		}
	}

	view_detach(win_dlg);
}

static void
attach_sketch_dlg(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist *tl;
	struct sketch *sk;
	struct window *win;
	struct box *bo;

	win = window_new(WINDOW_MODAL|WINDOW_NO_MINIMIZE, NULL);
	window_set_caption(win, _("Attach existing sketch"));

	tl = tlist_new(win, 0);
	tlist_set_item_height(tl, TILESZ);
	tlist_prescale(tl, "XXXXXXXXXXXXXXXXXXXXXXXXX", 5);

	TAILQ_FOREACH(sk, &tv->ts->sketches, sketches) {
		struct tlist_item *it;

		it = tlist_insert(tl, sk->vg->su, "%s (%ux%u, %.0f%%)",
		    sk->name, sk->vg->su->w, sk->vg->su->h,
		    sk->vg->scale*100.0);
		it->p1 = sk;
	}
	
	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	{
		struct button *bu;
	
		bu = button_new(bo, _("OK"));
		event_new(bu, "button-pushed", attach_sketch, "%p,%p,%p,%p,%p",
		    tv, pwin, win, tl_feats, tl);
	
		bu = button_new(bo, _("Cancel"));
		event_new(bu, "button-pushed", window_generic_detach, "%p",
		    win);
	}

	window_attach(pwin, win);
	window_show(win);
}

static void
select_feature(struct tlist *tl_feats, void *fp)
{
	struct feature *feat = fp;
	struct tlist_item *eit;

	/* Select the newly inserted feature. */
	event_post(NULL, tl_feats, "tlist-poll", NULL);
	tlist_unselect_all(tl_feats);
	TAILQ_FOREACH(eit, &tl_feats->items, items) {
		struct tile_element *tel;

		if (strcmp(eit->class, "feature") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_feature.ft == feat) {
			tlist_select(tl_feats, eit);
			break;
		}
	}
}

static void
insert_fill(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist_item *eit;
	struct fill *fill;
	struct tile_element *tel;

	fill = Malloc(sizeof(struct fill), M_RG);
	fill_init(fill, tv->ts, 0);
	TAILQ_INSERT_TAIL(&tv->ts->features, FEATURE(fill), features);
	tel = tile_add_feature(tv->tile, NULL, fill, 0, 0);
	close_element(tv);
	open_element(tv, tel, pwin);
	select_feature(tl_feats, fill);
}

static void
insert_polygon(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist_item *eit;
	struct polygon *poly;
	struct tile_element *tel;

	poly = Malloc(sizeof(struct polygon), M_RG);
	polygon_init(poly, tv->ts, 0);
	TAILQ_INSERT_TAIL(&tv->ts->features, FEATURE(poly), features);
	tel = tile_add_feature(tv->tile, NULL, poly, 0, 0);
	close_element(tv);
	open_element(tv, tel, pwin);
	select_feature(tl_feats, poly);
}

static void
insert_sketchproj(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *tl_feats = argv[3].p;
	struct tlist_item *eit;
	struct sketchproj *sproj;
	struct tile_element *tel;

	sproj = Malloc(sizeof(struct sketchproj), M_RG);
	sketchproj_init(sproj, tv->ts, 0);
	TAILQ_INSERT_TAIL(&tv->ts->features, FEATURE(sproj), features);
	tel = tile_add_feature(tv->tile, NULL, sproj, 0, 0);
	close_element(tv);
	open_element(tv, tel, pwin);
	select_feature(tl_feats, sproj);
}

static void
poll_feats(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	struct window *win = argv[3].p;
	struct tileview *tv = argv[4].p;
	struct tile_element *tel;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(tel, &t->elements, elements) {
		char label[TLIST_LABEL_MAX];
		struct tlist_item *it;

		if (tel->type == TILE_FEATURE) {
			struct feature *ft = tel->tel_feature.ft;
			struct feature_sketch *fsk;
			struct feature_pixmap *fpx;
	
			it = tlist_insert(tl, ICON(OBJ_ICON), "%s%s%s",
			    (tv->state==TILEVIEW_FEATURE_EDIT &&
			     tv->tv_feature.ft == ft) ? "* " : "",
			    tel->name,
			    tel->visible ? "" : _(" (invisible)"));
			it->class = "feature";
			it->p1 = tel;
			it->depth = 0;

			if (!TAILQ_EMPTY(&ft->sketches) ||
			    !TAILQ_EMPTY(&ft->pixmaps)) {
				it->flags |= TLIST_HAS_CHILDREN;
			}
			if (!tlist_visible_children(tl, it))
				continue;

			TAILQ_FOREACH(fsk, &ft->sketches, sketches) {
				it = tlist_insert(tl, ICON(DRAWING_ICON),
				    "%s%s%s",
				    (tv->state==TILEVIEW_SKETCH_EDIT &&
				     tv->tv_sketch.sk == fsk->sk) ? "* ": "",
				    tel->name,
				    fsk->visible ? "" : _(" (invisible)"));
				it->class = "feature-sketch";
				it->p1 = fsk;
			}

			TAILQ_FOREACH(fpx, &ft->pixmaps, pixmaps) {
				it = tlist_insert(tl, ICON(DRAWING_ICON),
				    "%s%s (%d,%d)%s",
				    (tv->state==TILEVIEW_PIXMAP_EDIT &&
				     tv->tv_pixmap.px == fpx->px) ? "* ": "",
				    tel->name, fpx->x, fpx->y,
				    fpx->visible ? "" : _(" (invisible)"));
				it->class = "feature-pixmap";
				it->p1 = fpx;
			}
		} else if (tel->type == TILE_PIXMAP) {
			struct pixmap *px = tel->tel_pixmap.px;

			it = tlist_insert(tl, px->su, "%s%s%s",
			    (tv->state==TILEVIEW_PIXMAP_EDIT &&
			     tv->tv_pixmap.px == px) ? "* ": "",
			    tel->name,
			    tel->visible ? "" : _(" (invisible)"));
			it->class = "pixmap";
			it->p1 = tel;
			it->depth = 0;
		} else if (tel->type == TILE_SKETCH) {
			struct sketch *sk = tel->tel_sketch.sk;
			struct vg *vg = sk->vg;
			struct vg_element *vge;

			it = tlist_insert(tl, sk->vg->su, "%s%s%s",
			    (tv->state==TILEVIEW_SKETCH_EDIT &&
			     tv->tv_sketch.sk == sk) ? "* ": "",
			    tel->name,
			    tel->visible ? "" : _(" (invisible)"));
			it->class = "sketch";
			it->p1 = tel;
			it->depth = 0;

			if (!TAILQ_EMPTY(&vg->vges)) {
				it->flags |= TLIST_HAS_CHILDREN;
			}
			if (!tlist_visible_children(tl, it))
				continue;

			TAILQ_FOREACH(vge, &vg->vges, vges) {
				it = tlist_insert(tl,
				    ICON(vg_element_types[vge->type]->icon),
				    "%s%s", (vge == vg->cur_vge) ? "* " : "",
				    vg_element_types[vge->type]->name);
				it->class = "sketch-element";
				it->p1 = vge;
				it->depth = 1;
			}
		}
	}

	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
edit_element(int argc, union evarg *argv)
{
	struct widget *sndr = argv[0].p;
	struct tileview *tv = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct window *pwin = argv[3].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct tlist_item *it;
	struct tile_element *tel = NULL;
	
	if (strcmp(sndr->type, "button") == 0 && !tv->edit_mode) {
		close_element(tv);
		return;
	}
	
	if ((it = tlist_selected_item(tl)) != NULL) {
		if (strcmp(it->class, "feature") == 0 ||
		    strcmp(it->class, "pixmap") == 0 ||
		    strcmp(it->class, "sketch") == 0) {
			tel = it->p1;
		} else if (tv->state == TILEVIEW_SKETCH_EDIT &&
		           strcmp(it->class, "sketch-element") == 0) {
			vg_select_element(tv->tv_sketch.sk->vg,
			    (struct vg_element *)it->p1);
			t->flags |= TILE_DIRTY;
			return;
		} else {
			return;
		}
	}
	close_element(tv);
	open_element(tv, tel, pwin);
}

static void
delete_element(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct tlist *tl_feats = argv[2].p;
	int detach_only = argv[3].i;
	struct tlist_item *it;
	struct tile_element *tel;

	if ((it = tlist_selected_item(tl_feats)) == NULL)
		return;

	if (tv->state == TILEVIEW_SKETCH_EDIT &&
	    strcmp(it->class, "sketch-element") == 0) {
	    	struct vg *vg = tv->tv_sketch.sk->vg;

		vg_destroy_element(vg, (struct vg_element *)it->p1);
		return;
	}

	tel = it->p1;

	/* XXX check that it's the element being deleted */
	if (tv->edit_mode)
		close_element(tv);
	
	if (strcmp(it->class, "feature") == 0) {
		tile_remove_feature(t, tel->tel_feature.ft, !detach_only);
	} else if (strcmp(it->class, "pixmap") == 0) {
		tile_remove_pixmap(t, tel->tel_pixmap.px, !detach_only);
	} else if (strcmp(it->class, "sketch") == 0) {
		tile_remove_sketch(t, tel->tel_sketch.sk, !detach_only);
	}

	t->flags |= TILE_DIRTY;
}

static void
resize_tile(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct mspinbutton *msb = argv[2].p;
	struct window *dlg_w = argv[3].p;
	struct checkbox *ckey_cb = argv[4].p;
	struct checkbox *alpha_cb = argv[5].p;
	struct spinbutton *alpha_sb = argv[6].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	int w = widget_get_int(msb, "xvalue");
	int h = widget_get_int(msb, "yvalue");
	u_int flags = 0;

	if (widget_get_bool(ckey_cb, "state"))
		flags |= TILE_SRCCOLORKEY;
	if (widget_get_bool(alpha_cb, "state"))
		flags |= TILE_SRCALPHA;

	tile_scale(ts, t, w, h, flags,
	    (Uint8)widget_get_int(alpha_sb, "value"));
	tileview_set_zoom(tv, 100, 0);
	view_detach(dlg_w);
}

static void
tile_infos(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct window *win;
	struct mspinbutton *msb;
	struct box *box;
	struct button *b;
	struct checkbox *ckey_cb, *alpha_cb;
	struct spinbutton *alpha_sb;
	struct radio *rad;

	win = window_new(WINDOW_MODAL|WINDOW_DETACH|WINDOW_NO_RESIZE|
		         WINDOW_NO_MINIMIZE, NULL);
	window_set_caption(win, _("Resize tile `%s'"), t->name);

	msb = mspinbutton_new(win, "x", _("Geometry:"));
	mspinbutton_set_range(msb, TILE_SIZE_MIN, TILE_SIZE_MAX);
	widget_set_int(msb, "xvalue", t->su->w);
	widget_set_int(msb, "yvalue", t->su->h);
	
	alpha_sb = spinbutton_new(win, _("Overall alpha: "));
	spinbutton_set_range(alpha_sb, 0, 255);
	widget_set_int(alpha_sb, "value", t->su->format->alpha);
	
	ckey_cb = checkbox_new(win, _("Colorkeying"));
	widget_set_int(ckey_cb, "state", t->flags & TILE_SRCCOLORKEY);

	alpha_cb = checkbox_new(win, _("Source alpha"));
	widget_set_int(alpha_cb, "state", t->flags & TILE_SRCALPHA);

	label_static(win, _("Snapping mode: "));
	rad = radio_new(win, gfx_snap_names);
	widget_bind(rad, "value", WIDGET_INT,
	    &SPRITE(t->ts,t->sprite).snap_mode);

	separator_new(win, SEPARATOR_HORIZ);
	
	label_new(win, LABEL_POLLED, _("Maps to sprite: %[u32]"), &t->sprite);
	
	box = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	{
		b = button_new(box, _("OK"));
		event_new(b, "button-pushed", resize_tile,
		    "%p,%p,%p,%p,%p,%p", tv, msb, win, ckey_cb, alpha_cb,
		    alpha_sb);

		b = button_new(box, _("Cancel"));
		event_new(b, "button-pushed", window_generic_detach, "%p", win);
	}

	window_attach(pwin, win);
	window_show(win);
}

static void
move_element_up(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct tile *t = tv->tile;
	struct tlist_item *it;
	struct tile_element *tel, *ptel;

	if ((it = tlist_selected_item(tl)) == NULL ||
	    (strcmp(it->class, "feature") != 0 &&
	     strcmp(it->class, "pixmap") != 0 &&
	     strcmp(it->class, "sketch") != 0)) {
		return;
	}
	tel = it->p1;
	if (tel != TAILQ_FIRST(&t->elements)) {
		ptel = TAILQ_PREV(tel, tile_elementq, elements);
		TAILQ_REMOVE(&t->elements, tel, elements);
		TAILQ_INSERT_BEFORE(ptel, tel, elements);
	}
	t->flags |= TILE_DIRTY;
}

static void
move_element_down(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct tile *t = tv->tile;
	struct tlist_item *it;
	struct tile_element *tel, *ntel;

	if ((it = tlist_selected_item(tl)) == NULL ||
	    (strcmp(it->class, "feature") != 0 &&
	     strcmp(it->class, "pixmap") != 0 &&
	     strcmp(it->class, "sketch") != 0)) {
		return;
	}
	tel = it->p1;
	if ((ntel = TAILQ_NEXT(tel, elements)) != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		TAILQ_INSERT_AFTER(&t->elements, ntel, tel, elements);
	}
	t->flags |= TILE_DIRTY;
}

static void
visible_element(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct tile *t = tv->tile;
	struct tlist_item *it;
	struct tile_element *tel, *ntel;

	if ((it = tlist_selected_item(tl)) == NULL ||
	    (strcmp(it->class, "pixmap") != 0 &&
	     strcmp(it->class, "sketch") != 0 &&
	     strcmp(it->class, "feature") != 0)) {
		return;
	}
	tel = it->p1;
	tel->visible = !tel->visible;
	t->flags |= TILE_DIRTY;
}

static void
tile_undo(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;

	switch (tv->state) {
	case TILEVIEW_TILE_EDIT:
		break;
	case TILEVIEW_FEATURE_EDIT:
		break;
	case TILEVIEW_SKETCH_EDIT:
		break;
	case TILEVIEW_PIXMAP_EDIT:
		pixmap_undo(tv, tv->tv_pixmap.tel);
		break;
	}
}

static void
tile_redo(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;

	switch (tv->state) {
	case TILEVIEW_TILE_EDIT:
		break;
	case TILEVIEW_FEATURE_EDIT:
		break;
	case TILEVIEW_SKETCH_EDIT:
		break;
	case TILEVIEW_PIXMAP_EDIT:
		pixmap_redo(tv, tv->tv_pixmap.tel);
		break;
	}
}

static void
regenerate_tile(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tile *t = tv->tile;

	t->flags |= TILE_DIRTY;
}

static void
feature_menus(struct tileview *tv, struct tlist *tl, struct window *win)
{
	struct AGMenuItem *mi;

	mi = tlist_set_popup(tl, "feature");
	{
		menu_action(mi, _("Toggle visibility"), OBJCREATE_ICON,
		    visible_element, "%p,%p", tv, tl);
#if 0
		menu_action(mi, _("Edit feature"), OBJEDIT_ICON,
		    edit_element, "%p,%p,%p", tv, tl, win);
#endif
		menu_separator(mi);

		menu_action(mi, _("Detach feature"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 1);
		menu_action(mi, _("Destroy feature"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 0);
		
		menu_separator(mi);

		menu_action_kb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT,
		    move_element_up, "%p,%p", tv, tl);
		menu_action_kb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT,
		    move_element_down, "%p,%p", tv, tl);
	}

	mi = tlist_set_popup(tl, "pixmap");
	{
		menu_action(mi, _("Toggle visibility"), OBJCREATE_ICON,
		    visible_element, "%p,%p", tv, tl);

		menu_separator(mi);
#if 0
		menu_action(mi, _("Edit pixmap"), OBJEDIT_ICON,
		    edit_element, "%p,%p,%p", tv, tl, win);
#endif
		menu_action(mi, _("Detach pixmap"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 1);
		menu_action(mi, _("Destroy pixmap"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 0);
		
		menu_separator(mi);
		
		menu_action_kb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT,
		    move_element_up, "%p,%p", tv, tl);
		menu_action_kb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT,
		    move_element_down, "%p,%p", tv, tl);
	}
	
	mi = tlist_set_popup(tl, "sketch");
	{
		menu_action(mi, _("Toggle visibility"), OBJCREATE_ICON,
		    visible_element, "%p,%p", tv, tl);

		menu_separator(mi);

		menu_action(mi, _("Edit sketch"), OBJEDIT_ICON,
		    edit_element, "%p,%p,%p", tv, tl, win);
		
		menu_action(mi, _("Detach sketch"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 1);
		
		menu_action(mi, _("Destroy sketch"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 0);
		
		menu_separator(mi);
		
		menu_action_kb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT,
		    move_element_up, "%p,%p", tv, tl);

		menu_action_kb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT,
		    move_element_down, "%p,%p", tv, tl);
	}
	
	mi = tlist_set_popup(tl, "sketch-element");
	{
		menu_action(mi, _("Edit sketch element"), OBJEDIT_ICON,
		    edit_element, "%p,%p,%p", tv, tl, win);
		
		menu_action(mi, _("Delete sketch element"), TRASH_ICON,
		    delete_element, "%p,%p,%i", tv, tl, 1);
#if 0
		menu_separator(mi);
		menu_action_kb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT,
		    move_vg_element_up, "%p,%p", tv, tl);

		menu_action_kb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT,
		    move_vg_element_down, "%p,%p", tv, tl);
#endif
	}
}

struct window *
tile_edit(struct tileset *ts, struct tile *t)
{
	struct window *win;
	struct box *box;
	struct AGMenu *me;
	struct AGMenuItem *mi;
	struct tileview *tv;
	struct tlist *tl_feats;
	struct toolbar *tbar;

	if ((win = window_new(WINDOW_DETACH, "tile-%s:%s",
	    OBJECT(ts)->name, t->name)) == NULL) {
		return (NULL);
	}
	window_set_caption(win, "%s <%s>", t->name, OBJECT(ts)->name);
	
	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, 0);
	tileview_set_tile(tv, t);
	{
		extern struct tileview_sketch_tool_ops sketch_line_ops;
		extern struct tileview_sketch_tool_ops sketch_circle_ops;

		tileview_reg_tool(tv, &sketch_line_ops);
		tileview_reg_tool(tv, &sketch_circle_ops);
	}
	
	tl_feats = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl_feats, TLIST_POLL|TLIST_TREE);
	WIDGET(tl_feats)->flags &= ~(WIDGET_WFILL);
	tlist_prescale(tl_feats, _("FEATURE #00 <#00>"), 5);
	event_new(tl_feats, "tlist-poll", poll_feats, "%p,%p,%p,%p",
	    ts, t, win, tv);
	feature_menus(tv, tl_feats, win);
	
	me = menu_new(win);

	tbar = Malloc(sizeof(struct toolbar), M_OBJECT);
	toolbar_init(tbar, TOOLBAR_HORIZ, 1, 0);

	mi = menu_add_item(me, _("Tile"));
	{
		menu_action(mi, _("Tile settings..."),
		    RG_PIXMAP_RESIZE_ICON,
		    tile_infos, "%p,%p", tv, win);
		menu_action(mi, _("Regenerate"), RG_PIXMAP_ICON,
		    regenerate_tile, "%p", tv);
	}
	
	mi = menu_add_item(me, _("Edit"));
	{
		menu_action_kb(mi, _("Undo"), -1, SDLK_z, KMOD_CTRL,
		    tile_undo, "%p", tv);
		menu_action_kb(mi, _("Redo"), -1, SDLK_r, KMOD_CTRL,
		    tile_redo, "%p", tv);
	}

	mi = menu_add_item(me, _("Features"));
	{
		menu_tool(mi, tbar, _("Fill"), RG_FILL_ICON,
		    SDLK_f, KMOD_CTRL|KMOD_SHIFT,
		    insert_fill, "%p,%p,%p", tv, win, tl_feats);
		
		menu_tool(mi, tbar, _("Polygon"), RG_POLYGON_ICON,
		    SDLK_p, KMOD_CTRL|KMOD_SHIFT,
		    insert_polygon, "%p,%p,%p", tv, win, tl_feats);

		menu_action_kb(mi, _("Sketch projection"), RG_SKETCH_PROJ_ICON,
		    SDLK_s, KMOD_CTRL|KMOD_SHIFT,
		    insert_sketchproj, "%p,%p,%p", tv, win, tl_feats);

		menu_separator(mi);

		menu_action_kb(mi, _("Extrusion"), RG_EXTRUSION_ICON,
		    SDLK_e, KMOD_CTRL|KMOD_SHIFT,
		    NULL, "%p,%p", ts, t);
		
		menu_action_kb(mi, _("Solid of revolution"),
		    RG_REVOLUTION_ICON,
		    SDLK_r, KMOD_CTRL|KMOD_SHIFT,
		    NULL, "%p,%p", ts, t);
	}
	
	mi = menu_add_item(me, _("Pixmaps"));
	{
		menu_tool(mi, tbar, _("Create new pixmap..."), RG_PIXMAP_ICON,
		    0, 0,
		    create_pixmap, "%p,%p,%p", tv, win, tl_feats);
		menu_tool(mi, tbar, _("Attach pixmap..."),
		    RG_PIXMAP_ATTACH_ICON,
		    0, 0,
		    attach_pixmap_dlg, "%p,%p,%p", tv, win, tl_feats);
	}
	
	mi = menu_add_item(me, _("Sketches"));
	{
		menu_tool(mi, tbar, _("Create new sketch..."), RG_SKETCH_ICON,
		    0, 0,
		    create_sketch, "%p,%p,%p", tv, win, tl_feats);
		menu_tool(mi, tbar, _("Attach sketch..."),
		    RG_SKETCH_ATTACH_ICON,
		    0, 0,
		    attach_sketch_dlg, "%p,%p,%p", tv, win, tl_feats);
	}

	box = tv->tel_box = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HFILL);
	box_set_padding(box, 0);
	box_set_spacing(box, 0);
	{
		struct box *fbox;
		struct button *fbu;

		fbox = box_new(box, BOX_VERT, BOX_HFILL);
		box_set_padding(fbox, 0);
		box_set_spacing(fbox, 0);
		{
			object_attach(fbox, tl_feats);
	
			fbu = button_new(fbox, _("Edit"));
			WIDGET(fbu)->flags |= WIDGET_WFILL;
			button_set_sticky(fbu, 1);
			widget_bind(fbu, "state", WIDGET_INT, &tv->edit_mode);

			event_new(fbu, "button-pushed", edit_element,
			    "%p,%p,%p", tv, tl_feats, win);
			event_new(tl_feats, "tlist-dblclick", edit_element,
			    "%p,%p,%p", tv, tl_feats, win);
		}
		
		fbox = box_new(box, BOX_VERT, BOX_WFILL|BOX_HFILL);
		box_set_padding(fbox, 0);
		box_set_spacing(fbox, 0);
		{
			object_attach(fbox, tbar);
			object_attach(fbox, tv);
			widget_focus(tv);
		}
	}

	/* Set the tile edition mode. */
	close_element(tv);

	window_scale(win, -1, -1);
	window_set_geometry(win,
	    view->w/4, view->h/4,
	    view->w/2, view->h/2);

	/* Center the tile. */
	tv->xoffs = (WIDGET(tv)->w - t->su->w)/2;
	tv->yoffs = (WIDGET(tv)->h - t->su->h)/2;

	return (win);
}

void
tile_open_menu(struct tileview *tv, int x, int y)
{
	struct tile *t = tv->tile;
	
	if (tv->menu != NULL)
		tile_close_menu(tv);

	tv->menu = Malloc(sizeof(struct AGMenu), M_OBJECT);
	menu_init(tv->menu);

	tv->menu_item = menu_add_item(tv->menu, NULL);
	{
		tileview_generic_menu(tv, tv->menu_item);
	}
	tv->menu->sel_item = tv->menu_item;
	tv->menu_win = menu_expand(tv->menu, tv->menu_item, x, y);
}

void
tile_close_menu(struct tileview *tv)
{
	menu_collapse(tv->menu, tv->menu_item);
	object_destroy(tv->menu);
	Free(tv->menu, M_OBJECT);

	tv->menu = NULL;
	tv->menu_item = NULL;
	tv->menu_win = NULL;
}

#endif /* EDITION */

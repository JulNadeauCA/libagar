/*	$Csoft: tile.c,v 1.18 2005/02/19 07:22:01 vedge Exp $	*/

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
tile_init(struct tile *t, struct tileset *ts, const char *name)
{
	strlcpy(t->name, name, sizeof(t->name));
	t->flags = 0;
	t->used = 0;
	t->su = NULL;
	t->ts = ts;
	TAILQ_INIT(&t->elements);
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
	t->flags = flags|TILE_DIRTY;
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
				SDL_BlitSurface(px->su, NULL, t->su, &rd);
			}
			break;
		}
	}
}

struct tile_element *
tile_add_feature(struct tile *t, void *ft, int x, int y)
{
	struct tile_element *tel;

	tel = Malloc(sizeof(struct tile_element), M_RG);
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
tile_add_pixmap(struct tile *t, struct pixmap *px, int x, int y)
{
	struct tile_element *tel;

	tel = Malloc(sizeof(struct tile_element), M_RG);
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
			text_tmsg(MSG_INFO, 500,
			    _("Destroying unreferenced pixmap `%s'."),
			    px->name);
			TAILQ_REMOVE(&t->ts->pixmaps, px, pixmaps);
			pixmap_destroy(px);
			Free(px, M_RG);
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
	write_uint16(buf, t->su->w);
	write_uint16(buf, t->su->h);

	nelements_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(tel, &t->elements, elements) {
		write_uint32(buf, (Uint32)tel->type);
		write_uint8(buf, (Uint8)tel->visible);

		switch (tel->type) {
		case TILE_FEATURE:
			{
				struct feature *ft = tel->tel_feature.ft;

				dprintf("%s: saving %s feature ref\n", t->name,
				    ft->name);
				write_string(buf, ft->name);
				write_sint32(buf, (Sint32)tel->tel_feature.x);
				write_sint32(buf, (Sint32)tel->tel_feature.y);
			}
			break;
		case TILE_PIXMAP:
			{
				struct pixmap *px = tel->tel_pixmap.px;

				dprintf("%s: saving %s pixmap ref\n", t->name,
				    px->name);
				write_string(buf, px->name);
				write_sint32(buf, (Sint32)tel->tel_pixmap.x);
				write_sint32(buf, (Sint32)tel->tel_pixmap.y);
				write_uint8(buf, (Uint8)tel->tel_pixmap.alpha);
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
	Uint16 w, h;
	Uint8 flags;
	
	flags = read_uint8(buf);
	w = read_uint16(buf);
	h = read_uint16(buf);
	tile_scale(ts, t, w, h, flags);

	nelements = read_uint32(buf);
	dprintf("%s: %ux%u, %u elements\n", t->name, w, h, nelements);
	for (i = 0; i < nelements; i++) {
		enum tile_element_type type;
		struct tile_element *tel;
		int visible;

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
				tel = tile_add_feature(t, ft, x, y);
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
				tel = tile_add_pixmap(t, px, x, y);
				tel->tel_pixmap.alpha = alpha;
				tel->visible = visible;
			}
			break;
		default:
			dprintf("unimplemented tile element %d\n", type);
			break;
		}
	}
	t->flags &= ~TILE_DIRTY;
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
element_closed(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct tileview *tv = argv[1].p;

	if (tv->edit_mode)
		tile_close_element(tv);
}

static void
pixmap_motion(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	struct tileview_ctrl *ctrl = argv[1].p;
	struct pixmap *px = argv[2].p;
	int xoffs = argv[3].i;
	int yoffs = argv[4].i;
	struct tile *t = tv->tile;
	int w = tileview_int(ctrl, 2);
	int h = tileview_int(ctrl, 3);

	if (w != px->su->w || h != px->su->h) 
		pixmap_scale(px, w, h, xoffs, yoffs);
}

static void
pixmap_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	struct tileview_ctrl *ctrl = argv[1].p;
	struct pixmap *px = argv[2].p;
	int xoffs = argv[3].i;
	int yoffs = argv[4].i;
	struct tile *t = tv->tile;

	tile_generate(t);
	view_scale_surface(t->su, tv->scaled->w, tv->scaled->h, &tv->scaled);
	t->flags &= ~(TILE_DIRTY);
}

void
tile_open_element(struct tileview *tv, struct tile_element *tel,
    struct window *pwin)
{
	switch (tel->type) {
	case TILE_FEATURE:
		{
			struct window *win;
			struct feature *ft = tel->tel_feature.ft;

			tv->state = TILEVIEW_FEATURE_EDIT;
			tv->tv_feature.ft = ft;

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
			} else {
				tv->tv_feature.win = NULL;
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
			tv->tv_pixmap.ctrl->motion =
			    event_new(tv, NULL, pixmap_motion, "%p,%p",
			    tv->tv_pixmap.ctrl, tel->tel_pixmap.px);
			tv->tv_pixmap.ctrl->buttonup =
			    event_new(tv, NULL, pixmap_buttonup, "%p,%p",
			    tv->tv_pixmap.ctrl, tel->tel_pixmap.px);

			win = pixmap_edit(tv, tel);
			window_attach(pwin, win);
			window_show(win);
			tv->tv_pixmap.win = win;
			event_new(win, "window-close", element_closed, "%p",
			    tv);
	
			tv->tile->flags |= TILE_DIRTY;
		}
		break;
	}
	tv->edit_mode = 1;
}

void
tile_close_element(struct tileview *tv)
{
	switch (tv->state) {
	case TILEVIEW_FEATURE_EDIT:
		if (tv->tv_feature.ft->ops->flags & FEATURE_AUTOREDRAW) {
			tileview_set_autoredraw(tv, 0, 0);
		}
		if (tv->tv_feature.win != NULL) {
			view_detach(tv->tv_feature.win);
			tv->tv_feature.win = NULL;
		}
		break;
	case TILEVIEW_PIXMAP_EDIT:
		if (tv->tv_pixmap.win != NULL) {
			tileview_remove_ctrl(tv, tv->tv_pixmap.ctrl);
			view_detach(tv->tv_pixmap.win);
			tv->tv_pixmap.win = NULL;
			tv->tv_pixmap.ctrl = NULL;
		}
		break;
	default:
		break;
	}
	tv->state = TILEVIEW_TILE_EDIT;
	tv->edit_mode = 0;
}

static void
insert_pixmap(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *etl = argv[3].p;
	struct tlist_item *eit;
	struct pixmap *px;
	struct tile_element *tel;
	unsigned int pixno = 0;
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

	tel = tile_add_pixmap(tv->tile, px, 0, 0);

	if (tv->edit_mode) {
		tile_close_element(tv);
	}
	tile_open_element(tv, tel, pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, etl, "tlist-poll", NULL);
	tlist_unselect_all(etl);
	TAILQ_FOREACH(eit, &etl->items, items) {
		struct tile_element *tel;

		if (strcmp(eit->class, "pixmap") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_pixmap.px == px) {
			tlist_select(etl, eit);
			break;
		}
	}
}

static void
insert_fill(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *etl = argv[3].p;
	struct tlist_item *eit;
	struct fill *fill;
	struct tile_element *tel;

	fill = Malloc(sizeof(struct fill), M_RG);
	fill_init(fill, tv->ts, 0);
	TAILQ_INSERT_TAIL(&tv->ts->features, FEATURE(fill), features);
	tel = tile_add_feature(tv->tile, fill, 0, 0);

	if (tv->edit_mode) {
		tile_close_element(tv);
	}
	tile_open_element(tv, tel, pwin);

	/* Select the newly inserted feature. */
	event_post(NULL, etl, "tlist-poll", NULL);
	tlist_unselect_all(etl);
	TAILQ_FOREACH(eit, &etl->items, items) {
		struct tile_element *tel;

		if (strcmp(eit->class, "feature") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_feature.ft == FEATURE(fill)) {
			tlist_select(etl, eit);
			break;
		}
	}
}

static void
poll_items(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
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
	
			it = tlist_insert(tl, ICON(OBJ_ICON), "%s%s",
			    ft->name,
			    tel->visible ? "" : " (invisible)");
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
				    "%s%s", fsk->sk->name,
				    fsk->visible ? "" : " (invisible)");
				it->class = "feature-sketch";
				it->p1 = fsk;
			}

			TAILQ_FOREACH(fpx, &ft->pixmaps, pixmaps) {
				it = tlist_insert(tl, ICON(DRAWING_ICON),
				    "%s (%d,%d) %s", fpx->px->name,
				    fpx->x, fpx->y,
				    fpx->visible ? "" : " (invisible)");
				it->class = "feature-pixmap";
				it->p1 = fpx;
			}
		} else if (tel->type == TILE_PIXMAP) {
			struct pixmap *px = tel->tel_pixmap.px;

			it = tlist_insert(tl, ICON(OBJ_ICON), "%s%s", px->name,
			    tel->visible ? "" : " (invisible)");
			it->class = "pixmap";
			it->p1 = tel;
			it->depth = 0;
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
	int replace = argv[4].i;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct tlist_item *it = NULL;

	if (replace) {
		if (tv->edit_mode) {
			tile_close_element(tv);
		}
		it = tlist_item_selected(tl);
	} else {
		if (strcmp(sndr->type, "button") != 0)
			tv->edit_mode = !tv->edit_mode;

		if (!tv->edit_mode) {
			tile_close_element(tv);
			return;
		} else {
			it = tlist_item_selected(tl);
		}
	}
	if (it == NULL) {
		tv->edit_mode = 0;
		return;
	}
	if (strcmp(it->class, "feature") == 0 ||
	    strcmp(it->class, "pixmap") == 0) {
		struct tile_element *tel = it->p1;

		tile_open_element(tv, tel, pwin);
	}
}

static void
delete_element(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	struct tlist *etl = argv[2].p;
	int detach_only = argv[3].i;
	struct tlist_item *it;
	struct tile_element *tel;

	if ((it = tlist_item_selected(etl)) == NULL) {
		return;
	}
	tel = it->p1;

	/* XXX check that it's the element being deleted */
	if (tv->edit_mode)
		tile_close_element(tv);
	
	if (strcmp(it->class, "feature") == 0) {
		tile_remove_feature(t, tel->tel_feature.ft, !detach_only);
	} else if (strcmp(it->class, "pixmap") == 0) {
		tile_remove_pixmap(t, tel->tel_pixmap.px, !detach_only);
	} else if (strcmp(it->class, "sketch") == 0) {
		//
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
	struct tileset *ts = tv->ts;
	struct tile *t = tv->tile;
	int w = widget_get_int(msb, "xvalue");
	int h = widget_get_int(msb, "yvalue");
	int flags = 0;

	if (widget_get_int(ckey_cb, "state") == 1)
		flags |= SDL_SRCALPHA;
	if (widget_get_int(alpha_cb, "state") == 1)
		flags |= SDL_SRCCOLORKEY;

	tile_scale(ts, t, w, h, flags);
	tileview_set_zoom(tv, 100, 0);
	view_detach(dlg_w);
}

static void
resize_tile_dlg(int argc, union evarg *argv)
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

	win = window_new(WINDOW_MODAL|WINDOW_DETACH|WINDOW_NO_RESIZE|
		         WINDOW_NO_MINIMIZE, NULL);
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
		b = button_new(box, _("OK"));
		event_new(b, "button-pushed", resize_tile,
		    "%p,%p,%p,%p,%p", tv, msb, win, ckey_cb, alpha_cb);

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

	if ((it = tlist_item_selected(tl)) == NULL ||
	    (strcmp(it->class, "feature") != 0 &&
	     strcmp(it->class, "pixmap") != 0)) {
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

	if ((it = tlist_item_selected(tl)) == NULL ||
	    (strcmp(it->class, "feature") != 0 &&
	     strcmp(it->class, "pixmap") != 0)) {
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

	if ((it = tlist_item_selected(tl)) == NULL ||
	    strcmp(it->class, "pixmap") != 0) {
		return;
	}
	tel = it->p1;
	tel->visible = !tel->visible;
	t->flags |= TILE_DIRTY;
}

struct window *
tile_edit(struct tileset *ts, struct tile *t)
{
	struct window *win;
	struct box *box;
	struct AGMenu *m;
	struct AGMenuItem *item;
	struct tileview *tv;
	struct tlist *etl;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, "%s <%s>", t->name, OBJECT(ts)->name);
	event_new(win, "window-close", close_tile, "%p,%p", ts, t);
	
	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, t, TILEVIEW_AUTOREGEN);
	
	etl = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(etl, TLIST_POLL|TLIST_TREE);
	WIDGET(etl)->flags &= ~(WIDGET_WFILL);
	tlist_prescale(etl, _("FEATURE #000"), 5);
	event_new(etl, "tlist-poll", poll_items, "%p,%p", ts, t);

	item = tlist_set_popup(etl, "feature");
	{
		ag_menu_action(item, _("Edit feature"), ICON(OBJEDIT_ICON),
		    SDLK_e, KMOD_CTRL,
		    edit_element, "%p,%p,%p,%p", tv, etl, win, 1);
		
		ag_menu_action(item, _("Detach feature"), ICON(TRASH_ICON),
		    SDLK_d, KMOD_CTRL,
		    delete_element, "%p,%p,%i", tv, etl, 1);
		
		ag_menu_action(item, _("Destroy feature"), ICON(TRASH_ICON),
		    SDLK_x, KMOD_CTRL,
		    delete_element, "%p,%p,%i", tv, etl, 0);
		
		ag_menu_separator(item);
		
		ag_menu_action(item, _("Move up"), ICON(OBJMOVEUP_ICON),
		    SDLK_u, KMOD_SHIFT,
		    move_element_up, "%p,%p", tv, etl);

		ag_menu_action(item, _("Move down"), ICON(OBJMOVEDOWN_ICON),
		    SDLK_d, KMOD_SHIFT,
		    move_element_down, "%p,%p", tv, etl);
	}

	item = tlist_set_popup(etl, "pixmap");
	{
		ag_menu_action(item, _("Edit pixmap"), ICON(OBJEDIT_ICON),
		    SDLK_e, KMOD_CTRL,
		    edit_element, "%p,%p,%p,%p", tv, etl, win, 1);
		
		ag_menu_action(item, _("Detach pixmap"), ICON(TRASH_ICON),
		    SDLK_d, KMOD_CTRL,
		    delete_element, "%p,%p,%i", tv, etl, 1);
		
		ag_menu_action(item, _("Destroy pixmap"), ICON(TRASH_ICON),
		    SDLK_x, KMOD_CTRL,
		    delete_element, "%p,%p,%i", tv, etl, 0);
		
		ag_menu_separator(item);
		
		ag_menu_action(item, _("Move up"), ICON(OBJMOVEUP_ICON),
		    SDLK_u, KMOD_SHIFT,
		    move_element_up, "%p,%p", tv, etl);

		ag_menu_action(item, _("Move down"), ICON(OBJMOVEDOWN_ICON),
		    SDLK_d, KMOD_SHIFT,
		    move_element_down, "%p,%p", tv, etl);
		
		ag_menu_separator(item);

		ag_menu_action(item, _("Toggle visibility"),
		    ICON(OBJCREATE_ICON),
		    SDLK_d, KMOD_SHIFT,
		    visible_element, "%p,%p", tv, etl);
	}

	m = ag_menu_new(win);
	item = ag_menu_add_item(m, _("Features"));
	{
		ag_menu_action(item, _("Fill"), NULL,
		    SDLK_f, KMOD_CTRL|KMOD_SHIFT,
		    insert_fill, "%p,%p,%p", tv, win, etl);
		    
		ag_menu_action(item, _("Sketch projection"), NULL,
		    SDLK_s, KMOD_CTRL|KMOD_SHIFT,
		    NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Polygon"), NULL,
		    SDLK_p, KMOD_CTRL|KMOD_SHIFT,
		    NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Extruded base"), NULL,
		    SDLK_e, KMOD_CTRL|KMOD_SHIFT,
		    NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Solid of revolution"), NULL,
		    SDLK_r, KMOD_CTRL|KMOD_SHIFT,
		    NULL, "%p,%p", ts, t);
	}
	
	item = ag_menu_add_item(m, _("Pixmaps"));
	{
		ag_menu_action(item, _("Insert pixmap..."),
		    ICON(DRAWING_ICON),
		    SDLK_p, KMOD_CTRL,
		    insert_pixmap, "%p,%p,%p", tv, win, etl);
	}

	item = ag_menu_add_item(m, _("Edit"));
	{
		ag_menu_action(item, _("Resize tile..."),
		    ICON(RESIZE_TOOL_ICON),
		    SDLK_r, KMOD_CTRL,
		    resize_tile_dlg, "%p,%p", tv, win);
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
			object_attach(fbox, etl);
	
			fbu = button_new(fbox, _("Edit"));
			WIDGET(fbu)->flags |= WIDGET_WFILL;
			button_set_sticky(fbu, 1);
			widget_bind(fbu, "state", WIDGET_INT, &tv->edit_mode);

			event_new(fbu, "button-pushed", edit_element,
			    "%p,%p,%p,%i", tv, etl, win, 0);
			event_new(etl, "tlist-dblclick", edit_element,
			    "%p,%p,%p,%i", tv, etl, win, 1);
		}
		
		object_attach(box, tv);
		widget_focus(tv);
	}

	t->used++;

	window_scale(win, -1, -1);
	window_set_geometry(win,
	    view->w/4, view->h/4,
	    view->w/2, view->h/2);
	return (win);
}


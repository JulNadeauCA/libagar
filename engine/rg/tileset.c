/*	$Csoft: tileset.c,v 1.13 2005/02/22 08:44:16 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/view.h>

#include <ctype.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/label.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/menu.h>

#include "tileset.h"

const struct version tileset_ver = {
	"agar tileset",
	0, 0
};

const struct object_ops tileset_ops = {
	tileset_init,
	tileset_reinit,
	tileset_destroy,
	tileset_load,
	tileset_save,
	tileset_edit
};

extern const struct feature_ops fill_ops;

const struct feature_ops *feature_tbl[] = {
	&fill_ops,
	NULL
};

void
tileset_init(void *obj, const char *name)
{
	struct tileset *ts = obj;

	object_init(ts, "tileset", name, &tileset_ops);
	gfx_alloc_pvt(ts, "tiles");
	OBJECT(ts)->flags |= OBJECT_REOPEN_ONLOAD;

	pthread_mutex_init(&ts->lock, NULL);
	TAILQ_INIT(&ts->tiles);
	TAILQ_INIT(&ts->sketches);
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);

	ts->icon = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
	    32, 32, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
 	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
#else
	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
#endif
	);
	if (ts->icon == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	ts->fmt = ts->icon->format;
}

void
tileset_reinit(void *obj)
{
	struct tileset *ts = obj;
	struct tile *t, *nt;
	struct sketch *sk, *nsk;
	struct pixmap *px, *npx;
	struct feature *ft, *nft;

	pthread_mutex_lock(&ts->lock);

	for (t = TAILQ_FIRST(&ts->tiles);
	     t != TAILQ_END(&ts->tiles);
	     t = nt) {
		nt = TAILQ_NEXT(t, tiles);
		tile_destroy(t);
		Free(t, M_RG);
	}
	TAILQ_INIT(&ts->tiles);

	for (sk = TAILQ_FIRST(&ts->sketches);
	     sk != TAILQ_END(&ts->sketches);
	     sk = nsk) {
		nsk = TAILQ_NEXT(sk, sketches);
#ifdef DEBUG
		if (sk->nrefs > 0)
			dprintf("sketch %s nrefs > 0\n", sk->name);
#endif
//		sketch_destroy(sk);
		vg_destroy(sk->vg);
		Free(sk, M_RG);
	}
	TAILQ_INIT(&ts->sketches);

	for (px = TAILQ_FIRST(&ts->pixmaps);
	     px != TAILQ_END(&ts->pixmaps);
	     px = npx) {
		npx = TAILQ_NEXT(px, pixmaps);
		pixmap_destroy(px);
		Free(px, M_RG);
	}
	TAILQ_INIT(&ts->pixmaps);

	for (ft = TAILQ_FIRST(&ts->features);
	     ft != TAILQ_END(&ts->features);
	     ft = nft) {
		nft = TAILQ_NEXT(ft, features);
		feature_destroy(ft);
		Free(ft, M_RG);
	}
	TAILQ_INIT(&ts->features);

	pthread_mutex_unlock(&ts->lock);
}

void
tileset_destroy(void *obj)
{
	struct tileset *ts = obj;

	pthread_mutex_destroy(&ts->lock);
	SDL_FreeSurface(ts->icon);
}

int
tileset_load(void *obj, struct netbuf *buf)
{
	struct tileset *ts = obj;
	struct pixmap *px;
	Uint32 nsketches, npixmaps, nfeatures, ntiles;
	Uint32 i;

	if (version_read(buf, &tileset_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&ts->lock);

	/* Load the vectorial sketches. */
	nsketches = read_uint32(buf);
	for (i = 0; i < nsketches; i++) {
		struct sketch *sk;
		int vgflags;

		sk = Malloc(sizeof(struct sketch), M_RG);
		copy_string(sk->name, buf, sizeof(sk->name));
		vgflags = (int)read_uint32(buf);

		sk->vg = Malloc(sizeof(struct vg), M_VG);
		vg_init(sk->vg, vgflags);
		if (vg_load(sk->vg, buf) == -1) {
			vg_destroy(sk->vg);
			Free(sk->vg, M_VG);
			Free(sk, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->sketches, sk, sketches);
		dprintf("%s: added sketch `%s'\n", OBJECT(ts)->name, sk->name);
	}

	/* Load the pixmaps. */
	npixmaps = read_uint32(buf);
	for (i = 0; i < npixmaps; i++) {
		struct pixmap *px;

		px = Malloc(sizeof(struct pixmap), M_RG);
		pixmap_init(px, ts, 0);
		if (pixmap_load(px, buf) == -1) {
			Free(px, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->pixmaps, px, pixmaps);
		dprintf("%s: added pixmap `%s'\n", OBJECT(ts)->name, px->name);
	}

	/* Load the features. */
	nfeatures = read_uint32(buf);
	for (i = 0; i < nfeatures; i++) {
		const struct feature_ops *ftops;
		char name[FEATURE_NAME_MAX];
		char type[FEATURE_TYPE_MAX];
		size_t len;
		struct feature *ft;
		int flags;
		
		copy_string(name, buf, sizeof(name));
		copy_string(type, buf, sizeof(type));
		flags = (int)read_uint32(buf);

		for (ftops = feature_tbl[0]; ftops != NULL; ftops++) {
			if (strcmp(ftops->type, type) == 0)
				break;
		}
		if (ftops == NULL) {
			/* XXX ignore? */
			error_set("%s: unknown feature type `%s'", name, type);
			goto fail;
		}

		ft = Malloc(ftops->len, M_RG);
		ft->ops = ftops;
		ft->ops->init(ft, ts, flags);

		if (feature_load(ft, buf) == -1) {
			feature_destroy(ft);
			Free(ft, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->features, ft, features);
	}
	
	/* Load the tiles. */
	ntiles = read_uint32(buf);
	for (i = 0; i < ntiles; i++) {
		char name[TILE_NAME_MAX];
		struct tile *t;
		
		t = Malloc(sizeof(struct tile), M_RG);
		copy_string(name, buf, sizeof(name));
		tile_init(t, ts, name);
		if (tile_load(ts, t, buf) == -1) {
			tile_destroy(t);
			Free(t, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	}

	/* Resolve the pixmap brush references. */
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		struct pixmap_brush *pbr;
		struct pixmap *ppx;

		TAILQ_FOREACH(pbr, &px->brushes, brushes) {
			if (pbr->px != NULL) {
				continue;
			}
			TAILQ_FOREACH(ppx, &ts->pixmaps, pixmaps) {
				if (strcmp(pbr->px_name, ppx->name) == 0) {
					pbr->px = ppx;
					pbr->px->nrefs++;
					break;
				}
			}
			if (ppx == NULL) {
				fatal("%s: bad pixmap ref", pbr->px_name);
			}
		}
	}
	pthread_mutex_unlock(&ts->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ts->lock);
	return (-1);
}

int
tileset_save(void *obj, struct netbuf *buf)
{
	struct tileset *ts = obj;
	Uint32 nsketches = 0, npixmaps = 0, ntiles = 0, nfeatures = 0;
	off_t nsketches_offs, npixmaps_offs, ntiles_offs, nfeatures_offs;
	struct sketch *sk;
	struct pixmap *px;
	struct tile *t;
	struct feature *ft;

	version_write(buf, &tileset_ver);

	pthread_mutex_lock(&ts->lock);

	/* Save the vectorial sketches. */
	nsketches_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		dprintf("%s: saving sketch %s\n", OBJECT(ts)->name, sk->name);
		write_string(buf, sk->name);
		write_uint32(buf, (Uint32)sk->vg->flags);
		vg_save(sk->vg, buf);
		nsketches++;
	}
	pwrite_uint32(buf, nsketches, nsketches_offs);

	/* Save the pixmaps. */
	npixmaps_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		dprintf("%s: saving pixmap %s\n", OBJECT(ts)->name, px->name);
		pixmap_save(px, buf);
		npixmaps++;
	}
	pwrite_uint32(buf, npixmaps, npixmaps_offs);

	/* Save the features. */
	nfeatures_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(ft, &ts->features, features) {
		dprintf("%s: saving feature %s (%s)\n", OBJECT(ts)->name,
		    ft->name, ft->ops->type);
		write_string(buf, ft->name);
		write_string(buf, ft->ops->type);
		write_uint32(buf, ft->flags);
		feature_save(ft, buf);
		nfeatures++;
	}
	pwrite_uint32(buf, nfeatures, nfeatures_offs);
	
	/* Save the tiles. */
	dprintf("%s: saving tiles\n", OBJECT(ts)->name);
	ntiles_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		tile_save(t, buf);
		ntiles++;
	}
	pwrite_uint32(buf, ntiles, ntiles_offs);
	
	pthread_mutex_unlock(&ts->lock);
	return (0);
}

static void
poll_pixmaps(int argc, union evarg *argv)
{
	char label[TLIST_LABEL_MAX];
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct pixmap *px;
	struct tlist_item *it;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		snprintf(label, sizeof(label), "%s (%ux%u) [#%u]",
		    px->name, px->su->w, px->su->h, px->nrefs);

		it = tlist_insert_item(tl, px->su, label, px);
		it->class = "pixmap";
	}

	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
poll_tiles(int argc, union evarg *argv)
{
	char label[TLIST_LABEL_MAX];
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t;
	struct tlist_item *it;
	struct tile_element *tel;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		snprintf(label, sizeof(label), "%s (%ux%u)", t->name,
		    t->su->w, t->su->h);
		it = tlist_insert_item(tl, t->su, label, t);
		it->depth = 0;
		it->class = "tile";

		if (!TAILQ_EMPTY(&t->elements)) {
			it->flags |= TLIST_HAS_CHILDREN;
			if (!tlist_visible_children(tl, it))
				continue;
		}
	
		TAILQ_FOREACH(tel, &t->elements, elements) {
			switch (tel->type) {
			case TILE_FEATURE:
				{
					struct feature *ft =
					    tel->tel_feature.ft;
					struct feature_sketch *fts;

					snprintf(label, sizeof(label),
					    "%s [%d,%d] %s", ft->name,
					    tel->tel_feature.x,
					    tel->tel_feature.y,
					    tel->visible ? "" : "(invisible)");

					it = tlist_insert_item(tl,
					    ICON(OBJ_ICON), label, tel);
					it->depth = 1;
					it->class = "tile-feature";

					TAILQ_FOREACH(fts, &ft->sketches,
					    sketches) {
						snprintf(label, sizeof(label),
						    "%s [at %d,%d]%s",
						    fts->sk->name,
						    fts->x, fts->y,
						    fts->visible ? "" :
						    "(invisible)");

						it = tlist_insert_item(tl,
						    ICON(DRAWING_ICON), label,
						    fts);
						it->depth = 2;
						it->class = "feature-sketch";
					}
				}
				break;
			case TILE_PIXMAP:
				{
					struct pixmap *px = tel->tel_pixmap.px;

					snprintf(label, sizeof(label),
					    "%s (%d,%d)%s", px->name,
					    tel->tel_pixmap.px->su->w,
					    tel->tel_pixmap.px->su->h,
					    tel->visible ? "" : "(invisible)");

					it = tlist_insert_item(tl, px->su,
					    label, tel);
					it->depth = 1;
					it->class = "tile-pixmap";
				}
				break;
			}
		}
	}
	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static char ins_tile_name[TILE_NAME_MAX];
static int ins_tile_w = 32;
static int ins_tile_h = 32;
static int ins_tile_alpha = 1;
static int ins_tile_ckeying = 0;

static void
insert_tile(int argc, union evarg *argv)
{
	struct window *pwin = argv[1].p;
	struct tileset *ts = argv[2].p;
	struct tile *t;
	int flags = 0;

	if (ins_tile_alpha)	flags |= TILE_SRCALPHA;
	if (ins_tile_ckeying)	flags |= TILE_SRCCOLORKEY;

	if (ins_tile_name[0] == '\0') {
		unsigned int nameno = 0;
tryname1:
		snprintf(ins_tile_name, sizeof(ins_tile_name), _("Tile #%d"),
		    nameno);
		TAILQ_FOREACH(t, &ts->tiles, tiles) {
			if (strcmp(t->name, ins_tile_name) == 0)
				break;
		}
		if (t != NULL) {
			nameno++;
			goto tryname1;
		}
	} else {
tryname2:
		TAILQ_FOREACH(t, &ts->tiles, tiles) {
			if (strcmp(t->name, ins_tile_name) == 0)
				break;
		}
		if (t != NULL) {
			char *np;
			int num = -1;

			for (np = &ins_tile_name[strlen(ins_tile_name)-1];
			     np > &ins_tile_name[0];
			     np--) {
				if (*np == '#' && *(np+1) != '\0') {
					np++;
					num = atoi(np) + 1;
					snprintf(np, sizeof(ins_tile_name) -
					    (np-ins_tile_name)-1, "%d", num);
					break;
				}
				if (!isdigit(*np)) {
					strlcat(ins_tile_name, "_",
					    sizeof(ins_tile_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	t = Malloc(sizeof(struct tile), M_RG);
	tile_init(t, ts, ins_tile_name);
	tile_scale(ts, t, ins_tile_w, ins_tile_h, flags);
	TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);

	ins_tile_name[0] = '\0';
	view_detach(pwin);
}

static void
insert_tile_dlg(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct window *pwin = argv[2].p;
	struct window *win;
	struct box *btnbox;
	struct button *btn;
	struct textbox *tb;
	struct mspinbutton *msb;
	struct checkbox *cb;

	win = window_new(WINDOW_MODAL|WINDOW_DETACH|WINDOW_NO_RESIZE|
	                 WINDOW_NO_MINIMIZE, NULL);
	window_set_caption(win, _("Insert new tile"));
	
	tb = textbox_new(win, _("Name:"));
	widget_bind(tb, "string", WIDGET_STRING, ins_tile_name,
	    sizeof(ins_tile_name));
	widget_focus(tb);

	msb = mspinbutton_new(win, "x", _("Size:"));
	widget_bind(msb, "xvalue", WIDGET_INT, &ins_tile_w);
	widget_bind(msb, "yvalue", WIDGET_INT, &ins_tile_h);
	mspinbutton_set_range(msb, TILE_SIZE_MIN, TILE_SIZE_MAX);

	cb = checkbox_new(win, _("Alpha blending"));
	widget_bind(cb, "state", WIDGET_INT, &ins_tile_alpha);
	
	cb = checkbox_new(win, _("Colorkeying"));
	widget_bind(cb, "state", WIDGET_INT, &ins_tile_ckeying);

	btnbox = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	{
		btn = button_new(btnbox, "OK");
		event_new(btn, "button-pushed", insert_tile, "%p,%p", win, ts);
		
		btn = button_new(btnbox, "Cancel");
		event_new(btn, "button-pushed", window_generic_detach, "%p",
		    win);
	}

	window_attach(pwin, win);
	window_show(win);
}

static void
delete_tiles(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tileset *ts = argv[2].p;
	struct tlist_item *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		struct tile *t = it->p1;

		if (!it->selected) {
			continue;
		}
		if (t->used) {
			text_msg(MSG_ERROR, _("The tile `%s' is in use."),
			    t->name);
			continue;
		}
		TAILQ_REMOVE(&ts->tiles, t, tiles);
		Free(t, M_RG);
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
edit_tiles(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct window *pwin = argv[3].p;
	struct window *win;
	struct tlist_item *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		struct tile *t = it->p1;

		if (!it->selected ||
		    strcmp(it->class, "tile") != 0) {
			continue;
		}
		if ((win = tile_edit(ts, t)) != NULL) {
			window_attach(pwin, win);
			window_show(win);
		}
	}
	pthread_mutex_unlock(&ts->lock);
}

struct window *
tileset_edit(void *p)
{
	struct tileset *ts = p;
	struct window *win;
	struct tlist *tl_tiles, *tl_pixmaps;
	struct box *box, *hbox, *bbox;
	struct textbox *tb;
	struct mspinbutton *msb;
	struct AGMenu *m;
	struct AGMenuItem *mi;
	struct button *bu;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Tile set: %s"), OBJECT(ts)->name);
	window_set_position(win, WINDOW_LOWER_CENTER, 0);

	tl_tiles = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl_tiles, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
	event_new(tl_tiles, "tlist-poll", poll_tiles, "%p", ts);
	
	tl_pixmaps = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl_pixmaps, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
	event_new(tl_pixmaps, "tlist-poll", poll_pixmaps, "%p", ts);

	mi = tlist_set_popup(tl_tiles, "tile");
	{
		ag_menu_action(mi, _("Edit tile..."), ICON(OBJEDIT_ICON), 0, 0,
		    edit_tiles, "%p,%p,%p", ts, tl_tiles, win);
		ag_menu_action(mi, _("Delete tile"), ICON(TRASH_ICON), 0, 0,
		    delete_tiles, "%p,%p", tl_tiles, ts);
	}

	m = ag_menu_new(win);
	mi = ag_menu_add_item(m, _("Tileset"));
	{
		ag_menu_action(mi, _("Insert tile..."), ICON(SHIFT_TOOL_ICON),
		    0, 0,
		    insert_tile_dlg, "%p,%p", ts, win);
#if 0
		ag_menu_action(mi, _("Insert feature..."), NULL, 0, 0,
		    insert_tile_dlg, "%p,%p", ts, win);
		ag_menu_action(mi, _("Insert sketch..."), NULL, 0, 0,
		    insert_tile_dlg, "%p,%p", ts, win);
		ag_menu_action(mi, _("Insert pixmap..."), NULL, 0, 0,
		    insert_tile_dlg, "%p,%p", ts, win);
#endif
	}
	
	hbox = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HFILL|BOX_HOMOGENOUS);

	box = box_new(hbox, BOX_VERT, BOX_HFILL);
	box_set_spacing(box, 0);
	box_set_padding(box, 2);
	box_set_depth(box, -1);
	{
		object_attach(box, tl_tiles);

		bbox = box_new(box, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
		{
			bu = button_new(bbox, _("Insert tile"));
			event_new(bu, "button-pushed", insert_tile_dlg,
			    "%p,%p", ts, win);

			bu = button_new(bbox, _("Edit tile(s)"));
			event_new(bu, "button-pushed", edit_tiles,
			    "%p,%p,%p", ts, tl_tiles, win);
			event_new(tl_tiles, "tlist-dblclick", edit_tiles,
			    "%p,%p,%p", ts, tl_tiles, win);

			bu = button_new(bbox, _("Delete tile(s)"));
			event_new(bu, "button-pushed", delete_tiles,
			    "%p,%p", tl_tiles, ts);
		}
	}
	
	box = box_new(hbox, BOX_VERT, BOX_HFILL);
	box_set_spacing(box, 0);
	box_set_depth(box, -1);
	{
		object_attach(box, tl_pixmaps);
	}
	return (win);
}

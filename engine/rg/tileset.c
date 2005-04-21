/*	$Csoft: tileset.c,v 1.29 2005/04/18 03:38:35 vedge Exp $	*/

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
#include <engine/view.h>

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/menu.h>
#include <engine/widget/notebook.h>

#include "tileset.h"

#include <ctype.h>

const struct version tileset_ver = {
	"agar tileset",
	1, 0
};

const struct object_ops tileset_ops = {
	tileset_init,
	tileset_reinit,
	tileset_destroy,
	tileset_load,
	tileset_save,
#ifdef EDITION
	tileset_edit
#else
	NULL
#endif
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

	/*
	 * Tilesets must be resident for the tiles to show up in the source
	 * artwork list of the map editor.
	 */
	OBJECT(ts)->flags |= OBJECT_REOPEN_ONLOAD|OBJECT_EDIT_RESIDENT;

	pthread_mutex_init(&ts->lock, NULL);
	TAILQ_INIT(&ts->tiles);
	TAILQ_INIT(&ts->sketches);
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->animations);

	ts->icon = SDL_CreateRGBSurface(
	    SDL_SWSURFACE|SDL_SRCALPHA|SDL_SRCCOLORKEY,
	    32, 32, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
 	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
#else
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000,
	    0xff000000
#endif
	);
	if (ts->icon == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	ts->fmt = ts->icon->format;
	ts->flags = 0;
	ts->max_sprites = 0;
}

void
tileset_reinit(void *obj)
{
	struct tileset *ts = obj;
	struct tile *t, *nt;
	struct sketch *sk, *nsk;
	struct pixmap *px, *npx;
	struct feature *ft, *nft;
	struct animation *ani, *nani;

	pthread_mutex_lock(&ts->lock);
	
	for (t = TAILQ_FIRST(&ts->tiles);
	     t != TAILQ_END(&ts->tiles);
	     t = nt) {
		nt = TAILQ_NEXT(t, tiles);
		tile_destroy(t);
		Free(t, M_RG);
	}

	ts->max_sprites = 0;
	gfx_alloc_sprites(OBJECT(ts)->gfx, 0);

	for (sk = TAILQ_FIRST(&ts->sketches);
	     sk != TAILQ_END(&ts->sketches);
	     sk = nsk) {
		nsk = TAILQ_NEXT(sk, sketches);
#ifdef DEBUG
		if (sk->nrefs > 0)
			dprintf("sketch %s nrefs > 0\n", sk->name);
#endif
		sketch_destroy(sk);
		Free(sk, M_RG);
	}

	for (px = TAILQ_FIRST(&ts->pixmaps);
	     px != TAILQ_END(&ts->pixmaps);
	     px = npx) {
		npx = TAILQ_NEXT(px, pixmaps);
		pixmap_destroy(px);
		Free(px, M_RG);
	}

	for (ft = TAILQ_FIRST(&ts->features);
	     ft != TAILQ_END(&ts->features);
	     ft = nft) {
		nft = TAILQ_NEXT(ft, features);
		feature_destroy(ft);
		Free(ft, M_RG);
	}

	for (ani = TAILQ_FIRST(&ts->animations);
	     ani != TAILQ_END(&ts->animations);
	     ani = nani) {
		nani = TAILQ_NEXT(ani, animations);
		animation_destroy(ani);
		Free(ani, M_RG);
	}
	
	TAILQ_INIT(&ts->tiles);
	TAILQ_INIT(&ts->sketches);
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->animations);
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
	struct gfx *gfx = OBJECT(ts)->gfx;
	struct version ver;
	struct pixmap *px;
	Uint32 nsketches, npixmaps, nfeatures, ntiles, nanimations;
	Uint32 i;

	if (version_read(buf, &tileset_ver, &ver) != 0)
		return (-1);

	pthread_mutex_lock(&ts->lock);
	ts->flags = read_uint32(buf);
	ts->max_sprites = read_uint32(buf);

	/* Resize the sprite array. */
	gfx_alloc_sprites(gfx, ts->max_sprites);

	/* Load the vectorial sketches. */
	nsketches = read_uint32(buf);
	for (i = 0; i < nsketches; i++) {
		struct sketch *sk;

		sk = Malloc(sizeof(struct sketch), M_RG);
		sketch_init(sk, ts, 0);
		if (sketch_load(sk, buf) == -1) {
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

	/* Load the animation information. */
	nanimations = read_uint32(buf);
	for (i = 0; i < nanimations; i++) {
		char name[ANIMATION_NAME_MAX];
		struct animation *ani;
		int flags;
		
		ani = Malloc(sizeof(struct animation), M_RG);
		copy_string(name, buf, sizeof(name));
		flags = (int)read_uint32(buf);
		animation_init(ani, ts, name, flags);
		if (animation_load(ani, buf) == -1) {
			animation_destroy(ani);
			Free(ani, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->animations, ani, animations);
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
	Uint32 nsketches = 0, npixmaps = 0, ntiles = 0, nfeatures = 0,
	       nanims = 0;
	off_t nsketches_offs, npixmaps_offs, ntiles_offs, nfeatures_offs,
	      nanims_offs;
	struct sketch *sk;
	struct pixmap *px;
	struct animation *ani;
	struct tile *t;
	struct feature *ft;

	version_write(buf, &tileset_ver);

	pthread_mutex_lock(&ts->lock);

	write_uint32(buf, ts->flags);
	write_uint32(buf, ts->max_sprites);

	/* Save the vectorial sketches. */
	nsketches_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		sketch_save(sk, buf);
		nsketches++;
	}
	pwrite_uint32(buf, nsketches, nsketches_offs);

	/* Save the pixmaps. */
	npixmaps_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		pixmap_save(px, buf);
		npixmaps++;
	}
	pwrite_uint32(buf, npixmaps, npixmaps_offs);

	/* Save the features. */
	nfeatures_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(ft, &ts->features, features) {
		write_string(buf, ft->name);
		write_string(buf, ft->ops->type);
		write_uint32(buf, ft->flags);
		feature_save(ft, buf);
		nfeatures++;
	}
	pwrite_uint32(buf, nfeatures, nfeatures_offs);
	
	/* Save the tiles. */
	ntiles_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		tile_save(t, buf);
		ntiles++;
	}
	pwrite_uint32(buf, ntiles, ntiles_offs);
	
	/* Save the animation information. */
	nanims_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(ani, &ts->animations, animations) {
		write_string(buf, ani->name);
		write_uint32(buf, (Uint32)ani->flags);
		animation_save(ani, buf);
		nanims++;
	}
	pwrite_uint32(buf, nanims, nanims_offs);
	
	pthread_mutex_unlock(&ts->lock);
	return (0);
}

struct tile *
tileset_find_tile(struct tileset *ts, const char *name)
{
	struct tile *t;

	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		if (strcmp(t->name, name) == 0)
			break;
	}
	if (t == NULL) {
		error_set("%s: unexisting tile", name);
	}
	return (t);
}

struct sketch *
tileset_find_sketch(struct tileset *ts, const char *name)
{
	struct sketch *sk;

	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		if (strcmp(sk->name, name) == 0)
			break;
	}
	if (sk == NULL) {
		error_set("%s: unexisting sketch", name);
	}
	return (sk);
}

struct pixmap *
tileset_find_pixmap(struct tileset *ts, const char *name)
{
	struct pixmap *px;

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		if (strcmp(px->name, name) == 0)
			break;
	}
	if (px == NULL) {
		error_set("%s: unexisting pixmap", name);
	}
	return (px);
}

struct animation *
tileset_find_animation(struct tileset *ts, const char *name)
{
	struct animation *ani;

	TAILQ_FOREACH(ani, &ts->animations, animations) {
		if (strcmp(ani->name, name) == 0)
			break;
	}
	if (ani == NULL) {
		error_set("%s: unexisting animation", name);
	}
	return (ani);
}

#ifdef EDITION

static void
poll_art(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct pixmap *px;
	struct sketch *sk;
	struct tlist_item *it;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = tlist_insert(tl, px->su, "%s (%ux%u) [#%u]",
		    px->name, px->su->w, px->su->h, px->nrefs);
		it->p1 = px;
		it->class = "pixmap";
	}
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		it = tlist_insert(tl, ICON(DRAWING_ICON),
		    "%s (%ux%u %.0f%%) [#%u]", sk->name, sk->vg->su->w,
		    sk->vg->su->h, sk->vg->scale*100.0, sk->nrefs);
		it->class = "sketch";
		it->p1 = sk;
	}

	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
poll_anims(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct animation *ani;
	struct tlist_item *it;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(ani, &ts->animations, animations) {
		it = tlist_insert(tl, NULL, "%s (%ux%u) [#%u]", ani->name,
		    ani->w, ani->h, ani->nrefs);
		it->p1 = ani;
		it->class = "anim";
	}
	
	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
poll_tiles(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t;
	struct tlist_item *it;
	struct tile_element *tel;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		it = tlist_insert(tl, t->su, "%s (%ux%u)", t->name,
		    t->su->w, t->su->h);
		it->depth = 0;
		it->class = "tile";
		it->p1 = t;

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

					it = tlist_insert(tl, ICON(OBJ_ICON),
					    "%s [%d,%d] %s", ft->name,
					    tel->tel_feature.x,
					    tel->tel_feature.y,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->class = "tile-feature";
					it->p1 = tel;

					TAILQ_FOREACH(fts, &ft->sketches,
					    sketches) {
						it = tlist_insert(tl,
						    ICON(DRAWING_ICON),
						    "%s [at %d,%d]%s",
						    fts->sk->name,
						    fts->x, fts->y,
						    fts->visible ? "" :
						    "(invisible)");
						it->depth = 2;
						it->class = "feature-sketch";
						it->p1 = fts;
					}
				}
				break;
			case TILE_PIXMAP:
				{
					struct pixmap *px = tel->tel_pixmap.px;

					it = tlist_insert(tl, px->su,
					    "%s (%ux%u)%s", px->name,
					    tel->tel_pixmap.px->su->w,
					    tel->tel_pixmap.px->su->h,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->class = "tile-pixmap";
					it->p1 = tel;
				}
				break;
			case TILE_SKETCH:
				{
					struct sketch *sk = tel->tel_sketch.sk;

					it = tlist_insert(tl,
					    ICON(DRAWING_ICON),
					    "%s (%ux%u)%s", sk->name,
					    tel->tel_sketch.sk->vg->su->w,
					    tel->tel_sketch.sk->vg->su->h,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->class = "tile-pixmap";
					it->p1 = tel;
				}
				break;
			}
		}
	}
	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static char ins_tile_name[TILE_NAME_MAX];
static char ins_anim_name[TILE_NAME_MAX];
static int ins_tile_w = 32;
static int ins_tile_h = 32;
static int ins_alpha = 1;
static int ins_colorkey = 0;

static void
insert_tile(int argc, union evarg *argv)
{
	struct window *pwin = argv[1].p;
	struct tileset *ts = argv[2].p;
	struct gfx *gfx = OBJECT(ts)->gfx;
	struct tile *t;
	u_int flags = 0;

	if (ins_alpha)		flags |= TILE_SRCALPHA;
	if (ins_colorkey)	flags |= TILE_SRCCOLORKEY;

	if (ins_tile_name[0] == '\0') {
		u_int nameno = 0;
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
	t->sprite = gfx_insert_sprite(gfx, NULL);
	tile_scale(ts, t, ins_tile_w, ins_tile_h, flags, SDL_ALPHA_OPAQUE);
	TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	
	if (gfx->nsprites > ts->max_sprites)
		ts->max_sprites = gfx->nsprites;

	ins_tile_name[0] = '\0';
	view_detach(pwin);
}

static void
insert_anim(int argc, union evarg *argv)
{
	struct window *pwin = argv[1].p;
	struct tileset *ts = argv[2].p;
	struct animation *ani;
	int flags = 0;

	if (ins_alpha)		flags |= ANIMATION_SRCALPHA;
	if (ins_colorkey)	flags |= ANIMATION_SRCCOLORKEY;

	if (ins_anim_name[0] == '\0') {
		u_int nameno = 0;
tryname1:
		snprintf(ins_anim_name, sizeof(ins_anim_name),
		    _("Animation #%d"), nameno);
		TAILQ_FOREACH(ani, &ts->animations, animations) {
			if (strcmp(ani->name, ins_anim_name) == 0)
				break;
		}
		if (ani != NULL) {
			nameno++;
			goto tryname1;
		}
	} else {
tryname2:
		TAILQ_FOREACH(ani, &ts->animations, animations) {
			if (strcmp(ani->name, ins_anim_name) == 0)
				break;
		}
		if (ani != NULL) {
			char *np;
			int num = -1;

			for (np = &ins_anim_name[strlen(ins_anim_name)-1];
			     np > &ins_anim_name[0];
			     np--) {
				if (*np == '#' && *(np+1) != '\0') {
					np++;
					num = atoi(np) + 1;
					snprintf(np, sizeof(ins_anim_name) -
					    (np-ins_anim_name)-1, "%d", num);
					break;
				}
				if (!isdigit(*np)) {
					strlcat(ins_anim_name, "_",
					    sizeof(ins_anim_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	ani = Malloc(sizeof(struct animation), M_RG);
	animation_init(ani, ts, ins_anim_name, flags);
	animation_scale(ani, ins_tile_w, ins_tile_h);
	TAILQ_INSERT_TAIL(&ts->animations, ani, animations);

	ins_anim_name[0] = '\0';
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
	window_set_caption(win, _("Create new tile"));
	
	tb = textbox_new(win, _("Name:"));
	widget_bind(tb, "string", WIDGET_STRING, ins_tile_name,
	    sizeof(ins_tile_name));
	widget_focus(tb);

	msb = mspinbutton_new(win, "x", _("Size:"));
	widget_bind(msb, "xvalue", WIDGET_INT, &ins_tile_w);
	widget_bind(msb, "yvalue", WIDGET_INT, &ins_tile_h);
	mspinbutton_set_range(msb, TILE_SIZE_MIN, TILE_SIZE_MAX);

	cb = checkbox_new(win, _("Alpha blending"));
	widget_bind(cb, "state", WIDGET_INT, &ins_alpha);
	
	cb = checkbox_new(win, _("Colorkeying"));
	widget_bind(cb, "state", WIDGET_INT, &ins_colorkey);

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
insert_anim_dlg(int argc, union evarg *argv)
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
	window_set_caption(win, _("Create new animation"));
	
	tb = textbox_new(win, _("Name:"));
	widget_bind(tb, "string", WIDGET_STRING, ins_anim_name,
	    sizeof(ins_anim_name));
	widget_focus(tb);

	msb = mspinbutton_new(win, "x", _("Size:"));
	widget_bind(msb, "xvalue", WIDGET_INT, &ins_tile_w);
	widget_bind(msb, "yvalue", WIDGET_INT, &ins_tile_h);
	mspinbutton_set_range(msb, TILE_SIZE_MIN, TILE_SIZE_MAX);

	cb = checkbox_new(win, _("Alpha blending"));
	widget_bind(cb, "state", WIDGET_INT, &ins_alpha);
	
	cb = checkbox_new(win, _("Colorkey"));
	widget_bind(cb, "state", WIDGET_INT, &ins_colorkey);

	btnbox = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
	{
		btn = button_new(btnbox, "OK");
		event_new(btn, "button-pushed", insert_anim, "%p,%p", win, ts);
		
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
		if (t->nrefs > 0) {
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
		if (!it->selected ||
		    strcmp(it->class, "tile") != 0) {
			continue;
		}
		if ((win = tile_edit(ts, (struct tile *)it->p1)) != NULL) {
			window_attach(pwin, win);
			window_show(win);
		}
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
edit_anims(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct window *pwin = argv[3].p;
	struct window *win;
	struct tlist_item *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected ||
		    strcmp(it->class, "anim") != 0) {
			continue;
		}
		if ((win = animation_edit((struct animation *)it->p1))
		    != NULL) {
			window_attach(pwin, win);
			window_show(win);
		}
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
export_pixmap(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_art = argv[2].p;
	
	text_msg(MSG_ERROR, "Todo");
}

static void
export_anim(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_anims = argv[2].p;
	
	text_msg(MSG_ERROR, "Todo");
}

static void
export_sketch(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_art = argv[2].p;
	
	text_msg(MSG_ERROR, "Todo");
}

static void
delete_pixmap(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_art = argv[2].p;
	struct tlist_item *it;

	if ((it = tlist_item_selected(tl_art)) != NULL) {
		struct pixmap *px = it->p1;
	
		if (px->nrefs > 0) {
			text_msg(MSG_ERROR,
			    _("The pixmap \"%s\" is currently in use."),
			    px->name);
			return;
		}
		TAILQ_REMOVE(&ts->pixmaps, px, pixmaps);
		pixmap_destroy(px);
		Free(px, M_RG);
	}
}

static void
delete_anim(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_anims = argv[2].p;
	struct tlist_item *it;

	if ((it = tlist_item_selected(tl_anims)) != NULL) {
		struct animation *ani = it->p1;
	
		if (ani->nrefs > 0) {
			text_msg(MSG_ERROR,
			    _("The animation \"%s\" is currently in use."),
			    ani->name);
			return;
		}
		TAILQ_REMOVE(&ts->animations, ani, animations);
		animation_destroy(ani);
		Free(ani, M_RG);
	}
}

static void
delete_sketch(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_art = argv[2].p;
	struct tlist_item *it;

	if ((it = tlist_item_selected(tl_art)) != NULL) {
		struct sketch *sk = it->p1;
	
		if (sk->nrefs > 0) {
			text_msg(MSG_ERROR,
			    _("The sketch \"%s\" is currently in use."),
			    sk->name);
			return;
		}
		TAILQ_REMOVE(&ts->sketches, sk, sketches);
		sketch_destroy(sk);
		Free(sk, M_RG);
	}
}

static void
duplicate_pixmap(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tlist *tl_art = argv[2].p;
	struct tlist_item *it;

	if ((it = tlist_item_selected(tl_art)) != NULL) {
		struct pixmap *px1 = it->p1;
		struct pixmap *px2;

		px2 = Malloc(sizeof(struct pixmap), M_RG);
		pixmap_init(px2, ts, 0);
		strlcpy(px2->name, _("Copy of "), sizeof(px2->name));
		strlcat(px2->name, px1->name, sizeof(px2->name));
		pixmap_scale(px2, px1->su->w, px1->su->h, 0, 0);
		TAILQ_INSERT_TAIL(&ts->pixmaps, px2, pixmaps);

		SDL_LockSurface(px1->su);
		SDL_LockSurface(px2->su);
		memcpy(
		    (Uint8 *)px2->su->pixels,
		    (Uint8 *)px1->su->pixels,
		    px1->su->h*px1->su->pitch +
		    px1->su->w*px1->su->format->BytesPerPixel);
		SDL_UnlockSurface(px2->su);
		SDL_UnlockSurface(px1->su);
	}
}

struct window *
tileset_edit(void *p)
{
	struct tileset *ts = p;
	struct window *win;
	struct tlist *tl_tiles, *tl_art, *tl_anims;
	struct box *box, *hbox, *bbox;
	struct textbox *tb;
	struct mspinbutton *msb;
	struct AGMenu *m;
	struct AGMenuItem *mi;
	struct button *bu;
	struct notebook *nb;
	struct notebook_tab *ntab;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Tile set: %s"), OBJECT(ts)->name);
	window_set_position(win, WINDOW_LOWER_CENTER, 0);

	tl_tiles = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl_tiles, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
	tlist_prescale(tl_tiles, "XXXXXXXXXXXXXXXXXXXXXXXX (00x00)", 6);
	event_new(tl_tiles, "tlist-poll", poll_tiles, "%p", ts);
	
	tl_art = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl_art, TLIST_POLL);
	event_new(tl_art, "tlist-poll", poll_art, "%p", ts);
	
	tl_anims = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(tl_anims, TLIST_POLL);
	event_new(tl_anims, "tlist-poll", poll_anims, "%p", ts);

	mi = tlist_set_popup(tl_tiles, "tile");
	{
		menu_action(mi, _("Edit tile..."), OBJEDIT_ICON,
		    edit_tiles, "%p,%p,%p", ts, tl_tiles, win);
		menu_action(mi, _("Delete tile"), TRASH_ICON,
		    delete_tiles, "%p,%p", tl_tiles, ts);
	}

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	ntab = notebook_add_tab(nb, _("Tiles"), BOX_VERT);
	notebook_select_tab(nb, ntab);
	{
		object_attach(ntab, tl_tiles);

		bbox = box_new(ntab, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
		{
			bu = button_new(bbox, _("Insert"));
			event_new(bu, "button-pushed", insert_tile_dlg,
			    "%p,%p", ts, win);

			bu = button_new(bbox, _("Edit"));
			event_new(bu, "button-pushed", edit_tiles,
			    "%p,%p,%p", ts, tl_tiles, win);
			event_new(tl_tiles, "tlist-dblclick",
			    edit_tiles, "%p,%p,%p", ts, tl_tiles, win);

			bu = button_new(bbox, _("Delete"));
			event_new(bu, "button-pushed", delete_tiles,
			    "%p,%p", tl_tiles, ts);
		}
	}

	ntab = notebook_add_tab(nb, _("Tile elements"), BOX_VERT);
	{
		object_attach(ntab, tl_art);
	
		mi = tlist_set_popup(tl_art, "pixmap");
		{
			menu_action(mi, _("Delete pixmap"), TRASH_ICON,
			    delete_pixmap, "%p,%p", ts, tl_art);
			
			menu_action(mi, _("Duplicate pixmap"), OBJDUP_ICON,
			    duplicate_pixmap, "%p,%p", ts, tl_art);
			
			menu_action(mi, _("Export to image file..."),
			    OBJSAVE_ICON,
			    export_pixmap, "%p,%p", ts, tl_art);
		}

		mi = tlist_set_popup(tl_art, "sketch");
		{
			menu_action(mi, _("Delete sketch"), TRASH_ICON,
			    delete_sketch, "%p,%p", ts, tl_art);
			
			menu_action(mi, _("Export to vector file..."),
			    OBJSAVE_ICON,
			    export_sketch, "%p,%p", ts, tl_art);
		}
	}
	
	ntab = notebook_add_tab(nb, _("Animations"), BOX_VERT);
	{
		object_attach(ntab, tl_anims);
		
		mi = tlist_set_popup(tl_anims, "anim");
		{
			menu_action(mi, _("Edit animation"), EDIT_ICON,
			    edit_anims, "%p,%p,%p", ts, tl_anims, win);
			menu_action(mi, _("Delete animation"), TRASH_ICON,
			    delete_anim, "%p,%p", ts, tl_anims);
			menu_action(mi, _("Export to animation file..."),
			    OBJSAVE_ICON,
			    export_anim, "%p,%p", ts, tl_anims);
		}
		
		bbox = box_new(ntab, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
		{
			bu = button_new(bbox, _("Insert"));
			event_new(bu, "button-pushed", insert_anim_dlg, "%p,%p",
			    ts, win);

			bu = button_new(bbox, _("Edit"));
			event_new(bu, "button-pushed", edit_anims,
			    "%p,%p,%p", ts, tl_anims, win);
			event_new(tl_anims, "tlist-dblclick", edit_anims,
			    "%p,%p,%p", ts, tl_anims, win);

			bu = button_new(bbox, _("Delete"));
			event_new(bu, "button-pushed", delete_anim, "%p,%p",
			    ts, tl_anims);
		}
	}
	return (win);
}

#endif /* EDITION */

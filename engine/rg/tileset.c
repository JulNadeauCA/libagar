/*	$Csoft: tileset.c,v 1.63 2005/09/27 00:25:20 vedge Exp $	*/

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

#include <engine/loader/tmp.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/menu.h>
#include <engine/widget/notebook.h>
#include <engine/widget/radio.h>
#include <engine/widget/combo.h>

#include "tileset.h"

#include <ctype.h>

const AG_Version rgTilesetVer = {
	"agar tileset",
	7, 0
};

const AG_ObjectOps rgTilesetOps = {
	RG_TilesetInit,
	RG_TilesetReinit,
	RG_TilesetDestroy,
	RG_TilesetLoad,
	RG_TilesetSave,
#ifdef EDITION
	RG_TilesetEdit
#else
	NULL
#endif
};

extern const RG_FeatureOps rgFillOps;
extern const RG_FeatureOps rgSketchProjOps;

const RG_FeatureOps *feature_tbl[] = {
	&rgFillOps,
	&rgSketchProjOps,
	NULL
};

void
RG_TilesetInit(void *obj, const char *name)
{
	RG_Tileset *ts = obj;

	AG_ObjectInit(ts, "tileset", name, &rgTilesetOps);
	AGOBJECT(ts)->gfx = AG_GfxNew(ts);
	AGOBJECT(ts)->gfx->used = 0;
	AGOBJECT(ts)->flags |= AG_OBJECT_REOPEN_ONLOAD;

	pthread_mutex_init(&ts->lock, &agRecursiveMutexAttr);
	TAILQ_INIT(&ts->tiles);
	TAILQ_INIT(&ts->sketches);
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->animations);
	TAILQ_INIT(&ts->textures);

	ts->icon = SDL_CreateRGBSurface(
	    SDL_SWSURFACE|SDL_SRCALPHA|SDL_SRCCOLORKEY,
	    32, 32, agSurfaceFmt->BitsPerPixel,
	    agSurfaceFmt->Rmask,
	    agSurfaceFmt->Gmask,
	    agSurfaceFmt->Bmask,
	    agSurfaceFmt->Amask);

	if (ts->icon == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	ts->fmt = ts->icon->format;
	ts->flags = 0;
	ts->max_sprites = 0;
	ts->template[0] = '\0';
}

void
RG_TilesetReinit(void *obj)
{
	RG_Tileset *ts = obj;
	RG_Tile *t, *nt;
	RG_Sketch *sk, *nsk;
	RG_Pixmap *px, *npx;
	RG_Feature *ft, *nft;
	RG_Anim *ani, *nani;
	RG_Texture *tex, *ntex;

	pthread_mutex_lock(&ts->lock);
	
	for (t = TAILQ_FIRST(&ts->tiles);
	     t != TAILQ_END(&ts->tiles);
	     t = nt) {
		nt = TAILQ_NEXT(t, tiles);
		RG_TileDestroy(t);
		Free(t, M_RG);
	}

#if 0
	ts->max_sprites = 0;
	AG_GfxAllocSprites(AGOBJECT(ts)->gfx, 0);
	AG_GfxAllocAnims(AGOBJECT(ts)->gfx, 0);
#endif

	for (sk = TAILQ_FIRST(&ts->sketches);
	     sk != TAILQ_END(&ts->sketches);
	     sk = nsk) {
		nsk = TAILQ_NEXT(sk, sketches);
		RG_SketchDestroy(sk);
		Free(sk, M_RG);
	}
	for (px = TAILQ_FIRST(&ts->pixmaps);
	     px != TAILQ_END(&ts->pixmaps);
	     px = npx) {
		npx = TAILQ_NEXT(px, pixmaps);
		RG_PixmapDestroy(px);
		Free(px, M_RG);
	}
	for (ft = TAILQ_FIRST(&ts->features);
	     ft != TAILQ_END(&ts->features);
	     ft = nft) {
		nft = TAILQ_NEXT(ft, features);
		AG_FeatureDestroy(ft);
		Free(ft, M_RG);
	}
	for (ani = TAILQ_FIRST(&ts->animations);
	     ani != TAILQ_END(&ts->animations);
	     ani = nani) {
		nani = TAILQ_NEXT(ani, animations);
		RG_AnimDestroy(ani);
		Free(ani, M_RG);
	}
	for (tex = TAILQ_FIRST(&ts->textures);
	     tex != TAILQ_END(&ts->textures);
	     tex = ntex) {
		ntex = TAILQ_NEXT(tex, textures);
		RG_TextureDestroy(tex);
		Free(tex, M_RG);
	}
	
	TAILQ_INIT(&ts->tiles);
	TAILQ_INIT(&ts->sketches);
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->animations);
	TAILQ_INIT(&ts->textures);
	pthread_mutex_unlock(&ts->lock);
}

void
RG_TilesetDestroy(void *obj)
{
	RG_Tileset *ts = obj;

	pthread_mutex_destroy(&ts->lock);
	SDL_FreeSurface(ts->icon);
}

int
RG_TilesetLoad(void *obj, AG_Netbuf *buf)
{
	RG_Tileset *ts = obj;
	AG_Gfx *gfx = AGOBJECT(ts)->gfx;
	RG_Pixmap *px;
	Uint32 nsketches, npixmaps, nfeatures, ntiles, nanimations, ntextures;
	Uint32 i;

	if (AG_ReadVersion(buf, &rgTilesetVer, NULL) != 0)
		return (-1);
	
	pthread_mutex_lock(&ts->lock);
	AG_CopyString(ts->template, buf, sizeof(ts->template));
	ts->flags = AG_ReadUint32(buf);
	ts->max_sprites = AG_ReadUint32(buf);

	/* Resize the graphics array. */
	AG_GfxAllocSprites(gfx, ts->max_sprites);

	/* Load the vectorial sketches. */
	nsketches = AG_ReadUint32(buf);
	for (i = 0; i < nsketches; i++) {
		RG_Sketch *sk;

		sk = Malloc(sizeof(RG_Sketch), M_RG);
		RG_SketchInit(sk, ts, 0);
		if (RG_SketchLoad(sk, buf) == -1) {
			Free(sk, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->sketches, sk, sketches);
	}

	/* Load the pixmaps. */
	npixmaps = AG_ReadUint32(buf);
	for (i = 0; i < npixmaps; i++) {
		RG_Pixmap *px;

		px = Malloc(sizeof(RG_Pixmap), M_RG);
		RG_PixmapInit(px, ts, 0);
		if (RG_PixmapLoad(px, buf) == -1) {
			Free(px, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->pixmaps, px, pixmaps);
	}

	/* Load the features. */
	nfeatures = AG_ReadUint32(buf);
	for (i = 0; i < nfeatures; i++) {
		const RG_FeatureOps **ftops;
		char name[RG_FEATURE_NAME_MAX];
		char type[RG_FEATURE_TYPE_MAX];
		size_t len;
		RG_Feature *ft;
		int flags;
		
		AG_CopyString(name, buf, sizeof(name));
		AG_CopyString(type, buf, sizeof(type));
		flags = (int)AG_ReadUint32(buf);
		len = (size_t)AG_ReadUint32(buf);

		for (ftops = &feature_tbl[0]; *ftops != NULL; ftops++) {
			if (strcmp((*ftops)->type, type) == 0)
				break;
		}
		if (*ftops == NULL) {
			dprintf("%s: unimplemented feature: %s; "
			        "skipping %lu bytes.\n", name, type,
				(u_long)len);
			AG_NetbufSeek(buf, len-4, SEEK_CUR);
			continue;
		}

		ft = Malloc((*ftops)->len, M_RG);
		ft->ops = *ftops;
		ft->ops->init(ft, ts, flags);
		if (RG_FeatureLoad(ft, buf) == -1) {
			AG_FeatureDestroy(ft);
			Free(ft, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->features, ft, features);
	}
	
	/* Load the tiles. */
	ntiles = AG_ReadUint32(buf);
	dprintf("%u tiles\n", ntiles);
	for (i = 0; i < ntiles; i++) {
		AG_Map *cMap;
		char name[RG_TILE_NAME_MAX];
		RG_Tile *t;
		
		t = Malloc(sizeof(RG_Tile), M_RG);
		AG_CopyString(name, buf, sizeof(name));
		RG_TileInit(t, ts, name);

		if (RG_TileLoad(t, buf) == -1) {
			RG_TileDestroy(t);
			Free(t, M_RG);
			goto fail;
		}

		/* Allocate the surface fragments. */
		RG_TileScale(ts, t, t->su->w, t->su->h, t->flags,
		    t->su->format->alpha);
		RG_TileGenerate(t);
		
		TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	}

	/* Load the animation information. */
	nanimations = AG_ReadUint32(buf);
	for (i = 0; i < nanimations; i++) {
		char name[RG_ANIMATION_NAME_MAX];
		RG_Anim *ani;
		int flags;
		
		ani = Malloc(sizeof(RG_Anim), M_RG);
		AG_CopyString(name, buf, sizeof(name));
		flags = (int)AG_ReadUint32(buf);
		RG_AnimInit(ani, ts, name, flags);
		if (RG_AnimLoad(ani, buf) == -1) {
			RG_AnimDestroy(ani);
			Free(ani, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->animations, ani, animations);
	}
	
	/* Load the textures. */
	ntextures = AG_ReadUint32(buf);
	for (i = 0; i < ntextures; i++) {
		char name[RG_TEXTURE_NAME_MAX];
		RG_Texture *tex;
		
		tex = Malloc(sizeof(RG_Texture), M_RG);
		AG_CopyString(name, buf, sizeof(name));
		RG_TextureInit(tex, ts, name);
		if (RG_TextureLoad(tex, buf) == -1) {
			RG_TextureDestroy(tex);
			Free(tex, M_RG);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->textures, tex, textures);
	}

	/* Resolve the pixmap brush references. */
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		struct rg_pixmap_brush *pbr;
		RG_Pixmap *ppx;

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
			if (ppx == NULL)
				fatal("%s: bad pixmap ref", pbr->px_name);
		}
	}
	pthread_mutex_unlock(&ts->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ts->lock);
	return (-1);
}

int
RG_TilesetSave(void *obj, AG_Netbuf *buf)
{
	RG_Tileset *ts = obj;
	Uint32 nsketches = 0, npixmaps = 0, ntiles = 0, nfeatures = 0,
	       nanims = 0, ntextures = 0;
	off_t nsketches_offs, npixmaps_offs, ntiles_offs, nfeatures_offs,
	      nanims_offs, ntextures_offs;
	RG_Sketch *sk;
	RG_Pixmap *px;
	RG_Anim *ani;
	RG_Tile *t;
	RG_Feature *ft;
	RG_Texture *tex;

	AG_WriteVersion(buf, &rgTilesetVer);

	pthread_mutex_lock(&ts->lock);

	AG_WriteString(buf, ts->template);
	AG_WriteUint32(buf, ts->flags);
	AG_WriteUint32(buf, ts->max_sprites);

	/* Save the vectorial sketches. */
	nsketches_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		RG_SketchSave(sk, buf);
		nsketches++;
	}
	AG_PwriteUint32(buf, nsketches, nsketches_offs);

	/* Save the pixmaps. */
	npixmaps_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		RG_PixmapSave(px, buf);
		npixmaps++;
	}
	AG_PwriteUint32(buf, npixmaps, npixmaps_offs);

	/* Save the features. */
	nfeatures_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(ft, &ts->features, features) {
		off_t ftsize_offs;

		AG_WriteString(buf, ft->name);
		AG_WriteString(buf, ft->ops->type);
		AG_WriteUint32(buf, ft->flags);

		/* Encode the size to allow skipping unimplemented features. */
		ftsize_offs = AG_NetbufTell(buf);
		AG_WriteUint32(buf, 0);
		RG_FeatureSave(ft, buf);
		AG_PwriteUint32(buf, AG_NetbufTell(buf) - ftsize_offs,
		    ftsize_offs);

		nfeatures++;
	}
	AG_PwriteUint32(buf, nfeatures, nfeatures_offs);
	
	/* Save the tiles. */
	ntiles_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		RG_TileSave(t, buf);
		ntiles++;
	}
	AG_PwriteUint32(buf, ntiles, ntiles_offs);
	dprintf("saved %u tiles\n", ntiles);
	
	/* Save the animation information. */
	nanims_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(ani, &ts->animations, animations) {
		AG_WriteString(buf, ani->name);
		AG_WriteUint32(buf, (Uint32)ani->flags);
		RG_AnimSave(ani, buf);
		nanims++;
	}
	AG_PwriteUint32(buf, nanims, nanims_offs);
	
	/* Save the textures . */
	ntextures_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(tex, &ts->textures, textures) {
		AG_WriteString(buf, tex->name);
		RG_TextureSave(tex, buf);
		ntextures++;
	}
	AG_PwriteUint32(buf, ntextures, ntextures_offs);
	
	pthread_mutex_unlock(&ts->lock);
	return (0);
}

RG_Tile *
RG_TilesetFindTile(RG_Tileset *ts, const char *name)
{
	RG_Tile *t;

	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		if (strcmp(t->name, name) == 0)
			break;
	}
	if (t == NULL) {
		AG_SetError("%s: unexisting tile", name);
	}
	return (t);
}

RG_Sketch *
RG_TilesetFindSketch(RG_Tileset *ts, const char *name)
{
	RG_Sketch *sk;

	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		if (strcmp(sk->name, name) == 0)
			break;
	}
	if (sk == NULL) {
		AG_SetError("%s: unexisting sketch", name);
	}
	return (sk);
}

RG_Pixmap *
RG_TilesetFindPixmap(RG_Tileset *ts, const char *name)
{
	RG_Pixmap *px;

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		if (strcmp(px->name, name) == 0)
			break;
	}
	if (px == NULL) {
		AG_SetError("%s: unexisting pixmap", name);
	}
	return (px);
}

RG_Pixmap *
RG_TilesetResvPixmap(const char *tsname, const char *pxname)
{
	RG_Pixmap *px;
	RG_Tileset *ts;

	if ((ts = AG_ObjectFind(tsname)) == NULL) {
		AG_SetError("%s: no such tileset", tsname);
		return (NULL);
	}
	if (!AGOBJECT_TYPE(ts, "tileset")) {
		AG_SetError("%s: not a tileset", tsname);
		return (NULL);
	}

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		if (strcmp(px->name, pxname) == 0)
			break;
	}
	if (px == NULL) {
		AG_SetError("%s has no `%s' pixmap", tsname, pxname);
	}
	return (px);
}

RG_Tile *
RG_TilesetResvTile(const char *tsname, const char *tname)
{
	RG_Tileset *ts;
	RG_Tile *t;

	if ((ts = AG_ObjectFind(tsname)) == NULL) {
		AG_SetError("%s: no such tileset", tsname);
		return (NULL);
	}
	if (!AGOBJECT_TYPE(ts, "tileset")) {
		AG_SetError("%s: not a tileset", tsname);
		return (NULL);
	}

	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		if (strcmp(t->name, tname) == 0)
			break;
	}
	if (t == NULL) {
		AG_SetError("%s has no `%s' tile", tsname, tname);
	}
	return (t);
}

RG_Anim *
RG_TilesetFindAnim(RG_Tileset *ts, const char *name)
{
	RG_Anim *ani;

	TAILQ_FOREACH(ani, &ts->animations, animations) {
		if (strcmp(ani->name, name) == 0)
			break;
	}
	if (ani == NULL) {
		AG_SetError("%s: unexisting animation", name);
	}
	return (ani);
}

#ifdef EDITION

static void
poll_graphics(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	RG_Pixmap *px;
	RG_Sketch *sk;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u) [#%u]",
		    px->name, px->su->w, px->su->h, px->nrefs);
		it->p1 = px;
		it->class = "pixmap";
		AG_TlistSetIcon(tl, it, px->su);
	}
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		it = AG_TlistAdd(tl, NULL,
		    "%s (%ux%u %.0f%%) [#%u]", sk->name, sk->vg->su->w,
		    sk->vg->su->h, sk->vg->scale*100.0, sk->nrefs);
		it->class = "sketch";
		it->p1 = sk;
		AG_TlistSetIcon(tl, it, sk->vg->su);
	}

	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
poll_textures(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	RG_Texture *tex;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(tex, &ts->textures, textures) {
		RG_Tile *t;

		if (tex->tileset[0] != '\0' && tex->tile[0] != '\0' &&
		    (t = RG_TilesetResvTile(tex->tileset, tex->tile))
		     != NULL) {
			it = AG_TlistAdd(tl, NULL, "%s (<%s> %ux%u)",
			    tex->name, t->name, t->su->w, t->su->h);
			AG_TlistSetIcon(tl, it, t->su);
		} else {
			it = AG_TlistAdd(tl, NULL, "%s (<%s:%s>?)",
			    tex->name, tex->tileset, tex->tile);
		}
		it->class = "texture";
		it->p1 = tex;
	}
	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
poll_anims(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	RG_Anim *ani;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(ani, &ts->animations, animations) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u) [#%u]", ani->name,
		    ani->w, ani->h, ani->nrefs);
		it->p1 = ani;
		it->class = "anim";
	}
	
	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
poll_tiles(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	RG_Tile *t;
	AG_TlistItem *it;
	RG_TileElement *tel;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u)", t->name,
		    t->su->w, t->su->h);
		it->depth = 0;
		it->class = "tile";
		it->p1 = t;
		AG_TlistSetIcon(tl, it, t->su);

		if (!TAILQ_EMPTY(&t->elements)) {
			it->flags |= AG_TLIST_HAS_CHILDREN;
			if (!AG_TlistVisibleChildren(tl, it))
				continue;
		}
	
		TAILQ_FOREACH(tel, &t->elements, elements) {
			switch (tel->type) {
			case RG_TILE_FEATURE:
				{
					RG_Feature *ft =
					    tel->tel_feature.ft;
					RG_FeatureSketch *fts;

					it = AG_TlistAdd(tl, AGICON(OBJ_ICON),
					    "%s [%d,%d] %s", ft->name,
					    tel->tel_feature.x,
					    tel->tel_feature.y,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->class = "tile-feature";
					it->p1 = tel;

					TAILQ_FOREACH(fts, &ft->sketches,
					    sketches) {
						it = AG_TlistAdd(tl,
						    AGICON(DRAWING_ICON),
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
			case RG_TILE_PIXMAP:
				{
					RG_Pixmap *px = tel->tel_pixmap.px;

					it = AG_TlistAdd(tl, NULL,
					    "%s (%ux%u)%s", px->name,
					    tel->tel_pixmap.px->su->w,
					    tel->tel_pixmap.px->su->h,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->class = "tile-pixmap";
					it->p1 = tel;
					AG_TlistSetIcon(tl, it, px->su);
				}
				break;
			case RG_TILE_SKETCH:
				{
					RG_Sketch *sk = tel->tel_sketch.sk;

					it = AG_TlistAdd(tl,
					    AGICON(DRAWING_ICON),
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
	AG_TlistRestore(tl);
	pthread_mutex_unlock(&ts->lock);
}

static char ins_tile_name[RG_TILE_NAME_MAX];
static char ins_tile_class[RG_TILE_CLASS_MAX];
static char ins_texture_name[RG_TEXTURE_NAME_MAX];
static char ins_anim_name[RG_TILE_NAME_MAX];
static int ins_tile_w = AGTILESZ;
static int ins_tile_h = AGTILESZ;
static int ins_alpha = 0;
static int ins_colorkey = 1;
static enum ag_gfx_snap_mode ins_snap_mode = AG_GFX_SNAP_NOT;

static void
insert_tile(int argc, union evarg *argv)
{
	AG_Window *pwin = argv[1].p;
	RG_Tileset *ts = argv[2].p;
	AG_Gfx *gfx = AGOBJECT(ts)->gfx;
	RG_Tile *t;
	u_int flags = 0;

	if (ins_alpha)		flags |= RG_TILE_SRCALPHA;
	if (ins_colorkey)	flags |= RG_TILE_SRCCOLORKEY;

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

	t = Malloc(sizeof(RG_Tile), M_RG);
	RG_TileInit(t, ts, ins_tile_name);
	if (strcmp(ts->template, "Perso") == 0) {
		RG_TileScale(ts, t, 16, 32, flags, SDL_ALPHA_OPAQUE);
		AG_SpriteSetOrigin(&gfx->sprites[t->s], 8, 31);
		AG_SPRITE(t->ts,t->s).snap_mode = AG_GFX_SNAP_TO_GRID;
		RG_TILE_LAYER2(t,0,0) = +2;
		RG_TILE_LAYER2(t,0,1) = +1;
		RG_TILE_ATTR2(t,0,1) = AG_NITEM_BLOCK;
	} else if (strcmp(ts->template, "Terrain") == 0) {
		RG_TileScale(ts, t, 64, 64, flags, SDL_ALPHA_OPAQUE);
		AG_SpriteSetOrigin(&gfx->sprites[t->s], 24, 24);
		AG_SPRITE(t->ts,t->s).snap_mode = AG_GFX_SNAP_TO_GRID;
		RG_TILE_ATTR2(t,0,0) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,1,0) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,2,0) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,3,0) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,0,1) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,0,2) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,0,3) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,3,1) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,3,2) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,3,3) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,1,3) = AG_NITEM_BLOCK;
		RG_TILE_ATTR2(t,2,3) = AG_NITEM_BLOCK;
	} else {
		RG_TileScale(ts, t, ins_tile_w, ins_tile_h, flags,
		    SDL_ALPHA_OPAQUE);
		AG_SpriteSetOrigin(&gfx->sprites[t->s], t->su->w/2, t->su->h/2);
		AG_SPRITE(t->ts,t->s).snap_mode = ins_snap_mode;
	}
	TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);

	ins_tile_name[0] = '\0';
	AG_ViewDetach(pwin);
}

static void
insert_texture(int argc, union evarg *argv)
{
	AG_Window *dlgwin = argv[1].p;
	AG_Window *pwin = argv[2].p;
	RG_Tileset *ts = argv[3].p;
	AG_Window *win;
	RG_Texture *tex;
	u_int flags = 0;

	if (ins_texture_name[0] == '\0') {
		u_int nameno = 0;
tryname1:
		snprintf(ins_texture_name, sizeof(ins_texture_name),
		    _("Texture #%d"), nameno);
		TAILQ_FOREACH(tex, &ts->textures, textures) {
			if (strcmp(tex->name, ins_texture_name) == 0)
				break;
		}
		if (tex != NULL) {
			nameno++;
			goto tryname1;
		}
	} else {
tryname2:
		TAILQ_FOREACH(tex, &ts->textures, textures) {
			if (strcmp(tex->name, ins_texture_name) == 0)
				break;
		}
		if (tex != NULL) {
			char *np;
			int num = -1;

			for (np = &ins_texture_name[strlen(ins_texture_name)-1];
			     np > &ins_texture_name[0];
			     np--) {
				if (*np == '#' && *(np+1) != '\0') {
					np++;
					num = atoi(np) + 1;
					snprintf(np, sizeof(ins_texture_name) -
					    (np-ins_texture_name)-1, "%d", num);
					break;
				}
				if (!isdigit(*np)) {
					strlcat(ins_texture_name, "_",
					    sizeof(ins_texture_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	tex = Malloc(sizeof(RG_Texture), M_RG);
	RG_TextureInit(tex, ts, ins_texture_name);
	TAILQ_INSERT_TAIL(&ts->textures, tex, textures);
	
	ins_texture_name[0] = '\0';
	AG_ViewDetach(dlgwin);
	
	if ((win = RG_TextureEdit(tex)) != NULL) {
		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
insert_anim(int argc, union evarg *argv)
{
	AG_Window *pwin = argv[1].p;
	RG_Tileset *ts = argv[2].p;
	RG_Anim *ani;
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

	ani = Malloc(sizeof(RG_Anim), M_RG);
	RG_AnimInit(ani, ts, ins_anim_name, flags);
	RG_AnimScale(ani, ins_tile_w, ins_tile_h);
	TAILQ_INSERT_TAIL(&ts->animations, ani, animations);

	ins_anim_name[0] = '\0';
	AG_ViewDetach(pwin);
}

static void
insert_tile_dlg(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Window *pwin = argv[2].p;
	AG_Window *win;
	AG_Box *btnbox;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;
	AG_Checkbox *cb;
	AG_Radio *rad;
	AG_Combo *com;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_DETACH|AG_WINDOW_NO_RESIZE|
	                 AG_WINDOW_NO_MINIMIZE, NULL);
	AG_WindowSetCaption(win, _("Create new tile"));
	
	tb = AG_TextboxNew(win, _("Name: "));
	AG_WidgetBind(tb, "string", AG_WIDGET_STRING, ins_tile_name,
	    sizeof(ins_tile_name));
	AG_SetEvent(tb, "textbox-return", insert_tile, "%p,%p", win, ts);
	AG_WidgetFocus(tb);

	com = AG_ComboNew(win, AG_COMBO_ANY_TEXT, _("Class: "));
	AG_WidgetBind(com->tbox, "string", AG_WIDGET_STRING, ins_tile_class,
	    sizeof(ins_tile_class));
	if (strcmp(ts->template, "Terrain") == 0) {
		AG_TlistAdd(com->list, NULL, "Ground");
		AG_TlistAdd(com->list, NULL, "Rock");
	}

	msb = AG_MSpinbuttonNew(win, "x", _("Size: "));
	AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT, &ins_tile_w);
	AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT, &ins_tile_h);
	AG_MSpinbuttonSetRange(msb, RG_TILE_SIZE_MIN, RG_TILE_SIZE_MAX);

	cb = AG_CheckboxNew(win, _("Alpha blending"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &ins_alpha);
	
	cb = AG_CheckboxNew(win, _("Colorkeying"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &ins_colorkey);
	
	AG_LabelStatic(win, _("Snapping mode: "));
	rad = AG_RadioNew(win, agGfxSnapNames);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &ins_snap_mode);

	btnbox = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonAct(btnbox, _("OK"), 0, insert_tile, "%p,%p", win, ts);
		AG_ButtonAct(btnbox, _("Cancel"), 0, AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
insert_texture_dlg(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Window *pwin = argv[2].p;
	AG_Window *win;
	AG_Box *btnbox;
	AG_Textbox *tb;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_DETACH|AG_WINDOW_NO_RESIZE|
	                 AG_WINDOW_NO_MINIMIZE, NULL);
	AG_WindowSetCaption(win, _("Create a new texture"));
	
	tb = AG_TextboxNew(win, _("Name:"));
	AG_WidgetBind(tb, "string", AG_WIDGET_STRING, ins_texture_name,
	    sizeof(ins_texture_name));
	AG_SetEvent(tb, "textbox-return", insert_texture, "%p,%p,%p", win,
	    pwin, ts);
	AG_WidgetFocus(tb);

	btnbox = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonAct(btnbox, _("OK"), 0, insert_texture, "%p,%p,%p",
		    win, pwin, ts);
		AG_ButtonAct(btnbox, _("Cancel"), 0, AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
insert_anim_dlg(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Window *pwin = argv[2].p;
	AG_Window *win;
	AG_Box *btnbox;
	AG_Button *btn;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;
	AG_Checkbox *cb;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_DETACH|AG_WINDOW_NO_RESIZE|
	                 AG_WINDOW_NO_MINIMIZE, NULL);
	AG_WindowSetCaption(win, _("Create new animation"));
	
	tb = AG_TextboxNew(win, _("Name:"));
	AG_WidgetBind(tb, "string", AG_WIDGET_STRING, ins_anim_name,
	    sizeof(ins_anim_name));
	AG_SetEvent(tb, "textbox-return", insert_anim, "%p,%p", win, ts);
	AG_WidgetFocus(tb);

	msb = AG_MSpinbuttonNew(win, "x", _("Size:"));
	AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT, &ins_tile_w);
	AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT, &ins_tile_h);
	AG_MSpinbuttonSetRange(msb, RG_TILE_SIZE_MIN, RG_TILE_SIZE_MAX);

	cb = AG_CheckboxNew(win, _("Alpha blending"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &ins_alpha);
	
	cb = AG_CheckboxNew(win, _("Colorkey"));
	AG_WidgetBind(cb, "state", AG_WIDGET_INT, &ins_colorkey);

	btnbox = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonAct(btnbox, _("OK"), 0, insert_anim, "%p,%p", win, ts);
		AG_ButtonAct(btnbox, _("Cancel"), 0, AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
delete_tiles(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	RG_Tileset *ts = argv[2].p;
	AG_TlistItem *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		RG_Tile *t = it->p1;

		if (!it->selected) {
			continue;
		}
		if (t->nrefs > 0) {
			AG_TextMsg(AG_MSG_ERROR, _("The tile `%s' is in use."),
			    t->name);
			continue;
		}
		TAILQ_REMOVE(&ts->tiles, t, tiles);
		Free(t, M_RG);
	}
	pthread_mutex_unlock(&ts->lock);
}

int
RG_TilesetInsertSprite(RG_Tileset *ts, SDL_Surface *su)
{
	int s;

	s = AG_GfxAddSprite(AGOBJECT(ts)->gfx, su);
	if (s >= ts->max_sprites) {
		ts->max_sprites = s+1;
	}
	return (s);
}

static void
dup_tile(RG_Tileset *ts, RG_Tile *t1)
{
	char name[RG_TILE_NAME_MAX];
	RG_Tile *t2;
	RG_TileElement *e1, *e2;
	int x, y;
	int ncopy = 0;

	t2 = Malloc(sizeof(RG_Tile), M_RG);
tryname1:
	snprintf(name, sizeof(name), _("Copy #%d of %s"), ncopy++, t1->name);
	if (RG_TilesetFindTile(ts, name) != NULL) {
		goto tryname1;
	}
	RG_TileInit(t2, ts, name);
	RG_TileScale(ts, t2, t1->su->w, t1->su->h, t1->flags,
	    t1->su->format->alpha);
	strlcpy(t2->clname, t1->clname, sizeof(t2->clname));
	for (y = 0; y < t1->nh; y++) {
		for (x = 0; x < t1->nw; x++) {
			t2->attrs[y*t1->nw+x] = t1->attrs[y*t1->nw+x];
			t2->layers[y*t1->nw+x] = t1->layers[y*t1->nw+x];
		}
	}
	AG_SPRITE(ts,t2->s).xOrig = AG_SPRITE(ts,t1->s).xOrig;
	AG_SPRITE(ts,t2->s).yOrig = AG_SPRITE(ts,t1->s).yOrig;
	AG_SPRITE(ts,t2->s).snap_mode = AG_SPRITE(ts,t1->s).snap_mode;
	
	TAILQ_FOREACH(e1, &t1->elements, elements) {
		RG_TileElement *e2;
		RG_Pixmap *px1, *px2;

		ncopy = 0;
		switch (e1->type) {
		case RG_TILE_PIXMAP:
			px1 = e1->tel_pixmap.px;
			px2 = Malloc(sizeof(RG_Pixmap), M_RG);
			RG_PixmapInit(px2, ts, 0);
tryname2:
			snprintf(px2->name, sizeof(px2->name),
			    _("Copy #%d of %s"), ncopy++, px1->name);
			if (RG_TilesetFindPixmap(ts, px2->name) != NULL) {
				goto tryname2;
			}
			RG_PixmapScale(px2, px1->su->w, px1->su->h, 0, 0);
			SDL_LockSurface(px1->su);
			SDL_LockSurface(px2->su);
			memcpy((Uint8 *)px2->su->pixels,
			    (Uint8 *)px1->su->pixels,
			    px1->su->h*px1->su->pitch +
			    px1->su->w*px1->su->format->BytesPerPixel);
			SDL_UnlockSurface(px2->su);
			SDL_UnlockSurface(px1->su);

			e2 = RG_TileAddPixmap(t2, e1->name, px2,
			    e1->tel_pixmap.x, e1->tel_pixmap.y);
			e2->visible = e1->visible;
			TAILQ_INSERT_TAIL(&ts->pixmaps, px2, pixmaps);
			break;
		default:
			break;
		}
	}

	RG_TileGenerate(t2);
	TAILQ_INSERT_TAIL(&ts->tiles, t2, tiles);
}

static void
dup_tiles(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl = argv[2].p;
	AG_TlistItem *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		RG_Tile *t = it->p1;

		if (!it->selected) {
			continue;
		}
		dup_tile(ts, t);
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
edit_tiles(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl = argv[2].p;
	AG_Window *pwin = argv[3].p;
	AG_Window *win;
	AG_TlistItem *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected ||
		    strcmp(it->class, "tile") != 0) {
			continue;
		}
		if ((win = RG_TileEdit(ts, (RG_Tile *)it->p1)) != NULL) {
			AG_WindowAttach(pwin, win);
			AG_WindowShow(win);
		}
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
edit_anims(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl = argv[2].p;
	AG_Window *pwin = argv[3].p;
	AG_Window *win;
	AG_TlistItem *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected ||
		    strcmp(it->class, "anim") != 0) {
			continue;
		}
		if ((win = RG_AnimEdit((RG_Anim *)it->p1))
		    != NULL) {
			AG_WindowAttach(pwin, win);
			AG_WindowShow(win);
		}
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
edit_textures(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl = argv[2].p;
	AG_Window *pwin = argv[3].p;
	AG_Window *win;
	AG_TlistItem *it;

	pthread_mutex_lock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected) {
			continue;
		}
		if ((win = RG_TextureEdit((RG_Texture *)it->p1)) != NULL) {
			AG_WindowAttach(pwin, win);
			AG_WindowShow(win);
		}
	}
	pthread_mutex_unlock(&ts->lock);
}

static void
delete_pixmap(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl_art = argv[2].p;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl_art)) != NULL) {
		RG_Pixmap *px = it->p1;
	
		if (px->nrefs > 0) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The pixmap \"%s\" is currently in use."),
			    px->name);
			return;
		}
		TAILQ_REMOVE(&ts->pixmaps, px, pixmaps);
		RG_PixmapDestroy(px);
		Free(px, M_RG);
	}
}

static void
delete_anim(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl_anims = argv[2].p;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl_anims)) != NULL) {
		RG_Anim *ani = it->p1;
	
		if (ani->nrefs > 0) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The animation \"%s\" is currently in use."),
			    ani->name);
			return;
		}
		TAILQ_REMOVE(&ts->animations, ani, animations);
		RG_AnimDestroy(ani);
		Free(ani, M_RG);
	}
}

static void
delete_sketch(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl_art = argv[2].p;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl_art)) != NULL) {
		RG_Sketch *sk = it->p1;
	
		if (sk->nrefs > 0) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The sketch \"%s\" is currently in use."),
			    sk->name);
			return;
		}
		TAILQ_REMOVE(&ts->sketches, sk, sketches);
		RG_SketchDestroy(sk);
		Free(sk, M_RG);
	}
}

static void
delete_textures(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl_textures = argv[2].p;
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl_textures->items, items) {
		RG_Texture *tex = it->p1;

		if (!it->selected)
			continue;

		TAILQ_REMOVE(&ts->textures, tex, textures);
		RG_TextureDestroy(tex);
		Free(tex, M_RG);
	}
}

static void
duplicate_pixmap(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;
	AG_Tlist *tl_art = argv[2].p;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl_art)) != NULL) {
		RG_Pixmap *px1 = it->p1;
		RG_Pixmap *px2;
		int ncopy = 0;

		px2 = Malloc(sizeof(RG_Pixmap), M_RG);
		RG_PixmapInit(px2, ts, 0);
tryname:
		snprintf(px2->name, sizeof(px2->name), _("Copy #%d of %s"),
		    ncopy++, px1->name);
		if (RG_TilesetFindPixmap(ts, px2->name) != NULL)
			goto tryname;

		RG_PixmapScale(px2, px1->su->w, px1->su->h, 0, 0);
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

static void
select_template(int argc, union evarg *argv)
{
	RG_Tileset *ts = argv[1].p;

	RG_TilesetReinit(ts);

	if (strcmp(ts->template, "Perso") == 0) {
		const char *tiles[] = {
			"Idle-N",
			"Idle-S",
			"Idle-W",
			"Idle-E"
		};
		const char *anims[] = {
			"Walk-N",
			"Walk-S",
			"Walk-W",
			"Walk-E"
		};
		const int ntiles = sizeof(tiles)/sizeof(tiles[0]);
		const int nanims = sizeof(anims)/sizeof(anims[0]);
		RG_Tile *t;
		RG_Anim *a;
		int i;

		for (i = 0; i < ntiles; i++) {
			t = Malloc(sizeof(RG_Tile), M_RG);
			RG_TileInit(t, ts, tiles[i]);
			RG_TileScale(ts, t, 16, 32, RG_TILE_SRCCOLORKEY,
			    SDL_ALPHA_OPAQUE);
			TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
		}
		for (i = 0; i < nanims; i++) {
			a = Malloc(sizeof(RG_Anim), M_RG);
			RG_AnimInit(a, ts, anims[i], ANIMATION_SRCCOLORKEY);
			RG_AnimScale(a, 16, 32);
			TAILQ_INSERT_TAIL(&ts->animations, a, animations);
		}
	}
}

void *
RG_TilesetEdit(void *p)
{
	RG_Tileset *ts = p;
	AG_Window *win;
	AG_Tlist *tl_tiles, *tl_art, *tl_anims, *tl_textures;
	AG_Box *box, *hbox, *bbox;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;
	AG_Menu *m;
	AG_MenuItem *mi;
	AG_Button *bu;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Combo *com;

	win = AG_WindowNew(AG_WINDOW_DETACH, NULL);
	AG_WindowSetCaption(win, _("Tile set: %s"), AGOBJECT(ts)->name);
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_CENTER, 1);

	tl_tiles = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl_tiles, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE);
	AG_TlistPrescale(tl_tiles, "XXXXXXXXXXXXXXXXXXXXXXXX (00x00)", 6);
	AG_SetEvent(tl_tiles, "tlist-poll", poll_tiles, "%p", ts);
	
	tl_art = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl_art, AG_TLIST_POLL);
	AG_SetEvent(tl_art, "tlist-poll", poll_graphics, "%p", ts);
	
	tl_textures = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl_textures, AG_TLIST_POLL);
	AG_SetEvent(tl_textures, "tlist-poll", poll_textures, "%p", ts);

	tl_anims = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tl_anims, AG_TLIST_POLL);
	AG_SetEvent(tl_anims, "tlist-poll", poll_anims, "%p", ts);

	com = AG_ComboNew(win, 0, _("Template: "));
	AG_WidgetBind(com->tbox, "string", AG_WIDGET_STRING, &ts->template,
	    sizeof(ts->template));
	AG_TlistAdd(com->list, NULL, "Perso");
	AG_TlistAdd(com->list, NULL, "Terrain");
	AG_SetEvent(com, "combo-selected", select_template, "%p", ts);

	mi = AG_TlistSetPopup(tl_tiles, "tile");
	{
		AG_MenuAction(mi, _("Edit tiles..."), OBJEDIT_ICON,
		    edit_tiles, "%p,%p,%p", ts, tl_tiles, win);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Duplicate tiles"),
		    OBJDUP_ICON,
		    dup_tiles, "%p,%p", ts, tl_tiles);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Delete tiles"), TRASH_ICON,
		    delete_tiles, "%p,%p", tl_tiles, ts);
	}

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAddTab(nb, _("Tiles"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tl_tiles);
		AG_SetEvent(tl_tiles, "tlist-dblclick", edit_tiles,
		    "%p,%p,%p", ts, tl_tiles, win);

		bbox = AG_BoxNew(ntab, AG_BOX_HORIZ,
		    AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
		{
			AG_ButtonAct(bbox, _("Insert"), 0, insert_tile_dlg,
			    "%p,%p", ts, win);
			AG_ButtonAct(bbox, _("Edit"), 0, edit_tiles,
			    "%p,%p,%p", ts, tl_tiles, win);
			AG_ButtonAct(bbox, _("Delete"), 0, delete_tiles,
			    "%p,%p", tl_tiles, ts);
		}
	}

	ntab = AG_NotebookAddTab(nb, _("Graphics"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tl_art);
	
		mi = AG_TlistSetPopup(tl_art, "pixmap");
		{
			AG_MenuAction(mi, _("Delete pixmap"), TRASH_ICON,
			    delete_pixmap, "%p,%p", ts, tl_art);
			
			AG_MenuAction(mi, _("Duplicate pixmap"), OBJDUP_ICON,
			    duplicate_pixmap, "%p,%p", ts, tl_art);
		}

		mi = AG_TlistSetPopup(tl_art, "sketch");
		{
			AG_MenuAction(mi, _("Delete sketch"), TRASH_ICON,
			    delete_sketch, "%p,%p", ts, tl_art);
		}
	}
	
	ntab = AG_NotebookAddTab(nb, _("Textures"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tl_textures);
	
		mi = AG_TlistSetPopup(tl_textures, "texture");
		{
			AG_MenuAction(mi, _("Delete texture"), TRASH_ICON,
			    delete_textures, "%p,%p", ts, tl_textures);
		}
		
		bbox = AG_BoxNew(ntab, AG_BOX_HORIZ, 
		    AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
		{
			AG_ButtonAct(bbox, _("Insert"), 0,
			    insert_texture_dlg, "%p,%p", ts, win);
			AG_ButtonAct(bbox, _("Edit"), 0,
			    edit_textures, "%p,%p,%p", ts, tl_textures, win);
			AG_ButtonAct(bbox, _("Delete"), 0,
			    delete_textures, "%p,%p", ts, tl_textures);
		}
		
		AG_SetEvent(tl_textures, "tlist-dblclick", edit_textures,
		    "%p,%p,%p", ts, tl_textures, win);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Animations"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tl_anims);
		AG_SetEvent(tl_anims, "tlist-dblclick",
		    edit_anims, "%p,%p,%p", ts, tl_anims, win);
		
		mi = AG_TlistSetPopup(tl_anims, "anim");
		{
			AG_MenuAction(mi, _("Edit animation"), EDIT_ICON,
			    edit_anims, "%p,%p,%p", ts, tl_anims, win);
			AG_MenuAction(mi, _("Delete animation"), TRASH_ICON,
			    delete_anim, "%p,%p", ts, tl_anims);
		}
		
		bbox = AG_BoxNew(ntab, AG_BOX_HORIZ,
		    AG_BOX_WFILL|AG_BOX_HOMOGENOUS);
		{
			AG_ButtonAct(bbox, _("Insert"), 0,
			    insert_anim_dlg, "%p,%p", ts, win);
			AG_ButtonAct(bbox, _("Edit"), 0, 
			    edit_anims, "%p,%p,%p", ts, tl_anims, win);
			AG_ButtonAct(bbox, _("Delete"), 0,
			    delete_anim, "%p,%p", ts, tl_anims);
		}
	}
	return (win);
}

#endif /* EDITION */

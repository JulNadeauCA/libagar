/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Package of feature-based tile elements.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/text.h>
#include <agar/gui/icons.h>

#include <agar/gui/checkbox.h>
#include <agar/gui/combo.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/notebook.h>
#include <agar/gui/pane.h>
#include <agar/gui/radio.h>
#include <agar/gui/textbox.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>
#include <agar/map/rg_texsel.h>
#include <agar/map/rg_icons.h>
#include <agar/map/rg_icons_data.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

extern const RG_FeatureOps rgFillOps;
/* extern const RG_FeatureOps rgSketchProjOps; */

const RG_FeatureOps *_Nullable feature_tbl[] = {
	&rgFillOps,
/*	&rgSketchProjOps, */
	NULL
};
extern const char *rgTileSnapModes[];

int rgInitedSubsystem = 0;

void
RG_InitSubsystem(void)
{
	if (rgInitedSubsystem++ > 0)
		return;

	AG_RegisterNamespace("RG", "RG_", "http://libagar.org/");
	AG_RegisterClass(&rgTileviewClass);
	AG_RegisterClass(&rgTextureSelectorClass);
	AG_RegisterClass(&rgTilesetClass);

	rgIcon_Init();
}

void
RG_DestroySubsystem(void)
{
	if (--rgInitedSubsystem > 0) {
		return;
	}
	AG_UnregisterClass(&rgTileviewClass);
	AG_UnregisterClass(&rgTextureSelectorClass);
	AG_UnregisterClass(&rgTilesetClass);
	AG_UnregisterNamespace("RG");
}

RG_Tileset *
RG_TilesetNew(void *parent, const char *name, Uint flags)
{
	RG_Tileset *ts;

	ts = Malloc(sizeof(RG_Tileset));
	AG_ObjectInit(ts, &rgTilesetClass);
	AG_ObjectSetNameS(ts, name);
	ts->flags |= flags;

	AG_ObjectAttach(parent, ts);
	return (ts);
}

static void
Init(void *_Nonnull obj)
{
	RG_Tileset *ts = obj;
	AG_Surface *Sicon;

	/* Restart the graphical editor on load. */
	OBJECT(ts)->flags |= AG_OBJECT_REOPEN_ONLOAD;

	AG_MutexInitRecursive(&ts->lock);
	TAILQ_INIT(&ts->tiles);
#if 0
	TAILQ_INIT(&ts->sketches);
#endif
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->textures);

	Sicon = AG_SurfaceRGBA(32,32, agSurfaceFmt->BitsPerPixel,
	    AG_SURFACE_ALPHA|AG_SURFACE_COLORKEY,
	    agSurfaceFmt->Rmask,
	    agSurfaceFmt->Gmask,
	    agSurfaceFmt->Bmask,
	    agSurfaceFmt->Amask);

	if (Sicon == NULL) {
		AG_FatalError(NULL);
	}
	ts->icon = Sicon;
	ts->fmt = &Sicon->format;
	ts->flags = 0;
	ts->tmpl[0] = '\0';
	ts->nTileTbl = 0;
	ts->tileTbl = NULL;
}

static void
Reset(void *_Nonnull obj)
{
	RG_Tileset *ts = obj;
	RG_Tile *t, *nt;
/*	RG_Sketch *sk, *nsk; */
	RG_Pixmap *px, *npx;
	RG_Feature *ft, *nft;
	RG_Texture *tex, *ntex;

	AG_MutexLock(&ts->lock);
	
	for (t = TAILQ_FIRST(&ts->tiles);
	     t != TAILQ_END(&ts->tiles);
	     t = nt) {
		nt = TAILQ_NEXT(t, tiles);
		RG_TileDestroy(t);
		Free(t);
	}
#if 0
	for (sk = TAILQ_FIRST(&ts->sketches);
	     sk != TAILQ_END(&ts->sketches);
	     sk = nsk) {
		nsk = TAILQ_NEXT(sk, sketches);
		RG_SketchDestroy(sk);
		Free(sk);
	}
#endif
	for (px = TAILQ_FIRST(&ts->pixmaps);
	     px != TAILQ_END(&ts->pixmaps);
	     px = npx) {
		npx = TAILQ_NEXT(px, pixmaps);
		RG_PixmapDestroy(px);
		Free(px);
	}
	for (ft = TAILQ_FIRST(&ts->features);
	     ft != TAILQ_END(&ts->features);
	     ft = nft) {
		nft = TAILQ_NEXT(ft, features);
		RG_FeatureDestroy(ft);
		Free(ft);
	}
	for (tex = TAILQ_FIRST(&ts->textures);
	     tex != TAILQ_END(&ts->textures);
	     tex = ntex) {
		ntex = TAILQ_NEXT(tex, textures);
		RG_TextureDestroy(tex);
		Free(tex);
	}
	
	TAILQ_INIT(&ts->tiles);
/*	TAILQ_INIT(&ts->sketches); */
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->textures);

	Free(ts->tileTbl);
	ts->tileTbl = NULL;
	ts->nTileTbl = 0;

	AG_MutexUnlock(&ts->lock);
}

static void
Destroy(void *_Nonnull obj)
{
	RG_Tileset *ts = obj;

	AG_MutexDestroy(&ts->lock);
	AG_SurfaceFree(ts->icon);
	Free(ts->tileTbl);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	RG_Tileset *ts = obj;
	RG_Pixmap *px;
	Uint count, i;

	AG_MutexLock(&ts->lock);
	AG_CopyString(ts->tmpl, buf, sizeof(ts->tmpl));
	ts->flags = AG_ReadUint32(buf);

	count = (Uint)AG_ReadUint32(buf);
	if (count != 0)
		AG_FatalError("Vector sketches not supported");
#if 0
	/* Load the vectorial sketches. */
	for (i = 0; i < count; i++) {
		RG_Sketch *sk;

		sk = Malloc(sizeof(RG_Sketch));
		RG_SketchInit(sk, ts, 0);
		if (RG_SketchLoad(sk, buf) == -1) {
			Free(sk);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->sketches, sk, sketches);
	}
#endif

	/* Load the pixmaps. */
	count = AG_ReadUint32(buf);
	for (i = 0; i < count; i++) {
		RG_Pixmap *px;

		px = Malloc(sizeof(RG_Pixmap));
		RG_PixmapInit(px, ts, 0);
		if (RG_PixmapLoad(px, buf) == -1) {
			Free(px);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->pixmaps, px, pixmaps);
	}

	/* Load the features. */
	count = AG_ReadUint32(buf);
	Debug(ts, "Loading %u features\n", count);
	for (i = 0; i < count; i++) {
		const RG_FeatureOps **ftops;
		char name[RG_FEATURE_NAME_MAX];
		char type[RG_FEATURE_TYPE_MAX];
		Uint32 len, flags;
		RG_Feature *ft;
		
		AG_CopyString(name, buf, sizeof(name));
		AG_CopyString(type, buf, sizeof(type));
		flags = AG_ReadUint32(buf);
		len = AG_ReadUint32(buf);

		for (ftops = &feature_tbl[0]; *ftops != NULL; ftops++) {
			if (strcmp((*ftops)->type, type) == 0)
				break;
		}
		if (*ftops == NULL) {
			Debug(ts, "Unimplemented feature: %s (class=%s); "
			          "skipping %lu bytes.\n", name, type,
				  (Ulong)len);
			AG_Seek(buf, len-4, AG_SEEK_CUR);
			continue;
		}

		ft = Malloc((*ftops)->len);
		ft->ops = *ftops;
		ft->ops->init(ft, ts, (Uint)flags);
		if (RG_FeatureLoad(ft, buf) == -1) {
			RG_FeatureDestroy(ft);
			Free(ft);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->features, ft, features);
	}
	
	/* Load the tiles. */
	count = AG_ReadUint32(buf);
	Debug(ts, "Loading %u tiles\n", count);
	for (i = 0; i < count; i++) {
		char name[RG_TILE_NAME_MAX];
		RG_Tile *t;
		
		t = Malloc(sizeof(RG_Tile));
		AG_CopyString(name, buf, sizeof(name));
		RG_TileInit(t, ts, name);

		if (RG_TileLoad(t, buf) == -1) {
			RG_TileDestroy(t);
			Free(t);
			goto fail;
		}
		RG_TileScale(ts, t, t->su->w, t->su->h);
		RG_TileGenerate(t);
		TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	}

	/* Load the textures. */
	count = AG_ReadUint32(buf);
	Debug(ts, "Loading %u textures\n", count);
	for (i = 0; i < count; i++) {
		char name[RG_TEXTURE_NAME_MAX];
		RG_Texture *tex;
		
		tex = Malloc(sizeof(RG_Texture));
		AG_CopyString(name, buf, sizeof(name));
		RG_TextureInit(tex, name);
		if (RG_TextureLoad(tex, buf) == -1) {
			RG_TextureDestroy(tex);
			Free(tex);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->textures, tex, textures);
	}

	/* Load and resolve the static tile mappings. */
	ts->nTileTbl = AG_ReadUint32(buf);
	Debug(ts, "Tiletbl has %u entries\n", (Uint)ts->nTileTbl);
	ts->tileTbl = Realloc(ts->tileTbl, ts->nTileTbl*sizeof(RG_Tile *));
	for (i = 0; i < ts->nTileTbl; i++) {
		char name[RG_TILE_NAME_MAX];

		AG_CopyString(name, buf, sizeof(name));
		Debug(ts, "Tile mapping %u: <%s>\n", i, name);
		if (name[0] == '\0') {
			ts->tileTbl[i] = NULL;
		} else {
			if ((ts->tileTbl[i] = RG_TilesetFindTile(ts, name)) == NULL) {
				AG_SetError("%s: Bad tile mapping: %s (%u)",
				    OBJECT(ts)->name, name, (Uint)i);
				goto fail;
			}
		}
	}

	/* Resolve the pixmap brush references. */
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		RG_Brush *pbr;
		RG_Pixmap *ppx;

		TAILQ_FOREACH(pbr, &px->brushes, brushes) {
			if (pbr->px != NULL) {
				continue;
			}
			TAILQ_FOREACH(ppx, &ts->pixmaps, pixmaps) {
				if (strcmp(pbr->px_name, ppx->name) == 0) {
					pbr->px = ppx;
					pbr->px->nRefs++;
					break;
				}
			}
			if (ppx == NULL) {
				AG_SetError("%s: Bad pixmap ref", pbr->px_name);
				goto fail;
			}
		}
	}
	AG_MutexUnlock(&ts->lock);
	return (0);
fail:
	AG_MutexUnlock(&ts->lock);
	return (-1);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	RG_Tileset *ts = obj;
	Uint32 count, i;
	off_t offs;
/*	RG_Sketch *sk; */
	RG_Pixmap *px;
	RG_Tile *t;
	RG_Feature *ft;
	RG_Texture *tex;

	AG_MutexLock(&ts->lock);
	AG_WriteString(buf, ts->tmpl);
	AG_WriteUint32(buf, ts->flags);

	/* Save the vectorial sketches. */
	count = 0;
	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
#if 0
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		RG_SketchSave(sk, buf);
		count++;
	}
	AG_WriteUint32At(buf, count, offs);
#endif

	/* Save the pixmaps. */
	count = 0;
	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		RG_PixmapSave(px, buf);
		count++;
	}
	AG_WriteUint32At(buf, count, offs);

	/* Save the features. */
	count = 0;
	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(ft, &ts->features, features) {
		off_t ftsize_offs;

		AG_WriteString(buf, ft->name);
		AG_WriteString(buf, ft->ops->type);
		AG_WriteUint32(buf, ft->flags);

		/* Encode the size to allow skipping unimplemented features. */
		ftsize_offs = AG_Tell(buf);
		AG_WriteUint32(buf, 0);
		RG_FeatureSave(ft, buf);
		AG_WriteUint32At(buf, AG_Tell(buf)-ftsize_offs, ftsize_offs);

		count++;
	}
	AG_WriteUint32At(buf, count, offs);
	
	/* Save the tiles. */
	count = 0;
	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		RG_TileSave(t, buf);
		count++;
	}
	AG_WriteUint32At(buf, count, offs);
	Debug(ts, "Saved %u tiles\n", (Uint)count);
	
/*	AG_WriteUint32(buf, 0); LEGACY nAnims */
	
	/* Save the textures. */
	count = 0;
	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(tex, &ts->textures, textures) {
		AG_WriteString(buf, tex->name);
		RG_TextureSave(tex, buf);
		count++;
	}
	AG_WriteUint32At(buf, count, offs);
	Debug(ts, "Saved %u textures\n", (Uint)count);

	/* Save the static tile mappings. */
	AG_WriteUint32(buf, ts->nTileTbl);
	for (i = 0; i < ts->nTileTbl; i++) {
		AG_WriteString(buf, ts->tileTbl[i]->name);
	}
	/* AG_WriteUint32(buf, 0); LEGACY nAnimTbl */
	Debug(ts, "Saved %u tileTbl entries\n", ts->nTileTbl);

	AG_MutexUnlock(&ts->lock);
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

#if 0
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
#endif

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
RG_TilesetResvPixmap(void *vfsRoot, const char *tsname, const char *pxname)
{
	RG_Pixmap *px;
	RG_Tileset *ts;

	if ((ts = AG_ObjectFindS(vfsRoot, tsname)) == NULL) {
		AG_SetError("%s: no such tileset", tsname);
		return (NULL);
	}
	if (!AG_OfClass(ts, "RG_Tileset:*")) {
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
RG_TilesetResvTile(void *vfsRoot, const char *tsname, const char *tname)
{
	RG_Tileset *ts;
	RG_Tile *t;

	if ((ts = AG_ObjectFindS(vfsRoot, tsname)) == NULL) {
		AG_SetError("%s: no such tileset", tsname);
		return (NULL);
	}
	if (!AG_OfClass(ts, "RG_Tileset:*")) {
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

static void
PollGraphics(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Tileset *ts = RG_TILESET_PTR(1);
	RG_Pixmap *px;
/*	RG_Sketch *sk; */
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u) [#%u]",
		    px->name, px->su->w, px->su->h, px->nRefs);
		it->p1 = px;
		it->cat = "pixmap";
		AG_TlistSetIcon(tl, it, px->su);
	}
#if 0
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		it = AG_TlistAdd(tl, NULL,
		    "%s (%ux%u %.0f%%) [#%u]", sk->name, sk->vg->su->w,
		    sk->vg->su->h, sk->vg->scale*100.0, sk->nRefs);
		it->cat = "sketch";
		it->p1 = sk;
		AG_TlistSetIcon(tl, it, sk->vg->su);
	}
#endif
	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
PollTextures(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Tileset *ts = RG_TILESET_PTR(1);
	RG_Texture *tex;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(tex, &ts->textures, textures) {
		RG_Tile *t;

		if (tex->tileset[0] != '\0' && tex->tile[0] != '\0' &&
		    (t = RG_TilesetResvTile(OBJECT(ts)->root, tex->tileset,
		     tex->tile)) != NULL) {
			it = AG_TlistAdd(tl, NULL, "%s (<%s> %ux%u)",
			    tex->name, t->name, t->su->w, t->su->h);
			AG_TlistSetIcon(tl, it, t->su);
		} else {
			it = AG_TlistAdd(tl, NULL, "%s (<%s:%s>?)",
			    tex->name, tex->tileset, tex->tile);
		}
		it->cat = "texture";
		it->p1 = tex;
	}
	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
PollTiles(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Tileset *ts = RG_TILESET_PTR(1);
	RG_Tile *t;
	AG_TlistItem *it;
	RG_TileElement *tel;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u)", t->name,
		    t->su->w, t->su->h);
		it->depth = 0;
		it->cat = "tile";
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
/*					RG_FeatureSketch *fts; */

					it = AG_TlistAdd(tl, rgIconObject.s,
					    "%s [%d,%d] %s", ft->name,
					    tel->tel_feature.x,
					    tel->tel_feature.y,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->cat = "tile-feature";
					it->p1 = tel;
#if 0
					TAILQ_FOREACH(fts, &ft->sketches,
					    sketches) {
						it = AG_TlistAdd(tl,
						    rgIconDrawing.s,
						    "%s [at %d,%d]%s",
						    fts->sk->name,
						    fts->x, fts->y,
						    fts->visible ? "" :
						    "(invisible)");
						it->depth = 2;
						it->cat = "feature-sketch";
						it->p1 = fts;
					}
#endif
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
					it->cat = "tile-pixmap";
					it->p1 = tel;
					AG_TlistSetIcon(tl, it, px->su);
				}
				break;
#if 0
			case RG_TILE_SKETCH:
				{
					RG_Sketch *sk = tel->tel_sketch.sk;

					it = AG_TlistAdd(tl, rgIconDrawing.s,
					    "%s (%ux%u)%s", sk->name,
					    tel->tel_sketch.sk->vg->su->w,
					    tel->tel_sketch.sk->vg->su->h,
					    tel->visible ? "" : "(invisible)");
					it->depth = 1;
					it->cat = "tile-pixmap";
					it->p1 = tel;
				}
				break;
#endif
			}
		}
	}
	AG_TlistRestore(tl);
	AG_MutexUnlock(&ts->lock);
}

static char ins_tile_name[RG_TILE_NAME_MAX];
static char ins_texture_name[RG_TEXTURE_NAME_MAX];
static int ins_tile_w = RG_TILESZ*2;
static int ins_tile_h = RG_TILESZ*2;
static int ins_alpha = 0;
static int ins_colorkey = 1;
static Uint ins_snap_mode = RG_SNAP_NONE;

static int
InsertTileMapping(RG_Tileset *_Nonnull ts, RG_Tile *_Nonnull t,
    Uint32 *_Nullable id)
{
	Uint32 i;

	/* Try to recycle NULL ids if we reach the threshold. */
	if (ts->nTileTbl >= RG_TILE_ID_MINREUSE) {
		for (i = 0; i < ts->nTileTbl; i++) {
			if (ts->tileTbl[i] != NULL) {
				continue;
			}
			ts->tileTbl[i] = t;
			if (id != NULL) { *id = i; }
			return (0);
		}
	}
	if ((ts->nTileTbl+1) >= RG_TILE_ID_MAX) {
		AG_SetError("Out of tile ID space");
		return (-1);
	}
	ts->tileTbl = Realloc(ts->tileTbl, (ts->nTileTbl+1)*sizeof(RG_Tile *));
	ts->tileTbl[ts->nTileTbl] = t;
	if (id != NULL) { *id = ts->nTileTbl++; }
	return (0);
}

static __inline__ void
RemoveTileMappings(RG_Tileset *_Nonnull ts, RG_Tile *_Nonnull t)
{
	Uint32 i;

	for (i = 0; i < ts->nTileTbl; i++) {
		if (ts->tileTbl[i] == t)
			ts->tileTbl[i] = NULL;
	}
}

static void
InsertTile(AG_Event *_Nonnull event)
{
	AG_Window *pwin = AG_WINDOW_PTR(1);
	RG_Tileset *ts = RG_TILESET_PTR(2);
	RG_Tile *t;
	Uint tileFlags = 0;
	int w, h;

	if (ins_alpha)		tileFlags |= RG_TILE_SRCALPHA;
	if (ins_colorkey)	tileFlags |= RG_TILE_SRCCOLORKEY;

	if (ins_tile_name[0] == '\0') {
		Uint nameno = 0;
tryname1:
		Snprintf(ins_tile_name, sizeof(ins_tile_name), _("Tile #%d"),
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
					Snprintf(np, sizeof(ins_tile_name) -
					    (np-ins_tile_name)-1, "%d", num);
					break;
				}
				if (!isdigit((int) *np)) {
					Strlcat(ins_tile_name, "_",
					    sizeof(ins_tile_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	t = Malloc(sizeof(RG_Tile));
	RG_TileInit(t, ts, ins_tile_name);
	t->flags |= tileFlags;

	if (sscanf(ts->tmpl, "Sprite (%dx%d)", &w, &h) == 2) {
		RG_TileScale(ts, t, w, h);
		t->xOrig = w/2;
		t->yOrig = h/2;
		t->snap_mode = RG_SNAP_TO_GRID;
//		RG_TILE_LAYER2(t,0,0) = +2;
//		RG_TILE_LAYER2(t,0,1) = +1;
//		RG_TILE_ATTR2(t,0,1) = RG_TILE_BLOCK;
	} else if (sscanf(ts->tmpl, "Terrain (%dx%d)", &w, &h) == 2) {
		RG_TileScale(ts, t, w, h);
		t->xOrig = w/2;
		t->yOrig = h/2;
		t->snap_mode = RG_SNAP_TO_GRID;
	} else if (sscanf(ts->tmpl, "Icons (%dx%d)", &w, &h) == 2) {
		RG_TileScale(ts, t, w, h);
		t->xOrig = 0;
		t->yOrig = 0;
		t->snap_mode = RG_SNAP_TO_GRID;
	} else {
		RG_TileScale(ts, t, ins_tile_w, ins_tile_h);
		t->xOrig = t->su->w/2;
		t->yOrig = t->su->h/2;
		t->snap_mode = ins_snap_mode;
	}
	if (InsertTileMapping(ts, t, &t->main_id) == 0) {
		TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	} else {
		RG_TileDestroy(t);
		Free(t);
		AG_TextMsg(AG_MSG_ERROR, _("Failed to create item: %s"),
		    AG_GetError());
	}
	ins_tile_name[0] = '\0';
	AG_ObjectDetach(pwin);
}

static void
InsertTexture(AG_Event *_Nonnull event)
{
	AG_Window *dlgwin = AG_WINDOW_PTR(1);
	AG_Window *pwin = AG_WINDOW_PTR(2);
	RG_Tileset *ts = RG_TILESET_PTR(3);
	AG_Window *win;
	RG_Texture *tex;

	if (ins_texture_name[0] == '\0') {
		Uint nameno = 0;
tryname1:
		Snprintf(ins_texture_name, sizeof(ins_texture_name),
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
					Snprintf(np, sizeof(ins_texture_name) -
					    (np-ins_texture_name)-1, "%d", num);
					break;
				}
				if (!isdigit((int) *np)) {
					Strlcat(ins_texture_name, "_",
					    sizeof(ins_texture_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	tex = Malloc(sizeof(RG_Texture));
	RG_TextureInit(tex, ins_texture_name);
	TAILQ_INSERT_TAIL(&ts->textures, tex, textures);
	
	ins_texture_name[0] = '\0';
	AG_ObjectDetach(dlgwin);
	
	if ((win = RG_TextureEdit(OBJECT(ts)->root, tex)) != NULL) {
		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
InsertTileDlg(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Window *pwin = AG_WINDOW_PTR(2);
	AG_Window *win;
	AG_Box *btnbox;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;
/*	AG_Combo *com; */

	if ((win = AG_WindowNewNamedS(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE|
	    AG_WINDOW_NOMINIMIZE, "rg-instiledlg")) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Create new tile"));
	
	tb = AG_TextboxNew(win, 0, _("Name: "));
	AG_TextboxBindUTF8(tb, ins_tile_name, sizeof(ins_tile_name));
	AG_TextboxSizeHint(tb, "XXXXXXXXXXXXXXXXXXX");
	AG_SetEvent(tb, "textbox-return", InsertTile, "%p,%p", win, ts);
	AG_WidgetFocus(tb);

	msb = AG_MSpinbuttonNew(win, 0, "x", _("Size: "));
	AG_BindInt(msb, "xvalue", &ins_tile_w);
	AG_BindInt(msb, "yvalue", &ins_tile_h);
	AG_MSpinbuttonSetRange(msb, RG_TILE_SIZE_MIN, RG_TILE_SIZE_MAX);

	AG_CheckboxNewInt(win, 0, _("Alpha blending"), &ins_alpha);
	AG_CheckboxNewInt(win, 0, _("Colorkey"), &ins_colorkey);
	
	AG_LabelNewS(win, 0, _("Snapping mode: "));
	AG_RadioNewUint(win, AG_RADIO_HFILL, rgTileSnapModes, &ins_snap_mode);

	btnbox = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(btnbox, 0, _("OK"), InsertTile, "%p,%p",win,ts);
		AG_ButtonNewFn(btnbox, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
InsertTextureDlg(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Window *pwin = AG_WINDOW_PTR(2);
	AG_Window *win;
	AG_Box *btnbox;
	AG_Textbox *tb;

	if ((win = AG_WindowNewNamedS(AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	    AG_WINDOW_NOMINIMIZE, "rg-instexturedlg")) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Create a new texture"));
	
	tb = AG_TextboxNew(win, 0, _("Name:"));
	AG_TextboxBindUTF8(tb, ins_texture_name, sizeof(ins_texture_name));
	AG_TextboxSizeHint(tb, "XXXXXXXXXXXXXXXXXXX");
	AG_SetEvent(tb, "textbox-return", InsertTexture, "%p,%p,%p", win,
	    pwin, ts);
	AG_WidgetFocus(tb);

	btnbox = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(btnbox, 0, _("OK"), InsertTexture,
		    "%p,%p,%p", win, pwin, ts);
		AG_ButtonNewFn(btnbox, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
DeleteSelTiles(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	RG_Tileset *ts = RG_TILESET_PTR(2);
	AG_TlistItem *it;

	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		RG_Tile *t = it->p1;

		if (!it->selected) {
			continue;
		}
		if (t->nRefs > 0) {
			AG_TextMsg(AG_MSG_ERROR, _("The tile `%s' is in use."),
			    t->name);
			continue;
		}
		TAILQ_REMOVE(&ts->tiles, t, tiles);
		RemoveTileMappings(ts, t);
		RG_TileDestroy(t);
		Free(t);
	}
	AG_MutexUnlock(&ts->lock);
}

static void
TileDup(RG_Tileset *_Nonnull ts, RG_Tile *_Nonnull t1)
{
	char name[RG_TILE_NAME_MAX];
	RG_Tile *t2;
	RG_TileElement *e1;
	int x, y;
	int ncopy = 0;

	t2 = Malloc(sizeof(RG_Tile));
tryname1:
	Snprintf(name, sizeof(name), _("Copy #%d"), ncopy++);
	if (RG_TilesetFindTile(ts, name) != NULL) {
		goto tryname1;
	}
	RG_TileInit(t2, ts, name);
	t2->flags |= t1->flags;
	RG_TileScale(ts, t2, t1->su->w, t1->su->h);
	t2->su->alpha = t1->su->alpha;
	Strlcpy(t2->clname, t1->clname, sizeof(t2->clname));

	t2->xOrig = t1->xOrig;
	t2->yOrig = t1->yOrig;
	t2->snap_mode = t1->snap_mode;
	for (y = 0; y < t1->nh; y++) {
		for (x = 0; x < t1->nw; x++) {
			t2->attrs[y*t1->nw+x] = t1->attrs[y*t1->nw+x];
			t2->layers[y*t1->nw+x] = t1->layers[y*t1->nw+x];
		}
	}
	
	TAILQ_FOREACH(e1, &t1->elements, elements) {
		RG_TileElement *e2;
		RG_Pixmap *px1, *px2;

		ncopy = 0;
		switch (e1->type) {
		case RG_TILE_PIXMAP:
			px1 = e1->tel_pixmap.px;
			px2 = Malloc(sizeof(RG_Pixmap));
			RG_PixmapInit(px2, ts, 0);
tryname2:
			Snprintf(px2->name, sizeof(px2->name),
			    _("Copy #%d"), ncopy++);
			if (RG_TilesetFindPixmap(ts, px2->name) != NULL) {
				goto tryname2;
			}
			RG_PixmapScale(px2, px1->su->w, px1->su->h);
			memcpy((Uint8 *)px2->su->pixels,
			    (Uint8 *)px1->su->pixels,
			    px1->su->h * px1->su->pitch +
			    px1->su->w * px1->su->format.BytesPerPixel);

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
DupSelTiles(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tl = AG_TLIST_PTR(2);
	AG_TlistItem *it;

	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		RG_Tile *t = it->p1;

		if (!it->selected) {
			continue;
		}
		TileDup(ts, t);
	}
	AG_MutexUnlock(&ts->lock);
}

static void
EditSelTiles(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tl = AG_TLIST_PTR(2);
	AG_Window *pwin = AG_WINDOW_PTR(3);
	AG_Window *win;
	AG_TlistItem *it;

	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected ||
		    strcmp(it->cat, "tile") != 0) {
			continue;
		}
		if ((win = RG_TileEdit(ts, (RG_Tile *)it->p1)) != NULL) {
			AG_WindowAttach(pwin, win);
			AG_WindowShow(win);
		}
	}
	AG_MutexUnlock(&ts->lock);
}

static void
EditSelTextures(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tl = AG_TLIST_PTR(2);
	AG_Window *pwin = AG_WINDOW_PTR(3);
	AG_Window *win;
	AG_TlistItem *it;

	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected) {
			continue;
		}
		if ((win = RG_TextureEdit(OBJECT(ts)->root,
		    (RG_Texture *)it->p1)) != NULL) {
			AG_WindowAttach(pwin, win);
			AG_WindowShow(win);
		}
	}
	AG_MutexUnlock(&ts->lock);
}

static void
DeleteSelPixmaps(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tlGfx = AG_TLIST_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlGfx)) != NULL) {
		RG_Pixmap *px = it->p1;
	
		if (px->nRefs > 0) {
			return;
		}
		TAILQ_REMOVE(&ts->pixmaps, px, pixmaps);
		RG_PixmapDestroy(px);
		Free(px);
	}
}

#if 0
static void
DeleteSelSketches(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tlGfx = AG_TLIST_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlGfx)) != NULL) {
		RG_Sketch *sk = it->p1;
	
		if (sk->nRefs > 0) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The sketch \"%s\" is currently in use."),
			    sk->name);
			return;
		}
		TAILQ_REMOVE(&ts->sketches, sk, sketches);
		RG_SketchDestroy(sk);
		Free(sk);
	}
}
#endif

static void
DeleteSelGraphics(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tlGfx = AG_TLIST_PTR(2);
	AG_TlistItem *it;

	AG_TLIST_FOREACH(it, tlGfx) {
		if (strcmp(it->cat, "pixmap") == 0) {
			RG_Pixmap *px = it->p1;
	
			if (px->nRefs > 0) {
				continue;
			}
			TAILQ_REMOVE(&ts->pixmaps, px, pixmaps);
			RG_PixmapDestroy(px);
			Free(px);
#if 0
		} else if (strcmp(it->cat, "sketch") == 0) {
			RG_Sketch *sk = it->p1;
	
			if (sk->nRefs > 0) {
				continue;
			}
			TAILQ_REMOVE(&ts->sketches, sk, sketches);
			RG_SketchDestroy(sk);
			Free(sk);
#endif
		}
	}
}

static void
DeleteSelTextures(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tlTextures = AG_TLIST_PTR(2);
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tlTextures->items, items) {
		RG_Texture *tex = it->p1;

		if (!it->selected)
			continue;

		TAILQ_REMOVE(&ts->textures, tex, textures);
		RG_TextureDestroy(tex);
		Free(tex);
	}
}

static void
DupSelPixmaps(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Tlist *tlGfx = AG_TLIST_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlGfx)) != NULL) {
		RG_Pixmap *px1 = it->p1;
		RG_Pixmap *px2;
		int ncopy = 0;

		px2 = Malloc(sizeof(RG_Pixmap));
		RG_PixmapInit(px2, ts, 0);
tryname:
		Snprintf(px2->name, sizeof(px2->name), _("Copy #%d"),
		    ncopy++);
		if (RG_TilesetFindPixmap(ts, px2->name) != NULL)
			goto tryname;

		RG_PixmapScale(px2, px1->su->w, px1->su->h);
		TAILQ_INSERT_TAIL(&ts->pixmaps, px2, pixmaps);

		memcpy(
		    (Uint8 *)px2->su->pixels,
		    (Uint8 *)px1->su->pixels,
		    px1->su->h * px1->su->pitch +
		    px1->su->w * px1->su->format.BytesPerPixel);
	}
}

static void
SelectTemplate(AG_Event *_Nonnull event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	int w, h;

	AG_ObjectReset(ts);

	if (sscanf(ts->tmpl, "Sprite (%dx%d)", &w, &h) == 2) {
		const char *tiles[] = {
			"Idle-N", "Idle-S",
			"Idle-W", "Idle-E"
		};
		const int ntiles = sizeof(tiles)/sizeof(tiles[0]);
		RG_Tile *t;
		int i;

		for (i = 0; i < ntiles; i++) {
			t = Malloc(sizeof(RG_Tile));
			RG_TileInit(t, ts, tiles[i]);
			RG_TileScale(ts, t, w, h);
			TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
		}
	}
}

static void
PollTileTbl(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_TlistItem *it;
	Uint32 i;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	for (i = 0; i < ts->nTileTbl; i++) {
		if (ts->tileTbl[i] == NULL) {
			it = AG_TlistAdd(tl, NULL, "%u. NULL", (Uint)i);
			it->cat = "";
		} else {
			RG_Tile *t = ts->tileTbl[i];
			
			it = AG_TlistAdd(tl, NULL, "%u. %s (%ux%u)", (Uint)i,
			    t->name, t->su->w, t->su->h);
			it->cat = "tileid";
			it->p1 = t;
			AG_TlistSetIcon(tl, it, t->su);
		}
	}
	AG_TlistRestore(tl);
	AG_MutexUnlock(&ts->lock);
}

static void *_Nonnull
Edit(void *_Nonnull p)
{
	RG_Tileset *ts = p;
	AG_Window *win;
	AG_Tlist *tlTiles, *tlGfx, *tlTextures, *tl[4];
	AG_Tlist *tlTileTbl;
	AG_Box *bbox;
	AG_MenuItem *mi;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Combo *com;
	int i;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(ts)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 1);

	AG_SetStyle(win, "spacing", "0");
	AG_SetStyleF(win, "padding", "0 %d %d %d",
	    WIDGET(win)->paddingRight,
	    WIDGET(win)->paddingBottom,
	    WIDGET(win)->paddingLeft);

	tlTiles = tl[0] = AG_TlistNew(NULL, AG_TLIST_POLL | AG_TLIST_MULTI |
	                                    AG_TLIST_EXPAND);
	AG_TlistSizeHint(tlTiles, "XXXXXXXXXXXXXXXXX (00x00)", 6);
	AG_SetEvent(tlTiles, "tlist-poll", PollTiles, "%p", ts);

	tlGfx = tl[1] = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_EXPAND);
	AG_SetEvent(tlGfx, "tlist-poll", PollGraphics, "%p", ts);

	tlTextures = tl[2] = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tlTextures, "tlist-poll", PollTextures, "%p", ts);

	for (i = 0; i < 3; i++) {
		AG_TlistSetItemHeight(tl[i], 32);
		AG_TlistSetIconWidth(tl[i], 32);
		AG_SetStyle(tl[i], "font-size", "150%");
	}

	tlTileTbl = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tlTileTbl, "tlist-poll", PollTileTbl, "%p", ts);

	mi = AG_TlistSetPopup(tlTiles, "tile");
	{
		AG_MenuAction(mi, _("Edit tiles..."), rgIconEdit.s,
		    EditSelTiles, "%p,%p,%p", ts, tlTiles, win);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Duplicate tiles"), rgIconDuplicate.s,
		    DupSelTiles, "%p,%p", ts, tlTiles);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Delete tiles"), agIconTrash.s,
		    DeleteSelTiles, "%p,%p", tlTiles, ts);
	}

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Tiles"), AG_BOX_VERT);
	{
		AG_ObjectAttach(nt, tlTiles);
		AG_SetEvent(tlTiles, "tlist-dblclick",
		    EditSelTiles, "%p,%p,%p", ts, tlTiles, win);

		bbox = AG_BoxNewHoriz(nt, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
		{
			AG_ButtonNewFn(bbox, 0, _("Insert"),
			    InsertTileDlg, "%p,%p", ts, win);
			AG_ButtonNewFn(bbox, 0, _("Edit"),
			    EditSelTiles, "%p,%p,%p", ts, tlTiles, win);
			AG_ButtonNewFn(bbox, 0, _("Delete"),
			    DeleteSelTiles, "%p,%p", tlTiles, ts);
		}
		com = AG_ComboNew(nt, AG_COMBO_HFILL, _("Use Template: "));
		AG_ComboSizeHint(com, "XXXXXXXXXXXX (64x64) ", 8);
		AG_TlistAdd(com->list, agIconDoc.s, "Icons (16x16)");
		AG_TlistAdd(com->list, agIconDoc.s, "Icons (32x32)");
		AG_TlistAdd(com->list, agIconDoc.s, "Icons (64x64)");
		AG_TlistAdd(com->list, agIconDoc.s, "Sprite (16x16)");
		AG_TlistAdd(com->list, agIconDoc.s, "Sprite (16x32)");
		AG_TlistAdd(com->list, agIconDoc.s, "Sprite (32x32)");
		AG_TlistAdd(com->list, agIconDoc.s, "Sprite (32x64)");
		AG_TlistAdd(com->list, agIconDoc.s, "Sprite (64x64)");
		AG_TlistAdd(com->list, agIconDoc.s, "Terrain (64x64)");
		AG_TextboxBindUTF8(com->tbox, ts->tmpl, sizeof(ts->tmpl));
		AG_SetEvent(com, "combo-selected", SelectTemplate, "%p", ts);
	}

	nt = AG_NotebookAdd(nb, _("Graphics"), AG_BOX_VERT);
	{
		AG_ObjectAttach(nt, tlGfx);
	
		mi = AG_TlistSetPopup(tlGfx, "pixmap");
		{
			AG_MenuAction(mi, _("Delete pixmap"), agIconTrash.s,
			    DeleteSelPixmaps, "%p,%p", ts, tlGfx);
			AG_MenuAction(mi, _("Duplicate pixmap"),
			    rgIconDuplicate.s,
			    DupSelPixmaps, "%p,%p", ts, tlGfx);
		}
#if 0
		mi = AG_TlistSetPopup(tlGfx, "sketch");
		{
			AG_MenuAction(mi, _("Delete sketch"), agIconTrash.s,
			    DeleteSelSketches, "%p,%p", ts, tlGfx);
		}
#endif
		bbox = AG_BoxNewHoriz(nt, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
		{
			AG_ButtonNewFn(bbox, 0,
			    _("Delete selected/unreferenced"),
			    DeleteSelGraphics, "%p,%p", ts, tlGfx);
		}
	}
	
	nt = AG_NotebookAdd(nb, _("Textures"), AG_BOX_VERT);
	{
		AG_ObjectAttach(nt, tlTextures);
	
		mi = AG_TlistSetPopup(tlTextures, "texture");
		{
			AG_MenuAction(mi, _("Delete texture"), agIconTrash.s,
			    DeleteSelTextures, "%p,%p", ts, tlTextures);
		}
		
		bbox = AG_BoxNewHoriz(nt, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
		{
			AG_ButtonNewFn(bbox, 0, _("Insert"),
			    InsertTextureDlg, "%p,%p", ts, win);
			AG_ButtonNewFn(bbox, 0, _("Edit"),
			    EditSelTextures, "%p,%p,%p", ts, tlTextures, win);
			AG_ButtonNewFn(bbox, 0, _("Delete"),
			    DeleteSelTextures, "%p,%p", ts, tlTextures);
		}
		
		AG_SetEvent(tlTextures, "tlist-dblclick",
		    EditSelTextures, "%p,%p,%p", ts, tlTextures, win);
	}
	
	nt = AG_NotebookAdd(nb, _("Mappings"), AG_BOX_VERT);
	{
		AG_LabelNewS(nt, 0, _("Tiles:"));
		AG_ObjectAttach(nt, tlTileTbl);
	}
	return (win);
}

AG_ObjectClass rgTilesetClass = {
	"RG(Tileset)",
	sizeof(RG_Tileset),
	{ 9, 0 },
	Init,
	Reset,
	Destroy,
	Load,
	Save,
	Edit
};

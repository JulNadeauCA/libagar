/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/tlist.h>
#include <gui/button.h>
#include <gui/textbox.h>
#include <gui/mspinbutton.h>
#include <gui/checkbox.h>
#include <gui/menu.h>
#include <gui/notebook.h>
#include <gui/radio.h>
#include <gui/combo.h>
#include <gui/pane.h>
#include <gui/icons.h>

#include "tileset.h"
#include "tileview.h"
#include "animview.h"
#include "texsel.h"
#include "icons.h"
#include "icons_data.h"

#include <ctype.h>
#include <string.h>

extern const RG_FeatureOps rgFillOps;
/* extern const RG_FeatureOps rgSketchProjOps; */

const RG_FeatureOps *feature_tbl[] = {
	&rgFillOps,
/*	&rgSketchProjOps, */
	NULL
};
extern const char *rgTileSnapModes[];

void
RG_InitSubsystem(void)
{
	AG_RegisterNamespace("RG", "RG_", "http://libagar.org/");
	AG_RegisterClass(&rgTileviewClass);
	AG_RegisterClass(&rgAnimviewClass);
	AG_RegisterClass(&rgTextureSelectorClass);
	AG_RegisterClass(&rgTilesetClass);

	rgIcon_Init();
}

void
RG_DestroySubsystem(void)
{
	AG_UnregisterClass(&rgTileviewClass);
	AG_UnregisterClass(&rgAnimviewClass);
	AG_UnregisterClass(&rgTextureSelectorClass);
	AG_UnregisterClass(&rgTilesetClass);
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
Init(void *obj)
{
	RG_Tileset *ts = obj;

	/* Restart the graphical editor on load. */
	OBJECT(ts)->flags |= AG_OBJECT_REOPEN_ONLOAD;

	AG_MutexInitRecursive(&ts->lock);
	TAILQ_INIT(&ts->tiles);
#if 0
	TAILQ_INIT(&ts->sketches);
#endif
	TAILQ_INIT(&ts->pixmaps);
	TAILQ_INIT(&ts->features);
	TAILQ_INIT(&ts->animations);
	TAILQ_INIT(&ts->textures);

	ts->icon = AG_SurfaceRGBA(32,32, agSurfaceFmt->BitsPerPixel,
	    AG_SRCALPHA|AG_SRCCOLORKEY,
	    agSurfaceFmt->Rmask,
	    agSurfaceFmt->Gmask,
	    agSurfaceFmt->Bmask,
	    agSurfaceFmt->Amask);

	if (ts->icon == NULL) {
		AG_FatalError(NULL);
	}
	ts->fmt = ts->icon->format;
	ts->flags = 0;
	ts->tmpl[0] = '\0';
	ts->tiletbl = NULL;
	ts->ntiletbl = 0;
	ts->animtbl = NULL;
	ts->nanimtbl = 0;
}

static void
FreeDataset(void *obj)
{
	RG_Tileset *ts = obj;
	RG_Tile *t, *nt;
/*	RG_Sketch *sk, *nsk; */
	RG_Pixmap *px, *npx;
	RG_Feature *ft, *nft;
	RG_Anim *ani, *nani;
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
	for (ani = TAILQ_FIRST(&ts->animations);
	     ani != TAILQ_END(&ts->animations);
	     ani = nani) {
		nani = TAILQ_NEXT(ani, animations);
		RG_AnimDestroy(ani);
		Free(ani);
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
	TAILQ_INIT(&ts->animations);
	TAILQ_INIT(&ts->textures);

	Free(ts->tiletbl);
	ts->tiletbl = NULL;
	Free(ts->animtbl);
	ts->animtbl = NULL;

	ts->ntiletbl = 0;
	ts->nanimtbl = 0;

	AG_MutexUnlock(&ts->lock);
}

static void
Destroy(void *obj)
{
	RG_Tileset *ts = obj;

	AG_MutexDestroy(&ts->lock);
	AG_SurfaceFree(ts->icon);
	Free(ts->tiletbl);
	Free(ts->animtbl);
}

static int
Load(void *obj, AG_DataSource *buf, const AG_Version *ver)
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
			Debug(ts, "Unimplemented feature: %s (class=%s); "
			          "skipping %lu bytes.\n", name, type,
				  (Ulong)len);
			AG_Seek(buf, len-4, AG_SEEK_CUR);
			continue;
		}

		ft = Malloc((*ftops)->len);
		ft->ops = *ftops;
		ft->ops->init(ft, ts, flags);
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
		RG_TileScale(ts, t, t->su->w, t->su->h, t->flags);
		RG_TileGenerate(t);
		TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	}

	/* Load the animation information. */
	count = AG_ReadUint32(buf);
	Debug(ts, "Loading %u animations\n", count);
	for (i = 0; i < count; i++) {
		char name[RG_ANIMATION_NAME_MAX];
		RG_Anim *ani;
		int flags;
		
		ani = Malloc(sizeof(RG_Anim));
		AG_CopyString(name, buf, sizeof(name));
		flags = (int)AG_ReadUint32(buf);
		RG_AnimInit(ani, ts, name, flags);
		if (RG_AnimLoad(ani, buf) == -1) {
			RG_AnimDestroy(ani);
			Free(ani);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->animations, ani, animations);
	}
	
	/* Load the textures. */
	count = AG_ReadUint32(buf);
	Debug(ts, "Loading %u textures\n", count);
	for (i = 0; i < count; i++) {
		char name[RG_TEXTURE_NAME_MAX];
		RG_Texture *tex;
		
		tex = Malloc(sizeof(RG_Texture));
		AG_CopyString(name, buf, sizeof(name));
		RG_TextureInit(tex, ts, name);
		if (RG_TextureLoad(tex, buf) == -1) {
			RG_TextureDestroy(tex);
			Free(tex);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&ts->textures, tex, textures);
	}

	/* Load and resolve the static tile and animation mappings. */
	ts->ntiletbl = AG_ReadUint32(buf);
	Debug(ts, "Tiletbl has %u entries\n", (Uint)ts->ntiletbl);
	ts->tiletbl = Realloc(ts->tiletbl, ts->ntiletbl*sizeof(RG_Tile *));
	for (i = 0; i < ts->ntiletbl; i++) {
		char name[RG_TILE_NAME_MAX];

		AG_CopyString(name, buf, sizeof(name));
		Debug(ts, "Tile mapping %u: <%s>\n", i, name);
		if (name[0] == '\0') {
			ts->tiletbl[i] = NULL;
		} else {
			if ((ts->tiletbl[i] = RG_TilesetFindTile(ts, name))
			    == NULL) {
				AG_FatalError("%s: Bad tile mapping: %s (%u)",
				    OBJECT(ts)->name, name, (Uint)i);
			}
		}
	}
	ts->nanimtbl = AG_ReadUint32(buf);
	Debug(ts, "Animtbl has %u entries\n", (Uint)ts->nanimtbl);
	ts->animtbl = Realloc(ts->animtbl, ts->nanimtbl*sizeof(RG_Anim *));
	for (i = 0; i < ts->nanimtbl; i++) {
		char name[RG_ANIMATION_NAME_MAX];

		AG_CopyString(name, buf, sizeof(name));
		Debug(ts, "Anim mapping %u: <%s>\n", i, name);
		if (name[0] == '\0') {
			ts->animtbl[i] = NULL;
		} else {
			if ((ts->animtbl[i] = RG_TilesetFindAnim(ts, name))
			    == NULL) {
				AG_FatalError("%s: Bad anim mapping: %s (%u)",
				    OBJECT(ts)->name, name, (Uint)i);
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
					pbr->px->nrefs++;
					break;
				}
			}
			if (ppx == NULL)
				AG_FatalError("%s: Bad pixmap ref",
				    pbr->px_name);
		}
	}
	AG_MutexUnlock(&ts->lock);
	return (0);
fail:
	AG_MutexUnlock(&ts->lock);
	return (-1);
}

static int
Save(void *obj, AG_DataSource *buf)
{
	RG_Tileset *ts = obj;
	Uint32 count, i;
	off_t offs;
/*	RG_Sketch *sk; */
	RG_Pixmap *px;
	RG_Anim *ani;
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
	
	/* Save the animation information. */
	count = 0;
	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(ani, &ts->animations, animations) {
		AG_WriteString(buf, ani->name);
		AG_WriteUint32(buf, (Uint32)ani->flags);
		RG_AnimSave(ani, buf);
		count++;
	}
	AG_WriteUint32At(buf, count, offs);
	Debug(ts, "Saved %u animations\n", (Uint)count);
	
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

	/* Save the static tile and animation mappings. */
	AG_WriteUint32(buf, ts->ntiletbl);
	for (i = 0; i < ts->ntiletbl; i++) {
		AG_WriteString(buf, (ts->tiletbl[i]->name != NULL) ?
		                     ts->tiletbl[i]->name : "");
	}
	AG_WriteUint32(buf, ts->nanimtbl);
	for (i = 0; i < ts->nanimtbl; i++) {
		AG_WriteString(buf, (ts->animtbl[i]->name != NULL) ?
		                     ts->animtbl[i]->name : "");
	}
	Debug(ts, "Saved %u tiletbl and %u animtbl entries\n",
	    ts->ntiletbl, ts->nanimtbl);

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

static void
PollGraphics(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
	RG_Pixmap *px;
/*	RG_Sketch *sk; */
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);

	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u) [#%u]",
		    px->name, px->su->w, px->su->h, px->nrefs);
		it->p1 = px;
		it->cat = "pixmap";
		AG_TlistSetIcon(tl, it, px->su);
	}
#if 0
	TAILQ_FOREACH(sk, &ts->sketches, sketches) {
		it = AG_TlistAdd(tl, NULL,
		    "%s (%ux%u %.0f%%) [#%u]", sk->name, sk->vg->su->w,
		    sk->vg->su->h, sk->vg->scale*100.0, sk->nrefs);
		it->cat = "sketch";
		it->p1 = sk;
		AG_TlistSetIcon(tl, it, sk->vg->su);
	}
#endif
	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
PollTextures(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
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
PollAnims(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
	RG_Anim *ani;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);

	TAILQ_FOREACH(ani, &ts->animations, animations) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u) [#%u]", ani->name,
		    ani->w, ani->h, ani->nrefs);
		it->p1 = ani;
		it->cat = "anim";
	}
	
	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
PollTiles(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
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
/* static char ins_tile_class[RG_TILE_CLASS_MAX]; */
static char ins_texture_name[RG_TEXTURE_NAME_MAX];
static char ins_anim_name[RG_TILE_NAME_MAX];
static int ins_tile_w = RG_TILESZ;
static int ins_tile_h = RG_TILESZ;
static int ins_alpha = 0;
static int ins_colorkey = 1;
static Uint ins_snap_mode = RG_SNAP_NONE;

static int
InsertTileMapping(RG_Tileset *ts, RG_Tile *t, Uint32 *id)
{
	Uint32 i;

	/* Try to recycle NULL ids if we reach the threshold. */
	if (ts->ntiletbl >= RG_TILE_ID_MINREUSE) {
		for (i = 0; i < ts->ntiletbl; i++) {
			if (ts->tiletbl[i] != NULL) {
				continue;
			}
			ts->tiletbl[i] = t;
			if (id != NULL) { *id = i; }
			return (0);
		}
	}
	if ((ts->ntiletbl+1) >= RG_TILE_ID_MAX) {
		AG_SetError("Out of tile ID space");
		return (-1);
	}
	ts->tiletbl = Realloc(ts->tiletbl, (ts->ntiletbl+1)*sizeof(RG_Tile *));
	ts->tiletbl[ts->ntiletbl] = t;
	if (id != NULL) { *id = ts->ntiletbl++; }
	return (0);
}

static int
InsertAnimMapping(RG_Tileset *ts, RG_Anim *anim, Uint32 *id)
{
	Uint32 i;

	/* Try to recycle NULL ids if we reach the threshold. */
	if (ts->nanimtbl >= RG_ANIM_ID_MINREUSE) {
		for (i = 0; i < ts->nanimtbl; i++) {
			if (ts->animtbl[i] != NULL) {
				continue;
			}
			ts->animtbl[i] = anim;
			if (id != NULL) { *id = i; }
			return (0);
		}
	}
	if ((ts->nanimtbl+1) >= RG_ANIM_ID_MAX) {
		AG_SetError("Out of anim ID space");
		return (-1);
	}
	ts->animtbl = Realloc(ts->animtbl, (ts->nanimtbl+1)*sizeof(RG_Anim *));
	ts->animtbl[ts->nanimtbl] = anim;
	if (id != NULL) { *id = ts->nanimtbl++; }
	return (0);
}

static __inline__ void
RemoveTileMappings(RG_Tileset *ts, RG_Tile *t)
{
	Uint32 i;

	for (i = 0; i < ts->ntiletbl; i++) {
		if (ts->tiletbl[i] == t)
			ts->tiletbl[i] = NULL;
	}
}

static __inline__ void
RemoveAnimMappings(RG_Tileset *ts, RG_Anim *anim)
{
	Uint32 i;

	for (i = 0; i < ts->nanimtbl; i++) {
		if (ts->animtbl[i] == anim)
			ts->animtbl[i] = NULL;
	}
}

static void
InsertTile(AG_Event *event)
{
	AG_Window *pwin = AG_PTR(1);
	RG_Tileset *ts = AG_PTR(2);
	RG_Tile *t;
	Uint flags = 0;

	if (ins_alpha)		flags |= RG_TILE_SRCALPHA;
	if (ins_colorkey)	flags |= RG_TILE_SRCCOLORKEY;

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
				if (!isdigit(*np)) {
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
	if (strcmp(ts->tmpl, "Sprite") == 0) {
		RG_TileScale(ts, t, 16, 32, flags);
		t->xOrig = 8;
		t->yOrig = 31;
		t->snap_mode = RG_SNAP_TO_GRID;
		RG_TILE_LAYER2(t,0,0) = +2;
		RG_TILE_LAYER2(t,0,1) = +1;
		RG_TILE_ATTR2(t,0,1) = RG_TILE_BLOCK;
	} else if (strcmp(ts->tmpl, "Terrain") == 0) {
		RG_TileScale(ts, t, 64, 64, flags);
		t->xOrig = 24;
		t->yOrig = 24;
		t->snap_mode = RG_SNAP_TO_GRID;
		RG_TILE_ATTR2(t,0,0) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,1,0) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,2,0) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,3,0) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,0,1) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,0,2) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,0,3) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,3,1) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,3,2) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,3,3) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,1,3) = RG_TILE_BLOCK;
		RG_TILE_ATTR2(t,2,3) = RG_TILE_BLOCK;
	} else if (strcmp(ts->tmpl, "Icons (16x16)") == 0) {
		RG_TileScale(ts, t, 16, 16, flags);
		t->xOrig = 0;
		t->yOrig = 0;
		t->snap_mode = RG_SNAP_TO_GRID;
	} else if (strcmp(ts->tmpl, "Icons (32x32)") == 0) {
		RG_TileScale(ts, t, 32, 32, flags);
		t->xOrig = 0;
		t->yOrig = 0;
		t->snap_mode = RG_SNAP_TO_GRID;
	} else {
		RG_TileScale(ts, t, ins_tile_w, ins_tile_h, flags);
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
InsertTexture(AG_Event *event)
{
	AG_Window *dlgwin = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	RG_Tileset *ts = AG_PTR(3);
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
				if (!isdigit(*np)) {
					Strlcat(ins_texture_name, "_",
					    sizeof(ins_texture_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	tex = Malloc(sizeof(RG_Texture));
	RG_TextureInit(tex, ts, ins_texture_name);
	TAILQ_INSERT_TAIL(&ts->textures, tex, textures);
	
	ins_texture_name[0] = '\0';
	AG_ObjectDetach(dlgwin);
	
	if ((win = RG_TextureEdit(OBJECT(ts)->root, tex)) != NULL) {
		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
InsertAnim(AG_Event *event)
{
	AG_Window *pwin = AG_PTR(1);
	RG_Tileset *ts = AG_PTR(2);
	RG_Anim *ani;
	int flags = 0;

	if (ins_alpha)		flags |= RG_ANIM_SRCALPHA;
	if (ins_colorkey)	flags |= RG_ANIM_SRCCOLORKEY;

	if (ins_anim_name[0] == '\0') {
		Uint nameno = 0;
tryname1:
		Snprintf(ins_anim_name, sizeof(ins_anim_name),
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
					Snprintf(np, sizeof(ins_anim_name) -
					    (np-ins_anim_name)-1, "%d", num);
					break;
				}
				if (!isdigit(*np)) {
					Strlcat(ins_anim_name, "_",
					    sizeof(ins_anim_name));
					break;
				}
			}
			goto tryname2;
		}
	}

	ani = Malloc(sizeof(RG_Anim));
	RG_AnimInit(ani, ts, ins_anim_name, flags);
	RG_AnimScale(ani, ins_tile_w, ins_tile_h);
	if (InsertAnimMapping(ts, ani, &ani->main_id) == 0) {
		TAILQ_INSERT_TAIL(&ts->animations, ani, animations);
	} else {
		RG_AnimDestroy(ani);
		Free(ani);
		AG_TextMsg(AG_MSG_ERROR, _("Failed to create item: %s"),
		    AG_GetError());
	}
	ins_anim_name[0] = '\0';
	AG_ObjectDetach(pwin);
}

static void
InsertTileDlg(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
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
#if 0
	com = AG_ComboNew(win, AG_COMBO_ANY_TEXT|AG_COMBO_HFILL, _("Class: "));
	AG_TextboxBindUTF8(com->tbox, ins_tile_class, sizeof(ins_tile_class));
#endif
	msb = AG_MSpinbuttonNew(win, 0, "x", _("Size: "));
	AG_BindInt(msb, "xvalue", &ins_tile_w);
	AG_BindInt(msb, "yvalue", &ins_tile_h);
	AG_MSpinbuttonSetRange(msb, RG_TILE_SIZE_MIN, RG_TILE_SIZE_MAX);

	AG_CheckboxNewInt(win, 0, _("Alpha blending"), &ins_alpha);
	AG_CheckboxNewInt(win, 0, _("Colorkey"), &ins_colorkey);
	
	AG_LabelNewS(win, 0, _("Snapping mode: "));
	AG_RadioNewUint(win, AG_RADIO_HFILL, rgTileSnapModes, &ins_snap_mode);

	btnbox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(btnbox, 0, _("OK"), InsertTile, "%p,%p",win,ts);
		AG_ButtonNewFn(btnbox, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
InsertTextureDlg(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
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

	btnbox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(btnbox, 0, _("OK"), InsertTexture,
		    "%p,%p,%p", win, pwin, ts);
		AG_ButtonNewFn(btnbox, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
InsertAnimDlg(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	AG_Window *win;
	AG_Box *btnbox;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;

	if ((win = AG_WindowNewNamedS(AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	    AG_WINDOW_NOMINIMIZE, "rg-insanimdlg")) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Create new animation"));
	
	tb = AG_TextboxNew(win, 0, _("Name:"));
	AG_TextboxBindUTF8(tb, ins_anim_name, sizeof(ins_anim_name));
	AG_TextboxSizeHint(tb, "XXXXXXXXXXXXXXXXXXX");
	AG_SetEvent(tb, "textbox-return", InsertAnim, "%p,%p", win, ts);
	AG_WidgetFocus(tb);

	msb = AG_MSpinbuttonNew(win, 0, "x", _("Size:"));
	AG_BindInt(msb, "xvalue", &ins_tile_w);
	AG_BindInt(msb, "yvalue", &ins_tile_h);
	AG_MSpinbuttonSetRange(msb, RG_TILE_SIZE_MIN, RG_TILE_SIZE_MAX);

	AG_CheckboxNewInt(win, 0, _("Alpha blending"), &ins_alpha);
	AG_CheckboxNewInt(win, 0, _("Colorkey"), &ins_colorkey);

	btnbox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(btnbox, 0, _("OK"), InsertAnim, "%p,%p",win,ts);
		AG_ButtonNewFn(btnbox, 0, _("Cancel"), AGWINDETACH(win));
	}
	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
DeleteSelTiles(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	RG_Tileset *ts = AG_PTR(2);
	AG_TlistItem *it;

	AG_MutexLock(&ts->lock);
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
		RemoveTileMappings(ts, t);
		RG_TileDestroy(t);
		Free(t);
	}
	AG_MutexUnlock(&ts->lock);
}

static void
TileDup(RG_Tileset *ts, RG_Tile *t1)
{
	char name[RG_TILE_NAME_MAX];
	RG_Tile *t2;
	RG_TileElement *e1;
	int x, y;
	int ncopy = 0;

	t2 = Malloc(sizeof(RG_Tile));
tryname1:
	Snprintf(name, sizeof(name), _("Copy #%d of %s"), ncopy++, t1->name);
	if (RG_TilesetFindTile(ts, name) != NULL) {
		goto tryname1;
	}
	RG_TileInit(t2, ts, name);
	RG_TileScale(ts, t2, t1->su->w, t1->su->h, t1->flags);
	t2->su->format->alpha = t1->su->format->alpha;
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
			    _("Copy #%d of %s"), ncopy++, px1->name);
			if (RG_TilesetFindPixmap(ts, px2->name) != NULL) {
				goto tryname2;
			}
			RG_PixmapScale(px2, px1->su->w, px1->su->h, 0, 0);
			memcpy((Uint8 *)px2->su->pixels,
			    (Uint8 *)px1->su->pixels,
			    px1->su->h*px1->su->pitch +
			    px1->su->w*px1->su->format->BytesPerPixel);

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
DupSelTiles(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
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
EditSelTiles(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	AG_Window *pwin = AG_PTR(3);
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
EditSelAnims(AG_Event *event)
{
#ifdef AG_THREADS
	RG_Tileset *ts = AG_PTR(1);
#endif
	AG_Tlist *tl = AG_PTR(2);
	AG_Window *pwin = AG_PTR(3);
	AG_Window *win;
	AG_TlistItem *it;

	AG_MutexLock(&ts->lock);
	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected ||
		    strcmp(it->cat, "anim") != 0) {
			continue;
		}
		if ((win = RG_AnimEdit((RG_Anim *)it->p1))
		    != NULL) {
			AG_WindowAttach(pwin, win);
			AG_WindowShow(win);
		}
	}
	AG_MutexUnlock(&ts->lock);
}

static void
EditSelTextures(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	AG_Window *pwin = AG_PTR(3);
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
DeleteSelPixmaps(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tlGfx = AG_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlGfx)) != NULL) {
		RG_Pixmap *px = it->p1;
	
		if (px->nrefs > 0) {
			return;
		}
		TAILQ_REMOVE(&ts->pixmaps, px, pixmaps);
		RG_PixmapDestroy(px);
		Free(px);
	}
}

static void
DeleteSelAnims(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tlAnims = AG_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlAnims)) != NULL) {
		RG_Anim *ani = it->p1;
	
		if (ani->nrefs > 0) {
			AG_TextMsg(AG_MSG_ERROR,
			    _("The animation \"%s\" is currently in use."),
			    ani->name);
			return;
		}
		TAILQ_REMOVE(&ts->animations, ani, animations);
		RemoveAnimMappings(ts, ani);
		RG_AnimDestroy(ani);
		Free(ani);
	}
}

#if 0
static void
DeleteSelSketches(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tlGfx = AG_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlGfx)) != NULL) {
		RG_Sketch *sk = it->p1;
	
		if (sk->nrefs > 0) {
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
DeleteSelGraphics(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tlGfx = AG_PTR(2);
	AG_TlistItem *it;

	AG_TLIST_FOREACH(it, tlGfx) {
		if (strcmp(it->cat, "pixmap") == 0) {
			RG_Pixmap *px = it->p1;
	
			if (px->nrefs > 0) {
				continue;
			}
			TAILQ_REMOVE(&ts->pixmaps, px, pixmaps);
			RG_PixmapDestroy(px);
			Free(px);
#if 0
		} else if (strcmp(it->cat, "sketch") == 0) {
			RG_Sketch *sk = it->p1;
	
			if (sk->nrefs > 0) {
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
DeleteSelTextures(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tlTextures = AG_PTR(2);
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
DupSelPixmaps(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Tlist *tlGfx = AG_PTR(2);
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tlGfx)) != NULL) {
		RG_Pixmap *px1 = it->p1;
		RG_Pixmap *px2;
		int ncopy = 0;

		px2 = Malloc(sizeof(RG_Pixmap));
		RG_PixmapInit(px2, ts, 0);
tryname:
		Snprintf(px2->name, sizeof(px2->name), _("Copy #%d of %s"),
		    ncopy++, px1->name);
		if (RG_TilesetFindPixmap(ts, px2->name) != NULL)
			goto tryname;

		RG_PixmapScale(px2, px1->su->w, px1->su->h, 0, 0);
		TAILQ_INSERT_TAIL(&ts->pixmaps, px2, pixmaps);

		memcpy(
		    (Uint8 *)px2->su->pixels,
		    (Uint8 *)px1->su->pixels,
		    px1->su->h*px1->su->pitch +
		    px1->su->w*px1->su->format->BytesPerPixel);
	}
}

static void
SelectTemplate(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);

	AG_ObjectFreeDataset(ts);

	if (strcmp(ts->tmpl, "Sprite") == 0) {
		const char *tiles[] = {
			"Idle-N", "Idle-S",
			"Idle-W", "Idle-E"
		};
		const char *anims[] = {
			"Walk-N", "Walk-S",
			"Walk-W", "Walk-E"
		};
		const int ntiles = sizeof(tiles)/sizeof(tiles[0]);
		const int nanims = sizeof(anims)/sizeof(anims[0]);
		RG_Tile *t;
		RG_Anim *a;
		int i;

		for (i = 0; i < ntiles; i++) {
			t = Malloc(sizeof(RG_Tile));
			RG_TileInit(t, ts, tiles[i]);
			RG_TileScale(ts, t, 16, 32, RG_TILE_SRCCOLORKEY);
			TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
		}
		for (i = 0; i < nanims; i++) {
			a = Malloc(sizeof(RG_Anim));
			RG_AnimInit(a, ts, anims[i], RG_ANIM_SRCCOLORKEY);
			RG_AnimScale(a, 16, 32);
			TAILQ_INSERT_TAIL(&ts->animations, a, animations);
		}
	}
}

static void
PollTileTbl(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
	AG_TlistItem *it;
	Uint32 i;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	for (i = 0; i < ts->ntiletbl; i++) {
		if (ts->tiletbl[i] == NULL) {
			it = AG_TlistAdd(tl, NULL, "%u. NULL", (Uint)i);
			it->cat = "";
		} else {
			RG_Tile *t = ts->tiletbl[i];
			
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

static void
PollAnimTbl(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
	AG_TlistItem *it;
	Uint32 i;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	for (i = 0; i < ts->nanimtbl; i++) {
		if (ts->animtbl[i] == NULL) {
			it = AG_TlistAdd(tl, NULL, "%u. NULL", (Uint)i);
			it->cat = "";
		} else {
			RG_Anim *anim = ts->animtbl[i];
			
			it = AG_TlistAdd(tl, NULL, "%u. %s (%ux%u)", (Uint)i,
			    anim->name, anim->w, anim->h);
			it->cat = "animid";
			it->p1 = anim;
#if 0
			if (anim->nframes > 0)
				AG_TlistSetIcon(tl, it, anim->frames[0].su)
#endif
		}
	}
	AG_TlistRestore(tl);
	AG_MutexUnlock(&ts->lock);
}

static void *
Edit(void *p)
{
	RG_Tileset *ts = p;
	AG_Window *win;
	AG_Tlist *tlTiles, *tlGfx, *tlAnims, *tlTextures;
	AG_Tlist *tlTileTbl, *tlAnimTbl;
	AG_Box *bbox;
	AG_MenuItem *mi;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Combo *com;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, OBJECT(ts)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 1);
	AG_WindowSetSpacing(win, 0);
	AG_WindowSetPaddingTop(win, 0);

	tlTiles = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE|
	                            AG_TLIST_EXPAND);
	AG_TlistSizeHint(tlTiles, "XXXXXXXXXXXXXXXXXXXXXXXX (00x00)", 6);
	AG_SetEvent(tlTiles, "tlist-poll", PollTiles, "%p", ts);
	
	tlGfx = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_EXPAND);
	AG_SetEvent(tlGfx, "tlist-poll", PollGraphics, "%p", ts);
	tlTextures = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tlTextures, "tlist-poll", PollTextures, "%p", ts);
	tlAnims = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tlAnims, "tlist-poll", PollAnims, "%p", ts);
	tlTileTbl = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tlTileTbl, "tlist-poll", PollTileTbl, "%p", ts);
	tlAnimTbl = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tlAnimTbl, "tlist-poll", PollAnimTbl, "%p", ts);

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
	ntab = AG_NotebookAddTab(nb, _("Tiles"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tlTiles);
		AG_SetEvent(tlTiles, "tlist-dblclick",
		    EditSelTiles, "%p,%p,%p", ts, tlTiles, win);

		bbox = AG_BoxNewHoriz(ntab, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
		{
			AG_ButtonNewFn(bbox, 0, _("Insert"),
			    InsertTileDlg, "%p,%p", ts, win);
			AG_ButtonNewFn(bbox, 0, _("Edit"),
			    EditSelTiles, "%p,%p,%p", ts, tlTiles, win);
			AG_ButtonNewFn(bbox, 0, _("Delete"),
			    DeleteSelTiles, "%p,%p", tlTiles, ts);
		}
		com = AG_ComboNew(ntab, AG_COMBO_HFILL, _("Template: "));
		AG_TextboxBindUTF8(com->tbox, ts->tmpl, sizeof(ts->tmpl));
		AG_TlistAdd(com->list, NULL, "Icons (16x16)");
		AG_TlistAdd(com->list, NULL, "Icons (32x32)");
		AG_TlistAdd(com->list, NULL, "Sprite");
		AG_TlistAdd(com->list, NULL, "Terrain");
		AG_SetEvent(com, "combo-selected", SelectTemplate, "%p", ts);
	}

	ntab = AG_NotebookAddTab(nb, _("Graphics"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tlGfx);
	
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
		bbox = AG_BoxNewHoriz(ntab, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
		{
			AG_ButtonNewFn(bbox, 0,
			    _("Delete selected/unreferenced"),
			    DeleteSelGraphics, "%p,%p", ts, tlGfx);
		}
	}
	
	ntab = AG_NotebookAddTab(nb, _("Textures"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tlTextures);
	
		mi = AG_TlistSetPopup(tlTextures, "texture");
		{
			AG_MenuAction(mi, _("Delete texture"), agIconTrash.s,
			    DeleteSelTextures, "%p,%p", ts, tlTextures);
		}
		
		bbox = AG_BoxNewHoriz(ntab, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
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
	
	ntab = AG_NotebookAddTab(nb, _("Anims"), AG_BOX_VERT);
	{
		AG_ObjectAttach(ntab, tlAnims);
		AG_SetEvent(tlAnims, "tlist-dblclick",
		    EditSelAnims, "%p,%p,%p", ts, tlAnims, win);
		
		mi = AG_TlistSetPopup(tlAnims, "anim");
		{
			AG_MenuAction(mi, _("Edit animation"), rgIconEdit.s,
			    EditSelAnims, "%p,%p,%p", ts, tlAnims, win);
			AG_MenuAction(mi, _("Delete animation"), agIconTrash.s,
			    DeleteSelAnims, "%p,%p", ts, tlAnims);
		}
		
		bbox = AG_BoxNewHoriz(ntab, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
		{
			AG_ButtonNewFn(bbox, 0, _("Insert"),
			    InsertAnimDlg, "%p,%p", ts, win);
			AG_ButtonNewFn(bbox, 0, _("Edit"),
			    EditSelAnims, "%p,%p,%p", ts, tlAnims, win);
			AG_ButtonNewFn(bbox, 0, _("Delete"),
			    DeleteSelAnims, "%p,%p", ts, tlAnims);
		}
	}

	ntab = AG_NotebookAddTab(nb, _("Mappings"), AG_BOX_VERT);
	{
		AG_Pane *pane;

		pane = AG_PaneNew(ntab, AG_PANE_HORIZ, AG_PANE_EXPAND);
		AG_PaneMoveDividerPct(pane, 50);
		AG_PaneResizeAction(pane, AG_PANE_DIVIDE_EVEN);
		AG_LabelNewS(pane->div[0], 0, _("Tiles:"));
		AG_ObjectAttach(pane->div[0], tlTileTbl);
		AG_LabelNewS(pane->div[1], 0, _("Animations:"));
		AG_ObjectAttach(pane->div[1], tlAnimTbl);
	}
	return (win);
}

AG_ObjectClass rgTilesetClass = {
	"RG(Tileset)",
	sizeof(RG_Tileset),
	{ 8, 0 },
	Init,
	FreeDataset,
	Destroy,
	Load,
	Save,
	Edit
};

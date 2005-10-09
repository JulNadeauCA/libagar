/*	$Csoft: gfx.c,v 1.58 2005/09/07 03:56:57 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/config.h>
#include <engine/view.h>

#include <engine/map/map.h>
#include <engine/map/mapedit.h>

#include <engine/loader/den.h>
#include <engine/loader/xcf.h>
#include <engine/loader/surface.h>

#ifdef DEBUG
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#endif

#include <string.h>
#include <stdarg.h>

enum {
	FRAMES_INIT =	2,
	FRAMES_GROW =	8,
	NSUBMAPS_INIT =	1,
	NSUBMAPS_GROW =	4
};

const char *agGfxSnapNames[] = {
	N_("Free positioning"),
	N_("Snap to grid"),
	NULL
};

void
AG_SpriteInit(AG_Gfx *gfx, Uint32 s)
{
	AG_Sprite *spr = &gfx->sprites[s];

	spr->name[0] = '\0';
	spr->pgfx = gfx;
	spr->index = s;
	spr->su = NULL;
	spr->attrs = NULL;
	spr->layers = NULL;
	spr->xOrig = 0;
	spr->yOrig = 0;
	spr->snap_mode = AG_GFX_SNAP_TO_GRID;
	SLIST_INIT(&spr->csprites);

#ifdef HAVE_OPENGL
	spr->texture = 0;
	spr->texcoords[0] = 0.0f;
	spr->texcoords[1] = 0.0f;
	spr->texcoords[2] = 0.0f;
	spr->texcoords[3] = 0.0f;
#endif
}

u_int
AG_SpriteGetWtiles(AG_Sprite *spr)
{
	int w;

	if (spr->su == NULL) {
		return (0);
	}
	w = spr->su->w/AGTILESZ;
	if (w%AGTILESZ > 0) { w++; }
	return (w);
}

void
AG_SpriteGetNodeAttrs(AG_Sprite *spr, u_int *w, u_int *h)
{
	if (spr->su != NULL) {
		*w = spr->su->w/AGTILESZ;
		*h = spr->su->h/AGTILESZ;
		if ((*w)%AGTILESZ > 0) (*w)++;
		if ((*h)%AGTILESZ > 0) (*h)++;
	} else {
		*w = 0;
		*h = 0;
	}
}

static __inline__ void
sprite_free_transforms(AG_Sprite *spr)
{
	AG_CachedSprite *csprite, *ncsprite;
	AG_Transform *trans, *ntrans;

	for (csprite = SLIST_FIRST(&spr->csprites);
	     csprite != SLIST_END(&spr->csprites);
	     csprite = ncsprite) {
		ncsprite = SLIST_NEXT(csprite, sprites);
		for (trans = TAILQ_FIRST(&csprite->transforms);
		     trans != TAILQ_END(&csprite->transforms);
		     trans = ntrans) {
			ntrans = TAILQ_NEXT(trans, transforms);
			AG_TransformDestroy(trans);
		}
		SDL_FreeSurface(csprite->su);
		Free(csprite, M_GFX);
	}
	SLIST_INIT(&spr->csprites);
}

int
AG_SpriteFind(AG_Gfx *gfx, const char *name, Uint32 *offs)
{
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++) {
		AG_Sprite *spr = &gfx->sprites[i];

		if (strcmp(spr->name, name) == 0) {
			*offs = i;
			return (1);
		}
	}
	AG_SetError(_("No such sprite: \"%s\""), name);
	return (0);
}

void
AG_SpriteDestroy(AG_Gfx *gfx, Uint32 s)
{
	AG_Sprite *spr = &gfx->sprites[s];

	if (spr->su != NULL) {
		SDL_FreeSurface(spr->su);
		spr->su = NULL;
	}
	if (spr->attrs != NULL) {
		Free(spr->attrs, M_RG);
		spr->attrs = NULL;
	}
	if (spr->layers != NULL) {
		Free(spr->layers, M_RG);
		spr->layers = NULL;
	}
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		if (spr->texture != 0) {
			glDeleteTextures(1, &spr->texture);
			spr->texture = 0;
		}
		spr->texcoords[0] = 0.0f;
		spr->texcoords[1] = 0.0f;
		spr->texcoords[2] = 0.0f;
		spr->texcoords[3] = 0.0f;
	}
#endif
	sprite_free_transforms(spr);
}

void
AG_SpriteSetName(AG_Gfx *gfx, Uint32 s, const char *name)
{
	AG_Sprite *spr = &gfx->sprites[s];

	strlcpy(spr->name, name, sizeof(spr->name));
}

void
AG_SpriteSetClass(AG_Gfx *gfx, Uint32 s, const char *name)
{
	AG_Sprite *spr = &gfx->sprites[s];

	strlcpy(spr->clname, name, sizeof(spr->clname));
}

/*
 * Associate a different surface to a sprite; update the texture and
 * destroy the transform cache. The previous surface is freed if any.
 */
void
AG_SpriteSetSurface(AG_Gfx *gfx, Uint32 s, SDL_Surface *su)
{
	AG_Sprite *spr = &gfx->sprites[s];

	AG_SpriteDestroy(gfx, s);
	spr->su = su;
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		spr->texture = (su != NULL) ?
		               AG_SurfaceTexture(su, &spr->texcoords[0]) : 0;
	}
#endif
}

/* Set the snapping mode of a sprite entry. */
void
AG_SpriteSetSnapMode(AG_Sprite *spr, enum ag_gfx_snap_mode snap_mode)
{
	spr->snap_mode = snap_mode;
}

/* Set the origin point of a sprite. */
void
AG_SpriteSetOrigin(AG_Sprite *spr, int x, int y)
{
	spr->xOrig = x;
	spr->yOrig = y;
}

/* Flush cached transforms and regenerate the texture of a sprite. */
void
AG_SpriteUpdate(AG_Sprite *spr)
{
	sprite_free_transforms(spr);
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		if (spr->texture != 0) {
			glDeleteTextures(1, &spr->texture);
		}
		spr->texture = (spr->su != NULL) ?
		    AG_SurfaceTexture(spr->su, &spr->texcoords[0]) : 0;
	}
#endif
}

/* Allocate space for n new sprites and initialize them. */
void
AG_GfxAllocSprites(AG_Gfx *gfx, Uint32 n)
{
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++)
		AG_SpriteDestroy(gfx, i);

	if (n > 0) {
		gfx->sprites = Realloc(gfx->sprites, n*sizeof(AG_Sprite));
	} else {
		Free(gfx->sprites, M_GFX);
		gfx->sprites = NULL;
	}
	gfx->nsprites = n;

	for (i = 0; i < n; i++)
		AG_SpriteInit(gfx, i);
}

/* Allocate space for n new animations and initialize them. */
void
AG_GfxAllocAnims(AG_Gfx *gfx, Uint32 n)
{
	Uint32 i;

	for (i = 0; i < gfx->nanims; i++)
		AG_AnimDestroy(gfx, i);

	if (n > 0) {
		gfx->anims = Realloc(gfx->anims, n*sizeof(AG_Anim));
	} else {
		Free(gfx->anims, M_GFX);
		gfx->anims = NULL;
	}
	gfx->nanims = n;

	for (i = 0; i < n; i++)
		AG_AnimInit(gfx, i);
}

/* Allocate and initialize a new sprite at the end of the array. */
Uint32
AG_GfxAddSprite(AG_Gfx *gfx, SDL_Surface *su)
{
	AG_Sprite *spr;
	
	gfx->sprites = Realloc(gfx->sprites, (gfx->nsprites+1) *
	                                     sizeof(AG_Sprite));
	AG_SpriteInit(gfx, gfx->nsprites);
	AG_SpriteSetSurface(gfx, gfx->nsprites, su);
	return (gfx->nsprites++);
}

/*
 * Scan for pixels with a non-opaque alpha channel on a surface and
 * return 1 if there are any.
 */
int
AG_HasTransparency(SDL_Surface *su)
{
	int x, y;
	int rv = 0;
	Uint8 *pSrc;

	if (su->format->Amask == 0x0)
		return (0);

	if (SDL_MUSTLOCK(su)) {
		SDL_LockSurface(su);
	}
	pSrc = (Uint8 *)su->pixels;

	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint8 r, g, b, a;

			SDL_GetRGBA(AG_GET_PIXEL(su, pSrc), su->format,
			    &r, &g, &b, &a);

			if (a != SDL_ALPHA_OPAQUE) {
				rv = 1;
				goto out;
			}

			pSrc += su->format->BytesPerPixel;
		}
	}
out:
	if (SDL_MUSTLOCK(su)) {
		SDL_UnlockSurface(su);
	}
	return (rv);
}

/* Break a surface into tile-sized fragments and generate a map. */
AG_Map *
AG_GfxAddFragments(AG_Gfx *gfx, SDL_Surface *sprite)
{
	char mapname[AG_OBJECT_NAME_MAX];
	int x, y, mx, my;
	u_int mw, mh;
	SDL_Rect sd, rd;
	AG_Map *fragmap;

	sd.w = AGTILESZ;
	sd.h = AGTILESZ;
	rd.x = 0;
	rd.y = 0;
	mw = sprite->w/AGTILESZ + 1;
	mh = sprite->h/AGTILESZ + 1;

	fragmap = Malloc(sizeof(AG_Map), M_OBJECT);
	snprintf(mapname, sizeof(mapname), "f%u", gfx->nsubmaps);
	AG_MapInit(fragmap, mapname);
	if (AG_MapAllocNodes(fragmap, mw, mh) == -1)
		fatal("%s", AG_GetError());

	for (y = 0, my = 0; y < sprite->h; y += AGTILESZ, my++) {
		for (x = 0, mx = 0; x < sprite->w; x += AGTILESZ, mx++) {
			SDL_Surface *su;
			Uint32 saflags = sprite->flags & (SDL_SRCALPHA|
			                                  SDL_RLEACCEL);
			Uint32 sckflags = sprite->flags & (SDL_SRCCOLORKEY|
			                                   SDL_RLEACCEL);
			Uint8 salpha = sprite->format->alpha;
			Uint32 scolorkey = sprite->format->colorkey;
			AG_Node *node = &fragmap->map[my][mx];
			Uint32 nsprite;
			int fw = AGTILESZ;
			int fh = AGTILESZ;

			if (sprite->w - x < AGTILESZ)
				fw = sprite->w - x;
			if (sprite->h - y < AGTILESZ)
				fh = sprite->h - y;

			/* Allocate a surface for the fragment. */
			su = SDL_CreateRGBSurface(SDL_SWSURFACE |
			    (sprite->flags &
			    (SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
			    fw, fh, sprite->format->BitsPerPixel,
			    sprite->format->Rmask,
			    sprite->format->Gmask,
			    sprite->format->Bmask,
			    sprite->format->Amask);
			if (su == NULL)
				fatal("SDL_CreateRGBSurface: %s",
				    SDL_GetError());
			
			/* Copy the fragment as-is. */
			SDL_SetAlpha(sprite, 0, 0);
			SDL_SetColorKey(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(sprite, &sd, su, &rd);
			nsprite = AG_GfxAddSprite(gfx, su);
			SDL_SetAlpha(sprite, saflags, salpha);
			SDL_SetColorKey(sprite, sckflags, scolorkey);

			/*
			 * Enable alpha blending if there are any pixels
			 * with a non-opaque alpha channel on the surface.
			 */
			if (AG_HasTransparency(su))
				SDL_SetAlpha(su, SDL_SRCALPHA,
				    su->format->alpha);

			/* Map the sprite as a NULL reference. */
			AG_NodeAddSprite(fragmap, node, NULL, nsprite);
		}
	}

	AG_GfxAddSubmap(gfx, fragmap);
	return (fragmap);
}

/* Allocate a gfx structure for a given object. */
AG_Gfx *
AG_GfxNew(void *pobj)
{
	AG_Gfx *gfx;
	
	gfx = Malloc(sizeof(AG_Gfx), M_GFX);
	AG_GfxInit(gfx);
	gfx->pobj = pobj;
	return (gfx);
}

void
AG_GfxInit(AG_Gfx *gfx)
{
	gfx->pobj = NULL;
	gfx->sprites = NULL;
	gfx->nsprites = 0;
	gfx->anims = NULL;
	gfx->canims = NULL;
	gfx->nanims = 0;
	gfx->submaps = NULL;
	gfx->nsubmaps = 0;
	gfx->maxsubmaps = 0;
	gfx->used = 0;
}

static void
destroy_anim(AG_Anim *anim)
{
	Uint32 i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	Free(anim->frames, M_GFX);
}

void
AG_AnimDestroy(AG_Gfx *gfx, Uint32 name)
{
	AG_AnimCache *animcl = &gfx->canims[name];
	AG_CachedAnim *canim, *ncanim;
	AG_Transform *trans, *ntrans;

	for (canim = SLIST_FIRST(&animcl->anims);
	     canim != SLIST_END(&animcl->anims);
	     canim = ncanim) {
		ncanim = SLIST_NEXT(canim, anims);
		for (trans = TAILQ_FIRST(&canim->transforms);
		     trans != TAILQ_END(&canim->transforms);
		     trans = ntrans) {
			ntrans = TAILQ_NEXT(trans, transforms);
			AG_TransformDestroy(trans);
		}
		destroy_anim(canim->anim);
		Free(canim, M_GFX);
	}
	SLIST_INIT(&animcl->anims);

	destroy_anim(&gfx->anims[name]);
}

/* Release a graphics package that is no longer in use. */
void
AG_GfxDestroy(AG_Gfx *gfx)
{
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++) {
		AG_SpriteDestroy(gfx, i);
	}
	for (i = 0; i < gfx->nanims; i++) {
		AG_AnimDestroy(gfx, i);
	}
	for (i = 0; i < gfx->nsubmaps; i++) {
		AG_ObjectDestroy(gfx->submaps[i]);
		Free(gfx->submaps[i], M_OBJECT);
	}

	Free(gfx->sprites, M_GFX);
	Free(gfx->anims, M_GFX);
	Free(gfx->canims, M_GFX);
	Free(gfx->submaps, M_GFX);
	Free(gfx, M_GFX);
}

/* Insert a frame into an animation. */
Uint32
AG_GfxAddAnimFrame(AG_Anim *anim, SDL_Surface *surface)
{
	if (anim->frames == NULL) {
		anim->frames = Malloc(FRAMES_INIT*sizeof(SDL_Surface *), M_GFX);
		anim->maxframes = FRAMES_INIT;
		anim->nframes = 0;
	} else if (anim->nframes+1 > anim->maxframes) {
		anim->maxframes += FRAMES_GROW;
		anim->frames = Realloc(anim->frames,
		    anim->maxframes*sizeof(SDL_Surface *));
	}
	anim->frames[anim->nframes++] = surface;
	return (anim->nframes);
}

/* Allocate a new submap. */
Uint32
AG_GfxAddSubmap(AG_Gfx *gfx, AG_Map *m)
{
	if (gfx->submaps == NULL) {
		gfx->maxsubmaps = NSUBMAPS_INIT;
		gfx->submaps = Malloc(gfx->maxsubmaps*sizeof(AG_Map *),
		    M_GFX);
		gfx->nsubmaps = 0;
	} else if (gfx->nsubmaps+1 > gfx->maxsubmaps) {
		gfx->maxsubmaps += NSUBMAPS_GROW;
		gfx->submaps = Realloc(gfx->submaps,
		    gfx->maxsubmaps*sizeof(AG_Map *));
	}
	gfx->submaps[gfx->nsubmaps] = m;
	return (gfx->nsubmaps++);
}

void
AG_AnimInit(AG_Gfx *gfx, Uint32 i)
{
	AG_Anim *anim = &gfx->anims[i];
	AG_AnimCache *animcl = &gfx->canims[i];

	anim->frames = NULL;
	anim->maxframes = 0;
	anim->frame = 0;
	anim->nframes = 0;
	SLIST_INIT(&animcl->anims);
}

/* Allocate a new animation. */
Uint32
AG_GfxAddAnim(AG_Gfx *gfx)
{
	gfx->anims = Realloc(gfx->anims, (gfx->nanims+1) *  sizeof(AG_Anim));
	gfx->canims = Realloc(gfx->canims, (gfx->nanims+1) *
                                           sizeof(AG_AnimCache));
	AG_AnimInit(gfx, gfx->nanims);
	return (gfx->nanims++);
}

/* Load static graphics from a den archive. Used for widgets and such. */
int
AG_WireGfx(void *p, const char *name)
{
	char path[MAXPATHLEN];
	AG_Object *ob = p;
	AG_Den *den;
	Uint32 i;

	if (AG_ConfigFile("den-path", name, "den", path, sizeof(path)) == -1 ||
	    (den = AG_DenOpen(path, AG_DEN_READ)) == NULL)
		return (-1);
	
	ob->gfx = AG_GfxNew(ob);
	ob->gfx->used = AG_GFX_MAX_USED;
	for (i = 0; i < den->nmembers; i++) {
		if (AG_XCFLoad(den->buf, den->members[i].offs, ob->gfx) == -1) {
			AG_DenClose(den);
			AG_GfxDestroy(ob->gfx);
			ob->gfx = NULL;
			return (-1);
		}
	}
	AG_DenClose(den);
	return (0);
}

void
AG_GfxUsed(void *p)
{
	AG_Object *ob = p;

	if (ob->gfx != NULL && ob->gfx->used != AG_GFX_MAX_USED)
		ob->gfx->used++;
}

int
AG_GfxUnused(void *p)
{
	AG_Object *ob = p;
	
	if (ob->gfx != NULL && --ob->gfx->used == 0) {
		AG_GfxAllocSprites(ob->gfx, 0);
		AG_GfxAllocAnims(ob->gfx, 0);
		return (1);
	}
	return (0);
}

int
AG_GfxLoad(AG_Object *ob)
{
	extern const AG_Version ag_object_ver;
	AG_Gfx *gfx = ob->gfx;
	char path[MAXPATHLEN];
	AG_Netbuf *buf;
	off_t gfx_offs;
	Uint32 i, j;
	
	if (AG_ObjectCopyFilename(ob, path, sizeof(path)) == -1) {
		return (-1);
	}
	if ((buf = AG_NetbufOpen(path, "rb", AG_NETBUF_BIG_ENDIAN)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		return (-1);
	}
	
	if (AG_ReadVersion(buf, &ag_object_ver, NULL) == -1)
		goto fail;

	AG_ReadUint32(buf);				/* Skip data offs */
	gfx_offs = (off_t)AG_ReadUint32(buf);
	AG_NetbufSeek(buf, gfx_offs, SEEK_SET);

	if (AG_ReadUint8(buf) == 0)
		goto out;

	AG_ReadUint32(buf);				/* Pad: flags */

	AG_GfxAllocSprites(gfx, AG_ReadUint32(buf));
	dprintf("%s: %d sprites\n", ob->name, gfx->nsprites);
	for (i = 0; i < gfx->nsprites; i++) {
		AG_Sprite *spr = &gfx->sprites[i];

		AG_CopyString(spr->name, buf, sizeof(spr->name));
		if (AG_ReadUint8(buf)) {
			spr->su = AG_ReadSurface(buf, agSurfaceFmt);
		} else {
			spr->su = NULL;
		}
		spr->xOrig = (int)AG_ReadSint32(buf);
		spr->yOrig = (int)AG_ReadSint32(buf);
		spr->snap_mode = (enum ag_gfx_snap_mode)AG_ReadUint8(buf);

		if (AG_ReadUint8(buf)) {
			u_int nw, nh;
			int x, y;

			AG_SpriteGetNodeAttrs(spr, &nw, &nh);
			dprintf("%s: %d,%d attributes\n", ob->name, nw, nh);
			spr->attrs = Realloc(spr->attrs, nw*nh*sizeof(u_int));
			spr->layers = Realloc(spr->layers, nw*nh*sizeof(int));
			for (y = 0; y < nh; y++) {
				for (x = 0; x < nw; x++) {
					spr->attrs[y*nw + x] =
					    (u_int)AG_ReadUint32(buf);
					spr->layers[y*nw + x] =
					    (int)AG_ReadSint32(buf);
				}
			}
		} else {
			Free(spr->attrs, M_RG);
			Free(spr->layers, M_RG);
			spr->layers = NULL;
		}
	}

	AG_GfxAllocAnims(gfx, AG_ReadUint32(buf));
	dprintf("%s: %d anims\n", ob->name, gfx->nanims);
	for (i = 0; i < gfx->nanims; i++) {
		AG_Anim *anim = &gfx->anims[i];

		anim->frame = AG_ReadUint32(buf);
		anim->nframes = AG_ReadUint32(buf);
		anim->frames = Realloc(anim->frames, anim->nframes *
				                     sizeof(SDL_Surface *));
		for (j = 0; j < anim->nframes; j++)
			anim->frames[j] = AG_ReadSurface(buf, agVideoFmt);
	}

out:
	AG_NetbufClose(buf);
	return (0);
fail:
	AG_NetbufClose(buf);
	return (-1);
}

int
AG_GfxSave(AG_Object *ob, AG_Netbuf *buf)
{
	AG_Gfx *gfx = ob->gfx;
	Uint32 i, j;

	if (gfx == NULL) {
		AG_WriteUint8(buf, 0);
		return (0);
	} else {
		AG_WriteUint8(buf, 1);
	}

	AG_WriteUint32(buf, 0);				/* Pad: flags */

	dprintf("%s: saving %d sprites\n", ob->name, gfx->nsprites);
	AG_WriteUint32(buf, gfx->nsprites);
	for (i = 0; i < gfx->nsprites; i++) {
		AG_Sprite *spr = &gfx->sprites[i];

		AG_WriteString(buf, spr->name);
		if (spr->su != NULL) {
			AG_WriteUint8(buf, 1);
			AG_WriteSurface(buf, spr->su);
		} else {
			AG_WriteUint8(buf, 0);
		}
		AG_WriteSint32(buf, (Sint32)spr->xOrig);
		AG_WriteSint32(buf, (Sint32)spr->yOrig);
		AG_WriteUint8(buf, (Uint8)spr->snap_mode);

		if (spr->attrs != NULL && spr->layers != NULL) {
			int x, y;
			u_int nw, nh;

			AG_WriteUint8(buf, 1);
			AG_SpriteGetNodeAttrs(spr, &nw, &nh);
			for (y = 0; y < nh; y++) {
				for (x = 0; x < nw; x++) {
					AG_WriteUint32(buf,
					    (Uint32)spr->attrs[y*nw + x]);
					AG_WriteSint32(buf,
					    (Sint32)spr->layers[y*nw + x]);
				}
			}
		} else {
			AG_WriteUint8(buf, 0);
		}
	}

	dprintf("%s: saving %d anims\n", ob->name, gfx->nsprites);
	AG_WriteUint32(buf, gfx->nanims);
	for (i = 0; i < gfx->nanims; i++) {
		AG_Anim *anim = &gfx->anims[i];

		AG_WriteUint32(buf, anim->frame);
		AG_WriteUint32(buf, anim->nframes);
		for (j = 0; j < anim->nframes; j++)
			AG_WriteSurface(buf, anim->frames[j]);
	}
	return (0);
}

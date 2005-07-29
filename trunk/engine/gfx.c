/*	$Csoft: gfx.c,v 1.52 2005/07/27 06:34:43 vedge Exp $	*/

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

const char *gfx_snap_names[] = {
	N_("Free positioning"),
	N_("Snap to grid"),
	NULL
};

void
sprite_init(struct gfx *gfx, Uint32 s)
{
	struct sprite *spr = &gfx->sprites[s];

	spr->name[0] = '\0';
	spr->pgfx = gfx;
	spr->index = s;
	spr->su = NULL;
	spr->attrs = NULL;
	spr->layers = NULL;
	spr->xOrig = 0;
	spr->yOrig = 0;
	spr->snap_mode = GFX_SNAP_NOT;
	SLIST_INIT(&spr->csprites);

#ifdef HAVE_OPENGL
	spr->texture = 0;
	spr->texcoords[0] = 0.0f;
	spr->texcoords[1] = 0.0f;
	spr->texcoords[2] = 0.0f;
	spr->texcoords[3] = 0.0f;
#endif
}

void
sprite_get_nattrs(struct sprite *spr, u_int *w, u_int *h)
{
	if (spr->su != NULL) {
		*w = spr->su->w/TILESZ;
		*h = spr->su->h/TILESZ;
		if ((*w)%TILESZ > 0) (*w)++;
		if ((*h)%TILESZ > 0) (*h)++;
	} else {
		*w = 0;
		*h = 0;
	}
}

static __inline__ void
sprite_free_transforms(struct sprite *spr)
{
	struct gfx_cached_sprite *csprite, *ncsprite;
	struct transform *trans, *ntrans;

	for (csprite = SLIST_FIRST(&spr->csprites);
	     csprite != SLIST_END(&spr->csprites);
	     csprite = ncsprite) {
		ncsprite = SLIST_NEXT(csprite, sprites);
		for (trans = TAILQ_FIRST(&csprite->transforms);
		     trans != TAILQ_END(&csprite->transforms);
		     trans = ntrans) {
			ntrans = TAILQ_NEXT(trans, transforms);
			transform_destroy(trans);
		}
		SDL_FreeSurface(csprite->su);
		Free(csprite, M_GFX);
	}
	SLIST_INIT(&spr->csprites);
}

void
sprite_destroy(struct gfx *gfx, Uint32 s)
{
	struct sprite *spr = &gfx->sprites[s];

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
	if (view->opengl) {
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
sprite_set_name(struct gfx *gfx, Uint32 s, const char *name)
{
	struct sprite *spr = &gfx->sprites[s];

	strlcpy(spr->name, name, sizeof(spr->name));
}

/*
 * Associate a different surface to a sprite; update the texture and
 * destroy the transform cache. The previous surface is freed if any.
 */
void
sprite_set_surface(struct gfx *gfx, Uint32 s, SDL_Surface *su)
{
	struct sprite *spr = &gfx->sprites[s];

	sprite_destroy(gfx, s);
	spr->su = su;
#ifdef HAVE_OPENGL
	if (view->opengl) {
		spr->texture = (su != NULL) ?
		               view_surface_texture(su, &spr->texcoords[0]) : 0;
	}
#endif
}

/* Set the snapping mode of a sprite entry. */
void
sprite_set_snap_mode(struct sprite *spr, enum gfx_snap_mode snap_mode)
{
	spr->snap_mode = snap_mode;
}

/* Set the origin point of a sprite. */
void
sprite_set_origin(struct sprite *spr, int x, int y)
{
	spr->xOrig = x;
	spr->yOrig = y;
}

/* Flush cached transforms and regenerate the texture of a sprite. */
void
sprite_update(struct sprite *spr)
{
	sprite_free_transforms(spr);
#ifdef HAVE_OPENGL
	if (view->opengl) {
		if (spr->texture != 0) {
			glDeleteTextures(1, &spr->texture);
		}
		spr->texture = (spr->su != NULL) ?
		    view_surface_texture(spr->su, &spr->texcoords[0]) : 0;
	}
#endif
}

/* Allocate space for n new sprites and initialize them. */
void
gfx_alloc_sprites(struct gfx *gfx, Uint32 n)
{
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++)
		sprite_destroy(gfx, i);

	if (n > 0) {
		gfx->sprites = Realloc(gfx->sprites, n*sizeof(struct sprite));
	} else {
		Free(gfx->sprites, M_GFX);
		gfx->sprites = NULL;
	}
	gfx->nsprites = n;

	for (i = 0; i < n; i++)
		sprite_init(gfx, i);
}

/* Allocate space for n new animations and initialize them. */
void
gfx_alloc_anims(struct gfx *gfx, Uint32 n)
{
	Uint32 i;

	for (i = 0; i < gfx->nanims; i++)
		anim_destroy(gfx, i);

	if (n > 0) {
		gfx->anims = Realloc(gfx->anims, n*sizeof(struct gfx_anim));
	} else {
		Free(gfx->anims, M_GFX);
		gfx->anims = NULL;
	}
	gfx->nanims = n;

	for (i = 0; i < n; i++)
		anim_init(gfx, i);
}

/* Allocate and initialize a new sprite at the end of the array. */
Uint32
gfx_insert_sprite(struct gfx *gfx, SDL_Surface *su)
{
	struct sprite *spr;
	
	gfx->sprites = Realloc(gfx->sprites, (gfx->nsprites+1) *
	                                     sizeof(struct sprite));
	sprite_init(gfx, gfx->nsprites);
	sprite_set_surface(gfx, gfx->nsprites, su);
	return (gfx->nsprites++);
}

/*
 * Scan for pixels with a non-opaque alpha channel on a surface and
 * return 1 if there are any.
 */
int
gfx_transparent(SDL_Surface *su)
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

			SDL_GetRGBA(GET_PIXEL(su, pSrc), su->format,
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
struct map *
gfx_insert_fragments(struct gfx *gfx, SDL_Surface *sprite)
{
	char mapname[OBJECT_NAME_MAX];
	int x, y, mx, my;
	u_int mw, mh;
	SDL_Rect sd, rd;
	struct map *fragmap;

	sd.w = TILESZ;
	sd.h = TILESZ;
	rd.x = 0;
	rd.y = 0;
	mw = sprite->w/TILESZ + 1;
	mh = sprite->h/TILESZ + 1;

	fragmap = Malloc(sizeof(struct map), M_OBJECT);
	snprintf(mapname, sizeof(mapname), "f%u", gfx->nsubmaps);
	map_init(fragmap, mapname);
	if (map_alloc_nodes(fragmap, mw, mh) == -1)
		fatal("%s", error_get());

	for (y = 0, my = 0; y < sprite->h; y += TILESZ, my++) {
		for (x = 0, mx = 0; x < sprite->w; x += TILESZ, mx++) {
			SDL_Surface *su;
			Uint32 saflags = sprite->flags & (SDL_SRCALPHA|
			                                  SDL_RLEACCEL);
			Uint32 sckflags = sprite->flags & (SDL_SRCCOLORKEY|
			                                   SDL_RLEACCEL);
			Uint8 salpha = sprite->format->alpha;
			Uint32 scolorkey = sprite->format->colorkey;
			struct node *node = &fragmap->map[my][mx];
			Uint32 nsprite;
			int fw = TILESZ;
			int fh = TILESZ;

			if (sprite->w - x < TILESZ)
				fw = sprite->w - x;
			if (sprite->h - y < TILESZ)
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
			nsprite = gfx_insert_sprite(gfx, su);
			SDL_SetAlpha(sprite, saflags, salpha);
			SDL_SetColorKey(sprite, sckflags, scolorkey);

			/*
			 * Enable alpha blending if there are any pixels
			 * with a non-opaque alpha channel on the surface.
			 */
			if (gfx_transparent(su))
				SDL_SetAlpha(su, SDL_SRCALPHA,
				    su->format->alpha);

			/* Map the sprite as a NULL reference. */
			node_add_sprite(fragmap, node, NULL, nsprite);
		}
	}

	gfx_insert_submap(gfx, fragmap);
	return (fragmap);
}

/* Allocate a gfx structure for a given object. */
struct gfx *
gfx_new(void *pobj)
{
	struct gfx *gfx;
	
	gfx = Malloc(sizeof(struct gfx), M_GFX);
	gfx_init(gfx);
	gfx->pobj = pobj;
	return (gfx);
}

void
gfx_init(struct gfx *gfx)
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
destroy_anim(struct gfx_anim *anim)
{
	Uint32 i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	Free(anim->frames, M_GFX);
}

void
anim_destroy(struct gfx *gfx, Uint32 name)
{
	struct gfx_animcl *animcl = &gfx->canims[name];
	struct gfx_cached_anim *canim, *ncanim;
	struct transform *trans, *ntrans;

	for (canim = SLIST_FIRST(&animcl->anims);
	     canim != SLIST_END(&animcl->anims);
	     canim = ncanim) {
		ncanim = SLIST_NEXT(canim, anims);
		for (trans = TAILQ_FIRST(&canim->transforms);
		     trans != TAILQ_END(&canim->transforms);
		     trans = ntrans) {
			ntrans = TAILQ_NEXT(trans, transforms);
			transform_destroy(trans);
		}
		destroy_anim(canim->anim);
		Free(canim, M_GFX);
	}
	SLIST_INIT(&animcl->anims);

	destroy_anim(&gfx->anims[name]);
}

/* Release a graphics package that is no longer in use. */
void
gfx_destroy(struct gfx *gfx)
{
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++) {
		sprite_destroy(gfx, i);
	}
	for (i = 0; i < gfx->nanims; i++) {
		anim_destroy(gfx, i);
	}
	for (i = 0; i < gfx->nsubmaps; i++) {
		object_destroy(gfx->submaps[i]);
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
gfx_insert_anim_frame(struct gfx_anim *anim, SDL_Surface *surface)
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
gfx_insert_submap(struct gfx *gfx, struct map *m)
{
	if (gfx->submaps == NULL) {
		gfx->maxsubmaps = NSUBMAPS_INIT;
		gfx->submaps = Malloc(gfx->maxsubmaps*sizeof(struct map *),
		    M_GFX);
		gfx->nsubmaps = 0;
	} else if (gfx->nsubmaps+1 > gfx->maxsubmaps) {
		gfx->maxsubmaps += NSUBMAPS_GROW;
		gfx->submaps = Realloc(gfx->submaps,
		    gfx->maxsubmaps*sizeof(struct map *));
	}
	gfx->submaps[gfx->nsubmaps] = m;
	return (gfx->nsubmaps++);
}

void
anim_init(struct gfx *gfx, Uint32 i)
{
	struct gfx_anim *anim = &gfx->anims[i];
	struct gfx_animcl *animcl = &gfx->canims[i];

	anim->frames = NULL;
	anim->maxframes = 0;
	anim->frame = 0;
	anim->nframes = 0;
	SLIST_INIT(&animcl->anims);
}

/* Allocate a new animation. */
Uint32
gfx_insert_anim(struct gfx *gfx)
{
	gfx->anims = Realloc(gfx->anims, (gfx->nanims+1) * 
	                                 sizeof(struct gfx_anim));
	gfx->canims = Realloc(gfx->canims, (gfx->nanims+1) *
                                           sizeof(struct gfx_animcl));
	anim_init(gfx, gfx->nanims);
	return (gfx->nanims++);
}

/* Load static graphics from a den archive. Used for widgets and such. */
int
gfx_wire(void *p, const char *name)
{
	char path[MAXPATHLEN];
	struct object *ob = p;
	struct den *den;
	Uint32 i;

	if (config_search_file("den-path", name, "den", path, sizeof(path))
	    == -1 ||
	    (den = den_open(path, DEN_READ)) == NULL)
		return (-1);
	
	ob->gfx = gfx_new(ob);
	ob->gfx->used = GFX_MAX_USED;
	for (i = 0; i < den->nmembers; i++) {
		if (xcf_load(den->buf, den->members[i].offs, ob->gfx) == -1) {
			den_close(den);
			gfx_destroy(ob->gfx);
			ob->gfx = NULL;
			return (-1);
		}
	}
	den_close(den);
	return (0);
}

void
gfx_used(void *p)
{
	struct object *ob = p;

	if (ob->gfx != NULL && ob->gfx->used != GFX_MAX_USED)
		ob->gfx->used++;
}

int
gfx_unused(void *p)
{
	struct object *ob = p;
	
	if (ob->gfx != NULL && --ob->gfx->used == 0) {
		gfx_alloc_sprites(ob->gfx, 0);
		gfx_alloc_anims(ob->gfx, 0);
		return (1);
	}
	return (0);
}

int
gfx_load(struct object *ob)
{
	extern const struct version object_ver;
	struct gfx *gfx = ob->gfx;
	char path[MAXPATHLEN];
	struct netbuf *buf;
	off_t gfx_offs;
	Uint32 i, j;
	
	if (object_copy_filename(ob, path, sizeof(path)) == -1) {
		return (-1);
	}
	if ((buf = netbuf_open(path, "rb", NETBUF_BIG_ENDIAN)) == NULL) {
		error_set("%s: %s", path, error_get());
		return (-1);
	}
	
	if (version_read(buf, &object_ver, NULL) == -1)
		goto fail;

	read_uint32(buf);				/* Skip data offs */
	gfx_offs = (off_t)read_uint32(buf);
	netbuf_seek(buf, gfx_offs, SEEK_SET);

	if (read_uint8(buf) == 0) {
		dprintf("%s: no gfx\n", ob->name);
		goto out;
	}

	read_uint32(buf);				/* Pad: flags */

	gfx_alloc_sprites(gfx, read_uint32(buf));
	dprintf("%s: %d sprites\n", ob->name, gfx->nsprites);
	for (i = 0; i < gfx->nsprites; i++) {
		struct sprite *spr = &gfx->sprites[i];

		copy_string(spr->name, buf, sizeof(spr->name));
		if (read_uint8(buf)) {
			spr->su = read_surface(buf, sfmt);
		} else {
			spr->su = NULL;
		}
		spr->xOrig = (int)read_sint32(buf);
		spr->yOrig = (int)read_sint32(buf);
		spr->snap_mode = (enum gfx_snap_mode)read_uint8(buf);

		if (read_uint8(buf)) {
			u_int nw, nh;
			int x, y;

			sprite_get_nattrs(spr, &nw, &nh);
			dprintf("%s: %d,%d attributes\n", ob->name, nw, nh);
			spr->attrs = Realloc(spr->attrs, nw*nh*sizeof(u_int));
			spr->layers = Realloc(spr->layers, nw*nh*sizeof(int));
			for (y = 0; y < nh; y++) {
				for (x = 0; x < nw; x++) {
					spr->attrs[y*nw + x] =
					    (u_int)read_uint32(buf);
					spr->layers[y*nw + x] =
					    (int)read_sint32(buf);
				}
			}
		} else {
			Free(spr->attrs, M_RG);
			Free(spr->layers, M_RG);
			spr->layers = NULL;
		}
	}

	gfx_alloc_anims(gfx, read_uint32(buf));
	dprintf("%s: %d anims\n", ob->name, gfx->nanims);
	for (i = 0; i < gfx->nanims; i++) {
		struct gfx_anim *anim = &gfx->anims[i];

		anim->frame = read_uint32(buf);
		anim->nframes = read_uint32(buf);
		anim->frames = Realloc(anim->frames, anim->nframes *
				                     sizeof(SDL_Surface *));
		for (j = 0; j < anim->nframes; j++)
			anim->frames[j] = read_surface(buf, vfmt);
	}

out:
	netbuf_close(buf);
	return (0);
fail:
	netbuf_close(buf);
	return (-1);
}

int
gfx_save(struct object *ob, struct netbuf *buf)
{
	struct gfx *gfx = ob->gfx;
	Uint32 i, j;

	if (gfx == NULL) {
		write_uint8(buf, 0);
		return (0);
	} else {
		write_uint8(buf, 1);
	}

	write_uint32(buf, 0);				/* Pad: flags */

	dprintf("%s: saving %d sprites\n", ob->name, gfx->nsprites);
	write_uint32(buf, gfx->nsprites);
	for (i = 0; i < gfx->nsprites; i++) {
		struct sprite *spr = &gfx->sprites[i];

		write_string(buf, spr->name);
		if (spr->su != NULL) {
			write_uint8(buf, 1);
			write_surface(buf, spr->su);
		} else {
			write_uint8(buf, 0);
		}
		write_sint32(buf, (Sint32)spr->xOrig);
		write_sint32(buf, (Sint32)spr->yOrig);
		write_uint8(buf, (Uint8)spr->snap_mode);

		if (spr->attrs != NULL && spr->layers != NULL) {
			int x, y;
			u_int nw, nh;

			write_uint8(buf, 1);
			sprite_get_nattrs(spr, &nw, &nh);
			for (y = 0; y < nh; y++) {
				for (x = 0; x < nw; x++) {
					write_uint32(buf,
					    (Uint32)spr->attrs[y*nw + x]);
					write_sint32(buf,
					    (Sint32)spr->layers[y*nw + x]);
				}
			}
		} else {
			write_uint8(buf, 0);
		}
	}

	dprintf("%s: saving %d anims\n", ob->name, gfx->nsprites);
	write_uint32(buf, gfx->nanims);
	for (i = 0; i < gfx->nanims; i++) {
		struct gfx_anim *anim = &gfx->anims[i];

		write_uint32(buf, anim->frame);
		write_uint32(buf, anim->nframes);
		for (j = 0; j < anim->nframes; j++)
			write_surface(buf, anim->frames[j]);
	}
	return (0);
}

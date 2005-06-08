/*	$Csoft: gfx.c,v 1.47 2005/04/21 06:38:10 vedge Exp $	*/

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

#ifdef DEBUG
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#endif

#include <string.h>
#include <stdarg.h>

enum {
	NANIMS_INIT =	1,
	NANIMS_GROW =	4,
	FRAMES_INIT =	2,
	FRAMES_GROW =	8,
	NSUBMAPS_INIT =	1,
	NSUBMAPS_GROW =	4
};

struct gfxq gfxq = TAILQ_HEAD_INITIALIZER(gfxq);
pthread_mutex_t gfxq_lock;
	
const char *gfx_snap_names[] = {
	N_("Free positioning"),
	N_("Snap to grid"),
	NULL
};

void
sprite_init(struct sprite *spr)
{
	spr->su = NULL;
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
sprite_destroy(struct sprite *spr)
{
	if (spr->su != NULL) {
		SDL_FreeSurface(spr->su);
		spr->su = NULL;
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

/*
 * Associate a different surface to a sprite; update the texture and
 * destroy the transform cache. The previous surface is freed if any.
 */
void
sprite_set_surface(struct sprite *spr, SDL_Surface *su)
{
	sprite_destroy(spr);
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
		sprite_destroy(&gfx->sprites[i]);

	if (n > 0) {
		gfx->sprites = Realloc(gfx->sprites, n*sizeof(struct sprite));
	} else {
		Free(gfx->sprites, M_GFX);
		gfx->sprites = NULL;
	}
	gfx->nsprites = n;

	for (i = 0; i < n; i++)
		sprite_init(&gfx->sprites[i]);
}

/* Allocate and initialize a new sprite at the end of the array. */
Uint32
gfx_insert_sprite(struct gfx *gfx, SDL_Surface *su)
{
	struct sprite *spr;
	
	gfx->sprites = Realloc(gfx->sprites, (gfx->nsprites+1) *
	                                     sizeof(struct sprite));
	spr = &gfx->sprites[gfx->nsprites];
	sprite_init(spr);
	sprite_set_surface(spr, su);
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

/* Disable garbage collection. */
void
gfx_wire(struct gfx *gfx)
{
	pthread_mutex_lock(&gfx->used_lock);
	gfx->used = GFX_MAX_USED;
	pthread_mutex_unlock(&gfx->used_lock);
}

/* Decrement the shared reference count. */
void
gfx_unused(struct gfx *gfx)
{
	pthread_mutex_lock(&gfx->used_lock);
	if (gfx->used != GFX_MAX_USED &&
	    --gfx->used == 0) {
		pthread_mutex_unlock(&gfx->used_lock);
		gfx_destroy(gfx);
		return;
	}
	pthread_mutex_unlock(&gfx->used_lock);
}

/* Allocate a private gfx structure for a given object. */
struct gfx *
gfx_alloc_pvt(void *p, const char *name)
{
	struct object *ob = p;
	struct gfx *gfx;
	
	gfx = Malloc(sizeof(struct gfx), M_GFX);
	gfx_init(gfx, GFX_PRIVATE, name);
	pthread_mutex_lock(&ob->lock);
	if (ob->gfx != NULL) {
		gfx_unused(ob->gfx);
	}
	Free(ob->gfx_name, 0);
	ob->gfx_name = NULL;
	ob->gfx = gfx;
	ob->gfx_used = 1;
	pthread_mutex_unlock(&ob->lock);
	return (gfx);
}

void
gfx_init(struct gfx *gfx, int type, const char *name)
{
	gfx->name = name != NULL ? Strdup(name) : NULL;
	gfx->type = type;
	gfx->sprites = NULL;
	gfx->nsprites = 0;
	gfx->anims = NULL;
	gfx->canims = NULL;
	gfx->nanims = 0;
	gfx->maxanims = 0;
	gfx->submaps = NULL;
	gfx->nsubmaps = 0;
	gfx->maxsubmaps = 0;
	gfx->used = 1;
	pthread_mutex_init(&gfx->used_lock, NULL);
}

/*
 * Return a pointer to the named graphics package.
 * If the package is resident, increment the shared reference count.
 * Otherwise, load the package from disk.
 */
struct gfx *
gfx_fetch_shd(const char *name)
{
	char path[MAXPATHLEN];
	struct gfx *gfx = NULL;
	struct den *den;
	Uint32 i;

	pthread_mutex_lock(&gfxq_lock);

	TAILQ_FOREACH(gfx, &gfxq, gfxs) {
		if (strcmp(gfx->name, name) == 0)
			break;
	}
	if (gfx != NULL) {
		if (++gfx->used > GFX_MAX_USED) {
			gfx->used = GFX_MAX_USED;
		}
		goto out;
	}

	if (config_search_file("den-path", name, "den", path, sizeof(path))
	    == -1)
		goto fail;

	gfx = Malloc(sizeof(struct gfx), M_GFX);
	gfx_init(gfx, GFX_SHARED, name);

	if ((den = den_open(path, DEN_READ)) == NULL) {
		goto fail;
	}
	for (i = 0; i < den->nmembers; i++) {
		if (xcf_load(den->buf, den->members[i].offs, gfx) == -1) {
			den_close(den);
			goto fail;
		}
	}
	den_close(den);
	TAILQ_INSERT_HEAD(&gfxq, gfx, gfxs);			/* Cache */
out:
	pthread_mutex_unlock(&gfxq_lock);
	return (gfx);
fail:
	pthread_mutex_unlock(&gfxq_lock);
	if (gfx != NULL) {
		pthread_mutex_destroy(&gfx->used_lock);
		Free(gfx->name, 0);
		Free(gfx, M_GFX);
	}
	return (NULL);
}

static void
destroy_anim(struct gfx_anim *anim)
{
	Uint32 i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	Free(anim->frames, M_GFX);
	Free(anim, M_GFX);
}

static void
gfx_free_anim(struct gfx *gfx, Uint32 name)
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

	if (gfx->anims[name] != NULL) {
		destroy_anim(gfx->anims[name]);
		gfx->anims[name] = NULL;
	}
}

/* Release a graphics package that is no longer in use. */
void
gfx_destroy(struct gfx *gfx)
{
	Uint32 i;

	if (gfx->type == GFX_SHARED) {
		pthread_mutex_lock(&gfxq_lock);
		TAILQ_REMOVE(&gfxq, gfx, gfxs);
		pthread_mutex_unlock(&gfxq_lock);
	}

	for (i = 0; i < gfx->nsprites; i++) {
		sprite_destroy(&gfx->sprites[i]);
	}
	for (i = 0; i < gfx->nanims; i++) {
		gfx_free_anim(gfx, i);
	}
	for (i = 0; i < gfx->nsubmaps; i++) {
		object_destroy(gfx->submaps[i]);
		Free(gfx->submaps[i], M_OBJECT);
	}

	Free(gfx->sprites, M_GFX);
	Free(gfx->anims, M_GFX);
	Free(gfx->canims, M_GFX);
	Free(gfx->name, 0);
	Free(gfx->submaps, M_GFX);
	pthread_mutex_destroy(&gfx->used_lock);
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

/* Allocate a new animation. */
struct gfx_anim *
gfx_insert_anim(struct gfx *gfx)
{
	struct gfx_anim *anim;
	struct gfx_animcl *animcl;

	anim = Malloc(sizeof(struct gfx_anim), M_GFX);
	anim->frames = NULL;
	anim->maxframes = 0;
	anim->frame = 0;
	anim->nframes = 0;

	if (gfx->anims == NULL) {
		gfx->anims = Malloc(NANIMS_INIT*sizeof(struct anim *), M_GFX);
		gfx->canims = Malloc(NANIMS_INIT*sizeof(struct gfx_animcl),
		    M_GFX);
		gfx->maxanims = NANIMS_INIT;
		gfx->nanims = 0;
	} else if (gfx->nanims >= gfx->maxanims) {
		gfx->maxanims += NANIMS_GROW;
		gfx->anims = Realloc(gfx->anims,
		    gfx->maxanims*sizeof(struct gfx_anim *));
		gfx->canims = Realloc(gfx->canims,
		    gfx->maxanims*sizeof(struct gfx_animcl));
	}
	gfx->anims[gfx->nanims] = anim;
	animcl = &gfx->canims[gfx->nanims];
	SLIST_INIT(&animcl->anims);
	gfx->nanims++;
	return (anim);
}

#ifdef DEBUG
static void
poll_gfx(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct gfx *gfx;

	tlist_clear_items(tl);
	pthread_mutex_lock(&gfxq_lock);

	TAILQ_FOREACH(gfx, &gfxq, gfxs) {
		char label[TLIST_LABEL_MAX];
		struct tlist_item *it;
		Uint32 i;

		snprintf(label, sizeof(label), "%s (%lu refs)\n%ds/%da/%dm\n",
		    gfx->name,
		    (gfx->used != GFX_MAX_USED) ? (unsigned long)gfx->used : 0,
		    gfx->nsprites,
		    gfx->nanims, gfx->nsubmaps);
		it = tlist_insert_item(tl, NULL, label, gfx);
		it->depth = 0;

		if (gfx->nsprites > 0 || gfx->nanims > 0 || gfx->nsubmaps > 0)
			it->flags |= TLIST_HAS_CHILDREN;

		if ((it->flags & TLIST_HAS_CHILDREN) &&
		    tlist_visible_children(tl, it)) {
			for (i = 0; i < gfx->nsprites; i++) {
				struct sprite *spr = &gfx->sprites[i];
				SDL_Surface *su = spr->su;
				struct tlist_item *it;
				struct gfx_cached_sprite *csp;

				if (su != NULL) {
					snprintf(label, sizeof(label),
					    "%ux%ux%u (%d bytes%s)",
					    su->w, su->h,
					    su->format->BitsPerPixel,
					    (int)su->w*su->h*
					    su->format->BytesPerPixel,
					    (su->flags&SDL_SRCALPHA) ?
					    ", alpha":
					    (su->flags&SDL_SRCCOLORKEY) ?
					    ", ckey":
					    "");
				} else {
					strlcpy(label, _("(null)"),
					    sizeof(label));
				}

				it = tlist_insert_item(tl, su, label, spr);
				it->depth = 1;

				if (!SLIST_EMPTY(&spr->csprites)) {
					it->flags |= TLIST_HAS_CHILDREN;
				}
				if ((it->flags & TLIST_HAS_CHILDREN) &&
		    		    tlist_visible_children(tl, it)) {
					SLIST_FOREACH(csp, &spr->csprites,
					    sprites) {
						struct tlist_item *it;

						snprintf(label, sizeof(label),
						    "%u ticks\n",
						    csp->last_drawn);
						transform_print(
						    &csp->transforms,
						    label, sizeof(label));

						it = tlist_insert_item(tl,
						    csp->su, label, csp);
						it->depth = 2;
					}
				}
			}
			for (i = 0; i < gfx->nanims; i++) {
				struct gfx_anim *anim = gfx->anims[i];
				struct gfx_animcl *acl = &gfx->canims[i];
				struct gfx_cached_anim *can;
				struct tlist_item *it;

				snprintf(label, sizeof(label), "frame %u/%u",
				    anim->frame, anim->nframes);

				it = tlist_insert_item(tl, NULL, label,
				    gfx->anims[i]);
				it->depth = 1;

				if (!SLIST_EMPTY(&acl->anims)) {
					it->flags |= TLIST_HAS_CHILDREN;
				}
				if ((it->flags & TLIST_HAS_CHILDREN) &&
		    		    tlist_visible_children(tl, it)) {
					SLIST_FOREACH(can, &acl->anims, anims) {
						struct tlist_item *it;

						snprintf(label, sizeof(label),
						    "%u ticks\n",
						    can->last_drawn);
						transform_print(
						    &can->transforms,
						    label, sizeof(label));

						it = tlist_insert_item(tl, NULL,
						    label, can);
						it->depth = 2;
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&gfxq_lock);
	tlist_restore_selections(tl);
}

struct window *
gfx_debug_window(void)
{
	struct window *win;
	struct tlist *tl;

	if ((win = window_new(WINDOW_DETACH, "gfx-debug")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Resident graphics"));

	tl = tlist_new(win, TLIST_POLL|TLIST_TREE);
	tlist_set_item_height(tl, TILESZ);
	event_new(tl, "tlist-poll", poll_gfx, NULL);

	return (win);
}
#endif /* DEBUG */

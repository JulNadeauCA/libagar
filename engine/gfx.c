/*	$Csoft: gfx.c,v 1.25 2004/03/17 06:03:09 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/config.h>
#include <engine/mapedit/mapedit.h>
#include <engine/loader/den.h>
#include <engine/loader/xcf.h>
#ifdef DEBUG
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#endif

#include <string.h>

#ifdef DEBUG
#define DEBUG_GC	0x01
int	gfx_debug = DEBUG_GC;
#define engine_debug gfx_debug
#endif

enum {
	NANIMS_INIT =	1,
	NANIMS_GROW =	4,
	NSPRITES_INIT =	1,
	NSPRITES_GROW = 4,
	FRAMES_INIT =	2,
	FRAMES_GROW =	8,
	NSUBMAPS_INIT =	1,
	NSUBMAPS_GROW =	4
};

static TAILQ_HEAD(, gfx) gfxq = TAILQ_HEAD_INITIALIZER(gfxq);
pthread_mutex_t		 gfxq_lock;

static void gfx_destroy_anim(struct gfx_anim *);

/* Attach a sprite of arbitrary size. */
Uint32
gfx_insert_sprite(struct gfx *gfx, SDL_Surface *sprite, int map)
{
	struct gfx_spritecl *spritecl;

	if (gfx->sprites == NULL) {
		gfx->sprites = Malloc(
		    NSPRITES_INIT * sizeof(SDL_Surface *), M_GFX);
		gfx->csprites = Malloc(
		    NSPRITES_INIT * sizeof(struct gfx_spritecl), M_GFX);
		gfx->maxsprites = NSPRITES_INIT;
		gfx->nsprites = 0;
	} else if (gfx->nsprites+1 > gfx->maxsprites) {
		gfx->maxsprites += NSPRITES_GROW;
		gfx->sprites = Realloc(gfx->sprites,
		    gfx->maxsprites * sizeof(SDL_Surface *), M_GFX);
		gfx->csprites = Realloc(gfx->csprites,
		    gfx->maxsprites * sizeof(struct gfx_spritecl), M_GFX);
	}
	gfx->sprites[gfx->nsprites] = sprite;
	spritecl = &gfx->csprites[gfx->nsprites];
	SLIST_INIT(&spritecl->sprites);
	return (gfx->nsprites++);
}

/*
 * Scan for alpha pixels on a surface and choose an optimal
 * alpha/colorkey setting.
 */
void
gfx_scan_alpha(SDL_Surface *su)
{
	enum {
		GFX_ALPHA_TRANSPARENT = 0x01,
		GFX_ALPHA_OPAQUE =	0x02,
		GFX_ALPHA_ALPHA =	0x04
	} aflags = 0;
	Uint8 oldalpha = su->format->alpha;
#if 0
	Uint8 oldckey = su->format->colorkey;
#endif
	int x, y;

	if (SDL_MUSTLOCK(su)) {
		SDL_LockSurface(su);
	}
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint8 *pixel = (Uint8 *)su->pixels +
			    y*su->pitch + x*su->format->BytesPerPixel;
			Uint8 r, g, b, a;

			switch (su->format->BytesPerPixel) {
			case 4:
				SDL_GetRGBA(*(Uint32 *)pixel, su->format,
				    &r, &g, &b, &a);
				break;
			case 3:
			case 2:
				SDL_GetRGBA(*(Uint16 *)pixel, su->format,
				    &r, &g, &b, &a);
				break;
			case 1:
				SDL_GetRGBA(*pixel, su->format,
				    &r, &g, &b, &a);
				break;
			}

			switch (a) {
			case SDL_ALPHA_TRANSPARENT:
				aflags |= GFX_ALPHA_TRANSPARENT;
				break;
			case SDL_ALPHA_OPAQUE:
				aflags |= GFX_ALPHA_OPAQUE;
				break;
			default:
				aflags |= GFX_ALPHA_ALPHA;
				break;
			}
		}
	}
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);

	/* XXX use rleaccel */
	SDL_SetAlpha(su, 0, 0);
	SDL_SetColorKey(su, 0, 0);
	if (aflags & (GFX_ALPHA_ALPHA|GFX_ALPHA_TRANSPARENT)) {
		SDL_SetAlpha(su, SDL_SRCALPHA, oldalpha);
#if 0
	/* XXX causes some images to be rendered incorrectly. */
	} else if (aflags & GFX_ALPHA_TRANSPARENT) {
		dprintf("colorkey %u\n", oldckey);
		SDL_SetColorKey(su, SDL_SRCCOLORKEY, 0);
#endif
	}
}

/*
 * Break a surface into newly-allocated tile-sized fragments, generating
 * a map of the fragments.
 */
int
gfx_insert_fragments(struct gfx *gfx, SDL_Surface *sprite)
{
	char mapname[OBJECT_NAME_MAX];
	int x, y, mx, my;
	unsigned int mw, mh;
	SDL_Rect sd, rd;
	struct map *fragmap;

	sd.w = TILESZ;
	sd.h = TILESZ;
	rd.x = 0;
	rd.y = 0;
	mw = sprite->w/TILESZ;
	mh = sprite->h/TILESZ;

	fragmap = Malloc(sizeof(struct map), M_OBJECT);
	snprintf(mapname, sizeof(mapname), "f-%s%u", gfx->name, gfx->nsubmaps);
	map_init(fragmap, mapname);
	if (map_alloc_nodes(fragmap, mw, mh) == -1)
		return (-1);

	for (y = 0, my = 0; y < sprite->h; y += TILESZ, my++) {
		for (x = 0, mx = 0; x < sprite->w; x += TILESZ, mx++) {
			SDL_Surface *su;
			Uint32 saflags = sprite->flags & (SDL_SRCALPHA|
			                                  SDL_RLEACCEL);
			Uint8 salpha = sprite->format->alpha;
			struct node *node = &fragmap->map[my][mx];
			Uint32 nsprite;

			/* Allocate a surface for the fragment. */
			su = SDL_CreateRGBSurface(SDL_SWSURFACE |
			    (sprite->flags &
			    (SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
			    TILESZ, TILESZ, sprite->format->BitsPerPixel,
			    sprite->format->Rmask,
			    sprite->format->Gmask,
			    sprite->format->Bmask,
			    sprite->format->Amask);
			if (su == NULL) {
				error_set("SDL_CreateRGBSurface: %s",
				    SDL_GetError());
				return (-1);
			}
			
			/* Copy the fragment as-is. */
			SDL_SetAlpha(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(sprite, &sd, su, &rd);
			nsprite = gfx_insert_sprite(gfx, su, 0);
			SDL_SetAlpha(sprite, saflags, salpha);

			/* Adjust the alpha properties of the fragment. */
			gfx_scan_alpha(su);

			/* Map the sprite as a NULL reference. */
			node_add_sprite(fragmap, node, NULL, nsprite);
		}
	}

	gfx_insert_submap(gfx, fragmap);
	return (0);
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

void
gfx_init(struct gfx *gfx, const char *name)
{
	gfx->name = Strdup(name);
	gfx->sprites = NULL;
	gfx->csprites = NULL;
	gfx->nsprites = 0;
	gfx->maxsprites = 0;
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
gfx_fetch(const char *name)
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
	gfx_init(gfx, name);

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

/* Release a graphics package that is no longer in use. */
void
gfx_destroy(struct gfx *gfx)
{
	Uint32 i;

	pthread_mutex_lock(&gfxq_lock);
	TAILQ_REMOVE(&gfxq, gfx, gfxs);
	pthread_mutex_unlock(&gfxq_lock);

	/* Release the sprites. */
	for (i = 0; i < gfx->nsprites; i++) {
		struct gfx_spritecl *spritecl = &gfx->csprites[i];
		struct gfx_cached_sprite *csprite, *ncsprite;
		struct transform *trans, *ntrans;
		
		SDL_FreeSurface(gfx->sprites[i]);

		/* Free the sprite transform cache. */
		for (csprite = SLIST_FIRST(&spritecl->sprites);
		     csprite != SLIST_END(&spritecl->sprites);
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
	}
	Free(gfx->sprites, M_GFX);
	Free(gfx->csprites, M_GFX);

	/* Release the animations. */
	for (i = 0; i < gfx->nanims; i++) {
		struct gfx_animcl *animcl = &gfx->canims[i];
		struct gfx_cached_anim *canim, *ncanim;
		struct transform *trans, *ntrans;

		/* Free the anim transform cache. */
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
			gfx_destroy_anim(canim->anim);
			Free(canim, 0);
		}
		gfx_destroy_anim(gfx->anims[i]);
	}
	Free(gfx->anims, M_GFX);
	Free(gfx->canims, M_GFX);

	/* Release the submaps. */
	for (i = 0; i < gfx->nsubmaps; i++) {
		object_destroy(gfx->submaps[i]);
		Free(gfx->submaps[i], M_OBJECT);
	}
	Free(gfx->submaps, M_GFX);

	debug(DEBUG_GC, "freed %s (%d sprites, %d anims, %d maps)\n", gfx->name,
	    gfx->nsprites, gfx->nanims, gfx->nsubmaps);

	pthread_mutex_destroy(&gfx->used_lock);
	Free(gfx->name, 0);
	Free(gfx, M_GFX);
}

/* Insert a frame into an animation. */
Uint32
gfx_insert_anim_frame(struct gfx_anim *anim, SDL_Surface *surface)
{
	if (anim->frames == NULL) {			/* Initialize */
		anim->frames = Malloc(FRAMES_INIT * sizeof(SDL_Surface *),
		    M_GFX);
		anim->maxframes = FRAMES_INIT;
		anim->nframes = 0;
	} else if (anim->nframes+1 > anim->maxframes) {	/* Grow */
		anim->maxframes += FRAMES_GROW;
		anim->frames = Realloc(anim->frames,
		    anim->maxframes * sizeof(SDL_Surface *), M_GFX);
	}
	anim->frames[anim->nframes++] = surface;
	return (anim->nframes);
}

/* Insert a new submap. */
Uint32
gfx_insert_submap(struct gfx *gfx, struct map *m)
{
	if (gfx->submaps == NULL) {
		gfx->maxsubmaps = NSUBMAPS_INIT;
		gfx->submaps = Malloc(
		    gfx->maxsubmaps * sizeof(struct map *), M_GFX);
		gfx->nsubmaps = 0;
	} else if (gfx->nsubmaps+1 > gfx->maxsubmaps) {
		gfx->maxsubmaps += NSUBMAPS_GROW;
		gfx->submaps = Realloc(gfx->submaps,
		    gfx->maxsubmaps * sizeof(struct map *), M_GFX);
	}
	gfx->submaps[gfx->nsubmaps] = m;
	return (gfx->nsubmaps++);
}

/* Insert a new animation. */
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
		gfx->anims = Malloc(
		    NANIMS_INIT * sizeof(struct anim *), M_GFX);
		gfx->canims = Malloc(
		    NANIMS_INIT * sizeof(struct gfx_animcl), M_GFX);
		gfx->maxanims = NANIMS_INIT;
		gfx->nanims = 0;
	} else if (gfx->nanims >= gfx->maxanims) {
		gfx->maxanims += NANIMS_GROW;
		gfx->anims = Realloc(gfx->anims,
		    gfx->maxanims * sizeof(struct gfx_anim *), M_GFX);
		gfx->canims = Realloc(gfx->canims,
		    gfx->maxanims * sizeof(struct gfx_animcl), M_GFX);
	}
	gfx->anims[gfx->nanims] = anim;
	animcl = &gfx->canims[gfx->nanims];
	SLIST_INIT(&animcl->anims);
	gfx->nanims++;
	return (anim);
}

static void
gfx_destroy_anim(struct gfx_anim *anim)
{
	Uint32 i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	Free(anim->frames, 0);
	Free(anim, M_GFX);
}

#ifdef DEBUG
SDL_Surface *
gfx_get_sprite(struct object *ob, Uint32 i)
{
	if (ob->gfx == NULL)
		fatal("no gfx in %s", ob->name);
	if (i > ob->gfx->nsprites)
		fatal("no sprite at %s:%d", ob->name, i);

	return (ob->gfx->sprites[i]);
}

struct gfx_anim *
gfx_get_anim(struct object *ob, Uint32 i)
{
	if (ob->gfx == NULL)
		fatal("no gfx in %s", ob->name);
	if (i > ob->gfx->nanims)
		fatal("no anim at %s:%d", ob->name, i);

	return (ob->gfx->anims[i]);
}

static void
gfx_debug_poll(int argc, union evarg *argv)
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
				SDL_Surface *su = gfx->sprites[i];
				struct tlist_item *it;
				struct gfx_spritecl *scl = &gfx->csprites[i];
				struct gfx_cached_sprite *csp;

				snprintf(label, sizeof(label),
				    "%ux%ux%u (%d bytes)",
				    su->w, su->h, su->format->BitsPerPixel,
				    (int)su->w*su->h*su->format->BytesPerPixel);

				it = tlist_insert_item(tl, gfx->sprites[i],
				    label, gfx->sprites[i]);
				it->depth = 1;

				if (!SLIST_EMPTY(&scl->sprites)) {
					it->flags |= TLIST_HAS_CHILDREN;
				}
				if ((it->flags & TLIST_HAS_CHILDREN) &&
		    		    tlist_visible_children(tl, it)) {
					SLIST_FOREACH(csp, &scl->sprites,
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

	if ((win = window_new("gfx-debug")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Resident graphics"));
	window_set_closure(win, WINDOW_DETACH);

	tl = tlist_new(win, TLIST_POLL|TLIST_TREE);
	tlist_set_item_height(tl, text_font_height(font)*2 + 5);
	event_new(tl, "tlist-poll", gfx_debug_poll, NULL);

	return (win);
}

#endif /* DEBUG */


/*	$Csoft: art.c,v 1.32 2003/03/26 02:52:24 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/snprintf.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/config.h>

#include "xcf.h"

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>

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

static TAILQ_HEAD(, art) artq = TAILQ_HEAD_INITIALIZER(artq);
static pthread_mutex_t	 artq_lock = PTHREAD_MUTEX_INITIALIZER;

#ifdef DEBUG
#define DEBUG_GC	0x01
#define DEBUG_LOADING	0x02

int	art_debug = DEBUG_GC|DEBUG_LOADING;
#define engine_debug	art_debug
#endif

static void	art_destroy(struct art *);
static void	art_destroy_anim(struct art_anim *);

/* Attach a surface of arbitrary size. */
Uint32
art_insert_sprite(struct art *art, SDL_Surface *sprite, int map)
{
	struct art_spritecl *spritecl;

	if (art->sprites == NULL) {			/* Initialize */
		art->sprites = Malloc(NSPRITES_INIT * sizeof(SDL_Surface *));
		art->csprites = Malloc(NSPRITES_INIT *
		    sizeof(struct art_spritecl));
		art->maxsprites = NSPRITES_INIT;
		art->nsprites = 0;
	} else if (art->nsprites+1 > art->maxsprites) {	/* Grow */
		art->maxsprites += NSPRITES_GROW;
		art->sprites = Realloc(art->sprites,
		    art->maxsprites * sizeof(SDL_Surface *));
		art->csprites = Realloc(art->csprites,
		    art->maxsprites * sizeof(struct art_spritecl));
	}
	art->sprites[art->nsprites] = sprite;			/* Original */

	spritecl = &art->csprites[art->nsprites];		/* Cache line */
	SLIST_INIT(&spritecl->sprites);
	return (art->nsprites++);
}

/*
 * Scan for alpha pixels on a surface and choose an optimal
 * alpha/colorkey setting.
 */
void
art_scan_alpha(SDL_Surface *su)
{
	enum {
		ART_ALPHA_TRANSPARENT = 0x01,
		ART_ALPHA_OPAQUE =	0x02,
		ART_ALPHA_ALPHA =	0x04
	} aflags = 0;
	Uint8 oldalpha = su->format->alpha;
#if 0
	Uint8 oldckey = su->format->colorkey;
#endif
	int x, y;

	if (SDL_MUSTLOCK(su))
		SDL_LockSurface(su);
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
				aflags |= ART_ALPHA_TRANSPARENT;
				break;
			case SDL_ALPHA_OPAQUE:
				aflags |= ART_ALPHA_OPAQUE;
				break;
			default:
				aflags |= ART_ALPHA_ALPHA;
				break;
			}
		}
	}
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);

	/* XXX use rleaccel */
	SDL_SetAlpha(su, 0, 0);
	SDL_SetColorKey(su, 0, 0);
	if (aflags & (ART_ALPHA_ALPHA|ART_ALPHA_TRANSPARENT)) {
		SDL_SetAlpha(su, SDL_SRCALPHA, oldalpha);
#if 0
	/* XXX causes some images to be rendered incorrectly. */
	} else if (aflags & XCF_ALPHA_TRANSPARENT) {
		dprintf("colorkey %u\n", oldckey);
		SDL_SetColorKey(su, SDL_SRCCOLORKEY, 0);
#endif
	}
}

/* Break a surface into tile-sized fragments and map them. */
struct map *
art_insert_fragments(struct art *art, SDL_Surface *sprite)
{
	char mapname[OBJECT_NAME_MAX];
	int x, y, mx, my;
	unsigned int mw, mh;
	SDL_Rect sd, rd;
	struct map *fragmap;

	sd.w = TILEW;
	sd.h = TILEH;
	rd.x = 0;
	rd.y = 0;
	mw = sprite->w/TILEW;
	mh = sprite->h/TILEH;

	fragmap = Malloc(sizeof(struct map));
	snprintf(mapname, sizeof(mapname), "frag-%s%u", art->name,
	    art->nsubmaps);
	map_init(fragmap, mapname, NULL);
	if (map_alloc_nodes(fragmap, mw, mh) == -1) {
		fatal("map_alloc_nodes: %s", error_get());
	}

	for (y = 0, my = 0; y < sprite->h; y += TILEH, my++) {
		for (x = 0, mx = 0; x < sprite->w; x += TILEW, mx++) {
			SDL_Surface *su;
			Uint32 saflags = sprite->flags &
			    (SDL_SRCALPHA|SDL_RLEACCEL);
			Uint8 salpha = sprite->format->alpha;
			struct node *node = &fragmap->map[my][mx];
			Uint32 nsprite;

			/* Allocate a surface for the fragment. */
			su = SDL_CreateRGBSurface(SDL_SWSURFACE |
			    (sprite->flags &
			    (SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
			    TILEW, TILEH, sprite->format->BitsPerPixel,
			    sprite->format->Rmask,
			    sprite->format->Gmask,
			    sprite->format->Bmask,
			    sprite->format->Amask);
			if (su == NULL) {
				fatal("SDL_CreateRGBSurface: %s",
				    SDL_GetError());
			}
			
			/* Copy the fragment as-is. */
			SDL_SetAlpha(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(sprite, &sd, su, &rd);
			nsprite = art_insert_sprite(art, su, 0);
			SDL_SetAlpha(sprite, saflags, salpha);

			/* Adjust the alpha properties of the fragment. */
			art_scan_alpha(su);

			node_add_sprite(node, art->pobj, nsprite);
		}
	}

	art_insert_submap(art, fragmap);
	return (fragmap);
}

/* Disable garbage collection for an art structure. */
void
art_wire(struct art *art)
{
	pthread_mutex_lock(&art->used_lock);
	art->used = ART_MAX_USED;
	pthread_mutex_unlock(&art->used_lock);
}

/* Decrement the reference count of an art structure. */
void
art_unused(struct art *art)
{
	pthread_mutex_lock(&art->used_lock);
	if (art->used != ART_MAX_USED &&		/* Remain resident? */
	    --art->used == 0) {
		art_destroy(art);
	}
	pthread_mutex_unlock(&art->used_lock);
}

/* Load graphical data associated with an object. */
struct art *
art_fetch(char *archive, struct object *ob)
{
	struct art *art;
	struct fobj *fob;
	char path[FILENAME_MAX];
	Uint32 i;

	pthread_mutex_lock(&artq_lock);

	/* Look for a cached entry. XXX inefficient */
	TAILQ_FOREACH(art, &artq, arts) {
		if (strcmp(art->name, archive) == 0)
			break;
	}
	if (art != NULL) {
		if (++art->used > ART_MAX_USED)
			art->used = ART_MAX_USED; 	/* Remain resident */
		goto out;
	}

	/* Load the data file. */
	if (object_path(archive, "fob", path, sizeof(path)) == -1) {
		if (ob->flags & OBJECT_ART_CAN_FAIL) {
			ob->flags &= ~(OBJECT_ART);
			goto out;
		} else {
			fatal("loading %s: %s", archive, error_get());
		}
	}

	art = Malloc(sizeof(struct art));
	art->name = Strdup(archive);
	art->pobj = ob;				/* For submap refs */
	art->sprites = NULL;
	art->csprites = NULL;
	art->nsprites = 0;
	art->maxsprites = 0;
	art->anims = NULL;
	art->canims = NULL;
	art->nanims = 0;
	art->maxanims = 0;
	art->submaps = NULL;
	art->nsubmaps = 0;
	art->maxsubmaps = 0;
	art->used = 1;
	pthread_mutex_init(&art->used_lock, NULL);

	if (prop_get_bool(config, "object.art.map-tiles")) {
		char mapname[OBJECT_NAME_MAX];

		art->tile_map = Malloc(sizeof(struct map));
		snprintf(mapname, sizeof(mapname), "t-%s", archive);
		map_init(art->tile_map, mapname, NULL);

		if (map_alloc_nodes(art->tile_map, 2, 2) == -1)
			fatal("failed node alloc: %s", error_get());
	} else {
		art->tile_map = NULL;
	}

	/* Load images in XCF format. */
	if ((fob = fobj_load(path)) == NULL)
		fatal("%s: %s", path, fobj_error);

	for (i = 0; i < fob->head.nobjs; i++) {
		if (xcf_check(fob->fd, fob->offs[i]) == 0) {
			int rv;
				
			rv = xcf_load(fob->fd, (off_t)fob->offs[i], art);
			if (rv != 0) {
				fatal("xcf_load: %s", error_get());
			}
		} else {
			debug(DEBUG_LOADING, "not a xcf in slot %d\n", i);
		}
	}
	fobj_free(fob);

	TAILQ_INSERT_HEAD(&artq, art, arts);			/* Cache */
out:
	pthread_mutex_unlock(&artq_lock);
	return (art);
}

static void
art_destroy(struct art *art)
{
	Uint32 i;

	pthread_mutex_lock(&artq_lock);
	TAILQ_REMOVE(&artq, art, arts);
	pthread_mutex_unlock(&artq_lock);

	/* Release the sprites. */
	for (i = 0; i < art->nsprites; i++) {
		struct art_spritecl *spritecl = &art->csprites[i];
		struct art_cached_sprite *csprite, *ncsprite;
		struct transform *trans, *ntrans;
		
		SDL_FreeSurface(art->sprites[i]);

		/* Free the sprite transform cache. */
		for (csprite = SLIST_FIRST(&spritecl->sprites);
		     csprite != SLIST_END(&spritecl->sprites);
		     csprite = ncsprite) {
			ncsprite = SLIST_NEXT(csprite, sprites);
			for (trans = SLIST_FIRST(&csprite->transforms);
			     trans != SLIST_END(&csprite->transforms);
			     trans = ntrans) {
				ntrans = SLIST_NEXT(trans, transforms);
				transform_destroy(trans);
				free(trans);
			}
			SDL_FreeSurface(csprite->su);
			free(csprite);
		}
	}
	Free(art->sprites);
	Free(art->csprites);

	/* Release the animations. */
	for (i = 0; i < art->nanims; i++) {
		struct art_animcl *animcl = &art->canims[i];
		struct art_cached_anim *canim, *ncanim;
		struct transform *trans, *ntrans;

		/* Free the anim transform cache. */
		for (canim = SLIST_FIRST(&animcl->anims);
		     canim != SLIST_END(&animcl->anims);
		     canim = ncanim) {
			ncanim = SLIST_NEXT(canim, anims);
			for (trans = SLIST_FIRST(&canim->transforms);
			     trans != SLIST_END(&canim->transforms);
			     trans = ntrans) {
				ntrans = SLIST_NEXT(trans, transforms);
				transform_destroy(trans);
				free(trans);
			}
			art_destroy_anim(canim->anim);
			free(canim);
		}
		art_destroy_anim(art->anims[i]);
	}
	Free(art->anims);
	Free(art->canims);

	/* Release the submaps. */
	if (art->tile_map != NULL)
		object_destroy(art->tile_map);
	for (i = 0; i < art->nsubmaps; i++) {
		object_destroy(art->submaps[i]);
	}
	Free(art->submaps);

	debug(DEBUG_GC, "freed %s (%d sprites, %d anims)\n",
	    art->name, art->nsprites, art->nanims);

	free(art->name);
	free(art);
}

Uint32
art_insert_anim_frame(struct art_anim *anim, SDL_Surface *surface)
{
	if (anim->frames == NULL) {			/* Initialize */
		anim->frames = Malloc(FRAMES_INIT * sizeof(SDL_Surface *));
		anim->maxframes = FRAMES_INIT;
		anim->nframes = 0;
	} else if (anim->nframes+1 > anim->maxframes) {	/* Grow */
		anim->maxframes += FRAMES_GROW;
		anim->frames = Realloc(anim->frames,
		    anim->maxframes * sizeof(SDL_Surface *));
	}
	anim->frames[anim->nframes++] = surface;
	return (anim->nframes);
}

Uint32
art_insert_submap(struct art *art, struct map *m)
{
	if (art->submaps == NULL) {			/* Initialize */
		art->maxsubmaps = NSUBMAPS_INIT;
		art->submaps = Malloc(art->maxsubmaps * sizeof(struct map *));
		art->nsubmaps = 0;
	} else if (art->nsubmaps+1 > art->maxsubmaps) {	/* Grow */
		art->maxsubmaps += NSUBMAPS_GROW;
		art->submaps = Realloc(art->submaps,
		    art->maxsubmaps * sizeof(struct map *));
	}
	art->submaps[art->nsubmaps] = m;
	return (art->nsubmaps++);
}

struct art_anim *
art_insert_anim(struct art *art, int delay)
{
	struct art_anim *anim;
	struct art_animcl *animcl;

	anim = Malloc(sizeof(struct art_anim));
	anim->frames = NULL;
	anim->maxframes = 0;
	anim->frame = 0;
	anim->nframes = 0;
	anim->delta = 0;
	anim->delay = delay;

	if (art->anims == NULL) {			/* Initialize */
		art->anims = Malloc(NANIMS_INIT * sizeof(struct anim *));
		art->canims = Malloc(NANIMS_INIT * sizeof(struct art_animcl));
		art->maxanims = NANIMS_INIT;
		art->nanims = 0;
	} else if (art->nanims >= art->maxanims) {	/* Grow */
		art->maxanims += NANIMS_GROW;
		art->anims = Realloc(art->anims,
		    art->maxanims * sizeof(struct art_anim *));
		art->canims = Realloc(art->canims,
		    art->maxanims * sizeof(struct art_animcl));
	}
	art->anims[art->nanims] = anim;				/* Original */

	animcl = &art->canims[art->nanims];			/* Cache line */
	SLIST_INIT(&animcl->anims);
	art->nanims++;
	return (anim);
}

static void
art_destroy_anim(struct art_anim *anim)
{
	Uint32 i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	Free(anim->frames);
	free(anim);
}

void
art_anim_tick(struct art_anim *an, struct noderef *nref)
{
	Uint32 ticks;
	
	if ((nref->data.anim.flags & NODEREF_ANIM_AUTO) == 0) {
		return;
	}
	
	ticks = SDL_GetTicks();
	if ((ticks - an->delta) >= an->delay) {		/* XXX */
		an->delta = ticks;
		if (++an->frame > an->nframes - 1) {
			/* Loop */
			an->frame = 0;
		}
	}
}

#ifdef DEBUG

SDL_Surface *
art_get_sprite(struct object *ob, Uint32 i)
{
	if ((ob->flags & OBJECT_ART) == 0)
		fatal("%s: no art", ob->name);
	if (ob->art == NULL)
		fatal("%s: null art", ob->name);
	if (i > ob->art->nsprites)
		fatal("no sprite at %s:%d", ob->name, i);

	return (ob->art->sprites[i]);
}

struct art_anim *
art_get_anim(struct object *ob, Uint32 i)
{
	if ((ob->flags & OBJECT_ART) == 0)
		fatal("%s: no art", ob->name);
	if (ob->art == NULL)
		fatal("%s: null art", ob->name);
	if (i > ob->art->nanims)
		fatal("no anim at %s:%d", ob->name, i);
	return (ob->art->anims[i]);
}

static void
tl_medias_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct art *art;

	tlist_clear_items(tl);

	pthread_mutex_lock(&artq_lock);
	TAILQ_FOREACH(art, &artq, arts) {
		tlist_insert_item(tl,
		    art->nsprites > 0 ? art->sprites[0] : NULL,
		    art->name, art);
	}
	pthread_mutex_unlock(&artq_lock);

	tlist_restore_selections(tl);
}

static void
tl_medias_selected(int argc, union evarg *argv)
{
	struct tlist *tl_items = argv[1].p;

	widget_set_int(tl_items->vbar, "value", 0);
}

static void
tl_items_poll(int argc, union evarg *argv)
{
	char name[OBJECT_NAME_MAX + 96];
	struct tlist *tl = argv[0].p;
	struct tlist *tl_medias = argv[1].p;
	struct tlist_item *it_media;
	struct art *art;
	Uint32 i;
	int j;

	it_media = tlist_item_selected(tl_medias);
	if (it_media == NULL) {
		return;		/* No selection */
	}
	art = it_media->p1;

	tlist_clear_items(tl);

	for (i = 0; i < art->nsprites; i++) {			/* Sprites */
		SDL_Surface *su = art->sprites[i];
		struct art_spritecl *spritecl = &art->csprites[i];
		struct art_cached_sprite *csprite;

		snprintf(name, sizeof(name),
		    "%s:%u (%ux%u)", art->name, i, su->w, su->h);
		tlist_insert_item(tl, su, name, su);

		j = 0;
		SLIST_FOREACH(csprite, &spritecl->sprites, sprites) {
			snprintf(name, sizeof(name),
			    "%s:%u[%u] (%ux%u) - %u", art->name, i, j,
			     su->w, su->h, csprite->last_drawn);
			tlist_insert_item(tl, csprite->su, name, csprite);
			j++;
		}
	}

	for (i = 0; i < art->nanims; i++) {			/* Anims */
		struct art_anim *anim = art->anims[i];
		SDL_Surface *su = NULL;

		if (anim->nframes > 0) {
			su = anim->frames[0];
			snprintf(name, sizeof(name), "%s:%u (%ux%u)",
			    art->name, i, su->w, su->h);
		} else {
			snprintf(name, sizeof(name), "%s:%u", art->name, i);
		}
		tlist_insert_item(tl, su, name, anim);
	}
	
	for (i = 0; i < art->nsubmaps; i++) {			/* Submaps */
		struct map *submap = art->submaps[i];

		snprintf(name, sizeof(name), "%s:%u (%ux%u)", art->name, i,
		    submap->mapw, submap->maph);
		tlist_insert_item(tl, NULL, name, submap);
	}
	tlist_restore_selections(tl);
}

struct window *
art_browser_window(void)
{
	struct window *win;
	struct region *reg;
	struct tlist *tl_medias;

	if ((win = window_generic_new(348, 216, "monitor-art-browser"))
	    == NULL) {
		return (NULL);	/* Exists */
	}
	window_set_min_geo(win, 246, 109);
	window_set_caption(win, "Graphics");

	reg = region_new(win, REGION_HALIGN, 0, 0, 40, 100);
	{
		tl_medias = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl_medias, "tlist-poll", tl_medias_poll, NULL);
	}
	reg = region_new(win, REGION_VALIGN, 40, 0, 60, 100);
	{
		struct tlist *tl_items;
	
		tl_items = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl_items, "tlist-poll",
		    tl_items_poll, "%p", tl_medias);

		event_new(tl_medias, "tlist-changed", tl_medias_selected,
		    "%p", tl_items);
	}
	return (win);
}

#endif	/* DEBUG */


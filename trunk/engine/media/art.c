/*	$Csoft: art.c,v 1.11 2002/12/30 06:31:04 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <engine/compat/asprintf.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/config.h>

#include "xcf.h"

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/bitmap.h>

#include <libfobj/fobj.h>

enum {
	NANIMS_INIT =	1,
	NSPRITES_INIT =	1,
	NANIMS_GROW =	4,
	NSPRITES_GROW = 4,
	FRAMES_INIT =	2,
	FRAMES_GROW =	8
};

/* XXX use a hash table or a tree. */
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

/* Insert a sprite of arbitrary size, return the assigned sprite index. */
int
art_insert_sprite(struct art *art, SDL_Surface *sprite, int map)
{
	/* Insert into the sprite table. */
	if (art->sprites == NULL) {			/* Initialize */
		art->maxsprites = NSPRITES_INIT;
		art->sprites = emalloc(art->maxsprites * sizeof(SDL_Surface *));
		art->nsprites = 0;
	} else if (art->nsprites >= art->maxsprites) {	/* Grow */
		art->maxsprites += NSPRITES_GROW;
		art->sprites = erealloc(art->sprites,
		    art->maxsprites * sizeof(SDL_Surface *));
	}
	art->sprites[art->nsprites++] = sprite;
	return (art->cursprite++);
}

/* Break a sprite into tile-sized fragments. */
void
art_insert_sprite_tiles(struct art *art, SDL_Surface *sprite)
{
	int x, y, mw, mh;
	SDL_Rect sd, rd;

	sd.w = TILEW;
	sd.h = TILEH;
	rd.x = 0;
	rd.y = 0;
	mw = sprite->w/TILEW;
	mh = sprite->h/TILEH;

	for (y = 0; y < sprite->h; y += TILEH) {
		for (x = 0; x < sprite->w; x += TILEW) {
			SDL_Surface *s;

			/* Allocate a surface for the fragment. */
			s = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
			    TILEW, TILEH, 32,
			    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
			if (s == NULL) {
				fatal("SDL_CreateRGBSurface: %s\n",
				    SDL_GetError());
			}

			/* Copy the fragment as is. */
			SDL_SetAlpha(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(sprite, &sd, s, &rd);
			art_insert_sprite(art, s, 0);
			SDL_SetAlpha(sprite, SDL_SRCALPHA,
			    SDL_ALPHA_TRANSPARENT);
		}
	}
}

void
art_unused(struct art *art)
{
	pthread_mutex_lock(&art->used_lock);
	if (--art->used == 0) {
		art_destroy(art);
	} else if (art->used < 0) {
		fatal("ref count < 0\n");
	}
	pthread_mutex_unlock(&art->used_lock);
}

/* Load artwork when an object is initialized. */
struct art *
art_fetch(char *archive, struct object *ob)
{
	struct art *art;
	struct fobj *fob;
	char *path, *s;
	Uint32 i;

	pthread_mutex_lock(&artq_lock);

	TAILQ_FOREACH(art, &artq, arts) {			/* Cached? */
		if (strcmp(art->name, archive) == 0) {
			pthread_mutex_unlock(&artq_lock);
			return (art);
		}
	}

	/* Load the data file. */
	path = object_path(archive, "fob");
	if (path == NULL) {
		if (ob->flags & OBJECT_ART_CAN_FAIL) {
			ob->flags &= ~(OBJECT_ART);
			pthread_mutex_unlock(&artq_lock);
			return (NULL);
		} else {
			fatal("%s: %s\n", archive, error_get());
		}
	}

	art = emalloc(sizeof(struct art));
	art->name = Strdup(archive);
	art->sprites = NULL;
	art->nsprites = 0;
	art->maxsprites = 0;
	art->anims = NULL;
	art->nanims = 0;
	art->maxanims = 0;
	art->used = 1;
	art->pobj = ob;		/* XXX */
	art->cursprite = 0;
	art->curanim = 0;
	art->tiles.map = NULL;

	pthread_mutex_init(&art->used_lock, NULL);

	/* Create a tile map for level edition purposes. */
	if (prop_get_bool(config, "object.art.map-tiles")) {
		char *mapname;

		art->tiles.map = emalloc(sizeof(struct map));
		Asprintf(&mapname, "t-%s", archive);
		map_init(art->tiles.map, MAP_2D, mapname, NULL);
		free(mapname);
		map_alloc_nodes(art->tiles.map, 1, 1);
	}

	/* Load images in XCF format. */
	fob = fobj_load(path);
	for (i = 0; i < fob->head.nobjs; i++) {
		if (xcf_check(fob->fd, fob->offs[i]) == 0) {
			int rv;
				
			rv = xcf_load(fob->fd, (off_t)fob->offs[i], art);
			if (rv != 0) {
				fatal("xcf_load: %s\n", error_get());
			}
		} else {
			debug(DEBUG_LOADING, "not a xcf in slot %d\n", i);
		}
	}
	fobj_free(fob);

	/* Allow other objects to reuse this artwork. */
	TAILQ_INSERT_HEAD(&artq, art, arts);

	pthread_mutex_unlock(&artq_lock);
	return (art);
}

static void
art_destroy(struct art *art)
{
	int i;
	
	pthread_mutex_lock(&artq_lock);
	TAILQ_REMOVE(&artq, art, arts);
	pthread_mutex_unlock(&artq_lock);

	for (i = 0; i < art->nsprites; i++) {
		SDL_FreeSurface(art->sprites[i]);
	}
	for (i = 0; i < art->nanims; i++) {
		art_destroy_anim(art->anims[i]);
	}
	if (art->tiles.map != NULL) {
		debug(DEBUG_GC, "freeing %s's tile map\n", art->name);
		object_destroy(art->tiles.map);
	}
			
	debug(DEBUG_GC, "freed %s (%d sprites, %d anims)\n",
	    art->name, art->nsprites, art->nanims);

	free(art->name);
	free(art);
}

/* Insert a frame into an animation. */
void
art_insert_anim_frame(struct art_anim *anim, SDL_Surface *surface)
{
	int x, y;
	SDL_Rect sd, rd;

	if (anim->frames == NULL) {			/* Initialize */
		anim->frames = emalloc(FRAMES_INIT * sizeof(SDL_Surface *));
		anim->maxframes = FRAMES_INIT;
		anim->nframes = 0;
	} else if (anim->nframes >= anim->maxframes) {	/* Grow */
		anim->maxframes += FRAMES_GROW;
		anim->frames = erealloc(anim->frames,
		    anim->maxframes * sizeof(SDL_Surface *));
	}
	anim->frames[anim->nframes++] = surface;
}

/* Create a new animation. */
struct art_anim *
art_insert_anim(struct art *art, int delay)
{
	struct art_anim *anim;

	anim = emalloc(sizeof(struct art_anim));
	anim->frames = NULL;
	anim->maxframes = 0;
	anim->frame = 0;
	anim->nframes = 0;
	anim->delta = 0;
	anim->delay = delay;

	if (art->anims == NULL) {			/* Initialize */
		art->anims = emalloc(NANIMS_INIT * sizeof(struct anim *));
		art->maxanims = NANIMS_INIT;
		art->nanims = 0;
	} else if (art->nanims >= art->maxanims) {	/* Grow */
		art->maxanims += NANIMS_GROW;
		art->anims = erealloc(art->anims,
		    art->maxanims * sizeof(struct anim *));
	}
	art->anims[art->nanims++] = anim;
	return (anim);
}

static void
art_destroy_anim(struct art_anim *anim)
{
	int i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	free(anim->frames);
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
art_get_sprite(struct object *ob, int i)
{
	if (i > ob->art->nsprites) {
		fatal("no sprite at %s:%d\n", ob->name, i);
	}
	if ((ob->flags & OBJECT_ART) == 0) {
		fatal("%s has no art\n", ob->name);
	}
	return (ob->art->sprites[i]);
}

struct art_anim *
art_get_anim(struct object *ob, int i)
{
	if ((ob->flags & OBJECT_ART) == 0 && ob->art != NULL) {
		fatal("%s has no art\n", ob->name);
	}
	if (i > ob->art->nanims) {
		fatal("no anim at %s:%d\n", ob->name, i);
	}
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
	struct tlist *tl_sprites = argv[1].p;
	struct tlist *tl_anims = argv[2].p;

	widget_set_int(tl_sprites->vbar, "value", 0);
	widget_set_int(tl_anims->vbar, "value", 0);
}

static void
tl_sprites_selected(int argc, union evarg *argv)
{
	struct tlist *tl_sprites = argv[0].p;
	struct bitmap *bmp_sprite = argv[1].p;
	struct tlist_item *it_sprite = argv[2].p;

	bitmap_set_surface(bmp_sprite, (SDL_Surface *)it_sprite->p1);
}

static void
tl_sprites_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist *tl_medias = argv[1].p;
	struct tlist_item *it_media;
	struct art *art;
	int i;

	it_media = tlist_item_selected(tl_medias);
	if (it_media == NULL) {
		return;		/* No selection */
	}
	art = it_media->p1;

	tlist_clear_items(tl);
	for (i = 0; i < art->nsprites; i++) {
		SDL_Surface *su = art->sprites[i];
		char *s;

		Asprintf(&s, "%s:%d (%dx%d)", art->name, i, su->w, su->h);
		tlist_insert_item(tl, su, s, su);
		free(s);
	}
	tlist_restore_selections(tl);
}

static void
tl_anims_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist *tl_medias = argv[1].p;
	struct tlist_item *it_media;
	struct art *art;
	int i;

	it_media = tlist_item_selected(tl_medias);
	if (it_media == NULL) {
		return;		/* No selection */
	}
	art = it_media->p1;

	tlist_clear_items(tl);
	for (i = 0; i < art->nanims; i++) {
		struct art_anim *anim = art->anims[i];
		SDL_Surface *su = NULL;
		char *s;

		if (anim->nframes > 0) {
			su = anim->frames[0];
			Asprintf(&s, "%s:%d (%dx%d)", art->name, i,
			    su->w, su->h);
		} else {
			Asprintf(&s, "%s:%d", art->name, i);
		}
		tlist_insert_item(tl, su, s, anim);
		free(s);
	}
	tlist_restore_selections(tl);
}

static void
tl_anims_selected(int argc, union evarg *argv)
{
	struct tlist *tl_anims = argv[0].p;
	struct bitmap *bmp_anim = argv[1].p;
	struct tlist_item *it_anim = argv[2].p;
	struct art_anim *anim = it_anim->p1;

	if (anim->nframes > 0) {
		bitmap_set_surface(bmp_anim, anim->frames[0]);
	}
}

struct window *
art_browser_window(void)
{
	struct window *win;
	struct region *reg;
	struct tlist *tl_medias;
	struct bitmap *bmp_sprite, *bmp_anim;

	if ((win = window_generic_new(512, 246, "monitor-media-browser"))
	    == NULL) {
		return (NULL);	/* Exists */
	}
	window_set_caption(win, "Art browser");

	/* Art entries */
	reg = region_new(win, 0, 0, 0, 30, 100);
	{
		tl_medias = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl_medias, "tlist-poll", tl_medias_poll, NULL);
	}
	
	/* Bitmap display */
	reg = region_new(win, REGION_VALIGN, 70, 0, 30, 100);
	{
		bmp_sprite = bitmap_new(reg, NULL, 100, 50);
		bmp_anim = bitmap_new(reg, NULL, 100, 50);
	}

	/* Sprites/animation lists */
	reg = region_new(win, REGION_VALIGN, 30, 0, 40, 100);
	{
		struct tlist *tl_sprites, *tl_anims;
	
		tl_sprites = tlist_new(reg, 100, 50, TLIST_POLL);
		event_new(tl_sprites, "tlist-changed",
		    tl_sprites_selected, "%p", bmp_sprite);
		event_new(tl_sprites, "tlist-poll",
		    tl_sprites_poll, "%p", tl_medias);
		
		tl_anims = tlist_new(reg, 100, 50, TLIST_POLL);
		event_new(tl_anims, "tlist-changed",
		    tl_anims_selected, "%p", bmp_anim);
		event_new(tl_anims, "tlist-poll",
		    tl_anims_poll, "%p", tl_medias);
	
		event_new(tl_medias, "tlist-changed", tl_medias_selected,
		    "%p, %p", tl_sprites, tl_anims);
	}

	return (win);
}

#endif	/* DEBUG */


/*	$Csoft: media.c,v 1.5 2002/11/22 08:56:49 vedge Exp $	*/

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

#include "engine.h"

#include <sys/stat.h>

#include <libfobj/fobj.h>

#include "map.h"
#include "oldxcf.h"
#include "anim.h"

enum {
	NANIMS_INIT =	4,
	NSPRITES_INIT =	4,
	NANIMS_GROW =	2,
	NSPRITES_GROW = 2
};

static LIST_HEAD(, media_art) artsh =	LIST_HEAD_INITIALIZER(&artsh);
static LIST_HEAD(, media_audio) audiosh = LIST_HEAD_INITIALIZER(&audiosh);
static pthread_mutex_t media_lock = PTHREAD_MUTEX_INITIALIZER;
static SDL_TimerID gctimer;

extern int mapediting;

void
media_break_sprite(struct media_art *art, SDL_Surface *sprite)
{
	int x, y, mw, mh;
	SDL_Rect sd, rd;
	struct map *m = art->map;

	sd.w = TILEW;
	sd.h = TILEH;
	rd.x = 0;
	rd.y = 0;
	mw = sprite->w/TILEW;
	mh = sprite->h/TILEH;

	art->my++;
	for (y = 0; y < sprite->h; y += TILEH, art->my++) {
		for (x = 0, art->mx = 0; x < sprite->w; x += TILEW, art->mx++) {
			SDL_Surface *s;

			s = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
			    TILEW, TILEH, 32,
			    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
			if (s == NULL) {
				fatal("SDL_CreateRGBSurface: %s\n",
				    SDL_GetError());
			}

			SDL_SetAlpha(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(sprite, &sd, s, &rd);
			media_add_sprite(art, s, MAPREF_SAVE|MAPREF_SPRITE, 0);
			SDL_SetAlpha(sprite, SDL_SRCALPHA,
			    SDL_ALPHA_TRANSPARENT);

			if (mapediting) {
				struct node *n;
			
				if (art->mx > m->mapw) {	/* XXX pref */
					art->mx = 0;
				}
				map_adjust(m, (Uint32)art->mx, (Uint32)art->my);
				n = &m->map[art->my][art->mx];
				node_addref(n, art->pobj, art->cursprite++,
				    MAPREF_SAVE|MAPREF_SPRITE);
				n->flags = NODE_BLOCK;
			}
		}
	}
	SDL_FreeSurface(sprite);
}

void
media_add_sprite(struct media_art *art, SDL_Surface *sprite, Uint32 mflags,
    int map_tiles)
{
	if (art->sprites == NULL) {			/* Initialize */
		art->sprites = emalloc(NSPRITES_INIT * sizeof(SDL_Surface *));
		art->maxsprites = NSPRITES_INIT;
		art->nsprites = 0;
	} else if (art->nsprites >= art->maxsprites) {	/* Grow */
		SDL_Surface **newsprites;

		newsprites = erealloc(art->sprites,
		    (NSPRITES_GROW * art->maxsprites) * sizeof(SDL_Surface *));
		art->maxsprites += NSPRITES_GROW;
		art->sprites = newsprites;
	}
	art->sprites[art->nsprites++] = sprite;

	if (map_tiles && mapediting) { 
		struct node *n;

		if (++art->mx > 2) {	/* XXX pref */
			art->mx = 0;
			art->my++;
		}
		map_adjust(art->map, (Uint32)art->mx, (Uint32)art->my);
		n = &art->map->map[art->my][art->mx];
		node_addref(n, art->pobj, art->cursprite++, mflags);
		n->flags = NODE_BLOCK;
	}
}


/* Load images into the media pool. */
struct media_art *
media_get_art(char *media, struct object *ob)
{
	struct media_art *art = NULL, *eart;

	pthread_mutex_lock(&media_lock);
	LIST_FOREACH(eart, &artsh, arts) {
		if (strcmp(eart->name, media) == 0) {
			art = eart;	/* Already in pool */
		}
	}

	if (art == NULL) {
		struct fobj *fob;
		char *obpath;
		Uint32 i;
	
		obpath = object_path(media, "fob");
		if (obpath == NULL) {
			if (ob->flags & OBJECT_MEDIA_CAN_FAIL) {
				ob->flags &= ~(OBJECT_ART);
				goto done;
			} else {
				fatal("%s: %s\n", media, error_get());
			}
		}

		art = emalloc(sizeof(struct media_art));
		art->name = strdup(media);
		art->sprites = NULL;
		art->nsprites = 0;
		art->maxsprites = 0;
		art->anims = NULL;
		art->nanims = 0;
		art->maxanims = 0;
		art->used = 1;
		art->map = NULL;
		art->mx = -1;
		art->my = 0;
		art->pobj = ob;		/* XXX */
		art->cursprite = 0;
		art->curanim = 0;
		art->curflags = 0;
		pthread_mutex_init(&art->used_lock, NULL);

		if (mapediting) {
			char s[FILENAME_MAX];

			art->map = emalloc(sizeof(struct map));
			sprintf(s, "t-%s", media);
			map_init(art->map, s, NULL, 0);
			map_allocnodes(art->map, 1, 1);
		}

		fob = fobj_load(obpath);
		for (i = 0; i < fob->head.nobjs; i++) {
			if (xcf_check(fob->fd, fob->offs[i]) == 0) {
				int rv;
				
				rv = xcf_load(fob->fd, (off_t)fob->offs[i],
				    art);
				if (rv != 0) {
					fatal("xcf_load: %s\n", error_get());
				}
			} else {
				dprintf("not a xcf in slot %d\n", i);
			}
		}
		fobj_free(fob);
		
		LIST_INSERT_HEAD(&artsh, art, arts);
	}
done:
	pthread_mutex_unlock(&media_lock);

	return (art);
}

/* Load audio into the media pool. */
struct media_audio *
media_get_audio(char *media, struct object *ob)
{
	struct media_audio *audio = NULL, *eaudio;

	pthread_mutex_lock(&media_lock);

	LIST_FOREACH(eaudio, &audiosh, audios) {
		if (strcmp(eaudio->name, media) == 0) {
			audio = eaudio;	/* Already in pool */
		}
	}

	if (audio == NULL) {
		struct fobj *fob;
		Uint32 i;
		char *obpath;

		audio = emalloc(sizeof(struct media_audio));
		audio->name = strdup(media);
		/* XXX todo */
		audio->samples = NULL;
		audio->nsamples = 0;
		audio->maxsamples = 0;
		audio->used = 0;

		obpath = object_path(media, "fob");
		if (obpath == NULL) {
			if (ob->flags & OBJECT_MEDIA_CAN_FAIL) {
				ob->flags &= ~(OBJECT_AUDIO);
				goto done;
			} else {
				fatal("%s: %s\n", media, error_get());
			}
		}

		fob = fobj_load(obpath);
		for (i = 0; i < fob->head.nobjs; i++) {	/* XXX todo */
			/* ... */
		}
		fobj_free(fob);
		
		LIST_INSERT_HEAD(&audiosh, audio, audios);
	}
done:
	pthread_mutex_unlock(&media_lock);
	return (audio);
}

Uint32
media_start_gc(Uint32 ival, void *p)
{
	struct media_audio *audio;
	struct media_art *art, *nextart;

	pthread_mutex_lock(&media_lock);

	/* Art pool */
	for (art = LIST_FIRST(&artsh);
	     art != LIST_END(&artsh);
	     art = nextart) {
		nextart = LIST_NEXT(art, arts);
		if (art->used < 1) {
			int i;

			for (i = 0; i < art->nsprites; i++) {
				free(art->sprites[i]);
			}
			for (i = 0; i < art->nanims; i++) {
				anim_destroy(art->anims[i]);
			}
#if 0
			if (art->map != NULL) {
				map_destroy(art->map);
				free(art->map);
			}
#endif
			dprintf("freed: %s (%d sprites, %d anims)\n",
			    art->name, art->nsprites, art->nanims);

			free(art->name);
			free(art);
		}
	}

	/* Audio pool */
	LIST_FOREACH(audio, &audiosh, audios) {
		if (audio->used < 1) {
			/* gc */
			dprintf("gc audio\n");
		}
	}

	pthread_mutex_unlock(&media_lock);
	return (ival);
}

void
media_init_gc(void)
{
	gctimer = SDL_AddTimer(1000, media_start_gc, NULL);
}

void
media_destroy_gc(void)
{
	SDL_RemoveTimer(gctimer);
}

#ifdef DEBUG

SDL_Surface *
media_sprite(struct object *ob, int i)
{
	if (i > ob->art->nsprites) {
		fatal("no sprite at %s:%d\n", ob->name, i);
	}
	if ((ob->flags & OBJECT_ART) == 0) {
		fatal("%s has no art\n", ob->name);
	}
	return (ob->art->sprites[i]);
}

struct anim *
media_anim(struct object *ob, int i)
{
	if ((ob->flags & OBJECT_ART) == 0 && ob->art != NULL) {
		fatal("%s has no art\n", ob->name);
	}
	if (i > ob->art->nanims) {
		fatal("no anim at %s:%d\n", ob->name, i);
	}
	return (ob->art->anims[i]);
}

struct media_audio_sample *
media_sample(struct object *ob, int i)
{
	if ((ob->flags & OBJECT_AUDIO) == 0 && ob->audio != NULL) {
		fatal("%s has no audio\n", ob->name);
	}
	if (i > ob->audio->nsamples) {
		fatal("no sample at %s:%d\n", ob->name, i);
	}
	return (ob->audio->samples[i]);
}

#endif

/*	$Csoft: anim.c,v 1.19 2002/11/22 05:41:40 vedge Exp $	*/

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
#include "map.h"
#include "anim.h"

#include "compat/asprintf.h"

static const struct object_ops anim_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

enum {
	FRAMES_INIT = 16,
	FRAMES_GROW = 2,
	ANIMS_INIT = 4,
	ANIMS_GROW = 2
};

int
anim_add_frame(struct anim *anim, SDL_Surface *surface)
{
	int i, x, y;
	SDL_Rect sd, rd;

	anim->w = (surface->w / TILEW);
	anim->h = (surface->h / TILEH);
	anim->nparts = anim->w + anim->h - 1;

	if (anim->frames == NULL) {			/* Initialize */
		anim->frames = emalloc(anim->nparts * sizeof(SDL_Surface *));
		for (i = 0; i < anim->nparts; i++) {
			*(anim->frames + i) =
			    emalloc(FRAMES_INIT * sizeof(struct SDL_Surface *));
		}
		anim->maxframes = FRAMES_INIT;
		anim->nframes = 0;
	} else if (anim->nframes >= anim->maxframes) {	/* Grow */
		for (i = 0; i < anim->nparts; i++) {
			SDL_Surface **su;

			su = erealloc(*(anim->frames + i),
			    (anim->maxframes + FRAMES_GROW) *
			    sizeof(struct SDL_Surface *));
			*(anim->frames + i) = su;
		}
		anim->maxframes += FRAMES_GROW;
	}
	
	sd.w = TILEW;
	sd.h = TILEH;
	rd.x = 0;
	rd.y = 0;
	
	for (y = 0, i = 0; y < surface->h; y += TILEH) {
		for (x = 0; x < surface->w; x += TILEW) {
			SDL_Surface *s;

			s = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
			    TILEW, TILEH, 32,
			    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
			if (s == NULL) {
				fatal("SDL_AllocSurface: %s\n", SDL_GetError());
			}

			SDL_SetAlpha(surface, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(surface, &sd, s, &rd);
			anim->frames[i++][anim->nframes] = s;
			SDL_SetAlpha(surface, SDL_SRCALPHA,
			    SDL_ALPHA_TRANSPARENT);
		}
	}

	anim->nframes++;
	return (0);
}

void
anim_insert(struct media_art *art, struct anim *anim, int mflags)
{
	extern int mapediting;

	if (art->anims == NULL) {			/* Initialize */
		art->anims = emalloc(ANIMS_INIT * sizeof(struct anim *));
		art->maxanims = ANIMS_INIT;
		art->nanims = 0;
	} else if (art->nanims >= art->maxanims) {	/* Grow */
		struct anim **newanims;

		newanims = erealloc(art->anims,
		    (ANIMS_GROW * art->maxanims) * sizeof(struct anim *));
		art->maxanims += ANIMS_GROW;
		art->anims = newanims;
	}
	art->anims[art->nanims++] = anim;

	if (mapediting) {
		struct node *n;

		if (++art->mx > 2) {	/* XXX pref */
			art->mx = 0;
			art->my++;
		}
		map_adjust(art->map, art->mx, art->my);
		n = &art->map->map[art->my][art->mx];
		node_addref(n, art->pobj, art->curanim++, mflags);
		n->flags = NODE_BLOCK;
	}
}

void
anim_init(struct anim *anim, int delay)
{
	static int curanim = 0;
	static pthread_mutex_t curanim_lock = PTHREAD_MUTEX_INITIALIZER;
	char *aniname;

	pthread_mutex_lock(&curanim_lock);
	asprintf(&aniname, "anim%d", curanim++);
	object_init(&anim->obj, "anim", aniname, NULL, 0, &anim_ops);
	free(aniname);
	pthread_mutex_unlock(&curanim_lock);

	anim->frames = NULL;
	anim->maxframes = 0;
	anim->w = -1;
	anim->h = -1;
	anim->frame = 0;
	anim->nframes = 0;
	anim->nparts = 0;
	anim->delta = 0;
	anim->delay = delay;
}

void
anim_destroy(struct anim *anim)
{
	int i, j;

	for (j = 0; j < anim->nframes; j++) {
		for (i = 0; i < anim->nparts; i++) {
			SDL_FreeSurface(anim->frames[i][j]);
		}
#if 0
		free(*(anim->frames + j));
#endif
	}
	free(anim->frames);
	free(anim);
}


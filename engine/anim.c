/*	$Csoft: anim.c,v 1.11 2002/06/09 15:04:29 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "map.h"

enum {
	NFINIT = 16,	/* Pointers allocated at initialization. */
	NFGROW = 2	/* Pointers allocated at growth. */
};

int
anim_addframe(struct anim *anim, SDL_Surface *surface)
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
			    emalloc(NFINIT * sizeof(struct SDL_Surface *));
		}
		anim->maxframes = NFINIT;
		anim->nframes = 0;
	} else if (anim->nframes >= anim->maxframes) {	/* Grow */
		for (i = 0; i < anim->nparts; i++) {
			SDL_Surface **su;

			su = erealloc(*(anim->frames + i),
			    NFGROW * anim->maxframes *
			    sizeof(struct SDL_Surface *));
			*(anim->frames + i) = su;
		}
		anim->maxframes *= NFGROW;
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
anim_init(struct anim *anim, int delay)
{
	anim->frames = NULL;
	anim->maxframes = 0;
	anim->frame = 0;
	anim->nframes = 0;
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


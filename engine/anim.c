/*	$Csoft: anim.c,v 1.7 2002/04/02 19:55:13 vedge Exp $	*/

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

#include <engine/engine.h>

enum {
	NFINIT = 1,	/* Pointers allocated at initialization. */
	NFGROW = 2	/* Pointers allocated at growth. */
};

int
anim_addframe(struct anim *anim, SDL_Surface *surface)
{
	if (anim->frames == NULL) {			/* Initialize */
		anim->frames = (SDL_Surface **)emalloc(NFINIT *
		    sizeof(SDL_Surface *));
		anim->maxframes = NFINIT;
		anim->nframes = 0;
	} else if (anim->nframes >= anim->maxframes) {	/* Grow */
		SDL_Surface **newframes;

		newframes = (SDL_Surface **)realloc(anim->frames,
		    (NFGROW * anim->maxframes) * sizeof(SDL_Surface *));
		if (newframes == NULL) {
			dperror("realloc");
			return (-1);
		}
		anim->maxframes *= NFGROW;
		anim->frames = newframes;
	}
	anim->frames[anim->nframes++] = surface;
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
	int i;

	for (i = 0; i < anim->nframes; i++) {
		SDL_FreeSurface(anim->frames[i]);
	}
	free(anim->frames);
	free(anim);
}


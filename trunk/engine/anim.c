/*	$Csoft: anim.c,v 1.2 2002/02/08 00:18:01 vedge Exp $	*/

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
		anim->frames = (SDL_Surface **)malloc(NFINIT *
		    sizeof(SDL_Surface *));
		if (anim->frames == NULL) {
			perror("malloc");
			return (-1);
		}
		anim->maxframes = NFINIT;
		anim->nframes = 0;
	} else if (anim->nframes >= anim->maxframes) {	/* Grow */
		SDL_Surface **newframes;

		newframes = (SDL_Surface **)realloc(anim->frames,
		    (NFGROW * anim->maxframes) * sizeof(SDL_Surface *));
		if (newframes == NULL) {
			perror("realloc");
			return (-1);
		}
		anim->maxframes *= NFGROW;
		anim->frames = newframes;
	}
	anim->frames[anim->nframes++] = surface;
	return (0);
}

struct anim *
anim_create(int delay)
{
	struct anim *anim;

	anim = malloc(sizeof(struct anim));
	if (anim == NULL) {
		return (NULL);
	}

	anim->frames = NULL;
	anim->nframes = 0;
	anim->delay = delay;

	return (anim);
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


/*	$Csoft$	*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <engine/engine.h>

struct anim *
anim_create(int delay)
{
	struct anim *anim;

	anim = malloc(sizeof(struct anim));
	if (anim == NULL) {
		return (NULL);
	}

#if 0
	SLIST_INIT(&anim->framesh);
#endif
	anim->frames = NULL;
	anim->nframes = 0;
	anim->delay = delay;
	anim->gframe = 0;
	anim->gframedc = 0;
	if (pthread_mutex_init(&anim->lock, NULL) != 0) {
		return (NULL);
	}

	return (anim);
}

void
anim_destroy(struct anim *anim)
{
#if 0
	struct frame *f;

	SLIST_FOREACH(&anim->frames, f, frames);
#endif

	pthread_mutex_destroy(&anim->lock);
	free(anim);
}


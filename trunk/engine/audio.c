/*	$Csoft: audio.c,v 1.3 2003/06/21 06:39:44 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include <engine/loader/den.h>
#include <engine/loader/xcf.h>

enum {
	NSAMPLES_INIT =	1,
	NSAMPLES_GROW =	4
};

static TAILQ_HEAD(, audio) audioq = TAILQ_HEAD_INITIALIZER(audioq);
pthread_mutex_t		   audioq_lock = PTHREAD_MUTEX_INITIALIZER;

static void	audio_destroy(struct audio *);
static void	audio_destroy_sample(struct sample *);

/* Insert a new sample. */
Uint32
audio_insert_sample(struct audio *audio, SDL_AudioSpec *spec, Uint8 *data,
    size_t size)
{
	struct sample *nsamp;

	if (audio->samples == NULL) {				/* Initialize */
		audio->samples = Malloc(NSAMPLES_INIT * sizeof(struct sample));
		audio->maxsamples = NSAMPLES_INIT;
		audio->nsamples = 0;
	} else if (audio->nsamples+1 > audio->maxsamples) {	/* Grow */
		audio->maxsamples += NSAMPLES_GROW;
		audio->samples = Realloc(audio->samples,
		    audio->maxsamples * sizeof(struct sample));
	}

	nsamp = &audio->samples[audio->nsamples];
	nsamp->spec = *spec;
	nsamp->data = data;
	nsamp->size = size;
	return (audio->nsamples++);
}

/* Disable garbage collection. */
void
audio_wire(struct audio *audio)
{
	pthread_mutex_lock(&audio->used_lock);
	audio->used = AUDIO_MAX_USED;
	pthread_mutex_unlock(&audio->used_lock);
}

/* Decrement the reference count. */
void
audio_unused(struct audio *audio)
{
	pthread_mutex_lock(&audio->used_lock);
	if (audio->used != AUDIO_MAX_USED &&		/* Remain resident? */
	    --audio->used == 0) {
		pthread_mutex_unlock(&audio->used_lock);
		audio_destroy(audio);
		return;
	}
	pthread_mutex_unlock(&audio->used_lock);
}

/* Load the named audio package. */
struct audio *
audio_fetch(void *p, const char *key)
{
	char *path = NULL;
	struct audio *audio = NULL;
	struct den *den;

	pthread_mutex_lock(&audioq_lock);

	TAILQ_FOREACH(audio, &audioq, audios) {
		if (strcmp(audio->name, key) == 0)
			break;
	}
	if (audio != NULL) {				/* Cached? */
		if (++audio->used > AUDIO_MAX_USED) {
			audio->used = AUDIO_MAX_USED; 	/* Remain resident */
		}
		goto out;
	}

	if ((path = config_search_file("load-path", key, "den")) == NULL)
		fatal("searching %s: %s", key, error_get());

	audio = Malloc(sizeof(struct audio));
	audio->name = Strdup(key);
	audio->samples = NULL;
	audio->nsamples = 0;
	audio->maxsamples = 0;
	audio->used = 1;
	pthread_mutex_init(&audio->used_lock, NULL);

	if ((den = den_open(path, DEN_READ)) == NULL) {
		fatal("loading %s: %s", path, error_get());
	}
#if 0
	for (i = 0; i < den->nmembers; i++) {
		if (xcf_load(den->buf, den->members[i].offs, audio) == -1)
			fatal("loading xcf #%d: %s", i, error_get());
	}
#endif
	den_close(den);

	TAILQ_INSERT_HEAD(&audioq, audio, audios);		/* Cache */
out:
	pthread_mutex_unlock(&audioq_lock);
	Free(path);
	return (audio);
}

/* Release a group of audio samples. */
static void
audio_destroy(struct audio *audio)
{
	Uint32 i;

	pthread_mutex_lock(&audioq_lock);
	TAILQ_REMOVE(&audioq, audio, audios);
	pthread_mutex_unlock(&audioq_lock);

	/* Release the samples. */
	for (i = 0; i < audio->nsamples; i++) {
		audio_destroy_sample(&audio->samples[i]);
	}
	Free(audio->samples);

	dprintf("freed %s (%d samples)\n", audio->name, audio->nsamples);

	pthread_mutex_destroy(&audio->used_lock);
	free(audio->name);
	free(audio);
}

static void
audio_destroy_sample(struct sample *samp)
{
	free(samp->data);
}

#ifdef DEBUG
struct sample *
audio_get_sample(struct object *ob, Uint32 i)
{
	if (ob->audio == NULL)
		fatal("no audio in %s", ob->name);
	if (i > ob->audio->nsamples)
		fatal("no sample at %s:%d", ob->name, i);
	return (&ob->audio->samples[i]);
}
#endif /* DEBUG */

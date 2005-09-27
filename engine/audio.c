/*	$Csoft: audio.c,v 1.16 2005/07/16 16:07:27 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <string.h>

enum {
	NSAMPLES_INIT =	1,
	NSAMPLES_GROW =	4
};

static TAILQ_HEAD(, ag_audio) ag_audioq = TAILQ_HEAD_INITIALIZER(ag_audioq);
pthread_mutex_t		      ag_audioq_lock = PTHREAD_MUTEX_INITIALIZER;

/* Insert a new sample. */
Uint32
AG_AudioInsertSample(AG_Audio *audio, SDL_AudioSpec *spec, Uint8 *data,
    size_t size)
{
	AG_AudioSample *nsamp;

	if (audio->samples == NULL) {
		audio->samples = Malloc(NSAMPLES_INIT*sizeof(AG_AudioSample),
		    M_AUDIO);
		audio->maxsamples = NSAMPLES_INIT;
		audio->nsamples = 0;
	} else if (audio->nsamples+1 > audio->maxsamples) {
		audio->maxsamples += NSAMPLES_GROW;
		audio->samples = Realloc(audio->samples,
		    audio->maxsamples*sizeof(AG_AudioSample));
	}

	nsamp = &audio->samples[audio->nsamples];
	nsamp->spec = *spec;
	nsamp->data = data;
	nsamp->size = size;
	return (audio->nsamples++);
}

/* Disable garbage collection. */
void
AG_AudioWire(AG_Audio *audio)
{
	audio->used = AG_AUDIO_MAX_USED;
}

/*
 * Return a pointer to the named audio package.
 * If the package is resident, increment the reference count.
 * Otherwise, load the package from disk.
 */
AG_Audio *
AG_AudioFetch(const char *name)
{
	char path[MAXPATHLEN];
	AG_Audio *audio;
	AG_Den *den;

	pthread_mutex_lock(&ag_audioq_lock);

	TAILQ_FOREACH(audio, &ag_audioq, audios) {
		if (strcmp(audio->name, name) == 0)
			break;
	}
	if (audio != NULL) {
		if (++audio->used > AG_AUDIO_MAX_USED) {
			audio->used = AG_AUDIO_MAX_USED;
		}
		goto out;
	}

	if (AG_ConfigFile("load-path", name, "den", path, sizeof(path)) == -1)
		goto fail;

	audio = Malloc(sizeof(AG_Audio), M_AUDIO);
	audio->name = Strdup(name);
	audio->samples = NULL;
	audio->nsamples = 0;
	audio->maxsamples = 0;
	audio->used = 1;

	if ((den = AG_DenOpen(path, AG_DEN_READ)) == NULL)
		goto fail;
#if 0
	for (i = 0; i < den->nmembers; i++) {
		if (wav_load(den->buf, den->members[i].offs, audio) == -1)
			fatal("loading wav #%d: %s", i, AG_GetError());
	}
	for (i = 0; i < den->nmembers; i++) {
		if (ogg_load(den->buf, den->members[i].offs, audio) == -1)
			fatal("loading ogg #%d: %s", i, AG_GetError());
	}
#endif
	AG_DenClose(den);

	TAILQ_INSERT_HEAD(&ag_audioq, audio, audios);
out:
	pthread_mutex_unlock(&ag_audioq_lock);
	return (audio);
fail:
	pthread_mutex_unlock(&ag_audioq_lock);
	if (audio != NULL) {
		Free(audio->name, 0);
		Free(audio, M_AUDIO);
	}
	return (NULL);
}

/* Release an audio package that is no longer in use. */
void
AG_AudioDestroy(AG_Audio *audio)
{
	Uint32 i;

	pthread_mutex_lock(&ag_audioq_lock);
	TAILQ_REMOVE(&ag_audioq, audio, audios);
	pthread_mutex_unlock(&ag_audioq_lock);

	for (i = 0; i < audio->nsamples; i++)
		Free(audio->samples[i].data, M_AUDIO);

	Free(audio->name, 0);
	Free(audio->samples, M_AUDIO);
	Free(audio, M_AUDIO);
}


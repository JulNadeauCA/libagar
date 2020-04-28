/*
 * Copyright (c) 2011-2020 Julien Nadeau Carriere (vedge@csoft.net).
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

/*
 * Audio clip structure.
 */

#include <agar/config/have_sndfile.h>

#include <agar/core/core.h>
#include <agar/au/au_wave.h>

#include <string.h>
#include <math.h>

AU_Wave *
AU_WaveNew(void)
{
	AU_Wave *w;

	w = Malloc(sizeof(AU_Wave));
	w->flags = 0;
	w->nFrames = 0;
	w->frames = NULL;
	w->peak = 0.0;
	w->ch = 0;
#ifdef HAVE_SNDFILE
	w->vizFrames = NULL;
	w->nVizFrames = 0;
	w->file = NULL;
	memset(&w->info, 0, sizeof(w->info));
#endif
	AG_MutexInitRecursive(&w->lock);
	return (w);
}

AU_Wave *
AU_WaveFromFile(const char *path)
{
	AU_Wave *w = AU_WaveNew();

	if (AU_WaveLoad(w, path) == -1) {
		AU_WaveFree(w);
		return (NULL);
	}
	return (w);
}

void
AU_WaveFreeData(AU_Wave *w)
{
	Free(w->frames);
	w->frames = NULL;
	w->nFrames = 0;
	w->peak = 0.0;
	w->ch = 0;
#ifdef HAVE_SNDFILE
	Free(w->vizFrames);
	w->vizFrames = NULL;
	w->nVizFrames = 0;
#endif
}

void
AU_WaveFree(AU_Wave *w)
{
#ifdef HAVE_SNDFILE
	if (w->file != NULL)
		sf_close(w->file);
#endif
	AU_WaveFreeData(w);
	AG_MutexDestroy(&w->lock);
	Free(w);
}

/* Load audio stream from a file. */
int
AU_WaveLoad(AU_Wave *w, const char *path)
{
#ifdef HAVE_SNDFILE
	sf_count_t nReadFrames = 0;

	if (w->file != NULL) {
		sf_close(w->file);
		AU_WaveFreeData(w);
	}
	/*
	 * Read the raw audio data.
	 */
	memset(&w->info, 0, sizeof(w->info));
	w->file = sf_open(path, SFM_READ, &w->info);
	if (w->file == NULL) {
		AG_SetError("%s: sf_open() failed", path);
		return (-1);
	}
	w->nFrames = w->info.frames;
	w->ch = w->info.channels;

	if ((w->frames = AG_TryMalloc(w->nFrames*w->ch*sizeof(float))) == NULL) {
		goto fail;
	}
	nReadFrames = 0;
	while (nReadFrames < w->nFrames) {
		sf_count_t rv;

		rv = sf_readf_float(w->file, &w->frames[nReadFrames], 4096);
		if (rv == 0) {
			break;
		}
		nReadFrames += rv;
	}
	return (0);
fail:
	AU_WaveFreeData(w);
	return (-1);
#else
	AG_SetErrorS("AU_WaveLoad() requires libsndfile (--with-sndfile)");
	return (-1);
#endif /* !HAVE_SNDFILE */
}

/* Generate a reduced waveform for visualization purposes. */
int
AU_WaveGenVisual(AU_Wave *w, int reduce)
{
#ifdef HAVE_SNDFILE
	int i, j, ch;
	float *pIn, *pViz;

	if (reduce <= 0) {
		AG_SetError("Reduction factor <= 0");
		return (-1);
	}
	if (w->vizFrames != NULL) {
		Free(w->vizFrames);
		w->nVizFrames = 0;
	}

	sf_command(w->file, SFC_CALC_SIGNAL_MAX, &w->peak, sizeof(w->peak));
	w->nVizFrames = w->nFrames/reduce;
	if ((w->vizFrames = AG_TryMalloc(w->nVizFrames*sizeof(float))) == NULL) {
		w->nVizFrames = 0;
		return (-1);
	}

	pViz = &w->vizFrames[0];
	for (i = 0; i < w->nVizFrames; i++) {
		for (ch = 0; ch < w->ch; ch++)
			*pViz++ = 0.0;
	}
	pIn = &w->frames[0];
	pViz = &w->vizFrames[0];
	for (i = 0; i < w->nVizFrames; i++) {
		for (j = 0; j < reduce; j++) {
			for (ch = 0; ch < w->ch; ch++) {
				pViz[ch] += MAX(w->vizFrames[i],
				    fabs((*pIn++)/w->peak));
			}
		}
		for (ch = 0; ch < w->ch; ch++)
			*pViz++ /= reduce;
	}
	return (0);
#else
	AG_SetErrorS("AU_WaveGenVisual() requires libsndfile (--with-sndfile)");
	return (-1);
#endif /* !HAVE_SNDFILE */
}

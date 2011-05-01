/*
 * Copyright (c) 2011 Hypertriton, Inc. <http://hypertriton.com/>
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
 * PortAudio output driver.
 */

#include <core/core.h>

#include "au_init.h"
#include "au_dev_out.h"
#include "portaudio/include/portaudio.h"

typedef struct au_dev_out_pa {
	struct au_dev_out _inherit;
	PaStream *stream;
	int wrPos;
} AU_DevOutPA;

static void
Init(void *obj)
{
	AU_DevOutPA *dpa = obj;
	PaError rv;

	dpa->stream = NULL;
	dpa->wrPos = 0;
	
	if ((rv = Pa_Initialize()) != paNoError)
		AG_FatalError("Pa_Initialize: %s", Pa_GetErrorText(rv));
}

static void
Destroy(void *obj)
{
	Pa_Terminate();
}

static int
AudioUpdatePA(const void *pIn, void *pOut, unsigned long count,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
    void *pData)
{
	AU_DevOut *dev = pData;
	AU_DevOutPA *dpa = pData;
	float *out = (float *)pOut;
	float *buf;
	int i;
	
	AG_MutexLock(&dev->lock);
	if (dev->flags & AU_DEV_OUT_CLOSING) {
		dev->flags &= ~(AU_DEV_OUT_CLOSING);
		AG_MutexUnlock(&dev->lock);
		return (paComplete);
	}
	buf = &dev->buf[dpa->wrPos*dev->ch];
	for (i = 0; i < count*dev->ch; i++) {
		*out++ = *buf++;
	}
	dpa->wrPos += count;
	if (dpa->wrPos > dev->bufSize) {
		dpa->wrPos = 0;
		dev->bufSize = 0;
	}
	AG_CondBroadcast(&dev->wrRdy);
	AG_MutexUnlock(&dev->lock);
	return (paContinue);
}

static int
Open(void *obj, const char *path, int rate, int channels)
{
	AU_DevOut *dev = obj;
	AU_DevOutPA *dpa = obj;
	PaStreamParameters op;
	PaError rv;

	if (dpa->stream != NULL) {
		AG_SetError("Audio playback already started");
		return (-1);
	}
	if ((op.device = Pa_GetDefaultOutputDevice()) == -1) {
		AG_SetError("No audio output device");
		goto fail;
	}
	op.channelCount = channels;
	op.sampleFormat = paFloat32;
	op.suggestedLatency = Pa_GetDeviceInfo(op.device)->defaultLowOutputLatency;
	op.hostApiSpecificStreamInfo = NULL;

	rv = Pa_OpenStream(
	    &dpa->stream,
	    NULL,
	    &op,
	    rate,
	    0,
	    paClipOff,
	    AudioUpdatePA, dpa);
	if (rv != paNoError) {
		AG_SetError("Failed to open playback device: %s",
		    Pa_GetErrorText(rv));
		goto fail;
	}

	dev->rate = rate;
	dev->ch = channels;

	rv = Pa_StartStream(dpa->stream);
	if (rv != paNoError) {
		AG_SetError("PortAudio error: %s", Pa_GetErrorText(rv));
		goto fail;
	}
	return (0);
fail:
	return (-1);
}

static void
Close(void *obj)
{
	AU_DevOutPA *dpa = obj;

	if (dpa->stream != NULL) {
		Pa_CloseStream(dpa->stream);
		dpa->stream = NULL;
	}
}

const AU_DevOutClass auDevOut_pa = {
	"pa",
	sizeof(AU_DevOutPA),
	Init,
	Destroy,
	Open,
	Close
};

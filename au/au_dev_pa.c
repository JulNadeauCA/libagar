/*
 * Copyright (c) 2011-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/have_portaudio.h>
#ifdef HAVE_PORTAUDIO

#include <agar/core/core.h>
#include <agar/au/au_init.h>
#include <agar/au/au_dev_out.h>

#include <portaudio.h>

typedef struct au_dev_out_pa {
	struct au_dev_out _inherit;
	PaStream *stream;
	int wrPos;
	Uint32 _pad;
	AG_Thread th;
} AU_DevOutPA;

static void
Init(void *obj)
{
	AU_DevOut *dev = obj;
	AU_DevOutPA *dpa = obj;
	PaError rv;
	
	if (Pa_GetVersion() < 1899)
		AG_FatalError("Agar-AU requires PortAudio >= v19");

	dev->flags |= AU_DEV_OUT_THREADED;
	dpa->stream = NULL;
	dpa->wrPos = 0;
	
	if ((rv = Pa_Initialize()) != paNoError) {
		AG_Verbose("Pa_Initialize: %s", Pa_GetErrorText(rv));
		AG_FatalError("PortAudio Init Failed");
	}
}

static void *
AU_DevPaThread(void *obj)
{
	AU_DevOut *dev = obj;
	AU_DevOutPA *dpa = obj;
	struct timespec ts;
	PaError rv;

	for (;;) {
		AG_MutexLock(&dev->lock);
		if (dev->flags & AU_DEV_OUT_CLOSING) {
			dev->flags &= ~(AU_DEV_OUT_CLOSING);
			AG_MutexUnlock(&dev->lock);
			return (NULL);
		}
		AG_CondBroadcast(&dev->wrRdy);
		ts.tv_sec = time(NULL) + 1;
		ts.tv_nsec = 0;
		if (AG_CondTimedWait(&dev->rdRdy, &dev->lock, &ts) == 0) {
			rv = Pa_WriteStream(dpa->stream, dev->buf, dev->bufSize);
			if (rv != paNoError) {
				printf("Pa_WriteStream: %s\n",
				    Pa_GetErrorText(rv));
			}
			dev->bufSize = 0;
		}
		AG_CondBroadcast(&dev->wrRdy);
		AG_MutexUnlock(&dev->lock);
	}
	return (NULL);
}

static void
Destroy(void *obj)
{
	Pa_Terminate();
}

static int
Open(void *obj, const char *path, int rate, int channels)
{
	AU_DevOut *dev = obj;
	AU_DevOutPA *dpa = obj;
	PaStreamParameters op;
	const PaStreamInfo *streamInfo;
	PaError rv;

	if (dpa->stream != NULL) {
		AG_SetError("Audio playback already started");
		return (-1);
	}
	if ((op.device = Pa_GetDefaultOutputDevice()) == paNoDevice) {
		AG_SetError("No default audio output device");
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
	    NULL, NULL);
	if (rv != paNoError) {
		AG_SetError("Failed to open playback device: %s",
		    Pa_GetErrorText(rv));
		goto fail;
	}
	if ((streamInfo = Pa_GetStreamInfo(dpa->stream)) != NULL) {
		dev->rate = (int)streamInfo->sampleRate;
		if (dev->rate != rate)
			Verbose("PortAudio: Hardware uses different rate (%d)\n",
			    dev->rate);
	}
	dev->rate = rate;
	dev->ch = channels;

	rv = Pa_StartStream(dpa->stream);
	if (rv != paNoError) {
		AG_SetError("PortAudio error: %s", Pa_GetErrorText(rv));
		goto fail;
	}
	if (AG_ThreadTryCreate(&dpa->th, AU_DevPaThread, dpa) != 0) {
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
#endif /* HAVE_PORTAUDIO */

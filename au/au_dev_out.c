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
 * Generic audio output interface.
 */

#include <agar/config/have_portaudio.h>
#include <agar/config/have_sndfile.h>

#include <agar/core/core.h>
#include <agar/au/au_init.h>
#include <agar/au/au_dev_out.h>

/* Available audio output drivers */
extern const AU_DevOutClass auDevOut_pa;
extern const AU_DevOutClass auDevOut_file;
const AU_DevOutClass *auDevOutList[] = {
#ifdef HAVE_PORTAUDIO
	&auDevOut_pa,
#endif
#ifdef HAVE_SNDFILE
	&auDevOut_file,
#endif
	NULL
};
const AU_DevOut *auDevOut = NULL;

/* Start audio playback / dump on the specified output device. */
AU_DevOut *
AU_OpenOut(const char *path, int rate, int ch)
{
	AU_DevOut *dev = NULL;
	char devName[128], devArgs[128], *c;
	const AU_DevOutClass **pDevCls;
	float *buf;
	int i;

	/* Parse arguments */
	Strlcpy(devName, path, sizeof(devName));
	if ((c = strchr(devName, '(')) != NULL) {
		Strlcpy(devArgs, &c[1], sizeof(devArgs));
		*c = '\0';
		if ((c = strchr(devArgs, ')')) != NULL)
			*c = '\0';
	}

	/* Locate device class */
	for (pDevCls = &auDevOutList[0]; *pDevCls != NULL; pDevCls++) {
		if (AG_Strcasecmp((*pDevCls)->name, devName) == 0)
			break;
	}
	if (*pDevCls == NULL) {
		AG_SetError("No such output driver: %s", devName);
		goto fail;
	}
	if ((dev = TryMalloc((*pDevCls)->size)) == NULL)
		return (NULL);

	/* Initialize device instance */
	dev->cls = *pDevCls;
	dev->flags = 0;
	dev->bufSize = 0;
	dev->bufMax = AU_MINBUFSIZ;
	dev->nOverruns = 0;
	dev->rate = 0;
	dev->ch = ch;
	dev->bytesPerFrame = ch*sizeof(float);

	Verbose("Audio out: %s: %dHz, %d-Ch, %d Bytes/Frame\n",
	    path, rate, ch, dev->bytesPerFrame);

	if ((buf = TryMalloc(dev->bufMax*dev->bytesPerFrame)) == NULL) {
		goto fail;
	}
	dev->buf = buf;

	for (i = 0; i < dev->bufMax*ch; i++) {
		buf[i] = 0.0f;
	}
	dev->nChan = 0;
	dev->chan = NULL;
#ifdef AG_THREADS
	AG_MutexInit(&dev->lock);
	AG_CondInit(&dev->wrRdy);
	AG_CondInit(&dev->rdRdy);
#endif
	if (dev->cls->Init != NULL) {
		dev->cls->Init(dev);
	}
	if (dev->cls->Open != NULL &&
	    dev->cls->Open(dev, devArgs, rate, ch) == -1) {
		return (NULL);
	}
	return (dev);
fail:
	Free(dev);
	return (NULL);
}

void
AU_CloseOut(AU_DevOut *dev)
{
	AG_MutexLock(&dev->lock);
	dev->flags |= AU_DEV_OUT_CLOSING;
	for (;;) {
		AG_MutexUnlock(&dev->lock);
		AG_Delay(250);
		AG_MutexLock(&dev->lock);
		if ((dev->flags & AU_DEV_OUT_CLOSING) == 0)
			break;
	}
	AG_MutexUnlock(&dev->lock);

	if (dev->cls->Close != NULL) {
		AG_MutexLock(&dev->lock);
		dev->cls->Close(dev);
		AG_MutexUnlock(&dev->lock);
	}
	if (dev->cls->Destroy != NULL) {
		dev->cls->Destroy(dev);
	}
	Free(dev->chan);
#ifdef AG_THREADS
	AG_CondDestroy(&dev->wrRdy);
	AG_CondDestroy(&dev->rdRdy);
	AG_MutexDestroy(&dev->lock);
#endif
	free(dev->buf);
	free(dev);
}

int
AU_WriteFloat(AU_DevOut *dev, float *data, Uint nFrames)
{
	AG_MutexLock(&dev->lock);
	if (dev->bufSize+nFrames > dev->bufMax) {
		float *bufNew;
		if ((bufNew = AG_TryRealloc(dev->buf,
		    (dev->bufSize + nFrames)*dev->bytesPerFrame)) == NULL) {
			AG_MutexUnlock(&dev->lock);
			return (-1);
		}
		dev->buf = bufNew;
		dev->bufMax = dev->bufSize+nFrames;
	}
	memcpy(&dev->buf[dev->bufSize*dev->ch], data, nFrames*dev->bytesPerFrame);
	dev->bufSize += nFrames;
	AG_CondBroadcast(&dev->rdRdy);
	AG_MutexUnlock(&dev->lock);
	return (0);
}

/* Configure a new virtual channel. */
int
AU_AddChannel(AU_DevOut *dev)
{
	AU_Channel *chanNew, *ch;
	int rv;

	AG_MutexLock(&dev->lock);
	if ((chanNew = TryRealloc(dev->chan, (dev->nChan+1)*sizeof(AU_Channel)))
	    == NULL) {
		AG_MutexUnlock(&dev->lock);
		return (-1);
	}
	dev->chan = chanNew;
	ch = &dev->chan[(rv = dev->nChan++)];
	ch->vol = 1.0;
	ch->pan = 0.5;
	AG_MutexUnlock(&dev->lock);
	return (rv);
}

/* Delete a virtual channel. */
int
AU_DelChannel(AU_DevOut *dev, int ch)
{
	AG_MutexLock(&dev->lock);
	if (ch < 0 || ch >= dev->nChan) {
		AG_SetError("No such channel");
		AG_MutexUnlock(&dev->lock);
		return (-1);
	}
	if (ch < dev->nChan-1) {
		memmove(&dev->chan[ch], &dev->chan[ch+1],
		    (dev->nChan - ch - 1)*sizeof(AU_Channel));
	}
	dev->nChan--;
	AG_MutexUnlock(&dev->lock);
	return (0);
}

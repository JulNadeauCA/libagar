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
 * Generic audio output interface.
 */

#include <core/core.h>
#include "au_init.h"
#include "au_dev_out.h"

/* Available audio output drivers */
extern const AU_DevOutClass auDevOut_pa;
extern const AU_DevOutClass auDevOut_file;
const AU_DevOutClass *auDevOutList[] = {
	&auDevOut_pa,
	&auDevOut_file,
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
		if (strcasecmp((*pDevCls)->name, devName) == 0)
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

	if ((dev->buf = TryMalloc(dev->bufMax*dev->bytesPerFrame)) == NULL) {
		goto fail;
	}
	for (i = 0; i < dev->bufMax*ch; i++)
		dev->buf[i] = 0.0;

	AG_MutexInit(&dev->lock);
	AG_CondInit(&dev->wrRdy);
	AG_CondInit(&dev->rdRdy);
	dev->mix = NULL;
	dev->nMix = 0;

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
	Free(dev->mix);
	AG_CondDestroy(&dev->wrRdy);
	AG_CondDestroy(&dev->rdRdy);
	AG_MutexDestroy(&dev->lock);
	Free(dev->buf);
	Free(dev);
}

/* Configure a new mixer channel. */
int
AU_AddChannel(AU_DevOut *dev)
{
	AU_Channel *mixNew, *mc;
	int rv;

	AG_MutexLock(&dev->lock);
	if ((mixNew = TryRealloc(dev->mix, (dev->nMix+1)*sizeof(AU_Channel)))
	    == NULL) {
		AG_MutexUnlock(&dev->lock);
		return (-1);
	}
	dev->mix = mixNew;
	mc = &dev->mix[dev->nMix++];
	mc->vol = 1.0;
	mc->pan = 0.5;
	rv = dev->nMix - 1;
	AG_MutexUnlock(&dev->lock);
	return (rv);
}

/* Delete a mixer channel. */
int
AU_DelChannel(AU_DevOut *dev, int mcn)
{
	AG_MutexLock(&dev->lock);
	if (mcn < 0 || mcn >= dev->nMix) {
		AG_SetError("No such channel");
		AG_MutexUnlock(&dev->lock);
		return (-1);
	}
	if (mcn < dev->nMix-1) {
		memmove(&dev->mix[mcn], &dev->mix[mcn+1],
		    (dev->nMix-1)*sizeof(AU_Channel));
	}
	dev->nMix--;
	AG_MutexUnlock(&dev->lock);
	return (0);
}

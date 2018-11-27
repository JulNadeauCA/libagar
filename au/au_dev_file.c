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
 * Audio file output driver.
 */
#include <agar/config/have_sndfile.h>
#ifdef HAVE_SNDFILE

#include <agar/core/core.h>
#include <agar/au/au_init.h>
#include <agar/au/au_dev_out.h>

#include <sndfile.h>
#include <errno.h>
#include <time.h>

typedef struct au_dev_out_wav {
	struct au_dev_out _inherit;
	SNDFILE  *file;
	SF_INFO   info;
	AG_Thread th;
	float    *silence;			/* 1s of silence */
} AU_DevOutFile;

static void
Init(void *obj)
{
	AU_DevOut *dev = obj;
	AU_DevOutFile *df = obj;

	dev->flags |= AU_DEV_OUT_THREADED;
	df->file = NULL;
	df->silence = NULL;
	memset(&df->info, 0, sizeof(df->info));
}

static void *
AU_DevFileThread(void *obj)
{
	AU_DevOut *dev = obj;
	AU_DevOutFile *df = obj;
	long rv;

	for (;;) {
		AG_MutexLock(&dev->lock);
		if (dev->flags & AU_DEV_OUT_CLOSING) {
			dev->flags &= ~(AU_DEV_OUT_CLOSING);
			AG_MutexUnlock(&dev->lock);
			return (NULL);
		}
		AG_CondBroadcast(&dev->wrRdy);
		/* XXX */
		if (dev->bufSize > dev->rate/1000) {
			rv = sf_writef_float(df->file, dev->buf, dev->bufSize);
			if (rv < dev->bufSize) {
				dev->flags |= AU_DEV_OUT_ERROR;
			}
			dev->bufSize = 0;
		} else {
			/* Write ~1ms of silence */
			rv = sf_writef_float(df->file, df->silence,
			    dev->rate/1000);
			if (rv < dev->rate) {
				dev->flags |= AU_DEV_OUT_ERROR;
			}
		}
		AG_MutexUnlock(&dev->lock);
		AG_Delay(1);
	}
	return (NULL);
}

static int
Open(void *obj, const char *path, int rate, int ch)
{
	AU_DevOut *dev = obj;
	AU_DevOutFile *df = obj;
	const char *c;
	int i;

	if (df->file != NULL) {
		AG_SetError("Audio dump to file already in progress");
		return (-1);
	}
	memset(&df->info, 0, sizeof(df->info));
	df->info.samplerate = rate;
	df->info.channels = ch;
	dev->rate = rate;
	dev->ch = ch;
	
	/* Allocate 1s of silence */
	if (df->silence == NULL &&
	    (df->silence = TryMalloc(rate*dev->bytesPerFrame)) == NULL) {
		return (-1);
	}
	for (i = 0; i < rate*ch; i++)
		df->silence[i] = 0.0;

	if ((c = strrchr(path, '.')) != NULL &&		/* XXX */
	    AG_Strcasecmp(c, ".ogg") == 0) {
		df->info.format = SF_FORMAT_OGG|SF_FORMAT_VORBIS;
	} else {
		df->info.format = SF_FORMAT_WAV|SF_FORMAT_FLOAT;
	}

	if ((df->file = sf_open(path, SFM_WRITE, &df->info)) == NULL) {
		AG_SetError("%s(%d): %s", path, rate, sf_strerror(NULL));
		return (-1);
	}
	if (AG_ThreadTryCreate(&df->th, AU_DevFileThread, df) != 0) {
		sf_write_sync(df->file);
		sf_close(df->file);
		df->file = NULL;
		return (-1);
	}
	return (0);
}

static void
Close(void *obj)
{
	AU_DevOutFile *df = obj;

	if (df->file != NULL) {
		sf_write_sync(df->file);
		sf_close(df->file);
		df->file = NULL;
	}
	Free(df->silence);
	df->silence = NULL;
}

const AU_DevOutClass auDevOut_file = {
	"file",
	sizeof(AU_DevOutFile),
	Init,
	NULL,	/* Destroy */
	Open,
	Close
};

#endif /* HAVE_SNDFILE */

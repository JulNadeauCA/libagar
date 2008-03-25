/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Routines related to surfaces of pixels.
 */

#include <core/core.h>

#include "gi.h"
#include "gi_gl.h"

#include <config/have_sdl.h>

void
GI_SurfaceInit(GI_Surface *s)
{
	s->fmt = NULL;
	s->w = 0;
	s->h = 0;
	s->pitch = 0;
	s->flags = 0;
	s->p = NULL;
	AG_MutexInit(&s->lock);
}

void
GI_SurfaceDestroy(GI_Surface *s)
{
	Free(s->p);
	AG_MutexDestroy(&s->lock);
}

GI_Surface *
GI_SurfaceNew(int w, int h, GI_PixelFormat *fmt, Uint flags)
{
	GI_Surface *s;

	s = Malloc(sizeof(GI_Surface));
	GI_SurfaceInit(s);
	s->fmt = fmt;
	s->flags |= flags;
}

void
GI_SurfaceFree(GI_Surface *s)
{
	GI_SurfaceDestroy(s);
	Free(s);
}

/* Compute shift amounts from masks. */
#define COMPUTE_SHIFT(mask, shift, loss)		\
	for (s = mask; (s & 0x01) == 0; s >>= 1) 	\
		(shift)++;				\
	while (s & 0x01) {				\
		(loss)--;				\
		s >>= 1;				\
	}
static __inline__ void
ComputeShifts(GI_PixelFormat *pf)
{
	Uint32 s;

	pf->Rshift = 0;	pf->Rloss = 8;
	pf->Gshift = 0;	pf->Gloss = 8;
	pf->Bshift = 0;	pf->Bloss = 8;
	pf->Ashift = 0;	pf->Aloss = 8;

	COMPUTE_SHIFTS(pf->Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTS(pf->Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTS(pf->Bmask, pf->Bshift, pf->Bloss);
	COMPUTE_SHIFTS(pf->Amask, pf->Ashift, pf->Aloss);
}
#undef COMPUTE_SHIFTS

/* Return a RGB pixel format. */
GI_PixelFormat *
GI_PixelFormatRGB(Uint bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	GI_PixelFormat *pf;
	int d;

	pf = Malloc(sizeof(GI_PixelFormat));
	pf->palette = NULL;
	pf->bpp = bpp;
	pf->Bpp = (bpp+7)/8;
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = 0;
	ComputeShifts(pf);
	return (pf);
}

#ifdef HAVE_SDL

/* Convert SDL pixel format to GI pixel format. */
GI_PixelFormat *
GI_PixelFormatFromSDL(SDL_PixelFormat *fmt)
{
	GI_PixelFormat *pf;

	pf = Malloc(sizeof(GI_PixelFormat));
	pf->palette = ConvertPalette(fmt->palette);
	pf->depth = fmt->BytesPerPixel;
	pf->Rloss = fmt->Rloss;
	pf->Gloss = fmt->Gloss;
	pf->Bloss = fmt->Bloss;
	pf->Aloss = fmt->Aloss;
	pf->Rshift = fmt->Rshift;
	pf->Gshift = fmt->Gshift;
	pf->Bshift = fmt->Bshift;
	pf->Ashift = fmt->Ashift;
	pf->Rmask = fmt->Rmask;
	pf->Gmask = fmt->Gmask;
	pf->Bmask = fmt->Bmask;
	pf->Amask = fmt->Amask;
	return (pf);
}

#endif /* HAVE_SDL */

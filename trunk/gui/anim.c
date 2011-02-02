/*
 * Copyright (c) 2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include "geometry.h"
#include "surface.h"
#include "anim.h"
#include "gui_math.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* Create a new animation of the specified pixel format. */
AG_Anim *
AG_AnimNew(enum ag_anim_type type, Uint w, Uint h, const AG_PixelFormat *pf,
    Uint flags)
{
	AG_Anim *a;

	if ((a = TryMalloc(sizeof(AG_Anim))) == NULL) {
		return (NULL);
	}
	if ((a->format = AG_PixelFormatDup(pf)) == NULL) {
		Free(a);
		return (NULL);
	}
	a->type = type;
	a->flags = flags;
	a->w = w;
	a->h = h;
	a->n = 0;
	a->pitch = w*pf->BytesPerPixel;
	a->clipRect = AG_RECT(0,0,w,h);
	a->pixels = NULL;
	return (a);
}

/* Create an empty animation. */
AG_Anim *
AG_AnimEmpty(void)
{
	return AG_AnimNew(AG_ANIM_PACKED, 0,0, agSurfaceFmt, 0);
}

/* Create a new color-index animation of given dimensions and depth. */
AG_Anim *
AG_AnimIndexed(Uint w, Uint h, int bpp, Uint flags)
{
	AG_PixelFormat *pf;
	AG_Anim *a;

	if ((pf = AG_PixelFormatIndexed(bpp)) == NULL) {
		return (NULL);
	}
	a = AG_AnimNew(AG_ANIM_INDEXED, w,h, pf, 0);
	AG_PixelFormatFree(pf);
	return (a);
}

/* Create a new packed-pixel animation with the specified RGB pixel format. */
AG_Anim *
AG_AnimRGB(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask)
{
	AG_PixelFormat *pf;
	AG_Anim *a;

	if ((pf = AG_PixelFormatRGB(bpp, Rmask, Gmask, Bmask)) == NULL) {
		return (NULL);
	}
	a = AG_AnimNew(AG_ANIM_PACKED, w,h, pf, 0);
	AG_PixelFormatFree(pf);
	return (a);
}

/*
 * Create a new packed-pixel animation with the specified RGBA pixel format.
 * The SRCALPHA flag is set implicitely.
 */
AG_Anim *
AG_AnimRGBA(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask, Uint32 Amask)
{
	AG_PixelFormat *pf;
	AG_Anim *a;

	if ((pf = AG_PixelFormatRGBA(bpp, Rmask, Gmask, Bmask, Amask)) == NULL) {
		return (NULL);
	}
	a = AG_AnimNew(AG_ANIM_PACKED, w,h, pf, AG_SRCALPHA);
	AG_PixelFormatFree(pf);
	return (a);
}

/* Load an animation from a series of PNG files. */
AG_Anim *
AG_AnimFromPNGs(const char *pattern)
{
	AG_Anim *a = NULL;
	AG_Surface *su;
	char path[AG_PATHNAME_MAX];
	int i;

	for (i = 0; ; i++) {
		Snprintf(path, sizeof(path), pattern, i);
		if (!AG_FileExists(path)) {
			if (i == 0) {
				continue;
			} else {
				break;
			}
		}
		if ((su = AG_SurfaceFromPNG(path)) == NULL) {
			break;
		}
		if (a == NULL) {
			a = AG_AnimRGBA(su->w, su->h,
			    su->format->BitsPerPixel, 0,
			    su->format->Rmask,
			    su->format->Gmask,
			    su->format->Bmask,
			    su->format->Amask);
			if (a == NULL) {
				AG_SurfaceFree(su);
				return (NULL);
			}
		}
		if (AG_AnimFrameNew(a,su) == -1) {
			goto fail;
		}
	}
	return (a);
fail:
	AG_AnimFree(a);
	return (NULL);
}

/* Set one or more entries in an indexed animation's palette. */
int
AG_AnimSetPalette(AG_Anim *a, AG_Color *c, Uint offs, Uint count)
{
	Uint i;

	if (a->type != AG_ANIM_INDEXED) {
		AG_SetError("Not an indexed animation");
		return (-1);
	}
	if (offs >= a->format->palette->nColors ||
	    offs+count >= a->format->palette->nColors) {
		AG_SetError("Bad palette offset/count");
		return (-1);
	}
	for (i = 0; i < count; i++) {
		a->format->palette->colors[offs+i] = c[i];
	}
	return (0);
}

/* Return a newly-allocated duplicate of an animation. */
AG_Anim *
AG_AnimDup(const AG_Anim *sa)
{
	AG_Anim *a;
	int i;

	a = AG_AnimNew(sa->type,
	    sa->w, sa->h, sa->format,
	    (sa->flags & AG_SAVED_ANIM_FLAGS));
	if (a == NULL) {
		return (NULL);
	}
	if ((a->pixels = TryMalloc(sa->n*sizeof(void *))) == NULL) {
		goto fail;
	}
	for (i = 0; i < sa->n; i++) {
		if ((a->pixels[i] = TryMalloc(sa->h*sa->pitch)) == NULL) {
			goto fail;
		}
		memcpy(a->pixels[i], sa->pixels[i], sa->h*sa->pitch);
		a->n++;
	}
	return (a);
fail:
	AG_AnimFree(a);
	return (NULL);
}

/* Resize an animation; pixels are left uninitialized. */
int
AG_AnimResize(AG_Anim *a, Uint w, Uint h)
{
	Uint8 *pixelsNew;
	int i;
	int pitchNew = w*a->format->BytesPerPixel;

	for (i = 0; i < a->n; i++) {
		if ((pixelsNew = TryRealloc(a->pixels[i], h*pitchNew)) == NULL) {
			for (i--; i >= 0; i--) {
				Free(a->pixels[i]);
			}
			return (-1);
		}
		a->pixels[i] = pixelsNew;
	}
	a->pitch = pitchNew;
	a->w = w;
	a->h = h;
	a->clipRect = AG_RECT(0,0,w,h);
	return (0);
}

/* Free the specified animation. */
void
AG_AnimFree(AG_Anim *a)
{
	int i;

	AG_PixelFormatFree(a->format);

	for (i = 0; i < a->n; i++) {
		Free(a->pixels[i]);
	}
	Free(a->pixels);
	Free(a);
}

/* Insert a new animation frame. */
int
AG_AnimFrameNew(AG_Anim *a, const AG_Surface *su)
{
	void **pixelsNew;

	if ((pixelsNew = TryRealloc(a->pixels, (a->n+1)*sizeof(void *)))
	    == NULL) {
		return (-1);
	}
	a->pixels = pixelsNew;
	if ((a->pixels[a->n] = TryMalloc(su->h*su->pitch)) == NULL) {
		return (-1);
	}
	memcpy(a->pixels[a->n], su->pixels, su->h*su->pitch);
	return (a->n++);
}

/* Return a new surface from a given frame#. */
AG_Surface *
AG_AnimFrameToSurface(const AG_Anim *a, int f)
{
	AG_Surface *su;

	if (f < 0 || f >= a->n) {
		AG_SetError("No such frame#");
		return (NULL);
	}
	if (a->format->Amask != 0) {
		su = AG_SurfaceFromPixelsRGBA(a->pixels[f], a->w, a->h,
		    a->format->BitsPerPixel,
		    a->format->Rmask,
		    a->format->Gmask,
		    a->format->Bmask,
		    a->format->Amask);
	} else {
		su = AG_SurfaceFromPixelsRGB(a->pixels[f], a->w, a->h,
		    a->format->BitsPerPixel,
		    a->format->Rmask,
		    a->format->Gmask,
		    a->format->Bmask);
	}
	return (su);
}

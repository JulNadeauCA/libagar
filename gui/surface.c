/*
 * Copyright (c) 2009-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Agar Graphics Surface. Surfaces can be Packed, Indexed or Grayscale, and
 * image transfers will handle any necessary inter-format conversion.
 * 
 * Packed mode allows 8-, 16-, 24-, 32- or 64-bit wide pixels (RGBA).
 * Indexed mode is 1-, 2-, 4- or 8-bit per pixel with per-surface palette.
 * Grayscale mode is a 16- or 32-bit value with a 16- or 32-bit alpha.
 */

#include <agar/core/core.h>
#include <agar/gui/surface.h>
#include <agar/gui/gui_math.h>

#include <agar/config/have_opengl.h>

#include <string.h>

const char *agSurfaceModeNames[] = {
	"RGB",
	"Indexed",
	"Grayscale",
	NULL
};
const char *agAlphaFuncNames[] = {
	"overlay",
	"zero",
	"one",
	"srcA",
	"dstA",
	"1-dstA",
	"1-srcA",
	NULL
};

AG_PixelFormat *agSurfaceFmt = NULL;  /* Recommended format for GUI surfaces */
enum ag_grayscale_mode agGrayscaleMode = AG_GRAYSCALE_BT709;

/* Import inlinables */
#undef AG_INLINE_HEADER
#include "inline_surface.h"

/*
 * Compute right shifts to extract RGBA components, as well as the
 * number of bits lost by packing components into our native fields.
 */
#define AG_GET_SHIFT_LOSS(mask, shift, loss) \
	shift = 0; \
	loss = AG_COMPONENT_BITS; \
	if (mask) { \
		for (m = mask ; !(m & 0x01); m >>= 1) { \
			shift++; \
		}  \
		for (; (m & 0x01); m >>= 1) { \
			loss--; \
		} \
	}

/* Initialize a packed-pixel format from RGB component masks */
void
AG_PixelFormatRGB(AG_PixelFormat *pf, int BitsPerPixel,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask)
{
	AG_PixelFormatRGBA(pf, BitsPerPixel, Rmask, Gmask, Bmask, 0);
}

/* Initialize a packed-pixel format from RGBA component masks */
void
AG_PixelFormatRGBA(AG_PixelFormat *pf, int BitsPerPixel,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask, AG_Pixel Amask)
{
	AG_Pixel m;
	
	pf->mode = AG_SURFACE_PACKED;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;

	switch (BitsPerPixel) {
	case 8:
		pf->PixelsPerByte = 1;
		break;
	case 16:
	case 24:
	case 32:
#if AG_MODEL == AG_LARGE
	case 48:
	case 64:
#endif
		pf->PixelsPerByte = 0;
		break;
	default:
#if AG_MODEL == AG_LARGE
		AG_FatalError("Packed RGB surfaces must be "
		              "8-, 16-, 24-, 32-, 48- or 64-bpp");
#else
		AG_FatalError("Packed RGB surfaces must be "
		              "8-, 16-, 24- or 32-bpp "
			      "(48- or 64-bpp needs AG_LARGE)");
#endif
	}

	AG_GET_SHIFT_LOSS(Rmask, pf->Rshift, pf->Rloss);
	AG_GET_SHIFT_LOSS(Gmask, pf->Gshift, pf->Gloss);
	AG_GET_SHIFT_LOSS(Bmask, pf->Bshift, pf->Bloss);

	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;

	if (Amask == 0) {
		pf->Amask = 0;
		pf->Ashift = 0;
		pf->Aloss = AG_COMPONENT_BITS;
	} else {
		AG_GET_SHIFT_LOSS(Amask, pf->Ashift, pf->Aloss);
		pf->Amask = Amask;
	}
}

#undef AG_GET_SHIFT_LOSS

/*
 * Initialize a palletized format in 1-, 2-, 4- or 8-bpp. Zero the palette in
 * >1bpp mode, or initialize it to (white,black) in monochrome (1bpp) mode.
 */
void
AG_PixelFormatIndexed(AG_PixelFormat *pf, int BitsPerPixel)
{
	AG_Palette *pal;
	
	pf->mode = AG_SURFACE_INDEXED;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;
	pf->PixelsPerByte = 8/BitsPerPixel;

	switch (BitsPerPixel) {
	case 1:
	case 2:
	case 4:
	case 8:
		break;
	default:
		AG_FatalError("Indexed surfaces must be 1-, 2-, 4-, or 8-bpp");
	}

	pal = pf->palette = Malloc(sizeof(AG_Palette));
	pal->nColors = 1<<BitsPerPixel;				/* 2^n colors */
	pal->colors = Malloc(pal->nColors*sizeof(AG_Color));

	if (BitsPerPixel == 1) {
		AG_ColorWhite(&pal->colors[0]);
		AG_ColorBlack(&pal->colors[1]);
	} else {
		memset(pal->colors, 0xff, pal->nColors*sizeof(AG_Color));
	}
}

/*
 * Initialize a grayscale + alpha format in 16-, 32- or 64-bpp. Pixels must
 * be packed in order 0xVVAA, 0xVVVVAAAA or 0xVVVVVVVVAAAAAAAA. Inherit
 * agGrayscaleMode as default standard for RGB conversion.
 */
void
AG_PixelFormatGrayscale(AG_PixelFormat *pf, int BitsPerPixel)
{
	pf->mode = AG_SURFACE_GRAYSCALE;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;

	switch (BitsPerPixel) {
#if 0
	case 8:
		pf->PixelsPerByte = 1;
		break;
#endif
	case 16:
	case 32:
#if AG_MODEL == AG_LARGE
	case 64:
#endif
		pf->PixelsPerByte = 0;
		break;
	default:
#if AG_MODEL == AG_LARGE
		AG_FatalError("Grayscale surfaces must be "
		              "16-, 32- or 64-bpp");
#else
		AG_FatalError("Grayscale surfaces must be "
		              "16- or 32-bpp (64-bpp requires AG_LARGE)");
#endif
	}

	pf->graymode = agGrayscaleMode;
}

/* Return a newly-allocated duplicate an AG_PixelFormat structure. */
AG_PixelFormat *
AG_PixelFormatDup(const AG_PixelFormat *orig)
{
	AG_PixelFormat *pf;
	AG_Palette *origPal, *newPal;

	if ((pf = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (NULL);
	}
	memcpy(pf, orig, sizeof(AG_PixelFormat));

	switch (orig->mode) {
	case AG_SURFACE_INDEXED:
		if ((newPal = pf->palette = TryMalloc(sizeof(AG_Palette))) == NULL) {
			goto fail;
		}
		origPal = orig->palette;
		if ((newPal->colors =
		    TryMalloc(origPal->nColors * sizeof(AG_Color))) == NULL) {
			free(newPal);
			goto fail;
		}
		newPal->nColors = origPal->nColors;
		memcpy(newPal->colors, origPal->colors,
		    origPal->nColors * sizeof(AG_Color));
		break;
	default:
		break;
	}
	return (pf);
fail:
	free(pf);
	return (NULL);
}

/*
 * Calculate pitch and padding for a given surface depth and width.
 *
 * (1,2,4)-bpp surfaces must be at least (8,4,2) pixels wide.
 * Align 8- to 32-bpp surfaces to 4-byte boundary.
 * Align 64-bpp surfaces to 8-byte boundary.
 */
static __inline__ int
Get_Aligned_Pitch_Padding(const AG_Surface *_Nonnull S, Uint w,
    Uint *_Nonnull pitch, Uint *_Nonnull padding)
{
	const AG_PixelFormat *pf = &S->format;
	int BitsPerPixel = pf->BitsPerPixel;
	Uint len;

	if (BitsPerPixel < 8) {
		switch (BitsPerPixel) {
		case 1:
			if (w < 8) {
				AG_SetErrorS("1bpp surfaces must be >7 px wide");
				return (-1);
			}
			break;
		case 2:
			if (w < 4) {
				AG_SetErrorS("2bpp surfaces must be >3 px wide");
				return (-1);
			}
			break;
		case 4:
			if (w < 2) {
				AG_SetErrorS("4bpp surfaces must be >1 px wide");
				return (-1);
			}
			break;
		default:
			AG_SetErrorS("Bad bits/pixel");
			return (-1);
		}
		len = w / pf->PixelsPerByte;
	} else {
		len = w * pf->BytesPerPixel;
	}
	switch (BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 48:
	case 64:
		*pitch = (len + 8-1) & ~(8-1);		/* Align to 64-bit */
		break;
#endif
	default:
		*pitch = (len + 4-1) & ~(4-1);		/* Align to 32-bit */
		break;
	}
	*padding = *pitch - len;
	return (0);
}

/* Allocate memory for w x h pixels (unless EXT_PIXELS is set). */
static void
AG_SurfaceAllocPixels(AG_Surface *_Nonnull S)
{
	AG_Size size;

	if (Get_Aligned_Pitch_Padding(S, S->w, &S->pitch, &S->padding) == -1)
		AG_FatalError(NULL);

	if (S->pixels != NULL && !(S->flags & AG_SURFACE_EXT_PIXELS)) {
		free(S->pixels);
	}
	if ((size = S->h*S->pitch) > 0 && !(S->flags & AG_SURFACE_EXT_PIXELS)) {
		S->pixels = Malloc(size);
	} else {
		S->pixels = NULL;
	}
}

/*
 * Initialize an AG_Surface to the specified format and size.
 * If pf specifies a palettized format, copy the palette from it.
 */
void
AG_SurfaceInit(AG_Surface *S, const AG_PixelFormat *pf, Uint w, Uint h,
    Uint flags)
{
	S->flags = flags;
	S->w = w;
	S->h = h;

	if (pf != NULL) {
		memcpy(&S->format, pf, sizeof(AG_PixelFormat));
		switch (pf->mode) {
		case AG_SURFACE_PACKED:
			if ((flags & AG_SURFACE_GL_TEXTURE) == 0 &&
			    pf->BitsPerPixel == 32) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
				if (pf->Rmask == 0xff000000 &&
				    pf->Gmask == 0x00ff0000 &&
				    pf->Bmask == 0x0000ff00 &&
				    pf->Amask == 0x000000ff)
					S->flags |= AG_SURFACE_GL_TEXTURE;
#else
				if (pf->Rmask == 0x000000ff &&
				    pf->Gmask == 0x0000ff00 &&
				    pf->Bmask == 0x00ff0000 &&
				    pf->Amask == 0xff000000)
					S->flags |= AG_SURFACE_GL_TEXTURE;
#endif
			}
			break;
		case AG_SURFACE_INDEXED:
			S->format.palette = NULL;
			AG_SurfaceSetPalette(S, pf->palette);
			break;
		case AG_SURFACE_GRAYSCALE:
			break;
		default:
			AG_FatalError("Bad PixelFormat mode");
		}
	}
	S->pixels = NULL;
	S->clipRect.x = 0;
	S->clipRect.y = 0;
	S->clipRect.w = w;
	S->clipRect.h = h;
	S->colorkey = 0;
	S->alpha = AG_OPAQUE;
	S->frames = NULL;
	S->n = 0;
}

/*
 * Create a new surface in specified format (any mode).
 *
 * Allocates uninitialized pixels (unless EXT_PIXELS flag is used).
 * Raise exception if insufficient memory is available.
 * Raise exception if given mode+BitsPerPixel combination is not supported.
 * {4,2,1}-bpp indexed surfaces must be at least {2,4,8}px wide.
 */
AG_Surface *
AG_SurfaceNew(const AG_PixelFormat *pf, Uint w, Uint h, Uint flags)
{
	AG_Surface *S;

	S = Malloc(sizeof(AG_Surface));
	AG_SurfaceInit(S, pf, w,h, flags);
	AG_SurfaceAllocPixels(S);
	return (S);
}

/* Create an empty surface. */
AG_Surface *
AG_SurfaceEmpty(void)
{
	return AG_SurfaceNew(agSurfaceFmt, 0,0, 0);
}

/*
 * Create a new surface in specified palettized mode.
 *
 * Allocate pixel data unless EXT_PIXELS flag is given.
 * Raise exception if insufficient memory is available.
 * Raise exception if given mode+BitsPerPixel combination is not supported.
 * {4,2,1}-bpp indexed surfaces must be at least {2,4,8}px wide.
 */
AG_Surface *
AG_SurfaceIndexed(Uint w, Uint h, int BitsPerPixel, Uint flags)
{
	AG_Surface *S;
	
	S = Malloc(sizeof(AG_Surface));
	AG_SurfaceInit(S, NULL, w,h, flags);
	AG_PixelFormatIndexed(&S->format, BitsPerPixel);
	AG_SurfaceAllocPixels(S);
	return (S);
}

/* Create a new surface in packed pixel mode (parametric form without alpha) */
AG_Surface *
AG_SurfaceRGB(Uint w, Uint h, int BitsPerPixel, Uint flags,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask)
{
	AG_Surface *S;
	
	S = Malloc(sizeof(AG_Surface));
	AG_SurfaceInit(S, NULL, w,h, flags);
	AG_PixelFormatRGB(&S->format, BitsPerPixel, Rmask, Gmask, Bmask);
	AG_SurfaceAllocPixels(S);
	return (S);
}

/* Create a new surface in packed pixel mode (parametric form with alpha) */
AG_Surface *
AG_SurfaceRGBA(Uint w, Uint h, int BitsPerPixel, Uint flags,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask, AG_Pixel Amask)
{
	AG_Surface *S;
	
	S = Malloc(sizeof(AG_Surface));
	AG_SurfaceInit(S, NULL, w,h, flags);
	AG_PixelFormatRGBA(&S->format, BitsPerPixel, Rmask, Gmask, Bmask, Amask);

	if ((flags & AG_SURFACE_GL_TEXTURE) == 0 && BitsPerPixel == 32) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
		if (Rmask == 0xff000000 &&
		    Gmask == 0x00ff0000 &&
		    Bmask == 0x0000ff00 &&
		    Amask == 0x000000ff)
			S->flags |= AG_SURFACE_GL_TEXTURE;
#else
		if (Rmask == 0x000000ff &&
		    Gmask == 0x0000ff00 &&
		    Bmask == 0x00ff0000 &&
		    Amask == 0xff000000)
			S->flags |= AG_SURFACE_GL_TEXTURE;
#endif
	}
	AG_SurfaceAllocPixels(S);
	return (S);
}

/* Create a new surface in grayscale+alpha mode. */
AG_Surface *
AG_SurfaceGrayscale(Uint w, Uint h, int BitsPerPixel, Uint flags)
{
	AG_Surface *S;
	
	S = Malloc(sizeof(AG_Surface));
	AG_SurfaceInit(S, NULL, w,h, flags);
	AG_PixelFormatGrayscale(&S->format, BitsPerPixel);
	AG_SurfaceAllocPixels(S);
	return (S);
}

/*
 * Create a new surface in packed pixel mode (parametric form without alpha),
 * and initialize immediately from given pixel data.
 */
AG_Surface *
AG_SurfaceFromPixelsRGB(const void *pixels, Uint w, Uint h,
    Uint BitsPerPixel, AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask)
{
	AG_PixelFormat pf;
	AG_Surface *S;

	AG_PixelFormatRGB(&pf, BitsPerPixel, Rmask, Gmask, Bmask);
	S = AG_SurfaceNew(&pf, w,h, 0);
	memcpy(S->pixels, pixels, h*S->pitch);
	AG_PixelFormatFree(&pf);
	return (S);
}

/*
 * Create a new surface in packed pixel mode (parametric form with alpha),
 * and initialize immediately from given pixel data.
 */
AG_Surface *
AG_SurfaceFromPixelsRGBA(const void *pixels, Uint w, Uint h, Uint BitsPerPixel,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask, AG_Pixel Amask)
{
	AG_PixelFormat pf;
	AG_Surface *S;

	AG_PixelFormatRGBA(&pf, BitsPerPixel, Rmask, Gmask, Bmask, Amask);
	S = AG_SurfaceNew(&pf, w,h, AG_SURFACE_ALPHA);
	memcpy(S->pixels, pixels, h*S->pitch);
	AG_PixelFormatFree(&pf);
	return (S);
}

/* Load a surface from an image file. */
AG_Surface *
AG_SurfaceFromFile(const char *path)
{
	AG_Surface *S;
	const char *ext;

	if ((ext = strrchr(path, '.')) == NULL) {
		AG_SetErrorS("Invalid filename");
		return (NULL);
	}
	if (Strcasecmp(ext, ".bmp") == 0) {
		S = AG_SurfaceFromBMP(path);
	} else if (Strcasecmp(ext, ".png") == 0) {
		S = AG_SurfaceFromPNG(path);
	} else if (Strcasecmp(ext, ".jpg") == 0 || Strcasecmp(ext, ".jpeg") == 0) {
		S = AG_SurfaceFromJPEG(path);
	} else {
		AG_SetError(_("Unknown image extension: %s"), ext);
		return (NULL);
	}
	return (S);
}

/* Export surface to an image file (format determined by extension). */
int
AG_SurfaceExportFile(const AG_Surface *S, const char *path)
{
	const char *ext;

	if ((ext = strrchr(path, '.')) == NULL) {
		AG_SetErrorS("Invalid filename");
		return (-1);
	}
	if (Strcasecmp(ext, ".bmp") == 0) {
		return AG_SurfaceExportBMP(S, path, 0);
	} else if (Strcasecmp(ext, ".png") == 0) {
		return AG_SurfaceExportPNG(S, path, 0);
	} else if (Strcasecmp(ext, ".jpg") == 0 || Strcasecmp(ext, ".jpeg") == 0) {
		return AG_SurfaceExportJPEG(S, path, 100, 0);
	} else {
		AG_SetError(_("Unknown image extension: %s"), ext);
		return (-1);
	}
	return (0);
}

/* Set pixel data to an external address (or NULL = revert to auto-allocated) */
void
AG_SurfaceSetAddress(AG_Surface *S, Uint8 *pixels)
{
	if (S->pixels != NULL && !(S->flags & AG_SURFACE_EXT_PIXELS)) {
		free(S->pixels);
	}
	if (pixels != NULL) {
		S->pixels = pixels;
		S->flags |= AG_SURFACE_EXT_PIXELS;
	} else {
		S->pixels = (S->h*S->pitch > 0) ? Malloc(S->h*S->pitch) : NULL;
		S->flags &= ~(AG_SURFACE_EXT_PIXELS);
	}
}

/*
 * Copy a fixed array of colors to a range of entries in the palette
 * of an Indexed surface.
 *
 * If surface is not an Indexed surface, raise an exception.
 * If offset+count is out of bounds, raise an exception.
 */
void
AG_SurfaceSetColors(AG_Surface *S, AG_Color *c, Uint offs, Uint count)
{
	AG_Palette *pal = S->format.palette;
	Uint i;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: SetColors(%u+%u)\n", S, offs, count);
#endif
	if (S->format.mode != AG_SURFACE_INDEXED) {
		AG_FatalError("Not INDEXED");
	}
	if (offs+count > pal->nColors) {
		AG_SetError("Bad colors %u[+%u] > %u", offs, count, pal->nColors);
	}
	for (i = 0; i < count; i++)
		pal->colors[offs+i] = c[i];
}

/*
 * Copy an entire palette to the palette of an indexed surface.
 * If surface is not an Indexed surface, raise an exception.
 */
void
AG_SurfaceSetPalette(AG_Surface *S, const AG_Palette *a)
{
	AG_Palette *b;
	AG_Size size;
	
#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: SetPalette(%u)\n", S, a->nColors);
#endif
	if (S->format.mode != AG_SURFACE_INDEXED) {
		AG_FatalError("Not INDEXED");
	}
	b = Malloc(sizeof(AG_Palette));
	size = a->nColors * sizeof(AG_Color);
	b->colors = Malloc(size);
	b->nColors = a->nColors;
	memcpy(b->colors, a->colors, size);

	Free(S->format.palette);
	S->format.palette = b;
}

/* Copy pixel data from a memory address. */
void
AG_SurfaceCopyPixels(AG_Surface *S, const Uint8 *pixels)
{
#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: CopyPixels(%p)\n", S, pixels);
#endif
	memcpy(S->pixels, pixels, (S->h * S->pitch));
}

/* Clear the entire surface with the given color */
void
AG_SurfaceSetPixels(AG_Surface *S, const AG_Color *c)
{
	AG_Pixel px;
	Uint x, y;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: SetPixels(%x%x%x%x)\n", S,
		    c->r, c->g, c->b, c->a);
#endif
	/* TODO optimized cases */
	px = AG_MapPixel(&S->format, c);
	for (y = 0; y < S->h; y++) {
		for (x = 0; x < S->w; x++)
			AG_SurfacePut(S, x,y, px);
	}
}

/* Return a newly-allocated duplicate of a surface (in the same format). */
AG_Surface *
AG_SurfaceDup(const AG_Surface *a)
{
	AG_Surface *b;

	b = AG_SurfaceNew(&a->format,
	    a->w,
	    a->h,
	    (a->flags & AG_SAVED_SURFACE_FLAGS));
#ifdef AG_DEBUG
	if (a->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Dup(->%p)\n", a, b);
#endif
	memcpy(b->pixels, a->pixels, a->h * a->pitch);
	return (b);
}

/* Return a newly-allocated duplicate of a surface (in a given format). */
AG_Surface *
AG_SurfaceConvert(const AG_Surface *a, const AG_PixelFormat *pf)
{
	AG_Surface *b;

	b = AG_SurfaceNew(pf,
	    a->w,
	    a->h,
	    (a->flags & AG_SAVED_SURFACE_FLAGS));
#ifdef AG_DEBUG
	if (a->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Convert([%dx%dx%d %s]->%p[%dx%dx%d %s])\n",
		    a, a->w, a->h, a->format.BitsPerPixel, agSurfaceModeNames[a->format.mode],
		    b, b->w, b->h, b->format.BitsPerPixel, agSurfaceModeNames[b->format.mode]);
#endif
	AG_SurfaceCopy(b, a);
	return (b);
}

/*
 * Copy pixel data from a source surface S to a destination surface D.
 * Convert if pixel format differs. Clip coordinates if dimensions differ.
 *
 * Unlike the blit operation, coordinates are fixed (0,0), clipping rectangle
 * is ignored and alpha/colorkey settings of the target surface are ignored.
 */
void
AG_SurfaceCopy(AG_Surface *D, const AG_Surface *S)
{
	Uint w = MIN(S->w, D->w);
	Uint h = MIN(S->h, D->h);
	int x, y;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE) {
		Debug(NULL, "Surface <%p>: Copy([%dx%dx%d/%d %s]->%p[%dx%dx%d/%d %s])\n",
		    S, S->w, S->h, S->format.BitsPerPixel, S->pitch, agSurfaceModeNames[S->format.mode],
		    D, D->w, D->h, D->format.BitsPerPixel, D->pitch, agSurfaceModeNames[D->format.mode]);
		D->flags |= AG_SURFACE_TRACE;
	}
#endif
	if (AG_PixelFormatCompare(&S->format, &D->format) == 0) {  /* Block */
		Uint8 *pSrc = S->pixels, *pDst = D->pixels;
		Uint Spadding = S->padding;
		Uint Dpadding = D->padding;
		Uint pitch = (D->format.BitsPerPixel < 8) ?
		     w / D->format.PixelsPerByte :
		     w * D->format.BytesPerPixel;
#ifdef AG_DEBUG
		if (S->flags & AG_SURFACE_TRACE)
			Debug(NULL,
			    "Surface <%p>: Block Copy (padding=%u->%u, pitch=%u)\n",
			    S, Spadding, Dpadding, pitch);
#endif
		if (D->w > S->w) {
			Dpadding += (D->format.BitsPerPixel < 8) ?
			             (D->w - S->w) / D->format.PixelsPerByte :
			             (D->w - S->w) * D->format.BytesPerPixel;
		} else if (D->w < S->w) {
			Spadding += (S->format.BitsPerPixel < 8) ?
			             (S->w - D->w) / S->format.PixelsPerByte :
			             (S->w - D->w) * S->format.BytesPerPixel;
		}
		for (y = 0; y < h; y++) {
			memcpy(pDst, pSrc, pitch);
			pSrc += pitch + Spadding;
			pDst += pitch + Dpadding;
		}
	} else {                                                 /* Pixelwise */
#ifdef AG_DEBUG
		if (S->flags & AG_SURFACE_TRACE)
			Debug(NULL, "Surface <%p>: Pixelwise Copy\n", S);
#endif
		/* TODO optimized cases */
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				AG_Pixel px;
				AG_Color c;
				
				px = AG_SurfaceGet(S, x,y);
				AG_GetColor(&c, px, &S->format);
				AG_SurfacePut(D, x,y,
				    AG_MapPixel(&D->format, &c));
			}
		}
	}
	if (S->format.mode == AG_SURFACE_INDEXED &&
	    D->format.mode == AG_SURFACE_INDEXED)
		AG_SurfaceSetPalette(D, S->format.palette);
}

/*
 * Blit loop with both semi-transparent per-surface alpha and colorkey.
 * No opaque pixels are possible.
 */
static void
AG_SurfaceBlit_AlCo(const AG_Surface *_Nonnull S, AG_Surface *_Nonnull D,
    const AG_Rect *_Nonnull dr, int xDst, int yDst)
{
	AG_Pixel srcColorkey = S->colorkey;
	int x,y;
	int w = dr->w;
	int h = dr->h;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Blit with Alpha & Colorkey\n", S);
#endif
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet(S, x,y);
			if (px == srcColorkey) {
				continue;
			}
			AG_GetColor(&c, px, &S->format);
			c.a = MIN(c.a, S->alpha);
			if (c.a == AG_TRANSPARENT) {
				continue;
			}
			AG_SurfaceBlend(D, xDst+x, yDst+y, &c, AG_ALPHA_OVERLAY);
		}
	}
}

/*
 * Blit loop with fully opaque per-surface alpha and colorkey.
 * Possibly some opaque pixels.
 */
static void
AG_SurfaceBlit_Co(const AG_Surface *_Nonnull S, AG_Surface *_Nonnull D,
    const AG_Rect *_Nonnull dr, int xDst, int yDst)
{
	AG_Pixel srcColorkey = S->colorkey;
	int w = dr->w;
	int h = dr->h;
	int x, y;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Blit with Colorkey only\n", S);
#endif
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet(S, x,y);
			if (px == srcColorkey) {
				continue;
			}
			AG_GetColor(&c, px, &S->format);
			if (c.a == AG_TRANSPARENT) {
				continue;
			}
			if (c.a == AG_OPAQUE) {
				AG_SurfacePut(D, xDst+x, yDst+y,
				    AG_MapPixel(&D->format, &c));
			} else {
				AG_SurfaceBlend(D, xDst+x, yDst+y, &c,
				    AG_ALPHA_OVERLAY);
			}
		}
	}
}

/*
 * Copy a region of pixels (per srcRect) from a source to a destination
 * surface at coordinates xDst,yDst. Coordinates are checked and clipped.
 * The source and destination surfaces cannot be the same.
 *
 * Perform alpha blending if S has AG_SURFACE_ALPHA flag. Honor component
 * alpha if S defines an alpha channel. Also honor per-surface alpha.
 *
 * Perform colorkey tests if S has AG_SURFACE_COLORKEY flag.
 *
 * If srcRect is NULL, the entire surface is copied.
 */
void
AG_SurfaceBlit(const AG_Surface *S, const AG_Rect *srcRect, AG_Surface *D,
    int xDst, int yDst)
{
	AG_Rect sr, dr;
	Uint x, y;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE) {
		if (srcRect != NULL) {
			Debug(NULL, "Surface <%p>: Blit ([%ux%u@%d,%d]->"
			            "%p:[%d,%d])\n", S,
			    srcRect->w, srcRect->h,
			    srcRect->x, srcRect->y, D, xDst, yDst);
		} else {
			Debug(NULL, "Surface <%p>: Blit (->%p:[%d,%d])\n",
			    S, D, xDst, yDst);
		}
	}
#endif
	if (S->alpha == AG_TRANSPARENT) {           /* Is fully transparent */
		return;
	}
	if (srcRect != NULL) {
		sr = *srcRect;
		if (sr.x < 0) { sr.x = 0; }
		if (sr.y < 0) { sr.y = 0; }
		if (sr.x+sr.w >= S->w) { sr.w = S->w - sr.x; }
		if (sr.y+sr.h >= S->h) { sr.h = S->h - sr.y; }
	} else {
		sr.x = 0;
		sr.y = 0;
		sr.w = S->w;
		sr.h = S->h;
	}
	dr.x = (Sint16)xDst;
	dr.y = (Sint16)yDst;
	dr.w = (Uint16)sr.w;
	dr.h = (Uint16)sr.h;
	if (!AG_RectIntersect(&dr, &dr, &D->clipRect)) {
		return;
	}
	if (dr.w < sr.w) {					/* Partial */
		if (xDst < D->clipRect.x) {
			sr.x += (D->clipRect.x - xDst);
		}
		sr.w -= (sr.w - dr.w);
	}
	if (dr.h < sr.h) {
		if (yDst < D->clipRect.y) {
			sr.y += (D->clipRect.y - yDst);
		}
		sr.h -= (sr.h - dr.h);
	}

	if (S->alpha < AG_OPAQUE) {
		if (S->flags & AG_SURFACE_COLORKEY) {
			AG_SurfaceBlit_AlCo(S, D, &dr, xDst,yDst);
			return;
		}
		for (y = 0; y < dr.h; y++) {
			for (x = 0; x < dr.w; x++) {
				AG_Pixel px;
				AG_Color c;
			
				px = AG_SurfaceGet(S, x,y);
				AG_GetColor(&c, px, &S->format);

				c.a = MIN(c.a, S->alpha);
				if (c.a == AG_TRANSPARENT) {
					continue;
				}
				AG_SurfaceBlend(D, xDst+x, yDst+y, &c,
				    AG_ALPHA_OVERLAY);
			}
		}
		return;
	}
	if (S->flags & AG_SURFACE_COLORKEY) {
		AG_SurfaceBlit_Co(S, D, &dr, xDst,yDst);
		return;
	}
	if (S->format.Amask != 0) {
		for (y = 0; y < dr.h; y++) {
			for (x = 0; x < dr.w; x++) {
				AG_Pixel px;
				AG_Color c;
			
				px = AG_SurfaceGet(S, x,y);
				AG_GetColor(&c, px, &S->format);

				if (c.a == AG_TRANSPARENT) {
					continue;
				}
				if (c.a < AG_OPAQUE) {
					AG_SurfaceBlend(D, xDst+x,yDst+y, &c,
					    AG_ALPHA_OVERLAY);
				} else {
					AG_SurfacePut(D, xDst+x,yDst+y,
					    AG_MapPixel(&D->format, &c));
				}
			}
		}
	} else {
		for (y = 0; y < dr.h; y++) {
			for (x = 0; x < dr.w; x++) {
				AG_Pixel px;
				AG_Color c;

				px = AG_SurfaceGet(S, x,y);
				AG_GetColor(&c, px, &S->format);
				AG_SurfacePut(D, xDst+x, yDst+y,
				    AG_MapPixel(&D->format, &c));
			}
		}
	}
}

/*
 * Resize a surface to specified dimensions in pixels.
 *
 * When growing the surface, new pixels are left uninitialized.
 * If parameters are incorrect or insufficient memory, fail and return -1.
 * If EXT_PIXELS flag is set, disable it and discard the address.
 * The clipping rectangle is reset to cover whole surface.
 */
int
AG_SurfaceResize(AG_Surface *S, Uint w, Uint h)
{
	Uint8 *pixelsNew;
	Uint newPitch, newPadding;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Resize(%ux%u->%ux%u)\n", S,
		    S->w, S->h, w,h);
#endif
	if (Get_Aligned_Pitch_Padding(S, w, &newPitch, &newPadding) == -1)
		AG_FatalError(NULL);

	if ((S->flags & AG_SURFACE_EXT_PIXELS) == 0) {
		if ((pixelsNew = TryRealloc(S->pixels, h*newPitch)) == NULL) {
			return (-1);
		}
		S->pixels = pixelsNew;
	} else {
		S->flags &= ~(AG_SURFACE_EXT_PIXELS);
		if ((S->pixels = TryMalloc(h*newPitch)) == NULL)
			return (-1);
	}
	S->w = w;
	S->h = h;
	S->pitch = newPitch;
	S->padding = newPadding;
	S->clipRect.x = 0;
	S->clipRect.y = 0;
	S->clipRect.w = w;
	S->clipRect.h = h;
	return (0);
}

/* Free the specified surface. */
void
AG_SurfaceFree(AG_Surface *S)
{
	Uint i;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_MAPPED) {
		AG_FatalError("AG_SurfaceFree: in use");
	}
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Free (flags=0x%x)\n", S, S->flags);
#endif
	AG_PixelFormatFree(&S->format);

	if (S->flags & AG_SURFACE_ANIMATED) {
		for (i = 0; i < S->n; i++) {
			AG_AnimFrame *af = &S->frames[i];

			switch (af->type) {
			case AG_ANIM_FRAME_PIXELS:
				Free(af->pixels.p);
				break;
			case AG_ANIM_FRAME_COLORS:
				Free(af->colors.c);
				break;
			case AG_ANIM_FRAME_DATA:
				Free(af->data.header);
				Free(af->data.p);
				break;
			default:
				break;
			}
		}
		Free(S->frames);
	}
	if ((S->flags & AG_SURFACE_EXT_PIXELS) == 0) {
		Free(S->pixels);
	}
	if ((S->flags & AG_SURFACE_STATIC) == 0) {
		free(S);
	}
}

/*
 * Scale a surface to size w x h.
 * TODO optimized cases; filtering
 * TODO integer-only version
 */
AG_Surface *
AG_SurfaceScale(const AG_Surface *S, Uint w, Uint h, Uint flags)
{
	AG_Surface *D;
	float xf,yf;
	int x,y;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Resize(%ux%u->%ux%u, 0x%x)\n",
		    S, S->w, S->h, w,h, flags);
#endif
	if (S->format.mode == AG_SURFACE_INDEXED &&
	    S->format.BitsPerPixel < 8 &&
	    S->w < S->format.BitsPerPixel)
		AG_FatalError("(1,2,4)bpp surfaces must be >=(8,4,2) pixels wide");

	D = AG_SurfaceNew(&S->format, w,h, S->flags & AG_SAVED_SURFACE_FLAGS);
	D->colorkey = S->colorkey;
	D->alpha = S->alpha;

	if (S->w == w && S->h == h) {			/* Simple copy */
		AG_SurfaceCopy(D, S);
		return (D);
	}

	xf = (float)(S->w - 1) / (float)(D->w - 1);
	yf = (float)(S->h - 1) / (float)(D->h - 1);

	for (y = 0; y < D->h; y++) {
		for (x = 0; x < D->w; x++) {
			AG_SurfacePut(D, x,y, AG_SurfaceGet(S, 
			    (int)((float)x * xf),
			    (int)((float)y * yf)));
		}
	}
	return (D);
}

/* Fill a rectangle with pixels of a given color. */
void
AG_FillRect(AG_Surface *S, const AG_Rect *rDst, const AG_Color *c)
{
	AG_Rect r;
	AG_Pixel px;
	int x,y;

	if (rDst != NULL) {
		int cx = S->clipRect.x, cx2 = S->clipRect.x+S->clipRect.w;
		int cy = S->clipRect.y, cy2 = S->clipRect.y+S->clipRect.h;
#ifdef AG_DEBUG
		if (S->flags & AG_SURFACE_TRACE)
			Debug(NULL, "Surface <%p>: "
			    "Fill(%ux%u@%d,%d <- [%x%x%x%x])\n",
			    S, rDst->w, rDst->h, rDst->x, rDst->y,
			    c->r, c->g, c->b, c->a);
#endif
		r = *rDst;
		if (r.x < cx)       { r.x = cx; }
		if (r.y < cy)       { r.y = cy; }
		if (r.x+r.w >= cx2) { r.w = cx2-r.x; }
		if (r.y+r.h >= cy2) { r.h = cy2-r.y; }
	} else {
#ifdef AG_DEBUG
		if (S->flags & AG_SURFACE_TRACE)
			Debug(NULL, "Surface <%p>: Fill(whole <- [%x%x%x%x])\n",
			    S, c->r, c->g, c->b, c->a);
#endif
		r = S->clipRect;
	}
	px = AG_MapPixel(&S->format, c);

	/* XXX TODO optimized cases */
	for (y = r.y; y < r.h; y++)
		for (x = r.x; x < r.w; x++)
			AG_SurfacePut(S, x, y, px);
}

/* Return the pixel at x,y in a 1- to 8-bpp surface S. */
Uint8
AG_SurfaceGet8(const AG_Surface *S, int x, int y)
{
	const Uint8 *p = S->pixels + y*S->pitch +
	                             x/S->format.PixelsPerByte;
#ifdef AG_DEBUG
	if (x < 0 || y < 0 || x >= S->w || y >= S->h)
		AG_FatalError("Illegal SurfaceGet8");
#endif
	switch (S->format.BitsPerPixel) {
	case 8:
		return (*p);
	case 4:
		switch (x % 2) {
		case 0:	return (*p & 0x0f);
		case 1:	return (*p & 0xf0) >> 4;
		}
		break;
	case 2:
		switch (x % 4) {
		case 0:	return (*p & 0x03);
		case 1:	return (*p & 0x0c) >> 2;
		case 2:	return (*p & 0x30) >> 4;
		case 3:	return (*p & 0xc0) >> 6;
		}
		break;
	case 1:
		switch (x % 8) {
		case 0:	return (*p & 0x01);
		case 1:	return (*p & 0x02) >> 1;
		case 2:	return (*p & 0x04) >> 2;
		case 3:	return (*p & 0x08) >> 3;
		case 4:	return (*p & 0x10) >> 4;
		case 5:	return (*p & 0x20) >> 5;
		case 6:	return (*p & 0x40) >> 6;
		case 7:	return (*p & 0x80) >> 7;
		}
		break;
	}
	return (0);
}

/* Write the pixel x,y in a 1-, 2-, 4-, or 8-bit surface S. */
void
AG_SurfacePut8(AG_Surface *_Nonnull S, int x, int y, Uint8 px)
{
	Uint8 *p = S->pixels + y*S->pitch +
	                       x/S->format.PixelsPerByte;
#ifdef AG_DEBUG
	if (x < 0 || x >= S->w ||
	    y < 0 || y >= S->h)
		AG_FatalError("Illegal SurfacePut8");
#endif
	switch (S->format.BitsPerPixel) {
	case 8:
		*p = px;
		break;
	case 4:
#ifdef AG_DEBUG
		if (px > 0x0f) { AG_FatalError("Bad 4-bit pixel"); }
#endif
		switch (x % 2) {
		case 0: *p = (*p & 0xf0) | px;		break;
		case 1:	*p = (*p & 0x0f) | px << 4;	break;
		}
		break;
	case 2:
#ifdef AG_DEBUG
		if (px > 0x03) { AG_FatalError("Bad 2-bit pixel"); }
#endif
		switch (x % 4) {
		case 0:	*p = (*p & 0xfc) | px;		break;
		case 1:	*p = (*p & 0xf3) | px << 2;	break;
		case 2:	*p = (*p & 0xcf) | px << 4;	break;
		case 3:	*p = (*p & 0x3f) | px << 6;	break;
		}
		break;
	case 1:
#ifdef AG_DEBUG
		if (px > 0x01) { AG_FatalError("Bad 1-bit pixel"); }
#endif
		switch (x % 8) {
		case 0:	*p = (*p & 0xfe) | px;		break;
		case 1:	*p = (*p & 0xfd) | px << 1;	break;
		case 2:	*p = (*p & 0xfb) | px << 2;	break;
		case 3:	*p = (*p & 0xf7) | px << 3;	break;
		case 4:	*p = (*p & 0xef) | px << 4;	break;
		case 5:	*p = (*p & 0xdf) | px << 5;	break;
		case 6:	*p = (*p & 0xbf) | px << 6;	break;
		case 7:	*p = (*p & 0x7f) | px << 7;	break;
		}
		break;
	}
}

/* Map 8-bit RGB components to an opaque 32-bit pixel. */
Uint32
AG_MapPixel32_RGB8(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (r    >> pf->Rloss) << pf->Rshift |
		       (g    >> pf->Gloss) << pf->Gshift |
		       (b    >> pf->Bloss) << pf->Bshift |
		      ((0xff >> pf->Aloss) << pf->Ashift & (Uint32)pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf,
		    AG_8toH(r),
		    AG_8toH(g),
		    AG_8toH(b),
		    AG_OPAQUE);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf,
		    AG_8toH(r),
		    AG_8toH(g),
		    AG_8toH(b),
		    AG_OPAQUE);
	}
}

/* Map 8-bit RGBA components to a 32-bit pixel. */
Uint32
AG_MapPixel32_RGBA8(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (r >> pf->Rloss) << pf->Rshift |
		       (g >> pf->Gloss) << pf->Gshift |
		       (b >> pf->Bloss) << pf->Bshift |
		      ((a >> pf->Aloss) << pf->Ashift & (Uint32)pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf,
		    AG_8toH(r),
		    AG_8toH(g),
		    AG_8toH(b),
		    AG_8toH(a));
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf,
		    AG_8toH(r),
		    AG_8toH(g),
		    AG_8toH(b),
		    AG_8toH(a));
	}
}

/* Map (compressed) 16-bit RGB components to an opaque 32-bit pixel. */
Uint32
AG_MapPixel32_RGB16(const AG_PixelFormat *pf, Uint16 r, Uint16 g, Uint16 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (AG_16to8(r) >> pf->Rloss) << pf->Rshift |
		       (AG_16to8(g) >> pf->Gloss) << pf->Gshift |
		       (AG_16to8(b) >> pf->Bloss) << pf->Bshift |
		      ((0xff        >> pf->Aloss) << pf->Ashift & (Uint32)pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf,
		    AG_16toH(r),
		    AG_16toH(g),
		    AG_16toH(b),
		    AG_OPAQUE);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf,
		    AG_16toH(r),
		    AG_16toH(g),
		    AG_16toH(b),
		    AG_OPAQUE);
	}
}

/* Map (compressed) 16-bit RGBA components to a 32-bit pixel. */
Uint32
AG_MapPixel32_RGBA16(const AG_PixelFormat *pf, Uint16 r, Uint16 g, Uint16 b,
    Uint16 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (AG_16to8(r) >> pf->Rloss) << pf->Rshift |
		       (AG_16to8(g) >> pf->Gloss) << pf->Gshift |
		       (AG_16to8(b) >> pf->Bloss) << pf->Bshift |
		      ((AG_16to8(a) >> pf->Aloss) << pf->Ashift & (Uint32)pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf, r,g,b,a);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf, r,g,b,a);
	}
}

#if AG_MODEL == AG_LARGE

/* Map (decompressed) 8-bit RGB components to an opaque 64-bit pixel. */
Uint64
AG_MapPixel64_RGB8(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (AG_8to16(r) >> pf->Rloss) << pf->Rshift |
		       (AG_8to16(g) >> pf->Gloss) << pf->Gshift |
		       (AG_8to16(b) >> pf->Bloss) << pf->Bshift |
		      ((0xffff      >> pf->Aloss) << pf->Ashift & pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf,
		    AG_16toH(r),
		    AG_16toH(g),
		    AG_16toH(b),
		    AG_OPAQUE);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf,
		    AG_16toH(r),
		    AG_16toH(g),
		    AG_16toH(b),
		    AG_OPAQUE);
	}
}

/* Map (decompressed) 8-bit RGBA components to a 64-bit pixel. */
Uint64
AG_MapPixel64_RGBA8(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b,
    Uint8 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (AG_8to16(r) >> pf->Rloss) << pf->Rshift |
		       (AG_8to16(g) >> pf->Gloss) << pf->Gshift |
		       (AG_8to16(b) >> pf->Bloss) << pf->Bshift |
		      ((AG_8to16(a) >> pf->Aloss) << pf->Ashift & pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf,
		    AG_8toH(r),
		    AG_8toH(g),
		    AG_8toH(b),
		    AG_8toH(a));
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf,
		    AG_8toH(r),
		    AG_8toH(g),
		    AG_8toH(b),
		    AG_8toH(a));
	}
}

#endif /* AG_LARGE */

#undef  EXTRACT_COMPONENT
#define EXTRACT_COMPONENT(rv, mask, shift, loss, bits)			\
	tmp = (px & mask) >> shift;					\
	(rv) = (tmp << loss) + (tmp >> (bits - (loss << 1)))

/* Extract 8-bit RGB components from a 32-bit pixel. */
void
AG_GetColor32_RGB8(Uint32 px, const AG_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED: {
		Uint32 tmp;
		EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 8);
		EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 8);
		EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 8);
		break;
	}
	case AG_SURFACE_INDEXED: {
		AG_Color *c = &pf->palette->colors[px % pf->palette->nColors];
		*r = AG_Hto8(c->r);
		*g = AG_Hto8(c->g);
		*b = AG_Hto8(c->b);
		break;
	}
	case AG_SURFACE_GRAYSCALE: {
		Uint8 dummy;
		AG_GetColor32_Gray8(px, pf->graymode, r,g,b,&dummy);
		break;
	}
	/* pf->mode */
	}
}

/* Extract 8-bit RGBA components from a 32-bit pixel. */
void
AG_GetColor32_RGBA8(Uint32 px, const AG_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	AG_Color *c;
	Uint32 tmp;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 8);
		EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 8);
		EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 8);
		EXTRACT_COMPONENT(*a, pf->Amask, pf->Ashift, pf->Aloss, 8);
		break;
	case AG_SURFACE_INDEXED:
		c = &pf->palette->colors[(Uint)px % pf->palette->nColors];
		*r = AG_Hto8(c->r);
		*g = AG_Hto8(c->g);
		*b = AG_Hto8(c->b);
		*a = AG_Hto8(c->a);
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_GetColor32_Gray8(px, pf->graymode, r,g,b,a);
		break;
	}
}

/* Extract (decompressed) 16-bit RGB components from a 32-bit pixel. */
void
AG_GetColor32_RGB16(Uint32 px, const AG_PixelFormat *pf,
    Uint16 *r, Uint16 *g, Uint16 *b)
{
	AG_Color *c;
	Uint8 R,G,B;
	Uint32 tmp;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		EXTRACT_COMPONENT(R, pf->Rmask, pf->Rshift, pf->Rloss, 8);
		EXTRACT_COMPONENT(G, pf->Gmask, pf->Gshift, pf->Gloss, 8);
		EXTRACT_COMPONENT(B, pf->Bmask, pf->Bshift, pf->Bloss, 8);
		*r = AG_8to16(R);
		*g = AG_8to16(G);
		*b = AG_8to16(B);
		break;
	case AG_SURFACE_INDEXED:
		c = &pf->palette->colors[(Uint)px % pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		break;
	case AG_SURFACE_GRAYSCALE: {
		Uint16 dummy;
		AG_GetColor32_Gray16(px, pf->graymode, r,g,b,&dummy);
		break;
	}
	/* pf->mode */
	}
}

/* Extract (decompressed) 16-bit RGBA components from a 32-bit pixel. */
void
AG_GetColor32_RGBA16(Uint32 px, const AG_PixelFormat *pf,
    Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
{
	AG_Color *c;
	Uint8 R,G,B,A;
	Uint32 tmp;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		EXTRACT_COMPONENT(R, pf->Rmask, pf->Rshift, pf->Rloss, 8);
		EXTRACT_COMPONENT(G, pf->Gmask, pf->Gshift, pf->Gloss, 8);
		EXTRACT_COMPONENT(B, pf->Bmask, pf->Bshift, pf->Bloss, 8);
		EXTRACT_COMPONENT(A, pf->Amask, pf->Ashift, pf->Aloss, 8);
		*r = AG_8to16(R);
		*g = AG_8to16(G);
		*b = AG_8to16(B);
		*a = AG_8to16(A);
		break;
	case AG_SURFACE_INDEXED:
		c = &pf->palette->colors[(Uint)px % pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		*a = c->a;
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_GetColor32_Gray16(px, pf->graymode, r,g,b,a);
		break;
	}
}

/*
 * Extract an AG_Color from a 32-bit pixel.
 * Apply dynamic range decompression.
 */
void
AG_GetColor32(AG_Color *c, Uint32 px, const AG_PixelFormat *pf)
{
	Uint8 R,G,B,A;
	Uint32 tmp;
	
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		EXTRACT_COMPONENT(R, pf->Rmask, pf->Rshift, pf->Rloss, 8);
		EXTRACT_COMPONENT(G, pf->Gmask, pf->Gshift, pf->Gloss, 8);
		EXTRACT_COMPONENT(B, pf->Bmask, pf->Bshift, pf->Bloss, 8);
		c->r = AG_8to16(R);
		c->g = AG_8to16(G);
		c->b = AG_8to16(B);
		if (pf->Amask != 0) {
			EXTRACT_COMPONENT(A, pf->Amask, pf->Ashift, pf->Aloss, 8);
			c->a = AG_8to16(A);
		} else {
			c->a = AG_OPAQUE;
		}
		break;
	case AG_SURFACE_INDEXED:
		memcpy(c, &pf->palette->colors[(Uint)px % pf->palette->nColors],
		    sizeof(AG_Color));
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_GetColor32_Gray(c, px, pf->graymode);
		break;
	}
}

/* Extract an AG_Color from a 64-bit pixel */
#if AG_MODEL == AG_LARGE
void
AG_GetColor64(AG_Color *c, Uint64 px, const AG_PixelFormat *pf)
{
	Uint64 tmp;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		EXTRACT_COMPONENT(c->r, pf->Rmask, pf->Rshift, pf->Rloss, 16);
		EXTRACT_COMPONENT(c->g, pf->Gmask, pf->Gshift, pf->Gloss, 16);
		EXTRACT_COMPONENT(c->b, pf->Bmask, pf->Bshift, pf->Bloss, 16);
		if (pf->Amask != 0) {
			EXTRACT_COMPONENT(c->a, pf->Amask, pf->Ashift,
			    pf->Aloss, 16);
		} else {
			c->a = AG_OPAQUE;
		}
		break;
	case AG_SURFACE_INDEXED:
		memcpy(c, &pf->palette->colors[px % pf->palette->nColors],
		    sizeof(AG_Color));
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_GetColor64_Gray(c, px, pf->graymode);
		break;
	}
}
#endif /* AG_LARGE */

#undef EXTRACT_COMPONENT

/* Return palette entry closest to given 16-bit RGBA components. */
AG_Pixel
AG_MapPixelIndexed(const AG_PixelFormat *pf,
    AG_Component r, AG_Component g, AG_Component b, AG_Component a)
{
	Uint32 err, errMin=AG_COLOR_LAST*4;
	Uint i, iMin=0;

	for (i=0; i < pf->palette->nColors; i++) {
		AG_Color *C = &pf->palette->colors[i];
		
		/* TODO weighted */
		err = (Uint32)(abs(C->r - r) +
		               abs(C->g - g) +
		               abs(C->b - b) +
		               abs(C->a - a));

		if (err < errMin) {
			errMin = err;
			iMin = i;
		}
	}
	return (Uint8)iMin;
}

/* Convert RGBA components to packed grayscale+alpha by luminosity. */
AG_Pixel
AG_MapPixelGrayscale(const AG_PixelFormat *pf,
    AG_Component r, AG_Component g, AG_Component b, AG_Component a)
{
	float R = (float)r/AG_COLOR_LASTF;
	float G = (float)g/AG_COLOR_LASTF;
	float B = (float)b/AG_COLOR_LASTF;
	float A = (float)a/AG_COLOR_LASTF;
	AG_GrayComponent lum;

	switch (pf->graymode) {
	case AG_GRAYSCALE_BT709:
	default:
		lum = ( 0.21f*R +  0.72f*G +  0.07f*B);
		break;
	case AG_GRAYSCALE_RMY:
		lum = (  0.5f*R + 0.419f*G + 0.081f*B);
		break;
	case AG_GRAYSCALE_Y:
		lum = (0.299f*R + 0.587f*G + 0.114f*B);
		break;
	}
#if AG_MODEL == AG_LARGE
	return ((Uint64)lum << 32) | (Uint32)(A*4294967295.0f);
#else
	return (Uint16)(lum << 16) | (Uint16)(A*65535.0f);
#endif
}

/* Convert a 32-bit grayscale+alpha pixel to an AG_Color by luminosity. */
void
AG_GetColor32_Gray(AG_Color *c, Uint32 G, AG_GrayscaleMode mode)
{
	float lum = ((G & 0xffff0000) >> 8) / 65535.0f;

	switch (mode) {
	case AG_GRAYSCALE_BT709:
	default:
		c->r = (Uint16)(lum / 0.21f);
		c->g = (Uint16)(lum / 0.72f);
		c->b = (Uint16)(lum / 0.07f);
		break;
	case AG_GRAYSCALE_RMY:
		c->r = (Uint16)(lum / 0.500f);
		c->g = (Uint16)(lum / 0.419f);
		c->b = (Uint16)(lum / 0.081f);
		break;
	case AG_GRAYSCALE_Y:
		c->r = (Uint16)(lum / 0.299f);
		c->g = (Uint16)(lum / 0.587f);
		c->b = (Uint16)(lum / 0.114f);
		break;
	}
	c->a = (G & 0xffff);
}

/* Convert a 32-bit grayscale+alpha pixel to 8-bit RGBA by luminosity. */
void
AG_GetColor32_Gray8(Uint32 G, AG_GrayscaleMode grayMode,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	float lum = (255.0f * (float)((G & 0xffff0000) >> 16)) / 65535.0f;

	switch (grayMode) {
	case AG_GRAYSCALE_BT709:
	default:
		*r = (Uint8)(lum / 0.21f);
		*g = (Uint8)(lum / 0.72f);
		*b = (Uint8)(lum / 0.07f);
		break;
	case AG_GRAYSCALE_RMY:
		*r = (Uint8)(lum / 0.5f);
		*g = (Uint8)(lum / 0.419f);
		*b = (Uint8)(lum / 0.081f);
		break;
	case AG_GRAYSCALE_Y:
		*r = (Uint8)(lum / 0.299f);
		*g = (Uint8)(lum / 0.587f);
		*b = (Uint8)(lum / 0.114f);
		break;
	}
	*a = (Uint8)(255.0f * (float)(G & 0xffff) / 65535.0f);
}

/* Convert a 32-bit grayscale+alpha pixel to 16-bit RGBA by luminosity. */
void
AG_GetColor32_Gray16(Uint32 px, AG_GrayscaleMode grayMode,
    Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
{
	float lum = (float)((px & 0xffff0000) >> 16);

	switch (grayMode) {
	case AG_GRAYSCALE_BT709:
	default:
		*r = (Uint16)(lum / 0.21f);
		*g = (Uint16)(lum / 0.72f);
		*b = (Uint16)(lum / 0.07f);
		break;
	case AG_GRAYSCALE_RMY:
		*r = (Uint16)(lum / 0.5f);
		*g = (Uint16)(lum / 0.419f);
		*b = (Uint16)(lum / 0.081f);
		break;
	case AG_GRAYSCALE_Y:
		*r = (Uint16)(lum / 0.299f);
		*g = (Uint16)(lum / 0.587f);
		*b = (Uint16)(lum / 0.114f);
		break;
	}
	*a = (px & 0xffff);
}

#if AG_MODEL == AG_LARGE
/*
 * Convert a 64-bit grayscale+alpha pixel to 16-bit RGBA by luminosity
 * (with compressed alpha).
 */
void
AG_GetColor64_Gray16(Uint64 G, AG_GrayscaleMode grayMode,
    Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
{
	double lum = AG_32to16((G & 0xffffffff00000000) >> 16);

	switch (grayMode) {
	case AG_GRAYSCALE_BT709:
	default:
		*r = (Uint16)(lum / 0.21f);
		*g = (Uint16)(lum / 0.72f);
		*b = (Uint16)(lum / 0.07f);
		break;
	case AG_GRAYSCALE_RMY:
		*r = (Uint16)(lum / 0.500f);
		*g = (Uint16)(lum / 0.419f);
		*b = (Uint16)(lum / 0.081f);
		break;
	case AG_GRAYSCALE_Y:
		*r = (Uint16)(lum / 0.299f);
		*g = (Uint16)(lum / 0.587f);
		*b = (Uint16)(lum / 0.114f);
		break;
	}
	*a = AG_32to16(G & 0xffffffff);
}

/*
 * Convert a 64-bit grayscale+alpha pixel to an AG_Color by luminosity.
 * Compress alpha down from 32- to 16-bit.
 */
void
AG_GetColor64_Gray(AG_Color *c, Uint64 G, AG_GrayscaleMode mode)
{
	double lum = AG_32to16((G & 0xffffffff00000000) >> 16);

	switch (mode) {
	case AG_GRAYSCALE_BT709:
	default:
		c->r = (Uint16)(lum / 0.21);
		c->g = (Uint16)(lum / 0.72);
		c->b = (Uint16)(lum / 0.07);
		break;
	case AG_GRAYSCALE_RMY:
		c->r = (Uint16)(lum / 0.500);
		c->g = (Uint16)(lum / 0.419);
		c->b = (Uint16)(lum / 0.081);
		break;
	case AG_GRAYSCALE_Y:
		c->r = (Uint16)(lum / 0.299);
		c->g = (Uint16)(lum / 0.587);
		c->b = (Uint16)(lum / 0.114);
		break;
	}
	c->a = AG_32to16(G & 0xffffffff);
}
#endif /* AG_LARGE */

/*
 * Blend 8-bit RGBA components with the pixel at x,y and overwrite it with
 * the result. Apply dynamic range decompression (8- to 16-bit). No clipping.
 * Target pixel alpha, if any, is set according to fn.
 */
void
AG_SurfaceBlendRGB8(AG_Surface *S, int x, int y,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a, AG_AlphaFn fn)
{
	AG_Color c;

	c.r = AG_8toH(r);
	c.g = AG_8toH(g);
	c.b = AG_8toH(b);
	c.a = AG_8toH(a);

	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    &c, fn);
}

/*
 * Blend 8-bit RGBA components with the pixel at p and overwrite it with the
 * result. Apply dynamic range decompression (8- to 16-bit). No clipping.
 * Target pixel alpha, if any, is set according to fn.
 */
void
AG_SurfaceBlendRGB8_At(AG_Surface *S, Uint8 *p,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a, AG_AlphaFn fn)
{
	AG_Color c;

	c.r = AG_8toH(r);
	c.g = AG_8toH(g);
	c.b = AG_8toH(b);
	c.a = AG_8toH(a);
	AG_SurfaceBlend_At(S, p, &c, fn);
}

/* Initialize animation playback context. */
void
AG_AnimStateInit(AG_AnimState *ast, AG_Surface *s)
{
#ifdef AG_DEBUG
	if (!(s->flags & AG_SURFACE_ANIMATED)) { AG_FatalError("!ANIMATED"); }
#endif
	AG_MutexInitRecursive(&ast->lock);
	ast->s = s;
	ast->flags = 0;
	ast->f = 0;
}

void
AG_AnimStateDestroy(AG_AnimState *ast)
{
	AG_MutexDestroy(&ast->lock);
}

void
AG_AnimSetLoop(AG_AnimState *ast, int enable)
{
	AG_MutexLock(&ast->lock);
	if (enable) {
		ast->flags |= AG_ANIM_LOOP;
		ast->flags &= ~(AG_ANIM_PINGPONG);
	} else {
		ast->flags &= ~(AG_ANIM_LOOP);
	}
	AG_MutexUnlock(&ast->lock);
}

void
AG_AnimSetPingPong(AG_AnimState *ast, int enable)
{
	AG_MutexLock(&ast->lock);
	if (enable) {
		ast->flags |= AG_ANIM_PINGPONG;
		ast->flags &= ~(AG_ANIM_LOOP);
	} else {
		ast->flags &= ~(AG_ANIM_PINGPONG);
	}
	AG_MutexUnlock(&ast->lock);
}

/* Animation processing loop */
static void *_Nullable
AG_AnimThreadProc(void *_Nonnull arg)
{
#if 0
	TODO

	AG_AnimState *ast = arg;
	Uint32 delay;

	while (ast->flags & AG_ANIM_PLAYING) {
		AG_MutexLock(&ast->lock);
		AG_MutexLock(&ast->an->lock);

		if (ast->an->n < 1) {
			AG_MutexUnlock(&ast->an->lock);
			AG_MutexUnlock(&ast->lock);
			goto out;
		}
		if (ast->f & AG_ANIM_REVERSE) {
			if (--ast->f < 0) {
				if (ast->flags & AG_ANIM_LOOP) {
					ast->f = (ast->an->n - 1);
				} else if (ast->flags & AG_ANIM_PINGPONG) {
					ast->f = 0;
					ast->flags &= ~(AG_ANIM_REVERSE);
				} else {
					ast->flags &= ~(AG_ANIM_PLAYING);
					AG_MutexUnlock(&ast->an->lock);
					AG_MutexUnlock(&ast->lock);
					goto out;
				}
			}
		} else {
			if (++ast->f >= ast->an->n) {
				if (ast->flags & AG_ANIM_LOOP) {
					ast->f = 0;
				} else if (ast->flags & AG_ANIM_PINGPONG) {
					ast->f--;
					ast->flags |= AG_ANIM_REVERSE;
				} else {
					ast->flags &= ~(AG_ANIM_PLAYING);
					AG_MutexUnlock(&ast->an->lock);
					AG_MutexUnlock(&ast->lock);
					goto out;
				}
			}
		}

		delay = (Uint32)(1000.0/ast->fps);

		AG_MutexUnlock(&ast->an->lock);
		AG_MutexUnlock(&ast->lock);

		AG_Delay(delay);
	}
out:
#endif
	return (NULL);
}

int
AG_AnimPlay(AG_AnimState *ast)
{
	int rv = 0;

	AG_MutexLock(&ast->lock);
	ast->flags |= AG_ANIM_PLAYING;
#ifdef AG_THREADS
	if (AG_ThreadTryCreate(&ast->th, AG_AnimThreadProc, ast) != 0) {
		AG_SetErrorS("Failed to create playback thread");
		rv = -1;
		ast->flags &= ~(AG_ANIM_PLAYING);
	}
#else
	AG_SetErrorS("AG_AnimPlay() requires threads");
	rv = -1;
#endif
	AG_MutexUnlock(&ast->lock);
	return (rv);
}

void
AG_AnimStop(AG_AnimState *ast)
{
	AG_MutexLock(&ast->lock);
	ast->flags &= ~(AG_ANIM_PLAYING);
	AG_MutexUnlock(&ast->lock);
}

/*
 * Add a PIXELS frame to an animated surface. Decoders will combine the pixels
 * (copied from s) against those of the previous frame (or background).
 *
 * The rd coordinates are checked and lie fit inside of Sanim.
 * Return index (in Sanim->frames) of new frame or -1 if an error has occurred.
 */
int
AG_SurfaceAddFrame(AG_Surface *Sanim, const AG_Surface *Sframe,
    const AG_Rect *r, AG_AnimDispose dispose, Uint delay, Uint flags)
{
	AG_AnimFrame *afNew, *af;
	AG_Rect rd;

	if (r != NULL) {
		rd = *r;
		if (rd.x < 0 || rd.y < 0 ||
		    rd.w < 0 || rd.h < 0 ||
		    rd.x + rd.w > Sframe->w ||
		    rd.y + rd.h > Sframe->h) {
			AG_SetErrorS("Bad rectangle");
			return (-1);
		}
	} else {
		rd.x = 0;
		rd.y = 0;
		rd.w = Sframe->w;
		rd.h = Sframe->h;
	}

	if ((afNew = TryRealloc(Sanim->frames, (Sanim->n+1)*sizeof(AG_AnimFrame))) == NULL) {
		return (-1);
	}
	Sanim->frames = afNew;
	af = &Sanim->frames[Sanim->n];
	af->type = AG_ANIM_FRAME_PIXELS;
	af->flags = flags;
	af->dispose = dispose;
	af->delay = delay;

	if (AG_PixelFormatCompare(&Sframe->format, &Sanim->format) == 0) {
		const AG_Size len = (Sframe->h * Sframe->pitch);

		if ((af->pixels.p = TryMalloc(len)) == NULL) {
			return (-1);
		}
		memcpy(af->pixels.p, Sframe->pixels, len);
		af->pixels.x = (Uint16)rd.x;
		af->pixels.y = (Uint16)rd.y;
		af->pixels.w = (Uint16)rd.w;
		af->pixels.h = (Uint16)rd.h;
	} else {
		AG_Surface *Sconv;
	
		Sconv = AG_SurfaceConvert(Sframe, &Sanim->format);
		af->pixels.p = Sconv->pixels;
		Sconv->pixels = NULL;
		AG_SurfaceFree(Sconv);
	}
	return (Sanim->n++);
}

#ifdef AG_LEGACY
void AG_SurfaceLock(AG_Surface *su) { }
void AG_SurfaceUnlock(AG_Surface *su) { }
Uint32 AG_MapRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b) {
	return AG_MapPixel32_RGB8(pf, r,g,b);
}
Uint32 AG_MapRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return AG_MapPixel32_RGBA8(pf, r,g,b,a);
}
Uint32 AG_MapPixelRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b) {
	return AG_MapPixel32_RGB8(pf, r,g,b);
}
Uint32 AG_MapPixelRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return AG_MapPixel32_RGBA8(pf, r,g,b,a);
}
Uint32 AG_MapColorRGB(const AG_PixelFormat *pf, AG_Color c) {
#if AG_MODEL == AG_LARGE
	return AG_MapPixel32_RGB16(pf, c.r, c.g, c.b);
#else
	return AG_MapPixel32_RGB8(pf, c.r, c.g, c.b);
#endif
}
Uint32 AG_MapColorRGBA(const AG_PixelFormat *pf, AG_Color c) {
	return AG_MapPixel32(pf, &c);
}
AG_Surface *AG_DupSurface(AG_Surface *su) {
	return AG_SurfaceDup((const AG_Surface *)su);
}
void AG_GetRGB(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b) {
	AG_GetColor32_RGB8(px, pf, r,g,b);
}
void AG_GetRGBA(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
	AG_GetColor32_RGBA8(px, pf, r,g,b,a);
}
void AG_GetPixelRGB(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b) {
	AG_GetColor32_RGB8(px, pf, r,g,b);
}
void AG_GetPixelRGBA(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
	AG_GetColor32_RGBA8(px, pf, r,g,b,a);
}
AG_Color AG_GetColorRGB(Uint32 px, const AG_PixelFormat *pf) {
	AG_Color c;
	AG_GetColor32(&c, px, pf);
	c.a = AG_OPAQUE;
	return (c);
}
AG_Color AG_GetColorRGBA(Uint32 px, const AG_PixelFormat *pf) {
	AG_Color c;
	AG_GetColor32(&c, px, pf);
	return (c);
}
int AG_SamePixelFmt(const AG_Surface *a, const AG_Surface *b) {
	return (AG_PixelFormatCompare(&a->format, &b->format)) == 0;
}
int AG_ScaleSurface(const AG_Surface *s, Uint16 w, Uint16 h, AG_Surface **pDS) {
	*pDS = AG_SurfaceScale(s, (Uint)w, (Uint)h, 0);
	return (0);
}
AG_Surface *AG_SurfaceStdGL(Uint w, Uint h) {
	return AG_SurfaceStdRGBA(w, h);
}
#endif /* AG_LEGACY */

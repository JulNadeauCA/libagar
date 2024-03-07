/*
 * Copyright (c) 2009-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Agar Graphics Surface. Surfaces can be Packed, Indexed or Grayscale.
 * Color components can be 8bpc or 16bpc wide (16bpc needs a LARGE build).
 *
 *  1) Packed    : (16/24/32/40/48/64)-bit per pixel (any order RGB/RGBA).
 *  2) Indexed   : (1/2/4/8)-bit per pixel (with per-surface palette).
 *  3) Grayscale : (16/32/64)-bit per pixel (12/24/48-bit + 4/8/16-bit alpha).
 *
 * Surfaces can be copied / blitted to other surfaces in any format and
 * AG_Surface will handle the necessary inter-format conversion.
 */

#include <agar/core/core.h>
#include <agar/gui/surface.h>
#include <agar/gui/gui_math.h>

#include <agar/config/have_opengl.h>

#include <string.h>

/* Check for incorrect blitter maps / invocations. */
/* #define DEBUG_BLITTER_MAP */

/*
 * Expensive tests for invalid coordinates or bad pixel values in
 * AG_SurfaceGet*() and AG_SurfacePut*() / AG_SurfaceBlend*().
 */
/* #define DEBUG_SURFACE_GET */
/* #define DEBUG_SURFACE_PUT */

/* Import standard <= 8-bit palettes */
#include "palettes.h"

enum ag_grayscale_mode agGrayscaleMode = AG_GRAYSCALE_BT709;

/* Names for AG_SurfaceMode values. */
const char *agSurfaceModeNames[] = {
	N_("Packed"),
	N_("Indexed"),
	N_("Grayscale"),
	NULL
};

AG_PixelFormat *agSurfaceFmt = NULL;                       /* "Best" format */
AG_LowerBlit *agLowerBlits[AG_SURFACE_MODE_LAST] = { NULL, NULL, NULL };
int           agLowerBlits_Count[AG_SURFACE_MODE_LAST] = { 0,0,0 };

/* Import inlinables */
#undef AG_INLINE_HEADER
#include "inline_surface.h"

/* Import agExpandByte8[] lookup table (expand partial bytes to 0..255) */
#include "expand_byte8.h"

/*
 * Compute right shifts to extract RGBA components, as well as the
 * number of bits lost by packing components into our native fields
 * (which are 8-bit under MEDIUM and 16-bit under LARGE).
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

/* Extract a 16bpc component from a 40/48/64-bit packed pixel. */
#undef  EXTRACT_COMPONENT16
#define EXTRACT_COMPONENT16(rv, mask, shift, loss)         \
          tmp = (px & mask) >> shift;                       \
          (rv) = (tmp << loss) | (tmp >> (16 - (loss << 1)))

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

#ifdef AG_DEBUG
	switch (BitsPerPixel) {
	case 32:
	case 24:
# if AG_MODEL == AG_LARGE
	case 64:
	case 48:
	case 40:
# endif
	case 16:
	case 8:
		break;
	default:
# if AG_MODEL == AG_LARGE
		AG_FatalError("Packed RGB surfaces must be "
		              "8/16/24/32/40/48/64-bpp");
# else
		AG_FatalError("Packed RGB surfaces must be "
		              "8/16/24/32-bpp "
			      "(40/48/64-bpp needs AG_LARGE)");
# endif
	}
#endif /* AG_DEBUG */

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
 * Initialize an Indexed (palettized) format in 1/2/4/8-bpp. Initialize
 * the palette in 1/2/4-bpp mode (leave it uninitialized in 8-bpp mode).
 */
void
AG_PixelFormatIndexed(AG_PixelFormat *pf, int BitsPerPixel)
{
	AG_Palette *pal;
	AG_Size palSize;
	
	pf->mode = AG_SURFACE_INDEXED;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;

	switch (BitsPerPixel) {
	case 8: pf->PixelsPerByteShift = 0; break;
	case 4: pf->PixelsPerByteShift = 1; break;
	case 2: pf->PixelsPerByteShift = 2; break;
	case 1: pf->PixelsPerByteShift = 3; break;
	default:
		AG_FatalError("Indexed surfaces must be 1/2/4/8-bpp");
	}

	pal = pf->palette = Malloc(sizeof(AG_Palette));
	pal->nColors = 1 << BitsPerPixel;                     /* 2^n colors */
	palSize = pal->nColors * sizeof(AG_Color);
	pal->colors = Malloc(palSize);
	memcpy(pal->colors, agStdPalette[BitsPerPixel-1], palSize);
}

/*
 * Initialize a Grayscale format in 16/32/64-bpp (12/24/48 + 4/8/16 bit alpha).
 * Inherit agGrayscaleMode as default standard for RGB conversion.
 */
void
AG_PixelFormatGrayscale(AG_PixelFormat *pf, int BitsPerPixel)
{
	pf->mode = AG_SURFACE_GRAYSCALE;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;

#ifdef AG_DEBUG
	switch (BitsPerPixel) {
	case 32:
# if AG_MODEL == AG_LARGE
	case 64:
# endif
	case 16:
		break;
	default:
# if AG_MODEL == AG_LARGE
		AG_FatalError("Grayscale surfaces must be "
		              "16/32/64-bpp");
# else
		AG_FatalError("Grayscale surfaces must be "
		              "16/32-bpp (64-bpp requires AG_LARGE)");
# endif
	}
#endif /* AG_DEBUG */

	pf->graymode = agGrayscaleMode;
}

/* Return a newly-allocated duplicate an AG_PixelFormat structure. */
AG_PixelFormat *
AG_PixelFormatDup(const AG_PixelFormat *pfOrig)
{
	AG_PixelFormat *pf;
	AG_Palette *palOrig, *newPal;

	if ((pf = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (NULL);
	}
	memcpy(pf, pfOrig, sizeof(AG_PixelFormat));

	if (pfOrig->mode == AG_SURFACE_INDEXED) {
		if ((newPal = TryMalloc(sizeof(AG_Palette))) == NULL) {
			goto fail;
		}
		pf->palette = newPal;
		palOrig = pfOrig->palette;

		newPal->colors = TryMalloc(palOrig->nColors * sizeof(AG_Color));
		if (newPal->colors == NULL) {
			free(newPal);
			goto fail;
		}
		newPal->nColors = palOrig->nColors;
		memcpy(newPal->colors, palOrig->colors, palOrig->nColors *
		                                        sizeof(AG_Color));
	}
	return (pf);
fail:
	free(pf);
	return (NULL);
}

/*
 * Return the maximum numerical value of pixels for surfaces
 * of the given format.
 */
AG_Pixel
AG_PixelFormatMaximum(const AG_PixelFormat *pf)
{
	switch (pf->BitsPerPixel) {
	case 32:  return (AG_Pixel)(0xffffffffU);
	case 24:  return (AG_Pixel)(0xffffffU);
#if AG_MODEL == AG_LARGE
	case 64:  return (AG_Pixel)(0xffffffffffffffffU);
	case 48:  return (AG_Pixel)(0xffffffffffffU);
	case 40:  return (AG_Pixel)(0xffffffffffU);
#endif
	case 16:  return (AG_Pixel)(0xffffU);
	case 8:   return (AG_Pixel)(0xffU);
	case 4:   return (AG_Pixel)(0xfU);
	case 2:   return (AG_Pixel)(0x3U);
	case 1:   return (AG_Pixel)(0x1U);
	default:  return (AG_Pixel)(0U);
	}
}

/*
 * Calculate pitch and padding for a given surface depth and width.
 *
 * (1,2,4)-bpp surfaces must be at least (8,4,2) pixels wide respectively.
 * Align 8- to 32-bpp surfaces to a 4-byte boundary.
 * Align 40-, 48- and 64-bpp surfaces to an 8-byte boundary.
 */
static __inline__ int
Get_Aligned_Pitch_Padding(const AG_Surface *_Nonnull S, Uint w,
    Uint *_Nonnull pitch, Uint *_Nonnull padding)
{
	const AG_PixelFormat *pf = &S->format;
	const int BitsPerPixel = pf->BitsPerPixel;
	Uint len;

	if (BitsPerPixel < 8) {
#ifdef AG_DEBUG
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
			AG_SetErrorS("Unsupported bits/pixel");
			return (-1);
		}
#endif /* AG_DEBUG */
		len = (w >> pf->PixelsPerByteShift);
	} else {
		len = (w * pf->BytesPerPixel);
	}

	switch (BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 64:
	case 48:
	case 40:
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

/* Allocate memory for w x h pixels. No-op if EXT_PIXELS is set. */
static __inline__ void
AG_SurfaceAllocPixels(AG_Surface *_Nonnull S)
{
	AG_Size size;

	if (Get_Aligned_Pitch_Padding(S, S->w, &S->pitch, &S->padding) == -1)
		AG_FatalError(NULL);

	if (S->pixelsBase != NULL && !(S->flags & AG_SURFACE_EXT_PIXELS)) {
		free(S->pixelsBase);
	}
	size = S->h * S->pitch;
	if (size > 0 && !(S->flags & AG_SURFACE_EXT_PIXELS)) {
		S->pixelsBase = Malloc(size);
	} else {
		S->pixelsBase = NULL;
	}
	S->pixels = S->pixelsBase;
}

static __inline__ void
SetPackedOptimizations(AG_Surface *_Nonnull S)
{
#ifdef HAVE_OPENGL
	/*
	 * Determine whether this format can be uploaded to OpenGL textures
	 * without the need for conversion.
	 */
	if ((S->flags & AG_SURFACE_GL_TEXTURE) == 0) {
		const AG_PixelFormat *pf = &S->format;

		if (pf->BitsPerPixel == 32) {
# if AG_BYTEORDER == AG_BIG_ENDIAN
			if (pf->Rmask == 0xff000000 &&
			    pf->Gmask == 0x00ff0000 &&
			    pf->Bmask == 0x0000ff00 &&
			    pf->Amask == 0x000000ff)
				S->flags |= AG_SURFACE_GL_TEXTURE;
# else
			if (pf->Rmask == 0x000000ff &&
			    pf->Gmask == 0x0000ff00 &&
			    pf->Bmask == 0x00ff0000 &&
			    pf->Amask == 0xff000000)
				S->flags |= AG_SURFACE_GL_TEXTURE;
# endif
		}
	}
#endif /* HAVE_OPENGL */
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
			SetPackedOptimizations(S);
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
	memset(&S->pixelsBase, 0, sizeof(Uint8 *) +                   /* pixelsBase */
	                          sizeof(Uint8 *) +                   /* pixels */
	                          sizeof(AG_Rect) +                   /* clipRect */
			          sizeof(AG_AnimFrame *) +            /* frames */
			          sizeof(Uint) +                      /* n */
			          sizeof(Uint) +                      /* padding */
			          sizeof(Uint) +                      /* Lpadding */
			          sizeof(Uint) +                      /* alpha */
			          sizeof(Uint16)*AG_SURFACE_NGUIDES + /* guides */
			          sizeof(AG_Pixel));                  /* colorkey */
	S->alpha = AG_OPAQUE;
	S->clipRect.w = w;
	S->clipRect.h = h;
}

/*
 * Create a new surface in specified format (any mode).
 *
 * Newly-allocated pixels are left uninitialized.
 * No allocation is performed if EXT_PIXELS (external memory) is set.
 * Raise an exception if insufficient memory is available.
 * Raise an exception if given mode+BitsPerPixel combination is not supported.
 * Surfaces in {4,2,1}-bpp must be at minimum {2,4,8} pixels wide respectively.
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
 * Create a new surface in the specified Indexed (palettized) mode.
 *
 * Newly-allocated pixels are left uninitialized.
 * No allocation is performed if EXT_PIXELS (external memory) is set.
 * The palette is initialized to the standard palette of the given mode.
 * Raise an exception if insufficient memory is available or bpp unsupported.
 * Surfaces in {4,2,1}-bpp must be at minimum {2,4,8} pixels wide respectively.
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

/*
 * Create a new surface in the specified Packed RGB mode (no alpha).
 *
 * Newly-allocated pixels are left uninitialized.
 * No allocation is performed if EXT_PIXELS (external memory) is set.
 * Raise an exception if insufficient memory is available or bpp unsupported.
 */
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

/*
 * Create a new surface in the specified Packed RGBA mode.
 *
 * Newly-allocated pixels are left uninitialized.
 * No allocation is performed if EXT_PIXELS (external memory) is set.
 * Raise an exception if insufficient memory is available or bpp unsupported.
 */
AG_Surface *
AG_SurfaceRGBA(Uint w, Uint h, int BitsPerPixel, Uint flags,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask, AG_Pixel Amask)
{
	AG_Surface *S;
	
	S = Malloc(sizeof(AG_Surface));
	AG_SurfaceInit(S, NULL, w,h, flags);
	AG_PixelFormatRGBA(&S->format, BitsPerPixel, Rmask, Gmask, Bmask, Amask);
	SetPackedOptimizations(S);
	AG_SurfaceAllocPixels(S);
	return (S);
}

/*
 * Create a new surface in the specified Grayscale mode.
 *
 * Newly-allocated pixels are left uninitialized.
 * No allocation is performed if EXT_PIXELS (external memory) is set.
 * Inherit the default grayscale conversion mode from agGrayscaleMode.
 * Raise an exception if insufficient memory is available or bpp unsupported.
 */
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
 * Create a new Packed RGB (no alpha) surface and initialize it from the
 * given pixel data. See AG_SurfaceRGB().
 */
AG_Surface *
AG_SurfaceFromPixelsRGB(const void *pixels, Uint w, Uint h,
    Uint BitsPerPixel, AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask)
{
	AG_PixelFormat pf;
	AG_Surface *S;

	AG_PixelFormatRGB(&pf, BitsPerPixel, Rmask, Gmask, Bmask);
	S = AG_SurfaceNew(&pf, w,h, 0);
	memcpy(S->pixels, pixels, h * S->pitch);
	AG_PixelFormatFree(&pf);
	return (S);
}

/*
 * Create a new Packed RGBA surface and initialize it from the given pixel data.
 * See AG_SurfaceRGBA().
 */
AG_Surface *
AG_SurfaceFromPixelsRGBA(const void *pixels, Uint w, Uint h, Uint BitsPerPixel,
    AG_Pixel Rmask, AG_Pixel Gmask, AG_Pixel Bmask, AG_Pixel Amask)
{
	AG_PixelFormat pf;
	AG_Surface *S;

	AG_PixelFormatRGBA(&pf, BitsPerPixel, Rmask, Gmask, Bmask, Amask);
	S = AG_SurfaceNew(&pf, w,h, 0);
	memcpy(S->pixels, pixels, h * S->pitch);
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

/*
 * Set pixel data to an externally-allocated address pixelsBase.
 * If pixelsBase=NULL, revert to auto-allocated storage (removing any paddings).
 * Resets the surface's pixels pointer to pixelsBase.
 */
void
AG_SurfaceSetAddress(AG_Surface *S, Uint8 *pixelsBase)
{
	if (S->pixelsBase != NULL && !(S->flags & AG_SURFACE_EXT_PIXELS)) {
		free(S->pixelsBase);
	}
	if (pixelsBase != NULL) {                   /* Externally allocated */
		S->pixelsBase = pixelsBase;
		S->flags |= AG_SURFACE_EXT_PIXELS;
	} else {                                          /* Revert to auto */
		const AG_Size size = (S->h * S->pitch);

		S->pixelsBase = (size > 0) ? Malloc(size) : NULL;
		S->Lpadding = 0;
		S->padding = 0;
		S->flags &= ~(AG_SURFACE_EXT_PIXELS);
	}
	S->pixels = S->pixelsBase;
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
 * Copy an entire palette to the palette of an Indexed surface.
 * If surface is not an Indexed surface, raise an exception.
 */
void
AG_SurfaceSetPalette(AG_Surface *S, const AG_Palette *a)
{
	AG_Palette *b;
	AG_Size size;

#ifdef AG_DEBUG
	if (S->format.mode != AG_SURFACE_INDEXED)
		AG_FatalError("Not INDEXED");
#endif
	b = Malloc(sizeof(AG_Palette));
	size = a->nColors * sizeof(AG_Color);
	b->colors = Malloc(size);
	b->nColors = a->nColors;
	memcpy(b->colors, a->colors, size);

	Free(S->format.palette);
	S->format.palette = b;
}

/*
 * Return a newly-allocated duplicate of a surface in the same format.
 * The duplicate will not include paddings present in the orignal surface.
 */
AG_Surface *
AG_SurfaceDup(const AG_Surface *a)
{
	AG_Surface *b;

	b = AG_SurfaceNew(&a->format,
	    a->w,
	    a->h,
	    (a->flags & AG_SAVED_SURFACE_FLAGS));

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

	AG_SurfaceCopy(b, a);
	return (b);
}

/*
 * Copy raw pixel data from a source surface S to a destination surface D.
 * Block copy linewise if the formats allow, perform conversion otherwise.
 * Clip coordinates if the surfaces have different dimensions.
 *
 * Unlike the blit operation, coordinates are fixed (0,0), clipping rectangle
 * is ignored and alpha/colorkey settings of the target surface are ignored.
 */
void
AG_SurfaceCopy(AG_Surface *D, const AG_Surface *S)
{
	const Uint w = MIN(S->w, D->w);                 /* Width to copy */
	const Uint h = MIN(S->h, D->h);                 /* Height to copy */
	int x, y;

	if (AG_PixelFormatCompare(&S->format, &D->format) == 0) {
#ifdef DEBUG_SURFACE
		if (S->flags & AG_SURFACE_TRACE) {
			Debug(NULL, "SURFACE(%dx%dx%d): Block Copy to (%dx%d).\n",
			    S->w, S->h, S->format.BitsPerPixel,
			    D->w, D->h);
		}
#endif
		if (D->format.BitsPerPixel < 8) {         /* <8bpp block copy */
			const Uint8 *pSrc = S->pixels;
			Uint8 *pDst = D->pixels;
			Uint Spadding = S->padding, Dpadding = D->padding;
			const Uint pitch = (w >> D->format.PixelsPerByteShift);

			if (D->w > S->w) {
				Dpadding += (D->w - S->w) >> D->format.PixelsPerByteShift;
			} else if (D->w < S->w) {
				Spadding += (S->w - D->w) >> S->format.PixelsPerByteShift;
			}
			for (y = 0; y < h; y++) {
				pSrc += S->Lpadding;
				pDst += D->Lpadding;

				memcpy(pDst, pSrc, pitch);

				pSrc += pitch + Spadding;
				pDst += pitch + Dpadding;
			}
		} else {                                 /* >=8bpp block copy */
			const Uint8 *pSrc = S->pixels;
			Uint8 *pDst = D->pixels;
			Uint Spadding = S->padding, Dpadding = D->padding;
			const Uint pitch = (w * D->format.BytesPerPixel);

			if (D->w > S->w) {
				Dpadding += (D->w - S->w) * D->format.BytesPerPixel;
			} else if (D->w < S->w) {
				Spadding += (S->w - D->w) * S->format.BytesPerPixel;
			}
			for (y = 0; y < h; y++) {
				pSrc += S->Lpadding;
				pDst += D->Lpadding;

				memcpy(pDst, pSrc, pitch);

				pSrc += pitch + Spadding;
				pDst += pitch + Dpadding;
			}
		}
		return;
	}

#ifdef DEBUG_SURFACE
	if (S->flags & AG_SURFACE_TRACE) {
		Debug(NULL,
		    "SURFACE(%dx%dx%d): Conversion to (%s, %dx%dx%d)\n",
		    S->w, S->h, S->format.BitsPerPixel,
		    agSurfaceModeNames[D->format.mode],
		    D->w, D->h, D->format.BitsPerPixel);
	}
#endif
	if (S->format.mode == AG_SURFACE_PACKED) {    /* Packed RGB(A) source */
		const Uint8 *pSrc = S->pixels;
		Uint8 *pDst = D->pixels;

		for (y = 0; y < h; y++) {
			pSrc += S->Lpadding;
			pDst += D->Lpadding;
			for (x = 0; x < w; x++) {
				AG_Pixel px;
				AG_Color c;
			
				px = AG_SurfaceGet_At(S, pSrc);
				AG_GetColor(&c, px, &S->format);
				AG_SurfacePut_At(D, pDst,
				    AG_MapPixel(&D->format, &c));

				pSrc += S->format.BytesPerPixel;
				pDst += D->format.BytesPerPixel;
			}
			pSrc += S->padding;
			pDst += D->padding;
		}
	} else if (S->format.BitsPerPixel == 8) {     /* 8-bpp Indexed source */
		const Uint8 *pSrc = S->pixels;
		const AG_Palette *pal = S->format.palette;

		for (y = 0; y < h; y++) {
			pSrc += S->Lpadding;
			for (x = 0; x < w; x++) {
				AG_Color c;
			
				c = pal->colors[*pSrc % pal->nColors];
				AG_SurfacePut(D, x,y,
				    AG_MapPixel(&D->format, &c));
				pSrc++;
			}
			pSrc += S->padding;
		}
		if (D->format.mode == AG_SURFACE_INDEXED) {
			AG_SurfaceSetPalette(D, S->format.palette);
		}
	} else if (S->format.BitsPerPixel < 8) {     /* <8-bpp Indexed source */
		const AG_Palette *pal = S->format.palette;

		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				AG_Pixel px;
				AG_Color c;
			
				px = AG_SurfaceGet8(S, x,y);
				c = pal->colors[px % pal->nColors];
				AG_SurfacePut(D, x,y,
				    AG_MapPixel(&D->format, &c));
			}
		}
		if (D->format.mode == AG_SURFACE_INDEXED) {
			AG_SurfaceSetPalette(D, S->format.palette);
		}
	} else if (S->format.mode == AG_SURFACE_GRAYSCALE) {   /* Gray source */
		const Uint8 *pSrc = S->pixels;
		Uint8 *pDst = D->pixels;

		for (y = 0; y < h; y++) {
			pSrc += S->Lpadding;
			pDst += D->Lpadding;
			for (x = 0; x < w; x++) {
				AG_Pixel val;
				AG_Color c;
			
				switch (S->format.BitsPerPixel) {
				case 16:
					val = (*(Uint16 *)pSrc & 0xfff);
					c.b = c.g = c.r = AG_12toH(val);
					c.a = AG_4toH((*(Uint16 *)pSrc &
					              0xf000) >> 12);
					break;
				case 32:
					val = (*(Uint32 *)pSrc & 0xffffff);
					c.b = c.g = c.r = AG_24toH(val);
					c.a = AG_8toH((*(Uint32 *)pSrc &
					              0xf0000000) >> 24);
					break;
#if AG_MODEL == AG_LARGE
				case 64:
					val = (*(Uint64 *)pSrc & 0xffffffffffff);
					c.b = c.g = c.r = AG_48toH(val);
					c.a = AG_16toH((*(Uint64 *)pSrc &
					               0xffff000000000000) >> 48);
					break;
#endif
				}
				AG_SurfacePut_At(D, pDst,
				    AG_MapPixel(&D->format, &c));

				pSrc += S->format.BytesPerPixel;
				pDst += D->format.BytesPerPixel;
			}
			pSrc += S->padding;
			pDst += D->padding;
		}
	}
}

/*
 * General lower blit for sub-8bpp source to any bpp destination.
 * Ignore per-surface alpha and colorkey.
 */
static void
AG_LowerBlit_Sub8_to_Any(AG_Surface *D, const AG_Rect *rd, const AG_Surface *S,
    const AG_Rect *rs)
{
	const int w  = rd->w, h  = rd->h;
	const int xs = rs->x, ys = rs->y;
	const int xd = rd->x, yd = rd->y;
	int x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			Uint8 px;
			AG_Color c;

			px = AG_SurfaceGet8(S, (xs + x), (ys + y));

			AG_GetColor(&c, px, &S->format);
			if (c.a == AG_TRANSPARENT) {
				continue;
			}
			AG_SurfaceBlend(D,
			    (xd + x),
			    (yd + y),
			    &c);
		}
	}
}

/*
 * General lower blit for >= 8bpp source to 8-bpp destination.
 * Ignore per-surface alpha and colorkey.
 */
static void
AG_LowerBlit_Packed_to_8(AG_Surface *D, const AG_Rect *rd,
    const AG_Surface *S, const AG_Rect *rs)
{
	const int w  = rd->w, h  = rd->h;
	const int xs = rs->x, ys = rs->y;
	const int xd = rd->x, yd = rd->y;
	int x, y;

#ifdef DEBUG_BLITTER_MAP
	if (S->format.mode != AG_SURFACE_PACKED ||
	    D->format.mode != AG_SURFACE_INDEXED ||
	    D->format.BitsPerPixel != 8)
		AG_FatalError("Bad blitter map");
#endif
	for (y = 0; y < h; y++) {
		const Uint8 *pSrc = S->pixels +
		    ((ys + y) * S->pitch) + S->Lpadding +
		     (xs * S->format.BytesPerPixel);
		Uint8 *pDst = D->pixels +
		    ((yd + y) * D->pitch) + D->Lpadding +
		      xd;

		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet_At(S, pSrc);
			AG_GetColor(&c, px, &S->format);
			if (c.a == AG_OPAQUE) {
				AG_SurfacePut_At(D, pDst,
				    AG_MapPixel(&D->format, &c));
			} else if (c.a != AG_TRANSPARENT) {
				AG_SurfaceBlend_At(D, pDst, &c);
			}
			pSrc += S->format.BytesPerPixel;
			pDst++;
		}
	}
}

/*
 * General lower blit for >= 8 bpp packed source to sub-8bpp destination.
 * Ignore per-surface alpha and colorkey.
 */
static void
AG_LowerBlit_Packed_to_Sub8(AG_Surface *D, const AG_Rect *rd,
    const AG_Surface *S, const AG_Rect *rs)
{
	const int w  = rd->w, h  = rd->h;
	const int xs = rs->x, ys = rs->y;
	const int xd = rd->x, yd = rd->y;
	int x, y;

#ifdef DEBUG_BLITTER_MAP
	if (S->format.mode != AG_SURFACE_PACKED ||
	    D->format.mode != AG_SURFACE_INDEXED || D->format.BitsPerPixel >= 8)
		AG_FatalError("Bad blitter map");
#endif
	for (y = 0; y < h; y++) {
		const Uint8 *pSrc = S->pixels +
		    ((ys + y) * S->pitch) + S->Lpadding +
		     (xs * S->format.BytesPerPixel);

		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet_At(S, pSrc);
			AG_GetColor(&c, px, &S->format);
			if (c.a != AG_TRANSPARENT) {
				AG_SurfaceBlend8(D, (xd+x), (yd+y), &c);
			}
			pSrc += S->format.BytesPerPixel;
		}
	}
}

/*
 * General lower blit for >= 8 bpp packed source to grayscale destination.
 * Ignore per-surface alpha and colorkey.
 */
static void
AG_LowerBlit_Packed_to_Grayscale(AG_Surface *D, const AG_Rect *rd,
    const AG_Surface *S, const AG_Rect *rs)
{
	const int w  = rd->w, h  = rd->h;
	const int xs = rs->x, ys = rs->y;
	const int xd = rd->x, yd = rd->y;
	int x, y;

#ifdef DEBUG_BLITTER_MAP
	if (S->format.mode != AG_SURFACE_PACKED ||
	    D->format.mode != AG_SURFACE_GRAYSCALE)
		AG_FatalError("Bad blitter map");
#endif
	for (y = 0; y < h; y++) {
		const Uint8 *pSrc = S->pixels +
		    ((ys + y) * S->pitch) + S->Lpadding +
		     (xs * S->format.BytesPerPixel);
		Uint8 *pDst = D->pixels +
		    ((yd + y) * D->pitch) + D->Lpadding +
		     (xd * D->format.BytesPerPixel);

		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet_At(S, pSrc);
			AG_GetColor(&c, px, &S->format);

			if (c.a != AG_TRANSPARENT)
				AG_SurfacePut_At(D, pDst,
				    AG_MapPixel(&D->format, &c));

			pSrc += S->format.BytesPerPixel;
			pDst += D->format.BytesPerPixel;
		}
	}
}

/*
 * General lower blit for >= 8bpp source to >= 8bpp destination.
 * Ignore per-surface alpha and colorkey.
 */
static void
AG_LowerBlit_Any_to_Any(AG_Surface *D, const AG_Rect *rd, const AG_Surface *S,
    const AG_Rect *rs)
{
	const Uint8 *pSrc;
	Uint8 *pDst;
	const int w  = rd->w, h  = rd->h;
	const int xs = rs->x, ys = rs->y;
	const int xd = rd->x, yd = rd->y;
	int x, y;

	for (y = 0; y < h; y++) {
		pSrc = S->pixels + ((ys + y) * S->pitch) + S->Lpadding +
		                    (xs * S->format.BytesPerPixel);
		pDst = D->pixels + ((yd + y) * D->pitch) + D->Lpadding +
       		                    (xd * D->format.BytesPerPixel);
		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet_At(S, pSrc);
			AG_GetColor(&c, px, &S->format);
			if (c.a != AG_TRANSPARENT) {
				AG_SurfaceBlend_At(D, pDst, &c);
			}
			pSrc += S->format.BytesPerPixel;
			pDst += D->format.BytesPerPixel;
		}
	}
}

/*
 * General lower blit for >= 8bpp source to >= 8bpp destination.
 * Honor per-surface alpha and colorkey.
 */
static void
AG_LowerBlit_AlCo(AG_Surface *D, const AG_Rect *rd, const AG_Surface *S,
    const AG_Rect *rs)
{
	AG_Pixel srcColorkey = S->colorkey;
	const Uint8 *pSrc;
	Uint8 *pDst;
	const int w = rd->w;
	const int h = rd->h;
	int x,y;

	for (y = 0; y < h; y++) {
		pSrc = S->pixels + ((rs->y + y) * S->pitch) + S->Lpadding +
		                    (rs->x * S->format.BytesPerPixel);
		pDst = D->pixels + ((rd->y + y) * D->pitch) + D->Lpadding +
       		                    (rd->x * D->format.BytesPerPixel);
		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet_At(S,pSrc);
			if (px == srcColorkey) {
				goto next_pixel;
			}
			AG_GetColor(&c, px, &S->format);
			c.a = MIN(c.a, S->alpha);
			if (c.a == AG_TRANSPARENT) {
				goto next_pixel;
			}
			AG_SurfaceBlend_At(D, pDst, &c);
next_pixel:
			pSrc += S->format.BytesPerPixel;
			pDst += D->format.BytesPerPixel;
		}
	}
}

/*
 * General lower blit for >= 8bpp source to >= 8bpp destination.
 * Honor per-surface alpha; ignore colorkey.
 */
static void
AG_LowerBlit_Al(AG_Surface *D, const AG_Rect *rd, const AG_Surface *S,
    const AG_Rect *rs)
{
	const Uint8 *pSrc;
	Uint8 *pDst;
	int x, y;

	for (y = 0; y < rd->h; y++) {
		pSrc = S->pixels + ((rs->y + y) * S->pitch) + S->Lpadding +
		                    (rs->x * S->format.BytesPerPixel);
		pDst = D->pixels + ((rd->y + y) * D->pitch) + D->Lpadding +
       		                    (rd->x * D->format.BytesPerPixel);
		for (x = 0; x < rd->w; x++) {
			AG_Pixel px;
			AG_Color c;
		
			px = AG_SurfaceGet_At(S, pSrc);
			AG_GetColor(&c, px, &S->format);

			c.a = MIN(c.a, S->alpha);
			if (c.a != AG_TRANSPARENT) {
				AG_SurfaceBlend_At(D, pDst, &c);
			}
			pSrc += S->format.BytesPerPixel;
			pDst += D->format.BytesPerPixel;
		}
		pSrc += S->padding;
		pDst += D->padding;
	}
}

/*
 * General lower blit for >= 8bpp source to >= 8bpp destination.
 * Honor colorkey; ignore per-surface alpha.
 */
static void
AG_LowerBlit_Co(AG_Surface *D, const AG_Rect *rd, const AG_Surface *S,
    const AG_Rect *rs)
{
	const AG_Pixel srcColorkey = S->colorkey;
	const Uint8 *pSrc;
	Uint8 *pDst;
	const int w = rd->w;
	const int h = rd->h;
	int x, y;

	for (y = 0; y < h; y++) {
		pSrc = S->pixels + ((rs->y + y) * S->pitch) + S->Lpadding +
		                    (rs->x * S->format.BytesPerPixel);
		pDst = D->pixels + ((rd->y + y) * D->pitch) + D->Lpadding +
       		                    (rd->x * D->format.BytesPerPixel);
		for (x = 0; x < w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet_At(S,pSrc);
			if (px == srcColorkey) {
				goto next_pixel;
			}
			AG_GetColor(&c, px, &S->format);

			if (c.a == AG_TRANSPARENT) {
				goto next_pixel;
			}
			if (c.a == AG_OPAQUE) {
				AG_SurfacePut_At(D, pDst,
				    AG_MapPixel(&D->format, &c));
			} else {
				AG_SurfaceBlend_At(D, pDst, &c);
			}
next_pixel:
			pSrc += S->format.BytesPerPixel;
			pDst += S->format.BytesPerPixel;
		}
		pSrc += S->padding;
		pDst += D->padding;
	}
}

/*
 * Copy/blend a region of pixels (per srcRect) from a source surface S to a
 * destination surface D, at coordinates xDst,yDst of D.
 *
 * Source and destination cannot be the same surface. The surfaces formats may
 * differ, in which case conversion is done (see agLowerBlits_Std[] below).
 *
 * Clipping is performed according to the clipping rectangle of the destination
 * surface. The default clipping rectangle of a surface has the same dimensions
 * as the surface itself. Blitting completely outside the clipping rectangle is
 * a safe no-op.
 *
 * Alpha blending is performed only if the source surface has an alpha channel
 * (non-zero Amask) or a non-opaque per-surface alpha. Colorkey tests are done
 * only if the source surface has the AG_SURFACE_COLORKEY flag set.
 *
 * If the source rectangle rSrc is NULL then the entire surface is copied.
 */
void
AG_SurfaceBlit(const AG_Surface *S, const AG_Rect *rSrc, AG_Surface *D,
    int xDst, int yDst)
{
	AG_Rect rs, rd;
	AG_LowerBlit *lowerBlits;
	Uint want;
	int nLowerBlits, i;

#ifdef DEBUG_SURFACE
	if (S->flags & AG_SURFACE_TRACE) {
		Debug(NULL, "SURFACE(%dx%dx%d): " AGSI_BR_BLU "Blit" AGSI_RST " to (%dx%dx%d):[%d,%d]\n",
		    S->w, S->h, S->format.BitsPerPixel,
		    D->w, D->h, D->format.BitsPerPixel,
		    xDst, yDst);
	}
	if (D->flags & AG_SURFACE_TRACE) {
		Debug(NULL, "SURFACE(%dx%dx%d): " AGSI_BR_BLU "Blit" AGSI_RST " from (%dx%dx%d) at [%d,%d]\n",
		    D->w, D->h, D->format.BitsPerPixel,
		    S->w, S->h, S->format.BitsPerPixel,
		    xDst, yDst);
	}
#endif

	/*
	 * Upper Blit.
	 */
	if (S->alpha == AG_TRANSPARENT) {           /* Is fully transparent */
		return;
	}
	if (rSrc != NULL) {
		rs = *rSrc;
		if (rs.x < 0) { rs.x = 0; }
		if (rs.y < 0) { rs.y = 0; }
		if (rs.x + rs.w >= S->w) { rs.w = S->w - rs.x; }
		if (rs.y + rs.h >= S->h) { rs.h = S->h - rs.y; }
	} else {
		rs.x = 0;
		rs.y = 0;
		rs.w = S->w;
		rs.h = S->h;
	}
	rd.x = xDst;
	rd.y = yDst;
	rd.w = rs.w;
	rd.h = rs.h;
	if (!AG_RectIntersect(&rd, &rd, &D->clipRect)) {
		return;
	}
	if (rd.w < rs.w) {					/* Partial */
		if (xDst < D->clipRect.x) {
			rs.x += (D->clipRect.x - xDst);
		}
		rs.w -= (rs.w - rd.w);
	}
	if (rd.h < rs.h) {
		if (yDst < D->clipRect.y) {
			rs.y += (D->clipRect.y - yDst);
		}
		rs.h -= (rs.h - rd.h);
	}

	/*
	 * Lower Blit.
	 */
	want = 0;
	if (S->alpha < AG_OPAQUE)           { want |= AG_LOWERBLIT_PSALPHA_SRC; }
	if (S->flags & AG_SURFACE_COLORKEY) { want |= AG_LOWERBLIT_COLORKEY_SRC; }
	if (D->alpha < AG_OPAQUE)           { want |= AG_LOWERBLIT_PSALPHA_DST; }
	if (D->flags & AG_SURFACE_COLORKEY) { want |= AG_LOWERBLIT_COLORKEY_DST; }

	lowerBlits =  agLowerBlits[S->format.mode];
	nLowerBlits = agLowerBlits_Count[S->format.mode];

	for (i = 0; i < nLowerBlits; i++) {
		const AG_LowerBlit *b = &lowerBlits[i];

		if (((b->modeDst == AG_SURFACE_ANY ||
		      b->modeDst == D->format.mode)) &&
		    (b->depthSrc == S->format.BitsPerPixel || b->depthSrc == 0) &&
		    (b->depthDst == D->format.BitsPerPixel || b->depthDst == 0) &&
		    (b->caps == want) &&
		    (b->cpuExts & agCPU.ext) == b->cpuExts &&
		    (S->format.mode != AG_SURFACE_PACKED ||
		     D->format.mode != AG_SURFACE_PACKED ||
		     (((b->Rsrc == S->format.Rmask) || b->Rsrc == 0) &&
		      ((b->Gsrc == S->format.Gmask) || b->Gsrc == 0) &&
		      ((b->Bsrc == S->format.Bmask) || b->Bsrc == 0) &&
		      ((b->Asrc == S->format.Amask) || b->Asrc == 0) &&
		      ((b->Rdst == D->format.Rmask) || b->Rdst == 0) &&
		      ((b->Gdst == D->format.Gmask) || b->Gdst == 0) &&
		      ((b->Bdst == D->format.Bmask) || b->Bdst == 0) &&
		      ((b->Adst == D->format.Amask) || b->Adst == 0)))) {
			b->fn(D, &rd, S, &rs);
			return;
		}
	}
}

/*
 * Resize a surface to specified dimensions in pixels.
 *
 * If parameters are incorrect or insufficient memory, fail and return -1.
 * When growing the surface, new pixels are left uninitialized.
 * If EXT_PIXELS is set, discard address and revert to auto-allocated storage.
 * Paddings are reset. The clipping rectangle is reset to cover whole surface.
 * The pixels pointer of the surface is reset to pixelsBase.
 */
int
AG_SurfaceResize(AG_Surface *S, Uint w, Uint h)
{
	Uint8 *pixelsNew;
	Uint newPitch, newPadding;

	if (Get_Aligned_Pitch_Padding(S, w, &newPitch, &newPadding) == -1)
		AG_FatalError(NULL);

	if ((S->flags & AG_SURFACE_EXT_PIXELS) == 0) {
		if ((pixelsNew = TryRealloc(S->pixelsBase, h * newPitch)) == NULL) {
			return (-1);
		}
		S->pixelsBase = pixelsNew;
	} else {
		S->flags &= ~(AG_SURFACE_EXT_PIXELS);
		if ((S->pixelsBase = TryMalloc(h * newPitch)) == NULL)
			return (-1);
	}
	S->pixels = S->pixelsBase;
	S->w = w;
	S->h = h;
	S->pitch = newPitch;
	S->padding = newPadding;
	S->Lpadding = 0;
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
#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_MAPPED)
		AG_FatalError("Surface is in use");
# ifdef DEBUG_SURFACE
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "SURFACE(%dx%dx%d): Freed.\n",
		    S->w, S->h, S->format.BitsPerPixel);
# endif
#endif
	AG_PixelFormatFree(&S->format);

	if (S->flags & AG_SURFACE_ANIMATED) {
		Uint i;

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
		Free(S->pixelsBase);
	}
	if ((S->flags & AG_SURFACE_STATIC) == 0)
		free(S);
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

	if (S->format.mode == AG_SURFACE_INDEXED &&
	    S->format.BitsPerPixel < 8 &&
	    S->w < S->format.BitsPerPixel)
		AG_FatalError("(1,2,4)bpp surfaces must be "
		              "at least >=(8,4,2) pixels wide");

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
AG_FillRect(AG_Surface *S, const AG_Rect *rd, const AG_Color *c)
{
	AG_Rect r;
	AG_Pixel px;
	int x,y;

	if (rd != NULL) {
		const int cx = S->clipRect.x, cx2 = S->clipRect.x+S->clipRect.w;
		const int cy = S->clipRect.y, cy2 = S->clipRect.y+S->clipRect.h;

		r = *rd;
		if (r.x < cx)       { r.x = cx; }
		if (r.y < cy)       { r.y = cy; }
		if (r.x+r.w >= cx2) { r.w = cx2-r.x; }
		if (r.y+r.h >= cy2) { r.h = cy2-r.y; }
	} else {
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
	const Uint8 *p = S->pixels + (y * S->pitch) + S->Lpadding +
	                             (x >> S->format.PixelsPerByteShift);

#ifdef DEBUG_SURFACE_GET
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

/* Write the pixel x,y in a 1/2/4/8-bpp surface S. */
void
AG_SurfacePut8(AG_Surface *_Nonnull S, int x, int y, Uint8 px)
{
	Uint8 *p = S->pixels + (y * S->pitch) + S->Lpadding +
	                       (x >> S->format.PixelsPerByteShift);
#ifdef DEBUG_SURFACE_PUT
	if (x < 0 || x >= S->w ||
	    y < 0 || y >= S->h)
		AG_FatalError("Illegal SurfacePut8");
#endif
	switch (S->format.BitsPerPixel) {
	case 8:
		*p = px;
		break;
	case 4:
#ifdef DEBUG_SURFACE_PUT
		if (px > 0x0f) { AG_FatalError("Bad 4-bit pixel"); }
#endif
		switch (x % 2) {
		case 0: *p = (*p & 0xf0) | px;		break;
		case 1:	*p = (*p & 0x0f) | px << 4;	break;
		}
		break;
	case 2:
#ifdef DEBUG_SURFACE_PUT
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
#ifdef DEBUG_SURFACE_PUT
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

/* Blend the pixel x,y in a 1/2/4/8-bpp surface S against the color c. */
void
AG_SurfaceBlend8(AG_Surface *S, int x, int y, const AG_Color *c)
{
	Uint8 *p = S->pixels + (y * S->pitch) + S->Lpadding +
	                       (x >> S->format.PixelsPerByteShift);
	AG_Color pc;
	AG_Component a;
	Uint8 px = 0;

#ifdef DEBUG_SURFACE_PUT
	if (x < 0 || y < 0 || x >= S->w || y >= S->h)
		AG_FatalError("Illegal SurfaceBlend8");
#endif
	switch (S->format.BitsPerPixel) {
	case 8:
		px = (*p);
		AG_GetColor(&pc, px, &S->format);
		a = c->a;
		*p = AG_MapPixelIndexed(&S->format,
		   ((pc.r + ((c->r - pc.r) * a)) >> AG_COMPONENT_BITS),
		   ((pc.g + ((c->g - pc.g) * a)) >> AG_COMPONENT_BITS),
		   ((pc.b + ((c->b - pc.b) * a)) >> AG_COMPONENT_BITS),
		   AG_OPAQUE);
		break;
	case 4:
		switch (x % 2) {
		case 0: px = (*p & 0x0f);      break;
		case 1: px = (*p & 0xf0) >> 4; break;
		}

		AG_GetColor(&pc, px, &S->format);
		px = AG_MapPixelIndexed(&S->format, c->r, c->g, c->b, AG_OPAQUE);

		switch (x % 2) {
		case 0: *p = (*p & 0xf0) | px;		break;
		case 1:	*p = (*p & 0x0f) | px << 4;	break;
		}

		break;
	case 2:
		switch (x % 4) {
		case 0: px = (*p & 0x03);      break;
		case 1: px = (*p & 0x0c) >> 2; break;
		case 2: px = (*p & 0x30) >> 4; break;
		case 3: px = (*p & 0xc0) >> 6; break;
		}

		AG_GetColor(&pc, px, &S->format);
		px = AG_MapPixelIndexed(&S->format, c->r, c->g, c->b, AG_OPAQUE);

		switch (x % 4) {
		case 0:	*p = (*p & 0xfc) | px;		break;
		case 1:	*p = (*p & 0xf3) | px << 2;	break;
		case 2:	*p = (*p & 0xcf) | px << 4;	break;
		case 3:	*p = (*p & 0x3f) | px << 6;	break;
		}

		break;
	case 1:
		switch (x % 8) {
		case 0:	px = (*p & 0x01);
		case 1:	px = (*p & 0x02) >> 1;
		case 2:	px = (*p & 0x04) >> 2;
		case 3:	px = (*p & 0x08) >> 3;
		case 4:	px = (*p & 0x10) >> 4;
		case 5:	px = (*p & 0x20) >> 5;
		case 6:	px = (*p & 0x40) >> 6;
		case 7:	px = (*p & 0x80) >> 7;
		}

		if (MAX3(c->r, c->g, c->b) > AG_COLOR_LAST/2) {    /* Value */
			px = 1;
		} else {
			px = 0;
		}

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

/*
 * Functions for mapping RGBA components to encoded pixel data.
 */

/* Map 8bpc RGB components to an opaque 32-bit pixel. */
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

/* Map 8bpc RGBA components to a 32-bit pixel. */
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

/* Map 16bpc RGB components to an opaque 32-bit pixel. */
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

/* Map 16bpc RGBA components to a 32-bit pixel. */
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

/* Map 8bpc RGB components to an opaque 64-bit pixel. */
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

/* Map 8bpc RGBA components to a 64-bit pixel. */
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

/*
 * Functions for extracting RGBA components from encoded pixel data.
 */

#if AG_MODEL == AG_LARGE

# undef  EXTRACT_COMPONENT8
# define EXTRACT_COMPONENT8(rv, mask, shift, loss)          \
         tmp = (px & mask) >> shift;                        \
         (rv) = agExpandByte8[(16 - (loss << 1))][tmp]

#else /* AG_MEDIUM */

# undef  EXTRACT_COMPONENT8
# define EXTRACT_COMPONENT8(rv, mask, shift, loss)          \
         tmp = (px & mask) >> shift;                        \
         (rv) = agExpandByte8[loss][tmp]

#endif /* AG_MEDIUM */


/* Extract 8bpc RGB components from a 32-bit pixel. */
void
AG_GetColor32_RGB8(Uint32 px, const AG_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b)
{
	if (pf->mode == AG_SURFACE_PACKED) {
		Uint32 tmp;

		EXTRACT_COMPONENT8(*r, pf->Rmask, pf->Rshift, pf->Rloss);
		EXTRACT_COMPONENT8(*g, pf->Gmask, pf->Gshift, pf->Gloss);
		EXTRACT_COMPONENT8(*b, pf->Bmask, pf->Bshift, pf->Bloss);
	} else if (pf->mode == AG_SURFACE_INDEXED) {
		const AG_Color *c = &pf->palette->colors[px %
		                                         pf->palette->nColors];
		*r = AG_Hto8(c->r);
		*g = AG_Hto8(c->g);
		*b = AG_Hto8(c->b);
	} else if (pf->mode == AG_SURFACE_GRAYSCALE) {
		Uint8 dummy;

		AG_GetColor32_Gray8(px, r,g,b,&dummy);
	}
#ifdef AG_DEBUG
	else {
		AG_FatalError("Bad PixelFormat");
	}
#endif
}

/* Extract 8bpc RGBA components from a 32-bit pixel. */
void
AG_GetColor32_RGBA8(Uint32 px, const AG_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	if (pf->mode == AG_SURFACE_PACKED) {
		Uint32 tmp;

		EXTRACT_COMPONENT8(*r, pf->Rmask, pf->Rshift, pf->Rloss);
		EXTRACT_COMPONENT8(*g, pf->Gmask, pf->Gshift, pf->Gloss);
		EXTRACT_COMPONENT8(*b, pf->Bmask, pf->Bshift, pf->Bloss);
		EXTRACT_COMPONENT8(*a, pf->Amask, pf->Ashift, pf->Aloss);
	} else if (pf->mode == AG_SURFACE_INDEXED) {
		const AG_Color *c = &pf->palette->colors[(Uint)px %
		                                         pf->palette->nColors];
		*r = AG_Hto8(c->r);
		*g = AG_Hto8(c->g);
		*b = AG_Hto8(c->b);
		*a = AG_Hto8(c->a);
	} else if (pf->mode == AG_SURFACE_GRAYSCALE) {
		AG_GetColor32_Gray8(px, r,g,b,a);
	}
#ifdef AG_DEBUG
	else {
		AG_FatalError("Bad PixelFormat");
	}
#endif
}

/* Extract 16bpc RGB components from a 32-bit pixel. */
void
AG_GetColor32_RGB16(Uint32 px, const AG_PixelFormat *pf,
    Uint16 *r, Uint16 *g, Uint16 *b)
{
	if (pf->mode == AG_SURFACE_PACKED) {
		Uint32 tmp;
		Uint8 r8,g8,b8;

		EXTRACT_COMPONENT8(r8, pf->Rmask, pf->Rshift, pf->Rloss);
		EXTRACT_COMPONENT8(g8, pf->Gmask, pf->Gshift, pf->Gloss);
		EXTRACT_COMPONENT8(b8, pf->Bmask, pf->Bshift, pf->Bloss);
		*r = AG_8to16(r8);
		*g = AG_8to16(g8);
		*b = AG_8to16(b8);
	} else if (pf->mode == AG_SURFACE_INDEXED) {
		const AG_Color *c = &pf->palette->colors[(Uint)px %
		                                         pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
	} else if (pf->mode == AG_SURFACE_GRAYSCALE) {
		Uint16 dummy;

		AG_GetColor32_Gray16(px, r,g,b,&dummy);
	}
#ifdef AG_DEBUG
	else {
		AG_FatalError("Bad PixelFormat");
	}
#endif
}

/* Extract 16bpc RGBA components from a 32-bit pixel. */
void
AG_GetColor32_RGBA16(Uint32 px, const AG_PixelFormat *pf,
    Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
{
	if (pf->mode == AG_SURFACE_PACKED) {
		Uint32 tmp;
		Uint8 r8,g8,b8,a8;

		EXTRACT_COMPONENT8(r8, pf->Rmask, pf->Rshift, pf->Rloss);
		EXTRACT_COMPONENT8(g8, pf->Gmask, pf->Gshift, pf->Gloss);
		EXTRACT_COMPONENT8(b8, pf->Bmask, pf->Bshift, pf->Bloss);
		EXTRACT_COMPONENT8(a8, pf->Amask, pf->Ashift, pf->Aloss);
		*r = AG_8to16(r8);
		*g = AG_8to16(g8);
		*b = AG_8to16(b8);
		*a = AG_8to16(a8);
	} else if (pf->mode == AG_SURFACE_INDEXED) {
		const AG_Color *c = &pf->palette->colors[(Uint)px %
		                                         pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		*a = c->a;
	} else if (pf->mode == AG_SURFACE_GRAYSCALE) {
		AG_GetColor32_Gray16(px, r,g,b,a);
	}
#ifdef AG_DEBUG
	else {
		AG_FatalError("Bad PixelFormat");
	}
#endif
}

/* Return an AG_Color for a 32-bit pixel. */
void
AG_GetColor32(AG_Color *c, Uint32 px, const AG_PixelFormat *pf)
{
	if (pf->mode == AG_SURFACE_PACKED) {
		Uint32 tmp;
		Uint8 r8,g8,b8,a8;

		EXTRACT_COMPONENT8(r8, pf->Rmask, pf->Rshift, pf->Rloss);
		EXTRACT_COMPONENT8(g8, pf->Gmask, pf->Gshift, pf->Gloss);
		EXTRACT_COMPONENT8(b8, pf->Bmask, pf->Bshift, pf->Bloss);
		c->r = AG_8to16(r8);
		c->g = AG_8to16(g8);
		c->b = AG_8to16(b8);
		if (pf->Amask != 0) {
			EXTRACT_COMPONENT8(a8, pf->Amask, pf->Ashift, pf->Aloss);
			c->a = AG_8to16(a8);
		} else {
			c->a = AG_OPAQUE;
		}
	} else if (pf->mode == AG_SURFACE_INDEXED) {
		memcpy(c, &pf->palette->colors[(Uint)px % pf->palette->nColors],
		    sizeof(AG_Color));
	} else if (pf->mode == AG_SURFACE_GRAYSCALE) {
		AG_GetColor32_Gray(c, px);
	}
#ifdef AG_DEBUG
	else {
		AG_FatalError("Bad PixelFormat");
	}
#endif
}

/* Extract an AG_Color from a 64-bit pixel */
#if AG_MODEL == AG_LARGE
void
AG_GetColor64(AG_Color *c, Uint64 px, const AG_PixelFormat *pf)
{
	if (pf->mode == AG_SURFACE_PACKED) {
		Uint64 tmp;

		EXTRACT_COMPONENT16(c->r, pf->Rmask, pf->Rshift, pf->Rloss);
		EXTRACT_COMPONENT16(c->g, pf->Gmask, pf->Gshift, pf->Gloss);
		EXTRACT_COMPONENT16(c->b, pf->Bmask, pf->Bshift, pf->Bloss);
		if (pf->Amask != 0) {
			EXTRACT_COMPONENT16(c->a, pf->Amask, pf->Ashift,
			                          pf->Aloss);
		} else {
			c->a = AG_OPAQUE;
		}
	} else if (pf->mode == AG_SURFACE_INDEXED) {
		memcpy(c, &pf->palette->colors[px % pf->palette->nColors],
		    sizeof(AG_Color));
	} else if (pf->mode == AG_SURFACE_GRAYSCALE) {
		AG_GetColor64_Gray(c, px);
	}
# ifdef AG_DEBUG
	else {
		AG_FatalError("Bad PixelFormat");
	}
# endif
}
#endif /* AG_LARGE */

#undef EXTRACT_COMPONENT8
#undef EXTRACT_COMPONENT16

/* Return palette entry closest to given 16bpc RGBA components. */
AG_Pixel
AG_MapPixelIndexed(const AG_PixelFormat *pf,
    AG_Component r, AG_Component g, AG_Component b, AG_Component a)
{
	Uint32 err, errMin=AG_COLOR_LAST*4;
	Uint i, iMin=0;

	for (i=0; i < pf->palette->nColors; i++) {
		AG_Color *C = &pf->palette->colors[i];
		double dr = (double)abs(C->r - r);
		double dg = (double)abs(C->g - g);
		double db = (double)abs(C->b - b);
	
		/*
		 * Use the Euclidean distance method.
		 * TODO: alternate methods
		 */
		err = (Uint32)(sqrt(dr*dr + dg*dg + db*db));

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
	const double R = (double)r / AG_COLOR_LASTD;
	const double G = (double)g / AG_COLOR_LASTD;
	const double B = (double)b / AG_COLOR_LASTD;
	double lum;

	switch (pf->graymode) {
	case AG_GRAYSCALE_BT709:
	default:
		lum = ( 0.21*R +  0.72*G +  0.07*B);
		break;
	case AG_GRAYSCALE_RMY:
		lum = (  0.5*R + 0.419*G + 0.081*B);
		break;
	case AG_GRAYSCALE_Y:
		lum = (0.299*R + 0.587*G + 0.114*B);
		break;
	}

	switch (pf->BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 64:
		return (Uint64)(lum * 281474976710655.0) |
		      ((Uint64)(a) << 48);
#endif
	case 32:
		return (Uint32)(lum * 16777215.0) |
		      ((Uint32)(AG_Hto8(a)) << 24);
	case 16:
		return (Uint16)(lum * 4095.0) |
		      ((Uint16)(AG_Hto4(a)) << 12);
	}
	return (0);
}

/* Convert a 32-bit grayscale+alpha pixel to an AG_Color by luminosity. */
void
AG_GetColor32_Gray(AG_Color *c, Uint32 G)
{
	const float lum = (G & 0xffffff) / 16777215.0f;
	const Uint8 a = (G & 0xff000000) >> 24;

	c->r = (AG_Component)(lum * AG_COLOR_LASTF);
	c->g = (AG_Component)(lum * AG_COLOR_LASTF);
	c->b = (AG_Component)(lum * AG_COLOR_LASTF);
	c->a = AG_8toH(a);
}

/* Convert a 32-bit grayscale+alpha pixel to 8bpc RGBA by luminosity. */
void
AG_GetColor32_Gray8(Uint32 G, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	const float lum = (G & 0xffffff) / 16777215.0f;

	*r = (Uint8)(lum * 255.0f);
	*g = (Uint8)(lum * 255.0f);
	*b = (Uint8)(lum * 255.0f);
	*a = (G & 0xff000000) >> 24;
}

/* Convert a 32-bit grayscale+alpha pixel to 16bpc RGBA by luminosity. */
void
AG_GetColor32_Gray16(Uint32 G, Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
{
	const float lum = (G & 0xffffff) / 16777215.0f;

	*r = (Uint16)(lum * 65535.0f);
	*g = (Uint16)(lum * 65535.0f);
	*b = (Uint16)(lum * 65535.0f);
	*a = AG_8to16((G & 0xff000000) >> 24);
}

#if AG_MODEL == AG_LARGE
/*
 * Convert a 64-bit grayscale+alpha pixel to 16bpc RGBA by luminosity.
 */
void
AG_GetColor64_Gray16(Uint64 G, Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
{
	const double lum = (G & 0xffffffffffff) / 281474976710655.0;

	*r = (Uint16)(lum * 65535.0);
	*g = (Uint16)(lum * 65535.0);
	*b = (Uint16)(lum * 65535.0);
	*a = (G & 0xffff000000000000) >> 48;
}

/*
 * Convert a 64-bit grayscale+alpha pixel to an AG_Color by luminosity.
 */
void
AG_GetColor64_Gray(AG_Color *c, Uint64 G)
{
	const double lum = (G & 0xffffffffffff) / 281474976710655.0;

	c->r = (AG_Component)(lum * AG_COLOR_LASTD);
	c->g = (AG_Component)(lum * AG_COLOR_LASTD);
	c->b = (AG_Component)(lum * AG_COLOR_LASTD);
	c->a = (AG_Component)((G & 0xffff000000000000) >> 48);
}
#endif /* AG_LARGE */

/*
 * Blend 8bpc RGBA components with the pixel at x,y and overwrite it
 * with the result. No clipping is done.
 */
void
AG_SurfaceBlendRGB8(AG_Surface *S, int x, int y, Uint8 r, Uint8 g, Uint8 b,
    Uint8 a)
{
	AG_Color c;

	c.r = AG_8toH(r);
	c.g = AG_8toH(g);
	c.b = AG_8toH(b);
	c.a = AG_8toH(a);

	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch + S->Lpadding +
	    x*S->format.BytesPerPixel,
	    &c);
}

/*
 * Blend 8bpc RGBA components with the pixel at p and overwrite it with
 * the result. No clipping is done.
 */
void
AG_SurfaceBlendRGB8_At(AG_Surface *S, Uint8 *p,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_Color c;

	c.r = AG_8toH(r);
	c.g = AG_8toH(g);
	c.b = AG_8toH(b);
	c.a = AG_8toH(a);
	AG_SurfaceBlend_At(S, p, &c);
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
 * Add a PIXELS instruction to an animation. Decoders will combine the pixels
 * (copied from s) against those of the previous frame (or background) in
 * order to generate the current frame.
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

/*
 * Packed-Source Blitters.
 */
const AG_LowerBlit agLowerBlits_Std_Packed[] = {
	/*                  |-Depth-|          |-----Masks----| */
	/* Destination       Src Dst  Cap Cpu  R,G,B,A, R,G,B,A */
	{ AG_SURFACE_INDEXED, 0,  1,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Packed_to_Sub8 },
	{ AG_SURFACE_INDEXED, 0,  2,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Packed_to_Sub8 },
	{ AG_SURFACE_INDEXED, 0,  4,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Packed_to_Sub8 },
	{ AG_SURFACE_INDEXED, 0,  8,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Packed_to_8 },
	{
		AG_SURFACE_PACKED, 0, 0,
		AG_LOWERBLIT_PSALPHA_SRC | AG_LOWERBLIT_COLORKEY_SRC, 0,
		0,0,0,0, 0,0,0,0,
		AG_LowerBlit_AlCo
	},
	{
		AG_SURFACE_PACKED, 0, 0,
		AG_LOWERBLIT_PSALPHA_SRC, 0,
		0,0,0,0, 0,0,0,0,
		AG_LowerBlit_Al
	},{
		AG_SURFACE_PACKED, 0, 0,
		AG_LOWERBLIT_COLORKEY_SRC, 0,
		0,0,0,0, 0,0,0,0,
		AG_LowerBlit_Co
	},{
		AG_SURFACE_GRAYSCALE, 0, 0,
		0, 0,
		0,0,0,0, 0,0,0,0,
		AG_LowerBlit_Packed_to_Grayscale
	},{
		AG_SURFACE_ANY, 0, 0,
		0, 0,
		0,0,0,0, 0,0,0,0,
		AG_LowerBlit_Any_to_Any
	},
};

/*
 * Indexed-Source Blitters.
 */
const AG_LowerBlit agLowerBlits_Std_Indexed[] = {
	/*                 |-Depth-|          |-----Masks----| */
	/* Destination      Src Dst  Cap Cpu  R,G,B,A, R,G,B,A */
	{ AG_SURFACE_ANY,    1,  0,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Sub8_to_Any },
	{ AG_SURFACE_ANY,    2,  0,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Sub8_to_Any },
	{ AG_SURFACE_ANY,    4,  0,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Sub8_to_Any },
	{ AG_SURFACE_ANY,    8,  0,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Sub8_to_Any },
	{ AG_SURFACE_ANY,    0,  0,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Sub8_to_Any }
};

/*
 * Grayscale-Source Blitters.
 */
const AG_LowerBlit agLowerBlits_Std_Grayscale[] = {
	/*                 |-Depth-|          |-----Masks----| */
	/* Destination      Src Dst  Cap Cpu  R,G,B,A, R,G,B,A */
	{ AG_SURFACE_ANY,    0,  0,   0,  0,  0,0,0,0, 0,0,0,0, AG_LowerBlit_Any_to_Any },
};

/*
 * Standard blitter table. This is copied to a reallocable array so
 * that user-defined blitters can be added.
 */
const AG_LowerBlit *agLowerBlits_Std[AG_SURFACE_MODE_LAST] = {
	agLowerBlits_Std_Packed,
	agLowerBlits_Std_Indexed,
	agLowerBlits_Std_Grayscale
};
const int agLowerBlits_Std_Count[AG_SURFACE_MODE_LAST] = {
	sizeof(agLowerBlits_Std_Packed)    / sizeof(AG_LowerBlit),
	sizeof(agLowerBlits_Std_Indexed)   / sizeof(AG_LowerBlit),
	sizeof(agLowerBlits_Std_Grayscale) / sizeof(AG_LowerBlit)
};

/* Deprecated interfaces */
#ifdef AG_LEGACY
Uint32 AG_MapRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b)                             { return AG_MapPixel32_RGB8 (pf, r,g,b); }
Uint32 AG_MapRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a)                   { return AG_MapPixel32_RGBA8(pf, r,g,b,a); }
Uint32 AG_MapPixelRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b)                        { return AG_MapPixel32_RGB8 (pf, r,g,b); }
Uint32 AG_MapPixelRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a)              { return AG_MapPixel32_RGBA8(pf, r,g,b,a); }
void AG_GetRGB(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b)                 { AG_GetColor32_RGB8 (px, pf, r,g,b); }
void AG_GetRGBA(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)      { AG_GetColor32_RGBA8(px, pf, r,g,b,a); }
void AG_GetPixelRGB(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b)            { AG_GetColor32_RGB8 (px, pf, r,g,b); }
void AG_GetPixelRGBA(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) { AG_GetColor32_RGBA8(px, pf, r,g,b,a); }
AG_Color AG_GetColorRGB(Uint32 px, const AG_PixelFormat *pf)                                      { AG_Color c; AG_GetColor32(&c, px, pf); c.a = AG_OPAQUE; return (c); }
AG_Color AG_GetColorRGBA(Uint32 px, const AG_PixelFormat *pf)                                     { AG_Color c; AG_GetColor32(&c, px, pf); return (c); }
int AG_ScaleSurface(const AG_Surface *s, Uint16 w, Uint16 h, AG_Surface **pDS)                    { *pDS = AG_SurfaceScale(s, (Uint)w, (Uint)h, 0); return (0); }
AG_Surface *AG_SurfaceStdGL(Uint w, Uint h)                                                       { return AG_SurfaceStdRGBA(w, h); }
#endif /* AG_LEGACY */

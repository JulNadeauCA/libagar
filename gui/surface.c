/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/gui/surface.h>
#include <agar/gui/gui_math.h>

#include <string.h>

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
	case 64:
#endif
		pf->PixelsPerByte = 0;
		break;
	default:
#if AG_MODEL == AG_LARGE
		AG_FatalError("Packed RGB must be 8,16,24,32 or 64 bits/pixel");
#else
		AG_FatalError("Packed RGB must be 8,16,24 or 32 bits/pixel");
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
		pf->Aloss = 0;
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
		AG_FatalError("Indexed must be 1,2,4 or 8 bits/pixel");
	}

	pal = pf->palette = Malloc(sizeof(AG_Palette));
	pal->nColors = 1<<BitsPerPixel;				/* 2^n colors */
	pal->colors = Malloc(pal->nColors*sizeof(AG_Color));
	pal->availFirst = pal->availLast = 0;

	if (BitsPerPixel == 1) {
		pal->colors[0] = AG_ColorWhite();
		pal->colors[1] = AG_ColorBlack();
	} else {
		memset(pal->colors, 0, pal->nColors*sizeof(AG_Color));
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
	case 8:
		pf->PixelsPerByte = 1;
		break;
	case 16:
	case 32:
#if AG_MODEL == AG_LARGE
	case 64:
#endif
		pf->PixelsPerByte = 0;
		break;
	default:
#if AG_MODEL == AG_LARGE
		AG_FatalError("Grayscale must be 16,32 or 64 bits/pixel");
#else
		AG_FatalError("Grayscale must be 16 or 32 bits/pixel");
#endif
	}

	pf->graymode = agGrayscaleMode;
}

/* Return a newly-allocated duplicate an AG_PixelFormat structure. */
AG_PixelFormat *_Nullable
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
		newPal->availFirst = origPal->availFirst;
		newPal->availLast = origPal->availLast;
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
	Uint len;

	if (pf->BitsPerPixel < 8) {
		switch (pf->BitsPerPixel) {
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
	switch (pf->BitsPerPixel) {
#if AG_MODEL == AG_LARGE
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
static __inline__ void
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
AG_SurfaceInit(AG_Surface *_Nonnull S, const AG_PixelFormat *pf,
    Uint w, Uint h, Uint flags)
{
	S->flags = flags;
	S->w = w;
	S->h = h;

	if (pf != NULL) {
		memcpy(&S->format, pf, sizeof(AG_PixelFormat));
		switch (pf->mode) {
		case AG_SURFACE_PACKED:
			break;
		case AG_SURFACE_INDEXED:
			AG_SurfaceSetPalette(S, pf->palette);
			break;
		case AG_SURFACE_GRAYSCALE:
			break;
		default:
			AG_FatalError("Bad PixelFormat mode");
		}
	}
	S->pixels = NULL;
	S->clipRect = AG_RECT(0,0,w,h);
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
	AG_SurfaceAllocPixels(S);
	return (S);
}

/*
 * Create a new surface in packed pixel mode (parametric form without alpha),
 * and initialize immediately from given pixel data.
 */
AG_Surface *
AG_SurfaceFromPixelsRGB(const void *_Nonnull pixels, Uint w, Uint h,
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
AG_SurfaceFromPixelsRGBA(const void *_Nonnull pixels, Uint w, Uint h, Uint BitsPerPixel,
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
AG_SurfaceFromFile(const char *_Nonnull path)
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
AG_SurfaceExportFile(const AG_Surface *_Nonnull S, const char *_Nonnull path)
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
 * Create a new surface suitable to be used as an OpenGL texture. The
 * returned surface size may be different from requested (unless the
 * NPOT extension is available).
 */
AG_Surface *
AG_SurfaceStdGL(Uint rw, Uint rh)
{
	AG_Surface *Sgl;
	int w, h;

	/* TODO check for GL_ARB_texture_non_power_of_two. */
	w = PowOf2i(rw);
	h = PowOf2i(rh);
	Sgl = AG_SurfaceRGBA(w, h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	);
	if (Sgl == NULL) {
		AG_FatalError(NULL);
	}
	Sgl->flags |= AG_SURFACE_GL_TEXTURE;
	return (Sgl);
}

/* Set pixel data to an external address (or NULL = revert to auto-allocated) */
void
AG_SurfaceSetAddress(AG_Surface *_Nonnull S, Uint8 *_Nonnull pixels)
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
AG_SurfaceSetColors(AG_Surface *_Nonnull S, AG_Color *_Nonnull c, Uint offs,
    Uint count)
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
 * Copy an entire palette to the palette of an indexed surface.
 * If surface is not an Indexed surface, raise an exception.
 */
void
AG_SurfaceSetPalette(AG_Surface *_Nonnull S, const AG_Palette *_Nonnull a)
{
	AG_Palette *b;
	AG_Size size;
	
	if (S->format.mode != AG_SURFACE_INDEXED) {
		AG_FatalError("Not INDEXED");
	}
	b = Malloc(sizeof(AG_Palette));
	size = a->nColors * sizeof(AG_Color);
	b->colors = Malloc(size);
	b->nColors = a->nColors;
	b->availFirst = a->availFirst;
	b->availLast = a->availLast;
	memcpy(b->colors, a->colors, size);

	Free(S->format.palette);
	S->format.palette = b;
}

/*
 * Mark a range of colors in the palette of an indexed surface as unused
 * (and available for blending).
 */
void
AG_SurfaceSetAvailColors(AG_Surface *_Nonnull S, Uint first, Uint last)
{
	AG_Palette *pal;

	if (S->format.mode != AG_SURFACE_INDEXED) {
		AG_FatalError("Not INDEXED");
	}
	pal = S->format.palette;
	pal->availFirst = first;
	pal->availLast = last;
}

/* Copy pixel data from an external address. */
void
AG_SurfaceCopyPixels(AG_Surface *_Nonnull S, const Uint8 *_Nonnull pixels)
{
	memcpy(S->pixels, pixels, (S->h * S->pitch));
}

/* Clear the entire surface with the given color */
void
AG_SurfaceSetPixels(AG_Surface *_Nonnull S, AG_Color c)
{
	AG_Pixel px = AG_MapPixel(&S->format, c);
	Uint x, y;

	/* TODO optimized cases */
	for (y = 0; y < S->h; y++) {
		for (x = 0; x < S->w; x++)
			AG_SurfacePut(S, x,y, px);
	}
}

/* Return a newly-allocated duplicate of a surface (in the same format). */
AG_Surface *
AG_SurfaceDup(const AG_Surface *_Nonnull a)
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
AG_SurfaceConvert(const AG_Surface *_Nonnull a, const AG_PixelFormat *_Nonnull pf)
{
	AG_Surface *b;

	b = AG_SurfaceNew(&a->format,
	    a->w,
	    a->h,
	    (a->flags & AG_SAVED_SURFACE_FLAGS));
	AG_SurfaceCopy(b, a);
	return (b);
}

/*
 * Copy pixel data from a source surface Ss to a destination surface Sd.
 * Convert if pixel format differs. Clip coordinates if dimensions differ.
 *
 * Unlike the blit operation, coordinates are fixed (0,0), clipping rectangle
 * is ignored and alpha/colorkey settings of the target surface are ignored.
 */
void
AG_SurfaceCopy(AG_Surface *_Nonnull Sd, const AG_Surface *_Nonnull Ss)
{
	Uint w = MIN(Ss->w, Sd->w);
	Uint h = MIN(Ss->h, Sd->h);
	int x, y;

	if (AG_PixelFormatCompare(&Ss->format, &Sd->format) == 0) {  /* Block */
		Uint8 *pSrc = Ss->pixels, *pDst = Sd->pixels;
		Uint SsPadding = Ss->padding;
		Uint SdPadding = Sd->padding;
		Uint pitch = (Sd->format.BitsPerPixel < 8) ?
		     w / Sd->format.PixelsPerByte :
		     w * Sd->format.BytesPerPixel;

		if (Sd->w > Ss->w) {
			SdPadding += (Sd->format.BitsPerPixel < 8) ?
			             (Sd->w - Ss->w) / Sd->format.PixelsPerByte :
			             (Sd->w - Ss->w) * Sd->format.BytesPerPixel;
		} else if (Sd->w < Ss->w) {
			SsPadding += (Ss->format.BitsPerPixel < 8) ?
			             (Ss->w - Sd->w) / Ss->format.PixelsPerByte :
			             (Ss->w - Sd->w) * Ss->format.BytesPerPixel;
		}
		for (y = 0; y < h; y++) {
			memcpy(pDst, pSrc, pitch);
			pSrc += pitch + SsPadding;
			pDst += pitch + SdPadding;
		}
	} else {                                                 /* Pixelwise */
		/* TODO optimized cases */
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				AG_Pixel px = AG_SurfaceGet(Ss, x,y);
				
				AG_SurfacePut(Sd, x,y,
				    AG_MapPixel(&Sd->format,
				        AG_GetColor(px, &Ss->format)));
			}
		}
	}
}

/*
 * Blit loop with both semi-transparent per-surface alpha and colorkey.
 * No opaque pixels are possible.
 */
static void
AG_SurfaceBlit_AlCo(const AG_Surface *_Nonnull Ss, AG_Surface *_Nonnull Sd,
    AG_Rect dr)
{
	AG_Pixel srcColorkey = Ss->colorkey;
	int x, y;

	for (y = 0; y < dr.h; y++) {
		for (x = 0; x < dr.w; x++) {
			AG_Pixel px = AG_SurfaceGet(Ss, x,y);
			AG_Color c;

			if (px == srcColorkey) {
				continue;
			}
			c = AG_GetColor(px, &Ss->format);
			c.a = MIN(c.a, Ss->alpha);
			if (c.a == AG_TRANSPARENT) {
				continue;
			}
			AG_SurfaceBlend(Sd, x,y, c, AG_ALPHA_OVERLAY);
		}
	}
}

/*
 * Blit loop with fully opaque per-surface alpha and colorkey.
 * Possibly some opaque pixels.
 */
static void
AG_SurfaceBlit_Co(const AG_Surface *_Nonnull Ss, AG_Surface *_Nonnull Sd,
    AG_Rect dr)
{
	AG_Pixel srcColorkey = Ss->colorkey;
	int x, y;

	for (y = 0; y < dr.h; y++) {
		for (x = 0; x < dr.w; x++) {
			AG_Pixel px = AG_SurfaceGet(Ss, x,y);
			AG_Color c;

			if (px == srcColorkey) {
				continue;
			}
			c = AG_GetColor(px, &Ss->format);
			if (c.a == AG_TRANSPARENT) {
				continue;
			} else if (c.a == AG_OPAQUE) {
				AG_SurfacePut(Sd, x,y,
				    AG_MapPixel(&Sd->format,c));
			} else {
				AG_SurfaceBlend(Sd, x,y, c, AG_ALPHA_OVERLAY);
			}
		}
	}
}

/*
 * Copy a region of pixels (per srcRect) from a source to a destination
 * surface at coordinates xDst,yDst. Coordinates are checked and clipped.
 * The source and destination surfaces cannot be the same.
 *
 * Perform alpha blending if Ss has AG_SURFACE_ALPHA flag. Honor component
 * alpha if Ss defines an alpha channel. Also honor per-surface alpha.
 *
 * Perform colorkey tests if Ss has AG_SURFACE_COLORKEY flag.
 *
 * If srcRect is NULL, the entire surface is copied.
 */
void
AG_SurfaceBlit(const AG_Surface *_Nonnull Ss, const AG_Rect *srcRect,
    AG_Surface *_Nonnull Sd, int xDst, int yDst)
{
	Uint srcBytesPerPixel, dstBytesPerPixel;
	AG_Rect sr, dr;
	Uint x, y;
	int diff;

	if (Ss->alpha == 0) {				/* Transparent */
		return;
	}
	if (srcRect != NULL) {
		sr = *srcRect;
		if (sr.x < 0) { sr.x = 0; }
		if (sr.y < 0) { sr.y = 0; }
		if (sr.x+sr.w >= Ss->w) { sr.w = Ss->w - sr.x; }
		if (sr.y+sr.h >= Ss->h) { sr.h = Ss->h - sr.y; }
	} else {
		sr.x = 0;
		sr.y = 0;
		sr.w = Ss->w;
		sr.h = Ss->h;
	}
	dr.x = xDst;
	dr.y = yDst;
	dr.w = sr.w;
	dr.h = sr.h;
	dr = AG_RectIntersect(dr, Sd->clipRect);
	if (dr.w <= 0 || dr.h <= 0) {				/* Outside */
		return;
	}
	if (dr.w < sr.w) {					/* Partial */
		diff = (sr.w - dr.w);
		if (xDst+sr.w < Sd->clipRect.x + Sd->clipRect.w) {
			sr.x += diff;
		}
		sr.w -= diff;
	}
	if (dr.h < sr.h) {
		diff = (sr.h - dr.h);
		if (yDst+sr.h < Sd->clipRect.y + Sd->clipRect.h) {
			sr.y += diff;
		}
		sr.h -= diff;
	}

	srcBytesPerPixel = Ss->format.BytesPerPixel;
	dstBytesPerPixel = Sd->format.BytesPerPixel;

	if (Ss->alpha < AG_OPAQUE) {                    /* Per-surface alpha */
		if (Ss->flags & AG_SURFACE_COLORKEY) {
			AG_SurfaceBlit_AlCo(Ss, Sd, dr);
		} else {                                      /* No colorkey */
			for (y = 0; y < dr.h; y++) {
				for (x = 0; x < dr.w; x++) {
					AG_Pixel px = AG_SurfaceGet(Ss, x,y);
					AG_Color c = AG_GetColor(px, &Ss->format);

					c.a = MIN(c.a, Ss->alpha);
					if (c.a == AG_TRANSPARENT) {
						continue;
					}
					AG_SurfaceBlend(Sd, x,y, c,
					    AG_ALPHA_OVERLAY);
				}
			}
		}
		return;
	}
	if (Ss->flags & AG_SURFACE_COLORKEY) {
		AG_SurfaceBlit_Co(Ss, Sd, dr);
	} else {
		if (Ss->flags & AG_SURFACE_ALPHA) {
			for (y = 0; y < dr.h; y++) {
				for (x = 0; x < dr.w; x++) {
					AG_Pixel px = AG_SurfaceGet(Ss, x,y);
					AG_Color c = AG_GetColor(px, &Ss->format);

					if (c.a == AG_TRANSPARENT) {
						continue;
					}
					if (c.a < AG_OPAQUE) {
						AG_SurfaceBlend(Sd, x,y, c,
						    AG_ALPHA_OVERLAY);
					} else {
						AG_SurfacePut(Sd, x,y,
						    AG_MapPixel(&Sd->format, c));
					}
				}
			}
		} else {
			for (y = 0; y < dr.h; y++) {
				for (x = 0; x < dr.w; x++) {
					AG_Pixel px = AG_SurfaceGet(Ss, x,y);
					AG_Color c = AG_GetColor(px, &Ss->format);

					AG_SurfacePut(Sd, x,y,
					    AG_MapPixel(&Sd->format, c));
				}
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
AG_SurfaceResize(AG_Surface *_Nonnull S, Uint w, Uint h)
{
	Uint8 *pixelsNew;
	Uint newPitch, newPadding;

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
	S->clipRect = AG_RECT(0,0,w,h);
	return (0);
}

/* Free the specified surface. */
void
AG_SurfaceFree(AG_Surface *_Nonnull S)
{
	Uint i;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_MAPPED)
		AG_FatalError("AG_SurfaceFree: in use");
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
AG_SurfaceScale(const AG_Surface *_Nonnull Ss, Uint w, Uint h, Uint flags)
{
	AG_Surface *ds;
	float xf, yf;
	int x,y;

	if (Ss->format.mode == AG_SURFACE_INDEXED &&
	    Ss->format.BitsPerPixel < 8 &&
	    Ss->w < Ss->format.BitsPerPixel)
		AG_FatalError("(1,2,4)bpp surfaces must be >=(8,4,2) pixels wide");

	ds = AG_SurfaceNew(&Ss->format, w,h, Ss->flags & AG_SAVED_SURFACE_FLAGS);
	ds->colorkey = Ss->colorkey;
	ds->alpha = Ss->alpha;

	if (Ss->w == w && Ss->h == h) {			/* Simple copy */
		AG_SurfaceCopy(ds, Ss);
		return (ds);
	}

	xf = (float)(Ss->w - 1) / (float)(ds->w - 1);
	yf = (float)(Ss->h - 1) / (float)(ds->h - 1);
	for (y = 0; y < ds->h; y++) {
		for (x = 0; x < ds->w; x++) {
			AG_SurfacePut(ds, x,y,
			    AG_SurfaceGet(Ss, 
			        (int)((float)x * xf),
				(int)((float)y * yf)));
		}
	}
	return (ds);
}

/* Fill a rectangle with pixels of the specified color. */
void
AG_FillRect(AG_Surface *_Nonnull S, const AG_Rect *rDst, AG_Color c)
{
	AG_Rect r;
	AG_Pixel px;
	int x,y;

	if (rDst != NULL) {
		int cx = S->clipRect.x, cx2 = S->clipRect.x+S->clipRect.w;
		int cy = S->clipRect.y, cy2 = S->clipRect.y+S->clipRect.h;

		r = *rDst;
		if (r.x < cx)       { r.x = cx; }
		if (r.y < cy)       { r.y = cy; }
		if (r.x+r.w >= cx2) { r.w = cx2-r.x; }
		if (r.y+r.h >= cy2) { r.h = cy2-r.y; }
	} else {
		r = S->clipRect;
	}
	px = AG_MapPixel(&S->format, c);

	/* XXX TODO optimize */
	for (y = 0; y < r.h; y++) {
		for (x = 0; x < r.w; x++) {
			AG_SurfacePut(S,
			    r.x + x,
			    r.y + y,
			    px);
		}
	}
}

/* Return the pixel at x,y in a 1- to 8-bpp surface. */
Uint8
AG_SurfaceGet8(const AG_Surface *_Nonnull S, int x, int y)
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

/* Map 8-bit RGB(A) components to a 32-bit pixel. */
Uint32
AG_MapPixel32_RGB8(const AG_PixelFormat *_Nonnull pf, Uint8 r, Uint8 g, Uint8 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
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
Uint32
AG_MapPixel32_RGBA8(const AG_PixelFormat *_Nonnull pf,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
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

/* Map compressed 16-bit RGB components to an opaque 32-bit pixel. */
Uint32
AG_MapPixel32_RGB16(const AG_PixelFormat *_Nonnull pf,
    Uint16 r, Uint16 g, Uint16 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
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

/* Map compressed 16-bit RGBA components to a 32-bit pixel. */
Uint32
AG_MapPixel32_RGBA16(const AG_PixelFormat *_Nonnull pf,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
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

/* Map decompressed 8-bit RGB components to an opaque 64-bit pixel. */
Uint64
AG_MapPixel64_RGB8(const AG_PixelFormat *_Nonnull pf,
    Uint8 r, Uint8 g, Uint8 b)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
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

/* Map decompressed 8-bit RGB components to a 64-bit pixel. */
Uint64
AG_MapPixel64_RGBA8(const AG_PixelFormat *_Nonnull pf,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
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

/* Extract 8-bit RGB(A) components from a 32-bit pixel. */
void
AG_GetColor32_RGB8(Uint32 px, const AG_PixelFormat *_Nonnull pf,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b)
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
void
AG_GetColor32_RGBA8(Uint32 px, const AG_PixelFormat *_Nonnull pf,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b, Uint8 *_Nonnull a)
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

#if AG_MODEL == AG_LARGE
/*
 * Extract decompressed 16-bit RGB(A) components from a 32-bit pixel.
 */
void
AG_GetColor32_RGB16(Uint32 px, const AG_PixelFormat *_Nonnull pf,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b)
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
void
AG_GetColor32_RGBA16(Uint32 px, const AG_PixelFormat *_Nonnull pf,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b,
    Uint16 *_Nonnull a)
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
#endif /* AG_LARGE */

/*
 * Extract an AG_Color from a 32-bit pixel.
 * Apply dynamic range decompression.
 */
AG_Color
AG_GetColor32(Uint32 px, const AG_PixelFormat *_Nonnull pf)
{
	AG_Color c;
	Uint8 R,G,B,A;
	Uint32 tmp;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		EXTRACT_COMPONENT(R, pf->Rmask, pf->Rshift, pf->Rloss, 8);
		EXTRACT_COMPONENT(G, pf->Gmask, pf->Gshift, pf->Gloss, 8);
		EXTRACT_COMPONENT(B, pf->Bmask, pf->Bshift, pf->Bloss, 8);
		EXTRACT_COMPONENT(A, pf->Amask, pf->Ashift, pf->Aloss, 8);
		c.r = AG_8to16(R);
		c.g = AG_8to16(G);
		c.b = AG_8to16(B);
		c.a = AG_8to16(A);
		break;
	case AG_SURFACE_INDEXED:
		return pf->palette->colors[(Uint)px % pf->palette->nColors];
	case AG_SURFACE_GRAYSCALE:
		return AG_GetColor32_Gray(px, pf->graymode);
	}
	return (c);
}

#undef EXTRACT_COMPONENT

/* Return palette entry closest to given 16-bit RGBA components. */
AG_Pixel
AG_MapPixelIndexed(const AG_PixelFormat *_Nonnull pf,
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
/* TODO integer-only version */
AG_Pixel
AG_MapPixelGrayscale(const AG_PixelFormat *_Nonnull pf,
    AG_Component r, AG_Component g, AG_Component b, AG_Component a)
{
	float R = (float)r/AG_COLOR_LASTF;
	float G = (float)g/AG_COLOR_LASTF;
	float B = (float)b/AG_COLOR_LASTF;
	float A = (float)a/AG_COLOR_LASTF;
	AG_GrayComponent lum;

	switch (pf->graymode) {
	case AG_GRAYSCALE_BT709:
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
/* TODO integer-only version */
AG_Color
AG_GetColor32_Gray(Uint32 G, AG_GrayscaleMode mode)
{
	AG_Color c;
	float lum = ((G & 0xffff0000) >> 8) / 65535.0f;

	switch (mode) {
	case AG_GRAYSCALE_BT709:
		c.r = (Uint16)(lum / 0.21f);
		c.g = (Uint16)(lum / 0.72f);
		c.b = (Uint16)(lum / 0.07f);
		break;
	case AG_GRAYSCALE_RMY:
		c.r = (Uint16)(lum / 0.500f);
		c.g = (Uint16)(lum / 0.419f);
		c.b = (Uint16)(lum / 0.081f);
		break;
	case AG_GRAYSCALE_Y:
		c.r = (Uint16)(lum / 0.299f);
		c.g = (Uint16)(lum / 0.587f);
		c.b = (Uint16)(lum / 0.114f);
		break;
	}
	c.a = (G & 0xffff);
	return (c);
}

/* Convert a 32-bit grayscale+alpha pixel to 8-bit RGBA by luminosity. */
/* TODO integer-only version */
void
AG_GetColor32_Gray8(Uint32 G, AG_GrayscaleMode grayMode,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b, Uint8 *_Nonnull a)
{
	float lum = (255.0f * (float)((G & 0xffff0000) >> 16)) / 65535.0f;

	switch (grayMode) {
	case AG_GRAYSCALE_BT709:
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
/* TODO integer-only version */
void
AG_GetColor32_Gray16(Uint32 px, AG_GrayscaleMode grayMode,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b,
    Uint16 *_Nonnull a)
{
	float lum = (float)((px & 0xffff0000) >> 16);

	switch (grayMode) {
	case AG_GRAYSCALE_BT709:
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
/* TODO integer-only version */
void
AG_GetColor64_Gray16(Uint64 G, AG_GrayscaleMode grayMode,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g,
    Uint16 *_Nonnull b, Uint16 *_Nonnull a)
{
	double lum = AG_32to16((G & 0xffffffff00000000) >> 16);

	switch (grayMode) {
	case AG_GRAYSCALE_BT709:
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
/* TODO integer-only version */
AG_Color
AG_GetColor64_Gray(Uint64 G, AG_GrayscaleMode mode)
{
	double lum = AG_32to16((G & 0xffffffff00000000) >> 16);
	AG_Color c;

	switch (mode) {
	case AG_GRAYSCALE_BT709:
		c.r = (Uint16)(lum / 0.21);
		c.g = (Uint16)(lum / 0.72);
		c.b = (Uint16)(lum / 0.07);
		break;
	case AG_GRAYSCALE_RMY:
		c.r = (Uint16)(lum / 0.500);
		c.g = (Uint16)(lum / 0.419);
		c.b = (Uint16)(lum / 0.081);
		break;
	case AG_GRAYSCALE_Y:
		c.r = (Uint16)(lum / 0.299);
		c.g = (Uint16)(lum / 0.587);
		c.b = (Uint16)(lum / 0.114);
		break;
	}
	c.a = AG_32to16(G & 0xffffffff);
	return (c);
}
#endif /* AG_LARGE */

/*
 * Blend 8-bit RGBA components with the pixel at x,y and overwrite it with
 * the result. Apply dynamic range decompression (8- to 16-bit). No clipping.
 * Target pixel alpha, if any, is set according to fn.
 */
void
AG_SurfaceBlendRGB8(AG_Surface *_Nonnull S, int x, int y,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a, AG_AlphaFn fn)
{
	AG_Color c = {
	    AG_8toH(r),
	    AG_8toH(g),
	    AG_8toH(b),
	    AG_8toH(a)
	};
	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    c, fn);
}

/*
 * Blend 8-bit RGBA components with the pixel at p and overwrite it with the
 * result. Apply dynamic range decompression (8- to 16-bit). No clipping.
 * Target pixel alpha, if any, is set according to fn.
 */
void
AG_SurfaceBlendRGB8_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p,
    Uint8 r, Uint8 g, Uint8 b, Uint8 a, AG_AlphaFn fn)
{
	AG_Color c = {
	    AG_8toH(r),
	    AG_8toH(g),
	    AG_8toH(b),
	    AG_8toH(a)
	};
	AG_SurfaceBlend_At(S, p, c, fn);
}

/* Initialize animation playback context. */
void
AG_AnimStateInit(AG_AnimState *_Nonnull ast, AG_Surface *_Nonnull s)
{
#ifdef AG_DEBUG
	if (!(s->flags & AG_SURFACE_ANIMATED)) { AG_FatalError("!ANIMATED"); }
#endif
	AG_MutexInitRecursive(&ast->lock);
	ast->s = s;
	ast->flags = 0;
	ast->play = 0;
	ast->f = 0;
}

void
AG_AnimStateDestroy(AG_AnimState *_Nonnull ast)
{
	ast->play = 0;
	AG_MutexDestroy(&ast->lock);
}

void
AG_AnimSetLoop(AG_AnimState *_Nonnull ast, int enable)
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
AG_AnimSetPingPong(AG_AnimState *_Nonnull ast, int enable)
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

	while (ast->play) {
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
					ast->play = 0;
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
					ast->play = 0;
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
AG_AnimPlay(AG_AnimState *_Nonnull ast)
{
	int rv = 0;

	AG_MutexLock(&ast->lock);
	ast->play = 1;
#ifdef AG_THREADS
	if (AG_ThreadTryCreate(&ast->th, AG_AnimThreadProc, ast) != 0) {
		AG_SetErrorS("Failed to create playback thread");
		rv = -1;
		ast->play = 0;
	}
#else
	AG_SetErrorS("AG_AnimPlay() requires threads");
	rv = -1;
#endif
	AG_MutexUnlock(&ast->lock);
	return (rv);
}

void
AG_AnimStop(AG_AnimState *_Nonnull ast)
{
	AG_MutexLock(&ast->lock);
	ast->play = 0;
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
AG_SurfaceAddFrame(AG_Surface *_Nonnull Sanim, const AG_Surface *_Nonnull Sframe,
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
/*
 * Legacy interfaces
 */
void AG_SurfaceLock(AG_Surface *su) { /* No-op */ }
void AG_SurfaceUnlock(AG_Surface *su) { /* No-op */ }

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
	return AG_MapPixel32_RGB16(pf, c.r, c.g, c.b);
}
Uint32 AG_MapColorRGBA(const AG_PixelFormat *pf, AG_Color c) {
	return AG_MapPixel32(pf, c);
}
AG_Surface *_Nonnull AG_DupSurface(AG_Surface *_Nonnull su) {
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
	AG_Color c = AG_GetColor32(px, pf);
	c.a = AG_OPAQUE;
	return (c);
}
AG_Color AG_GetColorRGBA(Uint32 px, const AG_PixelFormat *pf) {
	return AG_GetColor32(px, pf);
}
int AG_SamePixelFmt(const AG_Surface *a, const AG_Surface *b) {
	return (AG_PixelFormatCompare(&a->format, &b->format)) == 0;
}
int AG_ScaleSurface(const AG_Surface *s, Uint16 w, Uint16 h, AG_Surface **pDS) {
	*pDS = AG_SurfaceScale(s, (Uint)w, (Uint)h, 0);
	return (0);
}
#endif /* AG_LEGACY */

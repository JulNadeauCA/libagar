/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/micro/surface.h>

#include <string.h>

/*
 * Test whether two pixel formats are identical.
 * In indexed mode, include both palettes in the comparison.
 */
Sint8
MA_PixelFormatCompare(const MA_PixelFormat *a, const MA_PixelFormat *b)
{
	if (a->mode != b->mode ||
	    a->BitsPerPixel != b->BitsPerPixel) {
		return (1);
	}
	switch (a->mode) {
	case MA_SURFACE_PACKED:
		return !(a->Rmask == b->Rmask &&
			 a->Gmask == b->Gmask &&
			 a->Bmask == b->Bmask &&
			 a->Amask == b->Amask);
	case MA_SURFACE_INDEXED:
		return (a->data.palette - b->data.palette);
	default:
		return (1);
	}
}

/*
 * Return the (1- to 16-bit) pixel at coordinates x,y in surface S.
 */
Uint16
MA_SurfaceGet16(const MA_Surface *S, Uint8 x, Uint8 y)
{
#ifdef AG_DEBUG
	if (x >= S->w || y >= S->h)
		AG_FatalError("Illegal MA_SurfaceGet16() access");
#endif
	if (S->format.BitsPerPixel < 8) {
		return (Uint16)MA_SurfaceGet8(S, x,y);
	} else {
		return MA_SurfaceGet16_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel);
	}
}

/*
 * Return the (8- to 16-bit) pixel at address p in surface S.
 */
Uint16
MA_SurfaceGet16_At(const MA_Surface *_Nonnull S, const Uint8 *_Nonnull p)
{
	switch (S->format.BitsPerPixel) {
	case 16:
		return (*(Uint16 *)p);
	}
	return (*p);
}
	
/*
 * Write to the (1- to 16-bit) pixel at coordinates x,y in surface S.
 */
void
MA_SurfacePut16(MA_Surface *S, Uint8 x, Uint8 y, Uint16 px)
{
	if (S->format.BitsPerPixel < 8) {
		MA_SurfacePut8(S, x,y, (Uint8)px);
	} else {
		MA_SurfacePut16_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel, px);
	}
}

/*
 * Write to the (8- to 16-bit) pixel at address p in surface S.
 */
void
MA_SurfacePut16_At(MA_Surface *S, Uint8 *p, Uint16 px)
{
	switch (S->format.BitsPerPixel) {
	case 16:
		*(Uint16 *)p = (Uint16)px;
		break;
	default:
		*p = (Uint8)px;
		break;
	}
}

/*
 * Blend a color c with the pixel at x,y and overwrite it with the result.
 * No clipping is done. Sets the pixel's alpha component according to fn.
 */
void
MA_SurfaceBlend(MA_Surface *S, Uint8 x, Uint8 y, const MA_Color *c)
{
	MA_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    c, fn);
}

/*
 * Blend a color c with the pixel at address p and overwrite it with the result.
 * No clipping is done. Sets the pixel's alpha component according to fn.
 */
void
MA_SurfaceBlendAt(MA_Surface *S, Uint8 *p, const MA_Color *_Nonnull c)
{
	MA_Pixel px = MA_SurfaceGet_At(S,p);
	MA_Color pc;
	MA_Component ca;

	if ((S->flags & MA_SURFACE_COLORKEY) &&		/* No blending needed */
	    (px == S->colorkey)) {
		MA_SurfacePut_At(S, p, MA_MapPixel(&S->format, c));
		return;
	}
//	pc.a = MA_COLOR_LAST;				/* compiler happy */
	MA_GetColor(&pc, px, &S->format);
	ca = c->a;
#if 1
	pc.a = ((pc.a+ca) > AG_COLOR_LAST) ? AG_COLOR_LAST : (pc.a+ca);
#endif
	MA_SurfacePut_At(S, p,
	    AG_MapPixel_RGBA(&S->format,
	        pc.r + (((c->r - pc.r)*ca) >> MA_COMPONENT_BITS),
	        pc.g + (((c->g - pc.g)*ca) >> MA_COMPONENT_BITS),
	        pc.b + (((c->b - pc.b)*ca) >> MA_COMPONENT_BITS),
	        pc.a));
}

/*
 * Compute right shifts to extract RGBA components, as well as the
 * number of bits lost by packing components into our native fields.
 */
#define MA_GET_SHIFT_LOSS(mask, shift, loss) \
	shift = 0; \
	loss = MA_COMPONENT_BITS; \
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
MA_PixelFormatRGB(MA_PixelFormat *pf, Uint8 BitsPerPixel,
    MA_Pixel Rmask, MA_Pixel Gmask, MA_Pixel Bmask)
{
	MA_PixelFormatRGBA(pf, BitsPerPixel, Rmask, Gmask, Bmask, 0);
}

/* Initialize a packed-pixel format from RGBA component masks */
void
MA_PixelFormatRGBA(MA_PixelFormat *pf, Uint8 BitsPerPixel,
    MA_Pixel Rmask, MA_Pixel Gmask, MA_Pixel Bmask, MA_Pixel Amask)
{
	MA_Pixel m;
	
	pf->mode = MA_SURFACE_PACKED;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;

	switch (BitsPerPixel) {
	case 8:
		pf->PixelsPerByte = 1;
		break;
	case 16:
	case 24:
	case 32:
		pf->PixelsPerByte = 0;
		break;
	default:
		AG_FatalError("Packed RGB surfaces must be 8- or 16-bpp");
	}

	MA_GET_SHIFT_LOSS(Rmask, pf->Rshift, pf->Rloss);
	MA_GET_SHIFT_LOSS(Gmask, pf->Gshift, pf->Gloss);
	MA_GET_SHIFT_LOSS(Bmask, pf->Bshift, pf->Bloss);

	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;

	if (Amask == 0) {
		pf->Amask = 0;
		pf->Ashift = 0;
		pf->Aloss = MA_COMPONENT_BITS;
	} else {
		MA_GET_SHIFT_LOSS(Amask, pf->Ashift, pf->Aloss);
		pf->Amask = Amask;
	}
}

#undef MA_GET_SHIFT_LOSS

/*
 * Initialize a palletized format in 1-, 2-, 4- or 8-bpp mode.
 */
void
MA_PixelFormatIndexed(MA_PixelFormat *pf, Uint8 BitsPerPixel)
{
	pf->mode = MA_SURFACE_INDEXED;
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
}

/*
 * Initialize a Grayscale+Alpha format. Packing is as follows:
 *
 *       Byte#---Lo--- ---Hi---
 *        bit#76543210 76543210
 *            -------- --------
 *  2-bpp           VA
 *  4-bpp         VVAA
 *  8-bpp     VVVVAAAA
 * 16-bpp     VVVVVVVV AAAAAAAA
 */
void
MA_PixelFormatGrayscale(MA_PixelFormat *pf, int BitsPerPixel)
{
	pf->mode = MA_SURFACE_GRAYSCALE;
	pf->BitsPerPixel = BitsPerPixel;
	pf->BytesPerPixel = (BitsPerPixel + 7)/8;

	switch (BitsPerPixel) {
	case 2:	 pf->PixelsPerByte = 4;	break;
	case 4:	 pf->PixelsPerByte = 2;	break;
	case 8:	 pf->PixelsPerByte = 1;	break;
	case 16: pf->PixelsPerByte = 0;	break;
	default:
		AG_FatalError("Grayscale must be 2-, 4-, 8- or 16-bpp");
	}
}

/*
 * Calculate pitch and padding for a given surface depth and width.
 *
 * (1,2,4)-bpp surfaces must be at least (8,4,2) pixels wide.
 * Align 8- to 16-bpp surfaces to 2-byte boundary.
 */
static void
Get_Aligned_Pitch_Padding(const MA_Surface *_Nonnull S, Uint8 w,
    Uint8 *_Nonnull pitch, Uint8 *_Nonnull padding)
{
	const MA_PixelFormat *pf = &S->format;
	Uint8 len;

	if (pf->BitsPerPixel < 8) {
#ifdef AG_DEBUG
		switch (pf->BitsPerPixel) {
		case 1:
			if (w < 8) { AG_FatalError("1bpp SUs must >7 px wide"); }
			break;
		case 2:
			if (w < 4) { AG_FatalError("2bpp SUs must >3 px wide"); }
			break;
		case 4:
			if (w < 2) { AG_FatalError("4bpp SUs must >1 px wide"); }
			break;
		default:
			AG_FatalError("Bad bits/pixel");
		}
#endif /* AG_DEBUG */
		len = w / pf->PixelsPerByte;
	} else {
		len = w * pf->BytesPerPixel;
	}
	switch (pf->BitsPerPixel) {
	default:
		*pitch = (len + 2-1) & ~(2-1);		/* Align to 16-bit */
		break;
	}
	*padding = *pitch - len;
}

/* Allocate memory for w x h pixels (unless EXT_PIXELS is set). */
static void
MA_SurfaceAllocPixels(MA_Surface *_Nonnull S)
{
	AG_Size size;

	if (Get_Aligned_Pitch_Padding(S, S->w, &S->pitch, &S->padding) == -1)
		AG_FatalError(NULL);

	if (S->pixels != NULL && !(S->flags & MA_SURFACE_EXT_PIXELS)) {
		free(S->pixels);
	}
	if ((size = S->h*S->pitch) > 0 && !(S->flags & MA_SURFACE_EXT_PIXELS)) {
		S->pixels = Malloc(size);
	} else {
		S->pixels = NULL;
	}
}

/*
 * Initialize an MA_Surface to the specified format and size.
 * If pf specifies a palettized format, copy the palette from it.
 */
void
MA_SurfaceInit(MA_Surface *S, const MA_PixelFormat *pf, Uint8 w, Uint8 h,
    Uint8 flags)
{
	S->flags = flags;
	S->w = w;
	S->h = h;

	if (pf != NULL) {
		memcpy(&S->format, pf, sizeof(MA_PixelFormat));
		switch (pf->mode) {
		case MA_SURFACE_PACKED:
			S->format.data.packed = Malloc(sizeof(MA_PixelPacking));
			memcpy(S->format.data.packed, &pf->data.packed,
			       sizeof(MA_PixelPacking));
			break;
		case MA_SURFACE_INDEXED:
		case MA_SURFACE_GRAYSCALE:
			break;
		default:
			AG_FatalError("Bad PixelFormat");
		}
	}
	S->pixels = NULL;
	S->colorkey = 0;
	S->alpha = MA_OPAQUE;
}

/*
 * Create a new surface in specified format (any mode).
 *
 * Allocates uninitialized pixels (unless EXT_PIXELS flag is used).
 * Raise exception if insufficient memory is available.
 * Raise exception if given mode+BitsPerPixel combination is not supported.
 * {4,2,1}-bpp indexed surfaces must be at least {2,4,8}px wide.
 */
MA_Surface *
MA_SurfaceNew(const MA_PixelFormat *pf, Uint8 w, Uint8 h, Uint8 flags)
{
	MA_Surface *S;

	S = Malloc(sizeof(MA_Surface));
	MA_SurfaceInit(S, pf, w,h, flags);
	MA_SurfaceAllocPixels(S);
	return (S);
}

/*
 * Create a new surface in specified palettized mode.
 *
 * Allocate pixel data unless EXT_PIXELS flag is given.
 * Raise exception if insufficient memory is available.
 * Raise exception if given mode+BitsPerPixel combination is not supported.
 * {4,2,1}-bpp indexed surfaces must be at least {2,4,8}px wide.
 */
MA_Surface *
MA_SurfaceIndexed(Uint8 w, Uint8 h, Uint8 BitsPerPixel, Uint8 flags)
{
	MA_Surface *S;
	
	S = Malloc(sizeof(MA_Surface));
	MA_SurfaceInit(S, NULL, w,h, flags);
	MA_PixelFormatIndexed(&S->format, BitsPerPixel);
	MA_SurfaceAllocPixels(S);
	return (S);
}

/* Create a new surface in grayscale+alpha mode. */
MA_Surface *
MA_SurfaceGrayscale(Uint8 w, Uint8 h, Uint8 BitsPerPixel, Uint8 flags)
{
	MA_Surface *S;
	
	S = Malloc(sizeof(MA_Surface));
	MA_SurfaceInit(S, NULL, w,h, flags);
	MA_PixelFormatGrayscale(&S->format, BitsPerPixel);
	MA_SurfaceAllocPixels(S);
	return (S);
}

/* Create a new surface in packed pixel mode (parametric form with alpha) */
MA_Surface *
MA_SurfaceRGBA(Uint8 w, Uint8 h, Uint8 BitsPerPixel, Uint8 flags,
    MA_Pixel Rmask, MA_Pixel Gmask, MA_Pixel Bmask, MA_Pixel Amask)
{
	MA_Surface *S;
	
	S = Malloc(sizeof(MA_Surface));
	MA_SurfaceInit(S, NULL, w,h, flags);
	MA_PixelFormatRGBA(&S->format, BitsPerPixel, Rmask, Gmask, Bmask, Amask);
	MA_SurfaceAllocPixels(S);
	return (S);
}

/* Set pixel data to an external address (or NULL = revert to auto-allocated) */
void
MA_SurfaceSetAddress(MA_Surface *S, Uint8 *pixels)
{
	if (S->pixels != NULL && !(S->flags & MA_SURFACE_EXT_PIXELS)) {
		free(S->pixels);
	}
	if (pixels != NULL) {
		S->pixels = pixels;
		S->flags |= MA_SURFACE_EXT_PIXELS;
	} else {
		S->pixels = (S->h*S->pitch > 0) ? Malloc(S->h*S->pitch) : NULL;
		S->flags &= ~(MA_SURFACE_EXT_PIXELS);
	}
}

/* Return a newly-allocated duplicate of a surface (in the same format). */
MA_Surface *
MA_SurfaceDup(const MA_Surface *a)
{
	MA_Surface *b;

	b = MA_SurfaceNew(&a->format,
	    a->w,
	    a->h,
	    (a->flags & AG_SAVED_SURFACE_FLAGS));

	memcpy(b->pixels, a->pixels, a->h * a->pitch);
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
MA_SurfaceCopy(MA_Surface *D, const MA_Surface *S)
{
	Uint8 w = MIN(S->w, D->w);
	Uint8 h = MIN(S->h, D->h);
	Uint8 x,y;

	if (MA_PixelFormatCompare(&S->format, &D->format) == 0) {  /* Block */
		Uint8 *pSrc = S->pixels, *pDst = D->pixels;
		Uint Spadding = S->padding;
		Uint Dpadding = D->padding;
		Uint pitch = (D->format.BitsPerPixel < 8) ?
		     w / D->format.PixelsPerByte :
		     w * D->format.BytesPerPixel;

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
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				MA_Pixel px;
				MA_Color c;
				
				px = MA_SurfaceGet(S, x,y);
				AG_GetColor(&c, px, &S->format);
				MA_SurfacePut(D, x,y,
				    AG_MapPixel(&D->format, &c));
			}
		}
	}
}

/*
 * Blit loop with both semi-transparent per-surface alpha and colorkey.
 * No opaque pixels are possible.
 */
static void
MA_SurfaceBlit_AlCo(const MA_Surface *_Nonnull S, MA_Surface *_Nonnull D,
    const AG_Rect *_Nonnull dr, Uint8 xDst, Uint8 yDst)
{
	MA_Pixel srcColorkey = S->colorkey;
	Uint8 x,y;
	Uint8 w = dr->w;
	Uint8 h = dr->h;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			MA_Pixel px;
			MA_Color c;

			px = MA_SurfaceGet(S, x,y);
			if (px == srcColorkey) {
				continue;
			}
			AG_GetColor(&c, px, &S->format);
			c.a = MIN(c.a, S->alpha);
			if (c.a == AG_TRANSPARENT) {
				continue;
			}
			MA_SurfaceBlend(D, xDst+x, yDst+y, &c, AG_ALPHA_OVERLAY);
		}
	}
}

/*
 * Blit loop with fully opaque per-surface alpha and colorkey.
 * Possibly some opaque pixels.
 */
static void
MA_SurfaceBlit_Co(const MA_Surface *_Nonnull S, MA_Surface *_Nonnull D,
    const AG_Rect *_Nonnull dr, Uint8 xDst, Uint8 yDst)
{
	MA_Pixel srcColorkey = S->colorkey;
	Uint8 w = dr->w;
	Uint8 h = dr->h;
	Uint8 x, y;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			MA_Pixel px;
			MA_Color c;

			px = MA_SurfaceGet(S, x,y);
			if (px == srcColorkey) {
				continue;
			}
			AG_GetColor(&c, px, &S->format);
			if (c.a == AG_TRANSPARENT) {
				continue;
			}
			if (c.a == MA_OPAQUE) {
				MA_SurfacePut(D, xDst+x, yDst+y,
				    AG_MapPixel(&D->format, &c));
			} else {
				MA_SurfaceBlend(D, xDst+x, yDst+y, &c,
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
 * Perform alpha blending if S has MA_SURFACE_ALPHA flag. Honor component
 * alpha if S defines an alpha channel. Also honor per-surface alpha.
 *
 * Perform colorkey tests if S has MA_SURFACE_COLORKEY flag.
 *
 * If srcRect is NULL, the entire surface is copied.
 */
void
MA_SurfaceBlit(const MA_Surface *S, const AG_Rect *srcRect, MA_Surface *D,
    Uint8 xDst, Uint8 yDst)
{
	AG_Rect sr, dr;
	Uint8 x, y;

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
	dr.x = xDst;
	dr.y = yDst;
	dr.w = sr.w;
	dr.h = sr.h;
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

	if (S->alpha < MA_OPAQUE) {
		if (S->flags & MA_SURFACE_COLORKEY) {
			MA_SurfaceBlit_AlCo(S, D, &dr, xDst,yDst);
			return;
		}
		for (y = 0; y < dr.h; y++) {
			for (x = 0; x < dr.w; x++) {
				MA_Pixel px;
				MA_Color c;
			
				px = MA_SurfaceGet(S, x,y);
				MA_GetColor(&c, px, &S->format);

				c.a = MIN(c.a, S->alpha);
				if (c.a == MA_TRANSPARENT) {
					continue;
				}
				MA_SurfaceBlend(D, xDst+x, yDst+y, &c);
			}
		}
		return;
	}
	if (S->flags & MA_SURFACE_COLORKEY) {
		MA_SurfaceBlit_Co(S, D, &dr, xDst,yDst);
		return;
	}
	if (S->format.Amask != 0) {
		for (y = 0; y < dr.h; y++) {
			for (x = 0; x < dr.w; x++) {
				MA_Pixel px;
				MA_Color c;
			
				px = MA_SurfaceGet(S, x,y);
				MA_GetColor(&c, px, &S->format);

				if (c.a == MA_TRANSPARENT) {
					continue;
				}
				if (c.a < MA_OPAQUE) {
					MA_SurfaceBlend(D, xDst+x, yDst+y, &c);
				} else {
					MA_SurfacePut(D, xDst+x, yDst+y,
					    MA_MapPixel(&D->format, &c));
				}
			}
		}
	} else {
		for (y = 0; y < dr.h; y++) {
			for (x = 0; x < dr.w; x++) {
				MA_Pixel px;
				MA_Color c;

				px = MA_SurfaceGet(S, x,y);
				MA_GetColor(&c, px, &S->format);
				MA_SurfacePut(D, xDst+x, yDst+y,
				    MA_MapPixel(&D->format, &c));
			}
		}
	}
}

/* Free the specified surface. */
void
MA_SurfaceFree(MA_Surface *S)
{
	Uint i;

#ifdef AG_DEBUG
	if (S->flags & MA_SURFACE_MAPPED)
		AG_FatalError("MA_SurfaceFree: in use");
#endif
	if ((S->flags & MA_SURFACE_EXT_PIXELS) == 0) {
		Free(S->pixels);
	}
	if ((S->flags & MA_SURFACE_STATIC) == 0)
		free(S);
}

/* Fill a rectangle with pixels of a given color. */
void
MA_FillRect(MA_Surface *S, const MA_Rect *rDst, const MA_Color *c)
{
	MA_Rect r;
	MA_Pixel px;
	Uint8 x,y;

	if (rDst != NULL) {
		r = *rDst;
		if (r.x + r.w >= S->w) { r.w = S->w - r.x; }
		if (r.y + r.h >= S->h) { r.h = S->h - r.y; }
	} else {
		r = S->clipRect;
	}
	px = MA_MapPixel(&S->format, c);

	/* XXX TODO optimized cases */
	for (y = r.y; y < r.h; y++)
		for (x = r.x; x < r.w; x++)
			MA_SurfacePut(S, x, y, px);
}

/* Return the pixel at x,y in a 1- to 8-bpp surface S. */
Uint8
MA_SurfaceGet8(const MA_Surface *S, Uint8 x, Uint8 y)
{
	const Uint8 *p = S->pixels + y*S->pitch +
	                             x/S->format.PixelsPerByte;
#ifdef AG_DEBUG
	if (x >= S->w || y >= S->h)
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
MA_SurfacePut8(MA_Surface *_Nonnull S, Uint8 x, Uint8 y, Uint8 px)
{
	Uint8 *p = S->pixels + y*S->pitch +
	                       x/S->format.PixelsPerByte;
#ifdef AG_DEBUG
	if (x >= S->w || y >= S->h)
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

/* Map 4-bit RGBA components to a 16-bit pixel. */
Uint16
MA_MapPixel16_RGBA4(const MA_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	switch (pf->mode) {
	case MA_SURFACE_PACKED:
	default:
		return (r >> pf->Rloss) << pf->Rshift |
		       (g >> pf->Gloss) << pf->Gshift |
		       (b >> pf->Bloss) << pf->Bshift |
		      ((a >> pf->Aloss) << pf->Ashift & (Uint16)pf->Amask);
	case MA_SURFACE_INDEXED:
	case MA_SURFACE_GRAYSCALE:
		/* TODO */
		return (0);
	}
}

#define EXTRACT_COMPONENT(rv, mask, shift, loss, bits)			\
	tmp = (px & mask) >> shift;					\
	(rv) = (tmp << loss) + (tmp >> (bits - (loss << 1)))

/* Extract 4-bit RGBA components from a 16-bit pixel. */
void
AG_GetColor16_RGBA4(Uint16 px, const MA_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	MA_Color *c;
	Uint16 tmp;

	switch (pf->mode) {
	case MA_SURFACE_PACKED:
		EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 4);
		EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 4);
		EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 4);
		EXTRACT_COMPONENT(*a, pf->Amask, pf->Ashift, pf->Aloss, 4);
		break;
	case MA_SURFACE_INDEXED:
	case MA_SURFACE_GRAYSCALE:
	default:
		*r = 0;			/* TODO */
		*g = 0;
		*b = 0;
		*a = 0;
		break;
	}
}

/*
 * Extract an MA_Color from a 16-bit pixel.
 */
void
AG_GetColor16(MA_Color *c, Uint16 px, const MA_PixelFormat *pf)
{
	Uint8 R,G,B,A;
	Uint16 tmp;
	
	switch (pf->mode) {
	case MA_SURFACE_PACKED:
		EXTRACT_COMPONENT(R, pf->Rmask, pf->Rshift, pf->Rloss, 4);
		EXTRACT_COMPONENT(G, pf->Gmask, pf->Gshift, pf->Gloss, 4);
		EXTRACT_COMPONENT(B, pf->Bmask, pf->Bshift, pf->Bloss, 4);
		c->r = R;
		c->g = G;
		c->b = B;
		if (pf->Amask != 0) {
			EXTRACT_COMPONENT(A, pf->Amask, pf->Ashift, pf->Aloss, 4);
			c->a = A;
		} else {
			c->a = MA_OPAQUE;
		}
		break;
	case MA_SURFACE_INDEXED:
	case MA_SURFACE_GRAYSCALE:
		/* TODO */
		c->r = 0;
		c->g = 0;
		c->b = 0;
		c->a = 0;
		break;
	}
}

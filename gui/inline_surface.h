/*
 * Copyright (c) 2009-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/* Test if a given mode and bits/pixel combination is supported. */
#ifdef AG_INLINE_HEADER
static __inline__ int _Const_Attribute
AG_PixelFormatIsSupported(AG_SurfaceMode mode, int BitsPerPixel)
#else
int
ag_pixel_format_is_supported(AG_SurfaceMode mode, int BitsPerPixel)
#endif
{
	switch (mode) {
	case AG_SURFACE_PACKED:
		switch (BitsPerPixel) {
		case 8:
		case 16:
		case 24:
		case 32:
		case 48:
		case 64:
			return (1);
		}
		break;
	case AG_SURFACE_INDEXED:
		switch (BitsPerPixel) {
		case 1:
		case 2:
		case 4:
		case 8:
			return (1);
		}
		break;
	case AG_SURFACE_GRAYSCALE:
		switch (BitsPerPixel) {
		case 32:
		case 64:
			return (1);
		}
		break;
	}
	return (0);
}

/*
 * Test whether two pixel formats are identical.
 * In indexed mode, include both palettes in the comparison.
 */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_PixelFormatCompare(const AG_PixelFormat *_Nonnull a,
                      const AG_PixelFormat *_Nonnull b)
#else
int
ag_pixel_format_compare(const AG_PixelFormat *a, const AG_PixelFormat *b)
#endif
{
	if (a->mode != b->mode ||
	    a->BitsPerPixel != b->BitsPerPixel) {
		return (1);
	}
	switch (a->mode) {
	case AG_SURFACE_PACKED:
	default:
		return !(a->Rmask == b->Rmask &&
			 a->Gmask == b->Gmask &&
			 a->Bmask == b->Bmask &&
			 a->Amask == b->Amask);
	case AG_SURFACE_INDEXED:
#ifdef AG_DEBUG
		if (a->palette->nColors != b->palette->nColors)  {
			AG_SetError("nColors %u != %u",
			    a->palette->nColors,
			    b->palette->nColors);
			AG_FatalError(NULL);
		}
#endif
		return memcmp(a->palette->colors, b->palette->colors,
		              a->palette->nColors * sizeof(AG_Color));
	case AG_SURFACE_GRAYSCALE:
		return !(a->graymode == b->graymode);
	}
}

/* Test if x,y lies inside surface's clipping rectangle */
#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_SurfaceClipped(const AG_Surface *_Nonnull S, int x, int y)
#else
int
ag_surface_clipped(const AG_Surface *S, int x, int y)
#endif
{
	return (x <  S->clipRect.x ||
	        x >= S->clipRect.x + S->clipRect.w ||
	        y <  S->clipRect.y ||
	        y >= S->clipRect.y + S->clipRect.h);
}

/*
 * Map an AG_Color to a 32-bit wide pixel value.
 * If the pixel format is >32-bit then return a 32-bit approximation.
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint32 _Pure_Attribute
AG_MapPixel32(const AG_PixelFormat *_Nonnull pf, const AG_Color *_Nonnull c)
#else
Uint32
ag_map_pixel32(const AG_PixelFormat *pf, const AG_Color *c)
#endif
{
#if AG_MODEL == AG_LARGE
	return AG_MapPixel32_RGBA16(pf, c->r, c->g, c->b, c->a);
#else
	return AG_MapPixel32_RGBA8(pf, c->r, c->g, c->b, c->a);
#endif
}

#if AG_MODEL == AG_LARGE
# define AG_EXTRACT_COMPONENT(rv, mask, shift, loss, bits)		\
	tmp = (px & mask) >> shift;					\
	(rv) = (tmp << loss) + (tmp >> (bits - (loss << 1)));
/*
 * Map 16-bit RGB components to a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ Uint64 _Pure_Attribute
AG_MapPixel64_RGB16(const AG_PixelFormat *_Nonnull pf,
   Uint16 r, Uint16 g, Uint16 b)
# else
Uint64
ag_map_pixel64_rgb16(const AG_PixelFormat *pf, Uint16 r, Uint16 g, Uint16 b)
# endif
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return ((Uint64)r >> pf->Rloss) << pf->Rshift |
		       ((Uint64)g >> pf->Gloss) << pf->Gshift |
		       ((Uint64)b >> pf->Bloss) << pf->Bshift |
		      ((AG_OPAQUE >> pf->Aloss) << pf->Ashift & pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf, r,g,b, AG_OPAQUE);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf, r,g,b, AG_OPAQUE);
	}
}

/*
 * Map 16-bit RGBA components to a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ Uint64 _Pure_Attribute
AG_MapPixel64_RGBA16(const AG_PixelFormat *_Nonnull pf,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a)
# else
Uint64
ag_map_pixel64_rgba16(const AG_PixelFormat *pf,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a)
# endif
{
	switch (pf->mode) {
	case AG_SURFACE_PACKED:
	default:
		return (r >> pf->Rloss) << pf->Rshift |
		       (g >> pf->Gloss) << pf->Gshift |
		       (b >> pf->Bloss) << pf->Bshift |
		      ((a >> pf->Aloss) << pf->Ashift & pf->Amask);
	case AG_SURFACE_INDEXED:
		return AG_MapPixelIndexed(pf, r,g,b,a);
	case AG_SURFACE_GRAYSCALE:
		return AG_MapPixelGrayscale(pf, r,g,b,a);
	}
}

/*
 * Map an AG_Color to a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ Uint64 _Pure_Attribute
AG_MapPixel64(const AG_PixelFormat *_Nonnull pf, const AG_Color *_Nonnull c)
# else
Uint64
ag_map_pixel64(const AG_PixelFormat *pf, const AG_Color *c)
# endif
{
	return AG_MapPixel64_RGBA16(pf, c->r, c->g, c->b, c->a);
}

/*
 * Extract 16-bit RGB components from a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_GetColor64_RGB16(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b)
# else
void
ag_get_color64_rgb16(Uint64 px, const AG_PixelFormat *pf,
    Uint16 *r, Uint16 *g, Uint16 *b)
# endif
{
	Uint64 tmp;
	AG_Color *c;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		AG_EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 16);
		AG_EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 16);
		AG_EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 16);
		break;
	case AG_SURFACE_INDEXED:
		c = &pf->palette->colors[px % pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		break;
	case AG_SURFACE_GRAYSCALE: {
		Uint16 dummy;
		AG_GetColor64_Gray16(px, pf->graymode, r,g,b, &dummy);
		break;
	}
	/* pf->mode */
	}
}

/*
 * Extract 16-bit RGBA components from a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_GetColor64_RGBA16(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b,
    Uint16 *_Nonnull a)
# else
void
ag_get_color64_rgba16(Uint64 px, const AG_PixelFormat *pf,
    Uint16 *r, Uint16 *g, Uint16 *b, Uint16 *a)
# endif
{
	Uint64 tmp;
	AG_Color *c;

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
		AG_EXTRACT_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss, 16);
		AG_EXTRACT_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss, 16);
		AG_EXTRACT_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss, 16);
		AG_EXTRACT_COMPONENT(*a, pf->Amask, pf->Ashift, pf->Aloss, 16);
		break;
	case AG_SURFACE_INDEXED:
		c = &pf->palette->colors[px % pf->palette->nColors];
		*r = c->r;
		*g = c->g;
		*b = c->b;
		*a = c->a;
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_GetColor64_Gray16(px, pf->graymode, r,g,b,a);
		break;
	}
}

/*
 * Extract compressed 8-bit RGB components from a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_GetColor64_RGB8(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b)
# else
void
ag_get_color64_rgb8(Uint64 px, const AG_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b)
# endif
{
	Uint16 R,G,B;

	AG_GetColor64_RGB16(px, pf, &R,&G,&B);
	*r = AG_16to8(R);
	*g = AG_16to8(G);
	*b = AG_16to8(B);
}

/*
 * Extract compressed 8-bit RGBA components from a 64-bit (1- to 64-bpp) pixel.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_GetColor64_RGBA8(Uint64 px, const AG_PixelFormat *_Nonnull pf,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b,
    Uint8 *_Nonnull a)
# else
void
ag_get_color64_rgba8(Uint64 px, const AG_PixelFormat *pf,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
# endif
{
	Uint16 R,G,B,A;

	AG_GetColor64_RGBA16(px, pf, &R,&G,&B,&A);
	*r = AG_16to8(R);
	*g = AG_16to8(G);
	*b = AG_16to8(B);
	*a = AG_16to8(A);
}

# undef AG_EXTRACT_COMPONENT
#endif /* AG_LARGE */

/*
 * Return the (1- to 64-bit) pixel at coordinates x,y in surface S.
 * If S is > 32-bit, return a compressed 32-bit approximation.
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint32 _Pure_Attribute
AG_SurfaceGet32(const AG_Surface *_Nonnull S, int x, int y)
#else
Uint32
ag_surface_get32(const AG_Surface *S, int x, int y)
#endif
{
#ifdef AG_DEBUG
	if (x < 0 || y < 0 || x >= S->w || y >= S->h)
		AG_FatalErrorF("Illegal AG_SurfaceGet32() access (%d,%d in %ux%u)", x,y, S->w, S->h);
#endif
	if (S->format.BitsPerPixel < 8) {
		return (Uint32)AG_SurfaceGet8(S, x,y);
	} else {
		return AG_SurfaceGet32_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel);
	}
}

/*
 * Return the (8- to 64-bit) pixel at address p in surface S.
 * If S is > 32-bit, return a compressed 32-bit approximation.
 */
#ifdef AG_INLINE_HEADER
static __inline__ Uint32 _Pure_Attribute
AG_SurfaceGet32_At(const AG_Surface *_Nonnull S, const Uint8 *_Nonnull p)
#else
Uint32
ag_surface_get32_at(const AG_Surface *S, const Uint8 *p)
#endif
{
	switch (S->format.BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 64:
	case 48: {
		AG_Color c;
		AG_GetColor64(&c, *(Uint64 *)p, &S->format);
		return AG_MapPixel32(&S->format, &c);
	}
#endif
	case 32:
		return (*(Uint32 *)p);
	case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		return ((p[0] << 16) +
		        (p[1] << 8) +
		         p[2]);
#else
		return  (p[0] +
		        (p[1] << 8) +
		        (p[2] << 16));
#endif
	case 16:
		return (*(Uint16 *)p);
	}
	return (*p);
}
	
#if AG_MODEL == AG_LARGE
/*
 * Return the (1- to 64-bit) pixel at coordinates x,y in surface S.
 */
# ifdef AG_INLINE_HEADER
static __inline__ Uint64 _Pure_Attribute
AG_SurfaceGet64(const AG_Surface *_Nonnull S, int x, int y)
# else
Uint64
ag_surface_get64(const AG_Surface *S, int x, int y)
# endif
{
# ifdef AG_DEBUG
	if (x < 0 || y < 0 || x >= S->w || y >= S->h)
		AG_FatalErrorF("Illegal AG_SurfaceGet64() access (%d,%d in %ux%u)", x,y, S->w, S->h);
# endif
	if (S->format.BitsPerPixel < 8) {
		return (Uint64)AG_SurfaceGet8(S, x,y);
	} else {
		return AG_SurfaceGet64_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel);
	}
}

/*
 * Return the (8- to 64-bit) pixel at address p in surface S.
 */
# ifdef AG_INLINE_HEADER
static __inline__ Uint64 _Pure_Attribute
AG_SurfaceGet64_At(const AG_Surface *_Nonnull S, const Uint8 *_Nonnull p)
# else
Uint64
ag_surface_get64_at(const AG_Surface *S, const Uint8 *p)
# endif
{
	switch (S->format.BitsPerPixel) {
	case 64:
		return (*(Uint64 *)p);
	case 48:
# if AG_BYTEORDER == AG_BIG_ENDIAN
		return (((Uint64)p[0] << 40) +
		        ((Uint64)p[1] << 32) +
		        ((Uint64)p[2] << 24) +
		        ((Uint64)p[3] << 16) +
		        ((Uint64)p[4] << 8) +
		         (Uint64)p[5]);
# else
		return  ((Uint64)p[0] +
		        ((Uint64)p[1] << 8) +
		        ((Uint64)p[2] << 16) +
			((Uint64)p[3] << 24) +
			((Uint64)p[4] << 32) +
			((Uint64)p[5] << 40));
# endif
	case 32:
		return (*(Uint32 *)p);
	case 24:
# if AG_BYTEORDER == AG_BIG_ENDIAN
		return ((p[0] << 16) +
		        (p[1] << 8) +
		         p[2]);
# else
		return  (p[0] +
		        (p[1] << 8) +
		        (p[2] << 16));
# endif
	case 16:
		return (*(Uint16 *)p);
	}
	return (*p);
}

#endif /* AG_LARGE */

/*
 * Write to the (1- to 64-bit) pixel at coordinates x,y in surface S.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfacePut32(AG_Surface *_Nonnull S, int x, int y, Uint32 px)
#else
void
ag_surface_put32(AG_Surface *S, int x, int y, Uint32 px)
#endif
{
	if (S->format.BitsPerPixel < 8) {
		AG_SurfacePut8(S, x,y, (Uint8)px);
	} else {
		AG_SurfacePut32_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel, px);
	}
}

/*
 * Write to the (8- to 32-bit) pixel at address p in surface S.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfacePut32_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p, Uint32 px)
#else
void
ag_surface_put32_at(AG_Surface *S, Uint8 *p, Uint32 px)
#endif
{
	switch (S->format.BitsPerPixel) {
#if AG_MODEL == AG_LARGE
	case 64:                                             /* Decompressed */
	case 48: {
		AG_Color c;
		AG_GetColor32(&c, px, &S->format);
		*(Uint64 *)p = AG_MapPixel64(&S->format, &c);
		break;
	}
#endif
	case 32:
		*(Uint32 *)p = px;
		break;
	case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		p[0] = (px >> 16) & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px)       & 0xff;
#else
		p[0] = (px)       & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px >> 16) & 0xff;
#endif
		break;
	case 16:
		*(Uint16 *)p = (Uint16)px;
		break;
	default:
		*p = (Uint8)px;
		break;
	}
}

#if AG_MODEL == AG_LARGE
/*
 * Write the pixel at x,y in a (1- to 64-bit) surface S.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfacePut64(AG_Surface *_Nonnull S, int x, int y, Uint64 px)
# else
void
ag_surface_put64(AG_Surface *S, int x, int y, Uint64 px)
# endif
{
	if (S->format.BitsPerPixel < 8) {
		AG_SurfacePut8(S, x,y, (Uint8)px);
	} else {
		AG_SurfacePut64_At(S, S->pixels +
		    y*S->pitch +
		    x*S->format.BytesPerPixel, px);
	}
}

/*
 * Write the pixel at address p in an (8- to 64-bpp) surface S.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfacePut64_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p, Uint64 px)
# else
void
ag_surface_put64_at(AG_Surface *S, Uint8 *p, Uint64 px)
# endif
{
	switch (S->format.BitsPerPixel) {
	case 64:
	case 48:
		*(Uint64 *)p = px;
		break;
	case 32:
		*(Uint32 *)p = px;
		break;
	case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		p[0] = (px >> 16) & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px)       & 0xff;
#else
		p[0] = (px)       & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px >> 16) & 0xff;
#endif
		break;
	case 16:
		*(Uint16 *)p = (Uint16)px;
		break;
	default:
		*p = (Uint8)px;
		break;
	}
}

/*
 * Blend 16-bit RGBA components with the pixel at x,y and overwrite it with
 * the result. No clipping. Target pixel alpha is set according to fn.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfaceBlendRGB16(AG_Surface *_Nonnull S, int x, int y,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a, AG_AlphaFn fn)
# else
void
ag_surface_blend_rgb16(AG_Surface *S, int x, int y,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a, AG_AlphaFn fn)
# endif
{
	AG_Color c = { r,g,b,a };

	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    &c, fn);
}

/*
 * Blend 16-bit RGBA components with the pixel at address p and overwrite it
 * with the result.  No clipping is done. Set target alpha according to fn.
 */
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfaceBlendRGB16_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a, AG_AlphaFn fn)
# else
void
ag_surface_blend_rgb16_at(AG_Surface *S, Uint8 *p,
    Uint16 r, Uint16 g, Uint16 b, Uint16 a, AG_AlphaFn fn)
# endif
{
	AG_Color c = { r,g,b,a };

	AG_SurfaceBlend_At(S,p, &c, fn);
}
#endif /* AG_LARGE */

/*
 * Blend a color c with the pixel at x,y and overwrite it with the result.
 * No clipping is done. Sets the pixel's alpha component according to fn.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfaceBlend(AG_Surface *_Nonnull S, int x, int y, const AG_Color *_Nonnull c,
    AG_AlphaFn fn)
#else
void
ag_surface_blend(AG_Surface *S, int x, int y, const AG_Color *c, AG_AlphaFn fn)
#endif
{
	AG_SurfaceBlend_At(S, S->pixels +
	    y*S->pitch +
	    x*S->format.BytesPerPixel,
	    c, fn);
}

/*
 * Blend a color c with the pixel at address p and overwrite it with the result.
 * No clipping is done. Sets the pixel's alpha component according to fn.
 */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfaceBlend_At(AG_Surface *_Nonnull S, Uint8 *_Nonnull p,
    const AG_Color *_Nonnull c, AG_AlphaFn fn)
#else
void
ag_surface_blend_at(AG_Surface *S, Uint8 *p, const AG_Color *_Nonnull c,
    AG_AlphaFn fn)
#endif
{
	AG_Pixel px = AG_SurfaceGet_At(S,p);
	AG_Color pc;
	AG_Component ca;

	if ((S->flags & AG_SURFACE_COLORKEY) &&		/* No blending needed */
	    (px == S->colorkey)) {
		AG_SurfacePut_At(S, p, AG_MapPixel(&S->format, c));
		return;
	}
	pc.a = AG_COLOR_LAST;				/* compiler happy */
	AG_GetColor(&pc, px, &S->format);
	ca = c->a;
	switch (fn) {
	case AG_ALPHA_OVERLAY:
		pc.a = ((pc.a+ca) > AG_COLOR_LAST) ? AG_COLOR_LAST : (pc.a+ca);
		break;
	case AG_ALPHA_DST:        /* pc.a = pc.a */			break;
	case AG_ALPHA_SRC:           pc.a = ca;				break;
	case AG_ALPHA_ZERO:          pc.a = 0;				break;
	case AG_ALPHA_ONE_MINUS_DST: pc.a = AG_COLOR_LAST - pc.a;	break;
	case AG_ALPHA_ONE_MINUS_SRC: pc.a = AG_COLOR_LAST - ca;		break;
	case AG_ALPHA_ONE: default:  pc.a = AG_COLOR_LAST;		break;
	}
	AG_SurfacePut_At(S, p,
	    AG_MapPixel_RGBA(&S->format,
	        pc.r + (((c->r - pc.r)*ca) >> AG_COMPONENT_BITS),
	        pc.g + (((c->g - pc.g)*ca) >> AG_COMPONENT_BITS),
	        pc.b + (((c->b - pc.b)*ca) >> AG_COMPONENT_BITS),
	        pc.a));
}

/* Release all resources possibly allocated by an AG_PixelFormat. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_PixelFormatFree(AG_PixelFormat *_Nonnull pf)
#else
void
ag_pixel_format_free(AG_PixelFormat *pf)
#endif
{
	if (pf->mode == AG_SURFACE_INDEXED) {
		AG_Free(pf->palette->colors);
		AG_Free(pf->palette);
	}
}

/* Set the source alpha flag and per-surface alpha. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfaceSetAlpha(AG_Surface *_Nonnull S, Uint flags, AG_Component alpha)
#else
void
ag_surface_set_alpha(AG_Surface *S, Uint flags, AG_Component alpha)
#endif
{
	if (flags & AG_SURFACE_ALPHA) {
		S->flags |= AG_SURFACE_ALPHA;
	} else {
		S->flags &= ~(AG_SURFACE_ALPHA);
	}
	S->alpha = alpha;
}

/* Set the source colorkey flag and per-surface colorkey. */
#ifdef AG_INLINE_HEADER
static __inline__ void
AG_SurfaceSetColorKey(AG_Surface *_Nonnull S, Uint flags, AG_Pixel colorkey)
#else
void
ag_surface_set_colorkey(AG_Surface *S, Uint flags, AG_Pixel colorkey)
#endif
{
	if (flags & AG_SURFACE_COLORKEY) {
		S->flags |= AG_SURFACE_COLORKEY;
	} else {
		S->flags &= ~(AG_SURFACE_COLORKEY);
	}
	S->colorkey = colorkey;
}

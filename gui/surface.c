/*
 * Copyright (c) 2009-2013 Hypertriton, Inc. <http://hypertriton.com/>
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

const char *agBlendFuncNames[] = {
	"dst+src",
	"src",
	"dst",
	"1-dst",
	"1-src",
	NULL
};

AG_PixelFormat *agSurfaceFmt = NULL;  /* Recommended format for GUI surfaces */

#define COMPUTE_SHIFTLOSS(mask, shift, loss) \
	shift = 0; \
	loss = 8; \
	if (mask) { \
		for (m = mask ; !(m & 0x01); m >>= 1) { \
			shift++; \
		}  \
		for (; (m & 0x01); m >>= 1) { \
			loss--; \
		} \
	}

/* Specify a packed-pixel format from three 32-bit bitmasks. */
AG_PixelFormat *
AG_PixelFormatRGB(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	AG_PixelFormat *pf;
	Uint32 m;
	Uint32 Amask = 0;

	if ((pf = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (NULL);
	}
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->colorkey = 0;
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->palette = NULL;
	COMPUTE_SHIFTLOSS(Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTLOSS(Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTLOSS(Bmask, pf->Bshift, pf->Bloss);
	COMPUTE_SHIFTLOSS(Amask, pf->Ashift, pf->Aloss);
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = 0;
	return (pf);
}

/* Specify a packed-pixel format from four 32-bit bitmasks. */
AG_PixelFormat *
AG_PixelFormatRGBA(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask,
    Uint32 Amask)
{
	AG_PixelFormat *pf;
	Uint32 m;

	if ((pf = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (NULL);
	}
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->colorkey = 0;
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->palette = NULL;
	COMPUTE_SHIFTLOSS(Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTLOSS(Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTLOSS(Bmask, pf->Bshift, pf->Bloss);
	COMPUTE_SHIFTLOSS(Amask, pf->Ashift, pf->Aloss);
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = Amask;
	return (pf);
}

/*
 * Specify an indexed pixel format. If bpp=2, the palette is initialized to
 * [0 = white] and [1 = black], otherwise the palette is initialized to all
 * black.
 */
AG_PixelFormat *
AG_PixelFormatIndexed(int bpp)
{
	AG_PixelFormat *pf;
	AG_Palette *pal;

	if ((pf = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (NULL);
	}
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->colorkey = 0;
	pf->alpha = AG_ALPHA_OPAQUE;

	if ((pal = pf->palette = TryMalloc(sizeof(AG_Palette))) == NULL) {
		Free(pf);
		return (NULL);
	}
	pal->nColors = 1<<bpp;
	if ((pal->colors = TryMalloc(pal->nColors*sizeof(AG_Color))) == NULL) {
		Free(pf->palette);
		Free(pf);
		return (NULL);
	}
	
	if (bpp == 2) {
		pal->colors[0].r = 255;
		pal->colors[0].g = 255;
		pal->colors[0].b = 255;
		pal->colors[1].r = 0;
		pal->colors[1].g = 0;
		pal->colors[1].b = 0;
	} else {
		memset(pal->colors, 0, pal->nColors*sizeof(AG_Color));
	}

	pf->Rmask = pf->Gmask = pf->Bmask = pf->Amask = 0;
	pf->Rloss = pf->Gloss = pf->Bloss = pf->Aloss = 8;
	pf->Rshift = pf->Gshift = pf->Bshift = pf->Ashift = 0;
	return (pf);
}

/* Return a newly-allocated duplicate an AG_PixelFormat structure. */
AG_PixelFormat *
AG_PixelFormatDup(const AG_PixelFormat *pf)
{
	AG_PixelFormat *pfd;

	if ((pfd = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (NULL);
	}
	if (pf->palette != NULL) {
		if ((pfd->palette = TryMalloc(sizeof(AG_Palette))) == NULL) {
			goto fail;
		}
		if ((pfd->palette->colors =
		    TryMalloc(pf->palette->nColors*sizeof(AG_Color))) == NULL) {
			Free(pfd->palette);
			goto fail;
		}
		pfd->palette->nColors = pf->palette->nColors;
		memcpy(pfd->palette->colors, pf->palette->colors,
		    pf->palette->nColors*sizeof(AG_Color));
	} else {
		pfd->palette = NULL;
	}
	pfd->BitsPerPixel = pf->BitsPerPixel;
	pfd->BytesPerPixel = pf->BytesPerPixel;
	pfd->colorkey = pf->colorkey;
	pfd->alpha = pf->alpha;
	pfd->Rloss = pf->Rloss;
	pfd->Gloss = pf->Gloss;
	pfd->Bloss = pf->Bloss;
	pfd->Aloss = pf->Aloss;
	pfd->Rshift = pf->Rshift;
	pfd->Gshift = pf->Gshift;
	pfd->Bshift = pf->Bshift;
	pfd->Ashift = pf->Ashift;
	pfd->Rmask = pf->Rmask;
	pfd->Gmask = pf->Gmask;
	pfd->Bmask = pf->Bmask;
	pfd->Amask = pf->Amask;
	return (pfd);
fail:
	Free(pfd);
	return (NULL);
}

/* Release an AG_PixelFormat structure. */
void
AG_PixelFormatFree(AG_PixelFormat *pf)
{
	if (pf->palette != NULL) {
		Free(pf->palette->colors);
		free(pf->palette);
	}
	free(pf);
}

/* Compare two palettes. */
int
AG_PixelFormatComparePalettes(const AG_Palette *pal1, const AG_Palette *pal2)
{
	if (pal1->nColors != pal2->nColors) {
		return (1);
	}
	return memcmp(pal1->colors, pal2->colors,
	              pal1->nColors*sizeof(AG_Color));
}

#undef COMPUTE_SHIFTLOSS

/* Create a new surface of the specified pixel format. */
AG_Surface *
AG_SurfaceNew(enum ag_surface_type type, Uint w, Uint h,
    const AG_PixelFormat *pf, Uint flags)
{
	AG_Surface *s;
	Uint pitch;

	if ((s = TryMalloc(sizeof(AG_Surface))) == NULL) {
		return (NULL);
	}
	if ((s->format = AG_PixelFormatDup(pf)) == NULL) {
		Free(s);
		return (NULL);
	}
	s->type = type;
	s->flags = flags;
	s->w = w;
	s->h = h;

	pitch = w*pf->BytesPerPixel;		/* 4-byte aligned */
	switch (pf->BitsPerPixel) {
	case 1:
		pitch = (pitch + 7)/8;
		break;
	case 4:
		pitch = (pitch + 1)/2;
		break;
	}
	s->pitch = (pitch + 3) & ~3;
	s->padding = s->pitch - w*pf->BytesPerPixel;
	s->clipRect = AG_RECT(0,0,w,h);

	if (h*s->pitch > 0) {
		if ((s->pixels = TryMalloc(h*s->pitch)) == NULL)
			goto fail;
	} else {
		s->pixels = NULL;
	}
	return (s);
fail:
	AG_PixelFormatFree(s->format);
	Free(s);
	return (NULL);
}

/* Create an empty surface. */
AG_Surface *
AG_SurfaceEmpty(void)
{
	return AG_SurfaceNew(AG_SURFACE_PACKED, 0,0, agSurfaceFmt, 0);
}

/* Create a new color-index surface of given dimensions and depth. */
AG_Surface *
AG_SurfaceIndexed(Uint w, Uint h, int bpp, Uint flags)
{
	AG_PixelFormat *pf;
	AG_Surface *s;

	if ((pf = AG_PixelFormatIndexed(bpp)) == NULL) {
		return (NULL);
	}
	s = AG_SurfaceNew(AG_SURFACE_INDEXED, w,h, pf, 0);
	AG_PixelFormatFree(pf);
	return (s);
}

/* Create a new packed-pixel surface with the specified RGB pixel format. */
AG_Surface *
AG_SurfaceRGB(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask)
{
	AG_PixelFormat *pf;
	AG_Surface *s;

	if ((pf = AG_PixelFormatRGB(bpp, Rmask, Gmask, Bmask)) == NULL) {
		return (NULL);
	}
	s = AG_SurfaceNew(AG_SURFACE_PACKED, w,h, pf, 0);
	AG_PixelFormatFree(pf);
	return (s);
}

/*
 * Create a new packed-pixel surface with the specified RGBA pixel format.
 * The SRCALPHA flag is set implicitely.
 */
AG_Surface *
AG_SurfaceRGBA(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask, Uint32 Amask)
{
	AG_PixelFormat *pf;
	AG_Surface *s;

	if ((pf = AG_PixelFormatRGBA(bpp, Rmask, Gmask, Bmask, Amask)) == NULL) {
		return (NULL);
	}
	s = AG_SurfaceNew(AG_SURFACE_PACKED, w,h, pf, AG_SRCALPHA);
	AG_PixelFormatFree(pf);
	return (s);
}

/* Create a new surface from pixel data in the specified packed RGB format. */
AG_Surface *
AG_SurfaceFromPixelsRGB(const void *pixels, Uint w, Uint h, int bpp,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	AG_PixelFormat *pf;
	AG_Surface *s;

	if ((pf = AG_PixelFormatRGB(bpp, Rmask, Gmask, Bmask)) == NULL) {
		return (NULL);
	}
	s = AG_SurfaceNew(AG_SURFACE_PACKED, w,h, pf, 0);
	memcpy(s->pixels, pixels, h*s->pitch);
	AG_PixelFormatFree(pf);
	return (s);
}

/*
 * Create a new surface from pixel data in the specified packed RGBA format.
 * The SRCALPHA flag is set implicitely.
 */
AG_Surface *
AG_SurfaceFromPixelsRGBA(const void *pixels, Uint w, Uint h, int bpp,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	AG_PixelFormat *pf;
	AG_Surface *s;

	if ((pf = AG_PixelFormatRGBA(bpp, Rmask, Gmask, Bmask, Amask)) == NULL) {
		return (NULL);
	}
	s = AG_SurfaceNew(AG_SURFACE_PACKED, w,h, pf, AG_SRCALPHA);
	memcpy(s->pixels, pixels, h*s->pitch);
	AG_PixelFormatFree(pf);
	return (s);
}

/* Load a surface from an image file. */
AG_Surface *
AG_SurfaceFromFile(const char *path)
{
	AG_Surface *su;
	const char *ext;

	if ((ext = strrchr(path, '.')) == NULL) {
		AG_SetError("Invalid filename");
		return (NULL);
	}
	if (Strcasecmp(ext, ".bmp") == 0) {
		su = AG_SurfaceFromBMP(path);
	} else if (Strcasecmp(ext, ".png") == 0) {
		su = AG_SurfaceFromPNG(path);
	} else if (Strcasecmp(ext, ".jpg") == 0 || Strcasecmp(ext, ".jpeg") == 0) {
		su = AG_SurfaceFromJPEG(path);
	} else {
		AG_SetError(_("Unknown image extension: %s"), ext);
		return (NULL);
	}
	return (su);
}

/* Export surface to an image file (format determined by extension). */
int
AG_SurfaceExportFile(const AG_Surface *su, const char *path)
{
	const char *ext;

	if ((ext = strrchr(path, '.')) == NULL) {
		AG_SetError("Invalid filename");
		return (-1);
	}
	if (Strcasecmp(ext, ".bmp") == 0) {
		return AG_SurfaceExportBMP(su, path);
	} else if (Strcasecmp(ext, ".png") == 0) {
		return AG_SurfaceExportPNG(su, path, 0);
	} else if (Strcasecmp(ext, ".jpg") == 0 || Strcasecmp(ext, ".jpeg") == 0) {
		return AG_SurfaceExportJPEG(su, path, 100, 0);
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
	AG_Surface *gsu;
	int w, h;

	/* TODO check for GL_ARB_texture_non_power_of_two. */
	w = PowOf2i(rw);
	h = PowOf2i(rh);
	gsu = AG_SurfaceRGBA(w, h, 32, 0,
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
	if (gsu == NULL) {
		return (NULL);
	}
	gsu->flags |= AG_SURFACE_GLTEXTURE;
	return (gsu);
}

/* Set one or more entries in an indexed surface's palette. */
int
AG_SurfaceSetPalette(AG_Surface *su, AG_Color *c, Uint offs, Uint count)
{
	Uint i;

	if (su->type != AG_SURFACE_INDEXED) {
		AG_SetError("Not an indexed surface");
		return (-1);
	}
	if (offs >= su->format->palette->nColors ||
	    offs+count >= su->format->palette->nColors) {
		AG_SetError("Bad palette offset/count");
		return (-1);
	}
	for (i = 0; i < count; i++) {
		su->format->palette->colors[offs+i] = c[i];
	}
	return (0);
}

/*
 * Return a newly-allocated duplicate of a surface.
 * The source surface must be locked.
 */
AG_Surface *
AG_SurfaceDup(const AG_Surface *ss)
{
	AG_Surface *s;

	s = AG_SurfaceNew(ss->type,
	    ss->w, ss->h, ss->format,
	    (ss->flags & AG_SAVED_SURFACE_FLAGS));
	if (s == NULL) {
		return (NULL);
	}
	memcpy(s->pixels, ss->pixels, ss->h*ss->pitch);
	return (s);
}

/* Return a newly-allocated duplicate of a surface, in specified format. */
AG_Surface *
AG_SurfaceConvert(const AG_Surface *ss, const AG_PixelFormat *pf)
{
	AG_Surface *ds;

	ds = AG_SurfaceNew(ss->type,
	    ss->w, ss->h, pf,
	    (ss->flags & AG_SAVED_SURFACE_FLAGS));
	AG_SurfaceCopy(ds, ss);
	return (ds);
}

/*
 * Copy pixel data from a source to a destination surface. Perform
 * conversion if pixel format differs. Perform clipping if dimensions
 * differ. Ignore the clipping rectangle and alpha/colorkey settings of
 * the target surface.
 */
void
AG_SurfaceCopy(AG_Surface *ds, const AG_Surface *ss)
{
	int w, h, x, y, padDst, padSrc;
	const Uint8 *pSrc;
	Uint8 *pDst;

	if (ds->w > ss->w) {
		w = ss->w;
		padDst = (ds->w - ss->w)*ds->format->BytesPerPixel;
		padSrc = 0;
	} else if (ds->w < ss->w) {
		w = ds->w;
		padDst = 0;
		padSrc = (ss->w - ds->w)*ss->format->BytesPerPixel;
	} else {
		w = ds->w;
		padSrc = 0;
		padDst = 0;
	}
	padSrc += ss->padding;
	padDst += ds->padding;
	h = MIN(ss->h, ds->h);

	pSrc = (Uint8 *)ss->pixels;
	pDst = (Uint8 *)ds->pixels;

	if (AG_PixelFormatCompare(ss->format, ds->format) == 0) {
		for (y = 0; y < h; y++) {
			memcpy(pDst, pSrc, w*ds->format->BytesPerPixel);
			pDst += w*ds->format->BytesPerPixel + padDst;
			pSrc += w*ss->format->BytesPerPixel + padSrc;
		}
	} else {					/* Format conversion */
		Uint32 px;
		AG_Color C;
	
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				px = AG_GET_PIXEL(ss,pSrc);
				C = AG_GetColorRGBA(px, ss->format);
				AG_PUT_PIXEL(ds,pDst,
				    AG_MapColorRGBA(ds->format, C));
				pSrc += ss->format->BytesPerPixel;
				pDst += ds->format->BytesPerPixel;
			}
			pDst += padDst;
			pSrc += padSrc;
		}
	}
}

/*
 * Copy a region of pixels srcRect from source surface ss to destination
 * surface ds, at destination coordinates xDst,yDst. It is safe to exceed
 * the dimensions of ds. Unlike AG_SurfaceCopy(), blending and colorkey
 * tests are done. If srcRect is passed NULL, the entire surface is copied.
 */
void
AG_SurfaceBlit(const AG_Surface *ss, const AG_Rect *srcRect, AG_Surface *ds,
    int xDst, int yDst)
{
	Uint32 pixel;
	Uint8 *pSrc, *pDst;
	AG_Color C;
	AG_Rect sr, dr;
	Uint x, y;

	/* Compute the effective source and destination rectangles. */
	if (srcRect != NULL) {
		sr = *srcRect;
		if (sr.x < 0) { sr.x = 0; }
		if (sr.y < 0) { sr.y = 0; }
		if (sr.x+sr.w >= ss->w) { sr.w = ss->w - sr.x; }
		if (sr.y+sr.h >= ss->h) { sr.h = ss->h - sr.y; }
	} else {
		sr.x = 0;
		sr.y = 0;
		sr.w = ss->w;
		sr.h = ss->h;
	}
	dr.x = MAX(xDst, ds->clipRect.x);
	dr.y = MAX(yDst, ds->clipRect.y);
	dr.w = (dr.x+sr.w > ds->clipRect.x+ds->clipRect.w) ?
	        (ds->clipRect.x+ds->clipRect.w - dr.x) : sr.w;
	dr.h = (dr.y+sr.h > ds->clipRect.y+ds->clipRect.h) ?
	        (ds->clipRect.y+ds->clipRect.h - dr.y) : sr.h;

	/* XXX TODO optimized cases */
	for (y = 0; y < dr.h; y++) {
		pSrc = (Uint8 *)ss->pixels + (sr.y+y)*ss->pitch +
		    sr.x*ss->format->BytesPerPixel;
		pDst = (Uint8 *)ds->pixels + (dr.y+y)*ds->pitch +
		    dr.x*ds->format->BytesPerPixel;
		for (x = 0; x < dr.w; x++) {
			pixel = AG_GET_PIXEL(ss, pSrc);
			if ((ss->flags & AG_SRCCOLORKEY) &&
			    (ss->format->colorkey == pixel)) {
				pSrc += ss->format->BytesPerPixel;
				pDst += ds->format->BytesPerPixel;
				continue;
			}
			C = AG_GetColorRGBA(pixel, ss->format);
			if (ss->format->alpha != AG_ALPHA_OPAQUE) {
				/* Apply per-surface alpha */
				C.a *= ss->format->alpha/255;
				AG_SurfaceBlendPixel(ds, pDst, C, AG_ALPHA_SRC);
			} else {
				if ((C.a != AG_ALPHA_OPAQUE) &&
				    (ss->flags & AG_SRCALPHA)) {
					AG_SurfaceBlendPixel(ds, pDst, C, AG_ALPHA_SRC);
				} else {
					AG_PUT_PIXEL(ds, pDst,
					    AG_MapColorRGB(ds->format, C));
				}
			}
			pSrc += ss->format->BytesPerPixel;
			pDst += ds->format->BytesPerPixel;
		}
	}
}

/* Resize a surface; pixels are left uninitialized. */
int
AG_SurfaceResize(AG_Surface *s, Uint w, Uint h)
{
	Uint8 *pixelsNew;
	int pitchNew = w*s->format->BytesPerPixel;

	if ((pixelsNew = TryRealloc(s->pixels, h*pitchNew)) == NULL) {
		return (-1);
	}
	s->pixels = pixelsNew;
	s->pitch = pitchNew;
	s->w = w;
	s->h = h;
	s->clipRect = AG_RECT(0,0,w,h);
	return (0);
}

/* Free the specified surface. */
void
AG_SurfaceFree(AG_Surface *s)
{
	AG_PixelFormatFree(s->format);
	Free(s->pixels);
	Free(s);
}

/*
 * Blend the specified components with the pixel at s:[x,y], using the
 * given alpha function. No clipping is done.
 */
void
AG_SurfaceBlendPixel(AG_Surface *s, Uint8 *pDst, AG_Color Cnew, AG_BlendFn fn)
{
	Uint32 pxDst;
	AG_Color Cdst;
/*	Uint8 a; */

	pxDst = AG_GET_PIXEL(s, pDst);
	if ((s->flags & AG_SRCCOLORKEY) && (pxDst == s->format->colorkey)) {
	 	AG_SurfacePutPixel(s, pDst,
		    AG_MapColorRGBA(s->format, Cnew));
	} else {
		Cdst = AG_GetColorRGBA(pxDst, s->format);
#if 0
		switch (fn) {
		case AG_ALPHA_DST:
			a = Cdst.a;
			break;
		case AG_ALPHA_SRC:
			a = Cnew.a;
			break;
		case AG_ALPHA_ZERO:
			a = 0;
			break;
		case AG_ALPHA_OVERLAY:
			a = (Uint8)((Cdst.a+Cnew.a) > 255) ? 255 :
			            (Cdst.a+Cnew.a);
			break;
		case AG_ALPHA_ONE_MINUS_DST:
			a = 255-Cdst.a;
			break;
		case AG_ALPHA_ONE_MINUS_SRC:
			a = 255-Cnew.a;
			break;
		case AG_ALPHA_ONE:
		default:
			a = 255;
			break;
		}
#endif
		AG_SurfacePutPixel(s, pDst,
		    AG_MapPixelRGBA(s->format,
		    (((Cnew.r - Cdst.r)*Cnew.a) >> 8) + Cdst.r,
		    (((Cnew.g - Cdst.g)*Cnew.a) >> 8) + Cdst.g,
		    (((Cnew.b - Cdst.b)*Cnew.a) >> 8) + Cdst.b,
		    Cdst.a));
	}
}

/*
 * Obtain the hue/saturation/value of a given RGB triplet.
 * Note that the hue is lost as saturation approaches 0.
 */
void
AG_RGB2HSV(Uint8 r, Uint8 g, Uint8 b, float *h, float *s, float *v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/255.0F;
	vG = (float)g/255.0F;
	vB = (float)b/255.0F;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {
		/* This is a gray color (zero hue, no saturation). */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0F + deltaMax/2.0F) / deltaMax;
		deltaG = ((vMax - vG)/6.0F + deltaMax/2.0F) / deltaMax;
		deltaB = ((vMax - vB)/6.0F + deltaMax/2.0F) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0F;
		} else if (vG == vMax) {
			*h = 120.0F + (deltaR - deltaB)*360.0F;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0F + (deltaG - deltaR)*360.0F;	/* 2/3 */
		}

		if (*h < 0.0F)		(*h)++;
		if (*h > 360.0F)	(*h)--;
	}
}

/* Convert hue/saturation/value to RGB. */
void
AG_HSV2RGB(float h, float s, float v, Uint8 *r, Uint8 *g, Uint8 *b)
{
	float var[3];
	float vR, vG, vB, hv;
	int iv;

	if (s == 0.0) {
		*r = (Uint8)v*255;
		*g = (Uint8)v*255;
		*b = (Uint8)v*255;
		return;
	}
	
	hv = h/60.0F;
	iv = Floor(hv);
	var[0] = v * (1.0F - s);
	var[1] = v * (1.0F - s*(hv - iv));
	var[2] = v * (1.0F - s*(1.0F - (hv - iv)));

	switch (iv) {
	case 0:		vR = v;		vG = var[2];	vB = var[0];	break;
	case 1:		vR = var[1];	vG = v;		vB = var[0];	break;
	case 2:		vR = var[0];	vG = v;		vB = var[2];	break;
	case 3:		vR = var[0];	vG = var[1];	vB = v;		break;
	case 4:		vR = var[2];	vG = var[0];	vB = v;		break;
	default:	vR = v;		vG = var[0];	vB = var[1];	break;
	}
	
	*r = vR*255;
	*g = vG*255;
	*b = vB*255;
}

/*
 * Allocate a new surface containing a pixmap of ss scaled to wxh.
 * XXX TODO optimize; filtering
 */
int
AG_ScaleSurface(const AG_Surface *ss, Uint16 w, Uint16 h, AG_Surface **ds)
{
	Uint8 *pDst;
	int x, y;
	int sameFormat;

	if (*ds == NULL) {
		*ds = AG_SurfaceNew(
		    AG_SURFACE_PACKED,
		    w, h, ss->format,
		    ss->flags & (AG_SRCALPHA|AG_SRCCOLORKEY));
		if (*ds == NULL) {
			return (-1);
		}
		(*ds)->format->alpha = ss->format->alpha;
		(*ds)->format->colorkey = ss->format->colorkey;
		sameFormat = 1;
	} else {
		//sameFormat = !AG_PixelFormatCompare((*ds)->format, ss->format);
		sameFormat = 0;
	}

	if (ss->w == w && ss->h == h) {
		AG_SurfaceCopy(*ds, ss);
		return (0);
	}

	pDst = (Uint8 *)(*ds)->pixels;
	for (y = 0; y < (*ds)->h; y++) {
		for (x = 0; x < (*ds)->w; x++) {
			Uint8 *pSrc = (Uint8 *)ss->pixels +
			    (y*ss->h/(*ds)->h)*ss->pitch +
			    (x*ss->w/(*ds)->w)*ss->format->BytesPerPixel;
			Uint32 pxSrc, pxDst;
			AG_Color C;

			pxSrc = AG_GET_PIXEL(ss,pSrc);
			if (sameFormat) {
				pxDst = pxSrc;
			} else {
				C = AG_GetColorRGBA(pxSrc, ss->format);
				pxDst = AG_MapColorRGBA((*ds)->format, C);
			}
			AG_SurfacePutPixel((*ds), pDst, pxDst);
			pDst += (*ds)->format->BytesPerPixel;
		}
	}
	return (0);
}

/* Set the alpha value of all pixels in a surface where a != 0. */
void
AG_SetAlphaPixels(AG_Surface *su, Uint8 alpha)
{
	Uint8 *pDst = (Uint8 *)su->pixels;
	int x, y;
	AG_Color C;

	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			/* XXX unnecessary conversion */
			C = AG_GetColorRGBA(AG_GET_PIXEL(su,pDst), su->format);
			if (C.a != 0) { C.a = alpha; }
			AG_SurfacePutPixel(su, pDst,
			    AG_MapColorRGBA(su->format, C));
			pDst += su->format->BytesPerPixel;
		}
	}
}

/* Fill a rectangle with pixels of the specified color. */
void
AG_FillRect(AG_Surface *su, const AG_Rect *rDst, AG_Color C)
{
	int x, y;
	Uint32 px;
	AG_Rect r;

	if (rDst != NULL) {
		r = *rDst;
		if (r.x < su->clipRect.x) { r.x = su->clipRect.x; }
		if (r.y < su->clipRect.y) { r.y = su->clipRect.y; }
		if (r.x+r.w >= su->clipRect.x+su->clipRect.w)
			r.w = su->clipRect.x+su->clipRect.w - r.x;
		if (r.y+r.h >= su->clipRect.y+su->clipRect.h)
			r.h = su->clipRect.y+su->clipRect.h - r.y;
	} else {
		r = su->clipRect;
	}
	px = AG_MapColorRGBA(su->format, C);

	/* XXX TODO optimize */
	for (y = 0; y < r.h; y++) {
		for (x = 0; x < r.w; x++) {
			AG_PUT_PIXEL2(su,
			    r.x + x,
			    r.y + y,
			    px);
		}
	}
}

/* Called by AG_MapPixelRGB() for color-index surfaces. */
Uint32
AG_MapPixelIndexedRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b)
{
	Uint i, iMin = 0;
	int err, errMin = 255*3;

	for (i = 0; i < pf->palette->nColors; i++) {
		AG_Color *C = &pf->palette->colors[i];

		err = Fabs(C->r - r) + Fabs(C->g - g) + Fabs(C->b - b);
		if (err < errMin) {
			errMin = err;
			iMin = i;
		}
	}
	return (Uint32)iMin;
}

/* Called by AG_MapPixelRGBA() for color-index surfaces. */
Uint32
AG_MapPixelIndexedRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b,
    Uint8 a)
{
	Uint i, iMin = 0;
	int err, errMin = 255*4;

	for (i = 0; i < pf->palette->nColors; i++) {
		AG_Color *C = &pf->palette->colors[i];

		err = Fabs(C->r - r) + Fabs(C->g - g) + Fabs(C->b - b) +
		      Fabs(C->a - a);
		if (err < errMin) {
			errMin = err;
			iMin = i;
		}
	}
	return (Uint32)iMin;
}

#ifdef AG_LEGACY
void        AG_SurfaceLock(AG_Surface *su) { /* No-op */ }
void        AG_SurfaceUnlock(AG_Surface *su) { /* No-op */ }
Uint32      AG_MapRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b) { return AG_MapPixelRGB(pf, r,g,b); }
Uint32      AG_MapRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a) { return AG_MapPixelRGBA(pf, r,g,b,a); }
AG_Surface *AG_DupSurface(AG_Surface *su) { return AG_SurfaceDup((const AG_Surface *)su); }
void        AG_GetRGB(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b) { AG_GetPixelRGB(px, pf, r,g,b); }
void        AG_GetRGBA(Uint32 px, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) { AG_GetPixelRGBA(px, pf, r,g,b,a); }
int         AG_SamePixelFmt(const AG_Surface *s1, const AG_Surface *s2) { return (AG_PixelFormatCompare(s1->format, s2->format)) == 0; }
#endif /* AG_LEGACY */

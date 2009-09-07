/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "opengl.h"

#include <core/core.h>
#include <core/config.h>

#include "geometry.h"
#include "surface.h"
#include "gui_math.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <config/have_jpeg.h>
#ifdef HAVE_JPEG
#undef HAVE_STDLIB_H		/* Work around SDL.h retardation */
#include <jpeglib.h>
#include <errno.h>
#endif

const char *agBlendFuncNames[] = {
	"dst+src",
	"src",
	"dst",
	"1-dst",
	"1-src",
	NULL
};

#define COMPUTE_SHIFTLOSS(mask, shift, loss) \
	shift = 0; \
	loss = 8; \
	if (mask != 0) { \
		for (m = mask ; (m & 0x01) == 0; m >>= 1) { \
			shift++; \
		} \
		while ((m & 0x01) != 0) { \
			loss--; \
			m >>= 1; \
		} \
	}

/* Specify a packed-pixel format from three 32-bit bitmasks. */
AG_PixelFormat *
AG_PixelFormatRGB(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	AG_PixelFormat *pf;
	Uint32 m;

	pf = Malloc(sizeof(AG_PixelFormat));
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->palette = NULL;
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = 0x00000000;
	COMPUTE_SHIFTLOSS(pf->Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTLOSS(pf->Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTLOSS(pf->Bmask, pf->Bshift, pf->Bloss);
	return (pf);
}

/* Specify a packed-pixel format from four 32-bit bitmasks. */
AG_PixelFormat *
AG_PixelFormatRGBA(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask,
    Uint32 Amask)
{
	AG_PixelFormat *pf;
	Uint32 m;

	pf = Malloc(sizeof(AG_PixelFormat));
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	pf->palette = NULL;
	pf->Rmask = Rmask;
	pf->Gmask = Gmask;
	pf->Bmask = Bmask;
	pf->Amask = Amask;
	COMPUTE_SHIFTLOSS(pf->Rmask, pf->Rshift, pf->Rloss);
	COMPUTE_SHIFTLOSS(pf->Gmask, pf->Gshift, pf->Gloss);
	COMPUTE_SHIFTLOSS(pf->Bmask, pf->Bshift, pf->Bloss);
	COMPUTE_SHIFTLOSS(pf->Amask, pf->Ashift, pf->Aloss);
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

	pf = Malloc(sizeof(AG_PixelFormat));
	pf->alpha = AG_ALPHA_OPAQUE;
	pf->BitsPerPixel = bpp;
	pf->BytesPerPixel = (bpp+7)/8;
	
	pal = pf->palette = Malloc(sizeof(AG_Palette));
	pal->ncolors = 1<<bpp;
	pal->colors = Malloc(pal->ncolors*sizeof(AG_Color));
	
	if (bpp == 2) {
		pal->colors[0].r = 255;
		pal->colors[0].g = 255;
		pal->colors[0].b = 255;
		pal->colors[1].r = 0;
		pal->colors[1].g = 0;
		pal->colors[1].b = 0;
	} else {
		memset(pal->colors, 0, pal->ncolors*sizeof(AG_Color));
	}

	pf->Rmask = pf->Gmask = pf->Bmask = pf->Amask = 0;
	pf->Rloss = pf->Gloss = pf->Bloss = pf->Aloss = 8;
	pf->Rshift = pf->Gshift = pf->Bshift = pf->Ashift = 0;
	return (pf);
}

AG_PixelFormat *
AG_PixelFormatDup(const AG_PixelFormat *pf)
{
	AG_PixelFormat *pfd;

	pfd = Malloc(sizeof(AG_PixelFormat));
	if (pf->palette != NULL) {
		pfd->palette = Malloc(pf->palette->ncolors*sizeof(AG_Color));
		memcpy(pfd->palette->colors, pf->palette->colors,
		    pf->palette->ncolors*sizeof(AG_Color));
	} else {
		pfd->palette = NULL;
	}
	pfd->BitsPerPixel = pf->BitsPerPixel;
	pfd->BytesPerPixel = pf->BytesPerPixel;
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
	pfd->colorkey = pf->colorkey;
	pfd->alpha = pf->alpha;
	return (pfd);
}

void
AG_PixelFormatFree(AG_PixelFormat *fmt)
{
	Free(fmt->palette);
	free(fmt);
}

#undef COMPUTE_SHIFTLOSS

static __inline__ Uint32
SDLFlags(Uint flags)
{
	Uint32 sdlFlags = SDL_SWSURFACE;
	if (flags & AG_HWSURFACE) { sdlFlags |= SDL_HWSURFACE; }
	if (flags & AG_SRCCOLORKEY) { sdlFlags |= SDL_SRCCOLORKEY; }
	if (flags & AG_SRCALPHA) { sdlFlags |= SDL_SRCALPHA; }
	return (sdlFlags);
}

/* Create a new surface of the specified pixel format. */
AG_Surface *
AG_SurfaceNew(Uint w, Uint h, AG_PixelFormat *fmt, Uint flags)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w, h,
	    fmt->BitsPerPixel,
	    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h,
		    fmt->BitsPerPixel, SDL_GetError());
		return (NULL);
	}
	if (fmt->palette != NULL) {
		SDL_SetPalette((SDL_Surface *)s, SDL_LOGPAL,
		    (SDL_Color *)fmt->palette, 0, fmt->palette->ncolors);
	}
	return (s);
}

/* Create an empty surface. */
AG_Surface *
AG_SurfaceEmpty(void)
{
	return (AG_Surface *)SDL_CreateRGBSurface(SDL_SWSURFACE, 0,0,8,0,0,0,0);
}

/* Create a new surface of the specified pixel format. */
AG_Surface *
AG_SurfaceIndexed(Uint w, Uint h, int bpp, Uint flags)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w,h, bpp,
	    0,0,0,0);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h, bpp,
		    SDL_GetError());
		return (NULL);
	}
	return (s);
}

/* Create a new surface with the specified RGB pixel-packing format. */
AG_Surface *
AG_SurfaceRGB(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w,h, bpp,
	    Rmask,Gmask,Bmask,0);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h, bpp,
		    SDL_GetError());
		return (NULL);
	}
	return (s);
}

/* Create a new surface with the specified RGB pixel-packing format. */
AG_Surface *
AG_SurfaceRGBA(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask, Uint32 Amask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurface(SDLFlags(flags), w,h, bpp,
	    Rmask,Gmask,Bmask,Amask);
	if (s == NULL) {
		AG_SetError("SDL_CreateRGBSurface(%ux%ux%u): %s", w, h, bpp,
		    SDL_GetError());
		return (NULL);
	}
	return (s);
}

/* Create a new surface from pixel data in the specified packed RGB format. */
AG_Surface *
AG_SurfaceFromPixelsRGB(void *pixels, Uint w, Uint h, int bpp, int pitch,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurfaceFrom(pixels, (int)w, (int)h,
	    bpp, pitch, Rmask, Gmask, Bmask, 0);
	return (s);
}

/* Create a new surface from pixel data in the specified packed RGBA format. */
AG_Surface *
AG_SurfaceFromPixelsRGBA(void *pixels, Uint w, Uint h, int bpp, int pitch,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	AG_Surface *s;

	s = (AG_Surface *)SDL_CreateRGBSurfaceFrom(pixels, (int)w, (int)h,
	    bpp, pitch, Rmask, Gmask, Bmask, Amask);
	return (s);
}

/* Create a new surface from a .bmp file. */
AG_Surface *
AG_SurfaceFromBMP(const char *path)
{
	AG_Surface *s;

	if ((s = (AG_Surface *)SDL_LoadBMP(path)) == NULL) {
		AG_SetError("SDL_LoadBMP(%s): %s", path, SDL_GetError());
		return (NULL);
	}
	return (s);
}

AG_Surface *
AG_SurfaceFromSDL(AG_Surface *su)
{
	return AG_DupSurface(su);
}

SDL_Surface *
AG_SurfaceToSDL(AG_Surface *su)
{
	return (SDL_Surface *)AG_DupSurface(su);
}

AG_Surface *
AG_SurfaceFromSurface(AG_Surface *su, AG_PixelFormat *fmt, Uint flags)
{
	return (AG_Surface *)SDL_ConvertSurface(su, fmt, SDLFlags(flags));
}

void
AG_SurfaceCopy(AG_Surface *ds, AG_Surface *ss)
{
	SDL_Surface *dsSDL = (SDL_Surface *)ds;
	SDL_Surface *ssSDL = (SDL_Surface *)ss;
	Uint32 aflagsSave = ssSDL->flags & (AG_SRCALPHA|AG_RLEACCEL);
	Uint8 alphaSave = ssSDL->format->alpha;
	Uint32 cflagsSave = ssSDL->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
	Uint32 ckeySave = ssSDL->format->colorkey;

	SDL_SetAlpha(ssSDL, 0, 0);
	SDL_SetColorKey(ssSDL, 0, 0);
	SDL_BlitSurface(ssSDL, NULL, dsSDL, NULL);
	SDL_SetAlpha(ssSDL, aflagsSave, alphaSave);
	SDL_SetColorKey(ssSDL, cflagsSave, ckeySave);
}

/* Free the specified surface. */
void
AG_SurfaceFree(AG_Surface *s)
{
	SDL_FreeSurface((SDL_Surface *)s);
}

/* Dump a surface to a JPEG image. */
int
AG_DumpSurface(AG_Surface *su, char *path_save)
{
#ifdef HAVE_JPEG
	char path[AG_PATHNAME_MAX];
	struct jpeg_error_mgr jerrmgr;
	struct jpeg_compress_struct jcomp;
	Uint8 *jcopybuf;
	FILE *fp;
	Uint seq = 0;
	int fd;
	JSAMPROW row[1];
	int x;

	AG_GetString(agConfig, "save-path", path, sizeof(path));
	Strlcat(path, AG_PATHSEP, sizeof(path));
	Strlcat(path, "screenshot", sizeof(path));
	if (AG_MkDir(path) == -1 && errno != EEXIST) {
		AG_SetError("mkdir %s: %s", path, strerror(errno));
		return (-1);
	}
	for (;;) {
		char file[AG_PATHNAME_MAX];

		Snprintf(file, sizeof(file), "%s/%s%u.jpg", path, agProgName,
		    seq++);
		if ((fd = open(file, O_WRONLY|O_CREAT|O_EXCL, 0600)) == -1) {
			if (errno == EEXIST) {
				continue;
			} else {
				AG_SetError("%s: %s", file, strerror(errno));
				goto out;
			}
		}
		break;
	}
	if (path_save != NULL)
		Strlcpy(path_save, path, AG_PATHNAME_MAX);

	if ((fp = fdopen(fd, "wb")) == NULL) {
		AG_SetError("fdopen: %s", strerror(errno));
		return (-1);
	}

	jcomp.err = jpeg_std_error(&jerrmgr);

	jpeg_create_compress(&jcomp);

	jcomp.image_width = su->w;
	jcomp.image_height = su->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, agScreenshotQuality, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(su->w*3);

	jpeg_start_compress(&jcomp, TRUE);
	while (jcomp.next_scanline < jcomp.image_height) {
		Uint8 *pSrc = (Uint8 *)su->pixels +
		    jcomp.next_scanline*su->pitch;
		Uint8 *pDst = jcopybuf;
		Uint8 r, g, b;

		for (x = su->w; x > 0; x--) {
			AG_GetRGB(AG_GET_PIXEL(su,pSrc), su->format, &r,&g,&b);
			*pDst++ = r;
			*pDst++ = g;
			*pDst++ = b;
			pSrc += su->format->BytesPerPixel;
		}
		row[0] = jcopybuf;
		jpeg_write_scanlines(&jcomp, row, 1);
	}
	jpeg_finish_compress(&jcomp);
	jpeg_destroy_compress(&jcomp);

	fclose(fp);
	close(fd);
	Free(jcopybuf);
out:
	return (0);
#else
	AG_SetError(_("Screenshot feature requires libjpeg"));
	return (-1);
#endif
}

/* Flip the lines of a surface; this is useful with glReadPixels(). */
void
AG_FlipSurface(Uint8 *src, int h, int pitch)
{
	Uint8 *tmp = Malloc(pitch);
	int h2 = h >> 1;
	Uint8 *p1 = &src[0];
	Uint8 *p2 = &src[(h-1)*pitch];
	int i;

	for (i = 0; i < h2; i++) {
		memcpy(tmp, p1, pitch);
		memcpy(p1, p2, pitch);
		memcpy(p2, tmp, pitch);

		p1 += pitch;
		p2 -= pitch;
	}
	Free(tmp);
}

/*
 * Blend the specified components with the pixel at s:[x,y], using the
 * given alpha function.
 *
 * Clipping is not done; the destination surface must be locked.
 */
void
AG_BlendPixelRGBA(AG_Surface *s, Uint8 *pDst, Uint8 sR, Uint8 sG, Uint8 sB,
    Uint8 sA, AG_BlendFn func)
{
	Uint32 cDst;
	Uint8 dR, dG, dB, dA;
	int alpha = 0;

	cDst = AG_GET_PIXEL(s, pDst);
	if ((s->flags & AG_SRCCOLORKEY) && (cDst == s->format->colorkey)) {
	 	AG_PUT_PIXEL(s, pDst, AG_MapRGBA(s->format, sR,sG,sB,sA));
	} else {
		AG_GetRGBA(cDst, s->format, &dR, &dG, &dB, &dA);
		switch (func) {
		case AG_ALPHA_OVERLAY:	alpha = dA+sA; break;
		case AG_ALPHA_SRC:	alpha = sA; break;
		case AG_ALPHA_DST:	alpha = dA; break;
		case AG_ALPHA_ONE_MINUS_DST: alpha = 1-dA; break;
		case AG_ALPHA_ONE_MINUS_SRC: alpha = 1-sA; break;
		}
		alpha = (alpha < 0) ? 0 : (alpha > 255) ? 255 : alpha;
		AG_PUT_PIXEL(s, pDst, AG_MapRGBA(s->format,
		    (((sR - dR) * sA) >> 8) + dR,
		    (((sG - dG) * sA) >> 8) + dG,
		    (((sB - dB) * sA) >> 8) + dB,
		    (Uint8)alpha));
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

/* Return a newly allocated surface containing a copy of ss. */
AG_Surface *
AG_DupSurface(AG_Surface *ss)
{
	AG_Surface *rs;

	rs = (AG_Surface *)SDL_ConvertSurface(ss, ss->format, SDL_SWSURFACE |
	    (ss->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCEL)));
	if (rs == NULL) {
		AG_SetError("SDL_ConvertSurface: %s", SDL_GetError());
		return (NULL);
	}
	rs->format->alpha = ss->format->alpha;
	rs->format->colorkey = ss->format->colorkey;
	return (rs);
}

/*
 * Allocate a new surface containing a pixmap of ss scaled to wxh.
 * The source surface must not be locked by the calling thread.
 *
 * XXX very primitive and inefficient
 */
int
AG_ScaleSurface(AG_Surface *ss, Uint16 w, Uint16 h, AG_Surface **ds)
{
	Uint8 r1, g1, b1, a1;
	Uint8 *pDst;
	int x, y;
	int same_fmt;

	if (*ds == NULL) {
		*ds = AG_SurfaceNew(w, h, ss->format,
		    ss->flags & (AG_SWSURFACE|AG_SRCALPHA|AG_SRCCOLORKEY));
		if (*ds == NULL) {
			return (-1);
		}
		(*ds)->format->alpha = ss->format->alpha;
		(*ds)->format->colorkey = ss->format->colorkey;
		same_fmt = 1;
	} else {
		//same_fmt = AG_SamePixelFmt(*ds, ss);
		same_fmt = 0;
	}

	if (ss->w == w && ss->h == h) {
		AG_SurfaceCopy(*ds, ss);
		return (0);
	}

	AG_SurfaceLock(ss);
	AG_SurfaceLock(*ds);
	pDst = (Uint8 *)(*ds)->pixels;
	for (y = 0; y < (*ds)->h; y++) {
		for (x = 0; x < (*ds)->w; x++) {
			Uint8 *pSrc = (Uint8 *)ss->pixels +
			    (y*ss->h/(*ds)->h)*ss->pitch +
			    (x*ss->w/(*ds)->w)*ss->format->BytesPerPixel;
			Uint32 cSrc = AG_GET_PIXEL(ss, pSrc);
			Uint32 cDst;

			if (same_fmt) {
				cDst = cSrc;
			} else {
				AG_GetRGBA(cSrc, ss->format,
				    &r1, &g1, &b1, &a1);
				cDst = AG_MapRGBA((*ds)->format,
				    r1, g1, b1, a1);
			}
			AG_PUT_PIXEL((*ds), pDst, cDst);
			pDst += (*ds)->format->BytesPerPixel;
		}
	}
	AG_SurfaceUnlock(*ds);
	AG_SurfaceUnlock(ss);
	return (0);
}

/* Set the alpha value of all pixels in a surface where a != 0. */
void
AG_SetAlphaPixels(AG_Surface *su, Uint8 alpha)
{
	int x, y;

	AG_SurfaceLock(su);
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint8 *dst = (Uint8 *)su->pixels +
			    y*su->pitch +
			    x*su->format->BytesPerPixel;
			Uint8 r, g, b, a;

			AG_GetRGBA(*(Uint32 *)dst, su->format, &r, &g, &b, &a);

			if (a != 0)
				a = alpha;

			AG_PUT_PIXEL(su, dst,
			    AG_MapRGBA(su->format, r, g, b, a));
		}
	}
	AG_SurfaceUnlock(su);
}


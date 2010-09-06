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

/*
 * Support for reading image files in PNG format via libpng.
 */

#include <core/core.h>
#include "gui.h"

#include <config/have_png.h>
#include <config/have_libpng14.h>
#if defined(HAVE_PNG)

#ifdef macintosh
# define MACOS
#endif
#include <png.h>

int agPNGInterlacing = 0;		/* Save with interlacing */

void
AG_SetPNGInterlacing(int enable)
{
	agPNGInterlacing = enable;	/* Atomic */
}

/* Load a surface from a PNG image file. */
AG_Surface *
AG_SurfaceFromPNG(const char *path)
{
	AG_DataSource *ds;
	AG_Surface *s;

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		return (NULL);
	}
	if ((s = AG_ReadSurfaceFromPNG(ds)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		AG_CloseFile(ds);
		return (NULL);
	}
	AG_CloseFile(ds);
	return (s);
}

static void
AG_PNG_ReadData(png_structp png, png_bytep buf, png_size_t size)
{
	AG_DataSource *ds = (AG_DataSource*)png_get_io_ptr(png);
	AG_Read(ds, buf, size, 1);
}

#if 1
static void
SurfaceColorkeyToAlpha(AG_Surface *su)
{
	Uint8 *p = su->pixels;
	int x, y;

	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint32 c = AG_GET_PIXEL(su,p);
			AG_Color C = AG_GetColorRGBA(c, su->format);
		
			if (C.a == 0) {
				c = AG_MapPixelRGBA(su->format, 0,200,0,0);
			}
			AG_PUT_PIXEL(su, p, c);
			p += su->format->BytesPerPixel;
		}
	}
}
#endif

/* Load a surface from PNG image data. */
AG_Surface *
AG_ReadSurfaceFromPNG(AG_DataSource *ds)
{
	int start;
	AG_Surface *volatile su = NULL;
	png_structp png;
	png_infop info;
	png_uint_32 width, height;
	int depth, colorType, intlaceType;
	Uint32 Rmask = 0, Gmask = 0, Bmask = 0, Amask = 0;
	AG_Palette *pal;
	png_bytep *volatile pData = NULL;
	int row, i, t;
	volatile int ckey = -1;
	png_color_16 *transColor;
	int channels;

	start = AG_Tell(ds);

	if ((png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
	    NULL, NULL, NULL)) == NULL) {
		AG_SetError("Out of memory (libpng)");
		goto fail;
	}
	if ((info = png_create_info_struct(png)) == NULL) {
		AG_SetError("png_create_info_struct() failed");
		goto fail;
	}

	png_set_read_fn(png, ds, AG_PNG_ReadData);

	/* Read PNG image information. */
	png_read_info(png, info);
	png_get_IHDR(png, info,
	    &width, &height, &depth,
	    &colorType, &intlaceType,
	    NULL, NULL);

	/* Strip the pixels with 16 bits/channel to 8 bits/channel. */
	png_set_strip_16(png);

	/*
	 * Always expand 1/2/4-bit images to 8-bit, and expand grayscales
	 * images of depth <8-bit to 8-bit.
	 */
	png_set_packing(png);
	if (colorType == PNG_COLOR_TYPE_GRAY)
		png_set_expand(png);

	png_set_tRNS_to_alpha(png);

	/* Figure out transparency information. */
	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
	        int num_trans;
		Uint8 *trans;

		png_get_tRNS(png, info, &trans, &num_trans, &transColor);
		if (colorType == PNG_COLOR_TYPE_PALETTE) {
			for (i = 0, t = -1;
			     i < num_trans;
			     i++) {
				if (trans[i] == 0) {
					if (t >= 0)
						break;
					t = i;
				} else if (trans[i] != 255) {
					break;
				}
			}
			if (i == num_trans) {
				ckey = t;
			} else {
				png_set_expand(png);
			}
		} else {
			ckey = 0;
		}
	}

	/* Expand grayscale to 24-bit RGB */
	if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	/* Update png_info structure per our requirements. */
	png_read_update_info(png, info);

	png_get_IHDR(png, info,
	    &width, &height, &depth,
	    &colorType, &intlaceType,
	    NULL, NULL);

#ifdef HAVE_LIBPNG14
	channels = (int)png_get_channels(png, info);
#else
	channels = info->channels;
#endif

	if (colorType != PNG_COLOR_TYPE_PALETTE) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
		int s = (channels == 4) ? 0 : 8;
		Rmask = 0xff000000 >> s;
		Gmask = 0x00ff0000 >> s;
		Bmask = 0x0000ff00 >> s;
		Amask = 0x000000ff >> s;
#else
		Rmask = 0x000000ff;
		Gmask = 0x0000ff00;
		Bmask = 0x00ff0000;
		Amask = (channels == 4) ? 0xff000000 : 0;
#endif
	}
	if ((su = AG_SurfaceRGBA(width, height, depth*channels, 0,
	    Rmask, Gmask, Bmask, Amask)) == NULL)
		goto fail;

	if (ckey != -1) {
	        if (colorType != PNG_COLOR_TYPE_PALETTE) {
		        ckey = AG_MapPixelRGB(su->format,
			    (Uint8)transColor->red,
			    (Uint8)transColor->green,
			    (Uint8)transColor->blue);
		}
	        AG_SurfaceSetColorKey(su, AG_SRCCOLORKEY, ckey);
	}

	/* Read image data */
	if ((pData = TryMalloc(sizeof(png_bytep)*height)) == NULL) {
		goto fail;
	}
	for (row = 0; row < (int)height; row++) {
		pData[row] = (png_bytep)(Uint8 *)su->pixels + row*su->pitch;
	}
	png_read_image(png, pData);
	SurfaceColorkeyToAlpha(su);

	/* Read palette information */
	if ((pal = su->format->palette) != NULL) {
#ifdef HAVE_LIBPNG14
		int numPalette;
		png_colorp palette;
#endif
		if (colorType == PNG_COLOR_TYPE_GRAY) {
			pal->nColors = 256;
			for (i = 0; i < 256; i++) {
				pal->colors[i].r = i;
				pal->colors[i].g = i;
				pal->colors[i].b = i;
			}
		}
#ifdef HAVE_LIBPNG14
		else if (png_get_PLTE(png, info, &palette, &numPalette)) {
			pal->nColors = numPalette; 
			for (i = 0; i < numPalette; i++) {
				pal->colors[i].b = palette[i].blue;
				pal->colors[i].g = palette[i].green;
				pal->colors[i].r = palette[i].red;
			}
		}
#else /* !HAVE_LIBPNG14 */
		else if (info->num_palette > 0) {
			pal->nColors = info->num_palette; 
			for (i = 0; i < info->num_palette; i++) {
				pal->colors[i].b = info->palette[i].blue;
				pal->colors[i].g = info->palette[i].green;
				pal->colors[i].r = info->palette[i].red;
			}
		}
#endif /* HAVE_LIBPNG14 */
	}
	if (png != NULL) {
		png_destroy_read_struct(&png,
		    info ? &info : (png_infopp)0, (png_infopp)0);
	}
	Free(pData);
	return (su); 
fail:
	if (png != NULL) {
		png_destroy_read_struct(&png,
		   info  ? &info : (png_infopp)0, (png_infopp)0);
	}
	Free(pData);
	if (su) {
		AG_SurfaceFree(su);
	}
	AG_Seek(ds, start, AG_SEEK_SET);
	return (NULL);
}

/* Convert to one of the supported PNG depths. */
static __inline__ int
GetSupportedPNGDepth(int bitsPerPixel)
{
	switch (bitsPerPixel) {
	case 1:
		return (1);
	case 2:
		return (2);
	case 3:
	case 4:
		return (4);
	case 5:
	case 6:
	case 7:
	case 8:
		return (8);
	default:
		return (16);
	}
}

/* Save a surface to a PNG image file. */
int
AG_SurfaceExportPNG(const AG_Surface *su, const char *path)
{
	FILE *f;
	png_structp png;
	png_infop info;
	int pngDepth, colorType;
	png_colorp pngPal = NULL;
	png_bytep pSrc;
	AG_Surface *suPNG = NULL;
	AG_PixelFormat *pfPNG;
	int i, nPasses, y;

	if ((f = fopen(path, "wb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		return (-1);
	}
	if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	    NULL, NULL, NULL)) == NULL) {
		AG_SetError("libpng png_create_write_struct() failed");
		goto fail_close;
	}
	if ((info = png_create_info_struct(png)) == NULL) {
		AG_SetError("libpng png_create_info_struct() failed");
		png_destroy_write_struct(&png, NULL);
		goto fail_close;
	}

	png_init_io(png, f);

	/* Convert surface to one of the supported PNG formats. */
	pngDepth = GetSupportedPNGDepth(su->format->BitsPerPixel);
	if (su->format->palette != NULL) {
		colorType = PNG_COLOR_TYPE_PALETTE;
		pfPNG = AG_PixelFormatIndexed(pngDepth);
	} else {
		if (su->format->Amask != 0) {
			colorType = PNG_COLOR_TYPE_RGBA;
			pfPNG = AG_PixelFormatRGBA(pngDepth,
			    0xff000000,
			    0x00ff0000,
			    0x0000ff00,
			    0x000000ff);
		} else {
			colorType = PNG_COLOR_TYPE_RGB;
			pfPNG = AG_PixelFormatRGB(pngDepth,
			    0xff00000,
			    0x00ff0000,
			    0x0000ff00);
		}
	}
	if ((suPNG = AG_SurfaceConvert(su, pfPNG)) == NULL)
		goto fail;

	png_set_IHDR(png, info,
	    suPNG->w, suPNG->h, suPNG->format->BitsPerPixel,
	    colorType,
	    (agPNGInterlacing ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE),
	    PNG_COMPRESSION_TYPE_BASE,
	    PNG_FILTER_TYPE_BASE);

	/* Write PLTE chunk if color-index mode. */
	if (colorType == PNG_COLOR_TYPE_PALETTE) {
		AG_Palette *pal = suPNG->format->palette;

		pngPal = (png_colorp)TryMalloc(pal->nColors*sizeof(png_color));
		if (pngPal == NULL) {
			goto fail;
		}
		for (i = 0; i < pal->nColors; i++) {
			pngPal[i].red = pal->colors[i].r;
			pngPal[i].green = pal->colors[i].g;
			pngPal[i].blue = pal->colors[i].b;
		}
		png_set_PLTE(png, info, pngPal, pal->nColors);
	}

	png_write_info(png, info);
	nPasses = png_set_interlace_handling(png);
	for (i = 0; i < nPasses; i++) {
		pSrc = (png_bytep)suPNG->pixels;
		for (y = 0; y < suPNG->h; y++) {
			png_write_row(png, pSrc);
			pSrc += suPNG->pitch;
		}
	}

	png_write_end(png, info);
	png_destroy_write_struct(&png, NULL);
	AG_SurfaceFree(suPNG);
	Free(pngPal);
	fclose(f);
	return (0);
fail:
	if (suPNG != NULL) { AG_SurfaceFree(suPNG); }
	png_destroy_write_struct(&png, NULL);
	Free(pngPal);
fail_close:
	fclose(f);
	return (-1);
}

#else /* !HAVE_PNG */

AG_Surface *
AG_SurfaceFromPNG(const char *path)
{
	AG_SetError(_("Agar not compiled with PNG support"));
	return (NULL);
}
int
AG_SurfaceExportPNG(const AG_Surface *su, const char *path)
{
	AG_SetError(_("Agar not compiled with PNG support"));
	return (-1);
}
AG_Surface *
AG_ReadSurfaceFromPNG(AG_DataSource *ds)
{
	AG_SetError(_("Agar not compiled with PNG support"));
	return (NULL);
}

#endif /* HAVE_PNG */

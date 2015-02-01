/*
 * Copyright (c) 2010-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/surface.h>

#include <agar/config/have_png.h>
#if defined(HAVE_PNG)
#include <agar/config/have_libpng14.h>

#ifdef macintosh
# define MACOS
#endif
#include <png.h>

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
	AG_Read(ds, buf, size);
}

/* Load a surface from PNG image data. */
AG_Surface *
AG_ReadSurfaceFromPNG(AG_DataSource *ds)
{
	AG_Surface *volatile su = NULL;
	png_structp png;
	png_infop info;
	png_uint_32 width, height;
	int depth, colorType, intlaceType, channels, start, row;
	Uint32 Rmask = 0, Gmask = 0, Bmask = 0, Amask = 0;
	png_bytep *volatile pData = NULL;
	volatile int colorKey = -1;
	png_color_16 *transColor;

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

	png_read_info(png, info);
	png_get_IHDR(png, info,
	    &width, &height, &depth,
	    &colorType, &intlaceType,
	    NULL, NULL);

	png_set_strip_16(png);
	png_set_packing(png);
	png_set_expand(png);
	png_set_tRNS_to_alpha(png);

	/* Read transparency information. */
	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
	        int num_trans;
		Uint8 *trans;
		png_get_tRNS(png, info, &trans, &num_trans, &transColor);
	}

	/* Expand grayscale to 24-bit RGB */
	if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	/* Update png_info structure per our requirements. */
	png_read_update_info(png, info);

	png_get_IHDR(png, info, &width, &height, &depth,
	    &colorType, &intlaceType, NULL, NULL);

#ifdef HAVE_LIBPNG14
	channels = (int)png_get_channels(png, info);
#else
	channels = info->channels;
#endif

#if AG_BYTEORDER == AG_BIG_ENDIAN
	{
		int s = (channels == 4) ? 0 : 8;
		Rmask = 0xff000000 >> s;
		Gmask = 0x00ff0000 >> s;
		Bmask = 0x0000ff00 >> s;
		Amask = 0x000000ff >> s;
	}
#else
	Rmask = 0x000000ff;
	Gmask = 0x0000ff00;
	Bmask = 0x00ff0000;
	Amask = (channels == 4) ? 0xff000000 : 0;
#endif
	if ((su = AG_SurfaceRGBA(width, height, depth*channels, 0,
	    Rmask, Gmask, Bmask, Amask)) == NULL)
		goto fail;

	if (colorKey != -1) {
		colorKey = AG_MapPixelRGB(su->format,
		    (Uint8)transColor->red,
		    (Uint8)transColor->green,
		    (Uint8)transColor->blue);
	        AG_SurfaceSetColorKey(su, AG_SRCCOLORKEY, colorKey);
	}

	/* Read image data */
	if ((pData = TryMalloc(sizeof(png_bytep)*height)) == NULL) {
		goto fail;
	}
	for (row = 0; row < (int)height; row++) {
		pData[row] = (png_bytep)(Uint8 *)su->pixels + row*su->pitch;
	}
	png_read_image(png, pData);

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

/* Save a surface to a PNG image file. */
int
AG_SurfaceExportPNG(const AG_Surface *su, const char *path, Uint flags)
{
	FILE *f;
	png_structp png;
	png_infop info;
	int pngDepth, pngType;
	png_colorp pngPal = NULL;
	png_color_8 sig_bit;
	png_byte ** rows = NULL;
	int i, x, y;
	Uint8 *pSrc;

	if ((f = fopen(path, "wb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		return (-1);
	}
	if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	    NULL, NULL, NULL)) == NULL) {
		AG_SetError("png_create_write_struct() failed");
		fclose(f);
		return (-1);
	}
	if ((info = png_create_info_struct(png)) == NULL) {
		AG_SetError("png_create_info_struct() failed");
		goto fail;
	}

	if (setjmp(png_jmpbuf(png))) {
		AG_SetError("png_init_io() failed");
		goto fail;
	}
	png_init_io(png, f);

	if (su->format->palette != NULL) {
		pngType = PNG_COLOR_TYPE_PALETTE;

		if (su->format->palette->nColors > 16)     { pngDepth = 8; }
		else if (su->format->palette->nColors > 4) { pngDepth = 4; }
		else if (su->format->palette->nColors > 2) { pngDepth = 2; }
		else					   { pngDepth = 1; }
	} else {
		if (su->format->Amask != 0) {
			pngType = PNG_COLOR_TYPE_RGB_ALPHA;
		} else {
			pngType = PNG_COLOR_TYPE_RGB;
		}
		pngDepth = 8;
	}
	if (setjmp(png_jmpbuf(png))) {
		AG_SetError("png_write_info() failed");
		goto fail;
	}

	png_set_IHDR(png, info,
	    su->w, su->h, pngDepth, pngType,
	    (flags & AG_EXPORT_PNG_ADAM7) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
	    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* Write PLTE chunk if color-index mode. */
	if (pngType == PNG_COLOR_TYPE_PALETTE) {
		AG_Palette *pal = su->format->palette;

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

	if ((rows = png_malloc(png, su->h*sizeof(png_byte *))) == NULL) {
		AG_SetError("png_malloc rows");
		goto fail;
	}
	pSrc = (Uint8 *)su->pixels;
	if (pngType == PNG_COLOR_TYPE_RGB_ALPHA) {
		for (y = 0; y < su->h; y++) {
			png_byte *row;
		
			if ((row = png_malloc(png, su->w*4)) == NULL) {
				for (y--; y >= 0; y--) { png_free(png, rows[y]); }
				png_free(png, rows);
				AG_SetError("png_malloc row");
				goto fail;
			}
			rows[y] = row;
			for (x = 0; x < su->w; x++) {
				AG_Color C;
	
				C = AG_GetColorRGBA(AG_GET_PIXEL(su,pSrc),
				    su->format);
				*row++ = C.r;
				*row++ = C.g;
				*row++ = C.b;
				*row++ = C.a;
				pSrc += su->format->BytesPerPixel;
			}
			pSrc += su->padding;
		}
	} else {
		for (y = 0; y < su->h; y++) {
			png_byte *row;
		
			if ((row = png_malloc(png, su->w*4)) == NULL) {
				for (y--; y >= 0; y--) { png_free(png, rows[y]); }
				png_free(png, rows);
				AG_SetError("png_malloc row");
				goto fail;
			}
			rows[y] = row;
			for (x = 0; x < su->w; x++) {
				AG_Color C;
	
				C = AG_GetColorRGB(AG_GET_PIXEL(su,pSrc),
				    su->format);
				*row++ = C.r;
				*row++ = C.g;
				*row++ = C.b;
				pSrc += su->format->BytesPerPixel;
			}
			for (x = 0; x < su->w; x++) {
				*row++ = 0;
			}
			pSrc += su->padding;
		}
	}

	png_write_info(png, info);

	if (pngType & PNG_COLOR_MASK_COLOR) {
		sig_bit.red = pngDepth;
		sig_bit.green = pngDepth;
		sig_bit.blue = pngDepth;
	} else {
		sig_bit.gray = pngDepth;
	}
	if (pngType & PNG_COLOR_MASK_ALPHA) {
		sig_bit.alpha = pngDepth;
	}
	png_set_sBIT(png, info, &sig_bit);
	
	if (setjmp(png_jmpbuf(png))) {
		AG_SetError("png_write_image() failed");
		for (y = 0; y < su->h; y++) { png_free(png, rows[y]); }
		png_free(png, rows);
		goto fail;
	}
	png_write_image(png, rows);
	png_write_end(png, info);

	for (y = 0; y < su->h; y++) { png_free(png, rows[y]); }
	png_free(png, rows);
	png_destroy_write_struct(&png, &info);
	Free(pngPal);
	fclose(f);
	return (0);
fail:
	png_destroy_write_struct(&png, NULL);
	Free(pngPal);
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
AG_SurfaceExportPNG(const AG_Surface *su, const char *path, Uint flags)
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

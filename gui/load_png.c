/*
 * Copyright (c) 2010-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Loader for PNG images via libpng.
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

/* Load a single surface from a PNG file. */
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

/*
 * Construct an animated surface from a series of PNG files.
 *
 * The pattern must be a printf(3)-like format string for a single integer
 * argument for the frame number. E.g., "%08d.jpg" loads "00000001.jpg",
 * "00000002.jpg", etc.
 *
 * first is the index of the first frame. last is the last frame or -1.
 * When last != -1, fail if a frame is not found (otherwise, keep loading
 * frames as long as the files are found and break out successfully).
 */
AG_Surface *
AG_SurfaceFromPNGs(const char *pattern, int first, int last,
    AG_AnimDispose afDispose, Uint afDelay, Uint afFlags)
{
	AG_Surface *Sanim = NULL;
	char path[AG_PATHNAME_MAX];
	int i;

	for (i = first; ; i++) {
		AG_Surface *Sframe;

		if (last != -1 && i == last)
			break;

		Snprintf(path, sizeof(path), pattern, i);
		if (!AG_FileExists(path)) {
			if (i == 0) {
				continue;
			} else {
				if (last != -1) {
					goto fail;
				}
				break;
			}
		}
		if ((Sframe = AG_SurfaceFromPNG(path)) == NULL) {
			break;
		}
		if (Sanim == NULL) {
			Sanim = AG_SurfaceNew(&Sframe->format, Sframe->w, Sframe->h,
			    (Sframe->flags & (AG_SURFACE_COLORKEY|AG_SURFACE_ALPHA))
			    | AG_SURFACE_ANIMATED);
		}
		if (AG_SurfaceAddFrame(Sanim, Sframe, NULL,
		    afDispose, afDelay, afFlags) == -1) {
			goto fail;
		}
		AG_SurfaceFree(Sframe);
	}
	return (Sanim);
fail:
	AG_SurfaceFree(Sanim);
	return (NULL);
}

static void
AG_PNG_ReadData(png_structp png, png_bytep buf, png_size_t size)
{
	AG_DataSource *ds = (AG_DataSource *)png_get_io_ptr(png);

	(void)AG_Read(ds, buf, size);
}

/* Load a surface from PNG image data. */
AG_Surface *
AG_ReadSurfaceFromPNG(AG_DataSource *ds)
{
	AG_Surface *S = NULL;
	png_bytep *volatile pData = NULL;
	png_structp png;
	png_infop info;
	png_uint_32 width, height;
	Uint32 Rmask=0, Gmask=0, Bmask=0, Amask=0;
#ifdef AG_DEBUG
	AG_Offset start = AG_Tell(ds);
#endif
	int depth, colorType, intlaceType, channels, row;

	if ((png = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL))
	    == NULL) {
		AG_SetErrorS("Out of memory (libpng)");
		goto fail;
	}
	if ((info = png_create_info_struct(png)) == NULL) {
		AG_SetErrorS("png_create_info_struct() failed");
		goto fail;
	}
	png_set_read_fn(png, ds, AG_PNG_ReadData);
	png_read_info(png, info);
	png_get_IHDR(png, info, &width,&height, &depth,
	    &colorType, &intlaceType, NULL, NULL);

#if AG_MODEL != AG_LARGE
	png_set_strip_16(png);
#endif
#ifdef PNG_READ_EXPAND_SUPPORTED
	if (colorType == PNG_COLOR_TYPE_GRAY ||
	    colorType == PNG_COLOR_TYPE_GA)
		png_set_expand_gray_1_2_4_to_8(png);
#endif
	if (png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_color_16 *tc;
		Uint8 *trans;
		AG_Pixel colorkey;
	        int nTrans;

		png_get_tRNS(png, info, &trans, &nTrans, &tc);
		colorkey = AG_MapPixel_RGB16(&S->format,
		    tc->red, tc->green, tc->blue);
#if AG_MODEL == AG_LARGE
		Debug(NULL, "PNG transparent colorkey: 0x%llx\n",
		    (unsigned long long)colorkey);
#else
		Debug(NULL, "PNG transparent colorkey: 0x%x\n", colorkey);
#endif
	        AG_SurfaceSetColorKey(S, AG_SURFACE_COLORKEY, colorkey);
	}

	/* Update png_info structure per our requirements. */
	png_read_update_info(png, info);

	png_get_IHDR(png, info, &width, &height, &depth,
	    &colorType, &intlaceType, NULL, NULL);
#ifdef HAVE_LIBPNG14
	channels = (int)png_get_channels(png, info);
#else
	channels = info->channels;
#endif
	switch (colorType) {
	case PNG_COLOR_TYPE_PALETTE:
		S = AG_SurfaceIndexed(width, height, depth, 0);
		break;
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		S = AG_SurfaceGrayscale(width, height, depth, 0);
		break;
	case PNG_COLOR_TYPE_RGB:
	case PNG_COLOR_TYPE_RGB_ALPHA:
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
		S = AG_SurfaceRGBA(width, height, depth*channels, 0,
		    Rmask, Gmask, Bmask, Amask);
		break;
	}
	if (S == NULL)
		goto fail;
	
	if ((pData = TryMalloc(sizeof(png_bytep)*height)) == NULL) {
		goto fail;
	}
	for (row = 0; row < (int)height; row++)
		pData[row] = (png_bytep)S->pixels + row*S->pitch;

	Debug(NULL, "PNG image (%ux%u; %d-bpp %s at 0x%lx (->%p) via libpng (%s)\n",
	    width, height, depth*channels, agSurfaceModeNames[S->format.mode],
	    (long)start, S, PNG_LIBPNG_VER_STRING);

	png_read_image(png, pData);

	if (S->format.mode == AG_SURFACE_INDEXED) {
		AG_Palette *pal = S->format.palette;
		int i;
#ifdef HAVE_LIBPNG14
		int nColors;
		png_colorp plte;

		if (png_get_PLTE(png, info, &plte, &nColors)) {
			if (nColors > pal->nColors) {
				AG_SetErrorS("Too many PLTE colors");
				goto fail;
			}
			pal->nColors = nColors; 
			for (i = 0; i < nColors; i++) {
				pal->colors[i].b = AG_8toH(plte[i].blue);
				pal->colors[i].g = AG_8toH(plte[i].green);
				pal->colors[i].r = AG_8toH(plte[i].red);
				pal->colors[i].a = AG_OPAQUE;
			}
		}
#else /* !HAVE_LIBPNG14 */
		if (info->num_palette > 0) {
			pal->nColors = info->num_palette; 
			for (i = 0; i < info->num_palette; i++) {
				pal->colors[i].b = AG_8toH(info->palette[i].blue);
				pal->colors[i].g = AG_8toH(info->palette[i].green);
				pal->colors[i].r = AG_8toH(info->palette[i].red);
				pal->colors[i].a = AG_OPAQUE;
			}
		}
#endif
	}

	png_destroy_read_struct(&png,
	    info ? &info : (png_infopp)0, (png_infopp)0);

	free(pData);
	return (S); 
fail:
	if (png) {
		png_destroy_read_struct(&png,
		   info ? &info : (png_infopp)0, (png_infopp)0);
	}
	Free(pData);
	if (S) {
		AG_SurfaceFree(S);
	}
	return (NULL);
}

/* Export a surface to a PNG image file. */
int
AG_SurfaceExportPNG(const AG_Surface *S, const char *path, Uint flags)
{
	FILE *f;
	png_structp png;
	png_infop info;
	int depth, BytesPerPixel, SrcBytesPerPixel, pngType;
	png_colorp plte = NULL;
	png_color_8 sig_bit;
	png_byte **rows = NULL, *row;
	AG_Palette *pal = S->format.palette;
	int i, x,y, w,h;
	Uint8 *pSrc;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE) {
		Debug(NULL, "Surface <%p>: Export to PNG via libpng (%s)\n", S,
		    PNG_LIBPNG_VER_STRING);
	}
#endif
	if ((f = fopen(path, "wb")) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		return (-1);
	}
	if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	    NULL, NULL, NULL)) == NULL) {
		AG_SetErrorS("png_create_write_struct() failed");
		fclose(f);
		return (-1);
	}
	if ((info = png_create_info_struct(png)) == NULL) {
		AG_SetErrorS("png_create_info_struct() failed");
		goto fail;
	}

	if (setjmp(png_jmpbuf(png))) {
		AG_SetErrorS("png_init_io() failed");
		goto fail;
	}
	png_init_io(png, f);

	if (S->format.mode == AG_SURFACE_INDEXED) {
		pngType = PNG_COLOR_TYPE_PALETTE;
#ifdef AG_DEBUG
		if (pal == NULL) { AG_FatalError("pal=NULL"); }
#endif
		if (S->format.BitsPerPixel > 8) {
			AG_SetErrorS("Indexed PNG images must be <= 8 bpp");
			goto fail;
		}
		if (pal->nColors > 16)     { depth = 8; }
		else if (pal->nColors > 4) { depth = 4; }
		else if (pal->nColors > 2) { depth = 2; }
		else                       { depth = 1; }
	} else {
		if (S->format.Amask != 0) {
			pngType = PNG_COLOR_TYPE_RGB_ALPHA;
		} else {
			pngType = PNG_COLOR_TYPE_RGB;
		
		}
		if (S->format.BitsPerPixel == 64 ||
		    S->format.BitsPerPixel == 48) {
#if AG_MODEL == AG_LARGE
			depth = 16;
#else
			AG_SetErrorS("48- and 64-bpp modes require AG_LARGE");
			goto fail;
#endif
		} else {
			depth = 8;
		}
	}
	w = S->w;
	h = S->h;

	if (setjmp(png_jmpbuf(png))) {
		AG_SetErrorS("png_write_info() failed");
		goto fail;
	}
	png_set_IHDR(png, info,
	    w, h, depth, pngType,
	    (flags & AG_EXPORT_PNG_ADAM7) ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
	    PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if ((rows = png_malloc(png, h*sizeof(png_byte *))) == NULL) {
		AG_SetErrorS("png_malloc rows");
		goto fail;
	}
	/* TODO: zero-copy optimized cases (i.e., surfaces in RGBA or RGB). */
	
	switch (pngType) {
	case PNG_COLOR_TYPE_RGB_ALPHA:
		BytesPerPixel = ((depth >> 3) << 2) * sizeof(png_byte);
		SrcBytesPerPixel = S->format.BytesPerPixel;
		Debug(NULL, "Saving %ux%u %dbpp surface <%p> to %s [RGBA%d]\n",
		    w,h, S->format.BitsPerPixel, S, path, depth);
		for (y=0, pSrc=S->pixels; y < h; y++) {
			if ((row = png_malloc(png, w*BytesPerPixel)) == NULL) {
				goto fail_row;
			}
			rows[y] = row;
			switch (depth) {
			case 16:
				for (x = 0; x < w; x++) {
					AG_GetColor_RGBA16(
					    AG_SurfaceGet_At(S,pSrc),
                                            &S->format,
					    (Uint16 *)&row[0],	/* <- r */
					    (Uint16 *)&row[2],	/* <- g */
					    (Uint16 *)&row[4],	/* <- b */
					    (Uint16 *)&row[6]);	/* <- a */
					row += 8;
					pSrc += SrcBytesPerPixel;
				}
				break;
			case 8:
			default:
				for (x = 0; x < w; x++) {
					AG_GetColor_RGBA8(
					    AG_SurfaceGet_At(S,pSrc),
					    &S->format,
					    &row[0],	/* <- r */
					    &row[1],	/* <- g */
					    &row[2],	/* <- b */
					    &row[3]);	/* <- a */
					row += 4;
					pSrc += SrcBytesPerPixel;
				}
			}
			pSrc += S->padding;
		}
		break;
	case PNG_COLOR_TYPE_RGB:
		BytesPerPixel = 3 * (depth >> 3) * sizeof(png_byte);
		SrcBytesPerPixel = S->format.BytesPerPixel;
		Debug(NULL, "Saving %ux%u %dbpp surface <%p> to %s [RGB%d]\n",
		    w,h, S->format.BitsPerPixel, S, path, depth);
		for (y=0, pSrc=S->pixels; y < h; y++) {
			if ((row = png_malloc(png, w*BytesPerPixel)) == NULL) {
				goto fail_row;
			}
			rows[y] = row;
			switch (depth) {
			case 16:
				for (x = 0; x < w; x++) {
					AG_GetColor_RGB16(
					    AG_SurfaceGet_At(S,pSrc),
					    &S->format,
					    (Uint16 *)&row[0],	/* <- r */
					    (Uint16 *)&row[2],	/* <- g */
					    (Uint16 *)&row[4]);	/* <- b */
					row += 6;
					pSrc += SrcBytesPerPixel;
				}
				break;
			case 8:
			default:
				for (x = 0; x < w; x++) {
					AG_GetColor_RGB8(
					    AG_SurfaceGet_At(S,pSrc),
					    &S->format,
					    &row[0],	/* <- r */
					    &row[1],	/* <- g */
					    &row[2]);	/* <- b */
					row += 3;
					pSrc += SrcBytesPerPixel;
				}
				break;
			}
			pSrc += S->padding;
		}
		break;
	case PNG_COLOR_TYPE_PALETTE:				/* Use PLTE */
		Debug(NULL,
		    "Saving %ux%u %dbpp surface %p to %s [%u-color PLTE]\n",
		    w,h, S->format.BitsPerPixel, S, path, pal->nColors);
		plte = (png_colorp)TryMalloc(pal->nColors * sizeof(png_color));
		if (plte == NULL) {
			goto fail;
		}
		for (i = 0; i < pal->nColors; i++) {
			plte[i].red   = pal->colors[i].r;
			plte[i].green = pal->colors[i].g;
			plte[i].blue  = pal->colors[i].b;
		}
		png_set_PLTE(png, info, plte, pal->nColors);

		for (y=0, pSrc=S->pixels; y < h; y++) {
			png_byte *row;
			size_t len;

			if (S->format.BitsPerPixel < 8) {
				len = w*sizeof(png_byte)/S->format.PixelsPerByte; 
				/* TODO zero copy */
				if ((row = png_malloc(png, len)) == NULL) {
					for (y--; y >= 0; y--) { png_free(png, rows[y]); }
					png_free(png, rows);
					AG_SetErrorS("png_malloc row");
					goto fail;
				}
				rows[y] = row;
				memcpy(row, pSrc, len);
				pSrc += len;
			} else {
				/* TODO zero copy */
				len = w*sizeof(png_byte);
				row = png_malloc(png, w*sizeof(png_byte));
				if (row == NULL) {
					for (y--; y >= 0; y--) { png_free(png, rows[y]); }
					png_free(png, rows);
					AG_SetErrorS("png_malloc row");
					goto fail;
				}
				rows[y] = row;
				memcpy(row, pSrc, w*sizeof(png_byte));
				pSrc += w*sizeof(png_byte);
			}
		}
		break;
	default:
		AG_SetErrorS("Bad PNG type");
		goto fail;
	}

	png_write_info(png, info);

	if (pngType & PNG_COLOR_MASK_COLOR) {
		sig_bit.red   = S->format.BitsPerPixel;
		sig_bit.green = S->format.BitsPerPixel;
		sig_bit.blue  = S->format.BitsPerPixel;
	} else {
		sig_bit.gray  = S->format.BitsPerPixel;
	}
	if (pngType & PNG_COLOR_MASK_ALPHA) {
		sig_bit.alpha = S->format.BitsPerPixel;
	}
	png_set_sBIT(png, info, &sig_bit);

	if (setjmp(png_jmpbuf(png))) {
		AG_SetErrorS("png_write_image() failed");
		for (y = 0; y < h; y++) { png_free(png, rows[y]); }
		png_free(png, rows);
		goto fail;
	}
	png_write_image(png, rows);
	png_write_end(png, info);

	for (y = 0; y < h; y++) { png_free(png, rows[y]); }
	png_free(png, rows);
	png_destroy_write_struct(&png, &info);
	Free(plte);
	fclose(f);
	return (0);
fail_row:
	for (y--; y >= 0; y--) { png_free(png, rows[y]); }
	png_free(png, rows);
	AG_SetErrorS("png_malloc row");
fail:
	png_destroy_write_struct(&png, NULL);
	Free(plte);
	fclose(f);
	return (-1);
}

#else /* !HAVE_PNG */

AG_Surface *
AG_SurfaceFromPNG(const char *path)
{
	AG_SetErrorS(_("No PNG support (need libpng)"));
	return (NULL);
}
AG_Surface *
AG_SurfaceFromPNGs(const char *pattern, int first, int last,
    AG_AnimDispose afDispose, Uint afDelay, Uint afFlags)
{
	AG_SetErrorS(_("No PNG support (need libpng)"));
	return (NULL);
}
int
AG_SurfaceExportPNG(const AG_Surface *su, const char *path, Uint flags)
{
	AG_SetErrorS(_("No PNG support (need libpng)"));
	return (-1);
}
AG_Surface *
AG_ReadSurfaceFromPNG(AG_DataSource *ds)
{
	AG_SetErrorS(_("No PNG support (need libpng)"));
	return (NULL);
}

#endif /* HAVE_PNG */

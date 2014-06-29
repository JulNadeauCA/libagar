/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Serialization functions for graphic surfaces.
 */

#include <agar/core/core.h>
#include <agar/core/load_version.h>
#include <agar/gui/gui.h>
#include <agar/gui/surface.h>
#include <agar/gui/load_surface.h>

const AG_Version agSurfaceVer = { 0, 0 };

enum {
	RAW_ENCODING,
	RLE_ENCODING,
	PNG_ENCODING,
	JPEG_ENCODING,
	TIFF_ENCODING
};

void
AG_WriteSurface(AG_DataSource *ds, AG_Surface *su)
{
	Uint8 *src;
	int x, y;

	AG_WriteVersion(ds, "AG_Surface", &agSurfaceVer);
	AG_WriteUint32(ds, RAW_ENCODING);

	AG_WriteUint32(ds, su->flags&(AG_SRCCOLORKEY|AG_SRCALPHA));
	AG_WriteUint16(ds, su->w);
	AG_WriteUint16(ds, su->h);
	AG_WriteUint8(ds, su->format->BitsPerPixel);
	AG_WriteUint8(ds, 0);				/* TODO grayscale */
	AG_WriteUint32(ds, su->format->Rmask);
	AG_WriteUint32(ds, su->format->Gmask);
	AG_WriteUint32(ds, su->format->Bmask);
	AG_WriteUint32(ds, su->format->Amask);
	AG_WriteUint8(ds, su->format->alpha);
	AG_WriteUint32(ds, su->format->colorkey);
#if 0
	printf("saving %dx%dx%d bpp%s%s surface\n", su->w, su->h,
	    su->format->BitsPerPixel,
	    (su->flags & AG_SRCALPHA) ? " alpha" : "",
	    (su->flags & AG_SRCCOLORKEY) ? " colorkey" : "");
	printf("masks: %08x,%08x,%08x,%08x\n", su->format->Rmask,
	    su->format->Gmask, su->format->Bmask, su->format->Amask);
	printf("colorkey=%08x, alpha=%02x\n", su->format->colorkey,
	    su->format->alpha);
#endif
	if (su->format->BitsPerPixel == 8) {
		int i;

		AG_WriteUint32(ds, su->format->palette->nColors);
		for (i = 0; i < su->format->palette->nColors; i++) {
			AG_WriteUint8(ds, su->format->palette->colors[i].r);
			AG_WriteUint8(ds, su->format->palette->colors[i].g);
			AG_WriteUint8(ds, su->format->palette->colors[i].b);
		}
	}

	src = (Uint8 *)su->pixels;
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			switch (su->format->BytesPerPixel) {
			case 4:
				AG_WriteUint32(ds, *(Uint32 *)src);
				break;
			case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
				AG_WriteUint32(ds,
				    (src[0] << 16) +
				    (src[1] << 8) +
				     src[2]);
#else
				AG_WriteUint32(ds,
				    (src[2] << 16) +
				    (src[1] << 8) +
				     src[0]);
#endif
				break;
			case 2:
				AG_WriteUint16(ds, *(Uint16 *)src);
				break;
			case 1:
				/* XXX do one big write */
				AG_WriteUint8(ds, *src);
				break;
			}
			src += su->format->BytesPerPixel;
		}
	}
}

AG_Surface *
AG_ReadSurface(AG_DataSource *ds)
{
	AG_Surface *su;
	Uint32 encoding;
	Uint32 flags;
	Uint16 w, h;
	Uint8 depth, grayscale;
	Uint32 Rmask, Gmask, Bmask, Amask;
	Uint8 *dst;
	int x, y;

	if (AG_ReadVersion(ds, "AG_Surface", &agSurfaceVer, NULL) != 0)
		return (NULL);

	encoding = AG_ReadUint32(ds);
	if (encoding != RAW_ENCODING) {
		AG_SetError(_("Unsupported surface encoding: %u"),
		    (unsigned int)encoding);
		return (NULL);
	}
	flags = AG_ReadUint32(ds);
	w = AG_ReadUint16(ds);
	h = AG_ReadUint16(ds);
	depth = AG_ReadUint8(ds);
	grayscale = AG_ReadUint8(ds);
	Rmask = AG_ReadUint32(ds);
	Gmask = AG_ReadUint32(ds);
	Bmask = AG_ReadUint32(ds);
	Amask = AG_ReadUint32(ds);

	su = AG_SurfaceRGBA(w, h, depth, flags, Rmask,Gmask,Bmask,Amask);
	su->format->alpha = AG_ReadUint8(ds);
	su->format->colorkey = AG_ReadUint32(ds);
#if 0	
	printf("loading %dx%dx%d bpp%s%s%s surface\n", w, h, depth,
	    grayscale ? " grayscale" : "",
	    (flags & AG_SRCALPHA) ? " alpha" : "",
	    (flags & AG_SRCCOLORKEY) ? " colorkey" : "");
	printf("masks: %08x,%08x,%08x,%08x\n", Rmask, Gmask, Bmask, Amask);
	printf("colorkey=%08x, alpha=%02x\n", su->format->colorkey,
	    su->format->alpha);
#endif
	if (depth == 8) {
		AG_Color *colors;
		Uint32 i, ncolors;

		ncolors = AG_ReadUint32(ds);
		colors = Malloc(ncolors*sizeof(AG_Color));

		if (grayscale) {
			for (i = 0; i < ncolors; i++) {
				colors[i].r = i;
				colors[i].g = i;
				colors[i].b = i;
			}
		} else {
			for (i = 0; i < ncolors; i++) {
				colors[i].r = AG_ReadUint8(ds);
				colors[i].g = AG_ReadUint8(ds);
				colors[i].b = AG_ReadUint8(ds);
			}
		}
		if (AG_SurfaceSetPalette(su, colors, 0, ncolors) == -1) {
			/* XXX leak */
			return (NULL);
		}
		Free(colors);
	}
	
	dst = (Uint8 *)su->pixels;
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			switch (su->format->BytesPerPixel) {
			case 4:
				*(Uint32 *)dst = AG_ReadUint32(ds);
				break;
			case 3:
				{
					Uint32 c = AG_ReadUint32(ds);
#if AG_BYTEORDER == AG_BIG_ENDIAN
					dst[0] = (c >> 16) & 0xff;
					dst[1] = (c >> 8) & 0xff;
					dst[2] = c & 0xff;
#else
					dst[2] = (c >> 16) & 0xff;
					dst[1] = (c >> 8) & 0xff;
					dst[0] = c & 0xff;
#endif
				}
				break;
			case 2:
				*(Uint16 *)dst = AG_ReadUint16(ds);
				break;
			case 1:
				/* XXX do one big read */
				*dst = AG_ReadUint8(ds);
				break;
			}
			dst += su->format->BytesPerPixel;
		}
	}
	return (su);
}

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

/*
 * Support for reading and writing Win32 image files in BMP format.
 */

#include <core/core.h>
#include "gui.h"

struct ag_bmp_header {
	char magic[2];
	Uint32 size;
	Uint16 resv[2];
	Uint32 offBits;
};

struct ag_bmp_info_header {
	Uint32 size;
	Sint32 w, h;
	Uint16 planes;
	Uint16 bitCount;
	enum {
		AG_BMP_RGB		= 0,
		AG_BMP_RLE8		= 1,
		AG_BMP_RLE4		= 2,
		AG_BMP_BITFIELDS	= 3
	} encoding;
	Uint32 sizeImage;
	Sint32 XPelsPerMeter, YPelsPerMeter;
	Uint32 clrUsed, clrImportant;
};

/* Load a surface from a given bmp file. */
AG_Surface *
AG_SurfaceFromBMP(const char *path)
{
	AG_DataSource *ds;
	AG_Surface *s;

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		return (NULL);
	}
	if ((s = AG_ReadSurfaceFromBMP(ds)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		AG_CloseFile(ds);
		return (NULL);
	}
	AG_CloseFile(ds);
	return (s);
}

/* Save a surface to a BMP file. */
int
AG_SurfaceExportBMP(const AG_Surface *su, const char *path)
{
	/* TODO */
	AG_SetError("Unimplemented");
	return (-1);
}

/*
 * Load surface contents from a Windows BMP file.
 *
 * Code for expanding 1bpp and 4bpp to 8bpp depth was shamefully
 * stolen from SDL.
 */
AG_Surface *
AG_ReadSurfaceFromBMP(AG_DataSource *ds)
{
	struct ag_bmp_header bh;
	struct ag_bmp_info_header bi;
	Uint32 Rmask = 0, Gmask = 0, Bmask = 0;
	AG_Surface *s;
	off_t offs;
	Uint8 *bits;
	int i, pitch, expandBpp = 0, pad;

	offs = AG_Tell(ds);
	if (AG_Read(ds, bh.magic, 1, 2) != AG_IO_SUCCESS ||
	    bh.magic[0] != 'B' || bh.magic[1] != 'M') {
		AG_SetError("Not a Windows BMP file (`%c%c')",
		bh.magic[0], bh.magic[1]);
		return (NULL);
	}

	/* Uses little-endian byte order */
	AG_SetByteOrder(ds, AG_BYTEORDER_LE);

	bh.size = AG_ReadUint32(ds);
	bh.resv[0] = AG_ReadUint16(ds);
	bh.resv[1] = AG_ReadUint16(ds);
	bh.offBits = AG_ReadUint32(ds);

	bi.size = AG_ReadUint32(ds);
	if (bi.size == 12) {
		bi.w = (Uint32)AG_ReadUint16(ds);
		bi.h = (Uint32)AG_ReadUint16(ds);
		bi.planes = AG_ReadUint16(ds);
		bi.bitCount = AG_ReadUint16(ds);
		bi.encoding = AG_BMP_RGB;
		bi.sizeImage = 0;
		bi.XPelsPerMeter = 0;
		bi.YPelsPerMeter = 0;
		bi.clrUsed = 0;
		bi.clrImportant = 0;
	} else {
		bi.w = AG_ReadUint32(ds);
		bi.h = AG_ReadUint32(ds);
		bi.planes = AG_ReadUint16(ds);
		bi.bitCount = AG_ReadUint16(ds);
		bi.encoding = AG_ReadUint32(ds);
		bi.sizeImage = AG_ReadUint32(ds);
		bi.XPelsPerMeter = AG_ReadUint32(ds);
		bi.YPelsPerMeter = AG_ReadUint32(ds);
		bi.clrUsed = AG_ReadUint32(ds);
		bi.clrImportant = AG_ReadUint32(ds);
	}
	
	/* Will convert 1bpp/4bpp to 8bpp */
	if (bi.bitCount == 1 || bi.bitCount == 4) {
		expandBpp = bi.bitCount;
		bi.bitCount = 8;
	}

	switch (bi.encoding) {
	case AG_BMP_RGB:
		if (bh.offBits == (14+bi.size)) {
			switch (bi.bitCount) {
			case 15:
			case 16:
				Rmask = 0x7C00;
				Gmask = 0x03E0;
				Bmask = 0x001F;
				break;
			case 24:
#if AG_BYTEORDER == AG_BIG_ENDIAN
			        Rmask = 0x000000FF;
			        Gmask = 0x0000FF00;
			        Bmask = 0x00FF0000;
				break;
#endif
			case 32:
				Rmask = 0x00FF0000;
				Gmask = 0x0000FF00;
				Bmask = 0x000000FF;
				break;
			default:
				break;
			}
			break;
		}
		/* FALLTHROUGH */
	case AG_BMP_BITFIELDS:
		switch (bi.bitCount) {
		case 15:
		case 16:
		case 32:
			Rmask = AG_ReadUint32(ds);
			Gmask = AG_ReadUint32(ds);
			Bmask = AG_ReadUint32(ds);
			break;
		default:
			break;
		}
		break;
	default:
		AG_SetError("BMP compression unimplemented");
		return (NULL);
	}

	/* Allocate Agar surface. */
	if ((s = AG_SurfaceRGB(bi.w, bi.h, bi.bitCount, 0, Rmask, Gmask, Bmask))
	    == NULL)
		return (NULL);

	/* Read palette information. */
	if (s->format->palette != NULL) {
		if (bi.clrUsed == 0) {
			bi.clrUsed = (1 << bi.bitCount);
		}
		if (bi.size == 12) {
			for (i = 0; i < bi.clrUsed; i++) {
				s->format->palette->colors[i].b = AG_ReadUint8(ds);
				s->format->palette->colors[i].g = AG_ReadUint8(ds);
				s->format->palette->colors[i].r = AG_ReadUint8(ds);
			}	
		} else {
			for (i = 0; i < bi.clrUsed; i++) {
				s->format->palette->colors[i].b = AG_ReadUint8(ds);
				s->format->palette->colors[i].g = AG_ReadUint8(ds);
				s->format->palette->colors[i].r = AG_ReadUint8(ds);
				(void)AG_ReadUint8(ds); /* unused */
			}	
		}
		s->format->palette->nColors = bi.clrUsed;
	}

	if (AG_Seek(ds, offs+bh.offBits, AG_SEEK_SET) == -1) {
		goto fail;
	}
	bits = (Uint8 *)s->pixels + s->h*s->pitch;
	switch (expandBpp) {
	case 1:
		pitch = (bi.w + 7) >> 3;
		pad = (((pitch)%4) ? (4-((pitch)%4)) : 0);
		break;
	case 4:
		pitch = (bi.w + 1) >> 1;
		pad = (((pitch)%4) ? (4-((pitch)%4)) : 0);
		break;
	default:
		pad = ((s->pitch%4) ? (4-(s->pitch%4)) : 0);
		break;
	}
	while (bits > (Uint8 *)s->pixels) {
		bits -= s->pitch;
		switch (expandBpp) {
		case 1:
		case 4:
			{
				Uint8 px = 0;
				for (i = 0; i < s->w; i++) {
					if (i%(8/expandBpp) == 0) {
						px = AG_ReadUint8(ds);
					}
					*(bits+i) = (px >> (8 - expandBpp));
					px <<= expandBpp;
				}
			}
			break;
		default:
			if (AG_Read(ds, bits, 1, s->pitch) != AG_IO_SUCCESS) {
				goto fail;
			}
#if AG_BYTEORDER == AG_BIG_ENDIAN
			switch (bi.bitCount) {
			case 15:
			case 16:
				{
				        Uint16 *px = (Uint16 *)bits;
					for (i = 0; i < s->w; i++) {
					        px[i] = AG_Swap16(px[i]);
					}
					break;
				}
			case 32:
				{
					Uint32 *px = (Uint32 *)bits;
					for (i = 0; i < s->w; i++) {
					        px[i] = AG_Swap32(px[i]);
					}
					break;
				}
			}
#endif /* AG_BYTEORDER == AG_BIG_ENDIAN */
			break;
		}
		if (pad != 0 &&
		    AG_Seek(ds, pad, AG_SEEK_CUR) == -1)
			goto fail;
	}
	return (s);
fail:
	AG_SurfaceFree(s);
	return (NULL);
}

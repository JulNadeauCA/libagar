/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Serialization functions for uncompressed graphic surfaces.
 */
#include <agar/core/core.h>
#include <agar/core/load_version.h>
#include <agar/gui/gui.h>
#include <agar/gui/surface.h>
#include <agar/gui/load_surface.h>

const AG_Version agSurfaceVer = { 1, 0 };

void
AG_WritePalette(AG_DataSource *ds, const AG_Palette *pal)
{
	int i;

	AG_WriteUint16(ds, pal->nColors);
	for (i = 0; i < pal->nColors; i++)
		AG_WriteColor(ds, &pal->colors[i]);
}

int
AG_ReadPalette(AG_Palette *pal, AG_DataSource *ds)
{
	Uint16 nColors;
	int i;

	if ((nColors = AG_ReadUint16(ds)) != pal->nColors) {
		AG_SetErrorS("Bad color count");
		return (-1);
	}
	for (i = 0; i < pal->nColors; i++) {
		AG_ReadColor(&pal->colors[i], ds);
	}
	return (0);
}

void
AG_WriteSurface(AG_DataSource *ds, AG_Surface *s)
{
	const AG_PixelFormat *pf = &s->format;

	AG_WriteVersion(ds, "AG_Surface", &agSurfaceVer);
	AG_WriteUint16(ds, s->flags & AG_SAVED_SURFACE_FLAGS);
	AG_WriteUint16(ds, s->w);
	AG_WriteUint16(ds, s->h);
	AG_WriteUint8(ds, pf->mode);
	AG_WriteUint8(ds, pf->BitsPerPixel);

	switch (pf->mode) {
	case AG_SURFACE_PACKED:
#if AG_MODEL == AG_LARGE
		if (pf->Rmask > 0xffffffff ||
		    pf->Gmask > 0xffffffff ||
		    pf->Bmask > 0xffffffff ||
		    pf->Amask > 0xffffffff) {
			AG_WriteUint8(ds, 64);
			AG_WriteUint64(ds, pf->Rmask);
			AG_WriteUint64(ds, pf->Gmask);
			AG_WriteUint64(ds, pf->Bmask);
			AG_WriteUint64(ds, pf->Amask);
		} else
#else /* !LARGE */
		{
			AG_WriteUint8(ds, 32);
			AG_WriteUint32(ds, pf->Rmask);
			AG_WriteUint32(ds, pf->Gmask);
			AG_WriteUint32(ds, pf->Bmask);
			AG_WriteUint32(ds, pf->Amask);
		}
#endif
		break;
	case AG_SURFACE_INDEXED:
		AG_WritePalette(ds, pf->palette);
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_WriteUint8(ds, pf->graymode);
		break;
	}

	AG_WriteRect(ds, &s->clipRect);

#if AG_MODEL == AG_LARGE
	if (s->colorkey > 0xffffffff) {
		AG_WriteUint8(ds, 64);
		AG_WriteUint64(ds, s->colorkey);
	} else
#endif
	{
		AG_WriteUint8(ds, 32);
		AG_WriteUint32(ds, s->colorkey);
	}
#if AG_MODEL == AG_LARGE
	if (s->alpha > 0xff) {
		AG_WriteUint8(ds, 16);
		AG_WriteUint16(ds, s->alpha);
	} else
#endif
	{
		AG_WriteUint8(ds, 8);
		AG_WriteUint8(ds, s->alpha);
	}

	AG_WriteUint16(ds, s->pitch);
	AG_Write(ds, s->pixels, (s->pitch * s->h));

	/* TODO serialize animation frames */
	AG_WriteUint32(ds, 0); /* s->n */
}

AG_Surface *
AG_ReadSurface(AG_DataSource *ds)
{
	AG_PixelFormat pf;
	AG_Surface *s;
	AG_SurfaceMode mode;
	Uint w,h, flags;
	int BitsPerPixel;
	Uint8 size;

	if (AG_ReadVersion(ds, "AG_Surface", &agSurfaceVer, NULL) != 0) {
		return (NULL);
	}
	flags = (AG_ReadUint16(ds) & AG_SAVED_SURFACE_FLAGS);
	w = AG_ReadUint16(ds);
	h = AG_ReadUint16(ds);
	mode = pf.mode = (AG_SurfaceMode)AG_ReadUint8(ds);
	BitsPerPixel = AG_ReadUint8(ds);

	if (!AG_PixelFormatIsSupported(mode, BitsPerPixel)) {
		AG_SetErrorS("Bad mode/bpp combination");
		return (NULL);
	}
	switch (pf.mode) {
	case AG_SURFACE_PACKED:
		if ((size = AG_ReadUint8(ds)) == 64) {
#if AG_MODEL == AG_LARGE
			Uint64 Rmask = AG_ReadUint64(ds);
			Uint64 Gmask = AG_ReadUint64(ds);
			Uint64 Bmask = AG_ReadUint64(ds);
			Uint64 Amask = AG_ReadUint64(ds);

			AG_PixelFormatRGBA(&pf, BitsPerPixel,
			    Rmask, Gmask, Bmask, Amask);
#else
			AG_SetErrorS("64bpp surfaces require AG_LARGE");
			return (NULL);
#endif
		} else if (size == 32) {
			Uint32 Rmask = AG_ReadUint32(ds);
			Uint32 Gmask = AG_ReadUint32(ds);
			Uint32 Bmask = AG_ReadUint32(ds);
			Uint32 Amask = AG_ReadUint32(ds);

			AG_PixelFormatRGBA(&pf, BitsPerPixel,
			    Rmask, Gmask, Bmask, Amask);
		} else {
			AG_SetErrorS("Bad mask size");
			return (NULL);
		}
		break;
	case AG_SURFACE_INDEXED:
		AG_PixelFormatIndexed(&pf, BitsPerPixel);
		if (AG_ReadPalette(pf.palette, ds) == -1) {
			return (NULL);
		}
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_PixelFormatGrayscale(&pf, BitsPerPixel);
		pf.graymode = AG_ReadUint8(ds);
		break;
	}
	if ((s = AG_SurfaceNew(&pf, w,h, flags)) == NULL) {
		return (NULL);
	}
	AG_ReadRect(&s->clipRect, ds);

	if ((size = AG_ReadUint8(ds)) == 64) {
#if AG_MODEL == AG_LARGE
		s->colorkey = AG_ReadUint64(ds);
#else
		AG_SetErrorS("64bpp colorkey needs AG_LARGE");
		goto fail;
#endif
	} else if (size == 32) {
		s->colorkey = AG_ReadUint32(ds);
	} else {
		AG_SetErrorS("Bad colorkey size");
		goto fail;
	}

	if ((size = AG_ReadUint8(ds)) == 16) {
#if AG_MODEL == AG_LARGE
		s->alpha = AG_ReadUint16(ds);
#else
		s->alpha = AG_16to8(AG_ReadUint16(ds));
#endif
	} else if (size == 8) {
#if AG_MODEL == AG_LARGE
		s->alpha = AG_8to16(AG_ReadUint8(ds));
#else
		s->alpha = AG_ReadUint8(ds);
#endif
	} else {
		AG_SetErrorS("Bad alpha size");
		goto fail;
	}

	if (s->pitch != AG_ReadUint16(ds)) {
		AG_SetErrorS("Bad pitch");
		goto fail;
	}
	if (AG_Read(ds, s->pixels, (h * s->pitch)) != 0) {
		goto fail;
	}
	(void)AG_ReadUint32(ds); /* s->n; TODO */
	return (s);
fail:
	AG_SurfaceFree(s);
	return (NULL);
}

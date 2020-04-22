/*
 * Loader for Windows BMP image files.
 */
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
   Code to load and save surfaces in Windows BMP format.

   Why support BMP format?  Well, it's a native format for Windows, and
   most image processing programs can read and write it.  It would be nice
   to be able to have at least one image format that we can natively load
   and save, and since PNG is so complex that it would bloat the library,
   BMP is a good alternative.

   This code currently supports Win32 DIBs in uncompressed 8 and 24 bpp.
*/

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/surface.h>

#define SAVE_32BIT_BMP

/* Compression encodings for BMP files */
#ifndef BI_RGB
# define BI_RGB		0
# define BI_RLE8	1
# define BI_RLE4	2
# define BI_BITFIELDS	3
#endif

/* Logical color space values for BMP files */
#ifndef LCS_WINDOWS_COLOR_SPACE
# define LCS_WINDOWS_COLOR_SPACE    0x57696E20	/* "Win " */
#endif

/*
 * Scan a 32bpp RGB surface for a pixel with non-zero alpha.
 * If all pixels have alpha=0 then set every alpha component fully opaque.
 */
static void
CorrectAlphaChannel_32RGB(AG_Surface *S)
{
	int hasAlpha = 0;
#if AG_BYTEORDER == AG_BIG_ENDIAN
	const int alphaChannelOffset = 0;
#else
	const int alphaChannelOffset = 3;
#endif
	Uint8 *alpha = S->pixels + alphaChannelOffset;
	Uint8 *end = alpha + (S->h * S->pitch);

	while (alpha < end) {
		if (*alpha != 0) {
			hasAlpha = 1;
			break;
		}
		alpha += 4;
	}
	if (!hasAlpha) {
		alpha = S->pixels + alphaChannelOffset;
		while (alpha < end) {
 			*alpha = 0xff;		/* Opaque */
			alpha += 4;
		}
	}
}

/* Load a surface from a BMP file */
AG_Surface *
AG_SurfaceFromBMP(const char *_Nonnull path)
{
	AG_DataSource *ds;
	AG_Surface *S;

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		return (NULL);
	}
	if ((S = AG_ReadSurfaceFromBMP(ds)) == NULL) {
		AG_SetError("%s: %s", path, AG_GetError());
		AG_CloseFile(ds);
		return (NULL);
	}
	AG_CloseFile(ds);
	return (S);
}

/* Load colormap from an Indexed BMP file. */
static int
ReadPaletteFromBMP(AG_Surface *S, AG_DataSource *_Nonnull ds, Uint32 biSize,
    Uint16 biBitCount, Uint biClrUsed)
{
	AG_Palette *pal = S->format.palette;
	Uint8 b,g,r;
	Uint i;

#ifdef AG_DEBUG
	if (pal == NULL) { AG_FatalError("pal=NULL"); }
#endif
	if (biClrUsed == 0) {
		biClrUsed = 1 << biBitCount;
	}
	if (biClrUsed > pal->nColors) {
		AG_Color *colors;
		Uint nColors = biClrUsed;

		colors = AG_TryRealloc(pal->colors, nColors * sizeof(AG_Color));
		if (colors == NULL) {
			return (-1);
		}
		pal->nColors = nColors;
		pal->colors = colors;
	} else if (biClrUsed < pal->nColors) {
		pal->nColors = biClrUsed;
	}
	if (biSize == 12) {
		for (i = 0; i < biClrUsed; ++i) {
			b = AG_ReadUint8(ds);
			g = AG_ReadUint8(ds);
			r = AG_ReadUint8(ds);
			pal->colors[i].r = AG_8toH(r);
			pal->colors[i].g = AG_8toH(g);
			pal->colors[i].b = AG_8toH(b);
			pal->colors[i].a = AG_OPAQUE;
		}
	} else {
		for (i = 0; i < biClrUsed; ++i) {
			b = AG_ReadUint8(ds);
			g = AG_ReadUint8(ds);
			r = AG_ReadUint8(ds);
			/*
			 * According to Microsoft documentation, the 
			 * fourth element is reserved and must be zero,
			 * so we shouldn't treat it as alpha.
			 */
			(void)AG_ReadUint8(ds);
			
			pal->colors[i].r = AG_8toH(r);
			pal->colors[i].g = AG_8toH(g);
			pal->colors[i].b = AG_8toH(b);
			pal->colors[i].a = AG_OPAQUE;
		}
	}
	return (0);
}

/* Read a BMP image from a data source. */
AG_Surface *
AG_ReadSurfaceFromBMP(AG_DataSource *_Nonnull ds)
{
	AG_Surface *S;
	int i, pad, topDown, expandBMP;
	AG_ByteOrder orderSaved;
	AG_Offset bmpHeaderOffset;
	AG_Pixel Rmask = 0;
	AG_Pixel Gmask = 0;
	AG_Pixel Bmask = 0;
	AG_Pixel Amask = 0;
	Uint8 *bits, *top, *end;
	int haveRGBMasks=0, haveAlphaMask=0, willCorrectAlpha=0;
	/* The Win32 BMP file header (14 bytes) */
	char magic[2];
	/* Uint32 bfSize = 0; */
	/* Uint16 bfReserved1 = 0; */
	/* Uint16 bfReserved2 = 0; */
	Uint32 bfOffBits = 0;
	/* The Win32 BITMAPINFOHEADER struct (40 bytes) */
	Uint32 biSize = 0;
	Sint32 biWidth = 0;
	Sint32 biHeight = 0;
	/* Uint16 biPlanes = 0; */
	Uint16 biBitCount = 0;
	Uint32 biCompression = 0;
	/* Uint32 biSizeImage = 0; */
	/* Sint32 biXPelsPerMeter = 0; */
	/* Sint32 biYPelsPerMeter = 0; */
	Uint32 biClrUsed = 0;
	/* Uint32 biClrImportant = 0; */

	/* Read in the BMP file header */
	bmpHeaderOffset = AG_Tell(ds);
	if (AG_Read(ds, magic, 2) != 0 || magic[0] != 'B' || magic[1] != 'M') {
		AG_SetErrorS("Not a Windows BMP file");
		return (NULL);
	}

	orderSaved = AG_SetByteOrder(ds, AG_BYTEORDER_LE);   /* Little Endian */

	/* bfSize      = */ AG_ReadUint32(ds);
	/* bfReserved1 = */ AG_ReadUint16(ds);
	/* bfReserved2 = */ AG_ReadUint16(ds);
	bfOffBits      =    AG_ReadUint32(ds);

	/* Read the Win32 BITMAPINFOHEADER */
	biSize = AG_ReadUint32(ds);
	if (biSize == 12) {                    /* really old BITMAPCOREHEADER */
		biWidth  = (Uint32)AG_ReadUint16(ds);
		biHeight = (Uint32)AG_ReadUint16(ds);
		/* biPlanes = */ AG_ReadUint16(ds);
		biBitCount = AG_ReadUint16(ds);
		biCompression = BI_RGB;
		Debug(NULL, "BMP: Old BITMAPCOREHEADER (%ux%ux%u; RGB)\n",
		    biWidth, biHeight, biBitCount);
	} else if (biSize >= 40) {        /* some version of BITMAPINFOHEADER */
		Uint32 headerSize;

		biWidth = AG_ReadUint32(ds);
		biHeight = AG_ReadUint32(ds);
		/* biPlanes = */ AG_ReadUint16(ds);
		biBitCount = AG_ReadUint16(ds);
		biCompression = AG_ReadUint32(ds);
		/* biSizeImage     = */ AG_ReadUint32(ds);
		/* biXPelsPerMeter = */ AG_ReadUint32(ds);
		/* biYPelsPerMeter = */ AG_ReadUint32(ds);
		biClrUsed = AG_ReadUint32(ds);
 		/* biClrImportant = */ AG_ReadUint32(ds);
		
		Debug(NULL, "BMP: %d-byte BITMAPINFOHEADER (%ux%ux%u; %s)\n",
		    biSize, biWidth, biHeight, biBitCount,
		    (biCompression==0) ? "RGB" :
		    (biCompression==1) ? "RLE8" :
		    (biCompression==2) ? "RLE4" :
		    (biCompression==3) ? "BITFIELDS" : "");

		/*
		 * 64 == BITMAPCOREHEADER2, an incompatible OS/2 2.x extension.
		 * Skip this stuff for now.
		 */
		if (biSize == 64) {
 			/* Ignore these extra fields. */
			if (biCompression == BI_BITFIELDS) {
				/*
				 * This value is actually Huffman compression
				 * in this variant.
				 */
				AG_SetErrorS("Compressed BMP not supported");
				goto fail;
			}
		} else {
			/*
			 * This is complicated. If compression is BI_BITFIELDS,
			 * then we have 3 DWORDS that specify the RGB masks.
			 * This is either stored here in an BITMAPV2INFOHEADER
			 * (which only differs in that it adds these RGB masks)
			 * and biSize >= 52, or we've got these masks stored in
			 * the exact same place, but strictly speaking, this
			 * is the bmiColors field in BITMAPINFO immediately
			 * following legacy v1 info header, just past biSize.
			 */
			if (biCompression == BI_BITFIELDS) {
				haveRGBMasks = 1;
				Rmask = AG_ReadUint32(ds);
				Gmask = AG_ReadUint32(ds);
				Bmask = AG_ReadUint32(ds);

				if (biSize >= 56) {     /* BITMAPV3INFOHEADER */
					haveAlphaMask = 1;
					Amask = AG_ReadUint32(ds);
				}
			} else {
				/*
				 * The mask fields are ignored for v2+ headers
				 * if not BI_BITFIELD.
				 */
				if (biSize >= 52) {     /* BITMAPV2INFOHEADER */
					/*Rmask = */ AG_ReadUint32(ds);
					/*Gmask = */ AG_ReadUint32(ds);
					/*Bmask = */ AG_ReadUint32(ds);
				}
				if (biSize >= 56) {     /* BITMAPV3INFOHEADER */
					/*Amask = */ AG_ReadUint32(ds);
				}
			}

			/*
			 * Insert other fields here; Wikipedia and MSDN say
			 * we're up to v5 of this header, but we ignore those
			 * for now (they add gamma, color spaces, etc).
			 * Ignoring the weird OS/2 2.x format, we currently
			 * parse up to v3 correctly (hopefully!).
			 */
		}
		
		/* Skip any header bytes we didn't handle... */
		headerSize = (Uint32) (AG_Tell(ds) - (bmpHeaderOffset + 14));
		if (biSize > headerSize)
			AG_Seek(ds, (biSize - headerSize), AG_SEEK_CUR);
	}
	if (biHeight < 0) {
		topDown = 1;
		biHeight = -biHeight;
	} else {
		topDown = 0;
	}

	/* Expand 1 and 4 bit bitmaps to 8 bits per pixel */
	/*
	 * XXX TODO remove this since agar's 1- and 4-bit modes should work.
	 */
	switch (biBitCount) {
	case 1:
	case 4:
		expandBMP = biBitCount;
		biBitCount = 8;
		break;
	default:
		expandBMP = 0;
		break;
	}

	/* We don't support any BMP compression right now */
	switch (biCompression) {
	case BI_RGB:
		if (haveRGBMasks || haveAlphaMask) {
			AG_SetErrorS("Unexpected masks");
			goto fail;
		}
		/* Default values for the BMP format */
		switch (biBitCount) {
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
#else
			Rmask = 0x00FF0000;
			Gmask = 0x0000FF00;
			Bmask = 0x000000FF;
#endif
			break;
		case 32:
			/* We don't know if this has alpha channel or not */
			willCorrectAlpha = 1;
			Amask = 0xFF000000;
			Rmask = 0x00FF0000;
			Gmask = 0x0000FF00;
			Bmask = 0x000000FF;
			break;
		default:
			break;
		}
		break;
	case BI_BITFIELDS:
		break;              /* we handled this in the info header. */
	default:
		AG_SetErrorS("Compressed BMP files not supported");
		goto fail;
	}

	switch (biBitCount) {
	case 1:
	case 4:
	case 8:
		S = AG_SurfaceIndexed(biWidth, biHeight, biBitCount, 0);
		Debug(NULL, "BMP image (%ux%u; %u-bpp; INDEXED)\n",
		    biWidth, biHeight, biBitCount);
		break;
	default:
		S = AG_SurfaceRGBA(biWidth, biHeight, biBitCount, 0,
		    Rmask, Gmask, Bmask, Amask);
#if AG_MODEL == AG_LARGE
		Debug(NULL, "BMP image (%ux%u; %u-bpp; "
		            "RGBA %08lx,%08lx,%08lx,%08lx)\n",
			    biWidth, biHeight, biBitCount,
			    (Ulong)Rmask, (Ulong)Gmask, (Ulong)Bmask,
			    (Ulong)Amask);
#else
		Debug(NULL, "BMP image (%ux%u; %u-bpp; "
		            "RGBA %08x,%08x,%08x,%08x)\n",
			    biWidth, biHeight, biBitCount,
			    Rmask, Gmask, Bmask, Amask);
#endif
		break;
	}
	if (S == NULL)
		goto fail;

	/* Read the color palette in Indexed mode. */
	if (S->format.mode == AG_SURFACE_INDEXED &&
	    ReadPaletteFromBMP(S, ds, biSize, biBitCount, biClrUsed) == -1) {
		goto fail;
	}

	/* Read the surface pixels.  Note that the bmp image is upside down */
	if (AG_Seek(ds, bmpHeaderOffset+bfOffBits, AG_SEEK_SET) == -1) {
		goto fail_surf;
	}
	top = S->pixels;
	end = S->pixels + (S->h * S->pitch);
	switch (expandBMP) {
	case 1: {
		int bmpPitch = (biWidth + 7) >> 3;
		pad = (((bmpPitch) % 4) ? (4 - ((bmpPitch) % 4)) : 0);
		break;
	}
	case 4: {
		int bmpPitch = (biWidth + 1) >> 1;
		pad = (((bmpPitch) % 4) ? (4 - ((bmpPitch) % 4)) : 0);
		break;
	}
	default:
		pad = ((S->pitch % 4) ? (4 - (S->pitch % 4)) : 0);
		break;
	}
	if (topDown) {
		bits = top;
	} else {
		bits = end - S->pitch;
	}
	while (bits >= top && bits < end) {
		switch (expandBMP) {
		case 1:
		case 4: {
			Uint8 px = 0;
			int shift = (8 - expandBMP);

	  		for (i = 0; i < S->w; ++i) {
				if (i % (8 / expandBMP) == 0) {
					if (AG_Read(ds, &px, 1) != 0)
						goto fail_surf;
				}
				*(bits + i) = (px >> shift);
				px <<= expandBMP;
			}
			break;
		}
		default:
			if (AG_Read(ds, bits, S->pitch) != 0) {
				goto fail_surf;
			}
#if AG_BYTEORDER == AG_BIG_ENDIAN
			/*
			 * Byte-swap the pixels if needed. Note that the 24bpp
			 * case has already been taken care of above.
			 */
			switch (biBitCount) {
			case 15:
			case 16: {
					Uint16 *px = (Uint16 *)bits;

					for (i = 0; i < S->w; i++)
						px[i] = AG_Swap16(px[i]);
					break;
				}
			case 32: {
					Uint32 *px = (Uint32 *)bits;

					for (i = 0; i < S->w; i++)
						px[i] = AG_Swap32(px[i]);
					break;
				}
			}
#endif /* BE */
			break;
		}
	        /* Skip padding bytes, ugh */
		if (pad) {
			Uint8 padbyte;
			for (i = 0; i < pad; ++i) {
				if (AG_Read(ds, &padbyte, 1) == -1)
					goto fail_surf;
			}
		}
		if (topDown) {
			bits += S->pitch;
		} else {
			bits -= S->pitch;
		}
	}
	if (willCorrectAlpha) {
		CorrectAlphaChannel_32RGB(S);
	}
	AG_SetByteOrder(ds, orderSaved);        /* Restore stream's byte order */
	return (S);
fail_surf:
	AG_SurfaceFree(S);
fail:
	AG_SetByteOrder(ds, orderSaved);
	AG_Seek(ds, bmpHeaderOffset, AG_SEEK_SET);
	return (NULL);
}

/* Export a surface to a BMP image */
int
AG_WriteSurfaceToBMP(AG_DataSource *ds, const AG_Surface *Sarg, Uint8 flags)
{
	AG_Offset offset;
	int i, pitch, pad;
	const AG_Surface *S;
	AG_Palette *pal;
	Uint8 *bits;
	int save32bit = 0;
	AG_ByteOrder orderSaved;
	/* The Win32 BMP file header (14 bytes) */
	char magic[2] = { 'B','M' };
	Uint32 bfSize;
	Uint16 bfReserved1, bfReserved2;
	Uint32 bfOffBits;
	/* The Win32 BITMAPINFOHEADER struct (40 bytes) */
	Uint32 biSize;
	Sint32 biWidth, biHeight, biPlanes;
	Uint16 biBitCount;
	Uint32 biCompression, biSizeImage;
	Sint32 biXPelsPerMeter, biYPelsPerMeter;
	Uint32 biClrUsed, biClrImportant;
	/* Additional members from BITMAPV4HEADER (108 bytes in total) */
	Uint32 bV4RedMask=0, bV4GreenMask=0, bV4BlueMask=0, bV4AlphaMask=0;
	Uint32 bV4CSType=0;
	Sint32 bV4Endpoints[3*3]={0};
	Uint32 bV4GammaRed=0, bV4GammaGreen=0, bV4GammaBlue=0;

	if (!(flags & AG_EXPORT_BMP_NO_32BIT) &&
	    Sarg->format.BitsPerPixel >= 8 &&
	    Sarg->format.Amask != 0) {
		save32bit = 1;
	}
	if (Sarg->format.mode == AG_SURFACE_INDEXED && !save32bit) {
		if (Sarg->format.BitsPerPixel != 8) {
			AG_SetError("%u bpp BMP files not supported",
			    Sarg->format.BitsPerPixel);
			return (-1);
		}
		S = Sarg;
	} else if ((Sarg->format.BitsPerPixel == 24) && !save32bit &&
#if AG_BYTEORDER == AG_LITTLE_ENDIAN
	           (Sarg->format.Rmask == 0x00FF0000) &&
	           (Sarg->format.Gmask == 0x0000FF00) &&
		   (Sarg->format.Bmask == 0x000000FF)
#else
		   (Sarg->format.Rmask == 0x000000FF) &&
		   (Sarg->format.Gmask == 0x0000FF00) &&
		   (Sarg->format.Bmask == 0x00FF0000)
#endif
	   ) {
		S = Sarg;
	} else {
		AG_PixelFormat pf;

		/*
		 * If the surface has a colorkey or alpha channel we'll save a
		 * 32-bit BMP with alpha channel, otherwise save a 24-bit BMP.
		 */
		if (save32bit) {
			AG_PixelFormatRGBA(&pf, 32,		/* BGRA32 */
			    0x000000ff,
			    0x0000ff00,
			    0x00ff0000,
			    0xff000000);
		} else {
			AG_PixelFormatRGB(&pf, 24,		/* BGR24 */
			    0x000000ff,
			    0x0000ff00,
			    0x00ff0000);
		}
		if ((S = AG_SurfaceConvert(Sarg, &pf)) == NULL)
			return (-1);
	}
	
	orderSaved = AG_SetByteOrder(ds, AG_BYTEORDER_LE);   /* Little Endian */

	bfSize = 0;                       /* We'll write this when we're done */
	bfReserved1 = 0;
	bfReserved2 = 0;
	bfOffBits = 0;                    /* We'll write this when we're done */

	/* Write the BMP file header values */
	offset = AG_Tell(ds);
	AG_Write(ds, magic, 2);
	AG_WriteUint32(ds, bfSize);
	AG_WriteUint16(ds, bfReserved1);
	AG_WriteUint16(ds, bfReserved2);
	AG_WriteUint32(ds, bfOffBits);

	/* Set the BMP info values */
	biSize = 40;
	biWidth = S->w;
	biHeight = S->h;
	biPlanes = 1;
	biBitCount = S->format.BitsPerPixel;
	biCompression = BI_RGB;
	biSizeImage = S->h * S->pitch;
	biXPelsPerMeter = 0;
	biYPelsPerMeter = 0;

	biClrUsed = (S->format.mode == AG_SURFACE_INDEXED) ?
	             S->format.palette->nColors : 0;
	biClrImportant = 0;

	/* Set the BMP info values for the version 4 header */
	if (save32bit && !(flags & AG_EXPORT_BMP_LEGACY)) {
		biSize = 108;
		biCompression = BI_BITFIELDS;
		/* The BMP format is always little endian, these masks stay the same */
		bV4RedMask   = 0x00ff0000;
		bV4GreenMask = 0x0000ff00;
		bV4BlueMask  = 0x000000ff;
		bV4AlphaMask = 0xff000000;
		bV4CSType = LCS_WINDOWS_COLOR_SPACE;
		bV4GammaRed = 0;
		bV4GammaGreen = 0;
		bV4GammaBlue = 0;
	}

	/* Write the BMP info values */
	AG_WriteUint32(ds, biSize);
	AG_WriteUint32(ds, biWidth);
	AG_WriteUint32(ds, biHeight);
	AG_WriteUint16(ds, biPlanes);
	AG_WriteUint16(ds, biBitCount);
	AG_WriteUint32(ds, biCompression);
	AG_WriteUint32(ds, biSizeImage);
	AG_WriteUint32(ds, biXPelsPerMeter);
	AG_WriteUint32(ds, biYPelsPerMeter);
	AG_WriteUint32(ds, biClrUsed);
	AG_WriteUint32(ds, biClrImportant);

	/* Write the BMP info values for the version 4 header */
	if (save32bit && !(flags & AG_EXPORT_BMP_LEGACY)) {
		AG_WriteUint32(ds, bV4RedMask);
		AG_WriteUint32(ds, bV4GreenMask);
		AG_WriteUint32(ds, bV4BlueMask);
		AG_WriteUint32(ds, bV4AlphaMask);
		AG_WriteUint32(ds, bV4CSType);
		for (i = 0; i < 3*3; i++) {
			AG_WriteUint32(ds, bV4Endpoints[i]);
		}
		AG_WriteUint32(ds, bV4GammaRed);
		AG_WriteUint32(ds, bV4GammaGreen);
		AG_WriteUint32(ds, bV4GammaBlue);
	}

	/* Write the palette (in BGR color order) */
	if ((pal = S->format.palette) != NULL) {
		for (i = 0; i < pal->nColors; ++i) {
			AG_WriteUint8(ds, pal->colors[i].b);
			AG_WriteUint8(ds, pal->colors[i].g);
			AG_WriteUint8(ds, pal->colors[i].r);
			AG_WriteUint8(ds, pal->colors[i].a);
		}
	}

	/* Write the bitmap offset */
	bfOffBits = (Uint32)(AG_Tell(ds) - offset);
	if (AG_Seek(ds, offset + 10, AG_SEEK_SET) == -1) {
		goto fail;
	}
	AG_WriteUint32(ds, bfOffBits);
	if (AG_Seek(ds, offset + bfOffBits, AG_SEEK_SET) == -1)
		goto fail;

	/* Write the bitmap image upside down */
	bits = S->pixels + (S->h * S->pitch);
	pitch = (S->w * S->format.BytesPerPixel);
	pad = ((pitch % 4) ? (4 - (pitch % 4)) : 0);
	while (bits > S->pixels) {
		bits -= S->pitch;
		if (AG_Write(ds, bits, pitch) != 0) {
			break;
		}
		if (pad) {
			const Uint8 padbyte = 0;
			for (i = 0; i < pad; ++i) {
				AG_Write(ds, &padbyte, 1);
			}
		}
	}

	/* Write the BMP file size */
	bfSize = (Uint32)(AG_Tell(ds) - offset);
	if (AG_Seek(ds, offset + 2, AG_SEEK_SET) == -1) { goto fail; }
	AG_WriteUint32(ds, bfSize);
	if (AG_Seek(ds, offset + bfSize, AG_SEEK_SET) == -1) { goto fail; }

	if (S != Sarg) {
		AG_SurfaceFree((AG_Surface *)S);
	}
	AG_SetByteOrder(ds, orderSaved);       /* Restore stream's byte order */
	return (0);
fail:
	AG_SetByteOrder(ds, orderSaved);
	return (-1);
}

/* Export a surface to a BMP file */
int
AG_SurfaceExportBMP(const AG_Surface *_Nonnull S, const char *_Nonnull path,
    Uint8 flags)
{
	AG_DataSource *ds;

	if ((ds = AG_OpenFile(path, "wb")) == NULL) {
		return (-1);
	}
	if (AG_WriteSurfaceToBMP(ds, S, flags) == -1) {
		AG_SetError("%s: %s", path, AG_GetError());
		AG_CloseFile(ds);
		return (-1);
	}
	AG_CloseFile(ds);
	return (0);
}

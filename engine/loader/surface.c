/*	$Csoft: surface.c,v 1.2 2005/02/14 06:40:19 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>

#include "netbuf.h"
#include "integral.h"
#include "surface.h"
#include "version.h"

const struct version surface_ver = {
	"agar surface",
	0, 0
};

enum {
	RAW_ENCODING,
	RLE_ENCODING,
	PNG_ENCODING,
	JPEG_ENCODING,
	TIFF_ENCODING
};

void
write_surface(struct netbuf *buf, SDL_Surface *su)
{
	Uint8 *src;
	int x, y;

	version_write(buf, &surface_ver);
	write_uint32(buf, RAW_ENCODING);

	write_uint32(buf, su->flags &
	    (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCEL));
	write_uint16(buf, su->w);
	write_uint16(buf, su->h);
	write_uint8(buf, su->format->BitsPerPixel);
	write_uint8(buf, 0);				/* TODO grayscale */
	write_uint32(buf, su->format->Rmask);
	write_uint32(buf, su->format->Gmask);
	write_uint32(buf, su->format->Bmask);
	write_uint32(buf, su->format->Amask);
	write_uint8(buf, su->format->alpha);
	write_uint32(buf, su->format->colorkey);
	
	dprintf("saving %dx%dx%d bpp%s%s surface\n", su->w, su->h,
	    su->format->BitsPerPixel,
	    (su->flags & SDL_SRCALPHA) ? " alpha" : "",
	    (su->flags & SDL_SRCCOLORKEY) ? " colorkey" : "");
	dprintf("masks: %08x,%08x,%08x,%08x\n", su->format->Rmask,
	    su->format->Gmask, su->format->Bmask, su->format->Amask);
	dprintf("colorkey=%08x, alpha=%02x\n", su->format->colorkey,
	    su->format->alpha);

	if (su->format->BitsPerPixel == 8) {
		int i;

		write_uint32(buf, su->format->palette->ncolors);
		for (i = 0; i < su->format->palette->ncolors; i++) {
			write_uint8(buf, su->format->palette->colors[i].r);
			write_uint8(buf, su->format->palette->colors[i].g);
			write_uint8(buf, su->format->palette->colors[i].b);
		}
	}

	if (SDL_MUSTLOCK(su))
		SDL_LockSurface(su);

	src = (Uint8 *)su->pixels;
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			switch (su->format->BytesPerPixel) {
			case 4:
				write_uint32(buf, *(Uint32 *)src);
				break;
			case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				write_uint32(buf,
				    (src[0] << 16) +
				    (src[1] << 8) +
				     src[2]);
#else
				write_uint32(buf,
				    (src[2] << 16) +
				    (src[1] << 8) +
				     src[0]);
#endif
				break;
			case 2:
				write_uint16(buf, *(Uint16 *)src);
				break;
			case 1:
				/* XXX do one big write */
				write_uint8(buf, *src);
				break;
			}
			src += su->format->BytesPerPixel;
		}
	}
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);
}

SDL_Surface *
read_surface(struct netbuf *buf, SDL_PixelFormat *pixfmt)
{
	SDL_Surface *su;
	Uint32 encoding;
	Uint32 flags;
	Uint16 w, h;
	Uint8 depth, grayscale;
	Uint32 Rmask, Gmask, Bmask, Amask;
	Uint8 *dst;
	int i;
	int x, y;

	if (version_read(buf, &surface_ver, NULL) != 0)
		return (NULL);

	encoding = read_uint32(buf);
	if (encoding != RAW_ENCODING) {
		error_set("unsupported encoding");
		return (NULL);
	}

	flags = read_uint32(buf);
	w = read_uint16(buf);
	h = read_uint16(buf);
	depth = read_uint8(buf);
	grayscale = read_uint8(buf);
	Rmask = read_uint32(buf);
	Gmask = read_uint32(buf);
	Bmask = read_uint32(buf);
	Amask = read_uint32(buf);

	su = SDL_CreateRGBSurface(flags|SDL_SWSURFACE, w, h, depth,
	    Rmask, Gmask, Bmask, Amask);
	su->format->alpha = read_uint8(buf);
	su->format->colorkey = read_uint32(buf);
	
	dprintf("loading %dx%dx%d bpp%s%s%s surface\n", w, h, depth,
	    grayscale ? " grayscale" : "",
	    (flags & SDL_SRCALPHA) ? " alpha" : "",
	    (flags & SDL_SRCCOLORKEY) ? " colorkey" : "");
	dprintf("masks: %08x,%08x,%08x,%08x\n", Rmask, Gmask, Bmask, Amask);
	dprintf("colorkey=%08x, alpha=%02x\n", su->format->colorkey,
	    su->format->alpha);

	if (depth == 8) {
		SDL_Color *colors;
		Uint32 ncolors;

		ncolors = read_uint32(buf);
		colors = Malloc(ncolors*sizeof(SDL_Color), 0);

		if (grayscale) {
			for (i = 0; i < ncolors; i++) {
				colors[i].r = i;
				colors[i].g = i;
				colors[i].b = i;
			}
		} else {
			for (i = 0; i < ncolors; i++) {
				colors[i].r = read_uint8(buf);
				colors[i].g = read_uint8(buf);
				colors[i].b = read_uint8(buf);
			}
		}
		SDL_SetPalette(su, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, ncolors);
		Free(colors, 0);
	}
	
	if (SDL_MUSTLOCK(su))
		SDL_LockSurface(su);

	dst = (Uint8 *)su->pixels;
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			switch (su->format->BytesPerPixel) {
			case 4:
				*(Uint32 *)dst = read_uint32(buf);
				break;
			case 3:
				{
					Uint32 c = read_uint32(buf);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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
				*(Uint16 *)dst = read_uint16(buf);
				break;
			case 1:
				/* XXX do one big read */
				*dst = read_uint8(buf);
				break;
			}
			dst += su->format->BytesPerPixel;
		}
	}
	
	if (SDL_MUSTLOCK(su)) {
		SDL_UnlockSurface(su);
	}
	return (su);
}

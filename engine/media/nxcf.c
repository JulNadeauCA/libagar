/*	$Csoft: xcf.c,v 1.36 2002/11/22 08:56:49 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include "engine.h"

#include <libfobj/fobj.h>

#include "map.h"
#include "xcf.h"

#define XCF_SIGNATURE		"gimp xcf "
#define XCF_HEADER_LEN		14
	
enum {
	LAYER_OFFTABLE_INIT =	1,
	LAYER_OFFTABLE_GROW =	4,
	CHANNEL_OFFTABLE_INIT =	1,
	CHANNEL_OFFTABLE_GROW = 2,
	LEVEL_OFFTABLE_INIT =	1,
	LEVEL_OFFTABLE_GROW =	4,
	TILE_OFFTABLE_INIT =	1,
	TILE_OFFTABLE_GROW =	4
};

/* XCF property */
enum {
	PROP_END,
	PROP_COLORMAP,
	PROP_ACTIVE_LAYER,
	PROP_ACTIVE_CHANNEL,
	PROP_SELECTION,
	PROP_FLOATING_SELECTION,
	PROP_OPACITY,
	PROP_MODE,
	PROP_VISIBLE,
	PROP_LINKED,
	PROP_PRESERVE_TRANSPARENCY,
	PROP_APPLY_MASK,
	PROP_EDIT_MASK,
	PROP_SHOW_MASK,
	PROP_SHOW_MASKED,
	PROP_OFFSETS,
	PROP_COLOR,
	PROP_COMPRESSION,
	PROP_GUIDES,
	PROP_RESOLUTION,
	PROP_TATTOO,
	PROP_PARASITES,
	PROP_UNIT,
	PROP_PATHS,
	PROP_USER_UNIT
};

static void	 xcf_init(struct xcf *, int, Uint32);
static int	 xcf_read_header(struct xcf *);
static int	 xcf_read_hierarchy(struct xcf *);
static int	 xcf_read_layers(struct xcf *);
static Uint8	*xcf_load_tile(struct xcf *, Uint32, Uint32, int, int);
static Uint8	*xcf_load_tile_flat(struct xcf *, Uint32, Uint32, int, int);
static Uint8	*xcf_load_tile_rle(struct xcf *, Uint32, Uint32, int, int);
static int	 xcf_convert_tile(struct xcf *, Uint8 *, SDL_Surface *,
		     int, int, int, int);
static void	 xcf_attach_tile(char *, SDL_Surface *, int, int, int, int);

static int
xcf_read_header(struct xcf *xcf)
{
	Uint32 offs;
	Uint32 prid, prlen;
	int i;

	/* Skip the signature. */
	elseek(xcf->fd, xcf->xcf_offs + 14, SEEK_SET);

	/* Geometry */
	xcf->w = read_uint32(xcf->fd);
	xcf->h = read_uint32(xcf->fd);

	/* Image format */
	xcf->image_type = read_sint32(xcf->fd);
	switch (xcf->image_type) {
	case XCF_RGB_IMAGE:
	case XCF_GREYSCALE_IMAGE:
		break;
	case XCF_INDEXED_IMAGE:
		error_set("unsupported image format: indexed");
		return (-1);
	}

	/* Compression and color map */
	do {
		prid = read_uint32(xcf->fd);
		prlen = read_uint32(xcf->fd);

		switch (prid) {
		case PROP_COMPRESSION:
			eread(xcf->fd, &xcf->compression, sizeof(Uint8));
			switch (xcf->compression) {
			case XCF_NO_COMPRESSION:
			case XCF_RLE_COMPRESSION:
				break;
			default:
				error_set("unknown XCF compression : 0x%x",
				    xcf->compression);
				return (-1);
			}
			break;
		case PROP_COLORMAP: {
				Uint32 cnum;

				/* TODO */
				cnum = read_uint32(xcf->fd);
				warning("%d-color map ignored\n", cnum);
				elseek(xcf->fd, 3 * sizeof(char) * cnum,
				    SEEK_CUR);
			}
			break;
		default:
			/* Skip */
			elseek(xcf->fd, prlen, SEEK_CUR);
		}
	} while (prid != PROP_END);

	/* Read the null terminated layer offset table. */
	xcf->nlayers = 0;
	while ((offs = read_uint32(xcf->fd))) {
		if (xcf->nlayers + 1 > xcf->maxlayers) {	/* Grow */
			xcf->maxlayers += LAYER_OFFTABLE_GROW + 1;
			xcf->layer_offs = erealloc(xcf->layer_offs,
			    xcf->maxlayers * sizeof(Uint32) + 1);
			dprintf("grow\n");
		}
		xcf->layer_offs[xcf->nlayers] = offs;
		xcf->nlayers++;
	}
	xcf->layer_offs[xcf->nlayers] = 0;

	dprintf("geo %dx%d type 0x%x comp 0x%x, %d layers\n", xcf->w, xcf->h,
	    xcf->image_type, xcf->compression, xcf->nlayers);
	dprintf("header ok: %d layers\n", xcf->nlayers);
	return (0);
}

static int
xcf_read_hierarchy(struct xcf *xcf)
{
	xcf->hier.w = read_uint32(xcf->fd);
	xcf->hier.h = read_uint32(xcf->fd);
	xcf->hier.bpp = read_uint32(xcf->fd);

	dprintf("w %d h %d bpp %d\n", xcf->hier.w, xcf->hier.h, xcf->hier.bpp);

	xcf->hier.nlevels = 0;
	do {
		if (xcf->hier.nlevels + 1 > xcf->hier.maxlevels) {
			dprintf("grow\n");
			xcf->hier.maxlevels += LEVEL_OFFTABLE_GROW + 1;
			xcf->hier.level_offs = erealloc(xcf->hier.level_offs,
			    xcf->hier.maxlevels * sizeof(Uint32) + 1);
		}
		xcf->hier.level_offs[xcf->hier.nlevels] = read_uint32(xcf->fd);
	} while (xcf->hier.level_offs[xcf->hier.nlevels++]);

	xcf->hier.level_offs[xcf->hier.nlevels] = 0;

	return (0);
}

static int
xcf_read_level(struct xcf *xcf)
{
	xcf->level.w = read_uint32(xcf->fd);
	xcf->level.h = read_uint32(xcf->fd);
	xcf->level.ntiles = 0;
	do {
		if (xcf->level.ntiles + 1 > xcf->level.maxtiles) {
			xcf->level.maxtiles += TILE_OFFTABLE_GROW;
			xcf->level.tile_offs = erealloc(xcf->level.tile_offs,
			    xcf->level.maxtiles * sizeof(Uint32));
			dprintf("grow\n");
		}
		xcf->level.tile_offs[xcf->level.ntiles] = read_uint32(xcf->fd);
	} while (xcf->level.tile_offs[xcf->level.ntiles++]);

	xcf->level.tile_offs[xcf->level.ntiles] = NULL;

	return (0);
}

static Uint8 *
xcf_load_tile_flat(struct xcf *xcf, Uint32 len, Uint32 bpp, int x, int y)
{
	Uint8 *data;
	ssize_t rv;

	data = emalloc((size_t)len);

	rv = read(xcf->fd, data, (size_t)bpp);
	if (rv == -1) {
		error_set("read(0x%x): %s\n", len, strerror(errno));
		free(data);
		return (NULL);
	}
	/* Short reads are ok here. */
	if (rv != (ssize_t)len) {
		error_set("read(%d): only read %ld bytes\n", len, (long)rv);
		free(data);
		return (NULL);
	}
	return (data);
}

static Uint8 *
xcf_load_tile_rle(struct xcf *xcf, Uint32 len, Uint32 bpp, int x, int y)
{
	ssize_t rv;
	int i, size, count, j, length;
	unsigned char *tilep, *tile, *data, *d, val;

	dprintf("len=%d, bpp=%d, geo=%dx%d\n", len, bpp, x, y);

	tile = emalloc(len);
	tilep = tile;

	rv = read(xcf->fd, tile, len);
	if (rv == -1) {
		warning("read(%ld): %s\n", (long)len, strerror(errno));
		free(tilep);
		return (NULL);
	}
	if (rv != (ssize_t)len) {
		warning("read(%ld): read %ld bytes\n", (long)len, (long)rv);
	}

	data = emalloc(x * y * bpp);
	for (i = 0; i < bpp; i++) {
		d = data + i;
		size = x * y;
		count = 0;

		while (size > 0) {
			val = *tile++;

			length = val;
			if (length >= 128) {
				length = 255 - (length - 1);
				if (length == 128) {
					length = (*tile << 8) + tile[1];
					tile += 2;
				}
				count += length;
				size -= length;

				while (length-- > 0) {
					*d = *tile++;
					d += bpp;
				}
			} else {
				length += 1;
				if (length == 128) {
					length = (*tile << 8) + tile[1];
					tile += 2;
				}
				count += length;
				size -= length;

				val = *tile++;

				dprintf("%d,%d, len = %d\n", x, y, length);
				for (j = 0; j < length; j++) {
					*d = val;
					d += bpp;
				}
			}
		}
	}

	free(tilep);
	return (data);
}

static Uint8 *
xcf_load_tile(struct xcf *xcf, Uint32 len, Uint32 bpp, int x, int y)
{
	switch (xcf->compression) {
	case XCF_NO_COMPRESSION:
		return (xcf_load_tile_flat(xcf, len, bpp, x, y));
	case XCF_RLE_COMPRESSION:
		return (xcf_load_tile_rle(xcf, len, bpp, x, y));
	}
	return (NULL);	
}

static int
xcf_read_layers(struct xcf *xcf)
{
	size_t rv;
	int i, fd = xcf->fd;
	Uint32 prid, prlen;

	for (i = xcf->nlayers; i > 0; i--) {		/* Read backwards. */
		SDL_Surface *s;
		Uint32 w, h, ltype;
		Uint32 xoffs = 0, yoffs = 0;
		Uint32 hierarchy_file_offs, layer_mask_offs;
		char *lname;
	
		s = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
		    xcf->w, xcf->h, 32,
		    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
		if (s == NULL) {
			error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
			return (-1);
		}
		
		/* Read the layer. */
		elseek(fd, xcf->xcf_offs + xcf->layer_offs[i - 1], SEEK_SET);
		w = read_uint32(fd);
		h = read_uint32(fd);
		ltype = read_uint32(fd);
		lname = read_string(fd);
		
		dprintf("=> layer %d/%d at 0x%x+0x%x, geo %dx%d, type %d,"
		        "name \"%s\"\n",
		    i, xcf->nlayers, xcf->xcf_offs, xcf->layer_offs[i - 1],
		    w, h, ltype, lname);

		/* Read the layer offset. */
		do {
			prid = read_uint32(fd);
			prlen = read_uint32(fd);

			switch (prid) {
			case PROP_OFFSETS:
				xoffs = read_uint32(fd);
				yoffs = read_uint32(fd);
				break;
			default:
				elseek(fd, prlen, SEEK_CUR);
			}
		} while (prid != PROP_END);

		/* Read the hierarchy and layer mask offsets. */
		hierarchy_file_offs = read_uint32(fd);
		layer_mask_offs = read_uint32(fd);
		dprintf("xo %d yo %d hierarchy offs %d layer mask offs %d\n",
		    xoffs, yoffs, hierarchy_file_offs, layer_mask_offs);

		/* Read the XCF hierarchy. */
		elseek(fd, xcf->xcf_offs + hierarchy_file_offs, SEEK_SET);
		if (xcf_read_hierarchy(xcf) != 0) {
			free(lname);
			SDL_FreeSurface(s);
			return (-1);
		}

		/* Read the XCF levels. */
		for (i = 0; i < xcf->hier.nlevels != NULL; i++) {
			int tx = 0, ty = 0;
			int ox, oy, j;
		
			elseek(fd, xcf->xcf_offs + xcf->hier.level_offs[i],
			    SEEK_SET);
			if (xcf_read_level(xcf) != 0) {
				free(lname);
				SDL_FreeSurface(s);
				return (-1);
			}

			/* Convert the tiles to surfaces. */
			for (j = 0; j < xcf->level.ntiles; j++) {
				Uint8 *tile;
			
				dprintf("=> level %d/%d: tile %d/%d\n", i,
				    xcf->hier.nlevels, j, xcf->level.ntiles);

				elseek(fd, xcf->xcf_offs +
				    xcf->level.tile_offs[j], SEEK_SET);
				
				ox = (tx + 64 > xcf->level.w) ?
				     (xcf->level.w % 64) : 64;
				oy = (ty + 64 > xcf->level.h) ?
				     (xcf->level.h % 64) : 64;

				if (xcf->level.tile_offs[j + 1] != NULL) {
					tile = xcf_load_tile(xcf,
					    xcf->level.tile_offs[j + 1] -
					    xcf->level.tile_offs[j],
					    xcf->hier.bpp,
					    ox, oy);
				} else {
					tile = xcf_load_tile(xcf,
					    ox * oy * 6,
					    xcf->hier.bpp,
					    ox, oy);
				}
				if (tile == NULL) {
					free(lname);
					SDL_FreeSurface(s);
					return (-1);
				}
				if (xcf_convert_tile(xcf, tile, s, tx, ty,
				    ox, oy) != 0) {
					free(lname);
					SDL_FreeSurface(s);
					return (-1);
				}
				xcf_attach_tile(lname, s, tx, ty, ox, oy);
			}
		}

		/* XXX ... */

		free(lname);
	}
	return (0);
}

static int
xcf_convert_tile(struct xcf *xcf, Uint8 *tile, SDL_Surface *su,
    int tx, int ty, int ox, int oy)
{
	Uint8 *p8;
	Uint32 *p32, *row;
	int x, y;

	p8 = tile;
	p32 = (Uint32 *)tile;

	if (SDL_MUSTLOCK(su))
		SDL_LockSurface(su);

	for (y = ty; y < ty + oy; y++) {
		row = (Uint32 *)((Uint8 *)su->pixels + y*su->pitch + (tx<<2));
		switch (xcf->hier.bpp) {
		case 1:
			if (xcf->image_type != XCF_GREYSCALE_IMAGE) {
				error_set("unknown 1bpp image type 0x%x",
				    xcf->image_type);
				if (SDL_MUSTLOCK(su))
					SDL_UnlockSurface(su);
				return (-1);
			}
			for (x = tx; x < tx + ox; x++) {
				*row++ = 0xff000000 |
				    (((Uint32)(*p8))   << 16) |
				    (((Uint32)(*p8))   << 8) |
				    (((Uint32)(*p8++)) << 0);
			}
			break;
		case 2:
			if (xcf->image_type != XCF_GREYSCALE_IMAGE) {
				error_set("unknown 2bpp image type 0x%x",
				    xcf->image_type);
				if (SDL_MUSTLOCK(su))
					SDL_UnlockSurface(su);
				return (-1);
			}
			for (x = tx; x < tx + ox; x++) {
				*row =  ((Uint32) *p8   << 16);
				*row |= ((Uint32) *p8   << 8);
				*row |= ((Uint32) *p8++ << 0);
				row++;
			}
			break;
		case 3:
			for (x = tx; x < tx + ox; x++) {
				*row = 0xff000000;
				*row |= ((Uint32)* (p8++)<<16);
				*row |= ((Uint32)* (p8++)<<8);
				*row |= ((Uint32)* (p8++)<<0);
				row++;
			}
			break;
		case 4:
			for (x = tx; x < tx + ox; x++) {
				*row++ = ((*p32 & 0x000000FF) << 16) |
				    ((*p32 & 0x0000FF00)) |
				    ((*p32 & 0x00FF0000) >> 16) |
				    ((*p32 & 0xFF000000));
				p32++;
			}
			break;
		}
	}
	
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);
	
	return (0);
}

static void
xcf_attach_tile(char *lname, SDL_Surface *su, int tx, int ty, int ox, int oy)
{
}

static void
xcf_init(struct xcf *xcf, int fd, Uint32 xcf_offs)
{
	xcf->fd = fd;
	xcf->xcf_offs = xcf_offs;
	xcf->w = 0;
	xcf->h = 0;
	xcf->image_type = 0;
	xcf->compression = XCF_NO_COMPRESSION;

	xcf->colormap.map = NULL;
	xcf->colormap.size = 0;

	xcf->layer_offs = emalloc(LAYER_OFFTABLE_INIT * sizeof(Uint32));
	xcf->maxlayers = LAYER_OFFTABLE_INIT;

	xcf->channel_offs = emalloc(CHANNEL_OFFTABLE_INIT * sizeof(Uint32));
	xcf->maxchannels = CHANNEL_OFFTABLE_INIT;
	xcf->nchannels = 0;

	xcf->hier.w = 0;
	xcf->hier.h = 0;
	xcf->hier.bpp = 0;
	xcf->hier.level_offs = emalloc(LEVEL_OFFTABLE_INIT * sizeof(Uint32));
	xcf->hier.maxlevels = LEVEL_OFFTABLE_INIT;

	xcf->level.w = 0;
	xcf->level.h = 0;
	xcf->level.tile_offs = emalloc(TILE_OFFTABLE_INIT * sizeof(Uint32));
	xcf->level.maxtiles = TILE_OFFTABLE_INIT;
	xcf->level.ntiles = 0;
}

/* Load XCF file from fob:slot to the given art structure. */
int
xcf_load(int fd, off_t xcf_offs, struct media_art *art)
{
	struct xcf xcf;

	xcf_init(&xcf, fd, xcf_offs);

	if (xcf_read_header(&xcf) != 0) {
		return (-1);
	}
	if (xcf_read_layers(&xcf) != 0) {
		return (-1);
	}
	return (0);
}

int 
xcf_check(int fd, off_t xcf_offs)
{
	char magic[XCF_HEADER_LEN];

	if ((pread(fd, magic, XCF_HEADER_LEN, xcf_offs) ==
	     XCF_HEADER_LEN)) {
		if (strncmp(magic, XCF_SIGNATURE, strlen(XCF_SIGNATURE)) == 0) {
			return (0);
		}
	}
	return (-1);
}


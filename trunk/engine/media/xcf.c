/*	$Csoft: xcf.c,v 1.20 2003/03/22 04:16:35 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <config/floating_point.h>
#include <config/have_ieee754.h>

#include <engine/compat/pread.h>

#include <engine/engine.h>
#include <engine/map.h>

#include <libfobj/fobj.h>

#include "xcf.h"

static SDL_Surface	*xcf_convert_layer(int, Uint32, struct xcf_header *,
			     struct xcf_layer *);
static void		 xcf_insert_surface(struct art *, SDL_Surface *,
			     char *, struct art_anim **);
static Uint8		*xcf_read_tile(struct xcf_header *, int, Uint32, int,
			    int, int);
static void		 xcf_read_property(int, struct xcf_prop *);

#ifdef DEBUG
#define DEBUG_COLORMAPS		0x0001
#define DEBUG_LAYER_OFFSETS	0x0002
#define DEBUG_LAYER_OPACITY	0x0004
#define DEBUG_LAYER_MODE	0x0008
#define DEBUG_IMAGE_COMPRESSION	0x0010
#define DEBUG_CHANNELS		0x0020
#define DEBUG_GUIDES		0x0040
#define DEBUG_RESOLUTIONS	0x0080
#define DEBUG_PARASITES		0x0100
#define DEBUG_UNITS		0x0200
#define DEBUG_LAYER_NAMES	0x0400
#define DEBUG_UNKNOWN_PROPS	0x0800
#define DEBUG_XCF		0x1000
#define DEBUG_ALPHA		0x2000

int	xcf_debug = DEBUG_ALPHA;
#define	engine_debug xcf_debug
#endif

enum {
	LEVEL_OFFSETS_INIT =	2,
	LEVEL_OFFSETS_GROW =	4,
	TILE_OFFSETS_INIT =	16,
	TILE_OFFSETS_GROW =	8
};

int 
xcf_check(int fd, off_t xcf_offs)
{
	char magic[XCF_MAGIC_LEN];

	if ((Pread(fd, magic, XCF_MAGIC_LEN, xcf_offs) == XCF_MAGIC_LEN)) {
		if (strncmp(magic, XCF_SIGNATURE, strlen(XCF_SIGNATURE)) == 0) {
			return (0);
		}
	}
	return (-1);
}

static void 
xcf_read_property(int fd, struct xcf_prop *prop)
{
	Uint32 i;
	Uint8 c;

	prop->id = read_uint32(fd);
	prop->length = read_uint32(fd);

	debug(DEBUG_XCF, "id %u len %u\n", prop->id, prop->length);
	
	switch (prop->id) {
	case PROP_COLORMAP:		      /* Colormap for indexed images */
		prop->data.colormap.size = read_uint32(fd);
		prop->data.colormap.data = emalloc(prop->data.colormap.size*3);
		Read(fd, prop->data.colormap.data, prop->data.colormap.size*3);
		debug(DEBUG_COLORMAPS, "%u-entry colormap\n",
		    prop->data.colormap.size);
		break;
	case PROP_OFFSETS:		         /* Offset of layer in image */
		prop->data.offset.x = read_sint32(fd);
		prop->data.offset.y = read_sint32(fd);
		debug(DEBUG_LAYER_OFFSETS, "offsets %d,%d\n",
		    prop->data.offset.x, prop->data.offset.y);
		break;
	case PROP_OPACITY:				    /* Layer opacity */
		prop->data.opacity = read_uint32(fd);
		debug(DEBUG_LAYER_OPACITY, "opacity %u\n",
		    prop->data.opacity);
		break;
	case PROP_MODE:					 /* Application mode */
		prop->data.mode = read_uint32(fd);
		debug(DEBUG_LAYER_MODE, "mode %u\n", prop->data.mode);
		break;
	case PROP_COMPRESSION:			    /* Tile compression mode */
		Read(fd, &c, 1);
		prop->data.compression = (enum xcf_compression)c;
		debug(DEBUG_IMAGE_COMPRESSION, "compression %s\n",
		    prop->data.compression == XCF_COMPRESSION_NONE ? "none" :
		    prop->data.compression == XCF_COMPRESSION_RLE ? "rle" :
		    "???");
		break;
	case PROP_COLOR:			       /* Color of a channel */
		Read(fd, &prop->data.color, 3);
		debug(DEBUG_CHANNELS, "color %u,%u,%u\n",
		    prop->data.color[0],
		    prop->data.color[1],
		    prop->data.color[2]);
		break;
	case PROP_GUIDES:			                  /* Guides */
		for (i = 0; i < prop->length / 5; i++) {
			prop->data.guide.position = read_sint32(fd);
			prop->data.guide.orientation = read_sint8(fd);
			debug(DEBUG_GUIDES,
			    "guide: position %u, orientation %u\n",
			    prop->data.guide.position,
			    prop->data.guide.orientation);
		}
		break;
#if defined(FLOATING_POINT) && defined(HAVE_IEEE754)
	case PROP_RESOLUTION:				 /* Image resolution */
		prop->data.resolution.x = read_float(fd);
		prop->data.resolution.y = read_float(fd);
		debug(DEBUG_RESOLUTIONS, "resolution %f x %f\n",
		    prop->data.resolution.x, prop->data.resolution.y);
		break;
#endif
	case PROP_TATTOO:					/* Tattoo */
		prop->data.tattoo_state = read_uint32(fd);
		break;
	case PROP_PARASITE:					/* Parasite */
		prop->data.parasite.name = read_string(fd, NULL);
		prop->data.parasite.flags = read_uint32(fd);
		prop->data.parasite.size = read_uint32(fd);
		prop->data.parasite.data = emalloc(prop->data.parasite.size);
		Read(fd, prop->data.parasite.data, prop->data.parasite.size);
		debug_n(DEBUG_PARASITES, "parasite: %s (flags 0x%X size %u)",
		    prop->data.parasite.name, prop->data.parasite.flags,
		    prop->data.parasite.size);
		if (strcmp(prop->data.parasite.name, "gimp-comment") == 0) {
			if (prop->data.parasite.size > 1 &&
			    prop->data.parasite.data[prop->data.parasite.size-2]
			    == '\n')  {
				/* Chop trailing newline. */
				prop->data.parasite.data
				    [prop->data.parasite.size-2] = '\0';
			}
			debug_n(DEBUG_PARASITES, " `%s'",
			    prop->data.parasite.data);
		}
		debug_n(DEBUG_PARASITES, "\n");
		free(prop->data.parasite.name);
		break;
	case PROP_UNIT:
		prop->data.unit = read_uint32(fd);
		debug(DEBUG_UNITS, "unit: %u\n", prop->data.unit);
		break;
	case PROP_USER_UNIT:
	case PROP_PATHS:
		/* XXX ... */
		break;
	default:
		debug(DEBUG_UNKNOWN_PROPS, "unknown: id %u len %u\n",
		    prop->id, prop->length);
		Lseek(fd, prop->length, SEEK_CUR);
	}
}

static Uint8 *
xcf_read_tile_flat(int fd, Uint32 len, int bpp, int x, int y)
{
	Uint8 *load;

	load = emalloc(len);
	Read(fd, load, len);
	return (load);
}

static Uint8 *
xcf_read_tile_rle(int fd, Uint32 len, int bpp, int x, int y)
{
	int i, size, count, j;
	Uint8 *tilep, *tile, *data;
	ssize_t rv;

	tilep = tile = emalloc(len);
	rv = read(fd, tile, len);
	if (rv == -1) {
		error_set("read(%ld): %s", (long)len, strerror(errno));
		free(tile);
		return (NULL);
	}
#if 0
	if (rv != (ssize_t)len) {
		warning("read(%ld): read %ld bytes\n", (long)len, (long)rv);
	}
#endif

	data = emalloc(x * y * bpp);
	for (i = 0; i < bpp; i++) {
		Uint8 *d = &data[i];
	
		size = x * y;
		count = 0;

		while (size > 0) {
			Uint8 val = *tile++;
			int length = val;

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
xcf_read_tile(struct xcf_header *head, int fd, Uint32 len, int bpp,
    int x, int y)
{
	switch (head->compression) {
	case XCF_COMPRESSION_NONE:
		return (xcf_read_tile_flat(fd, len, bpp, x, y));
	case XCF_COMPRESSION_RLE:
		return (xcf_read_tile_rle(fd, len, bpp, x, y));
	}
	error_set("unknown compression: %d", head->compression);
	return (NULL);
}

#define XCF_ALPHA_TRANSPARENT	0x01	/* Contains a transparent pixel */
#define XCF_ALPHA_ALPHA		0x02	/* Contains an alpha pixel */
#define XCF_ALPHA_OPAQUE	0x04	/* Contains an opaque pixel */

/* 32-bit RGBA */
static void
xcf_convert_tile32(int tx, int ox, Uint32 **row, Uint32 **p, int *aflags)
{
	int x;
	Uint8 alpha;
	
	for (x = tx; x < tx + ox; x++) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		**row =
		    (**p & 0x000000ff) |
		    (**p & 0x0000ff00) |
		    (**p & 0x00ff0000) |
		    (**p & 0xff000000);
		alpha = **row & 0x000000ff;
#else
		**row =
		    (**p & 0xff000000) |
		    (**p & 0x00ff0000) |
		    (**p & 0x0000ff00) |
		    (**p & 0x000000ff);
		alpha = (**row & 0xff000000) >> 24;
#endif
		switch (alpha) {
		case 0:
			*aflags |= XCF_ALPHA_TRANSPARENT;
			break;
		case 255:
			*aflags |= XCF_ALPHA_OPAQUE;
			break;
		default:
			*aflags |= XCF_ALPHA_ALPHA;
			break;
		}

		(*row)++;
		(*p)++;
	}
}

static SDL_Surface *
xcf_convert_layer(int fd, Uint32 xcfoffs, struct xcf_header *head,
    struct xcf_layer *layer)
{
	struct xcf_hierarchy *hier;
	int y, tx, ty, ox, oy, i;
	Uint32 *p;
	Uint8 *p8, *tile;
	SDL_Surface *su;
	int aflags = 0;

	su = SDL_CreateRGBSurface(
	    SDL_SWSURFACE,
	    head->w, head->h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
#else
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000,
	    0xff000000
#endif
	    );

	if (su == NULL) {
		error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}

	/* Read the hierarchy. */
	Lseek(fd, xcfoffs + layer->hierarchy_offset, SEEK_SET);
	hier = emalloc(sizeof(struct xcf_hierarchy));
	hier->w = read_uint32(fd);
	hier->h = read_uint32(fd);
	hier->bpp = read_uint32(fd);

	/* Read the level offsets. */
	hier->level_offsets = emalloc(LEVEL_OFFSETS_INIT * sizeof(Uint32));
	hier->maxlevel_offsets = LEVEL_OFFSETS_INIT;
	hier->nlevel_offsets = 0;
	i = 0;
	for (;;) {
		if (hier->nlevel_offsets+1 >= hier->maxlevel_offsets) {
			hier->maxlevel_offsets += LEVEL_OFFSETS_GROW;
			hier->level_offsets = erealloc(hier->level_offsets,
			    hier->maxlevel_offsets * sizeof(Uint32));
		}
		if ((hier->level_offsets[hier->nlevel_offsets] =
		    read_uint32(fd)) == 0) {
			break;
		}
		hier->nlevel_offsets++;
	}

	/* Read the levels. */
	for (i = 0; i < hier->nlevel_offsets; i++) {
		struct xcf_level *level;
		int j;

		Lseek(fd, xcfoffs + hier->level_offsets[i], SEEK_SET);
		level = emalloc(sizeof(struct xcf_level));
		level->w = read_uint32(fd);
		level->h = read_uint32(fd);
		level->tile_offsets = emalloc(TILE_OFFSETS_INIT *
		    sizeof(Uint32));
		level->maxtile_offsets = TILE_OFFSETS_INIT;
		level->ntile_offsets = 0;

		for (;;) {
			if (level->ntile_offsets+1 >= level->maxtile_offsets) {
				level->maxtile_offsets += TILE_OFFSETS_GROW;
				level->tile_offsets = erealloc(
				    level->tile_offsets,
				    level->maxtile_offsets * sizeof(Uint32));
			}
			if ((level->tile_offsets[level->ntile_offsets] =
			    read_uint32(fd)) == 0) {
				break;
			}
			level->ntile_offsets++;
		}

		ty = 0;
		tx = 0;
		for (j = 0; j < level->ntile_offsets; j++) {
			Lseek(fd, xcfoffs + level->tile_offsets[j], SEEK_SET);
			ox = (tx + 64 > level->w) ? (level->w % 64) : 64;
			oy = (ty + 64 > level->h) ? (level->h % 64) : 64;

			if (level->tile_offsets[j + 1] != 0) {
				tile = xcf_read_tile(head, fd,
				    level->tile_offsets[j + 1] -
				    level->tile_offsets[j],
				    hier->bpp,
				    ox, oy);
			} else {
				tile = xcf_read_tile(head, fd,
				    ox * oy * 6,
				    hier->bpp,
				    ox, oy);
			}
			if (tile == NULL) {
				return (NULL);
			}

			p8 = tile;
			p = (Uint32 *)p8;
			for (y = ty; y < ty + oy; y++) {
				Uint32 *row = (Uint32 *)((Uint8 *)su->pixels +
				    y*su->pitch + tx*4);

				switch (hier->bpp) {
				case 4:
					xcf_convert_tile32(tx, ox, &row, &p,
					    &aflags);
					break;
				case 3:
				case 2:
				case 1:
					dprintf("unsupported depth: %dBpp\n",
					    hier->bpp);
					break;
				}
			}
			tx += 64;
			if (tx >= level->w) {
				tx = 0;
				ty += 64;
			}
			if (ty >= level->h) {
				break;
			}
			free(tile);
		}
		free(level->tile_offsets);
		free(level);
	}
	free(hier->level_offsets);
	free(hier);

	/* Adjust the alpha/colorkey properties of the surface. */
	{	
		Uint8 oldalpha, oldckey;

		oldalpha = su->format->alpha;
		oldckey = su->format->colorkey;

		SDL_SetAlpha(su, 0, 0);
		SDL_SetColorKey(su, 0, 0);
		
		if (aflags & (XCF_ALPHA_ALPHA|XCF_ALPHA_TRANSPARENT)) {
			debug(DEBUG_ALPHA, "alpha: %s\n", layer->name);
			SDL_SetAlpha(su, SDL_SRCALPHA, oldalpha);
			/*
			 * XXX noderef_draw_scaled does not understand
			 * RLE acceleration!
			 */
#if 0
		/* XXX causes some images to be rendered incorrectly */
		} else if (aflags & XCF_ALPHA_TRANSPARENT) {
			debug(DEBUG_ALPHA, "colorkey (%u): %s\n", oldckey,
			    layer->name);
			SDL_SetColorKey(su, SDL_SRCCOLORKEY, 0);
#endif
		} else {
			debug(DEBUG_ALPHA, "opaque: %s\n", layer->name);
		}
	}

	return (su);
}

static void
xcf_insert_surface(struct art *art, SDL_Surface *su, char *name,
    struct art_anim **anim)
{
	if (strstr(name, "ms)") != NULL) {		/* XXX ugly */
		char *aname, *anamep;
		int adelay = 0;
	
		anamep = aname = Strdup(name);
		if ((aname = strchr(aname, '(')) != NULL) {
			char *sd;

			aname++;
			if ((sd = strchr(aname, 'm')) == NULL) {
				adelay = 0;
			} else {
				*sd = '\0';
				adelay = atoi(aname);
			}
		}
		free(anamep);

		/* Allocate a new animation. */
		*anim = art_insert_anim(art, adelay);
	} else {
		if (*anim != NULL && name[0] == '+') {
			/* Insert animation frame */
			art_insert_anim_frame(*anim, su);
		} else {
			*anim = NULL;
				
			if ((su->h > TILEH || su->w > TILEW) &&
			    strstr(name, "(break)") != NULL) {
				/* Break down into tiles. */
				art_insert_fragments(art, su);
			} else {
				/* Original size */
		   		art_insert_sprite(art, su, 0);
			}
		}
	}
}

int
xcf_load(int fd, off_t xcf_offs, struct art *art)
{
	struct xcf_header *head;
	int i, offsets;
	Uint32 offset;
	struct xcf_prop prop;
	struct art_anim *curanim = NULL;

	/* Skip the signature. */
	Lseek(fd, xcf_offs + XCF_MAGIC_LEN, SEEK_SET);

	/* Read the XCF header. */
	head = emalloc(sizeof(struct xcf_header));
	head->w = read_uint32(fd);
	head->h = read_uint32(fd);
	if (head->w > 65536 || head->h > 65536) {
		error_set("nonsense geometry: %ux%u", head->w, head->h);
		free(head);
		return (-1);
	}
	head->base_type = read_uint32(fd);
	switch (head->base_type) {
	case XCF_IMAGE_RGB:
	case XCF_IMAGE_GREYSCALE:
	case XCF_IMAGE_INDEXED:
		break;
	default:
		error_set("unknown base image type: %u", head->base_type);
		free(head);
		return (-1);
	}
	head->compression = XCF_COMPRESSION_NONE;
	head->colormap.size = 0;
	head->colormap.data = NULL;
	
	debug(DEBUG_XCF, "%ux%u, type %u\n", head->w, head->h, head->base_type);

	/* Read the image properties. */
	do {
		xcf_read_property(fd, &prop);

		switch (prop.id) {
		case PROP_COMPRESSION:
			head->compression = prop.data.compression;
			break;
		case PROP_COLORMAP:
			head->colormap.size = prop.data.colormap.size;
			head->colormap.data = emalloc(3 * head->colormap.size);
			memcpy(head->colormap.data, prop.data.colormap.data,
			    3 * head->colormap.size);
			break;
		case PROP_RESOLUTION:
			break;
		}
	} while (prop.id != PROP_END);

	/* Reader the layer offsets. */
	head->layer_offstable = NULL;
	offsets = 0;
	for (offsets = 0; (offset = read_uint32(fd)) != 0; offsets++) {
		/* XXX inefficient */
		head->layer_offstable = erealloc(head->layer_offstable,
		    sizeof(Uint32) * (offsets + 1));
		head->layer_offstable[offsets] = offset;
	}

	/* Read the XCF layers. */
	for (i = offsets; i > 0; i--) {
		struct xcf_layer *layer;
		struct xcf_prop prop;
		SDL_Surface *su;

		Lseek(fd, xcf_offs + head->layer_offstable[i - 1], SEEK_SET);
		layer = emalloc(sizeof(struct xcf_layer));
		layer->w = read_uint32(fd);
		layer->h = read_uint32(fd);
		layer->layer_type = read_uint32(fd);
		layer->name = read_string(fd, NULL);

		debug(DEBUG_LAYER_NAMES, "%s: %ux%u\n", layer->name,
		    layer->w, layer->h);

		/* Read the layer properties. */
		do {
			xcf_read_property(fd, &prop);

			switch (prop.id) {
			case PROP_OFFSETS:
				layer->offset_x = prop.data.offset.x;
				layer->offset_y = prop.data.offset.y;
				break;
			case PROP_OPACITY:
				layer->opacity = prop.data.opacity;
				break;
			case PROP_MODE:
				layer->mode = prop.data.mode;
				break;
			}
		} while (prop.id != PROP_END);

		layer->hierarchy_offset = read_uint32(fd);
		layer->mask_offset = read_uint32(fd);

		/* Convert this layer to a SDL surface. */
		su = xcf_convert_layer(fd, xcf_offs, head, layer);
		if (su == NULL) {
			free(layer->name);
			free(layer);
			free(head);
			return (-1);
		}

		/* Register this image. */
		xcf_insert_surface(art, su, layer->name, &curanim);

		free(layer->name);
		free(layer);
	}

	Free(head->colormap.data);
	free(head->layer_offstable);
	free(head);
	return (0);
}


/*	$Csoft: oldxcf.c,v 1.18 2002/12/13 11:18:36 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
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
#include <engine/map.h>

#include <libfobj/fobj.h>

#include "xcf.h"

struct xcf_prop {
	Uint32	id;
	Uint32	length;
	union {
		struct {
			Uint32	 num;
			char	*cmap;
		} color_map;
		struct {
			Uint32	drawable_offset;
		} floating_sel;
		Sint32	opacity;
		Sint32	mode;
		int	visible;
		int	linked;
		int	preserve_transparency;
		int	apply_mask;
		int	show_mask;
		struct {
			Sint32	x;
			Sint32	y;
		} offset;
		Uint8	 color[3];
		Uint8	 compression;
		struct {
			Sint32	x;
			Sint32	y;
		} resolution;
		struct {
			char	*name;
			Uint32	 flags;
			Uint32	 size;
			char	*data;
		} parasite;
	} data;
};

struct xcf_header {
	Uint32	 	 w;
	Uint32	 	 h;
	Sint32	 	 image_type;
	struct xcf_prop	*props;
	Uint32		*layeroffs;
	Uint32		*channeloffs;
	Uint8		 compression;
	Uint32		 cm_num;
	Uint8		*cm_map;
};

struct xcf_layer {
	char		*name;
	Uint32		 w, h;
	Sint32		 layer_type;
	Uint32		 hierarchy_file_offset;
	Uint32		 layer_mask_offset;
	Uint32		 offset_x, offset_y;
	struct xcf_prop	*properties;
};

struct xcf_hierarchy {
	Uint32	 w, h, bpp;
	Uint32	*level_file_offsets;
};

struct xcf_level {
	Uint32	 w, h;
	Uint32	*tile_file_offsets;
};

static SDL_Surface	*xcf_convert_layer(int, Uint32, struct xcf_header *,
			     struct xcf_layer *);
static void		 xcf_insert_surface(struct art *, SDL_Surface *,
			     char *);
static Uint8		*xcf_read_tile(struct xcf_header *, int, Uint32, int,
			    int, int);
static Uint8		*xcf_read_tile_flat(int, Uint32, int, int, int);
static Uint8		*xcf_read_tile_rle(int, Uint32, int, int, int);
static void		 xcf_read_property(int, struct xcf_prop *);

#if defined(__GNUC__) && defined(_SGI_SOURCE)
/* XXX hack for gcc 2.95 under IRIX. */
extern ssize_t	pread(int, void *, size_t, off64_t);
extern ssize_t	pwrite(int, const void *, size_t, off64_t);
#endif

#define XCF_SIGNATURE	"gimp xcf "
#define XCF_SIG_LEN	9
#define XCF_MAGIC_LEN	14

/* Tile compression algorithm */
enum {
	NO_COMPRESSION,
	RLE_COMPRESSION,
	ZLIB_COMPRESSION,
	FRACTAL_COMPRESSION
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

/* XCF Image type */
enum {
	IMAGE_RGB,
	IMAGE_GREYSCALE,
	IMAGE_INDEXED
};

int 
xcf_check(int fd, off_t xcf_offs)
{
	char magic[XCF_MAGIC_LEN];

	if ((pread(fd, magic, XCF_MAGIC_LEN, xcf_offs) == XCF_MAGIC_LEN)) {
		if (strncmp(magic, XCF_SIGNATURE, XCF_SIG_LEN) == 0) {
			return (0);
		}
	}
	return (-1);
}

static void 
xcf_read_property(int fd, struct xcf_prop *prop)
{
	prop->id = read_uint32(fd);
	prop->length = read_uint32(fd);

	switch (prop->id) {
	case PROP_COLORMAP:
		prop->data.color_map.num = read_uint32(fd);
		prop->data.color_map.cmap =
		    emalloc(sizeof(char) * prop->data.color_map.num*3);
		Read(fd, prop->data.color_map.cmap, prop->data.color_map.num*3);
		break;
	case PROP_OFFSETS:
		prop->data.offset.x = read_uint32(fd);
		prop->data.offset.y = read_uint32(fd);
		break;
	case PROP_OPACITY:
		prop->data.opacity = read_uint32(fd);
		break;
	case PROP_COMPRESSION:
	case PROP_COLOR:
		Read(fd, &prop->data, prop->length);
		break;
	default:
		/* Skip this property. */
		elseek(fd, prop->length, SEEK_CUR);
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
	int i, size, count, j, length;
	unsigned char *tilep, *tile, *data, *d, val;
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
	case NO_COMPRESSION:
		return (xcf_read_tile_flat(fd, len, bpp, x, y));
	case RLE_COMPRESSION:
		return (xcf_read_tile_rle(fd, len, bpp, x, y));
	}
	error_set("unknown compression: %d", head->compression);
	return (NULL);
}

static SDL_Surface *
xcf_convert_layer(int fd, Uint32 xcfoffs, struct xcf_header *head,
    struct xcf_layer *layer)
{
	struct xcf_hierarchy *hier;
	struct xcf_level *level;
	int x, y, tx, ty, ox, oy, i, j;
	Uint32 *p, *row;
	Uint16 *p16;
	Uint8 *p8, *tile;
	SDL_Surface *su;
	
	/* XXX */
	su = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
	    head->w, head->h, 32,
	    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	if (su == NULL) {
		error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}

	/* Read the hierarchy. */
	elseek(fd, xcfoffs + layer->hierarchy_file_offset, SEEK_SET);
	hier = emalloc(sizeof(struct xcf_hierarchy));
	hier->w = read_uint32(fd);
	hier->h = read_uint32(fd);
	hier->bpp = read_uint32(fd);

	/* Read the level offsets. */
	hier->level_file_offsets = NULL;
	i = 0;
	do {
		/* XXX inefficient */
		hier->level_file_offsets = erealloc(hier->level_file_offsets,
		    sizeof(Uint32) * (i + 1));
		hier->level_file_offsets[i] = read_uint32(fd);
	} while (hier->level_file_offsets[i++]);

	/* Read the levels. */
	level = NULL;
	for (i = 0; hier->level_file_offsets[i]; i++) {
		struct xcf_level *l;

		elseek(fd, xcfoffs + hier->level_file_offsets[i], SEEK_SET);
		level = emalloc(sizeof(struct xcf_level));
		level->w = read_uint32(fd);
		level->h = read_uint32(fd);
		level->tile_file_offsets = NULL;
		j = 0;
		do {
			/* XXX inefficient */
			level->tile_file_offsets =
			    erealloc(level->tile_file_offsets,
			    sizeof(Uint32) * (j + 1));
			level->tile_file_offsets[j] = read_uint32(fd);
		} while (level->tile_file_offsets[j++]);

		ty = 0;
		tx = 0;
		for (j = 0; level->tile_file_offsets[j]; j++) {
			elseek(fd, xcfoffs + level->tile_file_offsets[j],
			    SEEK_SET);
			ox = (tx + 64 > level->w) ? (level->w % 64) : 64;
			oy = (ty + 64 > level->h) ? (level->h % 64) : 64;

			if (level->tile_file_offsets[j + 1]) {
				tile = xcf_read_tile(head, fd,
				    level->tile_file_offsets[j + 1] -
				    level->tile_file_offsets[j],
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
			p16 = (Uint16 *) p8;
			p = (Uint32 *) p8;
			for (y = ty; y < ty + oy; y++) {
				row = (Uint32 *)
				    ((Uint8 *)su->pixels + y *
				    su->pitch + (tx << 2));
				switch (hier->bpp) {
				case 4:
					for (x = tx; x < tx + ox; x++) {
						*row++ =
						       ((*p & 0x000000FF) << 16)
						     | ((*p & 0x0000FF00))
						     | ((*p & 0x00FF0000) >> 16)
						     | ((*p & 0xFF000000));
						p++;
					}
					break;
				case 3:
					for (x = tx; x < tx + ox; x++) {
						*row = 0xFF000000;
						*row |= ((Uint32)* (p8++)<<16);
						*row |= ((Uint32)* (p8++)<<8);
						*row |= ((Uint32)* (p8++)<<0);
						row++;
					}
					break;
				case 2:
					/*
					 * Indexed/Greyscale + Alpha.
					 */
					switch (head->image_type) {
					case IMAGE_INDEXED:
						for (x = tx; x < tx + ox; x++) {
							*row = ((Uint32)
							    (head->cm_map[
							     *p8 * 3])<<16);
							*row |= ((Uint32)
							    (head->cm_map[
							     *p8 * 3 + 1])<<8);
							*row |= ((Uint32)
							    (head->cm_map[
							     *p8++ * 3+2])<<0);
							*row |= ((Uint32)
							     *p8++<<24);
							row++;
						}
						break;
					case IMAGE_GREYSCALE:
						for (x = tx; x < tx + ox; x++) {
							*row = ((Uint32)
							    *p8 << 16);
							*row |= ((Uint32)
							    *p8 << 8);
							*row |= ((Uint32)
							    *p8++ << 0);
							*row |= ((Uint32)
							    *p8++ << 24);
							row++;
						}
						break;
					default:
						error_set("bad 2Bpp type");
						return (NULL);
					}
					break;
				/*
				 * Indexed/Greyscale.
				 */
				case 1:
					switch (head->image_type) {
					case IMAGE_INDEXED:
						for (x = tx; x < tx + ox; x++) {
							*row++ = 0xFF000000
						 	    | ((Uint32)
							       (head->cm_map[
							        *p8 * 3])<<16)
							    | ((Uint32)
							       (head->cm_map[
							        *p8 * 3+1])<<8)
							    | ((Uint32)
							       (head->cm_map[
							       *p8 * 3+2])<<0);
							p8++;
						}
						break;
					case IMAGE_GREYSCALE:
						for (x = tx; x < tx + ox; x++) {
							*row++ = 0xFF000000
							    | (((Uint32)
							       (*p8)) << 16)
							    | (((Uint32)
							       (*p8)) << 8)
							    | (((Uint32)
							       (*p8)) << 0);
							p8++;
						}
						break;
					default:
						error_set("bad 1Bpp type\n");
						return (NULL);
					}
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
		free(level->tile_file_offsets);
		free(level);
	}
	free(hier->level_file_offsets);
	free(hier);

	return (su);
}

static void
xcf_insert_surface(struct art *art, SDL_Surface *su, char *name)
{
	struct art_anim *nanim = NULL;

	if (strstr(name, "ms)") != NULL) {
		char *aname, *anamep;
		int adelay = 0;
	
		aname = Strdup(name);
		anamep = aname;
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
		nanim = art_insert_anim(art, adelay);
	} else {
		if (nanim != NULL && name[0] == '+') {
			/* Insert animation frame */
			art_insert_anim_frame(nanim, su);
		} else {
			int offs;
			
			nanim = NULL;
				
			/* Original size */
		   	offs = art_insert_sprite(art, su);
			art_map_sprite(art, offs);

			if (su->h > TILEH ||
			    su->w > TILEW) {
				/* Break down into tiles. */
				art_insert_sprite_tiles(art, su);
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
	
	/* Skip the signature. */
	elseek(fd, xcf_offs + 14, SEEK_SET);

	/* Read the XCF header. */
	head = emalloc(sizeof(struct xcf_header));
	head->w = read_uint32(fd);
	head->h = read_uint32(fd);
	head->image_type = read_uint32(fd);
	head->props = NULL;
	head->compression = NO_COMPRESSION;
	head->cm_num = 0;
	head->cm_map = NULL;
	do {
		xcf_read_property(fd, &prop);

		switch (prop.id) {
		case PROP_COMPRESSION:
			head->compression = prop.data.compression;
			break;
		case PROP_COLORMAP:
			head->cm_num = prop.data.color_map.num;
			head->cm_map = emalloc(sizeof(char) * 3 * head->cm_num);
			memcpy(head->cm_map, prop.data.color_map.cmap,
			    3 * sizeof(char) * head->cm_num);
			break;
		}
	} while (prop.id != PROP_END);

	/* Reader the layer offsets. */
	head->layeroffs = NULL;
	offsets = 0;
	for (offsets = 0; (offset = read_uint32(fd)) != 0; offsets++) {
		/* XXX inefficient */
		head->layeroffs = erealloc(head->layeroffs,
		    sizeof(Uint32) * (offsets + 1));
		head->layeroffs[offsets] = offset;
	}

	/* Read the XCF layers. */
	for (i = offsets; i > 0; i--) {
		struct xcf_layer *layer;
		struct xcf_prop prop;
		SDL_Surface *su;

		/* Read this layer. */
		elseek(fd, xcf_offs + head->layeroffs[i - 1], SEEK_SET);
		layer = emalloc(sizeof(struct xcf_layer));
		layer->w = read_uint32(fd);
		layer->h = read_uint32(fd);
		layer->layer_type = read_uint32(fd);
		layer->name = read_string(fd, NULL);
		do {
			xcf_read_property(fd, &prop);

			switch (prop.id) {
			case PROP_OFFSETS:
				layer->offset_x = prop.data.offset.x;
				layer->offset_y = prop.data.offset.y;
				break;
			}
		} while (prop.id != PROP_END);
		layer->hierarchy_file_offset = read_uint32(fd);
		layer->layer_mask_offset = read_uint32(fd);

		/* Convert this layer to a SDL surface. */
		su = xcf_convert_layer(fd, xcf_offs, head, layer);
		if (su == NULL) {
			free(layer->name);
			free(layer);
			free(head->layeroffs);
			free(head);
			return (-1);
		}

		/* Register this image. */
		xcf_insert_surface(art, su, layer->name);

		free(layer->name);
		free(layer);
	}

	if (head->cm_num > 0) {
		free(head->cm_map);
	}
	free(head->layeroffs);
	free(head);
	return (0);
}


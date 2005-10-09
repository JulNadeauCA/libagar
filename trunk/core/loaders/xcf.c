/*	$Csoft: xcf.c,v 1.21 2005/09/19 05:26:50 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/have_ieee754.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/map/map.h>			/* For AGTILESZ */

#include <engine/loader/xcf.h>

#include <string.h>
#include <stdlib.h>

enum xcf_compression {
	XCF_COMPRESSION_NONE,
	XCF_COMPRESSION_RLE,
	XCF_COMPRESSION_ZLIB,		/* Unimplemented */
	XCF_COMPRESSION_FRACTAL		/* Unimplemented */
};

enum xcf_base_type {
	XCF_IMAGE_RGB,
	XCF_IMAGE_GREYSCALE,
	XCF_IMAGE_INDEXED
};

struct xcf_guide {
	Uint32 position;
	Uint8 orientation;
	SLIST_ENTRY(xcf_guide) guides;
};

struct xcf_prop {
	Uint32 id;
	Uint32 length;
	union {
		enum xcf_compression compression;	/* Tile compression */
		struct {
			Uint32 size;		/* Number of RGB triplets */
			char *data;		/* RGB triplet array */
		} colormap;
		struct {
			Sint32 position;	/* Guide coordinates */
			Sint8 orientation;	/* Guide orientation */
		} guide;
		struct {
			char *name;
			Uint32 flags;
			Uint32 size;
			Uint8 *data;
		} parasite;
		Uint32 tattoo_state;		/* Tattoo state */
		Uint32 unit;			/* Measurement unit */
		struct {
			Uint32 drawable_offset; /* Floating selection offset */
		} floating_sel;
		Uint32 opacity;			/* Layer opacity */
		Uint32 mode;			/* Application mode */
		struct {
			Sint32 x, y;		/* Layer offset in image */
		} offset;
		Uint8 color[3];			/* RGB triplet for color */
#if defined(HAVE_IEEE754)
		struct {
			float x, y;		/* Resolution */
		} resolution;
#endif
	} data;
};

struct xcf_header {
	Uint32 w, h;				/* Geometry in pixels */
	enum xcf_base_type base_type;		/* Type of image */
	Uint32 *layer_offstable;
	Uint32 *channel_offstable;
	Uint8 compression;
	struct {
		Uint32 size;
		Uint8 *data;
	} colormap;
};

struct xcf_layer {
	char *name;			/* Identifier */
	Uint32 w, h;			/* Geometry in pixels */
	Uint32 layer_type;		/* Image type information */
	Uint32 offset_x, offset_y;	/* Offset of layer in image */
	Uint32 opacity;			/* Layer opacity */
	Uint32 mode;			/* Application mode */
	Uint32 hierarchy_offset;	/* Offset of xcf_hierarchy */
	Uint32 mask_offset;		/* Offset of mask xcf_layer */
};

struct xcf_hierarchy {
	Uint32 w, h;
	Uint32 bpp;
	Uint32 *level_offsets;
	int    nlevel_offsets;
	int  maxlevel_offsets;
};

struct xcf_level {
	Uint32 w, h;
	Uint32 *tile_offsets;
	int    ntile_offsets;
	int  maxtile_offsets;
};

enum {
	PROP_END,
	PROP_COLORMAP,
	PROP_ACTIVE_LAYER,
	PROP_ACTIVE_CHANNEL,
	PROP_SELECTION,
	AG_PROP_FLOATING_SELECTION,
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
	PROP_PARASITE,
	PROP_UNIT,
	PROP_PATHS,
	PROP_USER_UNIT
};

static SDL_Surface *xcf_convert_layer(AG_Netbuf *, Uint32, struct xcf_header *, 
		                      struct xcf_layer *);
static int xcf_insert_surface(AG_Gfx *, SDL_Surface *, const char *);
static Uint8 *xcf_read_tile(struct xcf_header *, AG_Netbuf *, Uint32, int, int,
		            int);
static void xcf_read_property(AG_Netbuf *, struct xcf_prop *);

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

int	agXCFDebugLvl = 0;
#define	agDebugLvl agXCFDebugLvl
#endif

enum {
	LEVEL_OFFSETS_INIT =	2,
	LEVEL_OFFSETS_GROW =	4,
	TILE_OFFSETS_INIT =	16,
	TILE_OFFSETS_GROW =	8
};

#define XCF_SIGNATURE	"gimp xcf "
#define XCF_MAGIC_LEN	14
#define XCF_WIDTH_MAX	65536
#define XCF_HEIGHT_MAX	65536

static void 
xcf_read_property(AG_Netbuf *buf, struct xcf_prop *prop)
{
	Uint32 i;
	Uint8 c;

	prop->id = AG_ReadUint32(buf);
	prop->length = AG_ReadUint32(buf);

	debug(DEBUG_XCF, "id %u len %u\n", prop->id, prop->length);
	
	switch (prop->id) {
	case PROP_COLORMAP:		      /* Colormap for indexed images */
		prop->data.colormap.size = AG_ReadUint32(buf);
		prop->data.colormap.data = Malloc(prop->data.colormap.size*3,
		    M_LOADER);
		AG_NetbufRead(prop->data.colormap.data, prop->data.colormap.size,
		    3, buf);
		debug(DEBUG_COLORMAPS, "%u-entry colormap\n",
		    prop->data.colormap.size);
		break;
	case PROP_OFFSETS:		         /* Offset of layer in image */
		prop->data.offset.x = AG_ReadSint32(buf);
		prop->data.offset.y = AG_ReadSint32(buf);
		debug(DEBUG_LAYER_OFFSETS, "offsets %d,%d\n",
		    prop->data.offset.x, prop->data.offset.y);
		break;
	case PROP_OPACITY:				    /* Layer opacity */
		prop->data.opacity = AG_ReadUint32(buf);
		debug(DEBUG_LAYER_OPACITY, "opacity %u\n",
		    prop->data.opacity);
		break;
	case PROP_MODE:					 /* Application mode */
		prop->data.mode = AG_ReadUint32(buf);
		debug(DEBUG_LAYER_MODE, "mode %u\n", prop->data.mode);
		break;
	case PROP_COMPRESSION:			    /* Tile compression mode */
		c = AG_ReadUint8(buf);
		prop->data.compression = (enum xcf_compression)c;
		debug(DEBUG_IMAGE_COMPRESSION, "compression %s\n",
		    prop->data.compression == XCF_COMPRESSION_NONE ? "none" :
		    prop->data.compression == XCF_COMPRESSION_RLE ? "rle" :
		    "???");
		break;
	case PROP_COLOR:			       /* Color of a channel */
		AG_NetbufRead(&prop->data.color, sizeof(Uint8), 3, buf);
		debug(DEBUG_CHANNELS, "color %u,%u,%u\n",
		    prop->data.color[0],
		    prop->data.color[1],
		    prop->data.color[2]);
		break;
	case PROP_GUIDES:			                  /* Guides */
		for (i = 0; i < prop->length / 5; i++) {
			prop->data.guide.position = AG_ReadSint32(buf);
			prop->data.guide.orientation = AG_ReadSint8(buf);
			debug(DEBUG_GUIDES,
			    "guide: position %u, orientation %u\n",
			    prop->data.guide.position,
			    prop->data.guide.orientation);
		}
		break;
#if defined(HAVE_IEEE754)
	case PROP_RESOLUTION:				 /* Image resolution */
		prop->data.resolution.x = AG_ReadFloat(buf);
		prop->data.resolution.y = AG_ReadFloat(buf);
		debug(DEBUG_RESOLUTIONS, "resolution %f x %f\n",
		    prop->data.resolution.x, prop->data.resolution.y);
		break;
#endif
	case PROP_TATTOO:					/* Tattoo */
		prop->data.tattoo_state = AG_ReadUint32(buf);
		break;
	case PROP_PARASITE:					/* Parasite */
		prop->data.parasite.name = AG_ReadNulString(buf);
		prop->data.parasite.flags = AG_ReadUint32(buf);
		prop->data.parasite.size = AG_ReadUint32(buf);
		prop->data.parasite.data = Malloc(prop->data.parasite.size,
		    M_LOADER);
		AG_NetbufRead(prop->data.parasite.data, prop->data.parasite.size,
		    1, buf);
#if 0
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
#endif
		Free(prop->data.parasite.name, 0);
		Free(prop->data.parasite.data, M_LOADER);
		break;
	case PROP_UNIT:
		prop->data.unit = AG_ReadUint32(buf);
		debug(DEBUG_UNITS, "unit: %u\n", prop->data.unit);
		break;
	case PROP_USER_UNIT:
	case PROP_PATHS:
		/* XXX ... */
		break;
	default:
		debug(DEBUG_UNKNOWN_PROPS, "unknown: id %u len %u\n",
		    prop->id, prop->length);
		AG_NetbufSeek(buf, prop->length, SEEK_CUR);
	}
}

static Uint8 *
xcf_read_tile_flat(AG_Netbuf *buf, Uint32 len, int bpp, int x, int y)
{
	Uint8 *load;

	load = Malloc(len, M_LOADER);
	AG_NetbufRead(load, len, 1, buf);
	return (load);
}

static Uint8 *
xcf_read_tile_rle(AG_Netbuf *buf, Uint32 len, int bpp, int x, int y)
{
	int i, size, count, j;
	Uint8 *tilep, *tile, *data;
	ssize_t rv;

	tilep = tile = Malloc(len, M_LOADER);
	rv = AG_NetbufReadE(tile, sizeof(Uint8), len, buf);
#if 0
	if (rv < len) {
		AG_SetError("short read");
		Free(tile, M_LOADER);
		return (NULL);
	}
#endif

	data = Malloc(x * y * bpp, M_LOADER);
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

	Free(tilep, M_LOADER);
	return (data);
}

static Uint8 *
xcf_read_tile(struct xcf_header *head, AG_Netbuf *buf, Uint32 len, int bpp,
    int x, int y)
{
	switch (head->compression) {
	case XCF_COMPRESSION_NONE:
		return (xcf_read_tile_flat(buf, len, bpp, x, y));
	case XCF_COMPRESSION_RLE:
		return (xcf_read_tile_rle(buf, len, bpp, x, y));
	}
	AG_SetError(_("Unknown XCF compression: %d."), head->compression);
	return (NULL);
}

#define XCF_ALPHA_TRANSPARENT	0x01	/* Contains a transparent pixel */
#define XCF_ALPHA_ALPHA		0x02	/* Contains an alpha pixel */
#define XCF_ALPHA_OPAQUE	0x04	/* Contains an opaque pixel */

static void
xcf_convert_level(AG_Netbuf *buf, Uint32 xcfoffs,
    struct xcf_hierarchy *hier, struct xcf_header *head,
    struct xcf_level *level, SDL_Surface *su, int *aflags)
{
	int tx = 0, ty = 0;
	int ox, oy;
	int j;

	for (j = 0; j < level->ntile_offsets; j++) {
		Uint8 *tile;
		Uint32 *src;
		int y;

		AG_NetbufSeek(buf, xcfoffs + level->tile_offsets[j], SEEK_SET);
		ox = (tx+64 > level->w) ? (level->w % 64) : 64;
		oy = (ty+64 > level->h) ? (level->h % 64) : 64;

		if (level->tile_offsets[j+1] != 0) {
			tile = xcf_read_tile(head, buf,
			    level->tile_offsets[j+1] - level->tile_offsets[j],
			    hier->bpp, ox, oy);
		} else {
			tile = xcf_read_tile(head, buf, ox*oy*6, hier->bpp,
			    ox, oy);
		}
		if (tile == NULL) {
			dprintf("tile read error\n");
			return;
		}
	
		src = (Uint32 *)tile;
		for (y = ty; y < ty+oy; y++) {
			Uint8 *dst = (Uint8 *)su->pixels +
			    y*su->pitch +
			    tx*su->format->BytesPerPixel;
			Uint32 color;
			Uint8 r, g, b, a;
			int x;

			for (x = tx; x < tx+ox; x++) {
				switch (hier->bpp) {
				case 4:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
					r = (*src & 0xff000000)>>24;
					g = (*src & 0x00ff0000)>>16;
					b = (*src & 0x0000ff00)>>8;
					a = (*src & 0x000000ff);
#else
					r = (*src & 0x000000ff);
					g = (*src & 0x0000ff00)>>8;
					b = (*src & 0x00ff0000)>>16;
					a = (*src & 0xff000000)>>24;
#endif
					break;
				default:
					fatal("unsupported xcf depth");
				}

				color = SDL_MapRGBA(su->format, r, g, b, a);

				switch (su->format->BytesPerPixel) {
				case 4:
					*(Uint32 *)dst = color;
					break;
				case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
					dst[0] = (color>>16) & 0xff;
					dst[1] = (color>>8) & 0xff;
					dst[2] = color & 0xff;
#else
					dst[2] = (color>>16) & 0xff;
					dst[1] = (color>>8) & 0xff;
					dst[0] = color & 0xff;
#endif
					break;
				case 2:
					*(Uint16 *)dst = color;
					break;
				case 1:
					*dst = color;
					break;
				}
				dst += su->format->BytesPerPixel;
				src++;

				switch (a) {
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
			}
		}

		tx += 64;
		if (tx >= level->w) {
			tx = 0;
			ty += 64;
		}
		if (ty >= level->h) {
			Free(tile, M_LOADER);
			break;
		}
		Free(tile, M_LOADER);
	}
}

static SDL_Surface *
xcf_convert_layer(AG_Netbuf *buf, Uint32 xcfoffs, struct xcf_header *head,
    struct xcf_layer *layer)
{
	struct xcf_hierarchy *hier;
	SDL_Surface *su;
	Uint32 *p;
	int aflags = 0;
	int i;

	su = SDL_CreateRGBSurface(SDL_SWSURFACE, head->w, head->h, 32,
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
		AG_SetError("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}

	/* Read the hierarchy. */
	AG_NetbufSeek(buf, xcfoffs+layer->hierarchy_offset, SEEK_SET);

	hier = Malloc(sizeof(struct xcf_hierarchy), M_LOADER);
	hier->w = AG_ReadUint32(buf);
	hier->h = AG_ReadUint32(buf);
	hier->bpp = AG_ReadUint32(buf);

	/* Read the level offsets. */
	hier->level_offsets = Malloc(LEVEL_OFFSETS_INIT*sizeof(Uint32), M_LOADER);
	hier->maxlevel_offsets = LEVEL_OFFSETS_INIT;
	hier->nlevel_offsets = 0;
	i = 0;
	for (;;) {
		if (hier->nlevel_offsets+1 >= hier->maxlevel_offsets) {
			hier->maxlevel_offsets += LEVEL_OFFSETS_GROW;
			hier->level_offsets = Realloc(hier->level_offsets,
			    hier->maxlevel_offsets*sizeof(Uint32));
		}
		if ((hier->level_offsets[hier->nlevel_offsets] =
		    AG_ReadUint32(buf)) == 0) {
			break;
		}
		hier->nlevel_offsets++;
	}

	/* Read the levels. */
	for (i = 0; i < hier->nlevel_offsets; i++) {
		struct xcf_level *level;

		AG_NetbufSeek(buf, xcfoffs + hier->level_offsets[i], SEEK_SET);
		level = Malloc(sizeof(struct xcf_level), M_LOADER);
		level->w = AG_ReadUint32(buf);
		level->h = AG_ReadUint32(buf);
		level->tile_offsets = Malloc(TILE_OFFSETS_INIT*sizeof(Uint32),
		    M_LOADER);
		level->maxtile_offsets = TILE_OFFSETS_INIT;
		level->ntile_offsets = 0;

		for (;;) {
			if (level->ntile_offsets+1 >= level->maxtile_offsets) {
				level->maxtile_offsets += TILE_OFFSETS_GROW;
				level->tile_offsets = Realloc(
				    level->tile_offsets,
				    level->maxtile_offsets*sizeof(Uint32));
			}
			if ((level->tile_offsets[level->ntile_offsets] =
			    AG_ReadUint32(buf)) == 0) {
				break;
			}
			level->ntile_offsets++;
		}
		xcf_convert_level(buf, xcfoffs, hier, head, level, su, &aflags);

		Free(level->tile_offsets, M_LOADER);
		Free(level, M_LOADER);
	}
	Free(hier->level_offsets, M_LOADER);
	Free(hier, M_LOADER);

	/* Adjust the alpha/colorkey properties of the surface. */
	{	
		Uint8 oldalpha, oldckey;

		oldalpha = su->format->alpha;
		oldckey = su->format->colorkey;

		SDL_SetAlpha(su, 0, 0);
		SDL_SetColorKey(su, 0, 0);
	
		if (aflags & (XCF_ALPHA_ALPHA|XCF_ALPHA_TRANSPARENT)) {
			SDL_SetAlpha(su, SDL_SRCALPHA, oldalpha);
#if 0
		/* XXX causes some images to be rendered incorrectly */
		/*
		 * XXX the AG_NitemDraw_scaled hack does not understand
		 * RLE acceleration!
		 */
		} else if (aflags & XCF_ALPHA_TRANSPARENT) {
			SDL_SetColorKey(su, SDL_SRCCOLORKEY, 0);
#endif
		}
	}
	return (su);
}

static int
xcf_insert_surface(AG_Gfx *gfx, SDL_Surface *su, const char *name)
{
	return (0);
}

int
AG_XCFLoad(AG_Netbuf *buf, off_t xcf_offs, AG_Gfx *gfx)
{
	char magic[XCF_MAGIC_LEN];
	struct xcf_header *head;
	int i, offsets;
	Uint32 offset;
	struct xcf_prop prop;

	AG_NetbufSeek(buf, xcf_offs, SEEK_SET);

	if (AG_NetbufReadE(magic, sizeof(magic), 1, buf) < 1) {
		AG_SetError(_("Cannot read XCF magic."));
		return (-1);
	}
	if (strncmp(magic, XCF_SIGNATURE, strlen(XCF_SIGNATURE)) != 0) {
		AG_SetError(_("Bad XCF magic."));
		return (-1);
	}

	/* Read the XCF header. */
	head = Malloc(sizeof(struct xcf_header), M_LOADER);
	head->w = AG_ReadUint32(buf);
	head->h = AG_ReadUint32(buf);
	if (head->w > XCF_WIDTH_MAX || head->h > XCF_HEIGHT_MAX) {
		AG_SetError(_("Nonsense XCF geometry: %ux%u."), head->w, head->h);
		Free(head, M_LOADER);
		return (-1);
	}

	head->base_type = AG_ReadUint32(buf);
	switch (head->base_type) {
	case XCF_IMAGE_RGB:
	case XCF_IMAGE_GREYSCALE:
	case XCF_IMAGE_INDEXED:
		break;
	default:
		AG_SetError(_("Unknown base image type: %u."), head->base_type);
		Free(head, M_LOADER);
		return (-1);
	}
	head->compression = XCF_COMPRESSION_NONE;
	head->colormap.size = 0;
	head->colormap.data = NULL;
	
	debug(DEBUG_XCF, "%ux%u, type %u\n", head->w, head->h, head->base_type);

	/* Read the image properties. */
	do {
		xcf_read_property(buf, &prop);

		switch (prop.id) {
		case PROP_COMPRESSION:
			head->compression = prop.data.compression;
			break;
		case PROP_COLORMAP:
			head->colormap.size = prop.data.colormap.size;
			head->colormap.data = Malloc(3*head->colormap.size,
			    M_LOADER);
			memcpy(head->colormap.data, prop.data.colormap.data,
			    3*head->colormap.size);
			break;
		case PROP_RESOLUTION:
			break;
		}
	} while (prop.id != PROP_END);

	/* Reader the layer offsets. */
	head->layer_offstable = NULL;
	offsets = 0;
	for (offsets = 0; (offset = AG_ReadUint32(buf)) != 0; offsets++) {
		/* XXX inefficient */
		head->layer_offstable = Realloc(head->layer_offstable,
		    (offsets+1)*sizeof(Uint32));
		head->layer_offstable[offsets] = offset;
	}

	/* Read the XCF layers. */
	for (i = offsets; i > 0; i--) {
		struct xcf_layer *layer;
		struct xcf_prop prop;
		AG_Sprite *spr;
		Uint32 sname;
		SDL_Surface *su;

		AG_NetbufSeek(buf, xcf_offs + head->layer_offstable[i-1],
		    SEEK_SET);
		layer = Malloc(sizeof(struct xcf_layer), M_LOADER);
		layer->w = AG_ReadUint32(buf);
		layer->h = AG_ReadUint32(buf);
		layer->layer_type = AG_ReadUint32(buf);
		layer->name = AG_ReadNulString(buf);

		debug(DEBUG_LAYER_NAMES, "Layer `%s': %ux%u\n", layer->name,
		    layer->w, layer->h);

		/* Read the layer properties. */
		do {
			xcf_read_property(buf, &prop);

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

		layer->hierarchy_offset = AG_ReadUint32(buf);
		layer->mask_offset = AG_ReadUint32(buf);

		/* Convert this layer to a SDL surface. */
		if ((su = xcf_convert_layer(buf, xcf_offs, head, layer))
		    == NULL) {
			Free(layer->name, 0);
			Free(layer, M_LOADER);
			Free(head, M_LOADER);
			return (-1);
		}

		sname = AG_GfxAddSprite(gfx, su);
		spr = &gfx->sprites[sname];
		strlcpy(spr->name, layer->name, sizeof(spr->name));

		Free(layer->name, 0);
		Free(layer, M_LOADER);
	}

	Free(head->colormap.data, M_LOADER);
	Free(head->layer_offstable, M_LOADER);
	Free(head, M_LOADER);
	return (0);
fail:
	Free(head->colormap.data, M_LOADER);
	Free(head->layer_offstable, M_LOADER);
	Free(head, M_LOADER);
	return (-1);
}


/*
 * Copyright (c) 2002-2007 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Loader for Gimp 1.x XCF image format.
 */
#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/surface.h>
#include <agar/gui/load_xcf.h>

#include <string.h>

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
			Uint32 _pad;
			char *data;		/* RGB triplet array */
		} colormap;
		struct {
			Sint32 position;	/* Guide coordinates */
			Sint8 orientation;	/* Guide orientation */
			Uint8 _pad[3];
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
		struct {
			float x, y;		/* Resolution */
		} resolution;
	} data;
};

struct xcf_header {
	Uint32 w, h;				/* Geometry in pixels */
	enum xcf_base_type base_type;		/* Type of image */
	Uint32 _pad1;
	Uint32 *layer_offstable;
	Uint32 *channel_offstable;
	Uint8 compression;
	char _pad2[7];
	struct {
		Uint32 size;
		Uint32 _pad;
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
	Uint32 _pad;
};

struct xcf_hierarchy {
	Uint32 w, h;
	Uint32 bpp;
	Uint32 _pad;
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
enum {
	LEVEL_OFFSETS_INIT =	2,
	LEVEL_OFFSETS_GROW =	4,
	TILE_OFFSETS_INIT =	16,
	TILE_OFFSETS_GROW =	8
};

#define XCF_WIDTH_MAX	65536
#define XCF_HEIGHT_MAX	65536

static void 
ReadProp(AG_DataSource *buf, struct xcf_prop *prop)
{
	Uint32 i;
	Uint8 c;

	prop->id = AG_ReadUint32(buf);
	prop->length = AG_ReadUint32(buf);

	switch (prop->id) {
	case PROP_COLORMAP:		      /* Colormap for indexed images */
		prop->data.colormap.size = AG_ReadUint32(buf);
		prop->data.colormap.data = Malloc(prop->data.colormap.size*3);
		if (AG_Read(buf, prop->data.colormap.data,
		    prop->data.colormap.size*3) != 0) {
			AG_FatalError(NULL);
		}
		break;
	case PROP_OFFSETS:		         /* Offset of layer in image */
		prop->data.offset.x = AG_ReadSint32(buf);
		prop->data.offset.y = AG_ReadSint32(buf);
		break;
	case PROP_OPACITY:				    /* Layer opacity */
		prop->data.opacity = AG_ReadUint32(buf);
		break;
	case PROP_MODE:					 /* Application mode */
		prop->data.mode = AG_ReadUint32(buf);
		break;
	case PROP_COMPRESSION:			    /* Tile compression mode */
		c = AG_ReadUint8(buf);
		prop->data.compression = (enum xcf_compression)c;
		break;
	case PROP_COLOR:			       /* Color of a channel */
		if (AG_Read(buf, &prop->data.color, sizeof(Uint8)*3) != 0)
			AG_FatalError(NULL);
		break;
	case PROP_GUIDES:			                  /* Guides */
		for (i = 0; i < prop->length / 5; i++) {
			prop->data.guide.position = AG_ReadSint32(buf);
			prop->data.guide.orientation = AG_ReadSint8(buf);
		}
		break;
	case PROP_RESOLUTION:				 /* Image resolution */
		prop->data.resolution.x = AG_ReadFloat(buf);
		prop->data.resolution.y = AG_ReadFloat(buf);
		break;
	case PROP_TATTOO:					/* Tattoo */
		prop->data.tattoo_state = AG_ReadUint32(buf);
		break;
	case PROP_PARASITE:					/* Parasite */
		prop->data.parasite.name = AG_ReadNulString(buf);
		prop->data.parasite.flags = AG_ReadUint32(buf);
		prop->data.parasite.size = AG_ReadUint32(buf);
		prop->data.parasite.data = Malloc(prop->data.parasite.size);
		if (AG_Read(buf, prop->data.parasite.data,
		    prop->data.parasite.size) != 0) {
			AG_FatalError(NULL);
		}
		Free(prop->data.parasite.name);
		Free(prop->data.parasite.data);
		break;
	case PROP_UNIT:
		prop->data.unit = AG_ReadUint32(buf);
		break;
	case PROP_USER_UNIT:
	case PROP_PATHS:
		/* XXX ... */
		break;
	default:
		AG_Seek(buf, prop->length, AG_SEEK_CUR);
	}
}

static Uint8 *
ReadTileFlat(AG_DataSource *buf, Uint32 len, int bpp, int x, int y)
{
	Uint8 *load;

	load = Malloc(len);
	if (AG_Read(buf, load, len) != 0) {
		Free(load);
		return (NULL);
	}
	return (load);
}

static Uint8 *
ReadTileRLE(AG_DataSource *buf, Uint32 len, int Bpp, int x, int y)
{
	int i, size, count, j;
	Uint8 *tilep, *tile, *data;

	tilep = tile = Malloc(len);
	if (AG_Read(buf, tile, len) != 0) {
		AG_SetError("XCF Tile: Read error");
		return (NULL);
	}
	if ((data = TryMalloc(x*y*Bpp)) == NULL) {
		return (NULL);
	}
	for (i = 0; i < Bpp; i++) {
		Uint8 *d = &data[i];
	
		size = x*y;
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
					d += Bpp;
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
					d += Bpp;
				}
			}
		}
	}
	Free(tilep);
	return (data);
}

static Uint8 *
ReadTile(struct xcf_header *head, AG_DataSource *buf, Uint32 len, int Bpp,
    int x, int y)
{
	switch (head->compression) {
	case XCF_COMPRESSION_NONE:
		return ReadTileFlat(buf, len, Bpp, x, y);
	case XCF_COMPRESSION_RLE:
		return ReadTileRLE(buf, len, Bpp, x, y);
	}
	AG_SetError(_("Unknown XCF compression mode: %d"), head->compression);
	return (NULL);
}

#define XCF_ALPHA_TRANSPARENT	0x01	/* Contains a transparent pixel */
#define XCF_ALPHA_ALPHA		0x02	/* Contains an alpha pixel */
#define XCF_ALPHA_OPAQUE	0x04	/* Contains an opaque pixel */

static int
ConvertLevel(AG_DataSource *buf, Uint32 xcfoffs, struct xcf_hierarchy *hier,
    struct xcf_header *head, struct xcf_level *level, AG_Surface *su,
    int *aflags)
{
	int tx = 0, ty = 0;
	int ox, oy;
	int j;

	for (j = 0; j < level->ntile_offsets; j++) {
		Uint8 *tile, *p;
		int y;

		AG_Seek(buf, xcfoffs+level->tile_offsets[j], AG_SEEK_SET);
		ox = (tx+64 > (int)level->w) ? (level->w % 64) : 64;
		oy = (ty+64 > (int)level->h) ? (level->h % 64) : 64;

		if (level->tile_offsets[j+1] != 0) {
			tile = ReadTile(head, buf,
			    level->tile_offsets[j+1] - level->tile_offsets[j],
			    hier->bpp, ox, oy);
		} else {
			tile = ReadTile(head, buf, ox*oy*6, hier->bpp, ox, oy);
		}
		if (tile == NULL) {
			/* return (-1); */
			continue;
		}
	
		p = tile;
		for (y = ty; y < ty+oy; y++) {
			Uint8 *dst = su->pixels + (y * su->pitch) +
			                         (tx * su->format.BytesPerPixel);
			Uint32 px;
			Uint8 r, g, b, a;
			int x;

			for (x = tx; x < tx + ox; x++) {
				switch (hier->bpp) {
				case 4:
#if AG_BYTEORDER == AG_BIG_ENDIAN
					r = (*(Uint32 *)p & 0xff000000) >> 24;
					g = (*(Uint32 *)p & 0x00ff0000) >> 16;
					b = (*(Uint32 *)p & 0x0000ff00) >> 8;
					a = (*(Uint32 *)p & 0x000000ff);
#else
					r = (*(Uint32 *)p & 0x000000ff);
					g = (*(Uint32 *)p & 0x0000ff00) >> 8;
					b = (*(Uint32 *)p & 0x00ff0000) >> 16;
					a = (*(Uint32 *)p & 0xff000000) >> 24;
#endif
					break;
				case 3:
					r = p[2];
					g = p[1];
					b = p[0];
					a = 255;
					break;
				default:
					r = 0;
					g = 0;
					b = 0;
					a = 255;
					break;
				}

				px = AG_MapPixel32_RGBA8(&su->format, r,g,b,a);
				switch (su->format.BytesPerPixel) {
				case 4:
					*(Uint32 *)dst = px;
					break;
				case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
					dst[0] = (px >> 16) & 0xff;
					dst[1] = (px >> 8)  & 0xff;
					dst[2] =  px        & 0xff;
#else
					dst[2] = (px >> 16) & 0xff;
					dst[1] = (px >> 8)  & 0xff;
					dst[0] =  px        & 0xff;
#endif
					break;
				case 2:
					*(Uint16 *)dst = px;
					break;
				case 1:
					*dst = px;
					break;
				}
				dst += su->format.BytesPerPixel;
				p += hier->bpp;

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
		if (tx >= (int)level->w) {
			tx = 0;
			ty += 64;
		}
		if (ty >= (int)level->h) {
			Free(tile);
			break;
		}
		Free(tile);
	}
	return (0);
}

static AG_Surface *
ConvertLayer(AG_DataSource *buf, Uint32 xcfoffs, struct xcf_header *head,
    struct xcf_layer *layer)
{
	struct xcf_hierarchy *hier;
	AG_Surface *su;
	int aflags = 0;
	int i;

	su = AG_SurfaceRGBA(head->w, head->h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
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
	if (su == NULL)
		return (NULL);

	/* Read the hierarchy. */
	AG_Seek(buf, xcfoffs+layer->hierarchy_offset, AG_SEEK_SET);

	hier = Malloc(sizeof(struct xcf_hierarchy));
	hier->w = AG_ReadUint32(buf);
	hier->h = AG_ReadUint32(buf);
	hier->bpp = AG_ReadUint32(buf);
	if (hier->bpp != 4 && hier->bpp != 3) {
		AG_SetError(_("Cannot handle %dBpp XCF data"),
		    (int)hier->bpp);
		Free(hier);
		return (NULL);
	}

	/* Read the level offsets. */
	hier->level_offsets = Malloc(LEVEL_OFFSETS_INIT*sizeof(Uint32));
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

		AG_Seek(buf, xcfoffs+hier->level_offsets[i], AG_SEEK_SET);
		level = Malloc(sizeof(struct xcf_level));
		level->w = AG_ReadUint32(buf);
		level->h = AG_ReadUint32(buf);
		level->tile_offsets = Malloc(TILE_OFFSETS_INIT*sizeof(Uint32));
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
		if (ConvertLevel(buf, xcfoffs, hier, head, level, su, &aflags)
		    == -1) {
			Free(hier->level_offsets);
			Free(hier);
			Free(level->tile_offsets);
			Free(level);
			AG_SurfaceFree(su);
			return (NULL);				/* LEAK */
		}
		Free(level->tile_offsets);
		Free(level);
	}
	Free(hier->level_offsets);
	Free(hier);

	/* Adjust the alpha/colorkey properties of the surface. */
	{	
		Uint8 oldalpha = su->alpha;

		AG_SurfaceSetAlpha(su, 0, 0);
		AG_SurfaceSetColorKey(su, 0, 0);
		if (aflags & (XCF_ALPHA_ALPHA|XCF_ALPHA_TRANSPARENT))
			AG_SurfaceSetAlpha(su, AG_SURFACE_ALPHA, oldalpha);
	}
	return (su);
}

/*
 * Load a set of surfaces from an XCF file. The add_layer_fn callback function
 * is passed the converted surface for each layer in the file.
 */
int
AG_XCFLoad(AG_DataSource *buf, AG_Offset xcf_offs,
    void (*add_layer_fn)(AG_Surface *, const char *, void *), void *arg)
{
	char magic[14];
	struct xcf_header *head;
	int i, offsets;
	Uint32 offset;
	struct xcf_prop prop;

	AG_Seek(buf, xcf_offs, AG_SEEK_SET);

	if (AG_Read(buf, magic, sizeof(magic)) != 0 ||
	    strncmp(magic, "gimp xcf ", 9) != 0) {
		AG_SetError(_("Not a Gimp XCF file"));
		return (-1);
	}

	/* Read the XCF header. */
	head = Malloc(sizeof(struct xcf_header));
	head->w = AG_ReadUint32(buf);
	head->h = AG_ReadUint32(buf);
	if (head->w > XCF_WIDTH_MAX ||
	    head->h > XCF_HEIGHT_MAX) {
		AG_SetError(_("Bad XCF geometry: %ux%u"),
		    (unsigned int)head->w,
		    (unsigned int)head->h);
		Free(head);
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
		Free(head);
		return (-1);
	}
	head->compression = XCF_COMPRESSION_NONE;
	head->colormap.size = 0;
	head->colormap.data = NULL;
	
	/* Read the image properties. */
	do {
		ReadProp(buf, &prop);

		switch (prop.id) {
		case PROP_COMPRESSION:
			head->compression = prop.data.compression;
			break;
		case PROP_COLORMAP:
			head->colormap.size = prop.data.colormap.size;
			head->colormap.data = Malloc(3*head->colormap.size);
			memcpy(head->colormap.data, prop.data.colormap.data,
			    3*head->colormap.size);
			break;
		case PROP_RESOLUTION:
			break;
		}
	} while (prop.id != PROP_END);
	
	Debug(NULL, "XCF image (%ux%u; type %d; compression %d; colormap %d)\n",
	    head->w, head->h, head->base_type, head->compression,
	    head->colormap.size);
	Debug(NULL, "XCF layers: ");

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
		AG_Surface *su;

		AG_Seek(buf, xcf_offs+head->layer_offstable[i-1], AG_SEEK_SET);
		layer = Malloc(sizeof(struct xcf_layer));
		layer->w = AG_ReadUint32(buf);
		layer->h = AG_ReadUint32(buf);
		layer->layer_type = AG_ReadUint32(buf);
		layer->name = AG_ReadNulString(buf);

		Debug(NULL, "\"%s\" ", layer->name);

		/* Read the layer properties. */
		do {
			ReadProp(buf, &prop);

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
		if ((su = ConvertLayer(buf, xcf_offs, head, layer))
		    == NULL) {
			Free(layer->name);
			Free(layer);
			Free(head);
			return (-1);
		}

		add_layer_fn(su, layer->name, arg);
		Free(layer->name);
		Free(layer);
	}
	Debug(NULL, "\n");

	Free(head->colormap.data);
	Free(head->layer_offstable);
	Free(head);
	return (0);
}

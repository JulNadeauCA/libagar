/*	$Csoft: xcf.h,v 1.1 2002/12/17 06:48:49 vedge Exp $	*/
/*	Public domain	*/

#define XCF_SIGNATURE	"gimp xcf "
#define XCF_MAGIC_LEN	14

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
	Uint32		*layer_offstable;
	Uint32		*channel_offstable;
	Uint8		 compression;
	Uint32		 cmap_count;
	Uint8		*cmap;
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

int 	xcf_check(int, off_t);
int	xcf_load(int, off_t, struct art *);


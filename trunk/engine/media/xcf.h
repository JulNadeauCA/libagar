/*	$Csoft: xcf.h,v 1.4 2002/12/23 03:05:44 vedge Exp $	*/
/*	Public domain	*/

#include <config/have_ieee754.h>

#define XCF_SIGNATURE	"gimp xcf "
#define XCF_MAGIC_LEN	14

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
	Uint32	position;
	Uint8	orientation;

	SLIST_ENTRY(xcf_guide)	guides;
};

struct xcf_prop {
	Uint32	id;
	Uint32	length;
	union {
		/*
		 * Image properties
		 */
		enum xcf_compression	compression;	/* Tile compression */
		struct {
			Uint32	 size;		/* Number of RGB triplets */
			char	*data;		/* RGB triplet array */
		} colormap;
		struct {
			Sint32	 position;	/* Guide coordinates */
			Sint8	 orientation;	/* Guide orientation */
		} guide;
		struct {
			char	*name;
			Uint32	 flags;
			Uint32	 size;
			Uint8	*data;
		} parasite;
		Uint32	 tattoo_state;		/* Tattoo state */
		Uint32	 unit;			/* Measurement unit */
		/*
		 * Layer properties
		 */
		struct {
			Uint32	drawable_offset; /* Floating selection offset */
		} floating_sel;
		Uint32	opacity;		/* Layer opacity */
		Uint32	mode;			/* Application mode */
		struct {
			Sint32	x, y;		/* Layer offset in image */
		} offset;
		/*
		 * Channel properties
		 */
		Uint8	 color[3];		/* RGB triplet for color */
		struct {
#ifdef HAVE_IEEE754
			float	x, y;		/* Resolution */
#else
			Uint32	x, y;		/* Resolution */
#endif
		} resolution;
	} data;
};

struct xcf_header {
	Uint32	 	 	 w, h;			/* Geometry in pixels */
	enum xcf_base_type	 base_type;		/* Type of image */
	Uint32			*layer_offstable;
	Uint32			*channel_offstable;
	Uint8			 compression;
	struct {
		Uint32	 size;
		Uint8	*data;
	} colormap;
};

struct xcf_layer {
	char	*name;			/* Identifier */
	Uint32	 w, h;			/* Geometry in pixels */
	Uint32	 layer_type;		/* Image type information */
	Uint32	 offset_x, offset_y;	/* Offset of layer in image */
	Uint32	 opacity;		/* Layer opacity */
	Uint32	 mode;			/* Application mode */
	Uint32	 hierarchy_offset;	/* Offset of xcf_hierarchy */
	Uint32	 mask_offset;		/* Offset of mask xcf_layer */
};

struct xcf_hierarchy {
	Uint32	 w, h;
	Uint32	 bpp;
	Uint32	*level_offsets;
};

struct xcf_level {
	Uint32	 w, h;
	Uint32	*tile_offsets;
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
	PROP_PARASITE,
	PROP_UNIT,
	PROP_PATHS,
	PROP_USER_UNIT
};

int 	xcf_check(int, off_t);
int	xcf_load(int, off_t, struct art *);


/*	$Csoft: xcf.h,v 1.13 2002/09/16 01:51:12 vedge Exp $	*/
/*	Public domain	*/

struct xcf {
	int	 fd;
	Uint32	 xcf_offs;

	Uint32	 w, h;
	Sint32	 image_type;
#define XCF_RGB_IMAGE		0
#define XCF_GREYSCALE_IMAGE	1
#define XCF_INDEXED_IMAGE	2

	Uint8	 compression;
#define XCF_NO_COMPRESSION	0
#define XCF_RLE_COMPRESSION	1
#define XCF_ZLIB_COMPRESSION	2
#define XCF_FRACTAL_COMPRESSION	3
	
	struct {
		unsigned char	*map;
		Uint32		 size;
	} colormap;

	Uint32	*layer_offs;
	int	 nlayers, maxlayers;
	
	Uint32	*channel_offs;
	int	 nchannels, maxchannels;

	struct {
		Uint32	 w, h, bpp;
		Uint32	*level_offs;
		int	 nlevels, maxlevels;
	} hier;
	struct {
		Uint32	 w, h;
		Uint32	*tile_offs;
		int	 ntiles, maxtiles;
	} level;
};

int	xcf_load(int, off_t, struct art *);
int 	xcf_check(int, off_t);


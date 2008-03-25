/*	Public domain	*/

/* Pixel packing method for surfaces */
typedef struct gi_pixel_format {
	GI_Palette *palette;			/* For indexed formats */
	Uint   Bpp, bpp;			/* Bytes/bits per pixel */
	Uint8  Rloss, Gloss, Bloss, Aloss;	/* Precision loss */
	Uint8  Rshift, Gshift, Bshift, Ashift;	/* Component shift */
	Uint32 Rmask, Gmask, Bmask, Amask;	/* Component mask */
} GI_PixelFormat;

/* Surface of pixels */
typedef struct gi_surface {
	GI_PixelFormat *fmt;			/* Pixel packing mode */
	AG_Mutex lock;
	Uint w, h;				/* Dimensions in pixels */
	Uint pitch;				/* Scanline in bytes */
	Uint32 flags;
	void *p;				/* Pixel data */
} GI_Surface;

__BEGIN_DECLS
void        GI_SurfaceInit(GI_Surface *);
void        GI_SurfaceDestroy(GI_Surface *);
GI_Surface *GI_SurfaceNew(int, int, const GI_PixelFormat *, Uint);
void        GI_SurfaceFree(GI_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* HAVE_OPENGL && HAVE_SDL */

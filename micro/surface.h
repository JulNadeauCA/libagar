/*	Public domain	*/

#ifndef _AGAR_MICRO_SURFACE_H_
#define _AGAR_MICRO_SURFACE_H_

#include <agar/micro/geometry.h>
#include <agar/micro/colors.h>
#include <agar/micro/begin.h>

/* RGB pixel packing format */
typedef struct ma_pixel_packing {
	Uint16 Rmask,  Gmask,  Bmask,  Amask;	/* Component masks */
	Uint8  Rshift, Gshift, Bshift, Ashift;	/* Bits at right of component */
	Uint8  Rloss,  Gloss,  Bloss,  Aloss;	/* Bits lost by packing */
} MA_PixelPacking;

/* Pixel storage information */
typedef struct ma_pixel_format {	/* Bits per pixel: | 1  2  4  8  16 | */
	Uint8 mode;             
#define MA_SURFACE_PACKED    0		/* Packed RGB/RGBA |             Y  | */
#define MA_SURFACE_INDEXED   1		/* Palettized      | Y  Y  Y  Y     | */
#define MA_SURFACE_GRAYSCALE 2		/* Grayscale+Alpha |    Y  Y  Y  Y  | */

	Uint8 BitsPerPixel;		/* Depth (in bits/pixel) */
	Uint8 PixelsPerByte;		/* Depth (in pixels/byte) */

	union {
		MA_PixelPacking *packed; /* RGB format for PACKED mode */
		void *palette;		 /* Palette address for INDEXED */
	} data;
} MA_PixelFormat;

/* Micro-Agar graphics surface */
typedef struct ma_surface {
	MA_PixelFormat format;		/* Pixel format */
	Uint8 flags;
#define MA_SURFACE_COLORKEY	0x01	/* Enable color key for blit as src */
#define MA_SURFACE_ALPHA	0x02	/* Enable alpha for blit as src */
                             /* 0x04       Unused */
#define MA_SURFACE_MAPPED	0x08	/* Disallow MA_SurfaceFree() */
#define MA_SURFACE_STATIC	0x10	/* Don't free() in MA_SurfaceFree() */
#define MA_SURFACE_EXT_PIXELS	0x20	/* Pixels are allocated externally */
#define MA_SAVED_SURFACE_FLAGS	(MA_SURFACE_COLORKEY | MA_SURFACE_ALPHA)

	Uint8 w, h;			/* Size in pixels */
	Uint8 pitch;			/* Scanline byte length */
	Uint8 *_Nullable pixels;	/* Raw pixel data */

	Uint8 padding;			/* Scanline end padding */
	MA_Pixel colorkey;		/* Color key pixel */
	MA_Component alpha;		/* Per-surface alpha */
} MA_Surface;

__BEGIN_DECLS
extern MA_PixelFormat *_Nullable maSurfaceFmt;  /* Standard surface format */

void MA_PixelFormatRGB(MA_PixelFormat *_Nonnull, Uint8, MA_Pixel,MA_Pixel,MA_Pixel);
void MA_PixelFormatRGBA(MA_PixelFormat *_Nonnull, Uint8,
                        MA_Pixel,MA_Pixel,MA_Pixel,MA_Pixel);
void MA_PixelFormatIndexed(MA_PixelFormat *_Nonnull, Uint8);
void MA_PixelFormatGrayscale(MA_PixelFormat *_Nonnull, Uint8);

void                 MA_SurfaceInit(MA_Surface *_Nonnull,
                                    const MA_PixelFormat *_Nullable,
				    Uint8,Uint8, Uint8);
MA_Surface *_Nonnull MA_SurfaceNew(const MA_PixelFormat *_Nonnull,
                                   Uint8,Uint8, Uint8)
		                  _Warn_Unused_Result;
MA_Surface *_Nonnull MA_SurfaceIndexed(Uint8,Uint8, Uint8, Uint8)
                                      _Warn_Unused_Result;
MA_Surface *_Nonnull MA_SurfaceGrayscale(Uint8,Uint8, Uint8, Uint8)
                                        _Warn_Unused_Result;
MA_Surface *_Nonnull MA_SurfaceRGBA(Uint8,Uint8, Uint8, Uint8,
                                    MA_Pixel,MA_Pixel,MA_Pixel,MA_Pixel)
				   _Warn_Unused_Result;

void MA_SurfaceSetAddress(MA_Surface *_Nonnull, Uint8 *_Nonnull);

MA_Surface *_Nonnull MA_SurfaceDup(const MA_Surface *_Nonnull)
	                          _Warn_Unused_Result;
void                 MA_SurfaceCopy(MA_Surface *_Nonnull,
                                    const MA_Surface *_Nonnull);

void MA_SurfaceBlit(const MA_Surface *_Nonnull, const MA_Rect *_Nullable,
		    MA_Surface *_Nonnull, Uint8,Uint8);

void MA_SurfaceFree(MA_Surface *_Nonnull);

void MA_FillRect(MA_Surface *_Nonnull, const MA_Rect *_Nullable,
                 const MA_Color *_Nonnull);

Uint8  MA_SurfaceGet8(const MA_Surface *_Nonnull, Uint8,Uint8)
                     _Pure_Attribute;
void   MA_SurfacePut8(MA_Surface *_Nonnull, Uint8,Uint8, Uint8);

Uint16 AG_MapPixel16_RGBA4(const MA_PixelFormat *_Nonnull, Uint8,Uint8,Uint8,Uint8)
                          _Pure_Attribute;
void   AG_GetColor16_RGBA4(Uint16, const MA_PixelFormat *_Nonnull,
                           Uint8 *_Nonnull, Uint8 *_Nonnull,
                           Uint8 *_Nonnull, Uint8 *_Nonnull);
void   AG_GetColor16(MA_Color *_Nonnull, Uint16, const MA_PixelFormat *_Nonnull);

# define MA_SurfaceGet(S,x,y)                 MA_SurfaceGet32((S),(x),(y))
# define MA_SurfaceGet_At(S,p)                MA_SurfaceGet32_At((S),(p))
# define MA_GetColor(c,px,pf)                 MA_GetColor32((c),(px),(pf))
# define MA_SurfacePut(S,x,y,px)              MA_SurfacePut32((S),(x),(y),(px))
# define MA_SurfacePut_At(S,p,px)             MA_SurfacePut32_At((S),(p),(px))
# define MA_MapPixel(pf,px)                   MA_MapPixel32((pf),(px))
# define MA_MapPixel_RGB(pf,r,g,b)            MA_MapPixel32_RGB8((pf),(r),(g),(b))
# define MA_MapPixel_RGBA(pf,r,g,b,a)         MA_MapPixel32_RGBA8((pf),(r),(g),(b),(a))
# define MA_MapPixel_RGB8(pf,r,g,b)           MA_MapPixel32_RGB8((pf),(r),(g),(b))
# define MA_MapPixel_RGBA8(pf,r,g,b,a)        MA_MapPixel32_RGBA8((pf),(r),(g),(b),(a))
# define MA_MapPixel_RGB16(pf,r,g,b)          MA_MapPixel32_RGB16((pf),(r),(g),(b))
# define MA_MapPixel_RGBA16(pf,r,g,b,a)       MA_MapPixel32_RGBA16((pf),(r),(g),(b),(a))
# define MA_SurfaceBlendRGB(s,x,y,r,g,b,a,fn) MA_SurfaceBlendRGB8((s),(x),(y),(r),(g),(b),(a),(fn))
# define MA_GetColor_RGB8(px,pf,r,g,b)        MA_GetColor32_RGB8((px),(pf),(r),(g),(b))
# define MA_GetColor_RGBA8(px,pf,r,g,b,a)     MA_GetColor32_RGBA8((px),(pf),(r),(g),(b),(a))
# define MA_GetColor_RGB16(px,pf,r,g,b)       MA_GetColor32_RGB16((px),(pf),(r),(g),(b))
# define MA_GetColor_RGBA16(px,pf,r,g,b,a)    MA_GetColor32_RGBA16((px),(pf),(r),(g),(b),(a))

Sint8 MA_PixelFormatCompare(const MA_PixelFormat *_Nonnull,
                            const MA_PixelFormat *_Nonnull)
			   _Pure_Attribute;

Uint32 ag_map_pixel32(const MA_PixelFormat *_Nonnull, const MA_Color *_Nonnull)
                     _Pure_Attribute;

Uint16 MA_SurfaceGet16_At(const MA_Surface *_Nonnull, const Uint8 *_Nonnull)
                         _Pure_Attribute;
Uint16 MA_SurfaceGet16(const MA_Surface *_Nonnull, Uint8,Uint8)
                      _Pure_Attribute;

void   MA_SurfacePut16(MA_Surface *_Nonnull, Uint8,Uint8, Uint16);
void   MA_SurfacePut16_At(MA_Surface *_Nonnull, Uint8 *_Nonnull, Uint16);

void MA_SurfaceBlend(MA_Surface *_Nonnull, Uint8,Uint8, const MA_Color *_Nonnull);
void MA_SurfaceBlendAt(MA_Surface *_Nonnull, Uint8 *_Nonnull,
                       const MA_Color *_Nonnull);

__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_SURFACE_H_ */

/*	Public domain	*/

#ifndef _AGAR_GUI_SURFACE_H_
#define _AGAR_GUI_SURFACE_H_

#include <agar/gui/geometry.h>
#include <agar/gui/colors.h>

#include <agar/gui/begin.h>

/* Palette of AG_Color values */
typedef struct ag_palette {
	AG_Color *colors;		/* Color array */
	Uint     nColors;		/* Color count */
} AG_Palette;

/* Pixel storage information */
typedef struct ag_pixel_format {
	AG_Palette *palette;			/* For indexed formats */
	Uint8       BitsPerPixel;		/* Depth (bits/pixel) */
	Uint8       BytesPerPixel;		/* Depth (bytes/pixel) */

	Uint8       Rloss, Gloss, Bloss, Aloss;		/* Component loss */
	Uint8       Rshift, Gshift, Bshift, Ashift;	/* Component shift */
	Uint32      Rmask, Gmask, Bmask, Amask;		/* Component masks */

	Uint32 colorkey;			/* Color key pixel */
	Uint8 alpha;				/* Per-surface alpha value */
} AG_PixelFormat;

enum ag_surface_type {
	AG_SURFACE_PACKED,		/* Packed-pixel format */
	AG_SURFACE_INDEXED		/* Indexed format */
};

/* General packed-pixel or indexed surface. */
typedef struct ag_surface {
	enum ag_surface_type type;	/* Type of surface */
	AG_PixelFormat *format;		/* Pixel format */
	Uint flags;
#define AG_SRCCOLORKEY		0x01	/* Enable color key for blit as src */
#define AG_SRCALPHA		0x02	/* Enable alpha for blit as src */
#define AG_SURFACE_GLTEXTURE	0x04	/* Use directly as OpenGL texture */
#define AG_SAVED_SURFACE_FLAGS	(AG_SRCCOLORKEY|AG_SRCALPHA|AG_SURFACE_GLTEXTURE)
	Uint w, h;			/* Size in pixels */
	Uint pitch;			/* Scanline size in bytes */
	void *pixels;			/* Raw pixel data */
	AG_Rect clipRect;		/* Clipping rect for blit as dst */
	Uint padding;			/* Scanline end padding in bytes */
} AG_Surface;

typedef enum ag_blend_func {
	AG_ALPHA_ZERO,
	AG_ALPHA_ONE,
	AG_ALPHA_SRC,
	AG_ALPHA_DST,
	AG_ALPHA_ONE_MINUS_DST,
	AG_ALPHA_ONE_MINUS_SRC,
	AG_ALPHA_OVERLAY
} AG_BlendFn;

/* Clipping test for pixel at ax,ay in surface s (as dst) */
#define AG_CLIPPED_PIXEL(s, ax, ay)			\
	((ax) < (s)->clipRect.x ||			\
	 (ax) >= (s)->clipRect.x+(s)->clipRect.w ||	\
	 (ay) < (s)->clipRect.y ||			\
	 (ay) >= (s)->clipRect.y+(s)->clipRect.h)

#define AG_ALPHA_TRANSPARENT	0		/* Transparent alpha value */
#define AG_ALPHA_OPAQUE		255		/* Opaque alpha value */

/* Flags for AG_SurfaceExportPNG() */
#define AG_EXPORT_PNG_ADAM7	0x01		/* Enable Adam7 interlacing */

/* Flags for AG_SurfaceExportJPEG() */
#define AG_EXPORT_JPEG_JDCT_ISLOW	0x01	/* Slow, accurate integer DCT */
#define AG_EXPORT_JPEG_JDCT_IFAST	0x02	/* Faster, less accurate integer DCT */
#define AG_EXPORT_JPEG_JDCT_FLOAT	0x04	/* Floating-point method */

__BEGIN_DECLS
extern const char *agBlendFuncNames[];	/* For enum ag_blend_func */
extern AG_PixelFormat *agSurfaceFmt;  /* Recommended format for new surfaces */
extern AG_PixelFormat *agTextureFmt;  /* Recommended format for textures */

AG_PixelFormat *AG_PixelFormatRGB(int, Uint32, Uint32, Uint32);
AG_PixelFormat *AG_PixelFormatRGBA(int, Uint32, Uint32, Uint32, Uint32);
AG_PixelFormat *AG_PixelFormatIndexed(int);
AG_PixelFormat *AG_PixelFormatDup(const AG_PixelFormat *);
void            AG_PixelFormatFree(AG_PixelFormat *);
int             AG_PixelFormatComparePalettes(const AG_Palette *, const AG_Palette *);

AG_Surface     *AG_SurfaceNew(enum ag_surface_type, Uint, Uint,
                              const AG_PixelFormat *, Uint);
AG_Surface     *AG_SurfaceEmpty(void);
AG_Surface     *AG_SurfaceIndexed(Uint, Uint, int, Uint);
AG_Surface     *AG_SurfaceRGB(Uint, Uint, int, Uint, Uint32, Uint32, Uint32);
AG_Surface     *AG_SurfaceRGBA(Uint, Uint, int, Uint, Uint32, Uint32, Uint32,
                               Uint32);
AG_Surface     *AG_SurfaceFromPixelsRGB(const void *, Uint, Uint, int, Uint32,
                                        Uint32, Uint32);
AG_Surface     *AG_SurfaceFromPixelsRGBA(const void *, Uint, Uint, int, Uint32,
                                         Uint32, Uint32, Uint32);
AG_Surface     *AG_SurfaceStdGL(Uint, Uint);

int             AG_SurfaceSetPalette(AG_Surface *, AG_Color *, Uint, Uint);
AG_Surface     *AG_SurfaceDup(const AG_Surface *);
AG_Surface     *AG_SurfaceConvert(const AG_Surface *, const AG_PixelFormat *);
void            AG_SurfaceCopy(AG_Surface *, const AG_Surface *);
void            AG_SurfaceBlit(const AG_Surface *, const AG_Rect *,
                               AG_Surface *, int, int);
int             AG_SurfaceResize(AG_Surface *, Uint, Uint);
void            AG_SurfaceFree(AG_Surface *);

AG_Surface     *AG_SurfaceFromFile(const char *);
int             AG_SurfaceExportFile(const AG_Surface *, const char *);

AG_Surface     *AG_SurfaceFromSDL(void *);
void           *AG_SurfaceExportSDL(const AG_Surface *);

AG_Surface     *AG_ReadSurfaceFromBMP(AG_DataSource *);
AG_Surface     *AG_SurfaceFromBMP(const char *);
int             AG_SurfaceExportBMP(const AG_Surface *, const char *);

AG_Surface     *AG_ReadSurfaceFromPNG(AG_DataSource *);
AG_Surface     *AG_SurfaceFromPNG(const char *);
int             AG_SurfaceExportPNG(const AG_Surface *, const char *, Uint);

AG_Surface     *AG_ReadSurfaceFromJPEG(AG_DataSource *);
AG_Surface     *AG_SurfaceFromJPEG(const char *);
int             AG_SurfaceExportJPEG(const AG_Surface *, const char *, Uint, Uint);

void   AG_SurfaceBlendPixel(AG_Surface *, Uint8 *, AG_Color, AG_BlendFn);
void   AG_RGB2HSV(Uint8, Uint8, Uint8, float *, float *, float *);
void   AG_HSV2RGB(float, float, float, Uint8 *, Uint8 *, Uint8 *);
int    AG_ScaleSurface(const AG_Surface *, Uint16, Uint16, AG_Surface **);
void   AG_SetAlphaPixels(AG_Surface *, Uint8);
void   AG_FillRect(AG_Surface *, const AG_Rect *, AG_Color);
Uint32 AG_MapPixelIndexedRGB(const AG_PixelFormat *, Uint8, Uint8, Uint8);
Uint32 AG_MapPixelIndexedRGBA(const AG_PixelFormat *, Uint8, Uint8, Uint8,
                              Uint8);

#define AG_SurfaceStdRGB(w,h) \
	AG_SurfaceRGB((w),(h),agSurfaceFmt->BitsPerPixel,0, \
	    agSurfaceFmt->Rmask, \
	    agSurfaceFmt->Gmask, \
	    agSurfaceFmt->Bmask)
#define AG_SurfaceStdRGBA(w,h) \
	AG_SurfaceRGBA((w),(h),agSurfaceFmt->BitsPerPixel,0, \
	    agSurfaceFmt->Rmask, \
	    agSurfaceFmt->Gmask, \
	    agSurfaceFmt->Bmask, \
	    agSurfaceFmt->Amask)

/*
 * Generic pixel manipulation macros.
 */
#define AG_GET_PIXEL(s, p) AG_GetPixel((s),(p))
#define AG_GET_PIXEL2(s, x, y)						\
	AG_GetPixel((s),(Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
	    (x)*(s)->format->BytesPerPixel)

#define AG_PUT_PIXEL(s, p, c) AG_SurfacePutPixel((s),(p),(c))
#define AG_PUT_PIXEL2(s, x, y, c) do {					\
	AG_SurfacePutPixel((s),						\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    (c));							\
} while (0)
#define AG_PUT_PIXEL2_CLIPPED(s, x, y, c) do {				\
	if (!AG_CLIPPED_PIXEL((s), (x), (y))) {				\
		AG_SurfacePutPixel((s),					\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    (c));						\
	}								\
} while (0)

#define AG_BLEND_RGBA(s, p, r, g, b, a, fn) \
	AG_SurfaceBlendPixel((s),(p), \
	    AG_ColorRGBA((r),(g),(b),(a)),(fn))
#define AG_BLEND_RGBA2(s, x, y, r, g, b, a, fn) do {			\
	AG_SurfaceBlendPixel((s),					\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    AG_ColorRGBA((r),(g),(b),(a)),(fn));					\
} while (0)
#define AG_BLEND_RGBA2_CLIPPED(s, x, y, r, g, b, a, fn) do {		\
	if (!AG_CLIPPED_PIXEL((s), (x), (y))) {				\
		AG_SurfaceBlendPixel((s),				\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    AG_ColorRGBA((r),(g),(b),(a)),(fn));				\
	}								\
} while (0)

/* Compose a pixel value from RGB components. */
static __inline__ Uint32
AG_MapPixelRGB(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b)
{
	if (pf->palette != NULL) {
		return AG_MapPixelIndexedRGB(pf, r, g, b);
	}
	return (r >> pf->Rloss) << pf->Rshift |
	       (g >> pf->Gloss) << pf->Gshift |
	       (b >> pf->Bloss) << pf->Bshift | pf->Amask;
}

/* Compose a pixel value from RGBA components. */
static __inline__ Uint32
AG_MapPixelRGBA(const AG_PixelFormat *pf, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (pf->palette != NULL) {
		return AG_MapPixelIndexedRGBA(pf, r, g, b, a);
	}
	return (r >> pf->Rloss) << pf->Rshift |
	       (g >> pf->Gloss) << pf->Gshift |
	       (b >> pf->Bloss) << pf->Bshift |
	      ((a >> pf->Aloss) << pf->Ashift & pf->Amask);
}

/* Compose a pixel value from an AG_Color (ignore any alpha). */
static __inline__ Uint32
AG_MapColorRGB(const AG_PixelFormat *pf, AG_Color C)
{
	if (pf->palette != NULL) {
		return AG_MapPixelIndexedRGB(pf, C.r, C.g, C.b);
	}
	return (C.r >> pf->Rloss) << pf->Rshift |
	       (C.g >> pf->Gloss) << pf->Gshift |
	       (C.b >> pf->Bloss) << pf->Bshift | pf->Amask;
}

/* Compose a pixel value from an AG_Color (honor any alpha). */
static __inline__ Uint32
AG_MapColorRGBA(const AG_PixelFormat *pf, AG_Color C)
{
	if (pf->palette != NULL) {
		return AG_MapPixelIndexedRGBA(pf, C.r, C.g, C.b, C.a);
	}
	return (C.r >> pf->Rloss) << pf->Rshift |
	       (C.g >> pf->Gloss) << pf->Gshift |
	       (C.b >> pf->Bloss) << pf->Bshift |
	      ((C.a >> pf->Aloss) << pf->Ashift & pf->Amask);
}

#define AG_GET_PIXEL_COMPONENT(rv, mask, shift, loss)			\
	tmp = (pc & mask) >> shift;					\
	(rv) = (tmp << loss) + (tmp >> (8 - (loss << 1)));

/* Decompose a pixel value to RGB components. */
static __inline__ void
AG_GetPixelRGB(Uint32 pc, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g,
    Uint8 *b)
{
	Uint tmp;

	if (pf->palette != NULL) {
		AG_Color *C = &pf->palette->colors[(Uint)pc % pf->palette->nColors];
		*r = C->r;
		*g = C->g;
		*b = C->b;
	} else {
		AG_GET_PIXEL_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss);
		AG_GET_PIXEL_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss);
		AG_GET_PIXEL_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss);
	}
}

/* Decompose a pixel value to RGBA components. */
static __inline__ void
AG_GetPixelRGBA(Uint32 pc, const AG_PixelFormat *pf, Uint8 *r, Uint8 *g,
    Uint8 *b, Uint8 *a)
{
	Uint tmp;

	if (pf->palette != NULL) {
		AG_Color *C = &pf->palette->colors[(Uint)pc % pf->palette->nColors];
		*r = C->r;
		*g = C->g;
		*b = C->b;
		*a = C->a;
	} else {
		AG_GET_PIXEL_COMPONENT(*r, pf->Rmask, pf->Rshift, pf->Rloss);
		AG_GET_PIXEL_COMPONENT(*g, pf->Gmask, pf->Gshift, pf->Gloss);
		AG_GET_PIXEL_COMPONENT(*b, pf->Bmask, pf->Bshift, pf->Bloss);
		if (pf->Amask != 0) {
			AG_GET_PIXEL_COMPONENT(*a, pf->Amask, pf->Ashift, pf->Aloss);
		} else {
			*a = AG_ALPHA_OPAQUE;
		}
	}
}

/* Decompose a pixel value to an AG_Color (ignore any alpha, set opaque). */
static __inline__ AG_Color
AG_GetColorRGB(Uint32 pc, const AG_PixelFormat *pf)
{
	AG_Color C;
	Uint tmp;

	if (pf->palette != NULL) {
		return (pf->palette->colors[(Uint)pc % pf->palette->nColors]);
	}
	AG_GET_PIXEL_COMPONENT(C.r, pf->Rmask, pf->Rshift, pf->Rloss);
	AG_GET_PIXEL_COMPONENT(C.g, pf->Gmask, pf->Gshift, pf->Gloss);
	AG_GET_PIXEL_COMPONENT(C.b, pf->Bmask, pf->Bshift, pf->Bloss);
	C.a = AG_ALPHA_OPAQUE;
	return (C);
}

/* Decompose a pixel value to an AG_Color (honor any alpha). */
static __inline__ AG_Color
AG_GetColorRGBA(Uint32 pc, const AG_PixelFormat *pf)
{
	AG_Color C;
	Uint tmp;

	if (pf->palette != NULL) {
		return (pf->palette->colors[(Uint)pc % pf->palette->nColors]);
	}
	AG_GET_PIXEL_COMPONENT(C.r, pf->Rmask, pf->Rshift, pf->Rloss);
	AG_GET_PIXEL_COMPONENT(C.g, pf->Gmask, pf->Gshift, pf->Gloss);
	AG_GET_PIXEL_COMPONENT(C.b, pf->Bmask, pf->Bshift, pf->Bloss);
	if (pf->Amask != 0) {
		AG_GET_PIXEL_COMPONENT(C.a, pf->Amask, pf->Ashift, pf->Aloss);
	} else {
		C.a = AG_ALPHA_OPAQUE;
	}
	return (C);
}

#undef AG_GET_PIXEL_COMPONENT

/* Return pixel value at specified position in surface s. */
static __inline__ Uint32
AG_GetPixel(const AG_Surface *s, const Uint8 *pSrc)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		return (*(Uint32 *)pSrc);
	case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		return ((pSrc[0] << 16) +
		        (pSrc[1] << 8) +
		         pSrc[2]);
#else
		return  (pSrc[0] +
		        (pSrc[1] << 8) +
		        (pSrc[2] << 16));
#endif
	case 2:
		return (*(Uint16 *)pSrc);
	}
	return (*pSrc);
}

/* Write pixel value at specified position in surface s. */
static __inline__ void
AG_SurfacePutPixel(AG_Surface *s, Uint8 *pDst, Uint32 cDst)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)pDst = cDst;
		break;
	case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		pDst[0] = (cDst>>16) & 0xff;
		pDst[1] = (cDst>>8) & 0xff;
		pDst[2] = cDst & 0xff;
#else
		pDst[2] = (cDst>>16) & 0xff;
		pDst[1] = (cDst>>8) & 0xff;
		pDst[0] = cDst & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)pDst = (Uint16)cDst;
		break;
	default:
		*pDst = (Uint8)cDst;
		break;
	}
}

/*
 * Test whether two pixel formats are identical. If both formats are
 * color-index, compare the palettes as well.
 */
static __inline__ int
AG_PixelFormatCompare(const AG_PixelFormat *pf1, const AG_PixelFormat *pf2)
{
	if ((pf1->palette != NULL && pf2->palette == NULL) ||
	    (pf1->palette == NULL && pf2->palette != NULL)) {
		return (1);
	}
	if (pf1->palette != NULL && pf2->palette != NULL &&
	    AG_PixelFormatComparePalettes(pf1->palette, pf2->palette) != 0) {
		return (1);
	}
	return !(pf1->BitsPerPixel == pf2->BitsPerPixel &&
	         pf1->Rmask == pf2->Rmask &&
		 pf1->Gmask == pf2->Gmask &&
		 pf1->Bmask == pf2->Bmask &&
		 pf1->Amask == pf2->Amask &&
		 pf1->colorkey == pf2->colorkey);
}

/* Set the source alpha flag and per-surface alpha. */
static __inline__ void
AG_SurfaceSetAlpha(AG_Surface *s, Uint flags, Uint8 alpha)
{
	if (flags & AG_SRCALPHA) {
		s->flags |= AG_SRCALPHA;
	} else {
		s->flags &= ~(AG_SRCALPHA);
	}
	s->format->alpha = alpha;
}

/* Set the source colorkey flag and per-surface colorkey. */
static __inline__ void
AG_SurfaceSetColorKey(AG_Surface *s, Uint flags, Uint32 colorkey)
{
	if (flags & AG_SRCCOLORKEY) {
		s->flags |= AG_SRCCOLORKEY;
	} else {
		s->flags &= ~(AG_SRCCOLORKEY);
	}
	s->format->colorkey = colorkey;
}

/* Retrieve the surface's clipping rectangle. */
static __inline__ void
AG_GetClipRect(const AG_Surface *su, AG_Rect *r)
{
	*r = su->clipRect;
}

/* Set the surface's clipping rectangle. */
static __inline__ void
AG_SetClipRect(AG_Surface *su, const AG_Rect *r)
{
	su->clipRect = *r;
}

#ifdef AG_LEGACY
void        AG_SurfaceLock(AG_Surface *)							DEPRECATED_ATTRIBUTE;
void        AG_SurfaceUnlock(AG_Surface *)							DEPRECATED_ATTRIBUTE;
Uint32      AG_MapRGB(const AG_PixelFormat *, Uint8, Uint8, Uint8)				DEPRECATED_ATTRIBUTE;
Uint32      AG_MapRGBA(const AG_PixelFormat *, Uint8, Uint8, Uint8, Uint8)			DEPRECATED_ATTRIBUTE;
void        AG_GetRGB(Uint32, const AG_PixelFormat *, Uint8 *, Uint8 *, Uint8 *)		DEPRECATED_ATTRIBUTE;
void        AG_GetRGBA(Uint32, const AG_PixelFormat *, Uint8 *, Uint8 *, Uint8 *, Uint8 *)	DEPRECATED_ATTRIBUTE;
AG_Surface *AG_DupSurface(AG_Surface *)								DEPRECATED_ATTRIBUTE;
int         AG_SamePixelFmt (const AG_Surface *, const AG_Surface *)				DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SURFACE_H_ */

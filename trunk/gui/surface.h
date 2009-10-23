/*	Public domain	*/

#ifndef _AGAR_GUI_SURFACE_H_
#define _AGAR_GUI_SURFACE_H_
#include <agar/gui/begin.h>

/* XXX */
#undef HAVE_SNPRINTF
#undef HAVE_VSNPRINTF
#undef HAVE_SYS_TYPES_H
#undef HAVE_STDIO_H
#undef HAVE_STDLIB_H
#undef HAVE_STDARG_H
#undef Uint8
#undef Sint8
#undef Uint16
#undef Sint16
#undef Uint32
#undef Sint32
#undef Uint64
#undef Sint64
#include <SDL.h>

/* For transition to Agar-1.4 */
typedef SDL_Surface AG_Surface;
typedef SDL_PixelFormat AG_PixelFormat;
typedef SDL_Palette AG_Palette;

typedef enum ag_blend_func {
	AG_ALPHA_ZERO,
	AG_ALPHA_ONE,
	AG_ALPHA_SRC,
	AG_ALPHA_DST,
	AG_ALPHA_ONE_MINUS_DST,
	AG_ALPHA_ONE_MINUS_SRC,
	AG_ALPHA_OVERLAY
} AG_BlendFn;

/* Per-surface clipping test */
#define AG_CLIPPED_PIXEL(s, ax, ay)			\
	((ax) < (s)->clip_rect.x ||			\
	 (ax) >= (s)->clip_rect.x+(s)->clip_rect.w ||	\
	 (ay) < (s)->clip_rect.y ||			\
	 (ay) >= (s)->clip_rect.y+(s)->clip_rect.h)


#define AG_SWSURFACE		SDL_SWSURFACE
#define AG_HWSURFACE		SDL_HWSURFACE
#define AG_SRCCOLORKEY		SDL_SRCCOLORKEY
#define AG_SRCALPHA		SDL_SRCALPHA
#define AG_RLEACCEL		SDL_RLEACCEL
#define AG_ALPHA_TRANSPARENT	SDL_ALPHA_TRANSPARENT
#define AG_ALPHA_OPAQUE		SDL_ALPHA_OPAQUE
#define AG_LOGPAL		SDL_LOGPAL
#define AG_PHYSPAL		SDL_PHYSPAL

__BEGIN_DECLS
extern AG_PixelFormat *agSurfaceFmt;		/* Standard surface format */

void AG_SurfaceBlendPixelRGBA(AG_Surface *, Uint8 *, Uint8, Uint8, Uint8, Uint8,
                              AG_BlendFn);
void AG_RGB2HSV(Uint8, Uint8, Uint8, float *, float *, float *);
void AG_HSV2RGB(float, float, float, Uint8 *, Uint8 *, Uint8 *);

AG_PixelFormat *AG_PixelFormatRGB(int, Uint32, Uint32, Uint32);
AG_PixelFormat *AG_PixelFormatRGBA(int, Uint32, Uint32, Uint32, Uint32);
AG_PixelFormat *AG_PixelFormatIndexed(int);
AG_PixelFormat *AG_PixelFormatDup(const AG_PixelFormat *);
void            AG_PixelFormatFree(AG_PixelFormat *);

AG_Surface     *AG_SurfaceNew(Uint, Uint, AG_PixelFormat *, Uint);
AG_Surface     *AG_SurfaceEmpty(void);
AG_Surface     *AG_SurfaceIndexed(Uint, Uint, int, Uint);
AG_Surface     *AG_SurfaceRGB(Uint, Uint, int, Uint, Uint32, Uint32, Uint32);
AG_Surface     *AG_SurfaceRGBA(Uint, Uint, int, Uint, Uint32, Uint32, Uint32,
                               Uint32);
AG_Surface     *AG_SurfaceFromPixelsRGB(void *, Uint, Uint, int, int, Uint32,
                                        Uint32, Uint32);
AG_Surface     *AG_SurfaceFromPixelsRGBA(void *, Uint, Uint, int, int, Uint32,
                                         Uint32, Uint32, Uint32);
AG_Surface     *AG_SurfaceFromSDL(SDL_Surface *);
SDL_Surface    *AG_SurfaceToSDL(AG_Surface *);
AG_Surface     *AG_SurfaceFromSurface(AG_Surface *, AG_PixelFormat *, Uint);
AG_Surface     *AG_SurfaceFromBMP(const char *);
void            AG_SurfaceCopy(AG_Surface *, AG_Surface *);
void            AG_SurfaceFree(AG_Surface *);

#define AG_SurfaceStdRGB(w,h)		AG_SurfaceRGB((w),(h),agSurfaceFmt->BitsPerPixel,0,agSurfaceFmt->Rmask,agSurfaceFmt->Gmask,agSurfaceFmt->Bmask)
#define AG_SurfaceStdRGBA(w,h)		AG_SurfaceRGBA((w),(h),agSurfaceFmt->BitsPerPixel,0,agSurfaceFmt->Rmask,agSurfaceFmt->Gmask,agSurfaceFmt->Bmask,agSurfaceFmt->Amask)

#define AG_SurfaceLock(su)		SDL_LockSurface(su)
#define AG_SurfaceUnlock(su)		SDL_UnlockSurface(su)
#define AG_SetColorKey(su,f,key)	SDL_SetColorKey((SDL_Surface *)(su),(f),(key))
#define AG_SetAlpha(su,f,a)		SDL_SetAlpha((SDL_Surface *)(su),(f),(a))
#define AG_SetPalette(su,w,c,s,n)	SDL_SetPalette((SDL_Surface *)(su),(w),(SDL_Color *)(c),(s),(n))
#define AG_MapRGB(fmt,r,g,b)		SDL_MapRGB((SDL_PixelFormat *)(fmt),(r),(g),(b))
#define AG_MapRGBA(fmt,r,g,b,a)		SDL_MapRGBA((SDL_PixelFormat *)(fmt),(r),(g),(b),(a))
#define AG_MapColorRGB(fmt,c)		SDL_MapRGB((SDL_PixelFormat *)(fmt),(c)->r,(c)->g,(c)->b)
#define AG_MapColorRGBA(fmt,c)		SDL_MapRGBA((SDL_PixelFormat *)(fmt),(c)->r,(c)->g,(c)->b,(c)->a)
#define AG_GetRGB(pixel,fmt,r,g,b)	SDL_GetRGB((pixel),(SDL_PixelFormat *)(fmt),(r),(g),(b))
#define AG_GetRGBA(pixel,fmt,r,g,b,a)	SDL_GetRGBA((pixel),(SDL_PixelFormat *)(fmt),(r),(g),(b),(a))
#define AG_GetColorRGB(pixel,fmt,c)	SDL_GetRGB((pixel),(SDL_PixelFormat *)(fmt),(c)->r,(c)->g,(c)->b)
#define AG_GetColorRGBA(pixel,fmt,c)	SDL_GetRGBA((pixel),(SDL_PixelFormat *)(fmt),(c)->r,(c)->g,(c)->b,(c)->a)

AG_Surface *AG_DupSurface(AG_Surface *);
int         AG_ScaleSurface(AG_Surface *, Uint16, Uint16, AG_Surface **);
void        AG_SetAlphaPixels(AG_Surface *, Uint8);
int         AG_SurfaceExportJPEG(AG_Surface *, char *);
void        AG_FlipSurface(Uint8 *, int, int);

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

#define AG_BLEND_RGBA(s, p, r, g, b, a, m) \
	AG_SurfaceBlendPixelRGBA((s),(p),(r),(g),(b),(a),(m))
#define AG_BLEND_RGBA2(s, x, y, r, g, b, a, m) do {			\
	AG_SurfaceBlendPixelRGBA((s),					\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    (r),(g),(b),(a),(m));					\
} while (0)
#define AG_BLEND_RGBA2_CLIPPED(s, x, y, r, g, b, a, m) do {		\
	if (!AG_CLIPPED_PIXEL((s), (x), (y))) {				\
		AG_SurfaceBlendPixelRGBA((s),				\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    (r),(g),(b),(a),(m));				\
	}								\
} while (0)

/* Return pixel value at specified position in surface s. */
static __inline__ Uint32
AG_GetPixel(AG_Surface *s, Uint8 *pSrc)
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
		*(Uint16 *)pDst = cDst;
		break;
	default:
		*pDst = cDst;
		break;
	}
}

/* Convert an AG_Rect to an equivalent SDL_Rect. XXX */
static __inline__ SDL_Rect
AG_RectToSDL(const AG_Rect *r)
{
	SDL_Rect rs;
	rs.x = (Sint16)r->x;
	rs.y = (Sint16)r->y;
	rs.w = (Uint16)r->w;
	rs.h = (Uint16)r->h;
	return (rs);
}

/* Convert a SDL_Rect to an equivalent AG_Rect. XXX */
static __inline__ AG_Rect
AG_RectFromSDL(const SDL_Rect *r)
{
	AG_Rect rs;
	rs.x = (int)r->x;
	rs.y = (int)r->y;
	rs.w = (int)r->w;
	rs.h = (int)r->h;
	return (rs);
}


/* Test whether two surfaces use identical pixel formats. */
static __inline__ int
AG_SamePixelFmt(AG_Surface *s1, AG_Surface *s2)
{
	return (s1->format->BytesPerPixel == s2->format->BytesPerPixel &&
	        s1->format->Rmask == s2->format->Rmask &&
		s1->format->Gmask == s2->format->Gmask &&
		s1->format->Bmask == s2->format->Bmask &&
		s1->format->Amask == s2->format->Amask &&
		s1->format->colorkey == s2->format->colorkey);
}

/*
 * Copy the contents a surface (or a region within a surface) to a given
 * position in another surface.
 */
static __inline__ void
AG_SurfaceBlit(AG_Surface *src, const AG_Rect *rSrc, AG_Surface *dst,
    int xDst, int yDst)
{
	SDL_Rect rs, rd;

	rd.x = (Sint16)xDst;
	rd.y = (Sint16)yDst;

	if (rSrc != NULL) {
		rs.x = (Sint16)rSrc->x;
		rs.y = (Sint16)rSrc->y;
		rs.w = (Uint16)rSrc->w;
		rs.h = (Uint16)rSrc->h;
		SDL_BlitSurface(src, &rs, dst, &rd);
	} else {
		SDL_BlitSurface(src, NULL, dst, &rd);
	}
}

/*
 * Fill rectangle with the specified color.
 * The alpha component is copied as-is.
 */
static __inline__ void
AG_FillRect(AG_Surface *s, const AG_Rect *r, AG_Color C)
{
	Uint32 c;
	SDL_Rect rSDL;

	c = AG_MapRGBA(s->format, C.r, C.g, C.b, C.a);
	if (r != NULL) {
		rSDL = AG_RectToSDL(r);
		SDL_FillRect(s, &rSDL, c);
	} else {
		SDL_FillRect(s, NULL, c);
	}
}

/*
 * Get/set the clipping rectangle of a Surface. The clipping rectangle applies
 * to AG_SurfaceBlit()s where surface s is the destination of the blit.
 */
static __inline__ void
AG_GetClipRect(AG_Surface *s, AG_Rect *r)
{
	SDL_Rect rSDL;
	SDL_GetClipRect(s, &rSDL);
	r->x = (int)rSDL.x;
	r->y = (int)rSDL.y;
	r->w = (int)rSDL.w;
	r->h = (int)rSDL.h;
}
static __inline__ void
AG_SetClipRect(AG_Surface *s, const AG_Rect *r)
{
	SDL_Rect rSDL;
	rSDL.x = (Sint16)r->x;
	rSDL.y = (Sint16)r->y;
	rSDL.w = (Sint16)r->w;
	rSDL.h = (Sint16)r->h;
	SDL_SetClipRect(s, &rSDL);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SURFACE_H_ */

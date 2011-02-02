/*	Public domain	*/

#ifndef _AGAR_GUI_ANIM_H_
#define _AGAR_GUI_ANIM_H_

#include <agar/gui/begin.h>

enum ag_anim_type {
	AG_ANIM_PACKED,		/* Packed-pixel format */
	AG_ANIM_INDEXED		/* Indexed format */
};

/* Animated surface structure. */
typedef struct ag_anim {
	enum ag_anim_type type;		/* Type of animation */
	AG_PixelFormat *format;		/* Pixel format */
	Uint flags;
/* #define AG_SRCCOLORKEY 0x01 */
/* #define AG_SRCALPHA    0x02 */
#define AG_SAVED_ANIM_FLAGS (AG_SRCCOLORKEY|AG_SRCALPHA)
	Uint w, h;			/* Size in pixels */
	Uint n;				/* Number of frames */
	Uint pitch;			/* Scanline size in bytes */
	void **pixels;			/* Raw frame/pixel data */
	AG_Rect clipRect;		/* Clipping rect for blit as dst */
} AG_Anim;

__BEGIN_DECLS
AG_Anim    *AG_AnimNew(enum ag_anim_type, Uint, Uint, const AG_PixelFormat *,
                       Uint);
AG_Anim    *AG_AnimEmpty(void);
AG_Anim    *AG_AnimIndexed(Uint, Uint, int, Uint);
AG_Anim    *AG_AnimRGB(Uint, Uint, int, Uint, Uint32, Uint32, Uint32);
AG_Anim    *AG_AnimRGBA(Uint, Uint, int, Uint, Uint32, Uint32, Uint32, Uint32);
AG_Anim    *AG_AnimFromPNGs(const char *);
AG_Anim    *AG_AnimFromJPEGs(const char *);
int         AG_AnimSetPalette(AG_Anim *, AG_Color *, Uint, Uint);
AG_Anim    *AG_AnimDup(const AG_Anim *);
int         AG_AnimResize(AG_Anim *, Uint, Uint);
void        AG_AnimFree(AG_Anim *);

int         AG_AnimFrameNew(AG_Anim *, const AG_Surface *);
AG_Surface *AG_AnimFrameToSurface(const AG_Anim *, int);

#define AG_AnimStdRGB(w,h) \
	AG_AnimRGB((w),(h),agSurfaceFmt->BitsPerPixel,0, \
	    agSurfaceFmt->Rmask, \
	    agSurfaceFmt->Gmask, \
	    agSurfaceFmt->Bmask)
#define AG_AnimStdRGBA(w,h) \
	AG_AnimRGBA((w),(h),agSurfaceFmt->BitsPerPixel,0, \
	    agSurfaceFmt->Rmask, \
	    agSurfaceFmt->Gmask, \
	    agSurfaceFmt->Bmask, \
	    agSurfaceFmt->Amask)

static __inline__ Uint32
AG_AnimGetPixel(const AG_Anim *an, const Uint8 *pSrc)
{
	switch (an->format->BytesPerPixel) {
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

static __inline__ void
AG_AnimPutPixel(AG_Anim *an, Uint8 *pDst, Uint32 cDst)
{
	switch (an->format->BytesPerPixel) {
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

/* Set the source alpha flag and per-animation alpha. */
static __inline__ void
AG_AnimSetAlpha(AG_Anim *an, Uint flags, Uint8 alpha)
{
	if (flags & AG_SRCALPHA) {
		an->flags |= AG_SRCALPHA;
	} else {
		an->flags &= ~(AG_SRCALPHA);
	}
	an->format->alpha = alpha;
}

/* Set the source colorkey flag and per-animation colorkey. */
static __inline__ void
AG_AnimSetColorKey(AG_Anim *an, Uint flags, Uint32 colorkey)
{
	if (flags & AG_SRCCOLORKEY) {
		an->flags |= AG_SRCCOLORKEY;
	} else {
		an->flags &= ~(AG_SRCCOLORKEY);
	}
	an->format->colorkey = colorkey;
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_ANIM_H_ */

/*	Public domain	*/

#ifndef _AGAR_GUI_ANIM_H_
#define _AGAR_GUI_ANIM_H_

#include <agar/gui/begin.h>

enum ag_anim_type {
	AG_ANIM_PACKED,		/* Packed-pixel format */
	AG_ANIM_INDEXED		/* Indexed format */
};

/* Animation frame. */
typedef struct ag_anim_frame {
	void *pixels;			/* Pixel data */
	Uint flags;
} AG_AnimFrame;

/* Animated surface structure. */
typedef struct ag_anim {
	AG_Mutex lock;
	enum ag_anim_type type;		/* Type of animation */
	AG_PixelFormat *format;		/* Pixel format */
	Uint flags;
/* #define AG_SRCCOLORKEY 0x01 */
/* #define AG_SRCALPHA    0x02 */
#define AG_SAVED_ANIM_FLAGS (AG_SRCCOLORKEY|AG_SRCALPHA)
	Uint w, h;			/* Size in pixels */
	Uint n;				/* Number of frames */
	Uint pitch;			/* Scanline size in bytes */
	AG_AnimFrame *f;		/* Frame data */
	AG_Rect clipRect;		/* Clipping rect for blit as dst */
	double fpsOrig;			/* Original frames/second (hint) */
} AG_Anim;

/* Animation instance (playback status). */
typedef struct ag_anim_state {
	AG_Mutex lock;
	AG_Anim *an;			/* Back pointer to anim */
	Uint flags;
#define AG_ANIM_LOOP	 0x01		/* Loop playback */
#define AG_ANIM_PINGPONG 0x02		/* Loop in ping-pong fashion */
#define AG_ANIM_REVERSE	 0x04		/* Playback in reverse */
	int play;			/* Animation is playing */
	int f;				/* Current frame# */
	double fps;			/* Effective frames/second */
	AG_Thread th;			/* Animation thread */
} AG_AnimState;

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
AG_Anim    *AG_AnimDup(AG_Anim *);
int         AG_AnimResize(AG_Anim *, Uint, Uint);
void        AG_AnimFree(AG_Anim *);

void        AG_AnimStateInit(AG_Anim *, AG_AnimState *);
void        AG_AnimStateDestroy(AG_Anim *, AG_AnimState *);
void        AG_AnimSetOrigFPS(AG_Anim *, double);
void        AG_AnimSetFPS(AG_AnimState *, double);
void        AG_AnimSetLoop(AG_AnimState *, int);
void        AG_AnimSetPingPong(AG_AnimState *, int);
void        AG_AnimSetAlpha(AG_Anim *, Uint, Uint8);
void        AG_AnimSetColorKey(AG_Anim *, Uint, Uint32);

int         AG_AnimPlay(AG_AnimState *);
void        AG_AnimStop(AG_AnimState *);

int         AG_AnimFrameNew(AG_Anim *, const AG_Surface *);
AG_Surface *AG_AnimFrameToSurface(AG_Anim *, int);

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

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_ANIM_H_ */

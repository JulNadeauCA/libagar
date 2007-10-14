/*	Public domain	*/

#ifndef _AGAR_CORE_VIEW_H_
#define _AGAR_CORE_VIEW_H_

#ifdef _AGAR_INTERNAL
#include <config/view_8bpp.h>
#include <config/view_16bpp.h>
#include <config/view_24bpp.h>
#include <config/view_32bpp.h>
#else
#include <agar/config/view_8bpp.h>
#include <agar/config/view_16bpp.h>
#include <agar/config/view_24bpp.h>
#include <agar/config/view_32bpp.h>
#endif
#ifdef _AGAR_INTERNAL
#include <gui/iconmgr.h>
#else
#include <agar/gui/iconmgr.h>
#endif

#include "begin_code.h"

struct ag_window;
TAILQ_HEAD(ag_windowq, ag_window);

typedef struct ag_display {
	struct ag_object obj;

	SDL_Surface *v;			/* Video surface */
	SDL_Surface *stmpl;		/* Reference surface */
	int w, h;			/* Display geometry */
	int depth;			/* Depth in bpp */
	int opengl;			/* OpenGL rendering? (if available) */
	int rCur;			/* Estimated refresh delay in ms */
	Uint rNom;			/* Nominal refresh delay */
	SDL_Rect *dirty;		/* Video rectangles to update */
	Uint	 ndirty;
	Uint  maxdirty;
	AG_Mutex lock_gl;		/* Lock on OpenGL context */
	AG_Mutex lock;
	struct ag_windowq windows;	/* Windows in view */
	struct ag_windowq detach;	/* Windows to free */
	struct ag_window *winToFocus;	/* Give focus to this window,
					   when event processing is done */
	struct ag_window *winSelected;	/* Window being moved/resized/etc */
	struct ag_window **winModal;	/* Modal window stack */
	Uint nModal;
	enum {
		AG_WINOP_NONE,
		AG_WINOP_MOVE,			/* Window movement */
		AG_WINOP_LRESIZE,		/* Window resize */
		AG_WINOP_RRESIZE,
		AG_WINOP_HRESIZE
	} winop;
} AG_Display;

/* Alpha functions for BlendPixelRGBA(). */
typedef enum ag_blend_func {
	AG_ALPHA_OVERLAY,		/* dA = sA+dA (emulated in GL mode) */
	AG_ALPHA_SRC,			/* dA = sA */
	AG_ALPHA_DST,			/* dA = dA */
	AG_ALPHA_ONE_MINUS_DST,		/* dA = 1-dA */
	AG_ALPHA_ONE_MINUS_SRC		/* dA = 1-sA */
} AG_BlendFn;

/* Determine whether a pixel must be clipped or not. */
#define AG_CLIPPED_PIXEL(s, ax, ay)			\
	((ax) < (s)->clip_rect.x ||			\
	 (ax) >= (s)->clip_rect.x+(s)->clip_rect.w ||	\
	 (ay) < (s)->clip_rect.y ||			\
	 (ay) >= (s)->clip_rect.y+(s)->clip_rect.h)
#define AG_NONCLIPPED_PIXEL(s, ax, ay)			\
	((ax) >= (s)->clip_rect.x &&			\
	 (ax) < (s)->clip_rect.x+(s)->clip_rect.w &&	\
	 (ay) >= (s)->clip_rect.y &&			\
	 (ay) < (s)->clip_rect.y+(s)->clip_rect.h)

/*
 * Putpixel macros optimized for the display surface format.
 */

#ifdef VIEW_8BPP
# define AG_VIEW_PUT_PIXEL8(dst, c)	\
case 1:					\
	*(dst) = (c);			\
	break;
#else
# define AG_VIEW_PUT_PIXEL8(dst, c)
#endif

#ifdef VIEW_16BPP
# define AG_VIEW_PUT_PIXEL16(dst, c)	\
case 2:					\
	*(Uint16 *)(dst) = (c);		\
	break;
#else
# define AG_VIEW_PUT_PIXEL16(dst, c)
#endif

#ifdef VIEW_24BPP
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
#  define AG_VIEW_PUT_PIXEL24(dst, c)	\
case 3:					\
	(dst)[0] = ((c)>>16) & 0xff;	\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] =  (c) & 0xff;		\
	break;
# else
#  define AG_VIEW_PUT_PIXEL24(dst, c)	\
case 3:					\
	(dst)[0] =  (c) & 0xff;		\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] = ((c)>>16) & 0xff;	\
	break;
# endif
#else
# define AG_VIEW_PUT_PIXEL24(dst, c)
#endif

#ifdef VIEW_32BPP
# define AG_VIEW_PUT_PIXEL32(dst, c)	\
case 4:					\
	*(Uint32 *)(dst) = (c);		\
	break;
#else
# define AG_VIEW_PUT_PIXEL32(dst, c)
#endif

#define AG_VIEW_PUT_PIXEL(p, c) do {					\
	switch (agVideoFmt->BytesPerPixel) {				\
		AG_VIEW_PUT_PIXEL8((p),  (c))				\
		AG_VIEW_PUT_PIXEL16((p), (c))				\
		AG_VIEW_PUT_PIXEL24((p), (c))				\
		AG_VIEW_PUT_PIXEL32((p), (c))				\
	}								\
} while (0)
#define AG_VIEW_PUT_PIXEL2(vx, vy, c) do {				\
	Uint8 *_view_dst = (Uint8 *)agView->v->pixels +			\
	    (vy)*agView->v->pitch + (vx)*agVideoFmt->BytesPerPixel;	 \
	switch (agVideoFmt->BytesPerPixel) {				 \
		AG_VIEW_PUT_PIXEL8(_view_dst,  (c))			\
		AG_VIEW_PUT_PIXEL16(_view_dst, (c))			\
		AG_VIEW_PUT_PIXEL24(_view_dst, (c))			\
		AG_VIEW_PUT_PIXEL32(_view_dst, (c))			\
	}								\
} while (0)
#define AG_VIEW_PUT_PIXEL2_CLIPPED(vx, vy, c) do {			\
	if (AG_NONCLIPPED_PIXEL(agView->v, (vx), (vy))) {		\
		Uint8 *_view_dst = (Uint8 *)agView->v->pixels +		\
		    (vy)*agView->v->pitch + (vx)*agVideoFmt->BytesPerPixel; \
		switch (agVideoFmt->BytesPerPixel) {			 \
			AG_VIEW_PUT_PIXEL8(_view_dst,  (c))		\
			AG_VIEW_PUT_PIXEL16(_view_dst, (c))		\
			AG_VIEW_PUT_PIXEL24(_view_dst, (c))		\
			AG_VIEW_PUT_PIXEL32(_view_dst, (c))		\
		}							\
	}								\
} while (0)

/*
 * Generic putpixel/getpixel macros.
 */

#define AG_GET_PIXEL(s, p) AG_GetPixel((s),(p))
#define AG_GET_PIXEL2(s, x, y)						\
	AG_GetPixel((s),(Uint8 *)(s)->pixels + (y)*(s)->pitch +	\
	    (x)*(s)->format->BytesPerPixel)

#define AG_PUT_PIXEL(s, p, c) AG_PutPixel((s),(p),(c))
#define AG_PUT_PIXEL2(s, x, y, c) do {					\
	AG_PutPixel((s),						\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    (c));							\
} while (0)
#define AG_PUT_PIXEL2_CLIPPED(s, x, y, c) do {				\
	if (AG_NONCLIPPED_PIXEL((s), (x), (y))) {			\
		AG_PutPixel((s),					\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    (c));						\
	}								\
} while (0)

/*
 * Generic alpha blending macros.
 */

#define AG_BLEND_RGBA(s, p, r, g, b, a, m) \
	AG_BlendPixelRGBA((s),(p),(r),(g),(b),(a),(m))
#define AG_BLEND_RGBA2(s, x, y, r, g, b, a, m) do {			\
	AG_BlendPixelRGBA((s),						\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    (r),(g),(b),(a),(m));					\
} while (0)
#define AG_BLEND_RGBA2_CLIPPED(s, x, y, r, g, b, a, m) do {		\
	if (AG_NONCLIPPED_PIXEL((s), (x), (y))) {			\
		AG_BlendPixelRGBA((s),					\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    (r),(g),(b),(a),(m));				\
	}								\
} while (0)

/* Flags for AG_InitVideo() */
#define AG_VIDEO_HWSURFACE     0x001  /* Request hardware FB */
#define AG_VIDEO_ASYNCBLIT     0x002  /* Multithreaded blits */
#define AG_VIDEO_ANYFORMAT     0x004  /* Disable depth emulation */
#define AG_VIDEO_HWPALETTE     0x008  /* Exclusive palette access */
#define AG_VIDEO_DOUBLEBUF     0x010  /* Double buffering */
#define AG_VIDEO_FULLSCREEN    0x020  /* Start in fullscreen mode */
#define AG_VIDEO_RESIZABLE     0x040  /* Request resizable window */
#define AG_VIDEO_NOFRAME       0x080  /* Request frameless window */
#define AG_VIDEO_BGPOPUPMENU   0x100  /* Set a background popup menu */
#define AG_VIDEO_OPENGL	       0x200  /* Require OpenGL mode */
#define AG_VIDEO_OPENGL_OR_SDL 0x400  /* Prefer OpenGL mode */

__BEGIN_DECLS
extern AG_Display *agView;
extern SDL_PixelFormat *agVideoFmt;
extern SDL_PixelFormat *agSurfaceFmt;
extern const SDL_VideoInfo *agVideoInfo;
extern const char *agBlendFuncNames[];

int		 AG_InitVideo(int, int, int, Uint);
void		 AG_DestroyVideo(void);
int		 AG_ResizeDisplay(int, int);
int		 AG_SetRefreshRate(int);
void		 AG_BindGlobalKey(SDLKey, SDLMod, void (*)(void));
void		 AG_BindGlobalKeyEv(SDLKey, SDLMod, void (*)(AG_Event *));
int		 AG_UnbindGlobalKey(SDLKey, SDLMod);

/* GUI-related */
void		 AG_ViewVideoExpose(void);
void		 AG_ViewAttach(void *);
void		 AG_ViewDetach(struct ag_window *);
struct ag_window *AG_FindWindow(const char *);

/* Surface operations */
SDL_Surface	*AG_DupSurface(SDL_Surface *);
void		 AG_ScaleSurface(SDL_Surface *, Uint16, Uint16, SDL_Surface **);
void		 AG_SetAlphaPixels(SDL_Surface *, Uint8);
int		 AG_DumpSurface(SDL_Surface *, char *);
void		 AG_ViewCapture(void);
void		 AG_FlipSurface(Uint8 *, int, int);

/* OpenGL-specific operations */
#ifdef HAVE_OPENGL
#define AG_LockGL()	 AG_MutexLock(&agView->lock_gl)
#define AG_UnlockGL()	 AG_MutexUnlock(&agView->lock_gl)
Uint		 	 AG_SurfaceTexture(SDL_Surface *, float *);
void		 	 AG_UpdateTexture(SDL_Surface *, int);
SDL_Surface		*AG_CaptureGLView(void);
#else
#define	AG_LockGL()
#define	AG_UnlockGL()
#endif

/* Event processing */
void              AG_EventLoop_FixedFPS(void);
#define           AG_EventLoop() AG_EventLoop_FixedFPS()
Uint8  		  AG_MouseGetState(int *, int *);
#ifdef DEBUG
struct ag_window *AG_EventShowPerfGraph(void);
#endif

/* Pixel ops */
void AG_BlendPixelRGBA(SDL_Surface *, Uint8 *, Uint8, Uint8, Uint8, Uint8,
                       AG_BlendFn);
void AG_RGB2HSV(Uint8, Uint8, Uint8, float *, float *, float *);
void AG_HSV2RGB(float, float, float, Uint8 *, Uint8 *, Uint8 *);

/* Convert pixel to video format. */
static __inline__ Uint32
AG_VideoPixel(Uint32 c)
{
	Uint8 r, g, b;
	SDL_GetRGB(c, agSurfaceFmt, &r, &g, &b);
	return (SDL_MapRGB(agVideoFmt, r, g, b));
}

/* Convert pixel to surface format. */
static __inline__ Uint32
AG_SurfacePixel(Uint32 c)
{
	Uint8 r, g, b;
	SDL_GetRGB(c, agVideoFmt, &r, &g, &b);
	return (SDL_MapRGB(agSurfaceFmt, r, g, b));
}

static __inline__ Uint32
AG_GetPixel(SDL_Surface *s, Uint8 *pSrc)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		return (*(Uint32 *)pSrc);
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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
AG_PutPixel(SDL_Surface *s, Uint8 *pDst, Uint32 cDst)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)pDst = cDst;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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

static __inline__ int
AG_SamePixelFmt(SDL_Surface *s1, SDL_Surface *s2)
{
	return (s1->format->BytesPerPixel == s2->format->BytesPerPixel &&
	        s1->format->Rmask == s2->format->Rmask &&
		s1->format->Gmask == s2->format->Gmask &&
		s1->format->Bmask == s2->format->Bmask &&
		s1->format->Amask == s2->format->Amask &&
		s1->format->colorkey == s2->format->colorkey);
}

static __inline__ void
AG_QueueVideoUpdate(int x, int y, int w, int h)
{
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		agView->ndirty = 1;
	} else
#endif
	{
		if (x < 0) { x = 0; }
		if (y < 0) { y = 0; }
		if (x+w > agView->w) { w = agView->w - x; }
		if (y+h > agView->h) { h = agView->h - y; }
		if (w < 0) { x = 0; w = agView->w; }
		if (h < 0) { y = 0; h = agView->h; }

		if (agView->ndirty+1 > agView->maxdirty) {
			agView->maxdirty *= 2;
			agView->dirty = AG_Realloc(agView->dirty,
			    agView->maxdirty * sizeof(SDL_Rect));
		}
		agView->dirty[agView->ndirty].x = x;
		agView->dirty[agView->ndirty].y = y;
		agView->dirty[agView->ndirty].w = w;
		agView->dirty[agView->ndirty].h = h;
		agView->ndirty++;
	}
}

static __inline__ void
AG_CopySurfaceAsIs(SDL_Surface *sSrc, SDL_Surface *sDst)
{
	Uint32 svaflags, svcflags, svcolorkey;
	Uint8 svalpha;

	svaflags = sSrc->flags&(SDL_SRCALPHA|SDL_RLEACCEL);
	svalpha = sSrc->format->alpha;
	svcflags = sSrc->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
	svcolorkey = sSrc->format->colorkey;
	SDL_SetAlpha(sSrc, 0, 0);
	SDL_SetColorKey(sSrc, 0, 0);
	SDL_BlitSurface(sSrc, NULL, sDst, NULL);
	SDL_SetColorKey(sSrc, svcflags, svcolorkey);
	SDL_SetAlpha(sSrc, svaflags, svalpha);
}

__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_CORE_VIEW_H_ */

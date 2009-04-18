/*	Public domain	*/

#ifndef _AGAR_GUI_VIEW_H_
#define _AGAR_GUI_VIEW_H_

#include <agar/config/have_opengl.h>
#include <agar/config/view_8bpp.h>
#include <agar/config/view_16bpp.h>
#include <agar/config/view_24bpp.h>
#include <agar/config/view_32bpp.h>

#include <agar/gui/begin.h>

struct ag_window;
struct ag_style;
AG_TAILQ_HEAD(ag_windowq, ag_window);

/* Integer point */
typedef struct ag_point {
	int x, y;
} AG_Point;

/* Integer rectangle from coordinate and dimensions */
typedef struct ag_rect {
	int x, y;
	int w, h;
} AG_Rect;

/* Integer rectangle with computed extrema. */
typedef struct ag_rect2 {
	int x1, y1;
	int w, h;
	int x2, y2;
} AG_Rect2;

/* Clipping rectangle */
typedef struct ag_clip_rect {
	AG_Rect r;		/* Integer coordinates */
	double eqns[4][4];	/* Computed plane equations (GL) */
} AG_ClipRect;

/* For transition to Agar-1.4 */
typedef SDL_Surface AG_Surface;
typedef SDL_PixelFormat AG_PixelFormat;
typedef SDL_Palette AG_Palette;
typedef struct ag_color { Uint8 r, g, b, a; } AG_Color;

#define AG_SWSURFACE		SDL_SWSURFACE
#define AG_HWSURFACE		SDL_HWSURFACE
#define AG_SRCCOLORKEY		SDL_SRCCOLORKEY
#define AG_SRCALPHA		SDL_SRCALPHA
#define AG_RLEACCEL		SDL_RLEACCEL
#define AG_ALPHA_TRANSPARENT	SDL_ALPHA_TRANSPARENT
#define AG_ALPHA_OPAQUE		SDL_ALPHA_OPAQUE
#define AG_LOGPAL		SDL_LOGPAL
#define AG_PHYSPAL		SDL_PHYSPAL

enum ag_wm_operation {
	AG_WINOP_NONE,		/* No operation */
	AG_WINOP_MOVE,		/* Move window */
	AG_WINOP_LRESIZE,	/* Resize (via left control) */
	AG_WINOP_RRESIZE,	/* Resize (via right control) */
	AG_WINOP_HRESIZE	/* Resize (via horizontal control) */
};

typedef struct ag_display {
	struct ag_object obj;

	SDL_Surface *v;			/* Video surface */
	AG_Surface *stmpl;		/* Reference surface */
	int w, h;			/* Display geometry */
	int depth;			/* Depth in bpp */
	int opengl;			/* OpenGL rendering? (if available) */
	int rCur;			/* Estimated refresh delay in ms */
	Uint rNom;			/* Nominal refresh delay */
	SDL_Rect *dirty;		/* Video rectangles to update */
	Uint	 ndirty;
	Uint  maxdirty;
	
	/* XXX We should just use the object VFS instead of a list */

	struct ag_windowq windows;	/* Windows in view */
	struct ag_windowq detach;	/* Windows to free */
	struct ag_window *winToFocus;	/* Give focus to this window,
					   when event processing is done */
	struct ag_window *winSelected;	/* Window being moved/resized/etc */
	struct ag_window **winModal;	/* Modal window stack */
	Uint nModal;
	enum ag_wm_operation winop;	/* WM operation in progress */
	struct ag_style *style;		/* Default style for new windows */
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
# if AG_BYTEORDER == AG_BIG_ENDIAN
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
	if (!AG_CLIPPED_PIXEL(agView->v, (vx), (vy))) {			\
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
	if (!AG_CLIPPED_PIXEL((s), (x), (y))) {				\
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
	if (!AG_CLIPPED_PIXEL((s), (x), (y))) {				\
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
#define AG_VIDEO_NOBGCLEAR     0x800  /* Don't clear background on init */

__BEGIN_DECLS
extern AG_ObjectClass agDisplayClass;	/* Agar(Display) class definition */

extern AG_Display     *agView;		/* Main Display */
extern AG_PixelFormat *agVideoFmt;	/* Main Display pixel format */
extern AG_PixelFormat *agSurfaceFmt;	/* Preferred format for surfaces */

extern int         agRenderingContext;	/* Running in rendering context? */
extern const char *agBlendFuncNames[];	/* For enum ag_blend_func */

extern AG_ClipRect *agClipRects;	/* Clipping rectangle stack (first
					   entry always covers whole view */
extern int          agClipStateGL[4];	/* Saved GL clipping plane states */
extern Uint         agClipRectCount;

extern const SDL_VideoInfo *agVideoInfo; /* XXX */

extern int agFullscreenMode;
extern int agAsyncBlits;

int  AG_InitVideo(int, int, int, Uint);
int  AG_InitVideoSDL(SDL_Surface *, Uint);
int  AG_InitGUI(Uint);
void AG_DestroyGUI(void);
void AG_ClearBackground(void);
int  AG_SetRefreshRate(int);
void AG_ViewUpdateFB(const AG_Rect2 *);
void AG_ViewVideoExpose(void);
int  AG_ResizeDisplay(int, int);
void AG_DestroyVideo(void);
void AG_BeginRendering(void);
void AG_EndRendering(void);
int  AG_ProcessEvent(SDL_Event *);
void AG_SetVideoResizeCallback(void (*)(Uint, Uint));

void AG_BindGlobalKey(SDLKey, SDLMod, void (*)(void));
void AG_BindGlobalKeyEv(SDLKey, SDLMod, void (*)(AG_Event *));
int  AG_UnbindGlobalKey(SDLKey, SDLMod);
void AG_ClearGlobalKeys(void);

void              AG_ViewAttach(void *);
void              AG_ViewDetach(struct ag_window *);
struct ag_window *AG_FindWindow(const char *);

AG_Surface *AG_DupSurface(AG_Surface *);
int         AG_ScaleSurface(AG_Surface *, Uint16, Uint16, AG_Surface **);
void        AG_SetAlphaPixels(AG_Surface *, Uint8);
int         AG_DumpSurface(AG_Surface *, char *);
void        AG_ViewCapture(void);
void        AG_FlipSurface(Uint8 *, int, int);
#ifdef HAVE_OPENGL
Uint        AG_SurfaceTexture(AG_Surface *, float *);
void        AG_UpdateTexture(AG_Surface *, int);
AG_Surface *AG_CaptureGLView(void);
#endif

void              AG_EventLoop_FixedFPS(void);
#define           AG_EventLoop() AG_EventLoop_FixedFPS()
Uint8             AG_MouseGetState(int *, int *);
#ifdef AG_DEBUG
struct ag_window *AG_EventShowPerfGraph(void);
#endif

void AG_BlendPixelRGBA(AG_Surface *, Uint8 *, Uint8, Uint8, Uint8, Uint8,
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

#define AG_SurfaceStdRGB(w,h) \
	AG_SurfaceRGB((w),(h),agSurfaceFmt->BitsPerPixel,0,\
	    agSurfaceFmt->Rmask,agSurfaceFmt->Gmask,agSurfaceFmt->Bmask)
#define AG_SurfaceStdRGBA(w,h) \
	AG_SurfaceRGBA((w),(h),agSurfaceFmt->BitsPerPixel,0,\
	    agSurfaceFmt->Rmask,agSurfaceFmt->Gmask,agSurfaceFmt->Bmask,\
	    agSurfaceFmt->Amask)
#define AG_SurfaceVideoRGB(w,h) \
	AG_SurfaceRGB((w),(h),agVideoFmt->BitsPerPixel,0,\
	    agVideoFmt->Rmask,agVideoFmt->Gmask,agVideoFmt->Bmask)
#define AG_SurfaceVideoRGBA(w,h) \
	AG_SurfaceRGBA((w),(h),agVideoFmt->BitsPerPixel,0,\
	    agVideoFmt->Rmask,agVideoFmt->Gmask,agVideoFmt->Bmask,\
	    agVideoFmt->Amask)

#define AG_SurfaceLock(su) SDL_LockSurface(su)
#define AG_SurfaceUnlock(su) SDL_UnlockSurface(su)
#define AG_LockView() SDL_LockSurface(agView->v)
#define AG_UnlockView() SDL_UnlockSurface(agView->v)

#define AG_SetColorKey(su,f,key) \
	SDL_SetColorKey((SDL_Surface *)(su),(f),(key))
#define AG_SetAlpha(su,f,a) \
	SDL_SetAlpha((SDL_Surface *)(su),(f),(a))
#define AG_SetPalette(su,w,c,s,n) \
	SDL_SetPalette((SDL_Surface *)(su),(w),(SDL_Color *)(c),(s),(n))
#define AG_MapRGB(fmt,r,g,b) \
	SDL_MapRGB((SDL_PixelFormat *)(fmt),(r),(g),(b))
#define AG_MapRGBA(fmt,r,g,b,a) \
	SDL_MapRGBA((SDL_PixelFormat *)(fmt),(r),(g),(b),(a))
#define AG_GetRGB(pixel,fmt,r,g,b) \
	SDL_GetRGB((pixel),(SDL_PixelFormat *)(fmt),(r),(g),(b))
#define AG_GetRGBA(pixel,fmt,r,g,b,a) \
	SDL_GetRGBA((pixel),(SDL_PixelFormat *)(fmt),(r),(g),(b),(a))

/* Convert a pixel from SurfaceFmt to VideoFmt. */
static __inline__ Uint32
AG_VideoPixel(Uint32 c)
{
	Uint8 r, g, b;
	AG_GetRGB(c, agSurfaceFmt, &r,&g,&b);
	return AG_MapRGB(agVideoFmt, r,g,b);
}

/* Convert a pixel from VideoFmt to SurfaceFmt. */
static __inline__ Uint32
AG_SurfacePixel(Uint32 c)
{
	Uint8 r, g, b;
	AG_GetRGB(c, agVideoFmt, &r,&g,&b);
	return AG_MapRGB(agSurfaceFmt, r,g,b);
}

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
AG_PutPixel(AG_Surface *s, Uint8 *pDst, Uint32 cDst)
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

/* Return a Point at x,y. */
static __inline__ AG_Point
AG_POINT(int x, int y)
{
	AG_Point pt;
	pt.x = x;
	pt.y = y;
	return (pt);
}

/* Return a Rect of dimensions w,h at position x,y. */
static __inline__ AG_Rect
AG_RECT(int x, int y, int w, int h)
{
	AG_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

/* Return a Rect2 of dimensions w,h at position x,y. */
static __inline__ AG_Rect2
AG_RECT2(int x, int y, int w, int h)
{
	AG_Rect2 r;
	r.x1 = x;
	r.y1 = y;
	r.w = w;
	r.h = h;
	r.x2 = x+w;
	r.y2 = y+h;
	return (r);
}

/* Convert a Rect2 to a Rect. */
static __inline__ AG_Rect
AG_Rect2ToRect(AG_Rect2 r2)
{
	AG_Rect r;
	r.x = r2.x1;
	r.y = r2.y1;
	r.w = r2.w;
	r.h = r2.h;
	return (r);
}

/* Resize a Rect. */
static __inline__ void
AG_RectSize(AG_Rect *r, int w, int h)
{
	r->w = w;
	r->h = h;
}

/* Resize a Rect2. */
static __inline__ void
AG_RectSize2(AG_Rect2 *r, int w, int h)
{
	r->w = w;
	r->x2 = r->x1+w;
	r->h = h;
	r->y2 = r->y1+h;
}

/* Convert a Rect to a Rect2. */
static __inline__ AG_Rect2
AG_RectToRect2(AG_Rect r)
{
	AG_Rect2 r2;
	r2.x1 = r.x;
	r2.y1 = r.y;
	r2.w = r.w;
	r2.h = r.h;
	r2.x2 = r.x+r.w;
	r2.y2 = r.y+r.h;
	return (r2);
}

/* Return the intersection of two Rect's. */
static __inline__ AG_Rect
AG_RectIntersect(const AG_Rect *a, const AG_Rect *b)
{
	AG_Rect x;

	x.x = AG_MAX(a->x, b->x);
	x.y = AG_MAX(a->y, b->y);
	x.w = AG_MIN((a->x+a->w), (b->x+b->w)) - x.x;
	x.h = AG_MIN((a->y+a->h), (b->y+b->h)) - x.y;
	if (x.w < 0) { x.w = 0; }
	if (x.h < 0) { x.h = 0; }
	return (x);
}

/* Return the intersection of two Rect2's. */
static __inline__ AG_Rect2
AG_RectIntersect2(const AG_Rect2 *a, const AG_Rect2 *b)
{
	AG_Rect2 rx;

	rx.x1 = AG_MAX(a->x1, b->x1);
	rx.y1 = AG_MAX(a->y1, b->y1);
	rx.w = AG_MIN(a->x2, b->x2) - rx.x1;
	if (rx.w < 0) { rx.w = 0; }
	rx.h = AG_MIN(a->y2, b->y2) - rx.y1;
	if (rx.h < 0) { rx.h = 0; }
	rx.x2 = rx.x1 + rx.w;
	rx.y2 = rx.y1 + rx.h;
	return (rx);
}

/* Test whether two Rect's are the same. */
static __inline__ int
AG_RectCompare(const AG_Rect *a, const AG_Rect *b)
{
	return (a->x == b->x &&
	        a->y == b->y &&
		a->w == b->w &&
		a->h == b->h) ? 0:1;
}

/* Test whether two Rect2's are the same. */
static __inline__ int
AG_RectCompare2(const AG_Rect2 *a, const AG_Rect2 *b)
{
	return (a->x1 == b->x1 &&
	        a->y1 == b->y1 &&
		a->w == b->w &&
		a->h == b->h) ? 0:1;
}

/* Test whether a point intersects a Rect. */
static __inline__ int
AG_RectInside(const AG_Rect *r, int x, int y)
{
	return (x >= r->x &&
	        y >= r->y &&
		x < r->x+r->w &&
		y < r->y+r->h); 
}

/* Test whether a point intersects a Rect2. */
static __inline__ int
AG_RectInside2(const AG_Rect2 *r, int x, int y)
{
	return (x >= r->x1 &&
	        y >= r->y1 &&
		x < r->x2 &&
		y < r->y2); 
}

/* Convert an AG_Rect to an equivalent SDL_Rect. */
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

/* Convert a SDL_Rect to an equivalent AG_Rect. */
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

/* Return a Color structure for given RGB components. */
static __inline__ AG_Color
AG_COLOR_RGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	return (c);
}

/* Return a Color structure for given RGBA components. */
static __inline__ AG_Color
AG_COLOR_RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return (c);
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

/* Fill rectangle with pixels */
static __inline__ void
AG_FillRect(AG_Surface *s, const AG_Rect *r, Uint32 c)
{
	SDL_Rect rSDL;

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

#include <agar/gui/iconmgr.h>
#include <agar/gui/load_surface.h>

#endif	/* _AGAR_GUI_VIEW_H_ */

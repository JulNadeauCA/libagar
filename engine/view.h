/*	$Csoft: view.h,v 1.103 2005/09/27 00:25:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VIEW_H_
#define _AGAR_VIEW_H_
#include "begin_code.h"

#include <config/view_8bpp.h>
#include <config/view_16bpp.h>
#include <config/view_24bpp.h>
#include <config/view_32bpp.h>

struct ag_window;
TAILQ_HEAD(ag_windowq, ag_window);

typedef struct ag_display {
	SDL_Surface *v;			/* Video surface */
	SDL_Surface *stmpl;		/* Reference surface */

	int w, h;			/* Display geometry */
	int depth;			/* Depth in bpp */
	int opengl;			/* OpenGL rendering? (if available) */

	u_int rCur;			/* Estimated refresh delay in ms */
	u_int rNom;			/* Nominal refresh delay */

	SDL_Rect *dirty;		/* Video rectangles to update */
	u_int	 ndirty;
	u_int  maxdirty;

	pthread_mutex_t	lock;
	struct ag_windowq windows;	/* Windows in view */
	struct ag_windowq detach;	/* Windows to free */
	struct ag_window *focus_win;	/* Give focus to this window,
					   when event processing is done */
	struct ag_window *wop_win;	/* Window being moved/resized/etc */
	struct ag_window *modal_win;	/* Modal window (or NULL) */

	enum {
		AG_WINOP_NONE,
		AG_WINOP_MOVE,			/* Window movement */
		AG_WINOP_LRESIZE,		/* Window resize */
		AG_WINOP_RRESIZE,
		AG_WINOP_HRESIZE
	} winop;
} AG_Display;

/* Alpha functions for AG_BlendPixelRGBA(). */
enum ag_blend_func {
	AG_ALPHA_OVERLAY,		/* dA = sA+dA */
	AG_ALPHA_SRC,			/* dA = sA */
	AG_ALPHA_DST,			/* dA = dA */
	AG_ALPHA_MEAN,			/* dA = (sA+dA)/2 */
	AG_ALPHA_SOURCE_MINUS_DST,	/* dA = (sA-dA) */
	AG_ALPHA_DST_MINUS_SOURCE,	/* dA = (dA-sA) */
	AG_ALPHA_PYTHAGOREAN		/* dA = sqrt(sA^2+dA^2) */
};

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
# define _AG_VIEW_PUTPIXEL_8(dst, c)	\
case 1:					\
	*(dst) = (c);			\
	break;
#else
# define _AG_VIEW_PUTPIXEL_8(dst, c)
#endif
#ifdef VIEW_16BPP
# define _AG_VIEW_PUTPIXEL_16(dst, c)	\
case 2:					\
	*(Uint16 *)(dst) = (c);		\
	break;
#else
# define _AG_VIEW_PUTPIXEL_16(dst, c)
#endif
#ifdef VIEW_24BPP
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
#  define _AG_VIEW_PUTPIXEL_24(dst, c)	\
case 3:					\
	(dst)[0] = ((c)>>16) & 0xff;	\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] =  (c) & 0xff;		\
	break;
# else
#  define _AG_VIEW_PUTPIXEL_24(dst, c)	\
case 3:					\
	(dst)[0] =  (c) & 0xff;		\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] = ((c)>>16) & 0xff;	\
	break;
# endif
#else
# define _AG_VIEW_PUTPIXEL_24(dst, c)
#endif
#ifdef VIEW_32BPP
# define _AG_VIEW_PUTPIXEL_32(dst, c)	\
case 4:					\
	*(Uint32 *)(dst) = (c);		\
	break;
#else
# define _AG_VIEW_PUTPIXEL_32(dst, c)
#endif

#define AG_VIEW_PUT_PIXEL(p, c) do {					\
	switch (agVideoFmt->BytesPerPixel) {				\
		_AG_VIEW_PUTPIXEL_8((p),  (c))				\
		_AG_VIEW_PUTPIXEL_16((p), (c))				\
		_AG_VIEW_PUTPIXEL_24((p), (c))				\
		_AG_VIEW_PUTPIXEL_32((p), (c))				\
	}								\
} while (0)
#define AG_VIEW_PUT_PIXEL2(vx, vy, c) do {				\
	Uint8 *_view_dst = (Uint8 *)agView->v->pixels +			\
	    (vy)*agView->v->pitch + (vx)*agVideoFmt->BytesPerPixel;	 \
	switch (agVideoFmt->BytesPerPixel) {				 \
		_AG_VIEW_PUTPIXEL_8(_view_dst,  (c))			\
		_AG_VIEW_PUTPIXEL_16(_view_dst, (c))			\
		_AG_VIEW_PUTPIXEL_24(_view_dst, (c))			\
		_AG_VIEW_PUTPIXEL_32(_view_dst, (c))			\
	}								\
} while (0)
#define AG_VIEW_PUT_PIXEL2_CLIPPED(vx, vy, c) do {			\
	if (AG_NONCLIPPED_PIXEL(agView->v, (vx), (vy))) {		\
		Uint8 *_view_dst = (Uint8 *)agView->v->pixels +		\
		    (vy)*agView->v->pitch + (vx)*agVideoFmt->BytesPerPixel; \
		switch (agVideoFmt->BytesPerPixel) {			 \
			_AG_VIEW_PUTPIXEL_8(_view_dst,  (c))		\
			_AG_VIEW_PUTPIXEL_16(_view_dst, (c))		\
			_AG_VIEW_PUTPIXEL_24(_view_dst, (c))		\
			_AG_VIEW_PUTPIXEL_32(_view_dst, (c))		\
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

extern AG_Display *agView;
extern SDL_PixelFormat *agVideoFmt;
extern SDL_PixelFormat *agSurfaceFmt;
extern const SDL_VideoInfo *agVideoInfo;
extern const char *agBlendFuncNames[];

__BEGIN_DECLS
int		 AG_ViewInit(int, int, int, u_int);
int		 AG_ResizeDisplay(int, int);
void		 AG_ViewVideoExpose(void);
void		 AG_ViewAttach(void *);
void		 AG_ViewDetach(struct ag_window *);
__inline__ void	 AG_ViewDetachQueued(void);
void		 AG_ViewDestroy(void);
int		 AG_SetRefreshRate(int);
struct ag_window *AG_FindWindow(const char *);

__inline__ SDL_Surface	*AG_DupSurface(SDL_Surface *);
void			 AG_ScaleSurface(SDL_Surface *, Uint16, Uint16,
			                 SDL_Surface **);
void			 AG_SetAlphaPixels(SDL_Surface *, Uint8);
void			 AG_DumpSurface(SDL_Surface *, char *);
__inline__ void		 AG_UpdateRectQ(int, int, int, int);

#ifdef HAVE_OPENGL
GLuint		 AG_SurfaceTexture(SDL_Surface *, GLfloat *);
void		 AG_UpdateTexture(SDL_Surface *, int);
SDL_Surface	*AG_CaptureGLView(void);
#endif

__inline__ int	  AG_SamePixelFmt(SDL_Surface *, SDL_Surface *);
__inline__ Uint32 AG_GetPixel(SDL_Surface *, Uint8 *);
__inline__ void	  AG_PutPixel(SDL_Surface *, Uint8 *, Uint32);
__inline__ void	  AG_BlendPixelRGBA(SDL_Surface *, Uint8 *,
		                    Uint8, Uint8, Uint8, Uint8,
			  	    enum ag_blend_func);

__inline__ void AG_FlipSurface(Uint8 *, int, int);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VIEW_H_ */

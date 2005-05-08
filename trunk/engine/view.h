/*	$Csoft: view.h,v 1.95 2005/04/15 06:17:55 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VIEW_H_
#define _AGAR_VIEW_H_
#include "begin_code.h"

#include <config/view_8bpp.h>
#include <config/view_16bpp.h>
#include <config/view_24bpp.h>
#include <config/view_32bpp.h>

struct viewmap {
	int  w, h;			/* View geometry in nodes */

	/* Shares viewport->lock. */
	struct map	 *map;		/* Currently visible map */
	int		  x, y;		/* Offset in map */
	Sint16		  sx, sy;	/* Soft scroll offset */
	SDL_Rect	**maprects;	/* Destination rectangles for drawing
					   nodes (optimization) */
};

TAILQ_HEAD(windowq, window);

struct viewport {
	enum gfx_engine  gfx_engine;	/* Rendering method */
	SDL_Surface	*v;		/* Video surface */
	struct viewmap	*rootmap;	/* Non-NULL in game mode */
	int		 w, h;		/* Display geometry */
	int		 depth;		/* Depth in bpp */
	int		 opengl;	/* OpenGL rendering? (if available) */
	struct {
		int	 r;		/* Estimated refresh rate in ms */
		int	 rnom;		/* Nominal FPS (expressed as 1000/n) */
	} refresh;
	SDL_Rect *dirty;		/* Video rectangles to update */
	u_int	 ndirty;
	u_int  maxdirty;

	pthread_mutex_t	 lock;
	struct windowq	 windows;	/* Windows in view */
	struct windowq	 detach;	/* Windows to free */
	struct window	*focus_win;	/* Give focus to this window,
					   when event processing is done. */
	struct window	*wop_win;	/* Window being moved/resized/etc. */
	struct window	*modal_win;	/* Modal window (or NULL). */

	enum {
		VIEW_WINOP_NONE,
		VIEW_WINOP_MOVE,		/* Window movement */
		VIEW_WINOP_LRESIZE,		/* Window resize */
		VIEW_WINOP_RRESIZE,
		VIEW_WINOP_HRESIZE
	} winop;
};

/* Determine whether a pixel must be clipped or not. */
#define CLIPPED_PIXEL(s, ax, ay)			\
	((ax) < (s)->clip_rect.x ||			\
	 (ax) >= (s)->clip_rect.x+(s)->clip_rect.w ||	\
	 (ay) < (s)->clip_rect.y ||			\
	 (ay) >= (s)->clip_rect.y+(s)->clip_rect.h)
#define NONCLIPPED_PIXEL(s, ax, ay)			\
	((ax) >= (s)->clip_rect.x &&			\
	 (ax) < (s)->clip_rect.x+(s)->clip_rect.w &&	\
	 (ay) >= (s)->clip_rect.y &&			\
	 (ay) < (s)->clip_rect.y+(s)->clip_rect.h)

/*
 * Putpixel macros optimized for the display surface format.
 */

#ifdef VIEW_8BPP
# define _VIEW_PUTPIXEL_8(dst, c)	\
case 1:					\
	*(dst) = (c);			\
	break;
#else
# define _VIEW_PUTPIXEL_8(dst, c)
#endif
#ifdef VIEW_16BPP
# define _VIEW_PUTPIXEL_16(dst, c)	\
case 2:					\
	*(Uint16 *)(dst) = (c);		\
	break;
#else
# define _VIEW_PUTPIXEL_16(dst, c)
#endif
#ifdef VIEW_24BPP
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
#  define _VIEW_PUTPIXEL_24(dst, c)	\
case 3:					\
	(dst)[0] = ((c)>>16) & 0xff;	\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] =  (c) & 0xff;		\
	break;
# else
#  define _VIEW_PUTPIXEL_24(dst, c)	\
case 3:					\
	(dst)[0] =  (c) & 0xff;		\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] = ((c)>>16) & 0xff;	\
	break;
# endif
#else
# define _VIEW_PUTPIXEL_24(dst, c)
#endif
#ifdef VIEW_32BPP
# define _VIEW_PUTPIXEL_32(dst, c)	\
case 4:					\
	*(Uint32 *)(dst) = (c);		\
	break;
#else
# define _VIEW_PUTPIXEL_32(dst, c)
#endif

#define VIEW_PUT_PIXEL(p, c) do {					\
	switch (vfmt->BytesPerPixel) {					\
		_VIEW_PUTPIXEL_8((p),  (c))				\
		_VIEW_PUTPIXEL_16((p), (c))				\
		_VIEW_PUTPIXEL_24((p), (c))				\
		_VIEW_PUTPIXEL_32((p), (c))				\
	}								\
} while (0)
#define VIEW_PUT_PIXEL2(vx, vy, c) do {					\
	Uint8 *_view_dst = (Uint8 *)view->v->pixels +			\
	    (vy)*view->v->pitch + (vx)*vfmt->BytesPerPixel;		\
	switch (vfmt->BytesPerPixel) {					\
		_VIEW_PUTPIXEL_8(_view_dst,  (c))			\
		_VIEW_PUTPIXEL_16(_view_dst, (c))			\
		_VIEW_PUTPIXEL_24(_view_dst, (c))			\
		_VIEW_PUTPIXEL_32(_view_dst, (c))			\
	}								\
} while (0)
#define VIEW_PUT_PIXEL2_CLIPPED(vx, vy, c) do {				\
	if (NONCLIPPED_PIXEL(view->v, (vx), (vy))) {			\
		Uint8 *_view_dst = (Uint8 *)view->v->pixels +		\
		    (vy)*view->v->pitch + (vx)*vfmt->BytesPerPixel;	\
		switch (vfmt->BytesPerPixel) {				\
			_VIEW_PUTPIXEL_8(_view_dst,  (c))		\
			_VIEW_PUTPIXEL_16(_view_dst, (c))		\
			_VIEW_PUTPIXEL_24(_view_dst, (c))		\
			_VIEW_PUTPIXEL_32(_view_dst, (c))		\
		}							\
	}								\
} while (0)

/*
 * Generic putpixel/getpixel macros.
 */

#define GET_PIXEL(s, p) view_get_pixel((s),(p))
#define GET_PIXEL2(s, x, y)						\
	view_get_pixel((s),(Uint8 *)(s)->pixels + (y)*(s)->pitch +	\
	    (x)*(s)->format->BytesPerPixel)

#define PUT_PIXEL(s, p, c) view_put_pixel((s),(p),(c))
#define PUT_PIXEL2(s, x, y, c) do {					\
	view_put_pixel((s),						\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    (c));							\
} while (0)
#define PUT_PIXEL2_CLIPPED(s, x, y, c) do {				\
	if (NONCLIPPED_PIXEL((s), (x), (y))) {				\
		view_put_pixel((s),					\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    (c));						\
	}								\
} while (0)

/*
 * Generic alpha blending macros.
 */

#define BLEND_RGBA(s, p, r, g, b, a) \
	view_blend_rgba((s),(p),(r),(g),(b),(a))
#define BLEND_RGBA2(s, x, y, r, g, b, a) do {				\
	view_blend_rgba((s),						\
	    (Uint8 *)(s)->pixels + (y)*(s)->pitch +			\
	    (x)*(s)->format->BytesPerPixel,				\
	    (r),(g),(b),(a));						\
} while (0)
#define BLEND_RGBA2_CLIPPED(s, x, y, r, g, b, a) do {			\
	if (NONCLIPPED_PIXEL((s), (x), (y))) {				\
		view_blend_rgba((s),					\
		    (Uint8 *)(s)->pixels + (y)*(s)->pitch +		\
		    (x)*(s)->format->BytesPerPixel,			\
		    (r),(g),(b),(a));					\
	}								\
} while (0)

extern struct viewport *view;
extern SDL_PixelFormat *vfmt;
extern const SDL_VideoInfo *vinfo;

__BEGIN_DECLS
int		 view_init(enum gfx_engine);
int		 view_resize(int, int);
void		 view_videoexpose(void);
void		 view_attach(void *);
void		 view_detach(struct window *);
__inline__ void	 view_detach_queued(void);
void		 view_destroy(void);
int		 view_set_refresh(int);
struct window	*view_window_exists(const char *);
void		 view_parse_fpsspec(const char *);

__inline__ SDL_Surface	*view_surface(Uint32, int, int);
__inline__ SDL_Surface	*view_copy_surface(SDL_Surface *);
void			 view_scale_surface(SDL_Surface *, Uint16, Uint16,
			                    SDL_Surface **);
void			 view_pixels_alpha(SDL_Surface *, Uint8);
void			 view_capture(SDL_Surface *);
__inline__ void		 view_update(int, int, int, int);

#ifdef HAVE_OPENGL
GLuint			 view_surface_texture(SDL_Surface *, GLfloat *);
void			 view_update_texture(SDL_Surface *, int);
#endif

__inline__ int	  view_same_pixel_fmt(SDL_Surface *, SDL_Surface *);
__inline__ Uint32 view_get_pixel(SDL_Surface *, Uint8 *);
__inline__ void	  view_put_pixel(SDL_Surface *, Uint8 *, Uint32);
__inline__ void	  view_blend_rgba(SDL_Surface *, Uint8 *,
		                  Uint8, Uint8, Uint8, Uint8);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VIEW_H_ */

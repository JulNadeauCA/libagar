/*	$Csoft: view.h,v 1.87 2004/04/20 09:16:39 vedge Exp $	*/
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
		int	 current;	/* Estimated refresh rate in ms */
		int	 delay;		/* Current refresh delay in ms */
		int	 max_delay;	/* Nominal refresh rate delay in ms */
	} refresh;
	SDL_Rect	*dirty;		/* Video rectangles to update */
	unsigned int	 ndirty;	/* Number of rectangles to update */
	unsigned int	 maxdirty;	/* Size of dirty rectangle array */

	pthread_mutex_t	 lock;
	struct windowq	 windows;	/* Windows in view */
	struct windowq	 detach;	/* Windows to free */
	struct window	*focus_win;	/* Give focus to this window,
					   when event processing is done. */
	struct window	*wop_win;	/* Window being moved/resized/etc. */
	enum {
		VIEW_WINOP_NONE,
		VIEW_WINOP_MOVE,		/* Window movement */
		VIEW_WINOP_LRESIZE,		/* Window resize */
		VIEW_WINOP_RRESIZE,
		VIEW_WINOP_HRESIZE
	} winop;
};

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

#define VIEW_INSIDE_CLIP_RECT(s, ax, ay)		\
	((ax) >= (s)->clip_rect.x &&			\
	 (ax) < (s)->clip_rect.x+(s)->clip_rect.w &&	\
	 (ay) >= (s)->clip_rect.y &&			\
	 (ay) < (s)->clip_rect.y+(s)->clip_rect.h)

/* The surface must be locked if it is a hardware surface. */
#define VIEW_PUT_PIXEL(s, vx, vy, c) do {				\
	if (VIEW_INSIDE_CLIP_RECT((s), (vx), (vy))) {			\
		Uint8 *_view_dst = (Uint8 *)(s)->pixels +		\
		    (vy)*(s)->pitch + (vx)*(s)->format->BytesPerPixel;	\
		switch ((s)->format->BytesPerPixel) {			\
			_VIEW_PUTPIXEL_8(_view_dst,  (c))		\
			_VIEW_PUTPIXEL_16(_view_dst, (c))		\
			_VIEW_PUTPIXEL_24(_view_dst, (c))		\
			_VIEW_PUTPIXEL_32(_view_dst, (c))		\
		}							\
	}								\
} while (0)

#define VIEW_PUT_UNCLIPPED_PIXEL(s, vx, vy, c) do {		\
	Uint8 *_view_dst = (Uint8 *)(s)->pixels +		\
	    (vy)*(s)->pitch + (vx)*(s)->format->BytesPerPixel;	\
	switch ((s)->format->BytesPerPixel) {			\
		_VIEW_PUTPIXEL_8(_view_dst,  (c))		\
		_VIEW_PUTPIXEL_16(_view_dst, (c))		\
		_VIEW_PUTPIXEL_24(_view_dst, (c))		\
		_VIEW_PUTPIXEL_32(_view_dst, (c))		\
	}							\
} while (0)

extern struct viewport *view;
extern SDL_PixelFormat *vfmt;
extern const SDL_VideoInfo *vinfo;

__BEGIN_DECLS
int		 view_init(enum gfx_engine);
void		 view_attach(void *);
void		 view_detach(struct window *);
__inline__ void	 view_detach_queued(void);
void		 view_destroy(void);
int		 view_set_refresh(int);
struct window	*view_window_exists(const char *);
void		 view_parse_fpsspec(const char *);

__inline__ SDL_Surface	*view_surface(Uint32, int, int);
__inline__ SDL_Surface	*view_copy_surface(SDL_Surface *);
SDL_Surface		*view_scale_surface(SDL_Surface *, Uint16, Uint16);
void			 view_set_trans(SDL_Surface *, Uint8);
void			 view_capture(SDL_Surface *);
__inline__ void		 view_alpha_blend(SDL_Surface *, Sint16, Sint16,
			                  Uint8, Uint8, Uint8, Uint8);
__inline__ void		 view_update(int, int, int, int);
#ifdef HAVE_OPENGL
GLuint			 view_surface_texture(SDL_Surface *, GLfloat *);
#endif
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_VIEW_H_ */

/*	$Csoft: view.h,v 1.65 2003/01/03 23:28:56 vedge Exp $	*/
/*	Public domain	*/

#include <config/view_8bpp.h>
#include <config/view_16bpp.h>
#include <config/view_24bpp.h>
#include <config/view_32bpp.h>

enum gfx_engine {
	GFX_ENGINE_GUI,		/* Direct rendering, solid background */
	GFX_ENGINE_TILEBASED	/* Direct rendering, map background */
};

/* Map display */
struct viewmap {
	/* Read-only */
	int	w, h;			/* View geometry in nodes */

	/* Read-write (shares viewport->lock) */
	struct	map *map;		/* Currently visible map */
	int	x, y;			/* Offset in map */
	int	sx, sy;			/* Soft scroll offset */

	/* Optimizations */
	SDL_Rect **maprects;		/* Rectangles needed to draw the map. */
};

TAILQ_HEAD(windowq, window);

struct viewport {
	struct	object obj;

	/* Read-only */
	enum gfx_engine gfx_engine;	/* Rendering method */
	SDL_Surface	*v;		/* Video surface */
	struct viewmap	*rootmap;	/* Non-NULL in game mode */
	int		 w, h;		/* Display geometry */
	int		 depth;		/* Depth in bpp */
	int		 opengl;	/* OpenGL rendering? (if available) */
	struct {
		int	 current;	/* Estimated refresh rate in ms */
		int	 delay;		/* Current refresh delay in ms */
		int	 min_delay;	/* Minimum delay in ms */
		int	 max_delay;	/* Maximum delay in ms */
	} refresh;

	SDL_Rect	*dirty;		/* Video rectangles to update */
	int		 ndirty;	/* Number of rectangles to update */
	int		 maxdirty;	/* Size of dirty rectangle array */

	/* Read-write, thread-safe */
	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;
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
		VIEW_WINOP_HRESIZE,
		VIEW_WINOP_REGRESIZE_LEFT,	/* Region resize */
		VIEW_WINOP_REGRESIZE_RIGHT,
		VIEW_WINOP_REGRESIZE_UP,
		VIEW_WINOP_REGRESIZE_DOWN
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
	(dst)[2] =  (c) & 0xff;	\
	break;
# else
#  define _VIEW_PUTPIXEL_24(dst, c)	\
case 3:					\
	(dst)[0] =  (c) & 0xff;	\
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
	 (ax) <= (s)->clip_rect.x+(s)->clip_rect.w &&	\
	 (ay) >= (s)->clip_rect.y &&			\
	 (ay) <= (s)->clip_rect.y+(s)->clip_rect.h)

/* The surface must be locked. */
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

#define VIEW_UPDATE(rect) do {					\
	if (!view->opengl) {					\
		if (view->ndirty + 1 > view->maxdirty) {	\
			fatal("too many rects\n");		\
		}						\
		view->dirty[view->ndirty++] = (rect);		\
	}							\
} while (0)

extern struct viewport *view;	/* view.c */

int	 view_init(enum gfx_engine);
void	 view_attach(void *);
void	 view_detach(void *);
void	 view_detach_queued(void);
void	 view_destroy(void *);
int	 view_set_refresh(int, int);

SDL_Surface	*view_surface(int, int, int);
SDL_Surface	*view_scale_surface(SDL_Surface *, Uint16, Uint16);
#ifdef HAVE_OPENGL
GLuint		 view_surface_texture(SDL_Surface *, GLfloat *);
#endif
extern __inline__ void	view_alpha_blend(SDL_Surface *, Sint16, Sint16,
			    Uint8, Uint8, Uint8, Uint8);


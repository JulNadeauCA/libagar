/*	$Csoft: view.h,v 1.38 2002/07/29 04:20:42 vedge Exp $	*/
/*	Public domain	*/

typedef enum {
	/*
	 * Optimized for a GUI interface. Used by the map editor.
	 * - Real-time event/motion interpolation.
	 * - Allow animations inside windows.
	 * - No root map.
	 * - XXX Inefficient video updates.
	 */
	GFX_ENGINE_GUI,
	/*
	 * 2D tile-based map rendering. Used by the games.
	 * - Real-time event/motion interpolation.
	 * - Allow animations inside windows.
	 * - Root map allowing overlapping windows.
	 * - Efficient video updates.
	 */
	GFX_ENGINE_TILEBASED
} gfx_engine_t;

struct viewmap {
	/* Read-only */
	int	w, h;			/* View geometry in nodes */
	
	/* Read-write (shares viewport->lock) */
	struct	map *map;		/* Currently visible map */
	int	x, y;			/* Offset in map */

	int		**mask;		/* Mask covering the map view */
	SDL_Rect	**maprects;	/* Rectangles (optimization) */
	SDL_Rect	*rects;		/* List big enough to hold all
					   possible rectangles in the view. */
};

TAILQ_HEAD(windowq, window);

struct viewport {
	struct	object obj;

	/* Read-only */
	gfx_engine_t gfx_engine;	/* Type of rendering engine */
	int	w, h, bpp;		/* Viewport geometry */
	SDL_Surface	*v;		/* Screen. */
	struct	viewmap *rootmap;	/* Non-NULL in game mode.
					   (read-only pointer) */

	/* Read-write, thread-safe */
	struct	windowq windowsh;	/* Hidden/shown windows */
	struct	window *focus_win;	/* Give focus to this window */
	struct	window *wop_win;	/* Window operations */
	enum {
		VIEW_WINOP_NONE,
		VIEW_WINOP_MOVE,
		VIEW_WINOP_LRESIZE,	/* Left resize */
		VIEW_WINOP_RRESIZE,	/* Right resize */
		VIEW_WINOP_HRESIZE,	/* Height resize */
	} winop;

	pthread_mutex_t lock;
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

/* XXX inefficient */

#define VIEW_PUT_ALPHAPIXEL(s, avx, avy, c, a) do {		\
	Uint32 _view_col;					\
	Uint8 _view_alpha_rs, _view_alpha_gs, _view_alpha_bs;	\
	Uint8 _view_alpha_rd, _view_alpha_gd, _view_alpha_bd;	\
	Uint8 *_putpixel_dst;					\
								\
	_putpixel_dst = (Uint8 *)(s)->pixels +			\
	    (avy)*(s)->pitch +					\
	    (avx)*(s)->format->BytesPerPixel;			\
								\
	SDL_GetRGB((c), (s)->format, &_view_alpha_rs,		\
	    &_view_alpha_gs, &_view_alpha_bs);			\
	SDL_GetRGB(*_putpixel_dst, (s)->format, &_view_alpha_rd,\
	    &_view_alpha_gd, &_view_alpha_bd);			\
								\
	_view_alpha_rd = (((_view_alpha_rs - _view_alpha_rd) *	\
	    (a)) >> 8) + _view_alpha_rd;			\
	_view_alpha_gd = (((_view_alpha_gs - _view_alpha_gd) *	\
	    (a)) >> 8) + _view_alpha_gd;			\
	_view_alpha_bd = (((_view_alpha_bs - _view_alpha_bd) *	\
	    (a)) >> 8) + _view_alpha_bd;			\
	    							\
	_view_col = SDL_MapRGB((s)->format, _view_alpha_rd,	\
	    _view_alpha_gd, _view_alpha_bd);			\
								\
	switch ((s)->format->BytesPerPixel) {			\
		_VIEW_PUTPIXEL_8(_putpixel_dst,  _view_col)	\
		_VIEW_PUTPIXEL_16(_putpixel_dst, _view_col)	\
		_VIEW_PUTPIXEL_24(_putpixel_dst, _view_col)	\
		_VIEW_PUTPIXEL_32(_putpixel_dst, _view_col)	\
	}							\
} while (/*CONSTCOND*/0)

/*
 * Set the pixel at x,y to c inside surface s.
 * Surface must be locked.
 */
#define VIEW_PUT_PIXEL(s, vx, vy, c) do {				\
	Uint8 *_view_dst = (Uint8 *)(s)->pixels +			\
	    (vy)*(s)->pitch + (vx)*(s)->format->BytesPerPixel;		\
									\
	switch ((s)->format->BytesPerPixel) {				\
		_VIEW_PUTPIXEL_8(_view_dst,  (c))			\
		_VIEW_PUTPIXEL_16(_view_dst, (c))			\
		_VIEW_PUTPIXEL_24(_view_dst, (c))			\
		_VIEW_PUTPIXEL_32(_view_dst, (c))			\
	}								\
} while (/*CONSTCOND*/0)

#define VIEW_REDRAW() do {				\
	SDL_Event rdev;					\
							\
	rdev.type = SDL_VIDEOEXPOSE;			\
	SDL_PushEvent(&rdev);				\
} while (/*CONSTCOND*/0)

extern struct viewport *view;	/* view.c */

void	 view_init(gfx_engine_t);
void	 view_attach(void *);
void	 view_detach(void *);
void	 view_destroy(void *);

SDL_Surface	*view_surface(int, int, int);
void		 view_focus(struct window *);


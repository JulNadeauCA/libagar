/*	$Csoft: view.h,v 1.31 2002/06/27 00:17:39 vedge Exp $	*/
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
	int	wop_mapx, wop_mapy;
	enum {
		VIEW_WINOP_NONE,
		VIEW_WINOP_MOVE,
		VIEW_WINOP_RESIZE
	} winop;

	pthread_mutex_t lock;
};

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
# define _PUT_PIXEL_24(dst, c)		\
	(dst)[0] = ((c)>>16) & 0xff;	\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] =  (c) & 0xff
#else
# define _PUT_PIXEL_24(dst, c)		\
	(dst)[0] =  (c) & 0xff;		\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] = ((c)>>16) & 0xff
#endif

/* XXX inefficient */
#define VIEW_PUT_ALPHAPIXEL(s, avx, avy, c, a) do {		\
	Uint32 _view_col = (Uint32)(c);				\
	Uint8 _view_alpha_rs, _view_alpha_gs, _view_alpha_bs;	\
	Uint8 _view_alpha_rd, _view_alpha_gd, _view_alpha_bd;	\
	Uint8 *_putpixel_dst;					\
								\
	_putpixel_dst = (Uint8 *)(s)->pixels +			\
	    (avy)*(s)->pitch +					\
	    (avx)*(s)->format->BytesPerPixel;			\
								\
	SDL_GetRGB(_view_col, (s)->format, &_view_alpha_rs,	\
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
	case 1:							\
		*_putpixel_dst = _view_col;			\
		break;						\
	case 2:							\
		*(Uint16 *)_putpixel_dst = _view_col;		\
		break;						\
	case 3:							\
		_PUT_PIXEL_24(_putpixel_dst, _view_col);	\
		break;						\
	case 4:							\
		*(Uint32 *)_putpixel_dst = _view_col;		\
		break;						\
	}							\
} while (/*CONSTCOND*/0)

/*
 * Set the pixel at x,y to c inside surface s.
 * Surface must be locked.
 */
#define VIEW_PUT_PIXEL(s, vx, vy, c) do {			\
	Uint8 *_putpixel_dst;					\
								\
	_putpixel_dst = (Uint8 *)(s)->pixels +			\
	    (vy)*(s)->pitch +					\
	    (vx)*(s)->format->BytesPerPixel;			\
								\
	switch ((s)->format->BytesPerPixel) {			\
	case 1:							\
		*_putpixel_dst = (c);				\
		break;						\
	case 2:							\
		*(Uint16 *)_putpixel_dst = (c);			\
		break;						\
	case 3:							\
		_PUT_PIXEL_24(_putpixel_dst, (c));		\
		break;						\
	case 4:							\
		*(Uint32 *)_putpixel_dst = (c);			\
		break;						\
	}							\
} while (/*CONSTCOND*/0)

#define VIEW_REDRAW() do {				\
	SDL_Event rdev;					\
							\
	rdev.type = SDL_VIDEOEXPOSE;			\
	SDL_PushEvent(&rdev);				\
} while (/*CONSTCOND*/0)

/* Wrappers for high-level code. */

extern struct viewport	*view;	/* view.c */

void		 view_init(gfx_engine_t);

void		 view_attach(void *);
void		 view_detach(void *);
void		 view_destroy(void *);

void		 view_maskfill(SDL_Rect *, int);
SDL_Surface	*view_surface(int, int, int);
void		 view_focus(struct window *);

void		 view_center(struct map *, int, int);
void		 view_scroll(struct map *, int);


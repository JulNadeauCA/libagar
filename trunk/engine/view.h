/*	$Csoft: view.h,v 1.28 2002/06/09 10:08:04 vedge Exp $	*/
/*	Public domain	*/

enum {
	VIEW_MAPNAV,	/* Map navigation display */
	VIEW_MAPEDIT,	/* Map edition display */
	VIEW_FIGHT	/* Battle display */
};

TAILQ_HEAD(windowq, window);

struct viewport {
	struct	object obj;

	/* Read-only */

	int	mode;			/* Display mode */
	int	**mapmask;		/* Mask covering the map view */
	int	w, h, depth;		/* Viewport geometry */

	SDL_Surface	*v;		/* Screen. XXX unprotected */
	SDL_Rect	**maprects;	/* Rectangles (optimization) */
	SDL_Rect	*rects;		/* List big enough to hold all
					   possible rectangles in a view. */
	int	mapw, maph;		/* Map view geometry */
	int	vmapw, vmaph;		/* Map view geometry in tiles */
	int	mapx, mapy;		/* Map view coordinates */
	int	mapxoffs, mapyoffs;	/* Map view offsets. XXX */

	/* Read-write, thread-safe */

#if 1
	struct	map *map;		/* Currently visible map. XXX atomic */
#endif
	struct	windowq windowsh;	/* Hidden/shown windows */
	struct	window *wop_win;	/* Window operations */
	int	wop_mapx, wop_mapy;
	enum {
		VIEW_WINOP_NONE,
		VIEW_WINOP_MOVE,
		VIEW_WINOP_RESIZE
	} winop;
	pthread_mutex_t lock;
};

#define VIEW_MAPMASK(vx, vy)	\
    (mainview->mapmask[(vy) - mainview->mapyoffs][(vx) - mainview->mapxoffs])

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

extern struct viewport *mainview;	/* view.c */

void		 view_init(int, int, int, int, int);
void		 view_attach(void *);
void		 view_detach(void *);
void		 view_destroy(void *);

void		 view_fullscreen(int);
void		 view_center(int, int);
void		 view_maskfill(SDL_Rect *, int);
void		 view_redraw(void);
SDL_Surface	*view_surface(int, int, int);
void		 view_focus(struct window *);
#ifdef DEBUG
void		 view_dumpmask(void);
#endif
void		 scroll(struct map *, int);


/*	$Csoft: view.h,v 1.22 2002/05/19 14:32:54 vedge Exp $	*/

enum {
	VIEW_MAPNAV,	/* Map navigation display */
	VIEW_MAPEDIT,	/* Map edition display */
	VIEW_FIGHT	/* Battle display */
};

struct viewport {
	struct	object obj;

	/* Read-only once a mode is set, shares map lock */
	int	mode;			/* Display mode */
	int	fps;			/* Refresh rate in FPS */
	Uint32	flags;
	int	width, height, depth;	/* Viewport geometry */
	struct	map *map;		/* Currently visible map */
	Uint32	mapw, maph;		/* Map view geometry */
	Uint32	mapx, mapy;		/* Map view coordinates */
	int	mapxoffs, mapyoffs;	/* Map view offsets */
	int	vmapw, vmaph;

	int	 **mapmask;		/* Mask covering the map view */
	SDL_Rect **maprects;		/* Rectangles (optimization) */
	SDL_Rect *rects;		/* List big enough to hold all
					   possible rectangles in a view. */
	SDL_Surface	*v;		/* Surface */

	/* Read-write, thread-safe */
	TAILQ_HEAD(windows_head, window) windowsh;
	pthread_mutex_t lock;
};

#define VIEW_MAPMASK(view, vx, vy)	\
    ((view)->mapmask[(vy) - (view)->mapyoffs][(vx) - (view)->mapxoffs])

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

struct viewport *view_new(int, int, int, int);
void		 view_attach(void *, void *);
void		 view_detach(void *, void *);
void		 view_destroy(void *);

int		 view_setmode(struct viewport *, struct map *, int, char *);
void		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);
void		 view_maskfill(struct viewport *, SDL_Rect *, int);
void		 view_redraw(struct viewport *);
SDL_Surface	 *view_surface(int, int, int);
void		 view_focus(struct viewport *, struct window *);

#ifdef DEBUG
void		 view_dumpmask(struct viewport *);
#endif

void		 scroll(struct map *, int);


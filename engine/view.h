/*	$Csoft: view.h,v 1.19 2002/05/11 04:06:56 vedge Exp $	*/

enum {
	VIEW_MAPNAV,	/* Map navigation display */
	VIEW_MAPEDIT,	/* Map edition display */
	VIEW_FIGHT	/* Battle display */
};

/*
 * This structure shares the mutex of the map being displayed,
 * thus limiting maps to be shown in only one map at a time.
 */
struct viewport {
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
};

#define VIEW_MAPMASK(view, vx, vy)	\
    ((view)->mapmask[(vy) - (view)->mapyoffs][(vx) - (view)->mapxoffs])

/* XXX thread unsafe, but cannot change during execution. */
extern struct viewport *mainview;	/* view.c */

struct viewport *view_new(int, int, int, int);
int		 view_setmode(struct viewport *, struct map *, int, char *);
void		 view_destroy(struct viewport *);
void		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);
void		 view_maskfill(struct viewport *, SDL_Rect *, int);
void		 view_redraw(struct viewport *);
#ifdef DEBUG
void		 view_dumpmask(struct viewport *);
#endif

void		 scroll(struct map *, int);


/*	$Csoft: view.h,v 1.16 2002/05/02 06:28:59 vedge Exp $	*/

enum {
	VIEW_MAPNAV,	/* Map navigation display */
	VIEW_MAPEDIT,	/* Map edition display */
	VIEW_FIGHT	/* Battle display */
};

struct viewport {
	int	mode;			/* Display mode */
	int	fps;			/* Refresh rate in FPS */
	Uint32	flags;
	Uint32	width, height, depth;	/* Viewport geometry */
	struct	map *map;		/* Currently visible map */
	Uint32	mapw, maph;		/* Map view geometry */
	Uint32	mapx, mapy;		/* Map view coordinates */
	Uint32	mapxoffs, mapyoffs;	/* Map view offsets */
	Uint32	vmapw, vmaph;

	int	 **mapmask;		/* Mask covering the map view */
	SDL_Rect **maprects;		/* Rectangles (optimization) */
	SDL_Rect *rects;		/* List big enough to hold all
					   possible rectangles in a view. */
	pthread_mutex_t lock;		/* Lock on mask arrays */

	SDL_Surface	*v;		/* Surface */
};

#define VIEW_MAPMASK(view, vx, vy)	\
    ((view)->mapmask[(vy) - (view)->mapyoffs][(vx) - (view)->mapxoffs])

extern struct viewport *mainview;	/* view.c */

struct viewport *view_create(Uint32, Uint32, Uint32, Uint32);
int		 view_setmode(struct viewport *, struct map *, int, char *);
void		 view_destroy(struct viewport *);
int		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);
void		 view_maskfill(struct viewport *, SDL_Rect *, Uint32);
void		 view_redraw(struct viewport *);

void		 scroll(struct map *, int);


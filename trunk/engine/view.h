/*	$Csoft: view.h,v 1.20 2002/05/13 06:51:13 vedge Exp $	*/

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
#ifdef DEBUG
void		 view_dumpmask(struct viewport *);
#endif

void		 scroll(struct map *, int);


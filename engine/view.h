/*	$Csoft: view.h,v 1.8 2002/02/25 11:07:24 vedge Exp $	*/

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
	Uint32	mapw, maph;		/* Map coordinates */
	Uint32	mapx, mapy;		/* Map coordinates */
	Uint32	mapxoffs, mapyoffs;	/* Map display offset */

	SDL_Surface	*v;		/* Surface */
};

extern struct viewport *mainview;	/* view.c */

struct viewport *view_create(int, int, int, int);
int		 view_setmode(struct viewport *, struct map *, int, char *);
void		 view_destroy(struct viewport *);
int		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);

void		 scroll(struct map *, int);


/*	$Csoft: view.h,v 1.7 2002/02/21 02:20:14 vedge Exp $	*/

enum {
	VIEW_MAPNAV,	/* Map navigation display */
	VIEW_MAPEDIT,	/* Map edition display */
	VIEW_FIGHT	/* Battle display */
};

struct viewport {
	int	mode;			/* Display mode */
	int	flags;
	int	fps;			/* Refresh rate in FPS */
	int	width, height, depth;	/* Viewport geometry */
	struct	map *map;		/* Currently visible map */
	int	mapw, maph;		/* Map coordinates */
	int	mapx, mapy;		/* Map coordinates */
	int	mapxoffs, mapyoffs;	/* Map display offset */

	SDL_Surface	*v;		/* Surface */
};

extern struct viewport *mainview;	/* view.c */

struct viewport *view_create(int, int, int, int);
int		 view_setmode(struct viewport *, struct map *, int, char *);
void		 view_destroy(struct viewport *);
int		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);

void		 scroll(struct map *, int);


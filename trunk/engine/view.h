/*	$Csoft: view.h,v 1.10 2002/02/28 12:52:40 vedge Exp $	*/

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

#define MAPMASK_NORENDER	0x01	/* Don't render node */
	Uint32	**mapmask;		/* Mask covering the map view */

	SDL_Surface	*v;		/* Surface */
};

extern struct viewport *mainview;	/* view.c */

struct viewport *view_create(Uint32, Uint32, Uint32, Uint32);
int		 view_setmode(struct viewport *, struct map *, int, char *);
void		 view_destroy(struct viewport *);
int		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);
void		 view_maskfill(struct viewport *, SDL_Rect *, Uint32);

void		 scroll(struct map *, int);


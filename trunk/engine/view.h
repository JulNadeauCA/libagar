/*	$Csoft: view.h,v 1.3 2002/02/10 01:38:40 vedge Exp $	*/

struct viewport {
	int	fps;			/* Refresh rate in FPS */
	int	width, height, depth;	/* Viewport geometry */
	int	tilew, tileh;		/* Tile geometry */
	int	flags;			/* SDL surface flags */
	int	mapw, maph;		/* Width/height in nodes */
	int	mapx, mapy;		/* Map coordinates */
	int	redraw;			/* Need redrawing */
	struct	map *map;		/* Currently visible map */
	SLIST_HEAD(, window) winsh;	/* Viewport sub-surfaces */
	SDL_Surface *v;
};

struct window {
	char	*caption;
	int	 width, height;
	int	 x, y;
	struct	 viewport *view;	/* Back pointer to viewport */
	SDL_Surface *v;
	SLIST_ENTRY(window) wins;
};

extern struct viewport *mainview;	/* view.c */

struct viewport *view_create(int, int, int, int, int, int);
int		 view_setmode(struct viewport *);
void		 view_destroy(struct viewport *);
int		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);
struct window	*window_create(struct viewport *, int, int, int, int, char *);
void		 window_destroy(void *, void *);
void		 window_draw(struct window *);

void		 scroll(struct map *, int);


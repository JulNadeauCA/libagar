/*	$Csoft$	*/

struct viewport {
	int	fps;			/* Refresh rate in FPS */
	int	width, height, depth;	/* Viewport geometry */
	int	tilew, tileh;		/* Tile geometry */
	int	flags;			/* SDL surface flags */
	int	mapw, maph;		/* Width/height in map units */
	int	mapx, mapy;		/* Coordinates withint his map */
	int	redraw;			/* Need redrawing */
	struct	map *map;		/* Currently visible map */
	GSList	*wins;			/* Viewport sub-surfaces */
	SDL_TimerID mapdrawt;		/* Map redraw timer */
	SDL_TimerID mapanimt;		/* Map animation timer */
	SDL_Surface *v;
};

struct window {
	char	*caption;
	int	 width, height;
	int	 x, y;
	struct	 viewport *view;	/* Back pointer to viewport */
	SDL_Surface *v;
};

extern struct viewport *mainview;	/* view.c */

struct viewport *view_create(int, int, int, int, int, int);
int		 view_setmode(struct viewport *);
void		 view_destroy(struct viewport *);
int		 view_fullscreen(struct viewport *, int);
void		 view_center(struct viewport *, int, int);
struct window	*window_create(struct viewport *, int, int, int, int, char *);
void		 window_destroy(void *, void *);
void		 window_draw(struct window *w);

#define SCROLL_UP(m)	decrease(m->view->mapy, 1, 0)
#define SCROLL_DOWN(m)	increase(m->view->mapy, 1, \
			    m->maph - m->view->maph)
#define SCROLL_LEFT(m)	decrease(m->view->mapx, 1, 0)
#define SCROLL_RIGHT(m)	increase(m->view->mapx, 1, \
			    m->mapw - m->view->mapw)


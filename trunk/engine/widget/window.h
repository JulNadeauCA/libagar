/*	$Csoft: window.h,v 1.9 2002/04/30 00:57:36 vedge Exp $	*/

struct window {
	struct	 object obj;
	
	Uint32	 flags;
#define WINDOW_PLAIN	0x01	/* Solid, no borders */
#define WINDOW_FOCUS	0x02	/* Receive keyboard events */
#define WINDOW_ANIMATE	0x04	/* Redraw each tick */
#define WINDOW_TITLEBAR	0x08	/* Draw title bar */

	Uint32	bgtype;
#define WINDOW_SOLID	0x04
#define WINDOW_GRADIENT	0x08
#define WINDOW_CUBIC	0x10
#define WINDOW_CUBIC2	0x20

	char	*caption;		/* Titlebar text */

	Uint32	 bgcolor, fgcolor;	/* Gradient colors, if applicable */
	Uint32	 border[5];		/* Border colors */
	
	Sint16	 x, y;			/* Absolute coordinates */
	Uint16	 w, h;			/* Geometry */

	int	 redraw;		/* Redraw at next tick */

	SDL_Rect vmask;			/* View mask (units) */

	TAILQ_HEAD(, widget) widgetsh;	/* Widgets within this window */
	pthread_mutex_t lock;		/* Lock on widgets list */
	
	struct viewport *view;		/* Parent view */
	TAILQ_ENTRY(window) windows;	/* Windows within this view */
};

#define WINDOW(w)		((struct window *)(w))

#define WINDOW_INSIDE(wina, xa, ya)					\
	((xa) > (wina)->x		&& (ya) > (wina)->y &&		\
	 (xa) < ((wina)->x+(wina)->w)	&& (ya) < ((wina)->y+(wina)->h))

struct window	*window_new(struct viewport *, char *, Uint32,
		     Uint32, Sint16, Sint16, Uint16, Uint16);
void	 	 window_init(struct window *, struct viewport *, char *,
		     Uint32, Uint32, Sint16, Sint16, Uint16, Uint16);

void	 window_destroy(void *);
int	 window_link(void *);
int	 window_unlink(void *);

void	 window_draw(struct window *);
void	 window_mouse_motion(SDL_Event *);
void	 window_mouse_button(SDL_Event *);
void	 window_key(SDL_Event *);


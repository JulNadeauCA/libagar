/*	$Csoft: window.h,v 1.11 2002/05/02 06:26:29 vedge Exp $	*/

typedef enum {
	WINDOW_SOLID,		/* Plain, no decorations. */
	WINDOW_GRADIENT,	/* Blue/red gradient. */
	WINDOW_CUBIC,		/* Weird algorithm #1 */
	WINDOW_CUBIC2		/* Weird algorithm #2 */
} window_type_t;

typedef enum {
	WINSEG_HORIZ,		/* Horizontal segment */
	WINSEG_VERT		/* Vertical segment */
} window_seg_t;

struct winseg {
	struct	window *win;
	struct	winseg *pseg;
	int	req;			/* Requested % of parent */
	window_seg_t	type;		/* Segment style */
	int	x, y;			/* Allocated coordinates */
	int	w, h;			/* Allocated geometry */
	SLIST_ENTRY(winseg) winsegs;	/* Segments in window */
};

struct window {
	struct	 object obj;
	
	int	 flags;
#define WINDOW_PLAIN	0x01	/* Solid, no borders */
#define WINDOW_FOCUS	0x02	/* Receive keyboard events */
#define WINDOW_ANIMATE	0x04	/* Redraw each tick */
#define WINDOW_TITLEBAR	0x08	/* Draw title bar */

	window_type_t	type;

	char	*caption;		/* Titlebar text */

	Uint32	 bgcolor, fgcolor;	/* Gradient colors, if applicable */
	Uint32	 border[5];		/* Border colors */
	
	Sint16	 x, y;			/* Absolute coordinates */
	Uint16	 w, h;			/* Geometry */

	int	 redraw;		/* Redraw at next tick */

	SDL_Rect vmask;			/* View mask (units) */

	TAILQ_HEAD(, widget) widgetsh;	/* Widgets within this window */
	SLIST_HEAD(, winseg) winsegsh;	/* Widgets within this window */
	pthread_mutex_t lock;		/* Lock on lists */
	
	struct	winseg *rootseg;	/* Root segment */
	
	struct viewport *view;		/* Parent view */
	TAILQ_ENTRY(window) windows;	/* Windows within this view */
};

struct window_event {
	struct	widget *w;
	int	flags;
	SDL_Event ev;
};

#define WINDOW(w)		((struct window *)(w))

#define WINDOW_INSIDE(wina, xa, ya)					\
	((xa) > (wina)->x		&& (ya) > (wina)->y &&		\
	 (xa) < ((wina)->x+(wina)->w)	&& (ya) < ((wina)->y+(wina)->h))

struct window	*window_new(struct viewport *, char *, Uint32, window_type_t,
		     Sint16, Sint16, Uint16, Uint16);
void	 	 window_init(struct window *, struct viewport *, char *,
		     Uint32, Uint32, Sint16, Sint16, Uint16, Uint16);
struct winseg	*winseg_new(struct window *, struct winseg *, window_seg_t,
		     int);
void		 winseg_destroy(struct winseg *);

void	 window_destroy(void *);
void	 window_link(struct window *);
void	 window_unlink(struct window *);

void	 window_draw(struct window *);
void	 window_mouse_motion(SDL_Event *);
void	 window_mouse_button(SDL_Event *);
void	 window_key(SDL_Event *);
void	 window_draw_all(void);


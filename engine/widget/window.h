/*	$Csoft: window.h,v 1.38 2002/09/12 09:15:53 vedge Exp $	*/
/*	Public domain	*/

#include <engine/widget/region.h>

enum window_event {
	WINDOW_MOUSEBUTTONUP,
	WINDOW_MOUSEBUTTONDOWN,
	WINDOW_KEYUP,
	WINDOW_KEYDOWN,
	WINDOW_MOUSEMOTION,
	WINDOW_MOUSEOUT
};
	
typedef enum {
	WINDOW_SOLID =		0x100,	/* Plain, no decorations. */
	WINDOW_GRADIENT =	0x200,	/* Blue/red gradient. */
#define WINDOW_TYPE		(WINDOW_SOLID | WINDOW_GRADIENT)
#define WINDOW_DEFAULT_TYPE	(WINDOW_GRADIENT)
} window_type_t;

TAILQ_HEAD(regionsq, region);

struct window {
	struct	 widget wid;

	/*
	 * Read-only once attached
	 */
	int	 flags;
#define WINDOW_PLAIN		0x001	/* Solid, no borders */
#define WINDOW_TITLEBAR		0x008	/* Draw title bar */
#define WINDOW_ROUNDEDGES	0x010	/* Round edges */
#define WINDOW_SCALE		0x020	/* Scale regions/widgets */
#define WINDOW_SHOWN		0x040	/* Visible */
#define WINDOW_CENTER		0x080	/* Center window */
#define WINDOW_SAVE_POSITION	0x100	/* Save window position on hide/exit */

	window_type_t	type;

	char	*caption;		/* Titlebar text */
	Uint32	 caption_color;
	SDL_Surface *caption_s;

	Uint32	 bgcolor, fgcolor;	/* Gradient colors, if applicable */
	Uint32	*border;		/* Border colors */
	int	 borderw;		/* Border width */
	int	 titleh;		/* Titlebar height */
	int	 x, y;			/* Absolute coordinates */
	int	 w, h;			/* Geometry */
	int	 minw, minh;
	int	 spacing;		/* Spacing between regions */
	SDL_Rect body;			/* Area reserved for regions */
	SDL_Rect vmask;			/* View mask (in nodes)
					   Used only in tile mode. */
	/*
	 * Read-write, thread-safe
	 */
	int	 redraw;		/* Redraw at next tick */
	struct	 widget *focus;		/* Focused widget */

	struct	 regionsq regionsh;
	TAILQ_ENTRY(window) windows;	/* Windows in view */

	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;
};

#define WINDOW(w)	((struct window *)(w))

#define WINDOW_FOCUSED(w) (TAILQ_LAST(&view->windowsh, \
			   windowq) == (w))

#define WINDOW_CYCLE(w)	 do {					\
	TAILQ_REMOVE(&view->windowsh, (w), windows);		\
	TAILQ_INSERT_HEAD(&view->windowsh, (w), windows);	\
} while (/*CONSTCOND*/0)

#ifdef DEBUG

# define WINDOW_PUT_PIXEL(win, wrx, wry, c) do {			\
	if ((wrx) > (win)->w || (wry) > (win)->h ||			\
	    (wrx) < 0 || (wry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(win)->name,		\
		    (wrx), (wry), (win)->w, (win)->h);			\
	}								\
	VIEW_PUT_PIXEL(view->v, (win)->x+(wrx),	(win)->y+(wry), (c));	\
} while (/*CONSTCOND*/0)

# define WINDOW_PUT_ALPHAPIXEL(win, wrx, wry, c, wa) do {		\
	if ((wrx) > (win)->w || (wry) > (win)->h ||			\
	    (wrx) < 0 || (wry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(win)->name,		\
		    (wrx), (wry), (win)->w, (win)->h);			\
	}								\
	VIEW_PUT_ALPHAPIXEL(view->v, (win)->x+(wrx),			\
	    (win)->y+(wry), (c), (wa));					\
} while (/*CONSTCOND*/0)

#else

# define WINDOW_PUT_PIXEL(win, wrx, wry, c)				\
 	 VIEW_PUT_PIXEL(view->v, (win)->x+(wrx), (win)->y+(wry),	\
	    (c))

# define WINDOW_PUT_ALPHAPIXEL(win, wrx, wry, c, wa) \
	VIEW_PUT_ALPHAPIXEL(view->v, (win)->x+(wrx), (win)->y+(wry), (c), (wa))

#endif

#define WINDOW_SURFACE(win)	(view->v)

#define WINDOW_INSIDE(wina, xa, ya)					\
	((xa) > (wina)->x		&& (ya) > (wina)->y &&		\
	 (xa) < ((wina)->x+(wina)->w)	&& (ya) < ((wina)->y+(wina)->h))

struct window	*window_new(char *, char *, int, int, int, int, int, int, int);
void	 	 window_init(struct window *, char *, char *, int, int, int,
		     int, int, int, int);
int		 window_load(void *, int);
int		 window_save(void *, int);
void		 window_destroy(void *);
void		 window_attach(void *, void *);
void		 window_detach(void *, void *);

int	 window_show(struct window *);
int	 window_hide(struct window *);
void	 window_draw(struct window *);
int	 window_event_all(SDL_Event *);
void	 window_resize(struct window *);
void	 window_titlebar_printf(struct window *, const char *, ...);


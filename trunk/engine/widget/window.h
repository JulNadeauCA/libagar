/*	$Csoft: window.h,v 1.50 2002/12/16 02:14:26 vedge Exp $	*/
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
	
TAILQ_HEAD(regionsq, region);

struct window {
	struct	 widget wid;		/* For primitives and color scheme */

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
#define WINDOW_MATERIALIZE	0x200	/* Materialize effect */
#define WINDOW_DEMATERIALIZE	0x400	/* Dematerialize effect */
#define WINDOW_HIDDEN_BODY	0x1000	/* Only the titlebar is shown */
#define WINDOW_PERSISTENT	WINDOW_HIDDEN_BODY

	char	*caption;		/* Titlebar text */

	Uint32	*border;		/* Border colors */
	int	 borderw;		/* Border width */
	int	 titleh;		/* Titlebar height */
	SDL_Rect rd;			/* Current geometry */
	SDL_Rect saved_rd;		/* Original geometry */
	int	 minw, minh;
	int	 spacing;		/* Spacing between regions */
	SDL_Rect body;			/* Area reserved for regions */
	
	/*
	 * Read-write, thread-safe
	 */
	struct	 widget *focus;		/* Focused widget */

	struct	 regionsq regionsh;
	TAILQ_ENTRY(window) windows;	/* Windows in view */
	TAILQ_ENTRY(window) detach;	/* Windows to free */

	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;
};

#define WINDOW(w)	((struct window *)(w))

#define WINDOW_CYCLE(w)	 do {					\
	TAILQ_REMOVE(&view->windows, (w), windows);		\
	TAILQ_INSERT_HEAD(&view->windows, (w), windows);	\
} while (/*CONSTCOND*/0)

#ifdef DEBUG

# define WINDOW_PUT_PIXEL(win, wrx, wry, c) do {			\
	if ((wrx) > (win)->rd.w || (wry) > (win)->rd.h ||		\
	    (wrx) < 0 || (wry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(win)->name,		\
		    (wrx), (wry), (win)->rd.w, (win)->rd.h);		\
	}								\
	VIEW_PUT_PIXEL(view->v, (win)->rd.x+(wrx), (win)->rd.y+(wry), (c)); \
} while (/*CONSTCOND*/0)

# define WINDOW_PUT_ALPHAPIXEL(win, wrx, wry, c, wa) do {		\
	if ((wrx) > (win)->rd.w || (wry) > (win)->rd.h ||		\
	    (wrx) < 0 || (wry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(win)->name,		\
		    (wrx), (wry), (win)->rd.w, (win)->rd.h);		\
	}								\
	VIEW_PUT_ALPHAPIXEL(view->v, (win)->rd.x+(wrx),			\
	    (win)->rd.y+(wry), (c), (wa));				\
} while (/*CONSTCOND*/0)

#else	/* !DEBUG */

# define WINDOW_PUT_PIXEL(win, wrx, wry, c)				\
 	 VIEW_PUT_PIXEL(view->v, (win)->rd.x+(wrx), (win)->rd.y+(wry),	\
	    (c))

# define WINDOW_PUT_ALPHAPIXEL(win, wrx, wry, c, wa) \
	VIEW_PUT_ALPHAPIXEL(view->v, (win)->rd.x+(wrx), (win)->rd.y+(wry), \
	    (c), (wa))

#endif	/* DEBUG */

#define WINDOW_SURFACE(win)	(view->v)

#define WINDOW_INSIDE(win, xa, ya)				\
	((xa) > (win)->rd.x && (ya) > (win)->rd.y &&		\
	 (xa) < ((win)->rd.x+(win)->rd.w) && 			\
	 (ya) < ((win)->rd.y+(win)->rd.h))

#define WINDOW_FOCUSED(w)	(TAILQ_LAST(&view->windows, windowq) == (w))


struct window	*window_new(char *, int, int, int, int, int, int, int);
struct window	*window_generic_new(int, int, const char *, ...);
void	 	 window_init(struct window *, char *, int, int, int, int, int,
		     int, int);

int	 window_load(void *, int);
int	 window_save(void *, int);
void	 window_destroy(void *);
void	 window_attach(void *, void *);
void	 window_detach(void *, void *);

int	 window_show(struct window *);
int	 window_hide(struct window *);
void	 window_draw(struct window *);
int	 window_event(SDL_Event *);
void	 window_resize(struct window *);
void	 window_set_caption(struct window *, const char *, ...);

void	 window_generic_detach(int, union evarg *);
void	 window_generic_hide(int, union evarg *);


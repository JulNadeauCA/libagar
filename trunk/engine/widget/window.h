/*	$Csoft: window.h,v 1.54 2003/01/23 02:00:57 vedge Exp $	*/
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
	struct widget	wid;		/* For primitives and color schemes */

	/* Read-only once attached */

	int	flags;
#define WINDOW_TITLEBAR		0x01	/* Draw a title bar */
#define WINDOW_SCALE		0x02	/* Scale the initial geometry (%) */
#define WINDOW_CENTER		0x04	/* Center the initial position */
#define WINDOW_SHOWN		0x10	/* Window is visible */
#define WINDOW_SAVE_POSITION	0x20	/* Save position/geometry on close */
#define WINDOW_HIDDEN_BODY	0x40	/* Draw the titlebar only */
#define WINDOW_PERSISTENT	WINDOW_HIDDEN_BODY

	enum {
		WINDOW_NO_BUTTON,
		WINDOW_CLOSE_BUTTON,
		WINDOW_HIDE_BUTTON
	} clicked_button;
	Uint32	*border;		/* Border colors */
	Uint8	 borderw;		/* Border width */
	Uint8	 titleh;		/* Titlebar height */
	Uint16	 minw, minh;		/* Minimum window geometry */
	Uint8	 xspacing, yspacing;	/* Spacing between regions */
	SDL_Rect body;			/* Area reserved for regions */
	
	/* Read-write, thread-safe */
	pthread_mutex_t		lock;
	pthread_mutexattr_t	lockattr;
	SDL_Rect		 rd;		/* Current geometry */
	SDL_Rect		 saved_rd;	/* Original geometry */
	char			*caption;	/* Titlebar text */
	struct widget		*focus;		/* Focused widget */
	struct regionsq		 regionsh;	/* Regions */
	TAILQ_ENTRY(window)	 windows;	/* Windows in view */
	TAILQ_ENTRY(window)	 detach;	/* Windows to free */
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
void	 window_set_spacing(struct window *, Uint8, Uint8);
void	 window_set_geo(struct window *, Uint16, Uint16);
void	 window_set_position(struct window *, Sint16, Sint16);
void	 window_set_min_geo(struct window *, Uint16, Uint16);
void	 window_set_titleh(struct window *, Uint8);

void	 window_generic_detach(int, union evarg *);
void	 window_generic_hide(int, union evarg *);


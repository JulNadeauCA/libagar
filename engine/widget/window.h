/*	$Csoft: window.h,v 1.14 2002/05/19 14:30:24 vedge Exp $	*/

#include <engine/widget/region.h>

enum window_type {
	WINDOW_SOLID,		/* Plain, no decorations. */
	WINDOW_GRADIENT,	/* Blue/red gradient. */
	WINDOW_CUBIC,		/* Weird algorithm #1 */
	WINDOW_CUBIC2		/* Weird algorithm #2 */
};

struct window {
	struct	 object obj;

	/* Read-only once attached */
	int	 flags;
#define WINDOW_PLAIN	0x01	/* Solid, no borders */
#define WINDOW_FOCUS	0x02	/* Receive keyboard events */
#define WINDOW_ANIMATE	0x04	/* Redraw each tick */
#define WINDOW_TITLEBAR	0x08	/* Draw title bar */
	enum	 window_type type;
	char	*caption;		/* Titlebar text */
	Uint32	 bgcolor, fgcolor;	/* Gradient colors, if applicable */
	Uint32	 border[5];		/* Border colors */
	int	 x, y;			/* Absolute coordinates */
	int	 w, h;			/* Geometry */
	int	 xmargin, ymargin;
	struct	 viewport *view;	/* Parent view */
	SDL_Rect vmask;			/* View mask (units) */

	/* Read-write, thread-safe */
	int	 redraw;		/* Redraw at next tick */
	
	SLIST_HEAD(, region) regionsh;	/* Regions */
	TAILQ_ENTRY(window) windows;	/* Windows in view */
	pthread_mutex_t	lock;
};

struct window_event {
	struct	widget *w;
	int	flags;
	SDL_Event ev;
};

#define WINDOW(w)	((struct window *)(w))

#define WINDOW_INSIDE(wina, xa, ya)					\
	((xa) > (wina)->x		&& (ya) > (wina)->y &&		\
	 (xa) < ((wina)->x+(wina)->w)	&& (ya) < ((wina)->y+(wina)->h))

struct window	*window_new(char *, int, enum window_type, int, int, int, int);
void	 	 window_init(struct window *, struct viewport *, char *,
		     int, enum window_type, int, int, int, int);

void	 window_destroy(void *);
void	 window_onattach(void *, void *);
void	 window_ondetach(void *, void *);
void	 window_attach(void *, void *);
void	 window_detach(void *, void *);

void	 window_draw(struct window *);
void	 window_draw_all(void);
void	 window_widget_event(void *, void *, void *);
void	 window_resize(struct window *);


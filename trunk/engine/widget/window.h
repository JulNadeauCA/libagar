/*	$Csoft: window.h,v 1.1 2002/04/20 05:46:51 vedge Exp $	*/

struct window {
	struct	 object obj;
	
	Uint32	 flags;
#define WINDOW_PLAIN	0x01		/* Solid, no borders */
#define WINDOW_FOCUS	0x02

	Uint32	bgtype;
#define WINDOW_SOLID	0x04
#define WINDOW_GRADIENT	0x08
#define WINDOW_CUBIC	0x10

	char	*caption;		/* Titlebar text */

	Uint32	 bgcolor, fgcolor;	/* Gradient colors, if applicable */
	Uint32	 border[5];		/* Border colors */
	
	Sint16	 x, y, w, h;		/* Rectangle within view (pixels) */

	SDL_Rect vmask;			/* View mask (units) */

	TAILQ_HEAD(, widget) widgetsh;	/* Widgets within this window */
	pthread_mutex_t widgetslock;	/* Lock on widgets list */
	
	struct viewport *view;		/* Parent view */
	TAILQ_ENTRY(window) windows;	/* Windows within this view */
};

#define WINDOW(w)	((struct window *)(w))

struct window	*window_create(struct viewport *, char *, char *, Uint32,
		    Uint32, Sint16, Sint16, Uint16, Uint16);
int		 window_destroy(void *);
int		 window_load(void *, int);
int		 window_save(void *, int);
int		 window_link(void *);
int		 window_unlink(void *);

int		 window_init(void);
void		 window_quit(void);
void		 window_draw(struct window *);
void		 window_drawall(void);
void		 window_event(SDL_Event *, Uint32);


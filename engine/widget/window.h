/*	$Csoft$	*/

enum window_bg {
	WINDOW_SOLID,
	WINDOW_GRADIENT,
	WINDOW_CUBIC
};

struct window {
	struct	 object obj;

	Uint32	 flags;
#define WINDOW_PLAIN	0x01		/* Solid, no borders */

	enum	 window_bg bgtype;

	char	*caption;		/* Titlebar text */

	Uint32	*bgcolor, *fgcolor;	/* Gradient colors, if applicable */
	Uint32	 border[4];		/* Border colors */

	SDL_Surface	*surface;

	SDL_Rect rect;			/* Rectangle within view (pixels) */
	SDL_Rect vmask;			/* View mask (units) */

	TAILQ_HEAD(, widget) widgetsh;	/* Widgets within this window */
	pthread_mutex_t widgetslock;	/* Lock on widgets list */

	struct viewport *view;		/* Parent view */
	TAILQ_ENTRY(window) windows;	/* Windows within this view */
};

#define WINDOW(w)	((struct window *)(w))

struct window	*window_create(struct viewport *, char *, char *, Uint32,
		    enum window_bg, SDL_Rect, Uint32 *, Uint32 *);
int		 window_destroy(void *);
int		 window_load(void *, int);
int		 window_save(void *, int);
int		 window_link(void *);
int		 window_unlink(void *);

int		 window_init(void);
void		 window_quit(void);
void		 window_draw(struct window *);
void		 window_drawall(void);


/*	$Csoft: widget.h,v 1.40 2002/11/13 00:22:31 vedge Exp $	*/
/*	Public domain	*/

#define WIDGET_MAXCOLORS	16

struct widget_ops {
	const	 struct object_ops obops;

	/* Draw directly to video memory. */
	void	 (*widget_draw)(void *);

	/* Update cached surface. */
	void	 (*widget_update)(void *, SDL_Surface *);
};

struct widget_color {
	char	*name;		/* Identifier */
	int	ind;		/* Index into color array */
	SLIST_ENTRY(widget_color) colors;
};

SLIST_HEAD(widget_colorq, widget_color);

struct window;
struct region;

/* Structure shares the parent window's lock. */
struct widget {
	struct	 object obj;
	int	 flags;
#define WIDGET_NO_FOCUS			0x01	/* Cannot gain focus. */
#define WIDGET_UNFOCUSED_MOTION		0x02	/* Receive window-mousemotion
						   events even when the widget
						   isn't focused. */
#define WIDGET_UNFOCUSED_BUTTONUP	0x04	/* Receive window-mousebuttonup
						   events even when the widget
						   isn't focused. */
	struct {
		SDL_Surface	*source;	/* Source surface */
		int		 redraw;	/* Update the source surface */
		pthread_mutex_t  lock;
	} surface;

	char	*type;			/* Widget type identifier */
	struct	window *win;		/* Parent window */
	struct	region *reg;		/* Parent region */
	int	rw, rh;			/* Requested geometry (%) */
	int	x, y;			/* Allocated coordinates in window */
	int	w, h;			/* Allocated geometry */

	/* Color scheme */
	Uint32	color[WIDGET_MAXCOLORS];
	int	ncolors;
	struct	widget_colorq colors;

	TAILQ_ENTRY(widget) widgets;		/* Widgets inside region */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDGET_OPS(ob)	((struct widget_ops *)OBJECT((ob))->ops)

#define WIDGET_COLOR(wi,ind)	WIDGET(wi)->color[ind]

/* Expand to absolute widget coordinates. */
#define WIDGET_ABSX(wi)	((WIDGET((wi))->win->rd.x) + WIDGET((wi))->x)
#define WIDGET_ABSY(wi)	((WIDGET((wi))->win->rd.y) + WIDGET((wi))->y)

#define WIDGET_REDRAW(wi) do {					\
	pthread_mutex_lock(&WIDGET((wi))->surface.lock);	\
	WIDGET((wi))->surface.redraw++;				\
	pthread_mutex_unlock(&WIDGET((wi))->surface.lock);	\
} while (/*CONSTCOND*/0)

/* XXX optimize - move rect to wid structure */
#define WIDGET_DRAW(wi, s, xo, yo) do {					\
	static SDL_Rect _wdrd;						\
									\
	_wdrd.x = WIDGET_ABSX((wi)) + (xo);				\
	_wdrd.y = WIDGET_ABSY((wi)) + (yo);				\
	_wdrd.w = (s)->w;						\
	_wdrd.h = (s)->h;						\
	SDL_BlitSurface((s), NULL, WIDGET_SURFACE((wi)), &_wdrd);	\
} while (/*CONSTCOND*/0)

/* XXX optimize - move rect to wid structure */
#define WIDGET_FILL(wi, xo, yo, wdrw, wdrh, col) do {		\
	static SDL_Rect _wdrd;					\
								\
	_wdrd.x = WIDGET_ABSX((wi)) + (xo);			\
	_wdrd.y = WIDGET_ABSY((wi)) + (yo);			\
	_wdrd.w = (wdrw);					\
	_wdrd.h = (wdrh);					\
	SDL_FillRect(WIDGET_SURFACE((wi)), &_wdrd, (col));	\
} while (/*CONSTCOND*/0)

#ifdef DEBUG

# define WIDGET_PUT_PIXEL(wid, wdrx, wdry, c) do {			\
	if ((wdrx) > WIDGET((wid))->w || (wdry) > WIDGET((wid))->h ||	\
	    (wdrx) < 0 || (wdry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(wid)->name,		\
		    (wdrx), (wdry), WIDGET((wid))->w,			\
		    WIDGET((wid))->h);					\
	}								\
	WINDOW_PUT_PIXEL(WIDGET((wid))->win,				\
	    WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c));	\
} while (/*CONSTCOND*/0)

# define WIDGET_PUT_ALPHAPIXEL(wid, wdrx, wdry, c, wa) do {		\
	if ((wdrx) > WIDGET((wid))->w || (wdry) > WIDGET((wid))->h ||	\
	    (wdrx) < 0 || (wdry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(wid)->name,		\
		    (wdrx), (wdry), WIDGET((wid))->w,			\
		    WIDGET((wid))->h);					\
	}								\
	WINDOW_PUT_ALPHAPIXEL(WIDGET((wid))->win,			\
	    WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c),	\
	    (wa));							\
} while (/*CONSTCOND*/0)

#else

# define WIDGET_PUT_PIXEL(wid, wdrx, wdry, c)				\
	 WINDOW_PUT_PIXEL(WIDGET((wid))->win,				\
	     WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c))

# define WIDGET_PUT_ALPHAPIXEL(wid, wdrx, wdry, c, wa)			\
	 WINDOW_PUT_ALPHAPIXEL(WIDGET((wid))->win,			\
	     WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c), (wa))

#endif	/* DEBUG */

#define WIDGET_SURFACE(wid)	(WINDOW_SURFACE(WIDGET((wid))->win))

#define WIDGET_FOCUSED(wid)	(WIDGET((wid))->win->focus == WIDGET((wid)))

#define WIDGET_FOCUS(wid) do {					\
	if ((WIDGET((wid))->flags & WIDGET_NO_FOCUS) == 0) {	\
		WIDGET((wid))->win->focus = WIDGET((wid));	\
		event_post((wid), "widget-gainfocus", NULL);	\
	}							\
} while (/*CONSTCOND*/0)

/* Test whether absolute coordinates match the widget area. */
#define WIDGET_INSIDE(wida, xa, ya)				\
    ((xa) > WIDGET_ABSX((wida))	&&				\
     (ya) > WIDGET_ABSY((wida)) &&				\
     (xa) < (WIDGET_ABSX((wida)) + WIDGET((wida))->w) &&	\
     (ya) < (WIDGET_ABSY((wida)) + WIDGET((wida))->h))

#define WIDGET_INSIDE_RELATIVE(wida, xa, ya)	\
    ((xa) >= 0 && 				\
     (ya) >= 0 &&				\
     (xa) <= WIDGET((wida))->w &&		\
     (ya) <= WIDGET((wida))->h)

#ifdef DEBUG
# define WIDGET_ASSERT(ob, typestr) do {				\
	if (strcmp(WIDGET((ob))->type, typestr) != 0) {			\
		fprintf(stderr, "%s:%d: %s is not a %s\n", __FILE__,	\
		    __LINE__, WIDGET((ob))->type, typestr);		\
		abort();						\
	}								\
} while (/*CONSTCOND*/0)
#else
#define WIDGET_ASSERT(wid, name)
#endif

void	widget_init(struct widget *, char *, char *, const void *, int, int);
void	widget_destroy(void *);
void	widget_map_color(void *, int, char *, Uint8, Uint8, Uint8);

void	widget_attach(void *, void *);
void	widget_detach(void *, void *);


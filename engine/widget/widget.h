/*	$Csoft: widget.h,v 1.24 2002/07/07 06:34:25 vedge Exp $	*/
/*	Public domain	*/

struct window;

struct widget_ops {
	const	 struct object_ops obops;
	void	 (*widget_draw)(void *);
	void	 (*widget_animate)(void *);
};

struct widget {
	struct	 object obj;
	int	 flags;
#define WIDGET_HIDE		0x01	/* Don't draw widget */
#define WIDGET_MOUSEOUT		0x02	/* Receive window-mouseout events */
	
	char	*type;			/* Widget type identifier */
	struct	window *win;		/* Parent window */
	int	rw, rh;			/* Requested geometry (%) */
	int	x, y;			/* Allocated coordinates in window */
	int	w, h;			/* Allocated geometry */

	TAILQ_ENTRY(widget) widgets;	/* Widgets inside region */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDGET_OPS(ob)	((struct widget_ops *)OBJECT((ob))->ops)

/* Expand to absolute widget coordinates. */
#define WIDGET_ABSX(wi)	((WIDGET((wi))->win->x) + WIDGET((wi))->x)
#define WIDGET_ABSY(wi)	((WIDGET((wi))->win->y) + WIDGET((wi))->y)

#define WIDGET_DRAW(wi, s, xo, yo) do {					\
	static SDL_Rect wdrd;						\
									\
	wdrd.x = WIDGET_ABSX((wi)) + (xo);				\
	wdrd.y = WIDGET_ABSY((wi)) + (yo);				\
	wdrd.w = (s)->w;						\
	wdrd.h = (s)->h;						\
	SDL_BlitSurface((s), NULL, WIDGET_SURFACE((wi)), &wdrd);	\
} while (/*CONSTCOND*/0)

/* XXX inconsistent, should be region-relative */
#ifdef DEBUG

# define WIDGET_PUT_PIXEL(wid, wdrx, wdry, c) do {			\
	if ((wdrx) > WIDGET((wid))->w || (wdry) > WIDGET((wid))->h ||	\
	    (wdrx) < 0 || (wdry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(wid)->name,		\
		    (wdrx), (wdry), WIDGET((wid))->w, WIDGET((wid))->h);\
	}								\
	WINDOW_PUT_PIXEL(WIDGET((wid))->win,				\
	    WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c));	\
} while (/*CONSTCOND*/0)

# define WIDGET_PUT_ALPHAPIXEL(wid, wdrx, wdry, c, wa) do {		\
	if ((wdrx) > WIDGET((wid))->w || (wdry) > WIDGET((wid))->h ||	\
	    (wdrx) < 0 || (wdry) < 0) {					\
		fatal("%s: %d,%d > %dx%d\n", OBJECT(wid)->name,		\
		    (wdrx), (wdry), WIDGET((wid))->w, WIDGET((wid))->h);\
	}								\
	WINDOW_PUT_ALPHAPIXEL(WIDGET((wid))->win,			\
	    WIDGET((wid))->x+(wdrx), WIDGET((wid))->y+(wdry), (c), (wa)); \
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

#define WIDGET_FOCUSED(wid)	(WIDGET(wid)->win->focus == WIDGET(wid))

#define WIDGET_FOCUS(wid) do {			\
	WIDGET(wid)->win->focus = WIDGET(wid);	\
	WIDGET(wid)->win->redraw++;		\
} while (/*CONSTCOND*/0)

/* Test whether absolute coordinates match the widget area. */
#define WIDGET_INSIDE(wida, xa, ya)				\
    ((xa) > WIDGET_ABSX((wida))	&&				\
     (ya) > WIDGET_ABSY((wida)) &&				\
     (xa) < (WIDGET_ABSX((wida)) + WIDGET((wida))->w) &&	\
     (ya) < (WIDGET_ABSY((wida)) + WIDGET((wida))->h))

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

/* Sprites. All widgets share the same art. */
enum {
	CHECKBOX_UP,
	CHECKBOX_DOWN
};

void	widget_init(struct widget *, char *, char *, const void *, int, int);
void	widget_event(void *, SDL_Event *, int);


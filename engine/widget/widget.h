/*	$Csoft: widget.h,v 1.31 2002/07/29 05:29:29 vedge Exp $	*/
/*	Public domain	*/

#define WIDGET_MAXCOLORS	16

struct widget_ops {
	const	 struct object_ops obops;
	void	 (*widget_draw)(void *);
	void	 (*widget_animate)(void *);
};

struct widget_color {
	char	*name;		/* Identifier */
	int	ind;		/* Index into color array */
	SLIST_ENTRY(widget_color) colors;
};

SLIST_HEAD(widget_colorq, widget_color);

struct window;
struct region;

struct widget {
	struct	 object obj;
	int	 flags;
#define WIDGET_NO_FOCUS		0x01	/* Cannot gain focus */
#define WIDGET_MOUSEOUT		0x02	/* Receive window-mouseout events */
	
	char	*type;			/* Widget type identifier */
	struct	window *win;		/* Parent window */
	struct	region *reg;		/* Parent region */
	int	rw, rh;			/* Requested geometry (%) */
	int	x, y;			/* Allocated coordinates in window */
	int	w, h;			/* Allocated geometry */

	Uint32	color[WIDGET_MAXCOLORS];
	int	ncolors;
	struct	widget_colorq colors;

	TAILQ_ENTRY(widget) widgets;	/* Widgets inside region */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDGET_OPS(ob)	((struct widget_ops *)OBJECT((ob))->ops)

#define WIDGET_COLOR(wi,ind)	(WIDGET(wi)->color[ind])

/* Expand to absolute widget coordinates. */
#define WIDGET_ABSX(wi)	((WIDGET((wi))->win->x) + WIDGET((wi))->x)
#define WIDGET_ABSY(wi)	((WIDGET((wi))->win->y) + WIDGET((wi))->y)

#define WIDGET_DRAW(wi, s, xo, yo) do {					\
	static SDL_Rect _wdrd;						\
									\
	_wdrd.x = WIDGET_ABSX((wi)) + (xo);				\
	_wdrd.y = WIDGET_ABSY((wi)) + (yo);				\
	_wdrd.w = (s)->w;						\
	_wdrd.h = (s)->h;						\
	SDL_BlitSurface((s), NULL, WIDGET_SURFACE((wi)), &_wdrd);	\
} while (/*CONSTCOND*/0)

#define WIDGET_FILL(wi, xo, yo, w, h, col) do {				\
	static SDL_Rect _wdrd;						\
									\
	_wdrd.x = WIDGET_ABSX((wi)) + (xo);				\
	_wdrd.y = WIDGET_ABSY((wi)) + (yo);				\
	_wdrd.w = (w);							\
	_wdrd.h = (h);							\
	SDL_FillRect(WIDGET_SURFACE((wi)), &_wdrd, col);		\
} while (/*CONSTCOND*/0)

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

#define WIDGET_FOCUS(wid) do {					\
	if ((WIDGET(wid)->flags & WIDGET_NO_FOCUS) == 0) {	\
		WIDGET(wid)->win->focus = WIDGET(wid);		\
	}							\
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

void	widget_init(struct widget *, char *, char *, const void *, int, int);
void	widget_event(void *, SDL_Event *, int);
void	widget_map_color(void *, int, char *, Uint8, Uint8, Uint8);


/*	$Csoft: widget.h,v 1.16 2002/05/19 15:27:56 vedge Exp $	*/

struct window;

struct widget_ops {
	const	 struct object_ops obops;

	/* Render widget. */
	void	 (*widget_draw)(void *);
	/* Dispatch an event to this widget. */
	void	 (*widget_event)(void *, SDL_Event *, int);
};

struct widget {
	struct	 object obj;

	int	 flags;
#define WIDGET_HIDE		0x01	/* Don't draw widget */
#define WIDGET_FOCUS		0x02	/* Catch keys and mouse motion */

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

/* Blit surface to coordinates relative to the widget area. */
#define WIDGET_DRAW(wi, s, xo, yo) do {					\
	static SDL_Rect wdrd;						\
									\
	wdrd.x = WIDGET_ABSX((wi)) + (xo);				\
	wdrd.y = WIDGET_ABSY((wi)) + (yo);				\
	wdrd.w = (s)->w;						\
	wdrd.h = (s)->h;						\
	SDL_BlitSurface((s), NULL, WIDGET((wi))->win->view->v, &wdrd);	\
} while (/*CONSTCOND*/0)

/* Test whether absolute coordinates match the widget area. */
#define WIDGET_INSIDE(wida, xa, ya)				\
    ((xa) > WIDGET_ABSX((wida))	&&				\
     (ya) > WIDGET_ABSY((wida)) &&				\
     (xa) < (WIDGET_ABSX((wida)) + WIDGET((wida))->w) &&	\
     (ya) < (WIDGET_ABSY((wida)) + WIDGET((wida))->h))

/* Sprites. All widgets share the same art. */
enum {
	CHECKBOX_UP,
	CHECKBOX_DOWN
};

void	widget_init(struct widget *, char *, char *, const void *, int, int);
void	widget_event(void *, SDL_Event *, int);


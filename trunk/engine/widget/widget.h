/*	$Csoft: widget.h,v 1.14 2002/05/15 07:28:13 vedge Exp $	*/

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
#define WIDGET_HIDE		0x01	/* Don't draw widget. */
#define WIDGET_FOCUS		0x02	/* Catch keys and mouse motion. */

	struct	window *win;		/* Parent window */
	Sint16	x, y;			/* Coordinates within parent window */
	Uint16	w, h;			/* Can be defined by draw routine */

	SLIST_ENTRY(widget) widgets;	/* Widgets inside region */
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
	BUTTON_UP = 0,
	BUTTON_DOWN,
	CHECKBOX_UP,
	CHECKBOX_DOWN
};

void	widget_init(struct widget *, char *, char *, const void *, int, int);
void	widget_event(void *, SDL_Event *, int);


/*	$Csoft: widget.h,v 1.4 2002/04/21 13:33:44 vedge Exp $	*/

struct widvec {
	struct	 obvec obvec;
	void	 (*draw)(void *);
	void	 (*event)(void *, SDL_Event *, Uint32);
};

struct widget {
	struct	 object obj;

	Uint32	 flags;
#define WIDGET_HIDE		0x01	/* Don't draw widget. */
#define WIDGET_FOCUS		0x02	/* Catch keys and mouse motion. */

	struct	window *win;		/* Parent window */
	Sint16	x, y;			/* Coordinates within parent window */
	Uint16	w, h;			/* Can be defined by draw routine */

	TAILQ_ENTRY(widget) widgets;	/* Widgets within parent window */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDGET_VEC(ob)	((struct widvec *)OBJECT((ob))->vec)

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
    ((xa) > WIDGET_ABSX((wida))	&& (ya) > WIDGET((wida))->y &&	\
     (xa) < (WIDGET_ABSX((wida)) + WIDGET((wida))->w) &&	\
     (ya) < (WIDGET_ABSY((wida)) + WIDGET((wida))->h))

void		 widget_init(struct widget *, char *, Uint32, void *,
		     struct window *, Sint16, Sint16, Uint16, Uint16);
int		 widget_link(void *);
int		 widget_unlink(void *);

void		 widget_draw(void *);
void		 widget_event(void *, SDL_Event *, Uint32);


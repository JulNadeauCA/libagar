/*	$Csoft: widget.h,v 1.3 2002/04/20 06:20:57 vedge Exp $	*/

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
	SDL_Rect rect;			/* Rectangle within parent window;
					   width/height may be zero. */

	TAILQ_ENTRY(widget) widgets;	/* Widgets within parent window */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDVEC(ob)	((struct widvec *)OBJECT((ob))->vec)

#define WIDGET_DRAW(wi, s, xo, yo) do {					\
	SDL_Rect wdrd;							\
									\
	wdrd.x = WIDGET((wi))->rect.x + WIDGET((wi))->win->x + (xo);	\
	wdrd.y = WIDGET((wi))->rect.y + WIDGET((wi))->win->y + (yo);	\
	wdrd.w = (s)->w;						\
	wdrd.h = (s)->h;						\
	SDL_BlitSurface((s), NULL, WIDGET((wi))->win->view->v, &wdrd);	\
} while (/*CONSTCOND*/0)

struct widget	*widget_create(struct window *, char *, Uint32, SDL_Rect,
		     Uint32 *);
int		 widget_destroy(void *);
int		 widget_load(void *, int);
int		 widget_save(void *, int);
int		 widget_link(void *);
int		 widget_unlink(void *);

void		 widget_init(struct widget *, char *, Uint32, void *,
		     struct window *, SDL_Rect);
void		 widget_draw(void *);
void		 widget_event(void *, SDL_Event *, Uint32);


/*	$Csoft: widget.h,v 1.2 2002/04/20 05:48:48 vedge Exp $	*/

struct widvec {
	struct	 obvec obvec;
	void	 (*draw)(void *);
};

struct widget {
	struct	 object obj;

	Uint32	 flags;
#define WIDGET_HIDE		0x01	/* Don't draw widget. */
#define WIDGET_FOCUS		0x02	/* Catch keys and mouse motion. */

	Uint32	*fgcolor;		/* Foreground color */

	struct	window *win;		/* Parent window */
	SDL_Rect rect;			/* Rectangle within parent window;
					   width/height may be zero. */

	TAILQ_ENTRY(widget) widgets;	/* Widgets within parent window */
};

#define WIDGET(wi)	((struct widget *)(wi))
#define WIDVEC(ob)	((struct widvec *)OBJECT((ob))->vec)

#define WIDGET_DRAW(wi, s) do {						\
	static SDL_Rect rd;						\
									\
	rd = WIDGET((wi))->rect;					\
	rd.x += WIDGET((wi))->win->rect.x;				\
	rd.y += WIDGET((wi))->win->rect.y;				\
	SDL_BlitSurface(s, NULL, WIDGET((wi))->win->view->v, &rd);	\
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


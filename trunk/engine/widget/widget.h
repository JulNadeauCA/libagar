/*	$Csoft: widget.h,v 1.1 2002/04/18 03:57:28 vedge Exp $	*/

struct widvec {
	struct	 obvec obvec;
	void	 (*draw)(void *);
};

struct widget {
	struct	 object obj;

	Uint32	 flags;
#define WIDGET_HIDE		0x01	/* Don't draw widget. */

	Uint32	*fgcolor;		/* Foreground color */

	struct	window *win;		/* Parent window */
	SDL_Rect rect;			/* Rectangle within parent window */

	TAILQ_ENTRY(widget) widgets;	/* Widgets within parent window */
};

#define WIDVEC(ob)	((struct widvec *)OBJECT((ob))->vec)

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


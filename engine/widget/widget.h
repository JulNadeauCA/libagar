/*	$Csoft$	*/

struct widget {
	struct	 object obj;

	Uint32	 flags;
#define WIDGET_HIDE		0x01	/* Don't draw widget. */

	SDL_Rect rect;			/* Rectangle within window. */
	SDL_Rect vmask;			/* View mask (units). */
	Uint32	*bgcolor, *fgcolor;	/* Preferred color pattern. */

	struct	window *win;		/* Parent window. */
	TAILQ_ENTRY(widget) widgets;	/* Widgets within this window. */
};

struct widget	*widget_create(struct window *, char *, Uint32, SDL_Rect,
		     Uint32 *, Uint32 *);
int		 widget_destroy(void *);
int		 widget_load(void *, int);
int		 widget_save(void *, int);
int		 widget_link(void *);
int		 widget_unlink(void *);


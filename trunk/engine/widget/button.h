/*	$Csoft: button.h,v 1.8 2002/04/30 00:57:36 vedge Exp $	*/

struct button {
	struct	 widget wid;

	int	flags;
#define BUTTON_PRESSED	0x01

	char	*caption;

	enum {
		BUTTON_LEFT,
		BUTTON_CENTER,
		BUTTON_RIGHT
	} justify;

	Uint8	xmargin, ymargin;

	void	(*push)(struct button *);
};

struct button	*button_new(struct window *, char *, int, Sint16, Sint16);
void		 button_init(struct button *, char *, int, Sint16, Sint16);
void		 button_destroy(void *);

void	 button_draw(void *);
void	 button_event(void *, SDL_Event *, int);


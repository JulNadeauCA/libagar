/*	$Csoft: button.h,v 1.7 2002/04/26 11:40:48 vedge Exp $	*/

struct button {
	struct	 widget wid;

	Uint32	 flags;
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

struct button	*button_new(struct window *, char *, Uint32, Sint16, Sint16);
void		 button_init(struct button *, char *, Uint32, Sint16, Sint16);
void		 button_destroy(void *);

void	 button_draw(void *);
void	 button_event(void *, SDL_Event *, Uint32);


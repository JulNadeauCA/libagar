/*	$Csoft: button.h,v 1.6 2002/04/24 14:08:54 vedge Exp $	*/

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

void	 button_init(struct button *, char *, char *, Uint32, Sint16, Sint16);
void	 button_destroy(void *);

void	 button_draw(void *);
void	 button_event(void *, SDL_Event *, Uint32);


/*	$Csoft: button.h,v 1.2 2002/04/21 13:35:20 vedge Exp $	*/

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

/* Sprites */
enum {
	BUTTON_UP = 0,
	BUTTON_DOWN
};

struct button	*button_create(struct window *, char *, char *, Uint32,
		     Sint16, Sint16);
int		 button_destroy(void *);
int		 button_link(void *);
int		 button_unlink(void *);

void		 button_draw(void *);
void		 button_event(void *, SDL_Event *, Uint32);


/*	$Csoft: button.h,v 1.9 2002/05/02 06:28:30 vedge Exp $	*/

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

struct button	*button_new(struct region *, char *, int, int, int);
void		 button_init(struct button *, char *, int, int, int);
void		 button_destroy(void *);

void	 button_draw(void *);
void	 button_event(void *, SDL_Event *, int);


/*	$Csoft: button.h,v 1.10 2002/05/19 14:30:24 vedge Exp $	*/

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
};

struct button	*button_new(struct region *, char *, int, int, int);
void		 button_init(struct button *, char *, int, int, int);
void		 button_destroy(void *);

void	 button_draw(void *);


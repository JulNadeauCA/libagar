/*	$Csoft: button.h,v 1.11 2002/06/01 02:52:09 vedge Exp $	*/
/*	Public domain	*/

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


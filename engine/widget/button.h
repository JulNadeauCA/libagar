/*	$Csoft: button.h,v 1.15 2002/08/21 04:24:54 vedge Exp $	*/
/*	Public domain	*/

struct button {
	struct	 widget wid;

	int	flags;
#define BUTTON_PRESSED	0x01		/* Button is pressed */
#define BUTTON_STICKY	0x02		/* Button state is static */

	char	*caption;		/* String, or NULL */

	SDL_Surface	*label_s;	/* Label (or image) */
	SDL_Surface	*slabel_s;	/* Scaled label surface */

	enum {
		BUTTON_LEFT,
		BUTTON_CENTER,
		BUTTON_RIGHT
	} justify;

	Uint8	xmargin, ymargin;
};

struct button	*button_new(struct region *, char *, SDL_Surface *, int, int,
		     int);
void		 button_init(struct button *, char *, SDL_Surface *, int, int,
		     int);
void		 button_destroy(void *);

void	 button_draw(void *);


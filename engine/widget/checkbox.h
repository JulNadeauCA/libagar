/*	$Csoft: checkbox.h,v 1.2 2002/04/30 00:57:36 vedge Exp $	*/

struct checkbox {
	struct	 widget wid;

	int	 flags;
#define CHECKBOX_PRESSED	0x01

	char	*caption;
	Uint8	xspacing;	/* Horiz spacing */

	enum {
		CHECKBOX_LEFT,	/* Left of label */
		CHECKBOX_RIGHT	/* Right of label */
	} justify;

	void	(*push)(struct checkbox *);
};

struct checkbox	*checkbox_new(struct window *, char *, int, Sint16, Sint16);
void		 checkbox_init(struct checkbox *, char *, int, Sint16, Sint16);
void		 checkbox_destroy(void *);

void	 checkbox_draw(void *);
void	 checkbox_event(void *, SDL_Event *, int);


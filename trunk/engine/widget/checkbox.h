/*	$Csoft: checkbox.h,v 1.3 2002/05/02 06:28:30 vedge Exp $	*/

struct checkbox {
	struct	 widget wid;

	int	 flags;
#define CHECKBOX_PRESSED	0x01

	char		*caption;
	SDL_Surface	*label_s;

	Uint8	 xspacing;	/* Horiz spacing */

	enum {
		CHECKBOX_LEFT,	/* Left of label */
		CHECKBOX_RIGHT	/* Right of label */
	} justify;

	void	(*push)(struct checkbox *);
};

struct checkbox	*checkbox_new(struct region *, char *, int);
void		 checkbox_init(struct checkbox *, char *, int);
void		 checkbox_destroy(void *);

void	 checkbox_draw(void *);
void	 checkbox_event(void *, SDL_Event *, int);


/*	$Csoft$	*/

struct checkbox {
	struct	 widget wid;

	Uint32	 flags;
#define CHECKBOX_PRESSED	0x01

	char	*caption;
	Uint8	xspacing;	/* Horiz spacing */

	enum {
		CHECKBOX_LEFT,	/* Left of label */
		CHECKBOX_RIGHT	/* Right of label */
	} justify;

	void	(*push)(struct checkbox *);
};

void	 checkbox_init(struct checkbox *, char *, char *, Uint32, Sint16,
	     Sint16);
void	 checkbox_destroy(void *);

void	 checkbox_draw(void *);
void	 checkbox_event(void *, SDL_Event *, Uint32);


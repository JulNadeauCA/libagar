/*	$Csoft$	*/

struct button {
	struct	 widget wid;

	char	*caption;

	Uint32	 flags;
#define BUTTON_PRESSED	0x01

	enum {
		LABEL_LEFT,
		LABEL_CENTER,
		LABEL_RIGHT
	} justify;
};

struct button	*button_create(struct window *, char *, char *, Uint32,
		     Uint32, Uint32);
int		 button_destroy(void *);
int		 button_link(void *);
int		 button_unlink(void *);

void		 button_draw(void *);


/*	$Csoft: label.h,v 1.3 2002/04/22 04:38:23 vedge Exp $	*/

struct label {
	struct	 widget wid;

	char	 caption[1024];

	Uint32	 flags;
#define LABEL_BOLD	0x01
#define LABEL_ITALIC	0x02

	enum {
		LABEL_LEFT,
		LABEL_CENTER,
		LABEL_RIGHT
	} justify;
};

void	 label_init(struct label *, struct window *, char *, char *,
	     Uint32, Sint16, Sint16 y);
void	 label_destroy(void *);
int	 label_link(void *);
int	 label_unlink(void *);

void	 label_draw(void *);


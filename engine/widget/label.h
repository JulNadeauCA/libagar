/*	$Csoft: label.h,v 1.4 2002/04/24 14:08:54 vedge Exp $	*/

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

void	 label_init(struct label *, char *, char *, Uint32, Sint16, Sint16 y);
void	 label_destroy(void *);

void	 label_draw(void *);


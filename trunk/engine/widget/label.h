/*	$Csoft: label.h,v 1.2 2002/04/21 08:02:56 vedge Exp $	*/

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

struct label	*label_create(struct window *, char *, char *, Uint32,
		     Sint16, Sint16);
int		 label_destroy(void *);
int		 label_link(void *);
int		 label_unlink(void *);

void		 label_draw(void *);


/*	$Csoft: label.h,v 1.1 2002/04/18 03:57:28 vedge Exp $	*/

struct label {
	struct	 widget wid;

	char	*caption;

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
		     Uint32, Uint32);
int		 label_destroy(void *);
int		 label_link(void *);
int		 label_unlink(void *);

void		 label_draw(void *);


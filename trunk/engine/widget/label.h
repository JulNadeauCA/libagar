/*	$Csoft: label.h,v 1.6 2002/04/30 00:57:36 vedge Exp $	*/

struct label {
	struct	 widget wid;

	char	 caption[1024];

	int	 flags;
#define LABEL_BOLD	0x01
#define LABEL_ITALIC	0x02

	enum {
		LABEL_LEFT,
		LABEL_CENTER,
		LABEL_RIGHT
	} justify;
};

struct label	*label_new(struct window *, char *, int, Sint16, Sint16);
void		 label_init(struct label *, char *, int, Sint16, Sint16);
void	 	 label_destroy(void *);

void	 label_draw(void *);


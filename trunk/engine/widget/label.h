/*	$Csoft: label.h,v 1.7 2002/05/02 06:28:30 vedge Exp $	*/

struct label {
	struct	 widget wid;

	char	 *caption;
	SDL_Surface *label_s;

	int	 flags;
#define LABEL_BOLD	0x01
#define LABEL_ITALIC	0x02

	enum {
		LABEL_LEFT,
		LABEL_CENTER,
		LABEL_RIGHT
	} justify;
};

struct label	*label_new(struct region *, char *, int);
void		 label_init(struct label *, char *, int);
void	 	 label_destroy(void *);

void	 label_draw(void *);


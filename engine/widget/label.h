/*	$Csoft: label.h,v 1.10 2002/05/28 12:45:49 vedge Exp $	*/
/*	Public domain	*/

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

struct label	*label_new(struct region *, const char *, int);
void		 label_init(struct label *, const char *, int);
void	 	 label_destroy(void *);
void		 label_draw(void *);
void		 label_printf(struct label *, const char *, ...);


/*	$Csoft: label.h,v 1.12 2002/08/21 23:51:39 vedge Exp $	*/
/*	Public domain	*/

struct label {
	struct	 widget wid;

	struct {
		char		*caption;
		pthread_mutex_t	 lock;
		SDL_Surface	*surface;
	} text;
	
	int	 flags;
#define LABEL_BOLD	0x01		/* Bold text */
#define LABEL_ITALIC	0x02		/* Italic text */

	enum {
		LABEL_LEFT,
		LABEL_CENTER,
		LABEL_RIGHT
	} justify;
};

struct label	*label_new(struct region *, const char *, int, int);
void		 label_init(struct label *, const char *, int, int);
void	 	 label_destroy(void *);
void		 label_draw(void *);
void		 label_printf(struct label *, const char *, ...);


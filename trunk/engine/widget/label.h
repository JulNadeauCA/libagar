/*	$Csoft: label.h,v 1.13 2002/09/11 23:54:37 vedge Exp $	*/
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

struct label	*label_new(struct region *, int, int, const char *, ...);
void		 label_init(struct label *, const char *, int, int);
void	 	 label_destroy(void *);
void		 label_draw(void *);
void		 label_printf(struct label *, const char *, ...);


/*	$Csoft: bitmap.h,v 1.1 2002/09/01 08:55:43 vedge Exp $	*/
/*	Public domain	*/

struct bitmap {
	struct	 widget wid;

	SDL_Surface *surface;		/* Original */
	SDL_Surface *surface_s;		/* Scaled */
};

struct bitmap	*bitmap_new(struct region *, SDL_Surface *, int, int);
void		 bitmap_init(struct bitmap *, SDL_Surface *, int, int);
void	 	 bitmap_destroy(void *);
void		 bitmap_draw(void *);
void		 bitmap_scaled(int, union evarg *);


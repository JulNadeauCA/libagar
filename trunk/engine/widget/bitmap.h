/*	$Csoft: bitmap.h,v 1.2 2002/09/07 04:36:59 vedge Exp $	*/
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
void		 bitmap_set_surface(struct bitmap *, SDL_Surface *);


/*	$Csoft: bitmap.h,v 1.12 2002/08/21 23:51:39 vedge Exp $	*/
/*	Public domain	*/

struct bitmap {
	struct	 widget wid;

	SDL_Surface *surface;		/* Original */
	SDL_Surface *surface_s;		/* Scaled */

	int	 flags;
};

struct bitmap	*bitmap_new(struct region *, SDL_Surface *, int, int, int);
void		 bitmap_init(struct bitmap *, SDL_Surface *, int, int, int);
void	 	 bitmap_destroy(void *);
void		 bitmap_draw(void *);
void		 bitmap_scaled(int, union evarg *);


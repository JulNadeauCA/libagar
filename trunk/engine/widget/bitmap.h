/*	$Csoft: bitmap.h,v 1.4 2002/12/21 10:26:33 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BITMAP_H_
#define _AGAR_WIDGET_BITMAP_H_

#include <engine/widget/widget.h>

struct bitmap {
	struct widget	wid;

	SDL_Surface	*surface;		/* Original */
	SDL_Surface	*surface_s;		/* Scaled */
};

struct bitmap	*bitmap_new(struct region *, SDL_Surface *, int, int);
void		 bitmap_init(struct bitmap *, SDL_Surface *, int, int);
void	 	 bitmap_destroy(void *);
void		 bitmap_draw(void *);
void		 bitmap_scaled(int, union evarg *);
void		 bitmap_set_surface(struct bitmap *, SDL_Surface *);

#endif /* _AGAR_WIDGET_BITMAP_H */

/*	$Csoft: bitmap.h,v 1.5 2003/02/02 21:16:15 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BITMAP_H_
#define _AGAR_WIDGET_BITMAP_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct bitmap {
	struct widget	wid;
	SDL_Surface	*surface;		/* Original */
	SDL_Surface	*surface_s;		/* Scaled */
};

__BEGIN_DECLS
extern DECLSPEC struct bitmap	*bitmap_new(struct region *, SDL_Surface *,
				            int, int);
extern DECLSPEC void		 bitmap_init(struct bitmap *, SDL_Surface *,
				             int, int);
extern DECLSPEC void	 	 bitmap_destroy(void *);
extern DECLSPEC void		 bitmap_draw(void *);
extern DECLSPEC void		 bitmap_scaled(int, union evarg *);
extern DECLSPEC void		 bitmap_set_surface(struct bitmap *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BITMAP_H */

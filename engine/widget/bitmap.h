/*	$Csoft: bitmap.h,v 1.8 2003/06/06 03:18:14 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BITMAP_H_
#define _AGAR_WIDGET_BITMAP_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct bitmap {
	struct widget wid;

	SDL_Surface	*surface;		/* Original */
	SDL_Surface	*surface_s;		/* Scaled */
};

__BEGIN_DECLS
struct bitmap	*bitmap_new(void *);

void	 bitmap_init(struct bitmap *);
void	 bitmap_destroy(void *);
void	 bitmap_draw(void *);
void	 bitmap_scale(void *, int, int);
void	 bitmap_set_surface(struct bitmap *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BITMAP_H */

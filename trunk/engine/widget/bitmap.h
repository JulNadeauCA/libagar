/*	$Csoft: bitmap.h,v 1.9 2003/06/18 00:47:04 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BITMAP_H_
#define _AGAR_WIDGET_BITMAP_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

struct bitmap {
	struct widget wid;
	int pre_w, pre_h;
};

__BEGIN_DECLS
struct bitmap	*bitmap_new(void *);
void		 bitmap_init(struct bitmap *);
void		 bitmap_prescale(struct bitmap *, int, int);
void		 bitmap_destroy(void *);
void		 bitmap_draw(void *);
void		 bitmap_scale(void *, int, int);
void		 bitmap_set_surface(struct bitmap *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BITMAP_H */

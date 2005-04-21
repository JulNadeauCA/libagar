/*	$Csoft: palette.h,v 1.7 2005/01/23 11:49:13 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PALETTE_H_
#define _AGAR_WIDGET_PALETTE_H_

#include <engine/widget/scrollbar.h>

#include "begin_code.h"

enum palette_type {
	PALETTE_RGB,		/* RGB scrollbars */
	PALETTE_RGBA		/* RGBA scrollbars */
};

struct palette {
	struct widget	   wid;
	Uint32		   color;	/* Default pixel binding */

	enum palette_type  type;
	struct scrollbar   *bars[4];	/* RGB[A] scrollbars */
	int		   nbars;
	SDL_Rect	   rpreview;	/* Color preview rectangle */
};

__BEGIN_DECLS
struct palette	*palette_new(void *, enum palette_type, SDL_PixelFormat **);
void		 palette_init(struct palette *, enum palette_type,
		              SDL_PixelFormat **);
void		 palette_destroy(void *);
void		 palette_scale(void *, int, int);
void		 palette_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PALETTE_H_ */

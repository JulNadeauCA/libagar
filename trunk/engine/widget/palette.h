/*	$Csoft: palette.h,v 1.4 2003/04/25 09:47:10 vedge Exp $	*/
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
	Uint32		   color;	/* Pixel binding */

	enum palette_type  type;
	struct scrollbar   *bars[4];	/* RGB[A] scrollbars */
	int		   nbars;
	SDL_Rect	   rpreview;	/* Color preview rectangle */
};

__BEGIN_DECLS
extern DECLSPEC struct palette *palette_new(void *, enum palette_type);

extern DECLSPEC void	 palette_init(struct palette *, enum palette_type);
extern DECLSPEC void	 palette_destroy(void *);
extern DECLSPEC void	 palette_scale(void *, int, int);
extern DECLSPEC void	 palette_draw(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PALETTE_H_ */

/*	$Csoft: bitmap.h,v 1.10 2005/01/28 12:49:51 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_BITMAP_H_
#define _AGAR_WIDGET_BITMAP_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

typedef struct ag_bitmap {
	struct ag_widget wid;
	int pre_w, pre_h;
} AG_Bitmap;

__BEGIN_DECLS
AG_Bitmap	*AG_BitmapNew(void *);
void		 AG_BitmapInit(AG_Bitmap *);
void		 AG_BitmapPrescale(AG_Bitmap *, int, int);
void		 AG_BitmapDestroy(void *);
void		 AG_BitmapDraw(void *);
void		 AG_BitmapScale(void *, int, int);
void		 AG_BitmapSetSurface(AG_Bitmap *, SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_BITMAP_H */

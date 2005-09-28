/*	$Csoft: bitmap.h,v 1.11 2005/09/27 00:25:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

typedef struct ag_pixmap {
	struct ag_widget wid;
	int n;				/* Current surface */
	int s, t;			/* Source coordinates */
} AG_Pixmap;

__BEGIN_DECLS
AG_Pixmap *AG_PixmapFromSurface(void *, SDL_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceCopy(void *, SDL_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceScaled(void *, SDL_Surface *, u_int, u_int);
AG_Pixmap *AG_PixmapFromBMP(void *, const char *);
AG_Pixmap *AG_PixmapFromXCF(void *, const char *);

int	   AG_PixmapAddSurface(AG_Pixmap *, SDL_Surface *);
int	   AG_PixmapAddSurfaceCopy(AG_Pixmap *, SDL_Surface *);
int	   AG_PixmapAddSurfaceScaled(AG_Pixmap *, SDL_Surface *, u_int, u_int);

void	   AG_PixmapInit(AG_Pixmap *);
void	   AG_PixmapDestroy(void *);
void	   AG_PixmapDraw(void *);
void	   AG_PixmapScale(void *, int, int);

__inline__ void AG_PixmapSetSurface(AG_Pixmap *, int);
__inline__ void AG_PixmapSetCoords(AG_Pixmap *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PIXMAP_H */

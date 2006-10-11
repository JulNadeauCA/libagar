/*	$Csoft: bitmap.h,v 1.11 2005/09/27 00:25:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#include <agar/gui/widget.h>

#include "begin_code.h"

typedef struct ag_pixmap {
	struct ag_widget wid;
	Uint flags;
#define AG_PIXMAP_HFILL		0x01
#define AG_PIXMAP_VFILL		0x02
#define AG_PIXMAP_FORCE_SIZE	0x04	/* Always override image size */
#define AG_PIXMAP_EXPAND (AG_PIXMAP_HFILL|AG_PIXMAP_VFILL)

	int n;			/* Current surface (or -1) */
	int s, t;		/* Source coordinates */
	int pre_w, pre_h;	/* Geometry to use if there is no surface */
} AG_Pixmap;

__BEGIN_DECLS
AG_Pixmap *AG_PixmapNew(void *, Uint, Uint, Uint);
AG_Pixmap *AG_PixmapFromSurface(void *, Uint, SDL_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceCopy(void *, Uint, SDL_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceScaled(void *, Uint, SDL_Surface *, Uint, Uint);
AG_Pixmap *AG_PixmapFromBMP(void *, Uint, const char *);
AG_Pixmap *AG_PixmapFromXCF(void *, Uint, const char *);

void	   AG_PixmapInit(AG_Pixmap *, Uint);
void	   AG_PixmapDestroy(void *);
void	   AG_PixmapDraw(void *);
void	   AG_PixmapScale(void *, int, int);

int	   AG_PixmapAddSurface(AG_Pixmap *, SDL_Surface *);
int	   AG_PixmapAddSurfaceCopy(AG_Pixmap *, SDL_Surface *);
int	   AG_PixmapAddSurfaceScaled(AG_Pixmap *, SDL_Surface *, Uint, Uint);
void	   AG_PixmapReplaceSurfaceScaled(AG_Pixmap *, SDL_Surface *, Uint,
		                         Uint);

__inline__ void AG_PixmapSetSurface(AG_Pixmap *, int);
__inline__ void AG_PixmapSetCoords(AG_Pixmap *, int, int);

#define AG_PixmapReplaceSurface(px,su)	AG_WidgetReplaceSurface((px),(px)->n,su)
#define AG_PixmapUpdateSurface(px)	AG_WidgetUpdateSurface((px),(px)->n)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PIXMAP_H */

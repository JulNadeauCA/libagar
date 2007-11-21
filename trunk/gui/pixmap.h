/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

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
extern AG_WidgetClass agPixmapClass;

AG_Pixmap *AG_PixmapNew(void *, Uint, Uint, Uint);
AG_Pixmap *AG_PixmapFromSurface(void *, Uint, SDL_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceCopy(void *, Uint, SDL_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceScaled(void *, Uint, SDL_Surface *, Uint, Uint);
AG_Pixmap *AG_PixmapFromBMP(void *, Uint, const char *);
#if 0
AG_Pixmap *AG_PixmapFromXCF(void *, Uint, const char *);
#endif

int	   AG_PixmapAddSurface(AG_Pixmap *, SDL_Surface *);
int	   AG_PixmapAddSurfaceFromBMP(AG_Pixmap *, const char *);
int	   AG_PixmapAddSurfaceCopy(AG_Pixmap *, SDL_Surface *);
int	   AG_PixmapAddSurfaceScaled(AG_Pixmap *, SDL_Surface *, Uint, Uint);
void	   AG_PixmapReplaceSurfaceScaled(AG_Pixmap *, SDL_Surface *, Uint,
		                         Uint);

#define AG_PixmapReplaceSurface(px,su)	AG_WidgetReplaceSurface((px),(px)->n,su)
#define AG_PixmapUpdateSurface(px)	AG_WidgetUpdateSurface((px),(px)->n)

static __inline__ int
AG_PixmapSetSurface(AG_Pixmap *px, int name)
{
	if (name < 0 || name >= AGWIDGET(px)->nsurfaces) {
		return (-1);
	}
	px->n = name;
	return (0);
}

static __inline__ void
AG_PixmapSetCoords(AG_Pixmap *px, int s, int t)
{
	px->s = s;
	px->t = t;
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PIXMAP_H */

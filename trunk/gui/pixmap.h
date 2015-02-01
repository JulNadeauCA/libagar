/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#include <agar/gui/widget.h>
#include <agar/gui/window.h>

#include <agar/gui/begin.h>

typedef struct ag_pixmap {
	struct ag_widget wid;
	Uint flags;
#define AG_PIXMAP_HFILL		0x01
#define AG_PIXMAP_VFILL		0x02
#define AG_PIXMAP_FORCE_SIZE	0x04	/* Always override image size */
#define AG_PIXMAP_RESCALE	0x08	/* Scale image to fit widget */
#define AG_PIXMAP_UPDATE	0x10	/* Scaled copy needs updating */
#define AG_PIXMAP_EXPAND (AG_PIXMAP_HFILL|AG_PIXMAP_VFILL)

	int n;			/* Current surface (or -1) */
	int s, t;		/* Source coordinates */
	int pre_w, pre_h;	/* Geometry to use if there is no surface */
	int sScaled;		/* Scaled surface (for RESCALE) */
	AG_Rect rClip;		/* Clipping rectangle (for !RESCALE) */
} AG_Pixmap;

__BEGIN_DECLS
extern AG_WidgetClass agPixmapClass;

AG_Pixmap *AG_PixmapNew(void *, Uint, Uint, Uint);
AG_Pixmap *AG_PixmapFromSurface(void *, Uint, const AG_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceScaled(void *, Uint, const AG_Surface *, Uint, Uint);
AG_Pixmap *AG_PixmapFromSurfaceNODUP(void *, Uint, AG_Surface *);
AG_Pixmap *AG_PixmapFromFile(void *, Uint, const char *);
AG_Pixmap *AG_PixmapFromTexture(void *, Uint, Uint, int);

int	   AG_PixmapAddSurface(AG_Pixmap *, const AG_Surface *);
int	   AG_PixmapAddSurfaceScaled(AG_Pixmap *, const AG_Surface *, Uint, Uint);
int	   AG_PixmapAddSurfaceFromFile(AG_Pixmap *, const char *);

#ifdef AG_LEGACY
AG_Pixmap *AG_PixmapFromBMP(void *, Uint, const char *)			DEPRECATED_ATTRIBUTE;
int	   AG_PixmapAddSurfaceFromBMP(AG_Pixmap *, const char *)	DEPRECATED_ATTRIBUTE;
#define AG_PixmapFromSurfaceCopy AG_PixmapFromSurface
#define AG_PixmapReplaceCurrentSurface(px,su) \
	AG_PixmapReplaceSurface((px),(px)->n,(su))
#define AG_PixmapReplaceCurrentSurfaceScaled(px,su,w,h) \
        AG_PixmapReplaceSurfaceScaled((px),(px)->n,(su),(w),(h))
#define AG_PixmapUpdateCurrentSurface(px) \
	AG_PixmapUpdateSurface((px),(px)->n)
#endif /* AG_LEGACY */

static __inline__ void
AG_PixmapReplaceSurface(AG_Pixmap *px, int name, AG_Surface *su)
{
	AG_WidgetReplaceSurface(px, name, su);
	AG_Redraw(px);
}

static __inline__ void
AG_PixmapUpdateSurface(AG_Pixmap *px, int name)
{
	AG_WidgetUpdateSurface(px, name);
	AG_Redraw(px);
}

static __inline__ int
AG_PixmapSetSurface(AG_Pixmap *px, int name)
{
	AG_ObjectLock(px);
	if (name < 0 || name >= (int)AGWIDGET(px)->nsurfaces) {
		AG_ObjectUnlock(px);
		return (-1);
	}
	px->n = name;
	px->flags |= AG_PIXMAP_UPDATE;
	AG_ObjectUnlock(px);
	return (0);
}

static __inline__ void
AG_PixmapSetCoords(AG_Pixmap *px, int s, int t)
{
	AG_ObjectLock(px);
	px->s = s;
	px->t = t;
	AG_ObjectUnlock(px);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_PIXMAP_H */

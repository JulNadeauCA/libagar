/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#include <agar/gui/widget.h>

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
AG_Pixmap *AG_PixmapFromSurface(void *, Uint, AG_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceCopy(void *, Uint, AG_Surface *);
AG_Pixmap *AG_PixmapFromSurfaceScaled(void *, Uint, AG_Surface *, Uint, Uint);
AG_Pixmap *AG_PixmapFromBMP(void *, Uint, const char *);
#ifdef HAVE_OPENGL
AG_Pixmap *AG_PixmapFromTexture(void *, Uint, Uint, int);
#endif

int	   AG_PixmapAddSurface(AG_Pixmap *, AG_Surface *);
int	   AG_PixmapAddSurfaceFromBMP(AG_Pixmap *, const char *);
int	   AG_PixmapAddSurfaceCopy(AG_Pixmap *, AG_Surface *);
int	   AG_PixmapAddSurfaceScaled(AG_Pixmap *, AG_Surface *, Uint, Uint);
#define    AG_PixmapReplaceSurface(px,name,su) \
           AG_WidgetReplaceSurface((px),(name),(su))
#define    AG_PixmapUpdateSurface(px,name) \
           AG_WidgetUpdateSurface((px),(name))
void	   AG_PixmapReplaceSurfaceScaled(AG_Pixmap *, int, AG_Surface *, Uint,
		                         Uint);

#define AG_PixmapReplaceCurrentSurface(px,su) \
	AG_PixmapReplaceSurface((px),(px)->n,(su))
#define AG_PixmapReplaceCurrentSurfaceScaled(px,su,w,h) \
        AG_PixmapReplaceSurfaceScaled((px),(px)->n,(su),(w),(h))
#define AG_PixmapUpdateCurrentSurface(px) \
	AG_PixmapUpdateSurface((px),(px)->n)

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

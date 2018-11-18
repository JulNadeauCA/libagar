/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/begin.h>

typedef struct ag_pixmap {
	struct ag_widget wid;		/* AG_Widget(3) -> AG_Pixmap */
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

AG_Pixmap *_Nonnull AG_PixmapNew(void *_Nullable, Uint, Uint,Uint);
AG_Pixmap *_Nonnull AG_PixmapFromSurface(void *_Nullable, Uint,
                                         const AG_Surface *_Nullable);
AG_Pixmap *_Nonnull AG_PixmapFromSurfaceScaled(void *_Nullable, Uint,
                                               const AG_Surface *_Nullable,
					       Uint,Uint);
AG_Pixmap *_Nonnull AG_PixmapFromSurfaceNODUP(void *_Nullable, Uint,
                                              AG_Surface *_Nonnull);
AG_Pixmap *_Nonnull AG_PixmapFromFile(void *_Nullable, Uint, const char *_Nonnull);
AG_Pixmap *_Nonnull AG_PixmapFromTexture(void *_Nullable, Uint, Uint, int);

int AG_PixmapAddSurface(AG_Pixmap *_Nonnull, const AG_Surface *_Nonnull);
int AG_PixmapAddSurfaceScaled(AG_Pixmap *_Nonnull, const AG_Surface *_Nonnull,
                              Uint,Uint);
int AG_PixmapAddSurfaceFromFile(AG_Pixmap *_Nonnull, const char *_Nonnull);

static __inline__ void
AG_PixmapReplaceSurface(AG_Pixmap *_Nonnull px, int name, AG_Surface *_Nonnull s)
{
	AG_WidgetReplaceSurface(px, name, s);
	AG_Redraw(px);
}

static __inline__ void
AG_PixmapUpdateSurface(AG_Pixmap *_Nonnull px, int name)
{
	AG_WidgetUpdateSurface(px, name);
	AG_Redraw(px);
}

static __inline__ int
AG_PixmapSetSurface(AG_Pixmap *_Nonnull px, int name)
{
	AG_ObjectLock(px);
	if (name < 0 || name >= (int)AGWIDGET(px)->nSurfaces) {
		AG_ObjectUnlock(px);
		return (-1);
	}
	px->n = name;
	px->flags |= AG_PIXMAP_UPDATE;
	AG_ObjectUnlock(px);
	return (0);
}

static __inline__ void
AG_PixmapSetCoords(AG_Pixmap *_Nonnull px, int s, int t)
{
	AG_ObjectLock(px);
	px->s = s;
	px->t = t;
	AG_ObjectUnlock(px);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_PIXMAP_H */

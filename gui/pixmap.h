/*	Public domain	*/

#ifndef _AGAR_WIDGET_PIXMAP_H_
#define _AGAR_WIDGET_PIXMAP_H_

#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/begin.h>

typedef struct ag_pixmap {
	struct ag_widget wid;	/* AG_Widget -> AG_Pixmap */
	Uint flags;
#define AG_PIXMAP_HFILL		0x01
#define AG_PIXMAP_VFILL		0x02
#define AG_PIXMAP_FORCE_SIZE	0x04	/* Always override image size */
#define AG_PIXMAP_RESCALE	0x08	/* Scale image to fit widget */
#define AG_PIXMAP_UPDATE	0x10	/* Scaled copy needs updating */
#define AG_PIXMAP_EXPAND (AG_PIXMAP_HFILL|AG_PIXMAP_VFILL)

	int n;			/* Current surface (or -1) */
	int s, t;		/* Source coordinates */
	int wPre, hPre;		/* Size hint */
	int sScaled;		/* Scaled surface (for RESCALE) */
	Uint32 _pad;
} AG_Pixmap;

#define AGPIXMAP(p)              ((AG_Pixmap *)(p))
#define AGCPIXMAP(p)             ((const AG_Pixmap *)(p))
#define AG_PIXMAP_SELF()          AGPIXMAP( AG_OBJECT(0,"AG_Widget:AG_Pixmap:*") )
#define AG_PIXMAP_PTR(n)          AGPIXMAP( AG_OBJECT((n),"AG_Widget:AG_Pixmap:*") )
#define AG_PIXMAP_NAMED(n)        AGPIXMAP( AG_OBJECT_NAMED((n),"AG_Widget:AG_Pixmap:*") )
#define AG_CONST_PIXMAP_SELF()   AGCPIXMAP( AG_CONST_OBJECT(0,"AG_Widget:AG_Pixmap:*") )
#define AG_CONST_PIXMAP_PTR(n)   AGCPIXMAP( AG_CONST_OBJECT((n),"AG_Widget:AG_Pixmap:*") )
#define AG_CONST_PIXMAP_NAMED(n) AGCPIXMAP( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Pixmap:*") )

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

void AG_PixmapReplaceSurface(AG_Pixmap *_Nonnull, int, AG_Surface *_Nonnull);
void AG_PixmapUpdateSurface(AG_Pixmap *_Nonnull, int);
int  AG_PixmapSetSurface(AG_Pixmap *_Nonnull, int);
void AG_PixmapSizeHint(AG_Pixmap *px, int,int);
void AG_PixmapSetCoords(AG_Pixmap *_Nonnull, int,int);

AG_Surface *_Nonnull AG_PixmapGetSurface(const AG_Pixmap *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_PIXMAP_H */

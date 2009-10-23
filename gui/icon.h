/*	Public domain	*/

#ifndef _AGAR_WIDGET_ICON_H_
#define _AGAR_WIDGET_ICON_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

struct ag_window;
struct ag_socket;

typedef struct ag_icon {
	struct ag_widget wid;
	Uint flags;
#define AG_ICON_REGEN_LABEL	0x01	/* Update text label */
#define AG_ICON_DND		0x02	/* Drag-and-drop in progress */
#define AG_ICON_DBLCLICKED	0x04	/* Double click flag */
#define AG_ICON_BGFILL		0x08	/* Enable background fill */
	int surface;			/* Icon surface */
	char labelTxt[AG_LABEL_MAX];	/* Optional text label */
	int  labelSurface;		/* Optional label surface */
	int  labelPad;			/* Icon-label spacing */
	struct ag_window *wDND;		/* For drag/drop */
	struct ag_socket *sock;		/* Back pointer to socket */
	int xSaved, ySaved;		/* Saved coordinates */
	int wSaved, hSaved;		/* Saved geometry */
	AG_Color cBackground;		/* Background fill color */
} AG_Icon;

__BEGIN_DECLS
extern AG_WidgetClass agIconClass;

AG_Icon *AG_IconNew(void *, Uint);
AG_Icon *AG_IconFromSurface(AG_Surface *);
AG_Icon *AG_IconFromBMP(const char *);
void	 AG_IconSetSurface(AG_Icon *, AG_Surface *);
void	 AG_IconSetSurfaceNODUP(AG_Icon *, AG_Surface *);
void	 AG_IconSetText(AG_Icon *, const char *, ...)
	     FORMAT_ATTRIBUTE(printf, 2, 3)
	     NONNULL_ATTRIBUTE(2);
void	 AG_IconSetTextS(AG_Icon *, const char *);
void	 AG_IconSetBackgroundFill(AG_Icon *, int, AG_Color);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_ICON_H_ */

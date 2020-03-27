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
	struct ag_widget wid;		/* AG_Widget -> AG_Icon */
	Uint flags;
#define AG_ICON_REGEN_LABEL	0x01	/* Update text label */
#define AG_ICON_DND		0x02	/* Drag-and-drop in progress */
#define AG_ICON_DBLCLICKED	0x04	/* Double click flag */
#define AG_ICON_BGFILL		0x08	/* Enable background fill */
	int surface;			/* Icon surface */
	char labelTxt[AG_LABEL_MAX];	/* Optional text label */
	int  labelSurface;		/* Optional label surface */
	Uint32 _pad1;
	struct ag_window *_Nullable wDND; /* Drag/drop window object */
	struct ag_socket *_Nullable sock; /* Back pointer to socket */
	int xSaved, ySaved;		/* Saved coordinates */
	int wSaved, hSaved;		/* Saved geometry */
	AG_Color cBackground;		/* Background fill color */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad2;
#endif
	AG_Timer toDblClick;		/* For double click detection */
} AG_Icon;

#define AGICON(obj)            ((AG_Icon *)(obj))
#define AGCICON(obj)           ((const AG_Icon *)(obj))
#define AG_ICON_SELF()          AGICON( AG_OBJECT(0,"AG_Widget:AG_Icon:*") )
#define AG_ICON_PTR(n)          AGICON( AG_OBJECT((n),"AG_Widget:AG_Icon:*") )
#define AG_ICON_NAMED(n)        AGICON( AG_OBJECT_NAMED((n),"AG_Widget:AG_Icon:*") )
#define AG_CONST_ICON_SELF()   AGCICON( AG_CONST_OBJECT(0,"AG_Widget:AG_Icon:*") )
#define AG_CONST_ICON_PTR(n)   AGCICON( AG_CONST_OBJECT((n),"AG_Widget:AG_Icon:*") )
#define AG_CONST_ICON_NAMED(n) AGCICON( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Icon:*") )

__BEGIN_DECLS
extern AG_WidgetClass agIconClass;

AG_Icon *_Nonnull  AG_IconNew(void *_Nullable, Uint);
AG_Icon *_Nonnull  AG_IconFromSurface(AG_Surface *_Nonnull);
AG_Icon *_Nullable AG_IconFromBMP(const char *_Nonnull);

void AG_IconSetSurface(AG_Icon *_Nonnull, const AG_Surface *_Nullable);
void AG_IconSetSurfaceNODUP(AG_Icon *_Nonnull, AG_Surface *_Nonnull);
void AG_IconSetTextS(AG_Icon *_Nonnull, const char *_Nullable);
void AG_IconSetText(AG_Icon *_Nonnull, const char *_Nonnull, ...)
                   FORMAT_ATTRIBUTE(printf,2,3);
void AG_IconSetBackgroundFill(AG_Icon *_Nonnull, int, const AG_Color *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_ICON_H_ */

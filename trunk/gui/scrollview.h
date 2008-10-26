/*	Public domain	*/

#ifndef _AGAR_GUI_SCROLLVIEW_H_
#define _AGAR_GUI_SCROLLVIEW_H_

#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>

#include <agar/gui/begin.h>

typedef struct ag_scrollview {
	struct ag_widget wid;

	Uint flags;
#define AG_SCROLLVIEW_HFILL	0x01
#define AG_SCROLLVIEW_VFILL	0x02
#define AG_SCROLLVIEW_NOPAN_X	0x04 /* X is not pannable */ 
#define AG_SCROLLVIEW_NOPAN_Y	0x08 /* Y is not pannable */
#define AG_SCROLLVIEW_PANNING	0x10 /* Panning in progress */
#define AG_SCROLLVIEW_BY_MOUSE	0x20 /* Panning with mouse allowed */
#define AG_SCROLLVIEW_FRAME	0x80 /* Draw background and frame */
#define AG_SCROLLVIEW_EXPAND	(AG_SCROLLVIEW_HFILL|AG_SCROLLVIEW_VFILL)
#define AG_SCROLLVIEW_NOPAN_XY	(AG_SCROLLVIEW_NOPAN_X|AG_SCROLLVIEW_NOPAN_Y)

	enum ag_widget_packing pack;	/* Packing mode */
	int wPre, hPre;			/* Requested geometry */
	int xOffs, yOffs;		/* Display offset */
	int xMin, xMax, yMin, yMax;	/* Display boundaries */
	AG_Rect r;			/* Available space for widgets */
	AG_Scrollbar *hbar, *vbar;	/* Scrollbars for panning */
	int wBar, hBar;			/* Effective scrollbar sizes */
	int incr;			/* Scrolling increment */
} AG_Scrollview;

__BEGIN_DECLS
extern AG_WidgetClass agScrollviewClass;

AG_Scrollview *AG_ScrollviewNew(void *, Uint);
void           AG_ScrollviewSizeHint(AG_Scrollview *, Uint, Uint);
void           AG_ScrollviewSetIncrement(AG_Scrollview *, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_SCROLLVIEW_H_ */

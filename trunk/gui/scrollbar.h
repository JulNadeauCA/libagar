/*	Public domain	*/

#ifndef _AGAR_WIDGET_SCROLLBAR_H_
#define _AGAR_WIDGET_SCROLLBAR_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

enum ag_scrollbar_type {
	AG_SCROLLBAR_HORIZ,
	AG_SCROLLBAR_VERT
};

typedef struct ag_scrollbar {
	struct ag_widget wid;
	Uint flags;
#define AG_SCROLLBAR_HFILL	0x01
#define AG_SCROLLBAR_VFILL	0x02
#define AG_SCROLLBAR_FOCUSABLE	0x04
#define AG_SCROLLBAR_EXPAND	(AG_SCROLLBAR_HFILL|AG_SCROLLBAR_VFILL)
	int value;			/* Default value binding */
	int min, max;			/* Default range binding */
	int visible;			/* Number of visible items */
	enum ag_scrollbar_type type;
	int bw;				/* Scroll button size */
	int curbutton;			/* Button held */
	int barSz;			/* Scroll bar size */
	int arrowSz;			/* Arrow height */
} AG_Scrollbar;

#define AGSCROLLBAR(p) ((AG_Scrollbar *)p)

__BEGIN_DECLS
extern AG_WidgetClass agScrollbarClass;
AG_Scrollbar *AG_ScrollbarNew(void *, enum ag_scrollbar_type, Uint);

static __inline__ void
AG_ScrollbarSetBarSize(AG_Scrollbar *sb, int bsize)
{
	sb->barSz = (bsize > 10 || bsize == -1) ? bsize : 10;
}
static __inline__ void
AG_ScrollbarGetBarSize(AG_Scrollbar *sb, int *bsize)
{
	*bsize = sb->barSz;
}

static __inline__ int
AG_ScrollbarVisible(AG_Scrollbar *sb)
{
	int min, max;

	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		if (AGWIDGET(sb)->w < sb->bw ||
		    AGWIDGET(sb)->h < sb->bw*2 + sb->barSz) {
			return (0);
		}
		break;
	case AG_SCROLLBAR_HORIZ:
		if (AGWIDGET(sb)->w < sb->bw*2 + sb->barSz ||
		    AGWIDGET(sb)->h < sb->bw) {
			return (0);
		}
		break;
	}
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	return (max > 0 && max >= min);
}
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_SCROLLBAR_H_ */

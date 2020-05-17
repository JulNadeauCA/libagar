/*	Public domain	*/

#ifndef _AGAR_WIDGET_MPANE_H_
#define _AGAR_WIDGET_MPANE_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>

#include <agar/gui/begin.h>

enum ag_mpane_layout {
	AG_MPANE1,		/* Single view */
	AG_MPANE2V,		/* Dual views (left/right) */
	AG_MPANE2H,		/* Dual views (top/bottom) */
	AG_MPANE2L1R,		/* Two views left, one view right */
	AG_MPANE1L2R,		/* One view left, two views right */
	AG_MPANE2T1B,		/* Two views top, one view bottom */
	AG_MPANE1T2B,		/* One view top, two views bottom */
	AG_MPANE3L1R,		/* Three views left, one view right */
	AG_MPANE1L3R,		/* One view left, three views right */
	AG_MPANE3T1B,		/* Three views top, one view bottom */
	AG_MPANE1T3B,		/* One view top, three views bottom */
	AG_MPANE4		/* Four views */
};

typedef struct ag_mpane {
	struct ag_box box;
	enum ag_mpane_layout layout;
	Uint flags;
#define AG_MPANE_HFILL		0x01	/* Expand to fill available width */
#define AG_MPANE_VFILL		0x02	/* Expand to fill available height */
#define AG_MPANE_FRAMES		0x04	/* Draw pane backgrounds */
#define AG_MPANE_FORCE_DIV	0x08	/* Always divide in two */
#define AG_MPANE_EXPAND (AG_MPANE_HFILL|AG_MPANE_VFILL)
	struct ag_box *_Nonnull panes[4];
	Uint                   nPanes;
	Uint32 _pad;
} AG_MPane;

__BEGIN_DECLS
extern AG_WidgetClass agMPaneClass;

AG_MPane *_Nonnull AG_MPaneNew(void *_Nullable, enum ag_mpane_layout, Uint);
void               AG_MPaneSetLayout(AG_MPane *_Nonnull, enum ag_mpane_layout);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_MPANE_H_ */

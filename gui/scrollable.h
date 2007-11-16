/*	Public domain	*/

#ifndef _AGAR_GUI_SCROLLABLE_H_
#define _AGAR_GUI_SCROLLABLE_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/scrollbar.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/scrollbar.h>
#endif

#include "begin_code.h"

typedef struct ag_scrollable {
	struct ag_widget wParent;
	Uint flags;
#define AG_SCROLLABLE_HFILL	0x01
#define AG_SCROLLABLE_VFILL	0x02
#define AG_SCROLLABLE_EXPAND	(AG_SCROLLABLE_HFILL|AG_SCROLLABLE_VFILL)

	AG_Scrollbar *vbar;		/* Vertical scrollbar */
	AG_Scrollbar *hbar;		/* Horizontal scrollbar */
	int w, h;			/* Display area dimensions */
	int xOffs, yOffs;		/* Display offset */
	struct {
		int x, y;
		int cx, cy;
	} save;
} AG_Scrollable;

#define AGSCROLLABLE(p) ((AG_Scrollable *)p)

__BEGIN_DECLS
extern const AG_WidgetClass agScrollableClass;

void AG_ScrollableDrawBegin(AG_Scrollable *);
void AG_ScrollableDrawEnd(AG_Scrollable *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_GUI_SCROLLABLE_H_ */

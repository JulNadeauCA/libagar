/*	Public domain	*/

#ifndef _AGAR_WIDGET_PANE_H_
#define _AGAR_WIDGET_PANE_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/box.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#endif

#include "begin_code.h"

enum ag_pane_type {
	AG_PANE_HORIZ,
	AG_PANE_VERT
};

typedef struct ag_pane {
	struct ag_widget wid;
	enum ag_pane_type type;
	int flags;
#define AG_PANE_HFILL		0x001
#define AG_PANE_VFILL		0x002
#define AG_PANE_DIV1FILL	0x004	/* Expand div1 (default is div2) */
#define AG_PANE_FRAME		0x008	/* Display frames for each division */
#define AG_PANE_FORCE_DIV1FILL	0x010	/* Enforce div1 expansion */
#define AG_PANE_FORCE_DIV2FILL	0x020	/* Enforce div2 expansion */
#define AG_PANE_DIV		0x040	/* Initially divide area in two */
#define AG_PANE_FORCE_DIV	0x080	/* Force divide area in two */
#define AG_PANE_INITSCALE	0x100	/* Used internally */
#define AG_PANE_EXPAND (AG_PANE_HFILL|AG_PANE_VFILL)

	AG_Box *div[2];			/* Division containers */
	int	minw[2], minh[2];	/* Minimum geometry */
	int	dmoving;		/* Divider being moved */
	int	dx;			/* Actual divider position */
	int	rx;			/* Requested divider position */
	int	wDiv;			/* Divider width */
} AG_Pane;

__BEGIN_DECLS
extern AG_WidgetClass agPaneClass;

AG_Pane	*AG_PaneNew(void *, enum ag_pane_type, Uint);
#define	 AG_PaneNewHoriz(p,f) AG_PaneNew((p),AG_PANE_HORIZ,(f))
#define	 AG_PaneNewVert(p,f) AG_PaneNew((p),AG_PANE_VERT,(f))
void	 AG_PaneAttachBox(AG_Pane *, int, AG_Box *);
void	 AG_PaneAttachBoxes(AG_Pane *, AG_Box *, AG_Box *);
void	 AG_PaneSetDividerWidth(AG_Pane *, int);
void	 AG_PaneSetDivisionMin(AG_Pane *, int, int, int);
void	 AG_PaneSetDivisionPacking(AG_Pane *, int, enum ag_box_type);
int	 AG_PaneMoveDivider(AG_Pane *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PANE_H_ */

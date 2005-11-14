/*	$Csoft: hpane.h,v 1.1 2005/06/10 02:02:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PANE_H_
#define _AGAR_WIDGET_PANE_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>

#include "begin_code.h"

enum ag_pane_type {
	AG_PANE_HORIZ,
	AG_PANE_VERT
};

typedef struct ag_pane {
	struct ag_box box;
	enum ag_pane_type type;
	int flags;
#define AG_PANE_HFILL		0x01
#define AG_PANE_VFILL		0x02
#define AG_PANE_DIV1_FILL	0x04
#define AG_PANE_DIV2_FILL	0x08
#define AG_PANE_EXPAND (AG_PANE_HFILL|AG_PANE_VFILL)

	AG_Box *div[2];			/* Division containers */
	int	minw[2], minh[2];	/* Minimum geometry */
	int	dmoving;		/* Divider being moved */
	int	dx;			/* Actual divider position */
	int	rx;			/* Requested divider position */
	int	dw;			/* Divider width */
} AG_Pane;

__BEGIN_DECLS
AG_Pane		*AG_PaneNew(void *, enum ag_pane_type, Uint);
void		 AG_PaneInit(AG_Pane *, enum ag_pane_type, Uint);
void		 AG_PaneDraw(void *);
void		 AG_PaneScale(void *, int, int);
void		 AG_PaneAttachBoxes(AG_Pane *, AG_Box *, AG_Box *);
__inline__ void	 AG_PaneSetDividerWidth(AG_Pane *, int);
__inline__ void	 AG_PaneSetDivisionMin(AG_Pane *, int, int, int);
int		 AG_PaneMoveDivider(AG_Pane *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PANE_H_ */

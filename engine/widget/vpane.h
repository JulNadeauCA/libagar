/*	$Csoft: vpane.h,v 1.1 2005/08/21 07:09:39 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_VPANE_H_
#define _AGAR_WIDGET_VPANE_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>

#include "begin_code.h"

typedef struct ag_vpane_div {
	int moving;			/* Divider is being repositioned */
	int y;				/* Divider position */
	AG_Box *box1, *box2;		/* Coincident widgets */
	TAILQ_ENTRY(ag_vpane_div) divs;
} AG_VPaneDiv;

typedef struct ag_vpane {
	struct ag_box box;
	int flags;
#define AG_VPANE_WFILL	0x01		/* Expand to fill available width */
#define AG_VPANE_HFILL	0x02		/* Expand to fill available height */
	TAILQ_HEAD(, ag_vpane_div) divs;
} AG_VPane;

__BEGIN_DECLS
AG_VPane *AG_VPaneNew(void *, int);
void AG_VPaneInit(AG_VPane *, int);
void AG_VPaneDraw(void *);
void AG_VPaneScale(void *, int, int);
AG_VPaneDiv *AG_VPaneAddDiv(AG_VPane *, enum ag_box_type, int,
		            enum ag_box_type, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_VPANE_H_ */

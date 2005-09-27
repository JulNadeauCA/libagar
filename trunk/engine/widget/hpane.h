/*	$Csoft: hpane.h,v 1.1 2005/06/10 02:02:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_HPANE_H_
#define _AGAR_WIDGET_HPANE_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>

#include "begin_code.h"

typedef struct ag_hpane_div {
	int moving;			/* Divider is being repositioned */
	int x;				/* Divider position */
	AG_Box *box1, *box2;		/* Coincident widgets */
	TAILQ_ENTRY(ag_hpane_div) divs;
} AG_HPaneDiv;

typedef struct ag_hpane {
	struct ag_box box;
	int flags;
#define AG_HPANE_WFILL	0x01		/* Expand to fill available width */
#define AG_HPANE_HFILL	0x02		/* Expand to fill available height */
	TAILQ_HEAD(, ag_hpane_div) divs;
} AG_HPane;

__BEGIN_DECLS
AG_HPane	*AG_HPaneNew(void *, int);
void		 AG_HPaneInit(AG_HPane *, int);
void		 AG_HPaneDraw(void *);
void		 AG_HPaneScale(void *, int, int);
AG_HPaneDiv	*AG_HPaneAddDiv(AG_HPane *, enum ag_box_type, int,
		                enum ag_box_type, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_HPANE_H_ */

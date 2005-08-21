/*	$Csoft: vpane.h,v 1.1 2005/06/10 02:02:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_VPANE_H_
#define _AGAR_WIDGET_VPANE_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>

#include "begin_code.h"

struct vpane_div {
	int moving;			/* Divider is being repositioned */
	int y;				/* Divider position */
	struct box *box1, *box2;	/* Coincident widgets */
	TAILQ_ENTRY(vpane_div) divs;
};

struct vpane {
	struct box box;			/* Ancestor */
	int flags;
#define VPANE_WFILL	0x01		/* Expand to fill available width */
#define VPANE_HFILL	0x02		/* Expand to fill available height */
	TAILQ_HEAD(, vpane_div) divs;
};

__BEGIN_DECLS
struct vpane *vpane_new(void *, int);
void vpane_init(struct vpane *, int);
void vpane_destroy(void *);
void vpane_draw(void *);
void vpane_scale(void *, int, int);
struct vpane_div *vpane_add_div(struct vpane *, enum box_type, int,
		                enum box_type, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_VPANE_H_ */

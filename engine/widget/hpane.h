/*	$Csoft: hpane.h,v 1.2 2005/03/09 06:39:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_HPANE_H_
#define _AGAR_WIDGET_HPANE_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>

#include "begin_code.h"

struct hpane_div {
	int moving;			/* Divider is being repositioned */
	int x;				/* Divider position */
	struct box *box1, *box2;	/* Coincident widgets */
	TAILQ_ENTRY(hpane_div) divs;
};

struct hpane {
	struct box box;			/* Ancestor */
	int flags;
#define HPANE_WFILL	0x01		/* Expand to fill available width */
#define HPANE_HFILL	0x02		/* Expand to fill available height */
	TAILQ_HEAD(, hpane_div) divs;
};

__BEGIN_DECLS
struct hpane *hpane_new(void *, int);
void hpane_init(struct hpane *, int);
void hpane_destroy(void *);
void hpane_draw(void *);
void hpane_scale(void *, int, int);
struct hpane_div *hpane_add_div(struct hpane *, enum box_type, int,
		                enum box_type, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_HPANE_H_ */

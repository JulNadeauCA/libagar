/*	$Csoft: hbox.h,v 1.1 2003/06/06 02:59:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_HBOX_H_
#define _AGAR_WIDGET_HBOX_H_

#include <engine/widget/box.h>

struct hbox {
	struct box box;
};

#define HBOX_HOMOGENOUS	BOX_HOMOGENOUS
#define HBOX_WFILL	BOX_WFILL
#define HBOX_HFILL	BOX_HFILL

#define hbox_new(p, fl)	(struct hbox *)box_new((p), BOX_HORIZ, (fl))
#define hbox_init(b, fl) box_init((struct box *)(b), BOX_HORIZ, (fl))
#define hbox_set_homogenous(b, fl) box_set_homogenous((struct box *)(b), (fl))
#define hbox_set_padding(b, pad) box_set_padding((struct box *)(b), (pad))
#define hbox_set_spacing(b, sp) box_set_spacing((struct box *)(b), (sp))

#endif /* _AGAR_WIDGET_HBOX_H_ */

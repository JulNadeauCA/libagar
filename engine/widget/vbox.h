/*	$Csoft: vbox.h,v 1.1 2003/06/06 02:59:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_VBOX_H_
#define _AGAR_WIDGET_VBOX_H_

#include <engine/widget/box.h>

struct vbox {
	struct box box;
};

#define VBOX_HOMOGENOUS	BOX_HOMOGENOUS
#define VBOX_WFILL	BOX_WFILL
#define VBOX_HFILL	BOX_HFILL

#define vbox_new(p, fl)	(struct vbox *)box_new((p), BOX_VERT, (fl))
#define vbox_init(b, fl) box_init((struct box *)(b), BOX_VERT, (fl))
#define vbox_set_homogenous(b, fl) box_set_homogenous((struct box *)(b), (fl))
#define vbox_set_padding(b, pad) box_set_padding((struct box *)(b), (pad))
#define vbox_set_spacing(b, sp) box_set_spacing((struct box *)(b), (sp))

#endif /* _AGAR_WIDGET_VBOX_H_ */

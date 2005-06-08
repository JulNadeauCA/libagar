/*	$Csoft: vbox.h,v 1.2 2003/06/10 06:58:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_VBOX_H_
#define _AGAR_WIDGET_VBOX_H_

#include <engine/widget/box.h>

typedef struct ag_vbox {
	struct ag_box box;
} AG_VBox;

#define AG_VBOX_HOMOGENOUS	AG_BOX_HOMOGENOUS
#define AG_VBOX_WFILL		AG_BOX_WFILL
#define AG_VBOX_HFILL		AG_BOX_HFILL

#define AG_VBoxNew(p, fl) (AG_VBox *)AG_BoxNew((p), AG_BOX_VERT, (fl))
#define AG_VBoxInit(b, fl) AG_BoxInit((AG_Box *)(b), AG_BOX_VERT, (fl))
#define AG_VBoxSetHomogenous(b, fl) AG_BoxSetHomogenous((AG_Box *)(b), (fl))
#define AG_VBoxSetPadding(b, pad) AG_BoxSetPadding((AG_Box *)(b), (pad))
#define AG_VBoxSetSpacing(b, sp) AG_BoxSetSpacing((AG_Box *)(b), (sp))

#endif /* _AGAR_WIDGET_VBOX_H_ */

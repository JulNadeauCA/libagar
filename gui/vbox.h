/*	Public domain	*/

#ifndef _AGAR_WIDGET_VBOX_H_
#define _AGAR_WIDGET_VBOX_H_

#include <agar/gui/box.h>

#include <agar/gui/begin.h>

typedef struct ag_vbox {
	struct ag_box box;
} AG_VBox;

#define AG_VBOX_HOMOGENOUS	AG_BOX_HOMOGENOUS
#define AG_VBOX_HFILL		AG_BOX_HFILL
#define AG_VBOX_VFILL		AG_BOX_VFILL
#define AG_VBOX_EXPAND		(AG_BOX_HFILL|AG_BOX_VFILL)

#define AG_VBoxNew(p, fl) (AG_VBox *)AG_BoxNew((p), AG_BOX_VERT, (fl))
#define AG_VBoxInit(b, fl) AG_BoxInit((AG_Box *)(b), AG_BOX_VERT, (fl))
#define AG_VBoxSetHomogenous(b, fl) AG_BoxSetHomogenous((AG_Box *)(b), (fl))
#define AG_VBoxSetPadding(b, pad) AG_BoxSetPadding((AG_Box *)(b), (pad))
#define AG_VBoxSetSpacing(b, sp) AG_BoxSetSpacing((AG_Box *)(b), (sp))

#include <agar/gui/close.h>

#endif /* _AGAR_WIDGET_VBOX_H_ */

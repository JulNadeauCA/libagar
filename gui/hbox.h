/*	Public domain	*/

#ifndef _AGAR_WIDGET_HBOX_H_
#define _AGAR_WIDGET_HBOX_H_

#include <agar/gui/box.h>

#include <agar/gui/begin.h>

typedef struct ag_hbox {
	struct ag_box box;
} AG_HBox;

#define AG_HBOX_HOMOGENOUS	AG_BOX_HOMOGENOUS
#define AG_HBOX_HFILL		AG_BOX_HFILL
#define AG_HBOX_VFILL		AG_BOX_VFILL
#define AG_HBOX_EXPAND		(AG_BOX_HFILL|AG_BOX_VFILL)

#define AG_HBoxNew(p, fl) (AG_HBox *)AG_BoxNew((p), AG_BOX_HORIZ, (fl))
#define AG_HBoxInit(b, fl) AG_BoxInit((AG_Box *)(b), AG_BOX_HORIZ, (fl))
#define AG_HBoxSetHomogenous(b, fl) AG_BoxSetHomogenous((AG_Box *)(b), (fl))
#define AG_HBoxSetPadding(b, pad) AG_BoxSetPadding((AG_Box *)(b), (pad))
#define AG_HBoxSetSpacing(b, sp) AG_BoxSetSpacing((AG_Box *)(b), (sp))

#include <agar/gui/close.h>

#endif /* _AGAR_WIDGET_HBOX_H_ */

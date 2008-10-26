/*	Public domain	*/

#ifndef _AGAR_WIDGET_STATUSBAR_H_
#define _AGAR_WIDGET_STATUSBAR_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

#define AG_STATUSBAR_MAX_LABELS 8

typedef struct ag_statusbar {
	struct ag_box box;
	AG_Label *labels[AG_STATUSBAR_MAX_LABELS];
	int nlabels;
} AG_Statusbar;

__BEGIN_DECLS
extern AG_WidgetClass agStatusbarClass;

AG_Statusbar *AG_StatusbarNew(void *, Uint);
AG_Label     *AG_StatusbarAddLabel(AG_Statusbar *, enum ag_label_type,
	                           const char *, ...);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_STATUSBAR_H_ */

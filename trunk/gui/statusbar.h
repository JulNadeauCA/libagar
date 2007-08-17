/*	Public domain	*/

#ifndef _AGAR_WIDGET_STATUSBAR_H_
#define _AGAR_WIDGET_STATUSBAR_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/box.h>
#include <gui/label.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#endif

#include "begin_code.h"

#define AG_STATUSBAR_MAX_LABELS 8

typedef struct ag_statusbar {
	struct ag_box box;
	AG_Label *labels[AG_STATUSBAR_MAX_LABELS];
	int nlabels;
} AG_Statusbar;

__BEGIN_DECLS
extern const AG_WidgetOps agStatusbarOps;

AG_Statusbar *AG_StatusbarNew(void *, Uint);
void	      AG_StatusbarInit(AG_Statusbar *, Uint);
AG_Label     *AG_StatusbarAddLabel(AG_Statusbar *, enum ag_label_type,
	                           const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_STATUSBAR_H_ */

/*	$Csoft: statusbar.h,v 1.1 2004/03/30 15:50:44 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_STATUSBAR_H_
#define _AGAR_WIDGET_STATUSBAR_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>

#include "begin_code.h"

#define AG_STATUSBAR_MAX_LABELS 8

typedef struct ag_statusbar {
	struct ag_box box;
	AG_Label *labels[AG_STATUSBAR_MAX_LABELS];
	int nlabels;
} AG_Statusbar;

__BEGIN_DECLS
AG_Statusbar *AG_StatusbarNew(void *);
void	      AG_StatusbarInit(AG_Statusbar *);
AG_Label     *AG_StatusbarAddLabel(AG_Statusbar *, enum ag_label_type,
	                           const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_STATUSBAR_H_ */

/*	$Csoft: ucombo.h,v 1.3 2004/09/12 05:52:26 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_UCOMBO_H_
#define _AGAR_WIDGET_UCOMBO_H_

#include <engine/widget/widget.h>
#include <engine/widget/button.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>

#include "begin_code.h"

typedef struct ag_ucombo {
	struct ag_widget wid;
	AG_Button *button;		/* Selection button */
	AG_Tlist *list;			/* Item list */
	AG_Window *panel;
	int saved_w, saved_h;		/* Saved panel geometry */
} AG_UCombo;

__BEGIN_DECLS
AG_UCombo *AG_UComboNew(void *);
void	   AG_UComboInit(AG_UCombo *);
void	   AG_UComboScale(void *, int, int);
void	   AG_UComboDestroy(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_UCOMBO_H_ */

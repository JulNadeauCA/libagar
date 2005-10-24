/*	$Csoft: checkbox.h,v 1.17 2005/05/13 09:21:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <agar/gui/widget.h>

#include "begin_code.h"

typedef struct ag_checkbox {
	struct ag_widget wid;
	Uint flags;
#define AG_CHECKBOX_HFILL	0x01
#define AG_CHECKBOX_VFILL	0x02
#define AG_CHECKBOX_FOCUS	0x04
#define AG_CHECKBOX_EXPAND	(AG_CHECKBOX_HFILL|AG_CHECKBOX_VFILL)
	int state;
	SDL_Surface *label_su;
	int label_id;
} AG_Checkbox;

__BEGIN_DECLS
AG_Checkbox	*AG_CheckboxNew(void *, Uint, const char *);
void		 AG_CheckboxInit(AG_Checkbox *, Uint, const char *);
void		 AG_CheckboxScale(void *, int, int);
void		 AG_CheckboxDraw(void *);
void		 AG_CheckboxToggle(AG_Checkbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */

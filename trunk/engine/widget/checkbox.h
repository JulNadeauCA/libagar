/*	$Csoft: checkbox.h,v 1.17 2005/05/13 09:21:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <engine/widget/widget.h>

#include "begin_code.h"

typedef struct ag_checkbox {
	struct ag_widget wid;
	int state;
	SDL_Surface *label_su;
	int label_id;
} AG_Checkbox;

__BEGIN_DECLS
AG_Checkbox	*AG_CheckboxNew(void *, const char *, ...);
void		 AG_CheckboxInit(AG_Checkbox *, char *);
void		 AG_CheckboxScale(void *, int, int);
void		 AG_CheckboxDraw(void *);
void		 AG_CheckboxToggle(AG_Checkbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */

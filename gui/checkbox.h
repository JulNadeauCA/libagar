/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

typedef struct ag_checkbox {
	struct ag_widget wid;
	Uint flags;
#define AG_CHECKBOX_HFILL	0x01
#define AG_CHECKBOX_VFILL	0x02
#define AG_CHECKBOX_FOCUS	0x04
#define AG_CHECKBOX_EXPAND	(AG_CHECKBOX_HFILL|AG_CHECKBOX_VFILL)
	int state;
	char *labelTxt;
	int label;
} AG_Checkbox;

__BEGIN_DECLS
extern const AG_WidgetOps agCheckboxOps;

AG_Checkbox	*AG_CheckboxNew(void *, Uint, const char *);
AG_Checkbox	*AG_CheckboxNewFlag(void *, Uint *, Uint, const char *);
AG_Checkbox	*AG_CheckboxNewFlag32(void *, Uint32 *, Uint32, const char *);
void		 AG_CheckboxSetFromFlags(void *, Uint *, const AG_FlagDescr *);
void		 AG_CheckboxSetFromFlags32(void *, Uint32 *, 
		                           const AG_FlagDescr *);
void		 AG_CheckboxInit(AG_Checkbox *, Uint, const char *);
void		 AG_CheckboxToggle(AG_Checkbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */

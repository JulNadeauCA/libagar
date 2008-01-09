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
#define AG_CHECKBOX_EXPAND	(AG_CHECKBOX_HFILL|AG_CHECKBOX_VFILL)
#define AG_CHECKBOX_SET		0x04
	int state;
	char *labelTxt;
	int label;
	int spacing;
} AG_Checkbox;

__BEGIN_DECLS
extern AG_WidgetClass agCheckboxClass;

AG_Checkbox	*AG_CheckboxNew(void *, Uint, const char *);
AG_Checkbox	*AG_CheckboxNewFn(void *, Uint, const char *, AG_EventFn,
                                  const char *, ...);
AG_Checkbox	*AG_CheckboxNewInt(void *, Uint, int *, const char *);
AG_Checkbox	*AG_CheckboxNewFlag(void *, Uint *, Uint, const char *);
AG_Checkbox	*AG_CheckboxNewFlag32(void *, Uint32 *, Uint32, const char *);
void		 AG_CheckboxSetFromFlags(void *, Uint *, const AG_FlagDescr *);
void		 AG_CheckboxSetFromFlags32(void *, Uint32 *, 
		                           const AG_FlagDescr *);
void		 AG_CheckboxToggle(AG_Checkbox *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */

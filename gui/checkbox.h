/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

typedef struct ag_checkbox {
	struct ag_widget wid;		/* AG_Widget -> AG_Checkbox */
	Uint flags;
#define AG_CHECKBOX_HFILL	0x01
#define AG_CHECKBOX_VFILL	0x02
#define AG_CHECKBOX_EXPAND	(AG_CHECKBOX_HFILL|AG_CHECKBOX_VFILL)
#define AG_CHECKBOX_SET		0x04
	int state;			/* Default "state" binding */
	int spacing;			/* Spacing in pixels */
	AG_Label *_Nullable lbl;	/* Text label */
} AG_Checkbox;

#define AGCHECKBOX(obj)            ((AG_Checkbox *)(obj))
#define AG_CHECKBOX_SELF()         AG_OBJECT(0,"AG_Widget:AG_Checkbox:*")
#define AG_CHECKBOX_PTR(n)         AG_OBJECT((n),"AG_Widget:AG_Checkbox:*")
#define AG_CHECKBOX_NAMED(n)       AG_OBJECT_NAMED((n),"AG_Widget:AG_Checkbox:*")
#define AG_CONST_CHECKBOX_SELF()   AG_CONST_OBJECT(0,"AG_Widget:AG_Checkbox:*")
#define AG_CONST_CHECKBOX_PTR(n)   AG_CONST_OBJECT((n),"AG_Widget:AG_Checkbox:*")
#define AG_CONST_CHECKBOX_NAMED(n) AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Checkbox:*")

__BEGIN_DECLS
extern AG_WidgetClass agCheckboxClass;

AG_Checkbox *_Nonnull AG_CheckboxNew(void *_Nullable, Uint,
                                     const char *_Nullable, ...)
                                    FORMAT_ATTRIBUTE(printf,3,4);

AG_Checkbox *_Nonnull AG_CheckboxNewS(void *_Nullable, Uint, const char *_Nullable);

AG_Checkbox *_Nonnull AG_CheckboxNewFn(void *_Nullable, Uint, const char *_Nullable,
				       _Nonnull AG_EventFn,
				       const char *_Nullable, ...);

AG_Checkbox *_Nonnull AG_CheckboxNewInt(void *_Nullable, Uint, const char *_Nullable,
                                        int *_Nonnull);
#define               AG_CheckboxNewUint(o,f,l,p) AG_CheckboxNewInt((o),(f),(l),(int *)(p))

AG_Checkbox *_Nonnull AG_CheckboxNewFlag(void *_Nullable, Uint, const char *_Nullable,
                                         Uint *_Nonnull, Uint);

void AG_CheckboxSetFromFlags(void *_Nullable, Uint, Uint *_Nonnull,
                             const AG_FlagDescr *_Nonnull);

int  AG_CheckboxGetState(AG_Checkbox *_Nonnull) _Pure_Attribute;
void AG_CheckboxSetState(AG_Checkbox *_Nonnull, int);
void AG_CheckboxToggle(AG_Checkbox *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */

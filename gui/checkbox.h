/*	Public domain	*/

#ifndef _AGAR_WIDGET_CHECKBOX_H_
#define _AGAR_WIDGET_CHECKBOX_H_

#include <agar/gui/widget.h>
#include <agar/gui/label.h>

#include <agar/gui/begin.h>

typedef struct ag_checkbox {
	struct ag_widget wid;
	Uint flags;
#define AG_CHECKBOX_HFILL	0x01
#define AG_CHECKBOX_VFILL	0x02
#define AG_CHECKBOX_EXPAND	(AG_CHECKBOX_HFILL|AG_CHECKBOX_VFILL)
#define AG_CHECKBOX_SET		0x04
	int state;		/* Default "state" binding */
	int spacing;		/* Spacing in pixels */
	AG_Label *_Nullable lbl; /* Text label */
} AG_Checkbox;

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

AG_Checkbox *_Nonnull AG_CheckboxNewFlag(void *_Nullable, Uint, const char *_Nullable,
                                         Uint *_Nonnull, Uint);

AG_Checkbox *_Nonnull AG_CheckboxNewFlag32(void *_Nullable, Uint,
                                           const char *_Nullable,
                                           Uint32 *_Nonnull, Uint32);

void AG_CheckboxSetFromFlags(void *_Nullable, Uint, Uint *_Nonnull,
                             const AG_FlagDescr *_Nonnull);

void AG_CheckboxSetFromFlags32(void *_Nullable, Uint, Uint32 *_Nonnull,
                               const AG_FlagDescr *_Nonnull);

void AG_CheckboxToggle(AG_Checkbox *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_CHECKBOX_H_ */

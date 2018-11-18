/*	Public domain	*/

#ifndef _AGAR_WIDGET_SPINBUTTON_H_
#define _AGAR_WIDGET_SPINBUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/begin.h>

typedef struct ag_spinbutton {
	struct ag_widget wid;
	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	int incr;			/* Increment for buttons */
	int writeable;			/* 0 = read-only */
	char inTxt[64];			/* Input text buffer */
	AG_Textbox *_Nonnull input;	/* Input field */
	AG_Button  *_Nonnull incbu;	/* Increment button */
	AG_Button  *_Nonnull decbu;	/* Decrement button */
} AG_Spinbutton;

__BEGIN_DECLS
extern AG_WidgetClass agSpinbuttonClass;

AG_Spinbutton *_Nonnull AG_SpinbuttonNew(void *_Nullable, Uint,
                                         const char *_Nullable);
#define AG_SPINBUTTON_NOHFILL	0x01
#define AG_SPINBUTTON_VFILL	0x02

void AG_SpinbuttonAddValue(AG_Spinbutton *_Nonnull, int);
void AG_SpinbuttonSetValue(AG_Spinbutton *_Nonnull, ...);
void AG_SpinbuttonSetMin(AG_Spinbutton *_Nonnull, int);
void AG_SpinbuttonSetMax(AG_Spinbutton *_Nonnull, int);
void AG_SpinbuttonSetRange(AG_Spinbutton *_Nonnull, int,int);
void AG_SpinbuttonSetIncrement(AG_Spinbutton *_Nonnull, int);
void AG_SpinbuttonSetWriteable(AG_Spinbutton *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_SPINBUTTON_H_ */

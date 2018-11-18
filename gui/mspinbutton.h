/*	Public domain	*/

#ifndef _AGAR_WIDGET_MSPINBUTTON_H_
#define _AGAR_WIDGET_MSPINBUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>

#include <agar/gui/begin.h>

#define AG_MSPINBUTTON_NOHFILL	0x01
#define AG_MSPINBUTTON_VFILL	0x02

typedef struct ag_mspinbutton {
	struct ag_widget wid;
	const char *_Nonnull sep;		/* X,Y value separator */
	int xvalue, yvalue;			/* Default X,Y bindings */
	int min, max;				/* Default range bindings */
	int inc;				/* Increment for buttons */
	int writeable;				/* 0 = read-only */
	char inTxt[64];				/* Input text buffer */
	AG_Textbox *_Nonnull input;		/* Input field */
	AG_Button *_Nonnull xincbu;		/* X-increment button */
	AG_Button *_Nonnull xdecbu;		/* X-decrement button */
	AG_Button *_Nonnull yincbu;		/* Y-increment button */
	AG_Button *_Nonnull ydecbu;		/* Y-decrement button */
} AG_MSpinbutton;

__BEGIN_DECLS
extern AG_WidgetClass agMSpinbuttonClass;

AG_MSpinbutton *_Nonnull AG_MSpinbuttonNew(void *_Nullable, Uint,
                                           const char *_Nonnull,
					   const char *_Nullable);

void AG_MSpinbuttonAddValue(AG_MSpinbutton *_Nonnull, const char *_Nonnull, int);
void AG_MSpinbuttonSetValue(AG_MSpinbutton *_Nonnull, const char *_Nonnull, ...);
void AG_MSpinbuttonSetMin(AG_MSpinbutton *_Nonnull, int);
void AG_MSpinbuttonSetMax(AG_MSpinbutton *_Nonnull, int);
void AG_MSpinbuttonSetRange(AG_MSpinbutton *_Nonnull, int,int);
void AG_MSpinbuttonSetIncrement(AG_MSpinbutton *_Nonnull, int);
void AG_MSpinbuttonSetWriteable(AG_MSpinbutton *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_MSPINBUTTON_H_ */

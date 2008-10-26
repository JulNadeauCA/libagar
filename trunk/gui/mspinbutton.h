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
	const char *sep;			/* x/y value separator */
	int xvalue, yvalue;			/* Default x/y bindings */
	int min, max;				/* Default range bindings */
	int inc;				/* Increment for buttons */
	int writeable;				/* 0 = read-only */
	AG_Textbox *input;
	AG_Button *xincbu, *xdecbu;
	AG_Button *yincbu, *ydecbu;
} AG_MSpinbutton;

__BEGIN_DECLS
extern AG_WidgetClass agMSpinbuttonClass;

AG_MSpinbutton *AG_MSpinbuttonNew(void *, Uint, const char *, const char *);

void	AG_MSpinbuttonAddValue(AG_MSpinbutton *, const char *, int);
void	AG_MSpinbuttonSetValue(AG_MSpinbutton *, const char *, ...);
void	AG_MSpinbuttonSetMin(AG_MSpinbutton *, int);
void	AG_MSpinbuttonSetMax(AG_MSpinbutton *, int);
void	AG_MSpinbuttonSetRange(AG_MSpinbutton *, int, int);
void	AG_MSpinbuttonSetIncrement(AG_MSpinbutton *, int);
void	AG_MSpinbuttonSetWriteable(AG_MSpinbutton *, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_MSPINBUTTON_H_ */

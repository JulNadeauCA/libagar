/*	Public domain	*/

#ifndef _AGAR_WIDGET_FSPINBUTTON_H_
#define _AGAR_WIDGET_FSPINBUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/units.h>

#include <agar/gui/begin.h>

#define AG_FSPINBUTTON_NOHFILL	0x01
#define AG_FSPINBUTTON_VFILL	0x02

typedef struct ag_fspinbutton {
	struct ag_widget wid;

	double value;			/* Default value binding */
	double min, max;		/* Default range bindings */
	double inc;			/* Increment for buttons */
	char format[32];		/* Printing format */
	const AG_Unit *_Nonnull unit;	/* Active conversion unit */
	int writeable;			/* 0 = read-only */
	char inTxt[64];			/* Input text buffer */

	AG_Textbox *_Nonnull  input;	/* Input text box */
	AG_UCombo  *_Nullable units;	/* Unit selector */
	AG_Button  *_Nonnull  incbu;	/* Increment (+) button */
	AG_Button  *_Nonnull  decbu;	/* Decrement (-) button */
} AG_FSpinbutton;

__BEGIN_DECLS
extern AG_WidgetClass agFSpinbuttonClass;

AG_FSpinbutton *_Nonnull AG_FSpinbuttonNew(void *_Nullable, Uint,
                                           const char *_Nullable,
					   const char *_Nullable);

void    AG_FSpinbuttonSizeHint(AG_FSpinbutton *_Nonnull, const char *_Nonnull);
#define AG_FSpinbuttonPrescale AG_FSpinbuttonSizeHint

void	AG_FSpinbuttonSetValue(AG_FSpinbutton *_Nonnull, double);
void	AG_FSpinbuttonAddValue(AG_FSpinbutton *_Nonnull, double);
void	AG_FSpinbuttonSetMin(AG_FSpinbutton *_Nonnull, double);
void	AG_FSpinbuttonSetMax(AG_FSpinbutton *_Nonnull, double);
void	AG_FSpinbuttonSetRange(AG_FSpinbutton *_Nonnull, double,double);
void	AG_FSpinbuttonSetIncrement(AG_FSpinbutton *_Nonnull, double);
void	AG_FSpinbuttonSelectUnit(AG_FSpinbutton *_Nonnull, const char *_Nonnull);
void	AG_FSpinbuttonSetPrecision(AG_FSpinbutton *_Nonnull,
                                   const char *_Nonnull, int);
void	AG_FSpinbuttonSetWriteable(AG_FSpinbutton *_Nonnull, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FSPINBUTTON_H_ */

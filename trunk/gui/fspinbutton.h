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
	const AG_Unit *unit;		/* Conversion unit in use */
	int writeable;			/* 0 = read-only */
	AG_Textbox *input;
	AG_UCombo *units;
	AG_Button *incbu;
	AG_Button *decbu;
} AG_FSpinbutton;

__BEGIN_DECLS
extern AG_WidgetClass agFSpinbuttonClass;

AG_FSpinbutton *AG_FSpinbuttonNew(void *, Uint, const char *, const char *);
void		AG_FSpinbuttonSizeHint(AG_FSpinbutton *, const char *);
#define		AG_FSpinbuttonPrescale AG_FSpinbuttonSizeHint

void	AG_FSpinbuttonSetValue(AG_FSpinbutton *, double);
void	AG_FSpinbuttonAddValue(AG_FSpinbutton *, double);
void	AG_FSpinbuttonSetMin(AG_FSpinbutton *, double);
void	AG_FSpinbuttonSetMax(AG_FSpinbutton *, double);
void	AG_FSpinbuttonSetRange(AG_FSpinbutton *, double, double);
void	AG_FSpinbuttonSetIncrement(AG_FSpinbutton *, double);
void	AG_FSpinbuttonSelectUnit(AG_FSpinbutton *, const char *);
void	AG_FSpinbuttonSetPrecision(AG_FSpinbutton *, const char *, int);
void	AG_FSpinbuttonSetWriteable(AG_FSpinbutton *, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FSPINBUTTON_H_ */

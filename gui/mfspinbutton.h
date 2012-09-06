/*	Public domain	*/

#ifndef _AGAR_WIDGET_MFSPINBUTTON_H_
#define _AGAR_WIDGET_MFSPINBUTTON_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/units.h>

#include <agar/gui/begin.h>

typedef struct ag_mfspinbutton {
	struct ag_widget wid;
	Uint flags;
#define AG_MFSPINBUTTON_NOHFILL	0x01
#define AG_MFSPINBUTTON_VFILL	0x02
#define AG_MFSPINBUTTON_EXCL	0x04	/* Exclusive binding access */
	double xvalue, yvalue;		/* Default value bindings */
	double min, max;		/* Default range bindings */
	float minFlt, maxFlt;
	double inc;			/* Increment for buttons */
	char format[32];		/* Printing format */
	const char *sep;		/* x/y field separator */
	const AG_Unit *unit;		/* Conversion unit */
	int writeable;			/* 0 = read-only */
	char inTxt[128];		/* Input text buffer */
	AG_Textbox *input;
	AG_UCombo *units;
	AG_Button *xincbu, *xdecbu;
	AG_Button *yincbu, *ydecbu;
	AG_Timer updateTo;
} AG_MFSpinbutton;

__BEGIN_DECLS
extern AG_WidgetClass agMFSpinbuttonClass;

AG_MFSpinbutton	*AG_MFSpinbuttonNew(void *, Uint, const char *, const char *,
		                    const char *);
void    AG_MFSpinbuttonUpdate(AG_MFSpinbutton *);
void	AG_MFSpinbuttonSetValue(AG_MFSpinbutton *, const char *, double);
void	AG_MFSpinbuttonAddValue(AG_MFSpinbutton *, const char *, double);
void	AG_MFSpinbuttonSetMin(AG_MFSpinbutton *, double);
void	AG_MFSpinbuttonSetMax(AG_MFSpinbutton *, double);
void	AG_MFSpinbuttonSetRange(AG_MFSpinbutton *, double, double);
void	AG_MFSpinbuttonSetIncrement(AG_MFSpinbutton *, double);
void	AG_MFSpinbuttonSelectUnit(AG_MFSpinbutton *, const char *);
void	AG_MFSpinbuttonSetPrecision(AG_MFSpinbutton *, const char *, int);
void	AG_MFSpinbuttonSetWriteable(AG_MFSpinbutton *, int);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_MFSPINBUTTON_H_ */

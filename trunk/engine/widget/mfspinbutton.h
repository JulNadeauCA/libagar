/*	$Csoft: mfspinbutton.h,v 1.2 2004/08/22 12:08:16 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MFSPINBUTTON_H_
#define _AGAR_WIDGET_MFSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/ucombo.h>
#include <engine/widget/units.h>

#include "begin_code.h"

typedef struct ag_mfspinbutton {
	struct ag_widget wid;
	pthread_mutex_t	lock;
	double xvalue, yvalue;		/* Default x/y value bindings */
	double min, max;		/* Default range bindings */
	double inc;			/* Increment for buttons */
	char format[32];		/* Printing format */
	const char *sep;		/* x/y field separator */
	const AG_Unit *unit;		/* Conversion unit */
	int writeable;			/* 0 = read-only */
	AG_Textbox *input;
	AG_UCombo *units;
	AG_Button *xincbu, *xdecbu;
	AG_Button *yincbu, *ydecbu;
} AG_MFSpinbutton;

__BEGIN_DECLS
AG_MFSpinbutton	*AG_MFSpinbuttonNew(void *, const char *, const char *,
		                    const char *, ...)
				    FORMAT_ATTRIBUTE(printf, 4, 5)
				    NONNULL_ATTRIBUTE(4);

void	AG_MFSpinbuttonInit(AG_MFSpinbutton *, const char *, const char *,		                    const char *);
void	AG_MFSpinbuttonDestroy(void *);
void	AG_MFSpinbuttonScale(void *, int, int);
void	AG_MFSpinbuttonDraw(void *);

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

#include "close_code.h"
#endif /* _AGAR_WIDGET_MFSPINBUTTON_H_ */

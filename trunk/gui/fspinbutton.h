/*	$Csoft: fspinbutton.h,v 1.12 2005/02/18 11:16:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_FSPINBUTTON_H_
#define _AGAR_WIDGET_FSPINBUTTON_H_

#include <engine/widget/widget.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/ucombo.h>
#include <engine/widget/units.h>

#include "begin_code.h"

typedef struct ag_fspinbutton {
	struct ag_widget wid;

	pthread_mutex_t	lock;
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
AG_FSpinbutton *AG_FSpinbuttonNew(void *, const char *, const char *, ...)
		                  FORMAT_ATTRIBUTE(printf, 3, 4)
		                  NONNULL_ATTRIBUTE(3);

void	AG_FSpinbuttonInit(AG_FSpinbutton *, const char *, const char *);
void	AG_FSpinbuttonDestroy(void *);
void	AG_FSpinbuttonPrescale(AG_FSpinbutton *, const char *);
void	AG_FSpinbuttonScale(void *, int, int);
void	AG_FSpinbuttonDraw(void *);

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

#include "close_code.h"
#endif /* _AGAR_WIDGET_FSPINBUTTON_H_ */

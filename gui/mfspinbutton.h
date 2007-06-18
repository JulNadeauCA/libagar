/*	Public domain	*/

#ifndef _AGAR_WIDGET_MFSPINBUTTON_H_
#define _AGAR_WIDGET_MFSPINBUTTON_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/textbox.h>
#include <gui/button.h>
#include <gui/ucombo.h>
#include <gui/units.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/units.h>
#endif

#include "begin_code.h"

#define AG_MFSPINBUTTON_NOHFILL	0x01
#define AG_MFSPINBUTTON_VFILL	0x02

typedef struct ag_mfspinbutton {
	struct ag_widget wid;
	AG_Mutex lock;
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
AG_MFSpinbutton	*AG_MFSpinbuttonNew(void *, Uint, const char *, const char *,
		                    const char *);
void		 AG_MFSpinbuttonInit(AG_MFSpinbutton *, Uint, const char *,
		                     const char *, const char *);
void		 AG_MFSpinbuttonDestroy(void *);
void		 AG_MFSpinbuttonScale(void *, int, int);
void		 AG_MFSpinbuttonDraw(void *);

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

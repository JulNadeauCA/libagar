/*	Public domain	*/

#ifndef _AGAR_WIDGET_NUMERICAL_H_
#define _AGAR_WIDGET_NUMERICAL_H_

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

typedef struct ag_numerical {
	struct ag_widget wid;
	Uint flags;
#define AG_NUMERICAL_HFILL	0x01
#define AG_NUMERICAL_VFILL	0x02

	AG_Mutex lock;
	double value;			/* Default value binding */
	double min, max;		/* Default range bindings */
	double inc;			/* Increment for buttons */
	char format[32];		/* Printing format */
	const AG_Unit *unit;		/* Conversion unit in use */
	int writeable;			/* 0 = read-only */
	AG_Textbox *input;		/* Input textbox */
	AG_UCombo *units;		/* Unit selector */
	AG_Button *incbu;		/* Increment button */
	AG_Button *decbu;		/* Decrement button */
	int wUnitSel, hUnitSel;		/* Initial size hints */
	int wPreUnit;
} AG_Numerical;

__BEGIN_DECLS
extern AG_WidgetClass agNumericalClass;

AG_Numerical *AG_NumericalNew(void *, Uint, const char *, const char *);
void          AG_NumericalSizeHint(AG_Numerical *, const char *);

void	AG_NumericalSetValue(AG_Numerical *, double);
void	AG_NumericalAddValue(AG_Numerical *, double);
#define AG_NumericalSubValue(num,val) \
	AG_NumericalAddValue((num),-(val))

void	AG_NumericalSetMin(AG_Numerical *, double);
#define AG_NumericalSetMinInt(num, v) AG_NumericalSetMin((num),(double)(v))
#define AG_NumericalSetMinFlt(num, v) AG_NumericalSetMin((num),(double)(v))
#define AG_NumericalSetMinDbl(num, v) AG_NumericalSetMin((num),(v))

void	AG_NumericalSetMax(AG_Numerical *, double);
#define AG_NumericalSetMaxInt(num, v) AG_NumericalSetMax((num),(double)(v))
#define AG_NumericalSetMaxFlt(num, v) AG_NumericalSetMax((num),(double)(v))
#define AG_NumericalSetMaxDbl(num, v) AG_NumericalSetMax((num),(v))

void	AG_NumericalSetRange(AG_Numerical *, double, double);
#define AG_NumericalSetRangeInt(num, min, max) \
	AG_NumericalSetRange((num),(double)(min),(double)(max))
#define AG_NumericalSetRangeFlt(num, min, max) \
	AG_NumericalSetRange((num),(double)(min),(double)(max))
#define AG_NumericalSetRangeDbl(num, min, max) \
	AG_NumericalSetRange((num),(min),(max))

void	AG_NumericalSetIncrement(AG_Numerical *, double);
void	AG_NumericalSelectUnit(AG_Numerical *, const char *);
int	AG_NumericalSetUnitSystem(AG_Numerical *, const char *);
void	AG_NumericalSetPrecision(AG_Numerical *, const char *, int);
void	AG_NumericalSetWriteable(AG_Numerical *, int);

float	AG_NumericalGetFloat(AG_Numerical *);
double	AG_NumericalGetDouble(AG_Numerical *);
int	AG_NumericalGetInt(AG_Numerical *);
#define AG_NumericalGetUint(n) ((Uint)AG_NumericalGetInt(n))
Uint32	AG_NumericalGetUint32(AG_Numerical *);
#define AG_NumericalGetUint8(n) ((Uint8)AG_NumericalGetUint32(n))
#define AG_NumericalGetUint16(n) ((Uint16)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint8(n) ((Sint8)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint16(n) ((Sint16)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint32(n) ((Sint32)AG_NumericalGetUint32(n))
#ifdef HAVE_64BIT
Uint64	AG_NumericalGetUint64(AG_Numerical *);
#define AG_NumericalGetSint64(n) ((Sint64)AG_NumericalGetUint64(n))
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_NUMERICAL_H_ */

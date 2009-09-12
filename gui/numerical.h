/*	Public domain	*/

#ifndef _AGAR_WIDGET_NUMERICAL_H_
#define _AGAR_WIDGET_NUMERICAL_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/units.h>

#include <agar/gui/begin.h>

typedef struct ag_numerical {
	struct ag_widget wid;
	Uint flags;
#define AG_NUMERICAL_HFILL	0x01
#define AG_NUMERICAL_VFILL	0x02

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

AG_Numerical *AG_NumericalNew(void *, Uint, const char *, const char *, ...)
                              FORMAT_ATTRIBUTE(printf,4,5);
AG_Numerical *AG_NumericalNewS(void *, Uint, const char *, const char *);

AG_Numerical *AG_NumericalNewDbl(void *, Uint, const char *, const char *,
                                 double *);
AG_Numerical *AG_NumericalNewDblR(void *, Uint, const char *, const char *,
                                  double *, double, double);
AG_Numerical *AG_NumericalNewFlt(void *, Uint, const char *, const char *,
                                 float *);
AG_Numerical *AG_NumericalNewFltR(void *, Uint, const char *, const char *,
                                 float *, float, float);

AG_Numerical *AG_NumericalNewInt(void *, Uint, const char *, const char *,
                                 int *);
AG_Numerical *AG_NumericalNewIntR(void *, Uint, const char *, const char *,
                                  int *, int, int);

AG_Numerical *AG_NumericalNewUint(void *, Uint, const char *, const char *,
                                  Uint *);
AG_Numerical *AG_NumericalNewUintR(void *, Uint, const char *, const char *,
                                  Uint *, Uint, Uint);

AG_Numerical *AG_NumericalNewUint8(void *, Uint, const char *, const char *,
                                   Uint8 *);
AG_Numerical *AG_NumericalNewUint8R(void *, Uint, const char *, const char *,
                                    Uint8 *, Uint8, Uint8);
				   
AG_Numerical *AG_NumericalNewSint8(void *, Uint, const char *, const char *,
                                   Sint8 *);
AG_Numerical *AG_NumericalNewSint8R(void *, Uint, const char *, const char *,
                                    Sint8 *, Sint8, Sint8);
				   
AG_Numerical *AG_NumericalNewUint16(void *, Uint, const char *, const char *,
                                    Uint16 *);
AG_Numerical *AG_NumericalNewUint16R(void *, Uint, const char *, const char *,
                                     Uint16 *, Uint16, Uint16);

AG_Numerical *AG_NumericalNewSint16(void *, Uint, const char *, const char *,
                                    Sint16 *);
AG_Numerical *AG_NumericalNewSint16R(void *, Uint, const char *, const char *,
                                     Sint16 *, Sint16, Sint16);
				    
AG_Numerical *AG_NumericalNewUint32(void *, Uint, const char *, const char *,
                                    Uint32 *);
AG_Numerical *AG_NumericalNewUint32R(void *, Uint, const char *, const char *,
                                    Uint32 *, Uint32, Uint32);
				    
AG_Numerical *AG_NumericalNewSint32(void *, Uint, const char *, const char *,
                                    Sint32 *);
AG_Numerical *AG_NumericalNewSint32R(void *, Uint, const char *, const char *,
                                    Sint32 *, Sint32, Sint32);

void    AG_NumericalSizeHint(AG_Numerical *, const char *);
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

float	AG_NumericalGetFlt(AG_Numerical *);
double	AG_NumericalGetDbl(AG_Numerical *);
int	AG_NumericalGetInt(AG_Numerical *);
#define AG_NumericalGetUint(n) ((Uint)AG_NumericalGetInt(n))
Uint32	AG_NumericalGetUint32(AG_Numerical *);
#define AG_NumericalGetUint8(n) ((Uint8)AG_NumericalGetUint32(n))
#define AG_NumericalGetUint16(n) ((Uint16)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint8(n) ((Sint8)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint16(n) ((Sint16)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint32(n) ((Sint32)AG_NumericalGetUint32(n))
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_NUMERICAL_H_ */

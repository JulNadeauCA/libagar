/*	Public domain	*/

#ifndef _AGAR_WIDGET_NUMERICAL_H_
#define _AGAR_WIDGET_NUMERICAL_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/units.h>

#include <agar/gui/begin.h>
	
typedef union ag_numerical_value {
	int i;
	Uint u;
	Uint8 u8;
	Sint8 s8;
	Uint16 u16;
	Sint16 s16;
	Uint32 u32;
	Sint32 s32;
#ifdef AG_HAVE_64BIT
	Uint64 u64;
	Sint64 s64;
#endif
	float flt;
	double dbl;
#ifdef AG_HAVE_LONG_DOUBLE
	long double ldbl;
#endif
} AG_NumericalValue;

typedef struct ag_numerical {
	struct ag_widget wid;
	Uint flags;
#define AG_NUMERICAL_HFILL	0x01
#define AG_NUMERICAL_VFILL	0x02
#define AG_NUMERICAL_INT	0x04	/* Default binding should be int */
#define AG_NUMERICAL_EXCL	0x08	/* Exclusive access to bindings */

	double incr;			/* Button increment value */
	char format[32];		/* Printing format */
	const AG_Unit *unit;		/* Conversion unit in use */
	int writeable;			/* 0 = read-only */
	char inTxt[64];			/* Input text buffer */
	AG_Textbox *input;		/* Input textbox */
	AG_UCombo *units;		/* Unit selector */
	AG_Button *incbu;		/* Increment button */
	AG_Button *decbu;		/* Decrement button */
	int wUnitSel, hUnitSel;		/* Size hints for entry box */
	int wPreUnit;			/* Size hint for unit selector */
	AG_Timer updateTo;		/* Timer for non-EXCL mode */
	AG_NumericalValue value;	/* Default value binding */
	AG_NumericalValue min, max;	/* Default range bindings */
} AG_Numerical;

__BEGIN_DECLS
extern AG_WidgetClass agNumericalClass;

AG_Numerical *AG_NumericalNew(void *, Uint, const char *, const char *, ...)
                              FORMAT_ATTRIBUTE(printf,4,5);
AG_Numerical *AG_NumericalNewS(void *, Uint, const char *, const char *);

AG_Numerical *AG_NumericalNewFlt(void *, Uint, const char *, const char *, float *);
AG_Numerical *AG_NumericalNewFltR(void *, Uint, const char *, const char *, float *, float, float);
AG_Numerical *AG_NumericalNewDbl(void *, Uint, const char *, const char *, double *);
AG_Numerical *AG_NumericalNewDblR(void *, Uint, const char *, const char *, double *, double, double);
#ifdef AG_HAVE_LONG_DOUBLE
AG_Numerical *AG_NumericalNewLdbl(void *, Uint, const char *, const char *, long double *);
AG_Numerical *AG_NumericalNewLdblR(void *, Uint, const char *, const char *, long double *, long double, long double);
#endif
AG_Numerical *AG_NumericalNewInt(void *, Uint, const char *, const char *, int *);
AG_Numerical *AG_NumericalNewIntR(void *, Uint, const char *, const char *, int *, int, int);
AG_Numerical *AG_NumericalNewUint(void *, Uint, const char *, const char *, Uint *);
AG_Numerical *AG_NumericalNewUintR(void *, Uint, const char *, const char *, Uint *, Uint, Uint);
AG_Numerical *AG_NumericalNewUint8(void *, Uint, const char *, const char *, Uint8 *);
AG_Numerical *AG_NumericalNewUint8R(void *, Uint, const char *, const char *, Uint8 *, Uint8, Uint8);
AG_Numerical *AG_NumericalNewSint8(void *, Uint, const char *, const char *, Sint8 *);
AG_Numerical *AG_NumericalNewSint8R(void *, Uint, const char *, const char *, Sint8 *, Sint8, Sint8);
AG_Numerical *AG_NumericalNewUint16(void *, Uint, const char *, const char *, Uint16 *);
AG_Numerical *AG_NumericalNewUint16R(void *, Uint, const char *, const char *, Uint16 *, Uint16, Uint16);
AG_Numerical *AG_NumericalNewSint16(void *, Uint, const char *, const char *, Sint16 *);
AG_Numerical *AG_NumericalNewSint16R(void *, Uint, const char *, const char *, Sint16 *, Sint16, Sint16);
AG_Numerical *AG_NumericalNewUint32(void *, Uint, const char *, const char *, Uint32 *);
AG_Numerical *AG_NumericalNewUint32R(void *, Uint, const char *, const char *, Uint32 *, Uint32, Uint32);
AG_Numerical *AG_NumericalNewSint32(void *, Uint, const char *, const char *, Sint32 *);
AG_Numerical *AG_NumericalNewSint32R(void *, Uint, const char *, const char *, Sint32 *, Sint32, Sint32);
#ifdef AG_HAVE_64BIT
AG_Numerical *AG_NumericalNewUint64(void *, Uint, const char *, const char *, Uint64 *);
AG_Numerical *AG_NumericalNewUint64R(void *, Uint, const char *, const char *, Uint64 *, Uint64, Uint64);
AG_Numerical *AG_NumericalNewSint64(void *, Uint, const char *, const char *, Sint64 *);
AG_Numerical *AG_NumericalNewSint64R(void *, Uint, const char *, const char *, Sint64 *, Sint64, Sint64);
#endif
void    AG_NumericalSizeHint(AG_Numerical *, const char *);

void	AG_NumericalIncrement(AG_Numerical *);
void	AG_NumericalDecrement(AG_Numerical *);
void	AG_NumericalSetValue(AG_Numerical *, AG_NumericalValue);
void    AG_NumericalUpdate(AG_Numerical *);
void	AG_NumericalSetIncrement(AG_Numerical *, double);
void	AG_NumericalSelectUnit(AG_Numerical *, const char *);
int	AG_NumericalSetUnitSystem(AG_Numerical *, const char *);
void	AG_NumericalSetPrecision(AG_Numerical *, const char *, int);
void	AG_NumericalSetWriteable(AG_Numerical *, int);

float	    AG_NumericalGetFlt(AG_Numerical *);
double	    AG_NumericalGetDbl(AG_Numerical *);
#ifdef AG_HAVE_LONG_DOUBLE
long double AG_NumericalGetLdbl(AG_Numerical *);
#endif

int	AG_NumericalGetInt(AG_Numerical *);
#define AG_NumericalGetUint(n) ((Uint)AG_NumericalGetInt(n))
#define AG_NumericalGetUint8(n) ((Uint8)AG_NumericalGetUint32(n))
#define AG_NumericalGetUint16(n) ((Uint16)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint8(n) ((Sint8)AG_NumericalGetUint32(n))
#define AG_NumericalGetSint16(n) ((Sint16)AG_NumericalGetUint32(n))
Uint32	AG_NumericalGetUint32(AG_Numerical *);
#define AG_NumericalGetSint32(n) ((Sint32)AG_NumericalGetUint32(n))
#ifdef AG_HAVE_64BIT
Uint64	AG_NumericalGetUint64(AG_Numerical *);
#define AG_NumericalGetSint64(n) ((Sint64)AG_NumericalGetUint64(n))
#endif

#ifdef AG_LEGACY
void    AG_NumericalSetRangeDbl(AG_Numerical *, double, double) DEPRECATED_ATTRIBUTE;
void    AG_NumericalSetRangeInt(AG_Numerical *, int, int) DEPRECATED_ATTRIBUTE;
void    AG_NumericalSetRangeFlt(AG_Numerical *, float, float) DEPRECATED_ATTRIBUTE;
#define AG_NumericalSetRange AG_NumericalSetRangeDbl
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_NUMERICAL_H_ */

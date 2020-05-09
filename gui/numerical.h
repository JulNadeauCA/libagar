/*	Public domain	*/

#ifndef _AGAR_WIDGET_NUMERICAL_H_
#define _AGAR_WIDGET_NUMERICAL_H_

#include <agar/gui/widget.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/units.h>

#include <agar/gui/begin.h>

#ifndef AG_NUMERICAL_INPUT_MAX
#define AG_NUMERICAL_INPUT_MAX 56
#endif

typedef struct ag_numerical {
	struct ag_widget wid;          /* AG_Widget -> AG_Numerical */
	Uint flags;
#define AG_NUMERICAL_HFILL     0x01
#define AG_NUMERICAL_VFILL     0x02
#define AG_NUMERICAL_INT       0x04  /* Make the default binding an int */
                                     /* (default = float) */
#define AG_NUMERICAL_EXCL      0x08  /* Assume exclusive access to variable */
#define AG_NUMERICAL_READONLY  0x10  /* Read-only mode */
#define AG_NUMERICAL_SLOW      0x20  /* Update every 2s (default = 250ms) */
#define AG_NUMERICAL_EXPAND   (AG_NUMERICAL_HFILL | AG_NUMERICAL_VFILL)

	char format[28];                     /* Print format (for reals) */
	const AG_Unit *_Nonnull unit;        /* Conversion unit in use */
	AG_UCombo *_Nullable units;          /* Unit selector */
	char inTxt[AG_NUMERICAL_INPUT_MAX];  /* Input text buffer */
	AG_Textbox *_Nonnull input;          /* Input textbox */
	AG_Button *_Nonnull incbu;           /* Increment button */
	AG_Button *_Nonnull decbu;           /* Decrement button */
	int wUnitSel, hUnitSel;              /* Size hints for entry box */
	int wPreUnit;                        /* Size hint for unit selector */
	Uint32 _pad;
	AG_Timer toUpdate;                   /* For refresh (not in EXCL mode) */
	AG_Timer toInc, toDec;               /* For keyboard increment/decrement */
} AG_Numerical;

#define AGNUMERICAL(obj)            ((AG_Numerical *)(obj))
#define AGCNUMERICAL(obj)           ((const AG_Numerical *)(obj))
#define AG_NUMERICAL_SELF()          AGNUMERICAL( AG_OBJECT(0,"AG_Widget:AG_Numerical:*") )
#define AG_NUMERICAL_PTR(n)          AGNUMERICAL( AG_OBJECT((n),"AG_Widget:AG_Numerical:*") )
#define AG_NUMERICAL_NAMED(n)        AGNUMERICAL( AG_OBJECT_NAMED((n),"AG_Widget:AG_Numerical:*") )
#define AG_CONST_NUMERICAL_SELF()   AGCNUMERICAL( AG_CONST_OBJECT(0,"AG_Widget:AG_Numerical:*") )
#define AG_CONST_NUMERICAL_PTR(n)   AGCNUMERICAL( AG_CONST_OBJECT((n),"AG_Widget:AG_Numerical:*") )
#define AG_CONST_NUMERICAL_NAMED(n) AGCNUMERICAL( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Numerical:*") )

__BEGIN_DECLS
extern AG_WidgetClass agNumericalClass;

AG_Numerical *_Nonnull AG_NumericalNew(void *_Nullable, Uint,
                                       const char *_Nullable,
				       const char *_Nullable, ...)
                                      FORMAT_ATTRIBUTE(printf,4,5);

AG_Numerical *_Nonnull AG_NumericalNewS(void *_Nullable, Uint,
                                        const char *_Nullable,
					const char *_Nullable);

AG_Numerical *_Nonnull AG_NumericalNewInt(void *_Nullable, Uint,
                                          const char *_Nullable,
                                          const char *_Nullable, int *_Nonnull);

AG_Numerical *_Nonnull AG_NumericalNewIntR(void *_Nullable, Uint,
                                           const char *_Nullable,
					   const char *_Nullable, int *_Nonnull,
					   int,int);

AG_Numerical *_Nonnull AG_NumericalNewUint(void *_Nullable, Uint, const char *_Nullable, const char *_Nullable, Uint *_Nonnull);
AG_Numerical *_Nonnull AG_NumericalNewUintR(void *_Nullable, Uint, const char *_Nullable, const char *_Nullable, Uint *_Nonnull, Uint,Uint);
AG_Numerical *_Nonnull AG_NumericalNewFlt(void *_Nullable, Uint, const char *_Nullable, const char *_Nullable, float *_Nonnull);
AG_Numerical *_Nonnull AG_NumericalNewFltR(void *_Nullable, Uint, const char *_Nullable, const char *_Nullable, float *_Nonnull, float,float);
AG_Numerical *_Nonnull AG_NumericalNewDbl(void *_Nullable, Uint, const char *_Nullable, const char *_Nullable, double *_Nonnull);
AG_Numerical *_Nonnull AG_NumericalNewDblR(void *_Nullable, Uint, const char *_Nullable, const char *_Nullable, double *_Nonnull, double,double);

float  AG_NumericalGetFlt(AG_Numerical *_Nonnull);
double AG_NumericalGetDbl(AG_Numerical *_Nonnull);

void AG_NumericalSizeHint(AG_Numerical *_Nonnull, const char *_Nullable);
void AG_NumericalIncrement(AG_Numerical *_Nonnull);
void AG_NumericalDecrement(AG_Numerical *_Nonnull);
void AG_NumericalUpdate(AG_Numerical *_Nonnull);

void AG_NumericalSelectUnit(AG_Numerical *_Nonnull, const char *_Nonnull);
int  AG_NumericalSetUnitSystem(AG_Numerical *_Nonnull, const char *_Nonnull);

void AG_NumericalSetWriteable(AG_Numerical *_Nonnull, int);
void AG_NumericalSetPrecision(AG_Numerical *_Nonnull, const char *_Nonnull, int);

int 	 AG_NumericalGetInt(AG_Numerical *_Nonnull);
#define  AG_NumericalGetUint(n)     ((Uint)AG_NumericalGetInt(n))

Uint32	 AG_NumericalGetUint32(AG_Numerical *_Nonnull);
# define AG_NumericalGetUint8(n)   ((Uint8)AG_NumericalGetUint32(n))
# define AG_NumericalGetUint16(n) ((Uint16)AG_NumericalGetUint32(n))
# define AG_NumericalGetSint8(n)   ((Sint8)AG_NumericalGetUint32(n))
# define AG_NumericalGetSint16(n) ((Sint16)AG_NumericalGetUint32(n))
# define AG_NumericalGetSint32(n) ((Sint32)AG_NumericalGetUint32(n))
#ifdef AG_HAVE_64BIT
Uint64	AG_NumericalGetUint64(AG_Numerical *_Nonnull);
#define AG_NumericalGetSint64(n) ((Sint64)AG_NumericalGetUint64(n))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_NUMERICAL_H_ */

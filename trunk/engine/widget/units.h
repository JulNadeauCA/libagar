/*	$Csoft: units.h,v 1.14 2005/06/01 08:48:31 vedge Exp $	*/
/*	Public domain	*/

#include <config/historical_units.h>

#ifndef _AGAR_WIDGET_UNITS_H_
#define _AGAR_WIDGET_UNITS_H_
#include "begin_code.h"

typedef struct {
	char *key;		/* Key */
	char *abbr;		/* Symbol */
	char *name;		/* Long name */
	double divider;		/* Base unit divider (for linear conv) */
	double (*func)(double, int);	/* For nonlinear conversions */
} AG_Unit;

__BEGIN_DECLS
const AG_Unit	  *AG_FindUnit(const char *);
__inline__ double  AG_Unit2Base(double, const AG_Unit *);
double		   AG_Base2Unit(double, const AG_Unit *);
__inline__ double  AG_Unit2Unit(double, const AG_Unit *, const AG_Unit *);
const AG_Unit	  *AG_BestUnit(const AG_Unit[], double);
const char	  *AG_UnitAbbr(const AG_Unit *);
__inline__ int	   AG_UnitFormat(double, const AG_Unit[], char *, size_t);

double	AG_UnitFahrenheit(double, int);
double	AG_UnitCelsius(double, int);
#ifdef HISTORICAL_UNITS
double	AG_UnitRankine(double, int);
double	AG_UnitReaumur(double, int);
#endif

#define	AG_Unit2Basef(n, u) ((float)AG_Unit2Base((float)(n), (u)))
#define	AG_Base2Unitf(n, u) ((float)AG_Base2Unit((float)(n), (u)))
#define	AG_Unit2Unitf(n, u1, u2) ((float)AG_Unit2Unit((float)(n), (u1), (u2)))

extern const AG_Unit *agUnitGroups[];
extern const int agnUnitGroups;

extern const AG_Unit agIdentityUnit[];
extern const AG_Unit agLengthUnits[];
extern const AG_Unit agVideoUnits[];
extern const AG_Unit agAreaUnits[];
extern const AG_Unit agVolumeUnits[];
extern const AG_Unit agSpeedUnits[];
extern const AG_Unit agMassUnits[];
extern const AG_Unit agTimeUnits[];
extern const AG_Unit agCurrentUnits[];
extern const AG_Unit agTemperatureUnits[];
extern const AG_Unit agSubstanceAmountUnits[];
extern const AG_Unit agLightUnits[];
extern const AG_Unit agPowerUnits[];
extern const AG_Unit agEMFUnits[];
extern const AG_Unit agResistanceUnits[];
extern const AG_Unit agResistanceTC1Units[];
extern const AG_Unit agResistanceTC2Units[];
extern const AG_Unit agCapacitanceUnits[];
extern const AG_Unit agInductanceUnits[];
extern const AG_Unit agFrequencyUnits[];
extern const AG_Unit agPressureUnits[];
extern const AG_Unit agMetabolicExpenditureUnits[];
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_UNITS_H_ */

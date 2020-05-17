/*	Public domain	*/

#ifndef _AGAR_WIDGET_UNITS_H_
#define _AGAR_WIDGET_UNITS_H_

#include <agar/gui/begin.h>

typedef struct ag_unit {
	char *_Nullable key;		/* Key (or list terminator) */
	char *_Nonnull abbr;		/* Abbreviated symbol */
	char *_Nonnull name;		/* Long name */
	double divider;			/* Conversion factor (linear) */
} AG_Unit;

typedef struct ag_unit_nl {
	AG_Unit unit;						/* Inherit */
	double (*_Nonnull func)(double, int);
} AG_UnitNL;

#define AG_UNIT(unit)    ((AG_Unit *)(unit))
#define AG_UNIT_NL(unit) ((AG_UnitNL *)(unit))

__BEGIN_DECLS

/* Nonlinear units */
double AG_UnitFahrenheit(double, int);
double AG_UnitCelsius(double, int);

extern const AG_Unit *_Nonnull  agUnitGroups[];
extern const char    *_Nullable agUnitGroupNames[];
extern const int     agnUnitGroups;

extern const AG_Unit agIdentityUnit[];
extern const AG_Unit agLengthUnits[];
extern const AG_Unit agAngleUnits[];
extern const AG_Unit agPercentageUnits[];
extern const AG_Unit agVideoUnits[];
extern const AG_Unit agAreaUnits[];
extern const AG_Unit agVolumeUnits[];
extern const AG_Unit agSpeedUnits[];
extern const AG_Unit agMassUnits[];
extern const AG_Unit agTimeUnits[];
extern const AG_Unit agCurrentUnits[];
extern const AG_UnitNL agTemperatureUnits[];
extern const AG_Unit agSubstanceAmountUnits[];
extern const AG_Unit agEnergyPerSubstanceAmountUnits[];
extern const AG_Unit agMolarHeatCapacityUnits[];
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
extern const AG_Unit agVacuumUnits[];
extern const AG_Unit agResistivityUnits[];
extern const AG_Unit agThermalConductivityUnits[];
extern const AG_Unit agThermalExpansionUnits[];
extern const AG_Unit agDensityUnits[];

int AG_UnitIsNonlinear(const char *_Nonnull) _Const_Attribute;
double AG_Unit2Base(double, const AG_Unit *_Nonnull) _Pure_Attribute;
double AG_Base2Unit(double, const AG_Unit *_Nonnull) _Pure_Attribute;

double AG_Unit2Unit(double, const AG_Unit *_Nonnull, const AG_Unit *_Nonnull)
                   _Pure_Attribute;

const char *_Nonnull AG_UnitAbbr(const AG_Unit *_Nonnull) _Pure_Attribute;

const AG_Unit *_Nullable AG_FindUnit(const char *_Nonnull);
const AG_Unit *_Nonnull  AG_BestUnit(const AG_Unit[_Nonnull], double);

int AG_UnitFormat(double, const AG_Unit[_Nonnull],
                  char *_Nonnull, AG_Size);

#define	AG_Unit2Basef(n, u) ((float)AG_Unit2Base((float)(n), (u)))
#define	AG_Base2Unitf(n, u) ((float)AG_Base2Unit((float)(n), (u)))
#define	AG_Unit2Unitf(n, u1, u2) ((float)AG_Unit2Unit((float)(n), (u1), (u2)))
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_UNITS_H_ */

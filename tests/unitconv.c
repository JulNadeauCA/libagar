/*	Public domain	*/
/*
 * This is a simple unit converter based on AG_Units(3).
 */

#include "agartest.h"

double value = 0.0;
const AG_Unit *unitGroup = agLengthUnits;
AG_Numerical *n1, *n2;

static void
SelectCategory(AG_Event *event)
{
	AG_Unit *group = AG_PTR(1), *unit;
		
	for (unit = &group[0]; unit->key != NULL; unit++) {
		if (unit->divider == 1) {
			AG_NumericalSetUnitSystem(n1, unit->key);
			AG_NumericalSetUnitSystem(n2, unit->key);
			return;
		}
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	const struct {
		const char *name;
		const AG_Unit *p;
	} units[] = {
		{ "Len", agLengthUnits },
		{ "Ang", agAngleUnits },
		{ "Mass", agMassUnits },
		{ "Area", agAreaUnits },
		{ "Vol", agVolumeUnits },
		{ "Spd", agSpeedUnits },
		{ "Time", agTimeUnits },
		{ "Temp", AG_UNIT(agTemperatureUnits) },
		{ "Pwr", agPowerUnits },
		{ "Press", agPressureUnits },
		{ "Vac", agVacuumUnits }
	};
	int i;
	AG_Toolbar *tb;

	AG_SetStyle(win, "padding", "10");

	tb = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 2, AG_TOOLBAR_HOMOGENOUS|
	                                             AG_TOOLBAR_STICKY|
						     AG_TOOLBAR_HFILL);

	for (i = 0; i < sizeof(units)/sizeof(units[0]); i++) {
		if (i == 6) {
			AG_ToolbarRow(tb, 1);
		}
		AG_ToolbarButton(tb, units[i].name, (i == 0),
		    SelectCategory, "%p", units[i].p);
	}

	AG_SeparatorNewHoriz(win);

	n1 = AG_NumericalNewS(win, AG_NUMERICAL_HFILL, "in", "Value: ");
	n2 = AG_NumericalNewS(win, AG_NUMERICAL_HFILL, "mm", "Value: ");
	AG_BindDouble(n1, "value", &value);
	AG_BindDouble(n2, "value", &value);
	AG_NumericalSizeHint(n1, "0000.00");
	AG_NumericalSizeHint(n2, "0000.00");
	AG_NumericalSetPrecision(n1, "g", 6);
	AG_NumericalSetPrecision(n2, "g", 6);
	return (0);
}

const AG_TestCase unitconvTest = {
	"unitconv",
	N_("Test AG_Units(3) conversion"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

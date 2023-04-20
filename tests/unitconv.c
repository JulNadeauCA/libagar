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
		{ AGSI_IDEOGRAM AGSI_HORIZONTAL_RULE   AGSI_RST " Len",   agLengthUnits },
		{ AGSI_IDEOGRAM AGSI_MEASURED_ANGLE    AGSI_RST " Ang",   agAngleUnits },
		{ AGSI_IDEOGRAM AGSI_SCALES            AGSI_RST " Mass",  agMassUnits },
		{ AGSI_IDEOGRAM AGSI_MEASURED_AREA     AGSI_RST " Area",  agAreaUnits },
		{ AGSI_IDEOGRAM AGSI_MEASURED_VOLUME   AGSI_RST " Vol",   agVolumeUnits },
		{ AGSI_IDEOGRAM AGSI_STOPWATCH         AGSI_RST " Spd",   agSpeedUnits },
		{ AGSI_IDEOGRAM AGSI_EMPTY_HOURGLASS   AGSI_RST " Time",  agTimeUnits },
		{ AGSI_IDEOGRAM AGSI_MEASURED_TEMP     AGSI_RST " Temp",  agTemperatureUnits },
		{ AGSI_IDEOGRAM AGSI_LIGHTNING         AGSI_RST " Pow",   agPowerUnits },
		{ AGSI_IDEOGRAM AGSI_PRESSURE_GAUGE    AGSI_RST " Press", agPressureUnits },
		{ AGSI_IDEOGRAM AGSI_BELL_JAR_W_VACUUM AGSI_RST " Vac",   agVacuumUnits }
	};
	const int nUnits = sizeof(units) / sizeof(units[0]);
	int i;
	AG_Toolbar *tb;

	AG_SetPadding(win, "10");

	tb = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 2, AG_TOOLBAR_HOMOGENOUS |
	                                             AG_TOOLBAR_STICKY |
						     AG_TOOLBAR_HFILL);
	for (i = 0; i < nUnits; i++) {
		if (i == 6) {
			AG_ToolbarRow(tb, 1);
		}
		AG_ToolbarButton(tb, units[i].name, (i == 0),
		    SelectCategory, "%p", units[i].p);
	}

	AG_SeparatorNewHoriz(win);

	n1 = AG_NumericalNewS(win, AG_NUMERICAL_HFILL, "in", "Value: ");
	n2 = AG_NumericalNewS(win, AG_NUMERICAL_HFILL, "mm", "Value: ");
	AG_SetMargin(n1, "0 0 10 0");
	AG_BindDouble(n1, "value", &value);
	AG_BindDouble(n2, "value", &value);
	AG_NumericalSizeHint(n1, "0000.00");
	AG_NumericalSizeHint(n2, "0000.00");
	AG_NumericalSetPrecision(n1, "g", 6);
	AG_NumericalSetPrecision(n2, "g", 6);
	return (0);
}

const AG_TestCase unitconvTest = {
	AGSI_IDEOGRAM AGSI_UNIT_CONVERSION AGSI_RST,
	"unitconv",
	N_("Test AG_Units(3) conversion"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

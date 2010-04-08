/*	Public domain	*/
/*
 * This is a simple unit converter based on AG_Units(3).
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

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

static void
CreateUI(void)
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
		{ "Temp", agTemperatureUnits },
		{ "Pwr", agPowerUnits },
		{ "Press", agPressureUnits },
		{ "Vac", agVacuumUnits },
		{ "Met", agMetabolicExpenditureUnits },
	};
	int i;
	AG_Window *win;
	AG_Toolbar *tb;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Unit Converter");
	AG_WindowSetPadding(win, 10, 10, 10, 10);

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

	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: unitconv [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("unitconv", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	
	CreateUI();
	agColors[WINDOW_BG_COLOR] = AG_ColorRGB(60,60,60);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


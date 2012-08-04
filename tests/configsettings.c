/*	Public domain	*/
/*
 * This demonstrates the use of AG_Variable(3) with the agConfig object
 * for saving general configuration settings.
 */

#include "agartest.h"

int someInt = 1234;
int someBool = 0;
char someString[64];

static void
LoadConfig(AG_Event *event)
{
	AG_TestInstance *ti = AG_PTR(1);

	if (AG_ConfigLoad() == 0) {
		TestMsg(ti, "Loaded configuration successfully");
	} else {
		TestMsg(ti, "AG_ConfigLoad: %s", AG_GetError());
	}
}

static void
SaveConfig(AG_Event *event)
{
	AG_TestInstance *ti = AG_PTR(1);

	if (AG_ConfigSave() == 0) {
		TestMsg(ti, "Saved configuration successfully");
	} else {
		TestMsg(ti, "AG_ConfigSave: %s", AG_GetError());
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_TestInstance *ti = obj;
	AG_Box *box;
	AG_Textbox *tb;

	someString[0] = '\0';

	/* Tie some globals to the config settings */
	AG_BindInt(agConfig, "some-int", &someInt);
	AG_BindInt(agConfig, "some-bool", &someBool);
	AG_BindString(agConfig, "some-string", someString, sizeof(someString));
	AG_SetInt(agConfig, "some-int", 2345);

	/* Create some widgets */
	AG_NumericalNewInt(win, 0, NULL, "Some int: ", &someInt);
	AG_CheckboxNewInt(win, 0, "Some bool", &someBool);
	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL, "Some string: ");
	AG_TextboxBindUTF8(tb, someString, sizeof(someString));

	box = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	{
		AG_ButtonNewFn(box, 0, "Load configuration", LoadConfig, "%p", ti);
		AG_ButtonNewFn(box, 0, "Save configuration", SaveConfig, "%p", ti);
	}
	return (0);
}

const AG_TestCase configSettingsTest = {
	"configSettings",
	N_("Test user-specified AG_Config(3) parameters"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};

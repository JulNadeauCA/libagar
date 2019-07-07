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
	char path[AG_PATHNAME_MAX];
	AG_TestInstance *ti = AG_PTR(1);

	if (AG_ObjectCopyFilename(agConfig, path, sizeof(path)) == 0)
		TestMsg(ti, "Loading from %s", path);

	if (AG_ConfigLoad() == 0) {
		TestMsg(ti, "Loaded configuration successfully");
	} else {
		TestMsg(ti, "AG_ConfigLoad: %s", AG_GetError());
	}
}

static void
SaveConfig(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_TestInstance *ti = AG_PTR(1);
	
	if (AG_ObjectCopyFilename(agConfig, path, sizeof(path)) == 0)
		TestMsg(ti, "Saving to %s", path);

	if (AG_ConfigSave() == 0) {
		TestMsg(ti, "Saved configuration successfully");
	} else {
		TestMsg(ti, "AG_ConfigSave: %s", AG_GetError());
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ConfigPath *cp;
	AG_TestInstance *ti = obj;
	AG_Box *box;
	AG_Textbox *tb;
	AG_Label *lbl;

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
#ifdef AG_UNICODE
	AG_TextboxBindUTF8(tb, someString, sizeof(someString));
#else
	AG_TextboxBindASCII(tb, someString, sizeof(someString));
#endif
	box = AG_BoxNewVert(win, AG_BOX_EXPAND|AG_BOX_FRAME);
	AG_BoxSetLabelS(box, "AG_ConfigFind() search paths:");
	AG_SetStyle(box, "font-size", "90%");
	AG_SetStyle(box, "font-weight", "bold");
	{
		AG_TAILQ_FOREACH(cp, &agConfig->paths[AG_CONFIG_PATH_DATA], paths) {
			lbl = AG_LabelNew(box, 0, "Data: %s", cp->s);
			AG_SetStyle(lbl, "font-family", "Courier");
		}
		AG_TAILQ_FOREACH(cp, &agConfig->paths[AG_CONFIG_PATH_FONTS], paths) {
			lbl = AG_LabelNew(box, 0, "Fonts: %s", cp->s);
			AG_SetStyle(lbl, "font-family", "Courier");
		}
	}

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
	TestGUI,
	NULL		/* bench */
};

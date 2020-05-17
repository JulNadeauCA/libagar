/*	Public domain	*/
/*
 * This demonstrates the use of AG_Variable(3) with the agConfig object
 * for saving general configuration settings.
 */

#include "agartest.h"

static int someInt = 1234;
static int someBool = 0;
static char someString[64];

static void
LoadConfig(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_TestInstance *ti = AG_PTR(1);
	AG_Object *cfg = AGOBJECT(agConfig);
	AG_Variable *V;

	if (AG_ObjectCopyFilename(cfg, path, sizeof(path)) == 0)
		TestMsg(ti, "Loading from %s", path);

	if (AG_ConfigLoad() == 0) {
		const char *s = "Configuration loaded successfully";
		TestMsg(ti, s);
		AG_TextTmsg(AG_MSG_INFO, 1250, "%s", s);
	} else {
		AG_TextMsgFromError();
		TestMsg(ti, "AG_ConfigLoad: %s", AG_GetError());
	}

	if (AG_Defined(cfg,"some-string")) {
		AG_GetString(cfg, "some-string", someString, sizeof(someString));
	}
	TAILQ_FOREACH(V, &cfg->vars, vars) {
		char val[AG_LABEL_MAX];

		AG_PrintVariable(val, sizeof(val), V);
		TestMsg(ti, "%s: (%s) %s -> %s", cfg->name,
		    agVariableTypes[V->type].name, V->name, val);
	}
}

static void
ShowAgarPrefs(AG_Event *event)
{
	AG_DEV_ConfigShow();
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

static void
UpdateString(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();

	AG_SetString(agConfig, "some-string", someString);
	AG_WidgetUnfocus(tb);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ConfigPath *cp;
	AG_TestInstance *ti = obj;
	AG_Box *box;
	AG_Textbox *tb;
	AG_Label *lbl;

	Strlcpy(someString, "hello", sizeof(someString));

	/* Tie some globals to the config settings */
	AG_BindInt(agConfig, "some-int", &someInt);
	AG_BindInt(agConfig, "some-bool", &someBool);
	AG_SetString(agConfig, "some-string", someString);
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
	AG_SetEvent(tb, "textbox-return", UpdateString, NULL);

	box = AG_BoxNewVert(win, AG_BOX_EXPAND);
	AG_BoxSetLabelS(box, "AG_ConfigFind() search paths:");
	AG_SetStyle(box, "font-size", "90%");
	AG_SetStyle(box, "font-weight", "bold");
	{
		AG_TAILQ_FOREACH(cp, &agConfig->paths[AG_CONFIG_PATH_DATA], paths) {
			lbl = AG_LabelNew(box, 0, "Data: %s", cp->s);
			AG_SetStyle(lbl, "font-family", "courier-prime");
		}
		AG_TAILQ_FOREACH(cp, &agConfig->paths[AG_CONFIG_PATH_FONTS], paths) {
			lbl = AG_LabelNew(box, 0, "Fonts: %s", cp->s);
			AG_SetStyle(lbl, "font-family", "courier-prime");
		}
	}

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_ButtonNewFn(box, 0, "Load Config", LoadConfig, "%p", ti);
		AG_ButtonNewFn(box, 0, "Agar Preferences", ShowAgarPrefs, NULL);
		AG_ButtonNewFn(box, 0, "Save Config", SaveConfig, "%p", ti);
	}
	return (0);
}

const AG_TestCase configsettingsTest = {
	"configsettings",
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

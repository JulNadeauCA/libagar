/*	Public domain	*/

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_ConfigPath *cp;
	AG_Label *lbl;
	int i;

	AG_SeparatorNewHoriz(win);
	{
		AG_LabelNewS(win, 0, "Home directory:");
		AG_GetString(agConfig, "home", path, sizeof(path));
		lbl = AG_LabelNewS(win, 0, path);
		AG_SetStyle(lbl, "font-family", "courier-prime");
	}
	AG_SeparatorNewHoriz(win);
	for (i = 0; i < AG_CONFIG_PATH_LAST; i++) {
		AG_LabelNew(win, 0, "%s: ", _(agConfigPathGroupNames[i]));
		AG_TAILQ_FOREACH(cp, &agConfig->paths[i], paths) {
			lbl = AG_LabelNewS(win, 0, cp->s);
			AG_SetStyle(lbl, "font-family", "courier-prime");
			AG_SetStyle(lbl, "font-size", "90%");
		}
	}
	return (0);
}

const AG_TestCase fspathsTest = {
	"fspaths",
	N_("Display filesystem path defaults"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

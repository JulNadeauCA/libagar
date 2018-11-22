/*	Public domain	*/

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_ConfigPath *cp;
	AG_Label *lbl;

	AG_SeparatorNewHoriz(win);
	{
		AG_LabelNewS(win, 0, "Home directory:");
		AG_GetString(agConfig, "home", path, sizeof(path));
		lbl = AG_LabelNewS(win, 0, path);
		AG_SetStyle(lbl, "font-family", "Courier");
	}
	AG_SeparatorNewHoriz(win);
	{
		AG_LabelNewS(win, 0, "Data Search Paths:");
		AG_SLIST_FOREACH(cp, &agConfig->paths[AG_CONFIG_PATH_DATA], paths) {
			lbl = AG_LabelNewS(win, 0, cp->s);
			AG_SetStyle(lbl, "font-family", "Courier");
		}
	}
	AG_SeparatorNewHoriz(win);
	{
		AG_LabelNewS(win, 0, "Font Search Paths:");
		AG_SLIST_FOREACH(cp, &agConfig->paths[AG_CONFIG_PATH_FONTS], paths) {
			lbl = AG_LabelNewS(win, 0, cp->s);
			AG_SetStyle(lbl, "font-family", "Courier");
		}
	}
	AG_SeparatorNewHoriz(win);
	{
		AG_LabelNewS(win, 0, "Temporary directory:");
		AG_GetString(agConfig, "tmp-path", path, sizeof(path));
		lbl = AG_LabelNewS(win, 0, path);
		AG_SetStyle(lbl, "font-family", "Courier");
	}
	return (0);
}

const AG_TestCase fsPathsTest = {
	"fsPaths",
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

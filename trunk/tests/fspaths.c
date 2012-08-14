/*	Public domain	*/

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];

	AG_GetString(agConfig, "home", path, sizeof(path));
	AG_LabelNew(win, 0, "home: %s", path);
	AG_SeparatorNewHoriz(win);
	AG_GetString(agConfig, "load-path", path, sizeof(path));
	AG_LabelNew(win, 0, "load-path: %s", path);
	AG_GetString(agConfig, "save-path", path, sizeof(path));
	AG_LabelNew(win, 0, "save-path: %s", path);
	AG_GetString(agConfig, "tmp-path", path, sizeof(path));
	AG_LabelNew(win, 0, "tmp-path: %s", path);
	AG_SeparatorNewHoriz(win);
	AG_GetString(agConfig, "font-path", path, sizeof(path));
	AG_LabelNew(win, 0, "font-path: %s", path);

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

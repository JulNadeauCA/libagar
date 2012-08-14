/*	Public domain	*/

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_LabelNew(win, 0, "Minimal!");
	return (0);
}

const AG_TestCase minimalTest = {
	"minimal",
	N_("Display a minimal AG_Window(3)"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

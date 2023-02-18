/*	Public domain	*/

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_LabelNew(win, 0, "Minimal!");
	return (0);
}

const AG_TestCase minimalTest = {
	AGSI_IDEOGRAM AGSI_SMALL_WINDOW AGSI_RST,
	"minimal",
	N_("Display a minimal AG_Window(3)"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

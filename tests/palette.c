/*	Public domain	*/

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_HSVPalNew(win, AG_HSVPAL_EXPAND);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	return (0);
}

const AG_TestCase paletteTest = {
	"palette",
	N_("Test the color picker widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

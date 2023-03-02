/*	Public domain	*/
/*
 * Demonstrate use of M_Bezier to draw a bezier curve.
 */

#include "agartest.h"
#include "bezier_widget.h"

static int inited = 0;

static int
Init(void *obj)
{
	if (inited++ == 0) {
		AG_RegisterClass(&myBezierWidgetClass);
	}
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MY_BezierWidget *bez;

	bez = MY_BezierWidgetNew(win, NULL);
	bez->ti = obj;
	AG_Expand(bez);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	return (0);
}

const AG_TestCase bezierTest = {
	AGSI_IDEOGRAM AGSI_BEZIER AGSI_RST,
	"bezier",
	N_("Test B\xc3\xa9zier curve drawing"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

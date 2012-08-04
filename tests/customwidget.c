/*	Public domain	*/
/*
 * Demonstrate use of the custom widget defined in mywidget.c.
 */

#include "agartest.h"
#include "customwidget_mywidget.h"

static int inited = 0;

static int
Init(void *obj)
{
	if (inited++ > 0) {
		return (0);
	}
	AG_RegisterClass(&myWidgetClass);
	return (0);
}

static void
Destroy(void *obj)
{
	if (--inited > 0) {
		return;
	}
	AG_UnregisterClass(&myWidgetClass);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyWidget *my;

	my = MyWidgetNew(win, "foo");
	my->ti = obj;
	AG_Expand(my);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	return (0);
}

const AG_TestCase customWidgetTest = {
	"customWidget",
	N_("Test registering a custom Agar widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI
};

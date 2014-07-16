/*	Public domain	*/
/*
 * Test modal window dialog behavior.
 */

#include "agartest.h"

static void
CreateWindow(AG_Event *event)
{
	AG_Window *win, *winParent = AG_PTR(1);
	int count = AG_INT(2);

	if ((win = AG_WindowNew(AG_WINDOW_MODAL)) == NULL) {
		return;
	}
	AG_LabelNew(win, 0, "Modal window #%d\n(Parent = %s)", count++,
	    AGOBJECT(winParent)->name);
	AG_ButtonNewFn(win, 0, "Create another",
	    CreateWindow, "%p,%i", win, count++);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Event ev;

	AG_LabelNewS(win, 0, "Creating a test modal window...");
	AG_EventArgs(&ev, "%p,%i", win, 1);
	CreateWindow(&ev);
	return (0);
}

const AG_TestCase modalWindowHandlerTest = {
	"modalWindowHandler",
	N_("Test behavior of modal dialog windows"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

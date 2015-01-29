/*	Public domain	*/
/*
 * Test modal window dialog behavior.
 */

#include "agartest.h"

static int count = 0;

static void
CreateWindow(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1);
	AG_Window *win;

	if ((win = AG_WindowNew(AG_WINDOW_MODAL)) == NULL) {
		return;
	}
	AG_LabelNew(win, 0, "Modal window #%d,\nChild of %s",
	    count++,
	    AGOBJECT(winParent)->name);
	AG_ButtonNewFn(win, 0, "Create Child Window",
	    CreateWindow, "%p", win);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ButtonNewFn(win, 0, "Create Modal Window",
	    CreateWindow, "%p", win);
	count = 0;
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

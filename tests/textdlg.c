/*	Public domain	*/
/*
 * This demo tests the various canned dialogs provided by AG_Text(3).
 */

#include "agartest.h"

char someString[256];
double v = 10.0;

static void
TestCanned(AG_Event *event)
{
	AG_TextWarning("my-warning-key", "This is a warning");
	AG_TextError("This is an error message");
	AG_TextTmsg(AG_MSG_INFO, 3000, "This is a timed message");
	AG_TextMsg(AG_MSG_INFO, "This is an informational message");
	AG_TextInfo("infomsg",
	    "This is an informational message with \n"
	    "multiple lines");
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ButtonNewFn(win, 0, "Test canned dialogs", TestCanned, "%p", win);
	return (0);
}

const AG_TestCase textdlgTest = {
	"textdlg",
	N_("Test canned dialogs in AG_Text(3)"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

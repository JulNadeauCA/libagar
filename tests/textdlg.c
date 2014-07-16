/*	Public domain	*/
/*
 * This demo tests the various canned dialogs provided by AG_Text(3).
 */

#include "agartest.h"

char someString[256];
double v = 10.0;

static void
TestPrompts(AG_Event *event)
{
	/* Edit an existing floating-point variable. */
	AG_TextEditFloat(&v, 0.0, 100.0, "cm", "Edit a float value: ");
	
	/* Edit a fixed-size string buffer. */
	AG_Strlcpy(someString, "Test string", sizeof(someString));
	AG_TextEditString(someString, sizeof(someString), "Edit a string: ");
}

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
	AG_ButtonNewFn(win, 0, "Test prompts", TestPrompts, "%p", win);
	AG_ButtonNewFn(win, 0, "Test canned dialogs", TestCanned, "%p", win);
	return (0);
}

const AG_TestCase textDlgTest = {
	"textDlg",
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

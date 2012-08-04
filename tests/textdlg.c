/*	Public domain	*/
/*
 * This demo tests the various canned dialogs provided by AG_Text(3).
 */

#include "agartest.h"

char someString[256];
double v = 10.0;

static void
EnteredString(AG_Event *event)
{
	char *s = AG_STRING(1);

	AG_TextInfo("got-string", "Got string \"%s\"", s);
}

static void
TestPrompts(AG_Event *event)
{
	/* Prompt for a series of options. */
	{
		AG_Button *btns[3];

		btns[0] = AG_ButtonNewS(NULL, 0, NULL);
		btns[1] = AG_ButtonNewS(NULL, 0, NULL);
		btns[2] = AG_ButtonNewS(NULL, 0, NULL);
		AG_TextPromptOptions(btns, 3, "Multiple-choice selection: ");
		AG_ButtonText(btns[0], "Yes");
		AG_ButtonText(btns[1], "No");
		AG_ButtonText(btns[2], "Maybe");
	}
	
	/* Edit an existing floating-point variable. */
	AG_TextEditFloat(&v, 0.0, 100.0, "cm", "Edit a float value: ");
	
	/* Prompt for a string and invoke callback on return. */
	AG_TextPromptString("Prompt for a string: ", EnteredString, NULL);
	
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
	    "This is an informational message. Multiline text is "
	    "always allowed. Text will be wrapped to multiple lines "
	    "if it cannot be displayed properly on a single line.");
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
	TestGUI
};

/*	Public domain	*/
/*
 * Test modal window dialog behavior.
 */

#include "agartest.h"

static void
PopupReact(AG_Event *event)
{
	char s[128];
	int count = AG_INT(1);

	AG_Snprintf(s, sizeof(s), "This is the #%d popup prompt.",
	    count++);
	AG_TextPromptString(s, PopupReact, "%i", count++);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_LabelNew(win, 0, "Calling AG_TextPromptString(3)...");
	AG_LabelNew(win, 0, "Select \"Cancel\" to exit loop.");
	AG_TextPromptString("This is the first popup prompt.",
	    PopupReact, "%i", 2);
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

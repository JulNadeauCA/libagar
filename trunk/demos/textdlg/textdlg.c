/*	Public domain	*/
/*
 * This demo tests the various canned dialogs provided by AG_Text(3).
 */

#include <agar/core.h>
#include <agar/gui.h>

static void
EnteredString(AG_Event *event)
{
	char *s = AG_STRING(1);

	AG_TextInfo("Got string \"%s\"", s);
}

int
main(int argc, char *argv[])
{
	char s[256];
	double v = 10.0;

	/* Initialize Agar-GUI. */
	if (AG_InitCore("agar-textdlg-demo", 0) == -1 ||
	    AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	/* Prompt for a series of options. */
	{
		AG_Button *btns[3];

		btns[0] = AG_ButtonNew(NULL, 0, NULL);
		btns[1] = AG_ButtonNew(NULL, 0, NULL);
		btns[2] = AG_ButtonNew(NULL, 0, NULL);
		AG_TextPromptOptions(btns, 3, "Multiple-choice selection: ");
		AG_ButtonText(btns[0], "Yes");
		AG_ButtonText(btns[1], "No");
		AG_ButtonText(btns[2], "Maybe");
	}
	
	/* Prompt for a string and invoke callback on return. */
	AG_TextPromptString("Prompt for a string: ", EnteredString, NULL);

	/* Various canned dialogs as described in AG_Text(3). */
	AG_TextMsg(AG_MSG_INFO, "This is an informational message");
	AG_TextWarning("my-warning-key", "This is a warning");
	AG_TextError("This is an error message");
	AG_TextInfo("This is an informational message");
	AG_TextTmsg(AG_MSG_INFO, 3000, "This is a timed message");

	/* Edit an existing floating-point variable. */
	AG_TextEditFloat(&v, 0.0, 100.0, "cm", "Edit a float value: ");

	/* Edit an existing fixed-size string buffer. */
	AG_Strlcpy(s, "Test string", sizeof(s));
	AG_TextEditString(s, sizeof(s), "Edit a string: ");

	/* Use the standard Agar event loop. */
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

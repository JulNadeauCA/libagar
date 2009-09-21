/*	Public domain	*/
/*
 * This demonstrates the use of AG_Variable(3) with the agConfig object
 * for saving general configuration settings.
 */

#include <agar/core.h>
#include <agar/gui.h>

int someInt = 1234;
int someBool = 0;
char someString[64];

static void
SaveOnExit(void)
{
	printf("Saving configuration\n");
	AG_ConfigSave();
}

static void
LoadConfig(AG_Event *event)
{
	printf("Loading configuration\n");
	AG_ConfigLoad();
}

static void
SaveConfig(AG_Event *event)
{
	printf("Saving configuration\n");
	AG_ConfigSave();
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Box *box;
	AG_Textbox *tb;
	int i;

	someString[0] = '\0';

	if (AG_InitCore("agar-configsettings-demo", 0) == -1) {
		printf("AG_InitCore(%d): %s\n", i, AG_GetError());
		exit(1);
	}
	if (AG_InitVideo(320, 100, 32, AG_VIDEO_RESIZABLE) == -1) {
		printf("AG_InitVideo(%d): %s\n", i, AG_GetError());
		exit(1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	
	/* Tie some globals to the config settings */
	AG_BindInt(agConfig, "some-int", &someInt);
	AG_BindInt(agConfig, "some-bool", &someBool);
	AG_BindString(agConfig, "some-string", someString, sizeof(someString));
	AG_SetInt(agConfig, "some-int", 2345);

	/* Create some widgets */
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetPadding(win, 10, 10, 10, 10);
	AG_NumericalNewInt(win, 0, NULL, "Some int: ", &someInt);
	AG_CheckboxNewInt(win, 0, "Some bool", &someBool);
	tb = AG_TextboxNew(win, 0, "Some string: ");
	AG_TextboxBindUTF8(tb, someString, sizeof(someString));
	AG_ExpandHoriz(tb);

	box = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	{
		AG_ButtonNewFn(box, 0, "Load configuration", LoadConfig, NULL);
		AG_ButtonNewFn(box, 0, "Save configuration", SaveConfig, NULL);
	}

	AG_WindowMaximize(win);
	AG_WindowShow(win);

	AG_AtExitFunc(SaveOnExit);
	AG_EventLoop();
	AG_Quit();
	return (0);
}

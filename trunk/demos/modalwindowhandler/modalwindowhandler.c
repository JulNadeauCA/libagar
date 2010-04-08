/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>

void
PopupReact(AG_Event *event)
{
	char s[128];
	int count = AG_INT(1);

	AG_Snprintf(s, sizeof(s), "This is the #%d popup prompt.",
	    count++);
	AG_TextPromptString(s, PopupReact, "%i", count++);
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: modalwindowhandler [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-windowhandler-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);

	AG_TextPromptString("This is the first popup prompt.",
	    PopupReact, "%i", 2);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

/* Minimal application using Agar-GUI */

#include <agar/core.h>
#include <agar/gui.h>

#include <stdio.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;
	char *optArg, *driver = NULL;
	int optInd;
	int c;

	while ((c = AG_Getopt(argc, argv, "d:?h", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'd':
			driver = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: agarhello [-d driver]\n");
			return (1);
		}

	}

	/* Initialize ag_core and ag_gui. */
	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(driver) == -1)
		return (1);

	/* Configure standard hotkeys for zooming and terminating application. */
	AG_BindStdGlobalKeys();

	/* Create a new window with a label and close button. */
	win = AG_WindowNew(AG_WINDOW_MAIN);
	AG_SetStyle(win, "font-size", "250%");
	AG_LabelNew(win, 0, "Hello, world!");
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Bye", AGWINDETACH(win));
	AG_WindowShow(win);

	/* Loop processing events. */
	AG_EventLoop();
	return (0);
}

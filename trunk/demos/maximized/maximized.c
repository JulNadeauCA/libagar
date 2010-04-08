/*	Public domain	*/
/*
 * This application displays a maximized window with no titlebar or borders,
 * and with a padding setting of 0.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_Label *lbl;
	AG_Table *table;
	int i;

	/* Create a window without titlebar or decorations. */
	win = AG_WindowNew(agDriverSw ? AG_WINDOW_PLAIN : 0);
	AG_WindowSetCaption(win, "Agar maximized window demo");

	/* Disable the default spacing at the window's edges. */
	AG_WindowSetPadding(win, 0, 0, 0, 0);

	lbl = AG_LabelNew(win, AG_LABEL_HFILL, "Agar maximized window demo");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	AG_SpacerNewHoriz(win);

	/* Create an example table. */
	table = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableAddCol(table, "Foo", "<8888>", NULL);
	AG_TableAddCol(table, "Bar", NULL, NULL);
	for (i = 0; i < 100; i++) {
		AG_TableAddRow(table, "%d:%s", i, "Foo");
	}

	/* Resize the window to cover the whole view. */
	AG_WindowMaximize(win);

	/* The window is ready to be displayed. */
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
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
			printf("Usage: maximized [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-maximized-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


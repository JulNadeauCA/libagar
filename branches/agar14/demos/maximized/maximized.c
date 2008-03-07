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
	AG_Table *table;
	int i;

	/* Create a window without titlebar or decorations. */
	win = AG_WindowNew(AG_WINDOW_PLAIN);

	/* Disable the default spacing at the window's edges. */
	AG_WindowSetPadding(win, 0, 0, 0, 0);

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
	if (AG_InitCore("agar-maximized-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


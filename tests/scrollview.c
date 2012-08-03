/*	Public domain	*/
/*
 * This application demonstrates the use of the AG_Scrollview(3) widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Scrollview *sv;
	int x, y;
	AG_Box *hBox;
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
			printf("Usage: scrollview [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	/* Test with focusable widgets. */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Scrollview (reactive widgets)");
	AG_ButtonNew(win, 0, "Foo");
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND);
	for (y = 0; y < 20; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		AG_BoxSetSpacing(hBox, 1);
		AG_BoxSetPadding(hBox, 0);
		for (x = 0; x < 20; x++)
			AG_ButtonNew(hBox, 0, "Foo\n%c\n%d", (char)(0x41+x), y);
	}
	AG_ButtonNew(win, 0, "Bar");
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 400, 300);
	AG_WindowShow(win);

	/* Test with passive widgets. */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Scrollview (passive widgets)");
	AG_LabelNewS(win, 0, "Use middle mouse button to pan");
	AG_SeparatorNewHoriz(win);
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND|AG_SCROLLVIEW_BY_MOUSE);
	for (y = 0; y < 20; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		for (x = 0; x < 10; x++) {
			AG_LabelNewS(hBox, 0, "Foo");
			AG_LabelNewS(hBox, 0, "Bar");
		}
	}
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	AG_WindowShow(win);
	
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


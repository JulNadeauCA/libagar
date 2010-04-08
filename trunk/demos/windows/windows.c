/*	Public domain	*/
/*
 * Test various Window configurations.
 */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;
	char *driverSpec = NULL, *optArg;
	int c, i;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: windows [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-windows-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	
	for (i = 0; i < 5; i++) {
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "TL%d", i);
		AG_LabelNew(win, 0, "Top Left %d", i);
		AG_WindowSetPosition(win, AG_WINDOW_TL, 1);
		AG_WindowShow(win);
		
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "TR%d", i);
		AG_LabelNew(win, 0, "Top Right %d", i);
		AG_WindowSetPosition(win, AG_WINDOW_TR, 1);
		AG_WindowShow(win);
		
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "MC%d", i);
		AG_LabelNew(win, 0, "Center%d", i);
		AG_WindowSetPosition(win, AG_WINDOW_MC, 1);
		AG_WindowShow(win);
		
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "BL%d", i);
		AG_LabelNew(win, 0, "Bottom Left %d", i);
		AG_WindowSetPosition(win, AG_WINDOW_BL, 1);
		AG_WindowShow(win);
		
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "BR%d", i);
		AG_LabelNew(win, 0, "Bottom Right %d", i);
		AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
		AG_WindowShow(win);
		
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "BC%d", i);
		AG_LabelNew(win, 0, "Bottom Center %d", i);
		AG_WindowSetPosition(win, AG_WINDOW_BC, 1);
		AG_WindowShow(win);
	}
	
	win = AG_WindowNew(AG_WINDOW_NOMOVE|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOBUTTONS);
	AG_WindowSetCaption(win, "Unmovable");
	AG_LabelNew(win, 0, "Unmovable window");
	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);
	AG_WindowShow(win);
	
	win = AG_WindowNew(AG_WINDOW_KEEPABOVE);
	AG_WindowSetCaption(win, "KEEPABOVE");
	AG_LabelNew(win, 0, "KEEPABOVE");
	AG_WindowShow(win);
	
	win = AG_WindowNew(AG_WINDOW_KEEPBELOW);
	AG_WindowSetCaption(win, "KEEPBELOW");
	AG_LabelNew(win, 0, "KEEPBELOW");
	AG_WindowShow(win);
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Tweaked borders");
	AG_LabelNew(win, 0, "Window with tweaked borders");
	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);
	AG_WindowSetSideBorders(win, 10);
	AG_WindowSetBottomBorder(win, 10);
	AG_WindowShow(win);

	{
		int i;
		for (i = 0; i < 5; i++) {
			if ((win = AG_WindowNewNamed(0, "foo"))) {
				AG_WindowSetCaption(win, "Named");
				AG_LabelNew(win, 0, "Named window");
				AG_WindowShow(win);
			}
		}
	}

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


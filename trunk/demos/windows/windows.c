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
	int i;
	
	if (AG_InitCore("agar-windows-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

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

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


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
	int flags = AG_VIDEO_RESIZABLE;
	int i;
	
	if (AG_InitCore("agar-windows-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (argc > 1) {
		if (strcmp(argv[1], "-g") == 0) {
			flags |= AG_VIDEO_OPENGL;
		} else if (strcmp(argv[1], "-G") == 0) {
			AG_SetBool(agConfig, "view.opengl", 0);
		}
	}
	if (AG_InitVideo(640, 480, 32, flags) == -1) {
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
	
	win = AG_WindowNew(AG_WINDOW_NOMOVE|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOBUTTONS);
	AG_WindowSetCaption(win, "Unmovable");
	AG_LabelNew(win, 0, "Unmovable window", i);
	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);
	AG_WindowShow(win);
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Tweaked borders");
	AG_LabelNew(win, 0, "Window with tweaked borders", i);
	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);
	AG_WindowSetSideBorders(win, 10);
	AG_WindowSetBottomBorder(win, 10);
	AG_WindowShow(win);


	AG_EventLoop();
	AG_Destroy();
	return (0);
}


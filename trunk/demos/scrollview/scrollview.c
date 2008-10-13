/*	Public domain	*/
/*
 * This application demonstrates the use of the AG_Scrollview(3) widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	int guiFlags = AG_VIDEO_RESIZABLE;
	AG_Window *win;
	AG_Scrollview *sv;
	int x, y;
	AG_Box *hBox;
	
	if (AG_InitCore("agar-scrollview-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	if (argc > 1) {
		if (strcmp(argv[1], "-g") == 0) {
			printf("Forcing GL mode\n");
			AG_SetBool(agConfig, "view.opengl", 1);
			guiFlags |= AG_VIDEO_OPENGL;
		} else if (strcmp(argv[1], "-G") == 0) {
			printf("Forcing FB mode\n");
			AG_SetBool(agConfig, "view.opengl", 0);
		}
	}
	
	if (AG_InitVideo(640, 480, 32, guiFlags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	win = AG_WindowNew(0);


	AG_ButtonNew(win, 0, "Foo");
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND);

	/* Create a bunch of buttons under the Scrollview. */
	for (y = 0; y < 20; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		AG_BoxSetSpacing(hBox, 1);
		AG_BoxSetPadding(hBox, 0);
		for (x = 0; x < 20; x++)
			AG_ButtonNew(hBox, 0, "Foo\n%c\n%d", (char)(0x41+x), y);
	}
	AG_ButtonNew(win, 0, "Bar");

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 50, 50);
	AG_WindowShow(win);
	
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


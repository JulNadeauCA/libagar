/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-boxpacking-demo", 0) == -1 ||
	    AG_InitVideo(350, 200, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);

	AG_Window *win;
	AG_Textbox *tb;
	AG_Box *hBox;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	hBox = AG_BoxNewHoriz(win, 0);

	tb = AG_TextboxNew(hBox, 0, NULL);
	AG_TextboxSizeHint(tb, "<XXXXXXXXX>");
	AG_TextboxPrintf(tb, "Foo");

	tb = AG_TextboxNew(hBox, 0, NULL);
	AG_TextboxSizeHint(tb, "<XXXXXXXXX>");
	AG_TextboxPrintf(tb, "Bar");

	tb = AG_TextboxNew(hBox, 0, NULL);
	AG_TextboxSizeHint(tb, "<XXXXXXXXX>");
	AG_TextboxPrintf(tb, "Baz");

	AG_WindowMaximize(win);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

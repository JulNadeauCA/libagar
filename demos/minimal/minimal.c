/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;

	if (AG_InitCore("agar-minimal-demo", 0) == -1 ||
	    AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_LabelNew(win, 0, "Minimal!");
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

/* Minimal application using Agar-GUI */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;

	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(0) == -1)
		return (1);
	
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);

	win = AG_WindowNew(AG_WINDOW_MAIN);
	AG_SetStyle(win, "font-size", "250%");
	AG_LabelNew(win, 0, "Hello, world!");
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Bye", AGWINDETACH(win));
	AG_WindowShow(win);

	AG_EventLoop();
	return (0);
}

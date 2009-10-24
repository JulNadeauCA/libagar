/*	Public domain	*/
/*
 * This application demonstrates a simple way to catch and process arbitrary
 * keyboard events.
 */

#include <agar/core.h>
#include <agar/gui.h>

/*
 * This is our keyup/keydown event handler. The arguments are documented in
 * the EVENTS section of the AG_Window(3) manual page. See AG_KeySym(3) for
 * a list of available key symbols.
 */
static void
MyKeyboardHandler(AG_Event *event)
{
	int sym = AG_INT(1);
	int mod = AG_INT(2);
	Uint32 unicode = (Uint32)AG_ULONG(3);

	printf("%s: sym=%d, modifier=0x%x, unicode=0x%lx\n",
	    event->name, sym, (unsigned)mod, unicode);
}

static void
Quit(AG_Event *event)
{
	AG_Quit();
}

static void
CreateWindow(void)
{
	AG_Window *win;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_LabelNew(win, 0, "Press any key.");

	/*
	 * Attach our event handler function to both keydown and keyup
	 * events of the Window object. Note that we could have used
	 * any other object derived from the Widget class.
	 */
	AG_SetEvent(win, "key-down", MyKeyboardHandler, NULL);
	AG_SetEvent(win, "key-up", MyKeyboardHandler, NULL);
	
	/*
	 * Enable reception of keydown/keyup events by the window, regardless
	 * of whether it is currently focused or not.
	 */
	AGWIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_KEYUP;
	AGWIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_KEYDOWN;

	AG_ButtonNewFn(win, 0, "Quit", Quit, NULL);

	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-keyevents-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

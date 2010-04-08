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
	AG_Console *cons = AG_PTR(1);
	int sym = AG_INT(2);
	int mod = AG_INT(3);
	Uint32 unicode = (Uint32)AG_ULONG(4);

	AG_ConsoleMsg(cons,
	    "%s: sym=%d, modifier=0x%x, unicode=0x%lx",
	    event->name, sym, (unsigned)mod, unicode);
}

static void
Quit(AG_Event *event)
{
	AG_QuitGUI();
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_Label *lbl;
	AG_Console *cons;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar keyboard events demo");
	lbl = AG_LabelNew(win, AG_LABEL_HFILL, "Agar keyboard events demo");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	AG_SeparatorNewHoriz(win);

	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	AG_ConsoleMsg(cons, "Press any key...");

	/*
	 * Attach our event handler function to both keydown and keyup
	 * events of the Window object. Note that we could have used
	 * any other object derived from the Widget class.
	 */
	AG_SetEvent(win, "key-down", MyKeyboardHandler, "%p", cons);
	AG_SetEvent(win, "key-up", MyKeyboardHandler, "%p", cons);
	
	/*
	 * Enable reception of keydown/keyup events by the window, regardless
	 * of whether it is currently focused or not.
	 */
	AGWIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_KEYUP;
	AGWIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_KEYDOWN;

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", Quit, NULL);

	AG_WindowShow(win);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
}

int
main(int argc, char *argv[])
{
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
			printf("Usage: keyevents [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-keyevents-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

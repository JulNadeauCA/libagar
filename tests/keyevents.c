/*	Public domain	*/
/*
 * This application demonstrates a simple way to catch and process arbitrary
 * keyboard events.
 */

#include "agartest.h"

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
	    event->name, sym, (unsigned)mod, (long unsigned int)unicode);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Label *lbl;
	AG_Console *cons;

	lbl = AG_LabelNew(win, AG_LABEL_HFILL, "Low-level keyboard events test");
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

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
	return (0);
}

const AG_TestCase keyEventsTest = {
	"keyEvents",
	N_("Test low-level keyboard input"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

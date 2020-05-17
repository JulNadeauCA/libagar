/*	Public domain	*/
/*
 * This application demonstrates a simple way to catch and process arbitrary
 * keyboard events.
 */

#include "agartest.h"

#include <ctype.h>

/*
 * This is our keyup/keydown event handler. The arguments are documented in
 * the EVENTS section of the AG_Window(3) manual page. See AG_KeySym(3) for
 * a list of available key symbols.
 */
static void
MyKeyboardHandler(AG_Event *event)
{
	AG_Console *cons = AG_CONSOLE_PTR(1);
	const int sym = AG_INT(2);
	const int mod = AG_INT(3);
	const AG_Char ch = AG_CHAR(4);
	char pc[8], psym[16];

	/* Format the character for printing */
	pc[0] = '\0';
	if      (ch == '\n') { Strlcpy(pc, "\\n", sizeof(pc)); }
	else if (ch == '\r') { Strlcpy(pc, "\\r", sizeof(pc)); }
	else if (ch == '\t') { Strlcpy(pc, "\\t", sizeof(pc)); }
	else if (isprint(ch)) { Snprintf(pc, sizeof(pc), "'%c'", (char)ch); }
	else                  { Snprintf(pc, sizeof(pc), "U+%lx", (Ulong)ch); }

	/* Agar provides a table of strings for AG_KeySym values. */
	if (sym < agKeySymCount) {
		Snprintf(psym, sizeof(psym), "[%10s]",
		    (agKeySyms[sym] != NULL) ? agKeySyms[sym] : "null");
	} else {
		Snprintf(psym, sizeof(psym), "0x%04x", sym);
	}

#ifdef AG_UNICODE
	AG_ConsoleMsg(cons,
	    "%9s: sym=%s [%03u] mod=0x%04x ch=0x%lx (%s)",
	    event->name, psym, sym, (Uint)mod, (Ulong)ch, pc);
#else
	AG_ConsoleMsg(cons,
	    "%9s: sym=%s [%03u] mod=0x%04x, ch=%d (%s)",
	    event->name, psym, sym, (Uint)mod, ch, pc);
#endif
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Label *lbl;
	AG_Console *cons;

	lbl = AG_LabelNew(win, AG_LABEL_HFILL, "Keyboard Events");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	AG_SetStyle(lbl, "font-family", "cm-typewriter");
	AG_SetStyle(lbl, "font-weight", "bold");
	AG_SetStyle(lbl, "font-size", "200%");
	AG_SetStyle(lbl, "text-color", "AntiqueWhite");

	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	AG_SetStyle(cons, "font-family", "courier-prime");
	AG_SetStyle(cons, "background-color", "#333");
	AG_SetStyle(cons, "text-color", "AntiqueWhite");
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

const AG_TestCase keyeventsTest = {
	"keyevents",
	N_("Test low-level keyboard input"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

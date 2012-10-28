/*	Public domain	*/
/*
 * This application tests the AG_Console widget.
 */

#include "agartest.h"

AG_Textbox *textbox;

static void
AppendLine(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	char *s;
	
	s = AG_TextboxDupString(textbox);
	AG_ConsoleMsgS(cons, s);
	AG_TextboxSetString(textbox, "");
	Free(s);
}

static void
EnterJunk(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	int i;

	for (i = 0; i <= 100; i++) {
		AG_ConsoleMsg(cons, "%d/%d foo bar baz bezo foo bar baz "
		                    "bezo foo bar baz bezo foo bar baz bezo",
		    i, 100);
	}
}

static void
ClearLines(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);

	AG_ConsoleClear(cons);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Console *cons;
	AG_Box *box;
	AG_Button *btn;

	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		textbox = AG_TextboxNew(box, AG_TEXTBOX_EXCL|AG_TEXTBOX_HFILL,
		    "Input: ");
		AG_SetEvent(textbox, "textbox-return", AppendLine, "%p", cons);
		AG_WidgetFocus(textbox);

		btn = AG_ButtonNewFn(box, 0, "OK", AppendLine, "%p", cons);
		AG_WidgetSetFocusable(btn, 0);
	}
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(box, 0, "Junk", EnterJunk, "%p", cons);
		AG_ButtonNewFn(box, 0, "Clear", ClearLines, "%p", cons);
		AG_ButtonNewFlag(box, AG_BUTTON_STICKY, "Debug",
		    &cons->vBar->flags, AG_SCROLLBAR_TEXT);
	}
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
	return (0);
}

const AG_TestCase consoleTest = {
	"console",
	N_("Test the AG_Console(3) widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

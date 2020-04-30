/*	Public domain	*/
/*
 * This application tests the AG_Console widget.
 */

#include "agartest.h"

static void
AppendLine(AG_Event *event)
{
	AG_Console *cons = AG_CONSOLE_PTR(1);
	AG_Textbox *tb = AG_TEXTBOX_PTR(2);
	char *s;
	
	s = AG_TextboxDupString(tb);
	AG_ConsoleMsgS(cons, s);
	AG_TextboxSetString(tb, "");
	Free(s);
}

static void
Enter100Lines(AG_Event *event)
{
	const char *text = "The Quick Brown Fox Jumps Over The Lazy Dog";
	AG_Console *cons = AG_CONSOLE_PTR(1);
	int i;

	for (i = 0; i <= 100; i++)
		AG_ConsoleMsg(cons, "%d) %s", i, text);
}

static void
EnterMultiLine(AG_Event *event)
{
	AG_Console *cons = AG_CONSOLE_PTR(1);
	AG_Color red;
	AG_ColorRGB_8(&red, 255, 16, 16);

	AG_ConsoleMsgS(cons, "This message is all on one line");
	AG_ConsoleMsgS(cons, "This message is on two lines\nsecond line");
	AG_ConsoleMsgS(cons, "This message is on five lines\n2\n3\n4\n5");

	AG_ConsoleMsgColor(
		AG_ConsoleMsgS(cons,
			"Multi-line messages\ntake the color\nof their parent"),
		&red);
}

static void
DumpCoreOf(AG_Event *event)
{
	AG_Console *cons = AG_CONSOLE_PTR(1);
	AG_Object *obj = AG_OBJECT_PTR(2);
	AG_Size objSize = AG_UINT(3);

	AG_ConsoleBinary(cons, obj, objSize, obj->name, NULL);
}

static void
FollowFile(AG_Event *event)
{
	AG_Button *btnOpen = AG_BUTTON_SELF();
	AG_Console *cons = AG_CONSOLE_PTR(1);
	char *file = AG_STRING(2);
	Uint flags = AG_UINT(3);
	char *label = AG_STRING(4);
	AG_ConsoleFile *cf;

	if (AG_Defined(cons, file)) {
		AG_ConsoleMsgS(cons, _("File is already open"));
		return;
	}
	if ((cf = AG_ConsoleOpenFile(cons, label, file, 0, flags)) == NULL) {
		AG_ConsoleMsgS(cons, AG_GetError());
		return;
	}
	AG_ConsoleMsg(cons, _("Opened %s (fd = %d)"), cf->label, cf->fd);
	AG_SetPointer(cons, file, cf);
	AG_WidgetDisable(btnOpen);
}

static void
CloseFile(AG_Event *event)
{
	AG_Console *cons = AG_CONSOLE_PTR(1);
	AG_Button *btnOpen = AG_BUTTON_PTR(2);
	char *file = AG_STRING(3);
	AG_ConsoleFile *cf;

	if (!AG_Defined(cons, file) ||
	    (cf = AG_GetPointer(cons, file)) == NULL) {
		return;
	}
	AG_ConsoleMsg(cons, _("Closing %s"), cf->label);
	AG_ConsoleClose(cons, cf);
	AG_Unset(cons, file);
	AG_WidgetEnable(btnOpen);
}

static void
ClearLines(AG_Event *event)
{
	AG_Console *cons = AG_CONSOLE_PTR(1);

	AG_ConsoleClear(cons);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Console *cons;
	AG_Box *box;
	AG_Button *btn;
	AG_Textbox *tb;

	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		tb = AG_TextboxNew(box,
		    AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL,
		    _("Input: "));
		AG_SetEvent(tb, "textbox-return", AppendLine, "%p,%p", cons, tb);
		AG_WidgetFocus(tb);

		btn = AG_ButtonNewFn(box, 0, "OK", AppendLine, "%p,%p", cons, tb);
		AG_WidgetSetFocusable(btn, 0);
	}

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	AG_SetStyle(box, "font-size", "80%");
	{
		AG_ButtonNewFn(box, 0, "Clear",
		    ClearLines, "%p", cons);
		AG_ButtonNewFn(box, 0, "100 Lines",
		    Enter100Lines, "%p", cons);
		AG_ButtonNewFn(box, 0, "Multi-Line",
		    EnterMultiLine, "%p", cons);
		AG_ButtonNewFn(box, 0, "Dump Own Core",
		    DumpCoreOf, "%p,%p,%u", cons, cons, sizeof(AG_Console));
		AG_ButtonNewFn(box, 0, "Dump Window's Core",
		    DumpCoreOf, "%p,%p,%u", cons, win, sizeof(AG_Window));
	}
#if !defined(__WIN32__)
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	AG_SetStyle(box, "font-size", "80%");
	{
		const char *msglog = "/var/log/messages";

		btn = AG_ButtonNewFn(box, 0, "Follow /var/log/messages",
		    FollowFile, "%p,%s,%u,%s", cons, msglog, 0, "messages");
		AG_ButtonNewFn(box, 0, "Close messages",
		    CloseFile, "%p,%p,%s", cons, btn, msglog);
	}
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
	AG_SetStyle(box, "font-size", "80%");
	{
		const char *utxlog = "/var/log/utx.log";

		btn = AG_ButtonNewFn(box, 0, "Follow /var/log/utx.log",
		    FollowFile, "%p,%s,%u,%s", cons, utxlog,
		                               AG_CONSOLE_FILE_BINARY, "utxlog");
		AG_ButtonNewFn(box, 0, "Close utx.log",
		    CloseFile, "%p,%p,%s", cons, btn, utxlog);
	}
#endif /* !_WIN32 */

	AG_WindowSetGeometryAligned(win, AG_WINDOW_BR, 640, 800);
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

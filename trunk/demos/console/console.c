/*	Public domain	*/
/*
 * This application tests the AG_Console widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

AG_Textbox *textbox;

static void
AppendLine(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	char *s;
	
	s = AG_TextboxDupString(textbox);
	AG_ConsoleMsgS(cons, s);
	AG_TextboxSetString(textbox, "");
	free(s);
}

static void
EnterJunk(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	AG_ConsoleLine *cl;
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

static void
SelFont(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	AG_Window *win;
	AG_FontSelector *fs;

	if ((win = AG_WindowNewNamedS(0, "selfont")) != NULL) {
		/* Duplicate the default font */
		cons->font = AG_TextFontPct(100);

		AG_WindowSetCaption(win, "Select font");
		fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
		AG_BindPointer(fs, "font", (void **)&cons->font);
		AG_WindowShow(win);
	}
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Console *cons;
	AG_Box *box;
	AG_Button *btn;
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
			printf("Usage: console [-d agar-driver-spec]\n");
			return (1);
		}
	}

	if (AG_InitCore("agar-console-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar console demo");
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		textbox = AG_TextboxNew(box, AG_TEXTBOX_STATIC|AG_TEXTBOX_HFILL,
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
		AG_ButtonNewFn(box, 0, "Sel.Font", SelFont, "%p", cons);
		AG_ButtonNewFlag(box, AG_BUTTON_STICKY, "Debug",
		    &cons->vBar->flags, AG_SCROLLBAR_TEXT);
	}
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

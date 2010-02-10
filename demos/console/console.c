/*	Public domain	*/
/*
 * This application tests the AG_Console widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

AG_Font *myFont;

static void
AddLines(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	AG_ConsoleLine *cl;
	int i, nLines = AG_INT(2);
	
	for (i = 0; i < nLines; i++) {
		cl = AG_ConsoleMsg(cons,
		    "%d%d Foo bar baz bezo papi grow mami smoke\nMush snow storm",
		    i, (int)AG_GetTicks());
		if (i%2) {
			cl->font = agDefaultFont;
			cl->cBg = AG_ColorRGB(0,100,0);
		} else {
			cl->font = myFont;
			cl->cBg = AG_ColorRGBA(0,0,0,0);
		}
	}
}

static void
ClearLines(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);

	AG_ConsoleClear(cons);
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Console *cons;
	AG_Box *box;

	if (AG_InitCore("agar-console-demo", 0) == -1 ||
	    AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	win = AG_WindowNew(agDriverSw ? AG_WINDOW_PLAIN : 0);
	AG_WindowSetCaption(win, "Agar console demo");
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	box = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(box, 0, "Add Line", AddLines, "%p,%i", cons, 1);
		AG_ButtonNewFn(box, 0, "Add 10 Lines", AddLines, "%p,%i", cons, 10);
		AG_ButtonNewFn(box, 0, "Clear", ClearLines, "%p", cons);
	}
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
	AG_WindowShow(win);

	myFont = AG_TextFontPct(200);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

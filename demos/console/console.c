/*	Public domain	*/
/*
 * This application tests the AG_Console widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void
AddLine(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	AG_ConsoleMsg(cons, "Test %d Test Test Test Test Test Test Test Test",
	    (int)AG_GetTicks());
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
	int i;

	if (AG_InitCore("agar-console-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	win = AG_WindowNew(0);
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(box, 0, "Add Line", AddLine, "%p", cons);
		AG_ButtonNewFn(box, 0, "Clear", ClearLines, "%p", cons);
	}
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

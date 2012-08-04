/*	Public domain	*/
/*
 * Test various AG_Window placements and configurations.
 */

#include "agartest.h"

static void
TestInitialAlign(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1);
	AG_Window *win;
	int i;
	
	AG_LabelNewS(winParent, 0, "Creating test windows...");

	for (i = 0; i < 5; i++) {
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "TL%d", i);
			AG_LabelNew(win, 0, "Top Left #%d", i);
			AG_LabelNewS(win, 0, "(AG_WINDOW_TL)");
			AG_WindowSetPosition(win, AG_WINDOW_TL, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "TR%d", i);
			AG_LabelNew(win, 0, "Top Right #%d", i);
			AG_LabelNewS(win, 0, "(AG_WINDOW_TR)");
			AG_WindowSetPosition(win, AG_WINDOW_TR, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "MC%d", i);
			AG_LabelNew(win, 0, "Center #%d", i);
			AG_LabelNewS(win, 0, "(AG_WINDOW_MC)");
			AG_WindowSetPosition(win, AG_WINDOW_MC, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "BL%d", i);
			AG_LabelNew(win, 0, "Bottom Left #%d", i);
			AG_LabelNewS(win, 0, "(AG_WINDOW_BL)");
			AG_WindowSetPosition(win, AG_WINDOW_BL, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "BR%d", i);
			AG_LabelNew(win, 0, "Bottom Right #%d", i);
			AG_LabelNewS(win, 0, "(AG_WINDOW_BR)");
			AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "BC%d", i);
			AG_LabelNew(win, 0, "Bottom Center #%d", i);
			AG_LabelNewS(win, 0, "(AG_WINDOW_BC)");
			AG_WindowSetPosition(win, AG_WINDOW_BC, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
	}
	
	AG_LabelNewS(winParent, 0, "OK");
}

static void
TestWmAttributes(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1);
	AG_Window *win;
	int i;

	if ((win = AG_WindowNew(AG_WINDOW_NOMOVE|AG_WINDOW_NORESIZE|
	    AG_WINDOW_NOBUTTONS))) {
		AG_WindowSetCaption(win, "Unmovable");
		AG_LabelNewS(win, 0, "Unmovable window");
		AG_WindowAttach(winParent, win);
		AG_WindowShow(win);
	}
	
	if ((win = AG_WindowNew(AG_WINDOW_KEEPABOVE))) {
		AG_WindowSetCaption(win, "KEEPABOVE");
		AG_LabelNewS(win, 0, "KEEPABOVE");
		AG_WindowAttach(winParent, win);
		AG_WindowShow(win);
	}
	if ((win = AG_WindowNew(AG_WINDOW_KEEPBELOW))) {
		AG_WindowSetCaption(win, "KEEPBELOW");
		AG_LabelNewS(win, 0, "KEEPBELOW");
		AG_WindowAttach(winParent, win);
		AG_WindowShow(win);
	}
	if ((win = AG_WindowNew(0))) {
		AG_WindowSetCaption(win, "Tweaked borders");
		AG_LabelNewS(win, 0, "Window with tweaked borders");
		AG_WindowSetSideBorders(win, 10);
		AG_WindowSetBottomBorder(win, 10);
		AG_WindowAttach(winParent, win);
		AG_WindowShow(win);
	}
	for (i = 0; i < 5; i++) {
		if ((win = AG_WindowNewNamed(0, "foo"))) {
			AG_WindowSetCaption(win, "Named");
			AG_LabelNewS(win, 0, "Named window");
			AG_WindowAttach(winParent, win);
			AG_WindowShow(win);
		}
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Test initial window alignment",
	    TestInitialAlign, "%p", win);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Test WM window attributes",
	    TestWmAttributes, "%p", win);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	return (0);
}

const AG_TestCase windowsTest = {
	"windows",
	N_("Test various AG_Window(3) placements and options"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};

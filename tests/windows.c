/*	Public domain	*/
/*
 * Test various AG_Window placements and configurations.
 */

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	Uint testFlags;
	int makeTransient;
	int makePinned;
} MyTestInstance;

static void
TestDesktopAlign(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1);
	AG_Window *win;
	int i;
	
	for (i = 0; i < 2; i++) {
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "Auto%d", i);
			AG_LabelNew(win, 0, "Autopositioned #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_ALIGNMENT_NONE)" AGSI_RST);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "TL%d", i);
			AG_LabelNew(win, 0, "Top Left #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_TL)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_TL, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "TC%d", i);
			AG_LabelNew(win, 0, "Top Center #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_TC)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_TC, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "TR%d", i);
			AG_LabelNew(win, 0, "Top Right #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_TR)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_TR, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "MC%d", i);
			AG_LabelNew(win, 0, "Center #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_MC)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_MC, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "BL%d", i);
			AG_LabelNew(win, 0, "Bottom Left #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_BL)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_BL, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "BR%d", i);
			AG_LabelNew(win, 0, "Bottom Right #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_BR)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
		if ((win = AG_WindowNew(0))) {
			AG_WindowSetCaption(win, "BC%d", i);
			AG_LabelNew(win, 0, "Bottom Center #%d", i);
			AG_LabelNewS(win, 0, AGSI_COURIER "(AG_WINDOW_BC)" AGSI_RST);
			AG_WindowSetPosition(win, AG_WINDOW_BC, 1);
			AG_WindowAttach(winParent, win);
			AG_WindowMakeTransient(winParent, win);
			AG_WindowShow(win);
		}
	}
}

static void
CreateTestWindow(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *winParent = AG_WINDOW_PTR(2);
	AG_Window *win;

	if ((win = AG_WindowNew(ti->testFlags)) == NULL) {
		TestMsg(ti, "AG_WindowNew() failed: %s", AG_GetError());
		return;
	}
	AG_WindowSetCaption(win, "Test window");
	AG_LabelNewS(win, 0, "This is a test window");
	AG_LabelNew(win, 0, "Flags = 0x%x", ti->testFlags);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Close this window", AGWINDETACH(win));
	AG_WindowAttach(winParent, win);

	if (ti->makeTransient)
		AG_WindowMakeTransient(winParent, win);
	if (ti->makePinned)
		AG_WindowPin(winParent, win);

	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_FlagDescr winFlags[] = {
		{ AG_WINDOW_MODAL,		"MODAL",	1 },
		{ AG_WINDOW_KEEPABOVE,		"KEEPABOVE",	1 },
		{ AG_WINDOW_KEEPBELOW,		"KEEPBELOW",	1 },
		{ AG_WINDOW_NOTITLE,		"NOTITLE",	1 },
		{ AG_WINDOW_NOBORDERS,		"NOBORDERS",	1 },
		{ AG_WINDOW_NOHRESIZE,		"NOHRESIZE",	1 },
		{ AG_WINDOW_NOVRESIZE,		"NOVRESIZE",	1 },
		{ AG_WINDOW_NOCLOSE,		"NOCLOSE",	1 },
		{ AG_WINDOW_NOMINIMIZE,		"NOMINIMIZE",	1 },
		{ AG_WINDOW_NOMAXIMIZE,		"NOMAXIMIZE",	1 },
		{ AG_WINDOW_NOBACKGROUND,	"NOBACKGROUND",	1 },
		{ AG_WINDOW_NOMOVE,		"NOMOVE",	1 },
		{ AG_WINDOW_DENYFOCUS,		"DENYFOCUS",	1 },
		{ 0,				NULL,		0 }
	};
	AG_Box *box;

	ti->testFlags = 0;
	ti->makeTransient = 0;
	ti->makePinned = 0;

	AG_LabelNewS(win, 0, "Create test window with flags:");
	
	box = AG_BoxNewVert(win, AG_BOX_EXPAND);
	AG_SetStyle(box, "font-family", "courier-prime");
	{
		AG_CheckboxSetFromFlags(box, 0, &ti->testFlags, winFlags);
	}

	box = AG_BoxNewVert(win, AG_BOX_HFILL);
	{
		AG_CheckboxNewInt(box, 0, "Make transient", &ti->makeTransient);
		AG_CheckboxNewInt(box, 0, "Make pinned", &ti->makePinned);
	}

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Create Test Window",
	    CreateTestWindow, "%p,%p", ti, win);

	AG_SeparatorNewHoriz(win);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Test Desktop Alignment",
	    TestDesktopAlign, "%p", win);

	return (0);
}

const AG_TestCase windowsTest = {
	"windows",
	N_("Test various AG_Window(3) placements and options"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

/*	Public domain	*/
/*
 * This application demonstrates the use of the AG_Scrollview(3) widget.
 */

#include "agartest.h"

static void
TestWithReactiveWidgets(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1), *win;
	AG_Box *hBox;
	AG_Scrollview *sv;
	int x, y;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_LabelNewS(win, 0, "AG_Scrollview(3) with reactive widgets");
	AG_ButtonNew(win, AG_BUTTON_EXCL, "Foo");
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND);
	for (y = 0; y < 20; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		AG_BoxSetSpacing(hBox, 1);
		AG_BoxSetPadding(hBox, 0);
		for (x = 0; x < 20; x++)
			AG_ButtonNew(hBox, AG_BUTTON_EXCL,
			    "Foo\n%c\n%d", (char)(0x41+x), y);
	}
	AG_ButtonNew(win, AG_BUTTON_EXCL, "Bar");
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 400, 300);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
TestWithPassiveWidgets(AG_Event *event)
{
	AG_Window *winParent = AG_PTR(1), *win;
	AG_Box *hBox;
	AG_Scrollview *sv;
	int x, y;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "Scrollview (passive widgets)");
	AG_LabelNewS(win, 0, "AG_Scrollview(3) with passive widgets");
	AG_LabelNewS(win, 0, "Panning enabled (middle mouse button)");
	AG_SeparatorNewHoriz(win);
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND|AG_SCROLLVIEW_BY_MOUSE);
	for (y = 0; y < 20; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		for (x = 0; x < 10; x++) {
			AG_LabelNewS(hBox, 0, "Foo");
			AG_LabelNewS(hBox, 0, "Bar");
		}
	}
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ButtonNewFn(win, 0, "Test with reactive widgets",
	    TestWithReactiveWidgets, "%p", win);
	AG_ButtonNewFn(win, 0, "Test with passive widgets",
	    TestWithPassiveWidgets, "%p", win);

	return (0);
}

const AG_TestCase scrollviewTest = {
	"scrollview",
	N_("Test the AG_Scrollview(3) widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

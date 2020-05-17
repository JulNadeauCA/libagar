/*	Public domain	*/
/*
 * This program tests different aspects of widget focusing behavior.
 */

#include "agartest.h"

static void
mousemotion(AG_Event *event)
{
	AG_Widget *w = AG_WIDGET_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	if (AG_WidgetSensitive(w, w->rView.x1+x, w->rView.y1+y)) {
		if ((w->flags & AG_WIDGET_FOCUSED) == 0) {
			/*
			 * Place the focus over the widget's parent window
			 * using AG_WindowFocus(). AG_WidgetFocus() places
			 * the focus over the given widget inside the parent
			 * window.
			 *
			 * Both of these calls are necessary since the widget
			 * focus is independent from the window focus.
			 */
			AG_WindowFocus(AG_ParentWindow(w));
			AG_WidgetFocus(w);
			Debug(w, "Focused\n");
		}
	} else {
		if (w->flags & AG_WIDGET_FOCUSED) {
			/*
			 * Remove the focus from the widget. To remove the
			 * focus from the window, you would have to call
			 * AG_WindowFocus() on another window (windows don't
			 * have any focus flag -- whichever window is on top
			 * of the window list holds focus).
			 */
			AG_WidgetUnfocus(w);
			Debug(w, "Unfocused\n");
		}
	}
}

static void
keydown(AG_Event *event)
{
#ifdef AG_DEBUG
	AG_Widget *w = AG_WIDGET_SELF();
	int kb = AG_INT(1);

	Debug(w, "key %d\n", kb);
#endif
}

static void
buttondown(AG_Event *event)
{
	AG_Widget *w = AG_WIDGET_SELF();

	AG_WidgetFocus(w);
}

static void
TestUnfocusedMotion(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1), *win;
	AG_Button *btn;
	AG_Fixed *fx1, *fx2;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "focusing: Unfocused mousemotion");
	AG_LabelNew(win, 0, "Mouse hover to gain focus");

	fx1 = AG_FixedNew(win, AG_FIXED_EXPAND);

	btn = AG_ButtonNew(fx1, 0, "Foo");
	AG_FixedMove(fx1, btn, 0, 0);
	AG_FixedSize(fx1, btn, 32, 32);
	btn = AG_ButtonNew(fx1, 0, "Bar");
	AG_FixedMove(fx1, btn, 0, 32);
	AG_FixedSize(fx1, btn, 32, 32);
	AG_WidgetDisable(btn);
	btn = AG_ButtonNew(fx1, 0, "Baz");
	AG_FixedMove(fx1, btn, 0, 64);
	AG_FixedSize(fx1, btn, 32, 32);

	fx2 = AG_FixedNew(fx1, 0);
	fx2->style = AG_FIXED_STYLE_BOX;
	AGWIDGET(fx2)->flags |= AG_WIDGET_FOCUSABLE |
		                AG_WIDGET_UNFOCUSED_MOTION;
	AG_FixedMove(fx1, fx2, 64, 16);
	AG_FixedSize(fx1, fx2, 200, 140);
	AG_SetEvent(fx2, "mouse-motion", mousemotion, NULL);
	AG_SetEvent(fx2, "key-down", keydown, NULL);
	AG_SetEvent(fx2, "mouse-button-down", buttondown, NULL);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
TestTabCycle(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1), *win;
	AG_Box *b, *b1, *b2;
	int i;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "focusing: Tab cycle");

	AG_LabelNew(win, 0, "<TAB> = Cycle focus forward\n"
	                    "<SHIFT+TAB> = Cycle focus backward");
	b = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS|AG_BOX_EXPAND);

	AG_ButtonNew(b, AG_BUTTON_HFILL, "Foo");

	b1 = AG_BoxNewVert(b, AG_BOX_VFILL);
	for (i = 0; i < 5; i++)
		AG_ButtonNew(b1, AG_BUTTON_HFILL, "#%d", i);

	b2 = AG_BoxNewVert(b, AG_BOX_VFILL);
	for (i = 5; i < 10; i++)
		AG_ButtonNew(b2, AG_BUTTON_HFILL, "#%d", i);

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_LabelNewS(win, 0, "Tests for widget focus states");
	AG_ButtonNewFn(win, 0, "Test unfocused mouse motion",
	    TestUnfocusedMotion, "%p", win);
	AG_ButtonNewFn(win, 0, "Test <tab> focus cycling",
	    TestTabCycle, "%p", win);
	return (0);
}

const AG_TestCase focusingTest = {
	"focusing",
	N_("Test widget focus state control"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

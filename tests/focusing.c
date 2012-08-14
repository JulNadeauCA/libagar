/*	Public domain	*/
/*
 * This program tests different aspects of widget focusing behavior.
 */

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	AG_Widget *widget1, *widget2;
} MyTestInstance;

static void
mousemotion(AG_Event *event)
{
	AG_Widget *w = AG_SELF();
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
			printf("%s: focused\n", AGOBJECT(w)->name);
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
			printf("%s: unfocused\n", AGOBJECT(w)->name);
		}
	}
}

static void
keydown(AG_Event *event)
{
	AG_Widget *w = AG_SELF();
	int kb = AG_INT(1);

	printf("%s: key %d\n", AGOBJECT(w)->name, kb);
}

static void
buttondown(AG_Event *event)
{
	AG_Widget *w = AG_SELF();

	AG_WidgetFocus(w);
}

static void
TestUnfocusedMotion(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *winParent = AG_PTR(2), *win;
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

	ti->widget1 = AGWIDGET(btn);

	fx2 = AG_FixedNew(fx1, AG_FIXED_BOX);
	AGWIDGET(fx2)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION;
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
	MyTestInstance *ti = AG_PTR(1);
	AG_Window *winParent = AG_PTR(2), *win;
	AG_Box *b, *b1, *b2;
	AG_Button *btn;
	int i;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "focusing: Tab cycle");

	AG_LabelNew(win, 0, "<TAB> = Cycle focus forward\n"
	                    "<SHIFT+TAB> = Cycle focus backward");
	b = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS|AG_BOX_EXPAND);

	btn = AG_ButtonNew(b, AG_BUTTON_HFILL, "Foo");
	ti->widget2 = AGWIDGET(btn);

	b1 = AG_BoxNewVert(b, AG_BOX_VFILL);
	for (i = 0; i < 5; i++)
		AG_ButtonNew(b1, AG_BUTTON_HFILL, "#%d", i);

	b2 = AG_BoxNewVert(b, AG_BOX_VFILL);
	for (i = 5; i < 10; i++)
		AG_ButtonNew(b2, AG_BUTTON_HFILL, "#%d", i);

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
FocusWidget1(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);

	if (ti->widget1 != NULL)
		AG_WidgetFocus(ti->widget1);
}

static void
FocusWidget2(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	
	if (ti->widget2 != NULL)
		AG_WidgetFocus(ti->widget2);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->widget1 = NULL;
	ti->widget2 = NULL;
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_Button *btn;
	AG_Box *box;

	AG_LabelNewS(win, 0, "Tests for widget focus states");
	btn = AG_ButtonNewFn(win, 0, "Test unfocused mouse motion", TestUnfocusedMotion, "%p,%p", ti, win);
	AG_WidgetSetFocusable(btn, 0);
	btn = AG_ButtonNewFn(win, 0, "Test <tab> focus cycling", TestTabCycle, "%p,%p", ti, win);
	AG_WidgetSetFocusable(btn, 0);

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		btn = AG_ButtonNewFn(box, 0, "Focus widget 1", FocusWidget1, "%p", ti);
		AG_WidgetSetFocusable(btn, 0);
		btn = AG_ButtonNewFn(box, 0, "Focus widget 2", FocusWidget2, "%p", ti);
		AG_WidgetSetFocusable(btn, 0);
	}
	return (0);
}

const AG_TestCase focusingTest = {
	"focusing",
	N_("Test widget focus state control"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

/*	Public domain	*/
/*
 * This program tests different aspects of widget focusing behavior.
 */

#include <agar/core.h>
#include <agar/gui.h>

static AG_Widget *widget1, *widget2;

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
TestUnfocusedMouseMotion(void)
{
	AG_Window *win;
	AG_Button *btn;
	AG_Fixed *fx1, *fx2;

	win = AG_WindowNew(AG_WINDOW_NOCLOSE);
	AG_WindowSetCaption(win, "Unfocused mousemotion");
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

	widget1 = AGWIDGET(btn);

	fx2 = AG_FixedNew(fx1, AG_FIXED_BOX);
	AGWIDGET(fx2)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION;
	AG_FixedMove(fx1, fx2, 64, 16);
	AG_FixedSize(fx1, fx2, 200, 140);
	AG_SetEvent(fx2, "mouse-motion", mousemotion, NULL);
	AG_SetEvent(fx2, "key-down", keydown, NULL);
	AG_SetEvent(fx2, "mouse-button-down", buttondown, NULL);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win, 0, 0, 320, 240);
	AG_WindowShow(win);
}

static void
TestTabCycle(void)
{
	AG_Window *win;
	AG_Box *b, *b1, *b2;
	AG_Button *btn;
	int i;

	win = AG_WindowNew(AG_WINDOW_NOCLOSE);
	AG_WindowSetCaption(win, "Tab cycle");

	AG_LabelNew(win, 0, "<TAB> = Cycle focus forward\n"
	                    "<SHIFT+TAB> = Cycle focus backward");
	b = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS|AG_BOX_EXPAND);

	btn = AG_ButtonNew(b, AG_BUTTON_HFILL, "Foo");
	widget2 = AGWIDGET(btn);

	b1 = AG_BoxNewVert(b, AG_BOX_VFILL);
	for (i = 0; i < 5; i++)
		AG_ButtonNew(b1, AG_BUTTON_HFILL, "#%d", i);

	b2 = AG_BoxNewVert(b, AG_BOX_VFILL);
	for (i = 5; i < 10; i++)
		AG_ButtonNew(b2, AG_BUTTON_HFILL, "#%d", i);

	AG_WindowSetPosition(win, AG_WINDOW_MR, 0);
	AG_WindowShow(win);
}

static void
FocusWidget1(AG_Event *event)
{
	AG_WidgetFocus(widget1);
}

static void
FocusWidget2(AG_Event *event)
{
	AG_WidgetFocus(widget2);
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Button *btn;
	char *optArg, *driverSpec = NULL;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: focusing [-d agar-driver-spec]\n");
			exit(1);
		}
	}
	if (AG_InitCore("agar-focusing-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	TestUnfocusedMouseMotion();
	TestTabCycle();

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar widget focusing demo");
	AG_WindowSetPosition(win, AG_WINDOW_BC, 0);
	btn = AG_ButtonNewFn(win, 0, "Focus widget 1", FocusWidget1, NULL);
	AG_WidgetSetFocusable(btn, 0);
	btn = AG_ButtonNewFn(win, 0, "Focus widget 2", FocusWidget2, NULL);
	AG_WidgetSetFocusable(btn, 0);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


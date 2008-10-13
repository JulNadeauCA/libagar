/*	Public domain	*/
/*
 * This sample application demonstrates how widgets can control the focus
 * state. A dummy widget receives all mousemotion events and places the
 * focus on itself when the cursor is inside its area. Once the widget has
 * focus, it can receive keyboard events.
 */

#include <agar/core.h>
#include <agar/gui.h>

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
			AG_WindowFocus(AG_WidgetParentWindow(w));
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
CreateWindow(void)
{
	AG_Window *win;
	AG_Button *btn;
	AG_Fixed *fx1, *fx2;

	win = AG_WindowNew(0);
	fx1 = AG_FixedNew(win, AG_FIXED_EXPAND);

	btn = AG_ButtonNew(fx1, 0, "Foo");
	AG_FixedMove(fx1, btn, 0, 0);
	AG_FixedSize(fx1, btn, 32, 32);
	btn = AG_ButtonNew(fx1, 0, "Bar");
	AG_FixedMove(fx1, btn, 0, 32);
	AG_FixedSize(fx1, btn, 32, 32);
	AG_ButtonDisable(btn);

	fx2 = AG_FixedNew(fx1, AG_FIXED_BOX);
	AGWIDGET(fx2)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION;
	AG_FixedMove(fx1, fx2, 64, 16);
	AG_FixedSize(fx1, fx2, 200, 140);
	AG_SetEvent(fx2, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(fx2, "window-keydown", keydown, NULL);
	AG_SetEvent(fx2, "window-mousebuttondown", buttondown, NULL);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win, 0, 0, 320, 240);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-focusing-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


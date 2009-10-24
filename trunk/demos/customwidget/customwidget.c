/*	Public domain	*/
/*
 * Demonstrate use of the custom widget defined in mywidget.c.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include "mywidget.h"

int
main(int argc, char *argv[])
{
	AG_Window *win;
	MyWidget *my;

	if (AG_InitCore("agar-customwidget-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	/* We need to register our new widget class with the object system. */
	AG_RegisterClass(&myWidgetClass);

	/* Create test window containing our widget stretched over its area. */
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	my = MyWidgetNew(win, "foo");
	AG_Expand(my);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


/*	Public domain	*/
/*
 * This test program renders a widget to an AG_Surface(3).
 */

#include <agar/core.h>
#include <agar/gui.h>

int
main(int argc, char *argv[])
{
	AG_Window *win;
	AG_Pixmap *px;
	AG_Button *btn;
	AG_Surface *s;
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: rendertosurface [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-rendertosurface-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar render-to-surface demo");
	btn = AG_ButtonNew(win, 0, "Some widget");
	AG_WindowShow(win);

	/* Render the AG_Button to a surface. */
	if ((s = AG_WidgetSurface(btn)) == NULL) {
		AG_TextMsgFromError();
		goto out;
	}

	/* Display the rendered surface. */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Rendered surface of %s", AGOBJECT(btn)->name);
	px = AG_PixmapFromSurface(win, 0, s);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_BR, 200, 200);
	AG_WindowShow(win);

out:
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

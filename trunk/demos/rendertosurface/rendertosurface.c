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

	/* Initialize Agar-GUI. */
	if (AG_InitCore("agar-rendertosurface-demo", 0) == -1 ||
	    AG_InitVideo(320, 240, 32, AG_VIDEO_OPENGL|AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	win = AG_WindowNew(0);
	btn = AG_ButtonNew(win, 0, "Foo bar");
	AG_WindowShow(win);

	if ((s = AG_WidgetSurface(btn)) != NULL) {
		win = AG_WindowNew(0);
		AG_WindowSetCaption(win, "Surface of %s",
		    AGOBJECT(btn)->name);
		px = AG_PixmapFromSurface(win, 0, s);
		AG_WindowSetGeometryAligned(win, AG_WINDOW_BR,
		    100, 100);
		AG_WindowShow(win);
	} else {
		AG_TextMsgFromError();
	}

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

/*	Public domain	*/
/*
 * Test AG_Window fade-in and opacity (only applies to compositing
 * window managers).
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

float opval = 1.0, opmin = 0.0, opmax = 1.0;

static void
Changed(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_WindowSetOpacity(win, opval);
}

static void
Fadein(AG_Event *event)
{
	AG_Window *win;

	win = AG_WindowNew(AG_WINDOW_FADEIN);
	AG_WindowSetFadeIn(win, 1.0, 0.1);

	AG_PixmapFromBMP(win, 0, "agar.bmp");
	AG_LabelNew(win, 0, "Testing AG_WINDOW_FADEIN");

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", AGWINDETACH(win));
	AG_WindowShow(win);
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_Slider *sl;
	AG_Box *hb;

	win = AG_WindowNew(agDriverSw ? AG_WINDOW_PLAIN : 0);
	AG_WindowSetCaption(win, "Agar compositing demo");
	
	hb = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_PixmapFromBMP(hb, 0, "agar.bmp");
		AG_PixmapFromBMP(hb, 0, "agar.bmp");
		AG_PixmapFromBMP(hb, 0, "agar.bmp");
	}

	hb = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_LabelNew(hb, 0, "Window Opacity: ");
		sl = AG_SliderNewFlt(hb, AG_SLIDER_HORIZ, AG_SLIDER_HFILL,
		    &opval, &opmin, &opmax);
		AG_SetEvent(sl, "slider-changed", Changed, "%p", win);
	}

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Test AG_WINDOW_FADEIN", Fadein, NULL);
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Quit", AGWINDETACH(win));

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 380, -1);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
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
			printf("Usage: compositing [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore(NULL, 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

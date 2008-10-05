/*	Public domain	*/
/*
 * Test the various bindings provided by the AG_Scrollbar(3) widget using
 * arbitrary values and ranges.
 */

#include <agar/config/have_64bit.h>
#include <agar/config/have_long_double.h>

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

int vInt = 50000, minInt = -100000, maxInt = 100000, visInt = 0;
unsigned vUint = 0, minUint = -5, maxUint = 5, visUint = 0;
Sint8 v8 = 100, min8 = -100, max8 = 100, vis8 = 0;
Uint32 v32 = 1234, min32 = 1, max32 = 1000000, vis32 = 0;
float vFlt = 1.0, minFlt = 0.0, maxFlt = 10.0, visFlt = 0.0;
double vDbl = 1.0, minDbl = -1e6, maxDbl = 1e6, visDbl = 0.0;

static void
CreateWindow(void)
{
	AG_Window *win;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Int binding: %d", &vInt);
	AG_ScrollbarNewInt(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vInt, &minInt, &maxInt, &visInt);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Uint binding: %d", &vUint);
	AG_ScrollbarNewInt(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vUint, &minUint, &maxUint, &visUint);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "8-bit binding: %[s8]", &v8);
	AG_ScrollbarNewSint8(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &v8, &min8, &max8, &vis8);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "32-bit binding: %[u32]", &v32);
	AG_ScrollbarNewUint32(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &v32, &min32, &max32, &vis32);

	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Float binding: %f", &vFlt);
	AG_ScrollbarNewFloat(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vFlt, &minFlt, &maxFlt, &visFlt);
	
	AG_LabelNewPolled(win, AG_LABEL_HFILL, "Double binding: %lf", &vDbl);
	AG_ScrollbarNewDouble(win, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_HFILL,
	    &vDbl, &minDbl, &maxDbl, &visDbl);

	AG_WindowMaximize(win);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-scrollbar-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(400, 300, 32, AG_VIDEO_RESIZABLE) == -1) {
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

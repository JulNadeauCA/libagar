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
	AG_Label *lbl;
	AG_Scrollbar *sb;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	
	lbl = AG_LabelNewPolled(win, 0, "Int binding: %d", &vInt);
	sb = AG_ScrollbarNewInt(win, AG_SCROLLBAR_HORIZ, 0, &vInt, &minInt, &maxInt, &visInt);
	AG_ExpandHoriz(lbl);
	AG_ExpandHoriz(sb);
	
	lbl = AG_LabelNewPolled(win, 0, "Uint binding: %d", &vUint);
	sb = AG_ScrollbarNewInt(win, AG_SCROLLBAR_HORIZ, 0, &vUint, &minUint, &maxUint, &visUint);
	AG_ExpandHoriz(lbl);
	AG_ExpandHoriz(sb);

	lbl = AG_LabelNewPolled(win, 0, "8-bit binding: %[s8]", &v8);
	sb = AG_ScrollbarNewSint8(win, AG_SCROLLBAR_HORIZ, 0, &v8, &min8, &max8, &vis8);
	AG_ExpandHoriz(lbl);
	AG_ExpandHoriz(sb);
	
	lbl = AG_LabelNewPolled(win, 0, "32-bit binding: %[u32]", &v32);
	sb = AG_ScrollbarNewUint32(win, AG_SCROLLBAR_HORIZ, 0, &v32, &min32, &max32, &vis32);
	AG_ExpandHoriz(lbl);
	AG_ExpandHoriz(sb);

	lbl = AG_LabelNewPolled(win, 0, "Float binding: %f", &vFlt);
	sb = AG_ScrollbarNewFloat(win, AG_SCROLLBAR_HORIZ, 0, &vFlt, &minFlt, &maxFlt, &visFlt);
	AG_ExpandHoriz(lbl);
	AG_ExpandHoriz(sb);
	
	lbl = AG_LabelNewPolled(win, 0, "Double binding: %lf", &vDbl);
	sb = AG_ScrollbarNewDouble(win, AG_SCROLLBAR_HORIZ, 0, &vDbl, &minDbl, &maxDbl, &visDbl);
	AG_ExpandHoriz(lbl);
	AG_ExpandHoriz(sb);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, -1);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-scrollbar-demo", 0) == -1 ||
	    AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

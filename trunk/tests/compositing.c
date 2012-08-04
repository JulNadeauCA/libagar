/*	Public domain	*/
/*
 * Test AG_Window fade-in and opacity (only applies to compositing
 * window managers).
 */

#include "agartest.h"

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
	AG_WindowSetFadeIn(win, 1.0f, 0.1f);

	AG_PixmapFromBMP(win, 0, "agar.bmp");
	AG_LabelNew(win, 0, "Testing AG_WINDOW_FADEIN");

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Close window", AGWINDETACH(win));
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Slider *sl;
	AG_Box *hb;

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
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 380, -1);
	return (0);
}

const AG_TestCase compositingTest = {
	"compositing",
	N_("Test features for compositing window managers"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};

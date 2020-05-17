/*	Public domain	*/
/*
 * This test program renders a widget to an AG_Surface(3).
 */

#include "agartest.h"

static void
RenderToSurface(AG_Event *event)
{
	AG_Button *btn = AG_BUTTON_PTR(1);
	AG_Window *winParent = AG_WINDOW_PTR(2), *win;
	AG_Surface *S;
	
	/* Render the AG_Button to a surface. */
	if ((S = AG_WidgetSurface(btn)) == NULL) {
		AG_TextMsgFromError();
		return;
	}

	/* Display the rendered surface. */
	if ((win = AG_WindowNew(0)) != NULL) {
		AG_WindowSetCaptionS(win, "Rendered surface");
		AG_LabelNew(win, 0, "Surface generated from %s:", AGOBJECT(btn)->name);
		AG_SeparatorNewHoriz(win);
		AG_PixmapFromSurface(win, 0, S);
		AG_SeparatorNewHoriz(win);
		AG_LabelNew(win, 0,
		    "Format: %u x %u x %d bpp",
		    S->w, S->h,
		    S->format.BitsPerPixel);

		AG_WindowAttach(winParent, win);
		AG_WindowShow(win);
	}
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Button *btn;

	btn = AG_ButtonNew(win, 0, "Test button");
	AG_SeparatorNewHoriz(win);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL,
	    "Render above button to a surface",
	    RenderToSurface, "%p,%p", btn, win);

	return (0);
}

const AG_TestCase rendertosurfaceTest = {
	"rendertosurface",
	N_("Test rendering Agar GUI widgets to software surfaces"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

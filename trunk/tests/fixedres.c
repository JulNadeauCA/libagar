/*	Public domain	*/
/*
 * This program demonstrates the use of fixed-size widgets. This is
 * typically used for specialized applications like game menus (most
 * GUI applications should rely on the automatic widget packing provided
 * by widgets such as AG_Window, AG_Box, AG_Pane, etc.).
 */

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Fixed *fx;
	AG_Label *lb1, *lb2;
	AG_Button *btn;
	AG_Pixmap *px;

	/*
	 * Create a container which allows manual setting of the coordinates
	 * and geometry of its child widgets. We set AG_FIXED_EXPAND so the
	 * container will cover the entire window.
	 */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	/* agColors[WINDOW_BG_COLOR] = AG_ColorRGB(0,0,0); */

	/* Create some background pixmap from an image file. */
	if (!AG_ConfigFile("load-path", "menubg", "bmp", path, sizeof(path))) {
		if ((px = AG_PixmapFromFile(fx, 0, path)) == NULL) {
			AG_LabelNewS(win, 0, AG_GetError());
			fprintf(stderr, "%s\n", AG_GetError());
			exit(1);
		}
		AG_FixedMove(fx, px, 0, 0);
	}

	/*
	 * Create two labels. We don't initially attach the labels to a
	 * parent, so we must use AG_FixedPut().
	 */
	lb1 = AG_LabelNew(NULL, 0, "Fixed Widget Positioning");
	lb2 = AG_LabelNew(NULL, 0, "Demo");
	AG_FixedPut(fx, lb1, 20, 32);
	AG_FixedPut(fx, lb2, 20, 32+agTextFontHeight);

	/*
	 * Create a series of 32x32 buttons at the right. We initially attach
	 * the buttons to the container, so we must use AG_FixedMove().
	 */
	btn = AG_ButtonNew(fx, 0, "A");
	AG_FixedMove(fx, btn, 204, 48);
	AG_FixedSize(fx, btn, 32, 32);
	btn = AG_ButtonNew(fx, 0, "B");
	AG_FixedMove(fx, btn, 204+64, 48);
	AG_FixedSize(fx, btn, 32, 32);
	btn = AG_ButtonNew(fx, 0, "C");
	AG_FixedMove(fx, btn, 204+128, 48);
	AG_FixedSize(fx, btn, 32, 32);
	btn = AG_ButtonNew(fx, 0, "D");
	AG_FixedMove(fx, btn, 204+192, 48);
	AG_FixedSize(fx, btn, 32, 32);

	AG_WindowSetPadding(win, 0,0,0,0);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 640, 128);
	return (0);
}

const AG_TestCase fixedResTest = {
	"fixedRes",
	N_("Test the AG_Fixed(3) container widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

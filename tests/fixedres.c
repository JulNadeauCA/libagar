/*	Public domain	*/
/*
 * This demonstrates the use of the AG_Fixed container, which sizes and
 * positions widgets explicitely given X,Y coordinates and dimensions in
 * display pixels.
 */

#include <stdlib.h>
#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Fixed *fx;
	AG_Label *lb1;
	AG_Button *btn;
	AG_Pixmap *px;
	AG_Box *box;

	/*
	 * Create a container which allows manual setting of the coordinates
	 * and geometry of its child widgets. We set AG_FIXED_EXPAND so the
	 * container will cover the entire window.
	 */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	/* agColors[WINDOW_BG_COLOR] = AG_ColorRGB(0,0,0); */

	/* Create some background pixmap from an image file. */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "menubg.bmp", path, sizeof(path))) {
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
	lb1 = AG_LabelNew(NULL, 0, "I am at 20,32 in %s", AGOBJECT(fx)->name);
	AG_FixedPut(fx, lb1, 20, 32);

	/*
	 * We can always embed a normal AG_Box and items inside will
	 * be packed normally.
	 */
	box = AG_BoxNewVert(NULL, AG_BOX_BORDER);
	AG_SetStyle(box, "font-family", "Serif");
	AG_SetStyle(box, "font-size", "90%");
	AG_SetStyle(box, "line-color", "rgb(150,150,0)");
	AG_SetStyle(box, "line-color#focused", "rgb(200,200,0)");
	{
		AG_Box *hBox;
		int i;

		AG_LabelNewS(box, 0, "I am in a normal box");
		hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL | AG_BOX_BORDER);
		for (i = 0; i < 5; i++)
			AG_ButtonNew(hBox, 0, "%c", '1'+i);
	}
	AG_FixedPut(fx, box, 450, 35);

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
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 641, 195);
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

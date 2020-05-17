/*	Public domain	*/
/*
 * This demonstrates the use of the AG_Fixed container, which sizes and
 * positions widgets explicitely given X,Y coordinates and dimensions in
 * display pixels.
 */

#include <stdlib.h>
#include "agartest.h"

#include <agar/core/agsi.h>

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Fixed *fx;
	AG_Label *lb1;
	AG_Button *btn[4];
	AG_Pixmap *px;
	AG_Box *box;
	int i;

	/*
	 * Create a container which allows manual setting of the coordinates
	 * and geometry of its child widgets. We set AG_FIXED_EXPAND so the
	 * container will cover the entire window.
	 */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);
	AG_SetStyle(fx, "font-family", "fraktur");
	AG_SetStyle(fx, "font-size", "120%");
	AG_SetStyle(fx, "text-color", "#eee");

	/* Create some background pixmap from an image file. */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "menubg.bmp", path, sizeof(path))) {
		if ((px = AG_PixmapFromFile(fx, 0, path)) == NULL) {
			AG_LabelNewS(win, 0, AG_GetError());
		}
		AG_FixedMove(fx, px, 0, 0);
	}

	/*
	 * Create two labels. We don't initially attach the labels to a
	 * parent, so we must use AG_FixedPut().
	 */
	lb1 = AG_LabelNew(NULL, 0, "I'm at 20,32\n"
	                           "(in " AGSI_YEL "%s" AGSI_RST ")\n",
	                           AGOBJECT(fx)->name);
	AG_SetStyle(lb1, "font-family", "vera-mono");
	AG_FixedPut(fx, lb1, 20, 32);
	AG_FixedSize(fx, lb1, 180, 64);

	/*
	 * We can always embed a normal AG_Box and items inside will
	 * be packed normally.
	 */
	box = AG_BoxNewVert(NULL, 0);
	AG_SetStyle(box, "font-family", "vera");
	AG_SetStyle(box, "font-size", "80%");
	AG_SetStyle(box, "font-style", "italic");
	{
		AG_Box *hBox;
		AG_Button *btnNum;
		int i;

		AG_LabelNewS(box, 0, "I'm in a normal box");
		hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
		for (i = 0; i < 5; i++) {
			btnNum = AG_ButtonNew(hBox, 0, "%c", '1'+i);
			AG_SetStyle(btnNum, "padding", "5");
			AG_SetStyle(btnNum, "color", "#233");
		}
	}
	AG_FixedPut(fx, box, 450, 35);

	/*
	 * Create a series of 32x32 buttons at the right. We initially attach
	 * the buttons to the container, so we must use AG_FixedMove().
	 */
	btn[0] = AG_ButtonNew(fx, 0, "A");
	AG_FixedMove(fx, btn[0], 204, 48);
	btn[1] = AG_ButtonNew(fx, 0, "B");
	AG_FixedMove(fx, btn[1], 204+64, 48);
	btn[2] = AG_ButtonNew(fx, 0, "C");
	AG_FixedMove(fx, btn[2], 204+128, 48);
	btn[3] = AG_ButtonNew(fx, 0, "D");
	AG_FixedMove(fx, btn[3], 204+192, 48);
	for (i = 0; i < 4; i++) {
		AG_FixedSize(fx, btn[i], 32, 32);
		AG_SetStyle(btn[i], "color", "black");
		AG_SetStyle(btn[i], "high-color", "#775");
		AG_SetStyle(btn[i], "low-color", "#333");
	}

	/*
	 * Make this window non-resizable (alternatively, we could also
	 * have put everything into an AG_Scrollview).
	 */
	win->flags |= AG_WINDOW_NORESIZE;

	/* Disable padding around borders. */
	AG_SetStyle(win, "padding", "0");

	/* Request an explicit size in pixels. */
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 642, 200);

	return (0);
}

const AG_TestCase fixedresTest = {
	"fixedres",
	N_("Test the AG_Fixed(3) container widget"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

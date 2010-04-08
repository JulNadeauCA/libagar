/*	Public domain	*/
/*
 * This program demonstrates the use of fixed-size widgets. This is
 * typically used for specialized applications like game menus (most
 * GUI applications should rely on the automatic widget packing provided
 * by widgets such as AG_Window, AG_Box, AG_Pane, etc.).
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

static void
CreateGameMenu(void)
{
	AG_Window *win;
	AG_Fixed *fx;
	AG_Label *lb1, *lb2;
	AG_Button *btn;
	AG_Pixmap *px;

	win = AG_WindowNewNamedS(agDriverSw ? AG_WINDOW_PLAIN : 0,
	    "game-menu");
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_BL, 640, 128);
	AG_WindowSetCaption(win, "Agar fixed widget positioning demo");

	/*
	 * Create a container which allows manual setting of the coordinates
	 * and geometry of its child widgets. We set AG_FIXED_EXPAND so the
	 * container will cover the entire window.
	 */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	/*
	 * Set the window background color to black since the background
	 * pixmap has rounded edges.
	 */
	agColors[WINDOW_BG_COLOR] = AG_ColorRGB(0,0,0);

	/* Create the background pixmap from bmp file. */
	if ((px = AG_PixmapFromBMP(fx, 0, "Images/menubg.bmp")) == NULL) {
		fprintf(stderr, "%s\n", AG_GetError());
		exit(1);
	}
	AG_FixedMove(fx, px, 0, 0);

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

	/* Display the new window. */
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
			printf("Usage: fixedres [-d agar-driver-spec]\n");
			return (1);
		}
	}

	if (AG_InitCore("agar-fixedres-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateGameMenu();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


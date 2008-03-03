/*	Public domain	*/
/*
 * This program demonstrates the use of fixed-size widgets. This is
 * typically used for specialized applications like game menus. Most
 * GUI applications should rely on container widgets such as AG_Window
 * and AG_Box to position and size widgets.
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

	/* Create a fixed-size window with no titlebar or decorations. */
	win = AG_WindowNewNamed(AG_WINDOW_PLAIN, "game-menu");
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetGeometry(win, 0, 480-128, 640, 128);

	/*
	 * Create a container which allows manual setting of the coordinates
	 * and geometry of its child widgets. The AG_FIXED_EXPAND flag
	 * requests that the container cover the entire window.
	 */
	fx = AG_FixedNew(win, AG_FIXED_EXPAND);

	/*
	 * Set the window background color to black since the background
	 * pixmap has rounded edges.
	 */
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0, 0, 0);

	/* Create the background pixmap from bmp file. */
	if ((px = AG_PixmapFromBMP(fx, 0, "Images/menubg.bmp")) == NULL) {
		fprintf(stderr, "Cannot find menubg.bmp\n", AG_GetError());
		exit(1);
	}
	AG_FixedMove(fx, px, 0, 0);

	/*
	 * Create two labels. We don't initially attach the labels to a
	 * parent, so we must use AG_FixedPut().
	 */
	lb1 = AG_LabelNewStatic(NULL, 0, "Fixed Resolution");
	lb2 = AG_LabelNewStatic(NULL, 0, "Demo");
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
	if (AG_InitCore("agar-fixedres-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	CreateGameMenu();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


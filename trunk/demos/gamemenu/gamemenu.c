/*	$Csoft: tests.c,v 1.2 2005/09/27 04:11:01 vedge Exp $	*/
/*	Public domain	*/

/*
 * This program demonstrates the use of fixed-size widgets. This is
 * typically used for specialized applications like game menus. Most
 * GUI applications should rely on container widgets such as AG_Window
 * and AG_Box to position and size widgets.
 */

#include <engine/engine.h>
#include <engine/input.h>
#include <engine/config.h>
#include <engine/view.h>

#include <engine/widget/gui.h>

#include <string.h>
#include <unistd.h>

static void
CreateGameMenu(void)
{
	AG_Window *win;
	AG_Fixed *fx;
	AG_Label *lb1, *lb2;
	AG_Button *btn;
	AG_Pixmap *px;

	/* Create a fixed-size window with no titlebar or border. */
	win = AG_WindowNew(AG_WINDOW_NO_TITLEBAR|AG_WINDOW_NO_DECORATIONS,
	    "game-menu");
	AG_WindowSetPadding(win, 0, 0, 0);

	/*
	 * Create a container which allows manual setting of the coordinates
	 * and geometry of its child widgets. The flags request that this
	 * container cover the entire window.
	 */
	fx = AG_FixedNew(win, AG_FIXED_WFILL|AG_FIXED_HFILL);

	/*
	 * Set the window background color to black since the background
	 * pixmap has rounded edges.
	 */
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0, 0, 0);

	/* Create the background pixmap from bmp file. */
	px = AG_PixmapFromBMP(fx, "menubg.bmp");
	AG_FixedMove(fx, px, 0, 0);

	/*
	 * Create two labels. We don't initially attach the labels to a
	 * parent, so we must use AG_FixedPut().
	 */
	lb1 = AG_LabelNew(NULL, AG_LABEL_STATIC, "Foo");
	lb2 = AG_LabelNew(NULL, AG_LABEL_STATIC, "Bar");
	AG_FixedPut(fx, lb1, 10, 10);
	AG_FixedPut(fx, lb2, 10, AGWIDGET(lb1)->h+5);

	/*
	 * Create a series of 32x32 buttons at the right. We initially attach
	 * the buttons to the container, so we must use AG_FixedMove().
	 */
	btn = AG_ButtonNew(fx, "A");
	AG_FixedMove(fx, btn, 204, 48);
	AG_FixedSize(fx, btn, 32, 32);
	btn = AG_ButtonNew(fx, "B");
	AG_FixedMove(fx, btn, 204+64, 48);
	AG_FixedSize(fx, btn, 32, 32);
	btn = AG_ButtonNew(fx, "C");
	AG_FixedMove(fx, btn, 204+128, 48);
	AG_FixedSize(fx, btn, 32, 32);
	btn = AG_ButtonNew(fx, "D");
	AG_FixedMove(fx, btn, 204+192, 48);
	AG_FixedSize(fx, btn, 32, 32);

	/*
	 * Set the window geometry manually. This must be called after the
	 * widgets have been attached.
	 */
	AG_WindowSetGeometry(win, 0, 480-128, 640, 128);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("gamemenu-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFgGr:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case 'r':
			fps = atoi(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFgG] [-r fps]\n", agProgName);
			exit(0);
		}
	}

	/* Initialize a 640x480x32 display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(640, 480, 32, 0) == -1 ||
	    AG_InitInput(AG_INPUT_KBDMOUSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_SetRefreshRate(fps);
	
	CreateGameMenu();

	AG_EventLoop();
	AG_Quit();
	return (0);
fail:
	AG_Quit();
	return (1);
}


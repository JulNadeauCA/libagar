/*	$Csoft: gamemenu.c,v 1.6 2005/10/07 07:09:35 vedge Exp $	*/
/*	Public domain	*/

/*
 * This application displays a maximized window with no titlebar or borders,
 * and with a padding setting of 0.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_Button *btn;
	AG_Table *table;
	AG_Box *box;
	int i;

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOBORDERS|
	                   AG_WINDOW_NOTITLE);
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	box = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_EXPAND);
	AG_BoxSetPadding(box, 0);

	table = AG_TableNew(box, AG_TABLE_EXPAND);
	AG_TableAddCol(table, "Foo", "<8888>", NULL);
	AG_TableAddCol(table, "Bar", NULL, NULL);
	for (i = 0; i < 1000; i++) {
		AG_TableAddRow(table, "%d:%s", i, "Foo");
	}

	AG_WindowSetGeometry(win, 0, 0, agView->w, agView->h);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("nopadding-demo", 0) == -1) {
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
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	CreateWindow();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


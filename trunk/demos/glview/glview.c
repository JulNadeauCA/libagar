/*	$Csoft: gamemenu.c,v 1.3 2005/10/01 14:19:55 vedge Exp $	*/
/*	Public domain	*/

/*
 * This program demonstrates the use of Agar's GL context widget, AG_GLView.
 * Obviously, this application requires OpenGL. It is also necessary for Agar
 * to have been compiled with OpenGL support.
 */

#include <engine/engine.h>
#include <engine/config.h>
#include <engine/view.h>

#include <engine/widget/gui.h>
#include <engine/widget/glview.h>	/* This is a nonstandard widget */

#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("glview-demo", 0) == -1) {
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
	
	CreateGameMenu();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


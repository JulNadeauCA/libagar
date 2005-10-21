/*	$Csoft: gamemenu.c,v 1.3 2005/10/01 14:19:55 vedge Exp $	*/
/*	Public domain	*/

/*
 * This application demonstrates the use of Agar's graphical
 * object manager.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("objmgr-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFgGbBt:T:r:")) != -1) {
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
		case 'b':
			AG_SetBool(agConfig, "font.freetype", 0);
			AG_SetString(agConfig, "font-face", "minimal.xcf");
			AG_SetInt(agConfig, "font-size", 11);
			break;
		case 'B':
			AG_SetBool(agConfig, "font.freetype", 1);
			AG_SetString(agConfig, "font-face", "Vera.ttf");
			break;
		case 'T':
			AG_SetString(agConfig, "font-path", "%s", optarg);
			break;
		case 't':
			AG_TextParseFontSpec(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFgGbB] [-r fps] [-t fontspec] "
			       "[-T font-path]\n",
			    agProgName);
			exit(0);
		}
	}

	/* Initialize a 640x480x32 display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1 ||
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Reload the previous state. */
	AG_ObjectLoad(agWorld);

	/* Initialize the object manager. */
	AG_ObjMgrInit();

	/* Show the object manager window. */
	AG_WindowShow(AG_ObjMgrWindow());
	
	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


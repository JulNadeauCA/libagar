/*	$Csoft: gamemenu.c,v 1.3 2005/10/01 14:19:55 vedge Exp $	*/
/*	Public domain	*/

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/core/view.h>
#include <agar/core/objmgr.h>

#include <agar/game/map/mapedit.h>

#include <agar/gui/gui.h>

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

	{
		extern int agObjectIgnoreDataErrors;
		extern int agObjectIgnoreUnknownObjs;
		
		AG_MapEditorInit();
		agObjectIgnoreDataErrors = 1;
		agObjectIgnoreUnknownObjs = 1;
		AG_ObjectLoad(agWorld);
		agObjectIgnoreDataErrors = 0;
		agObjectIgnoreUnknownObjs = 0;
	}

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


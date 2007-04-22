/*	$Csoft: glview.c,v 1.5 2005/10/08 02:45:25 vedge Exp $	*/
/*	Public domain	*/

/*
 * This program demonstrates the use of the SG_View widget to
 * display a scene.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/sg.h>

#include <string.h>
#include <unistd.h>
#include <math.h>

static void
CreateWindow(void)
{
	AG_Window *win;
	SG_View *sv;
	AG_HBox *hb;
	SG_Sphere *s1, *s2;
	SG *sg;
	int i;
	float angle = 60.0;

	sg = SG_New(agWorld, "scene");
	AGOBJECT(sg)->flags |= AG_OBJECT_DATA_RESIDENT;
	AGOBJECT(sg)->data_used = 1;
	AGOBJECT(sg)->gfx = AG_GfxNew(sg);
	AGOBJECT(sg)->gfx->used = 1;
	
	s1 = SG_SphereNew(sg->root, "Sphere A");
	for (i = 0; i < 60; i++) {
		s2 = SG_SphereNew(s1, "Sphere B");
		s1 = s2;
		SG_Rotatev(s1, angle, SG_J);
		SG_Translate3(s1, 2.0, 0.4, 0.0);
		angle -= 0.4;
	}
#if 1
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Scene");
	sv = SG_ViewNew(win, sg, SG_VIEW_EXPAND);
	//sv->cam->eye = SG_VECTOR(108.0, 249.0, -2.5);
	//sv->cam->pitch = 64.0;
	//sv->cam->yaw = 270.0;
	sv->cam->zFar = 1000.0;

	AG_WindowSetGeometry(win, 120, 0, agView->w-120, agView->h);
	AG_WindowShow(win);
#endif
	{
		SG_Light *lt;

		lt = SG_LightNew(sg->root, "Light0");
		SG_Translate3(lt, 5.0, 3.0, 2.0);
		lt->Kc = 0.5;
		lt->Kl = 0.05;
	}
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("sgview-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFgGbBt:r:")) != -1) {
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
			break;
		case 'B':
			AG_SetBool(agConfig, "font.freetype", 1);
			break;
		case 't':
			AG_TextParseFontSpec(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFgGbB] [-r fps] [-t fontspec]\n",
			    agProgName);
			exit(0);
		}
	}

	/* Initialize the display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(1024, 700, 32, AG_VIDEO_OPENGL) == -1 ||
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	/* Initialize the object manager. */
	AG_ObjMgrInit();
	AG_WindowShow(AG_ObjMgrWindow());

	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0,0,0);
	CreateWindow();

	AG_EventLoop();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


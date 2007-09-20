/*	Public domain	*/
/*
 * This program demonstrates the use of the SG_View widget to
 * display a scene.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>
#include <agar/sg.h>

#include <string.h>
#include <unistd.h>
#include <math.h>

AG_Timeout rotTimer;
SG_Sphere *center;

static Uint32
Rotate(void *obj, Uint32 t, void *arg)
{
	SG_Camera *cam = arg;
	SG_Vector vCenter = SG_VectorSub(SG_NodePos(cam), SG_NodePos(center));

	SG_Orbitvd(cam, vCenter, SG_VecJ, 1.0);
	return (t);
}

static void
CreateWindow(void)
{
	AG_Window *win;
	SG_View *sv;
	AG_HBox *hb;
	SG_Sphere *s1, *s2;
	SG_Light *lt;
	SG *sg;
	int i;
	SG_Real angle = 60.0;

	/* Create a new scene graph. */
	sg = SG_New(agWorld, "scene");
	AGOBJECT(sg)->flags |= AG_OBJECT_RESIDENT;

	/* Create a bunch of spheres. */
	s1 = center = SG_SphereNew(sg->root, "Sphere A");
	for (i = 0; i < 60; i++) {
		s2 = SG_SphereNew(s1, "Sphere B");
		s1 = s2;
		SG_Rotatev(s1, angle, SG_VecJ);
		SG_Translate3(s1, 2.0, 0.4, 0.0);
		angle -= 0.4;
	}
	
	/* Create a light source. */
	lt = SG_LightNew(sg->root, "MyLight");
	SG_Translate3(lt, 5.0, 3.0, 2.0);
	lt->Kc = 0.5;				/* Set constant attenuation */
	lt->Kl = 0.05;				/* Set linear attenuation */

	/* Create a window containing a SG_View widget. */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Scene");
	sv = SG_ViewNew(win, sg, SG_VIEW_EXPAND);

	/* Move the camera. */
	SG_Translate3(sv->cam, -10.0, 10.0, 30.0);

	AG_WindowSetGeometry(win, 120, 0, agView->w-120, agView->h);
	AG_WindowShow(win);

	AG_SetTimeout(&rotTimer, Rotate, sv->cam, 0);
	AG_AddTimeout(sv, &rotTimer, 10);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	int w = 640, h = 480;
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
		case 'w':
			w = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
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
			printf("%s [-vfFgGbB] [-w px] [-h px] [-r fps] "
			       "[-t fontspec]\n", agProgName);
			exit(0);
		}
	}

	/* Initialize the display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(w, h, 32, AG_VIDEO_OPENGL|AG_VIDEO_RESIZABLE)
	    == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitInput(0);
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);

	/* Set some useful hotkeys */
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	/* Initialize Agar-DEV and show the object browser. */
	DEV_InitSubsystem(0);
	AG_WindowShow(DEV_Browser());

	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0,0,0);
	CreateWindow();

	AG_EventLoop();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


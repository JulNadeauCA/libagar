/*	$Csoft: glview.c,v 1.1 2005/10/04 18:02:17 vedge Exp $	*/
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

#include <GL/glu.h>

static GLfloat vx = 0.0, vy = 0.0, vz = 0.0;
static GLdouble spin = 0.0;

static void
ScaleCube(AG_Event *event)
{
	glLoadIdentity();
	glOrtho(-1.0, 1.0, 1.0, -1.0, -1.0, 1.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void
DrawCube(AG_Event *event)
{
	GLfloat redMat[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat blueMat[] = { 0.0, 1.0, 0.0, 0.5 };
	GLUquadric *q;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redMat);
		glRotatef(spin, vx, vy, 1.0);
		q = gluNewQuadric();
		gluSphere(q, 0.50, 10, 10);
	}
	glPopMatrix();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	spin = (spin + 2.0);
	if (spin > 360.0) { spin -= 360.0; }
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_GLView *glv;
	AG_VBox *vb;

	SDL_EnableKeyRepeat(250, 50);

	win = AG_WindowNew(AG_WINDOW_NOBACKGROUND);
	glv = AG_GLViewNew(win, AG_GLVIEW_WFILL|AG_GLVIEW_HFILL);

	AG_GLViewScaleFn(glv, ScaleCube, NULL);
	AG_GLViewDrawFn(glv, DrawCube, NULL);
	AG_WidgetFocus(glv);

	vb = AG_VBoxNew(win, 0);
	{
		AG_FSpinbutton *fsb;

		fsb = AG_FSpinbuttonNew(vb, NULL, "Vx:");
		AG_WidgetBindFloat(fsb, "value", &vx);
		
		fsb = AG_FSpinbuttonNew(vb, NULL, "Vy:");
		AG_WidgetBindFloat(fsb, "value", &vy);

		fsb = AG_FSpinbuttonNew(vb, NULL, "Vz:");
		AG_WidgetBindFloat(fsb, "value", &vz);
	}

	AG_WindowShow(win);
}

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
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_OPENGL) == -1 ||
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


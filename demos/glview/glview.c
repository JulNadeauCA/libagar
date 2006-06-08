/*	$Csoft: glview.c,v 1.5 2005/10/08 02:45:25 vedge Exp $	*/
/*	Public domain	*/

/*
 * This program demonstrates the use of Agar's GL context widget, AG_GLView.
 * Obviously, this application requires OpenGL. It is also necessary for Agar
 * to have been compiled with OpenGL support.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glu.h>

static GLdouble spin = 0.0, vx = 1.0, vz = -5.0;
static GLfloat material[4] = { 1.0, 0.0, 0.0, 1.0 };

static void
ScaleCube(AG_Event *event)
{
	glLoadIdentity();
	gluPerspective(60.0, 1.0, 0.01, 100.0);
}

static void
DrawCube(AG_Event *event)
{
	GLUquadric *q;
	
	glLoadIdentity();
	gluLookAt(0.5, vz, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material);
		glRotatef(spin, vx, 0.0, 1.0);
		q = gluNewQuadric();
		gluCylinder(q, 1.0, 0.0, 1.0, 5, 2);
		
		glRotatef(180.0, 0.0, 1.0, 0.0);
		q = gluNewQuadric();
		gluCylinder(q, 1.0, 0.0, 1.0, 5, 2);
	}
	glPopMatrix();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	spin = (spin + 2.0);
	if (spin > 360.0) { spin -= 360.0; }
}

static void
Mousebutton(AG_Event *event)
{
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	switch (button) {
	case SDL_BUTTON_WHEELDOWN:
		vz += 1.0;
		break;
	case SDL_BUTTON_WHEELUP:
		vz -= 1.0;
		break;
	}
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_GLView *glv;
	AG_HBox *hb;
	AG_VBox *vb;
	AG_HSVPal *pal;
	AG_FSpinbutton *fsb;

	SDL_EnableKeyRepeat(250, 50);

	win = AG_WindowNew(0);
	
	hb = AG_HBoxNew(win, AG_HBOX_HFILL|AG_HBOX_VFILL);
	{
		glv = AG_GLViewNew(hb, AG_GLVIEW_HFILL|AG_GLVIEW_VFILL);

		AG_GLViewScaleFn(glv, ScaleCube, NULL);
		AG_GLViewDrawFn(glv, DrawCube, NULL);
		AG_GLViewButtondownFn(glv, Mousebutton, NULL);
		AG_WidgetFocus(glv);

		pal = AG_HSVPalNew(hb, AG_HSVPAL_VFILL);
		AG_WidgetBindFloat(pal, "RGBAv", material);
	}

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		fsb = AG_FSpinbuttonNew(vb, 0, NULL, "Vx:");
		AG_WidgetBindDouble(fsb, "value", &vx);
	}

	AG_WindowShow(win);
	AG_WindowMaximize(win);
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

	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0,0,0);
	CreateWindow();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


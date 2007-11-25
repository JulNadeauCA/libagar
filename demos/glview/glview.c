/*	Public domain	*/
/*
 * This program demonstrates the use of Agar's low-level GL context widget,
 * AG_GLView. This widget is part of Agar-GUI, and does not require the
 * higher-level Agar-SG (scene graph) library.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

#include <string.h>
#include <unistd.h>

static GLdouble spin = 0.0, vz = -5.0, spin2 = 0.0;
static GLfloat material[4] = { 0.0, 0.5, 0.0, 1.0 };

/* Widget resize callback function. */
static void
MyScaleFunction(AG_Event *event)
{
	glLoadIdentity();

	/* Set a 60 degrees field of view with 1.0 aspect ratio. */
	gluPerspective(60.0, 1.0, 0.01, 100.0);
}

/* Draw callback function. */
static void
MyDrawFunction(AG_Event *event)
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
		glRotatef(spin, 1.0, 0.0, 1.0);
		glRotatef(-spin2, 0.0, 0.0, 1.0);
		q = gluNewQuadric();
		gluCylinder(q, 1.0, 0.0, 1.0, 20, 2);
		
		glRotatef(180.0, 0.0, 1.0, 0.0);
		q = gluNewQuadric();
		gluCylinder(q, 1.0, 0.0, 1.0, 20, 2);
	}
	glPopMatrix();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	spin = (spin + 2.0);
	if (spin > 360.0) { spin -= 360.0; }
	
	spin2 = (spin2 + 4.0);
	if (spin2 > 360.0) { spin2 -= 360.0; }
}

/*
 * Overlay callback function. This type of callback is useful for rendering
 * things such as status text on top of the OpenGL context.
 */
static void
MyOverlayFunction(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();
	Uint32 myColor = SDL_MapRGB(agVideoFmt, 255, 255, 255);
	SDL_Surface *myText;

	/* Render a text string using the font engine. */
	AG_PushTextState();
	AG_TextColorRGB(255, 255, 125);
	myText = AG_TextFormat("Spin = %.0f degrees, z = %.02f", spin, vz);
	AG_PopTextState();

	/*
	 * Blit the text at the lower left of the widget. Note that the
	 * AG_WidgetMapSurface() interface is much more efficient at this
	 * (hardware->hardware copy), unless the text changes frequently.
	 */
	AG_WidgetBlit(glv, myText,
	    0,
	    AGWIDGET(glv)->h - agTextFontHeight);

	SDL_FreeSurface(myText);
}

/* Mouse click callback function. */
static void
Mousebutton(AG_Event *event)
{
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		vz += 1.0;
		if (vz > 0.0) {
			vz = 0.0;
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		vz -= 1.0;
		break;
	}
}

static void
CreateMainWindow(void)
{
	AG_Window *win;
	AG_GLView *glv;
	AG_HBox *hb;
	AG_HSVPal *pal;
	AG_FSpinbutton *fsb;

	win = AG_WindowNew(0);
	
	hb = AG_HBoxNew(win, AG_HBOX_EXPAND);
	{
		/* Create the AG_GLView widget. */
		glv = AG_GLViewNew(hb, AG_GLVIEW_EXPAND);
		AG_WidgetFocus(glv);

		/* Set up our callback functions. */ 
		AG_GLViewScaleFn(glv, MyScaleFunction, NULL);
		AG_GLViewDrawFn(glv, MyDrawFunction, NULL);
		AG_GLViewOverlayFn(glv, MyOverlayFunction, NULL);
		AG_GLViewButtondownFn(glv, Mousebutton, NULL);

		/*
		 * Create an HSV palette widget and bind it directly
		 * to the material color ("RGBAv" binds to 4 floats).
		 */
		pal = AG_HSVPalNew(hb, AG_HSVPAL_VFILL);
		AG_WidgetBindFloat(pal, "RGBAv", material);
	}

	AG_WindowShow(win);
	AG_WindowMaximize(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	int wDisplay = 640, hDisplay = 480;
	char *s;

	if (AG_InitCore("glview-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFbBt:r:w:h:")) != -1) {
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
		case 'w':
			wDisplay = atoi(optarg);
			break;
		case 'h':
			hDisplay = atoi(optarg);
			break;
		case '?':
		default:
			printf("%s [-vfFbB] [-w width] [-h height] "
			        "[-r fps] [-t fontspec]\n", agProgName);
			exit(0);
		}
	}

	/* Pass AG_VIDEO_OPENGL flag to require an OpenGL display. */
	if (AG_InitVideo(wDisplay, hDisplay, 32,
	    AG_VIDEO_OPENGL|AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_SetRefreshRate(fps);

	/* Configure some useful hotkeys. */
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Set a black background. */
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0,0,0);

	/* Create the main window. */
	CreateMainWindow();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


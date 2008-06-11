/*	Public domain	*/
/*
 * This program demonstrates the use of Agar's low-level OpenGL context
 * widget, AG_GLView.
 *
 * AG_GLView is only concerned with saving/restoring visualization matrices
 * and the relevant OpenGL states. For high-level scene graph functionality,
 * you may be interested in FreeSG (http://freesg.org/), which provides a
 * general-purpose Agar visualization widget (SG_View).
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

#include <string.h>

#include <GL/glu.h>

static GLfloat spin = 0.0f, vz = -5.0f, spin2 = 0.0f;
static GLfloat material[4] = { 0.0f, 0.5f, 0.0f, 1.0f };

/* Widget resize callback function. */
static void
MyScaleFunction(AG_Event *event)
{
	glLoadIdentity();

	/* Set a 60 degrees field of view with 1.0 aspect ratio. */
	gluPerspective(60.0, 1.0, 0.01, 100.0);
}

/*
 * Rendering function. This routine renders our scene. Any GL command,
 * with the exception of glViewport(), can be issued here.
 */
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
		glRotatef(spin, 1.0f, 0.0f, 1.0f);
		glRotatef(-spin2, 0.0f, 0.0f, 1.0f);
		q = gluNewQuadric();
		gluCylinder(q, 1.0, 0.0, 1.0, 20, 2);
		gluDeleteQuadric(q);
		
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
		q = gluNewQuadric();
		gluCylinder(q, 1.0, 0.0, 1.0, 20, 2);
		gluDeleteQuadric(q);
	}
	glPopMatrix();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	spin = (spin + 2.0f);
	if (spin > 360.0f) { spin -= 360.0f; }
	
	spin2 = (spin2 + 4.0f);
	if (spin2 > 360.0f) { spin2 -= 360.0f; }
}

/*
 * Overlay callback function. This type of callback is useful for rendering
 * things such as status text on top of the OpenGL context.
 */
static void
MyOverlayFunction(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();
	Uint32 myColor = AG_MapRGB(agVideoFmt, 255,255,255);
	AG_Surface *myText;

	/* Render a text string using the font engine. */
	AG_PushTextState();
	AG_TextColorRGB(255, 255, 125);
	myText = AG_TextRenderf("Spin = %.0f degrees, z = %.02f", spin, vz);
	AG_PopTextState();

	/*
	 * Blit the text at the lower left of the widget. Note that the
	 * AG_WidgetMapSurface() interface is much more efficient at this
	 * (hardware->hardware copy), unless the text changes frequently.
	 */
	AG_WidgetBlit(glv, myText,
	    0,
	    AGWIDGET(glv)->h - agTextFontHeight);

	AG_SurfaceFree(myText);
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

	win = AG_WindowNew(AG_WINDOW_PLAIN);
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
	if (AG_InitCore("agar-glview-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	/* Pass AG_VIDEO_OPENGL flag to require an OpenGL display. */
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_OPENGL|AG_VIDEO_RESIZABLE)
	    == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Set a black background. */
	agColors[WINDOW_BG_COLOR] = AG_MapRGB(agVideoFmt, 0,0,0);

	CreateMainWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


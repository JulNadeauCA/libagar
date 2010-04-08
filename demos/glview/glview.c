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

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

const char *primitiveNames[] = { "Cube", "Sphere", NULL };
enum { CUBE, SPHERE } primitive = SPHERE;

const char *shadingNames[] = { "Flat Shading", "Smooth Shading", NULL };
enum { FLATSHADING, SMOOTHSHADING } shading = FLATSHADING;

GLfloat spin = 0.0f;
GLdouble vz = -5.0;
GLfloat ambient[4] = { 0.5f, 1.0f, 1.0f, 1.0f };
GLfloat diffuse[4] = { 0.5f, 1.0f, 1.0f, 1.0f };
GLfloat specular[4] = { 0.5f, 1.0f, 1.0f, 1.0f };
int wireframe = 0;

static GLdouble isoVtx[12][3] = {    
#define X .525731112119133606 
#define Z .850650808352039932
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};
static GLuint isoInd[20][3] = { 
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };


/* Widget resize callback function. */
static void
MyScaleFunction(AG_Event *event)
{
	GLdouble xMin, xMax, yMin, yMax;
	
	glLoadIdentity();

	/* Set a 60 degrees field of view with 1.0 aspect ratio. */
	yMax = 0.01*tan(0.523598f);
	yMin = -yMax;
	xMin = yMin;
	xMax = yMax;
	glFrustum(xMin, xMax, yMin, yMax, 0.01, 100.0);
}

static void
Norm(GLdouble *a)
{
    GLdouble d = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    a[0] /= d;
    a[1] /= d;
    a[2] /= d;
}

static void
DrawTriangle(GLdouble *a, GLdouble *b, GLdouble *c, int div, float r)
{
	if (div <= 0) {
		glNormal3dv(a); glVertex3d(a[0]*r, a[1]*r, a[2]*r);
		glNormal3dv(b); glVertex3d(b[0]*r, b[1]*r, b[2]*r);
		glNormal3dv(c); glVertex3d(c[0]*r, c[1]*r, c[2]*r);
	} else {
		GLdouble ab[3], ac[3], bc[3];
		int i;

		for (i = 0; i < 3; i++) {
			ab[i] = (a[i]+b[i])/2.0;
			ac[i] = (a[i]+c[i])/2.0;
			bc[i] = (b[i]+c[i])/2.0;
		}
		Norm(ab);
		Norm(ac);
		Norm(bc);
		DrawTriangle(a, ab, ac, div-1, r);
		DrawTriangle(b, bc, ab, div-1, r);
		DrawTriangle(c, ac, bc, div-1, r);
		DrawTriangle(ab, bc, ac, div-1, r);
	}
}

/* Render the test object. */
static void
MyDrawFunction(AG_Event *event)
{
	int i;
	GLfloat pos[4];
	
	glLoadIdentity();
	
	glPushAttrib(GL_POLYGON_BIT|GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_POLYGON);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glShadeModel(shading == FLATSHADING ? GL_FLAT : GL_SMOOTH);

	pos[0] = 10.0f;
	pos[1] = 10.0f;
	pos[2] = 0.0f;
	pos[3] = 1.0f;
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 10.0f);
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	
	pos[1] = -10.0f;
	pos[2] = 10.0f;
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 10.0f);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);

	glPushMatrix();
	glTranslated(0.0, 0.0, vz);
	glRotatef(spin,0.0f,1.0f,0.0f);
	glRotatef(spin,1.0f,1.0f,1.0f);

	switch (primitive) {
	case CUBE:
		glBegin(GL_QUADS);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, 1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glVertex3f(-1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f, 1.0f,-1.0f);
		glVertex3f( 1.0f, 1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f, 1.0f);
		glVertex3f( 1.0f,-1.0f,-1.0f);
		glEnd();
		break;
	case SPHERE:
		glBegin(GL_TRIANGLES);
		for (i = 0; i < 20; i++) {
			DrawTriangle(isoVtx[isoInd[i][0]],
			             isoVtx[isoInd[i][1]],
				     isoVtx[isoInd[i][2]],
				     2, 1.0f);
		}
    		glEnd();
	}

	glPopMatrix();
	glPopAttrib();

	if (++spin > 360.0f) { spin -= 360.0f; }
}

/*
 * Overlay callback function. This type of callback is useful for rendering
 * things such as status text on top of the OpenGL context.
 */
static void
MyOverlayFunction(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();
	AG_Surface *myText;

	/* Render a text string using the font engine. */
	AG_PushTextState();
	AG_TextColorRGB(255, 255, 125);
	myText = AG_TextRenderf("Zoom using mouse wheel\n"
	                        "Spin = %.0f degrees, z = %.02f", spin, vz);
	AG_PopTextState();

	/*
	 * Blit the text at the lower left of the widget. Note that the
	 * AG_WidgetMapSurface() interface is much more efficient at this
	 * (hardware->hardware copy), unless the text changes frequently.
	 */
	AG_WidgetBlit(glv, myText,
	    0,
	    AGWIDGET(glv)->h - agTextFontHeight*2 - 5);

	AG_SurfaceFree(myText);
}

/* Control the Z using the mouse wheel. */
static void
ButtonDown(AG_Event *event)
{
	int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_WHEELUP:
		vz -= 0.1;
		break;
	case AG_MOUSE_WHEELDOWN:
		vz += 0.1;
		break;
	}
}

static void
CreateMainWindow(void)
{
	AG_Window *win;
	AG_GLView *glv;
	AG_Box *hb;
	AG_HSVPal *pal;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar low-level OpenGL context demo");

	hb = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;

		/* Create the AG_GLView widget. */
		glv = AG_GLViewNew(hb, AG_GLVIEW_EXPAND);
		AG_WidgetFocus(glv);

		/* Set up our callback functions. */ 
		AG_GLViewScaleFn(glv, MyScaleFunction, NULL);
		AG_GLViewDrawFn(glv, MyDrawFunction, NULL);
		AG_GLViewOverlayFn(glv, MyOverlayFunction, NULL);
		AG_GLViewButtondownFn(glv, ButtonDown, NULL);

		/* Edit ambient and diffuse color components. */
		nb = AG_NotebookNew(hb, AG_NOTEBOOK_VFILL);
		{
			ntab = AG_NotebookAddTab(nb, "Amb", AG_BOX_VERT);
			pal = AG_HSVPalNew(ntab,
			    AG_HSVPAL_NOALPHA|AG_HSVPAL_VFILL);
			AG_BindFloat(pal, "RGBAv", ambient);

			ntab = AG_NotebookAddTab(nb, "Dif", AG_BOX_VERT);
			pal = AG_HSVPalNew(ntab,
			    AG_HSVPAL_NOALPHA|AG_HSVPAL_VFILL);
			AG_BindFloat(pal, "RGBAv", diffuse);

			ntab = AG_NotebookAddTab(nb, "Spe", AG_BOX_VERT);
			pal = AG_HSVPalNew(ntab,
			    AG_HSVPAL_NOALPHA|AG_HSVPAL_VFILL);
			AG_BindFloat(pal, "RGBAv", specular);
		}
	}
	hb = AG_BoxNewHoriz(win, AG_BOX_FRAME|AG_BOX_HFILL);
	{
		AG_RadioNewInt(hb, 0, primitiveNames, (int *)&primitive);
		AG_SeparatorNewVert(hb);
		AG_RadioNewInt(hb, 0, shadingNames, (int *)&shading);
		AG_SeparatorNewVert(hb);
		AG_ButtonNewInt(hb, AG_BUTTON_STICKY, "Wireframe Mode",
		    &wireframe);
	}

	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	char *optArg, *driverSpec = "<OpenGL>";
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: glview [-d agar-driver-spec]\n");
			exit(1);
		}
	}

	if (AG_InitCore("agar-glview-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	/* Set a black background. */
	agColors[WINDOW_BG_COLOR] = AG_ColorRGB(0,0,0);

	CreateMainWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

#else /* !HAVE_OPENGL */
#include "../common/stub.h"
int
main(int argc, char *argv[])
{
	return DemoFail(
	    "This demo requires OpenGL support. Please recompile Agar\n"
	    "with the `--with-opengl' configure option\n");
}
#endif /* HAVE_OPENGL */

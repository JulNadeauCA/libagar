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
#include <math.h>

const char *primitiveNames[] = { "Cube", "Sphere", NULL };
enum { CUBE, SPHERE } primitive = SPHERE;

GLfloat spin = 0.0f, vz = -5.0f;
GLfloat ambient[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
GLfloat diffuse[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
GLfloat specular[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
int wireframe = 1;

static GLfloat isoVtx[12][3] = {    
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
Norm(GLfloat *a)
{
    GLfloat d = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    a[0] /= d;
    a[1] /= d;
    a[2] /= d;
}

static void
DrawTriangle(GLfloat *a, GLfloat *b, GLfloat *c, int div, float r)
{
	if (div <= 0) {
		glNormal3fv(a); glVertex3f(a[0]*r, a[1]*r, a[2]*r);
		glNormal3fv(b); glVertex3f(b[0]*r, b[1]*r, b[2]*r);
		glNormal3fv(c); glVertex3f(c[0]*r, c[1]*r, c[2]*r);
	} else {
		GLfloat ab[3], ac[3], bc[3];
		int i;

		for (i = 0; i < 3; i++) {
			ab[i] = (a[i]+b[i])/2;
			ac[i] = (a[i]+c[i])/2;
			bc[i] = (b[i]+c[i])/2;
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

/* Render a cube. */
static void
MyDrawFunction(AG_Event *event)
{
	int i;
	
	glLoadIdentity();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_POLYGON);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, vz);
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
				     2, 1.0);
		}
    		glEnd();
	}

	glPopMatrix();
	glPopAttrib();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHTING);

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

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		vz -= 0.1;
		break;
	case SDL_BUTTON_WHEELDOWN:
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

	win = AG_WindowNew(AG_WINDOW_PLAIN);
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
		AG_GLViewButtondownFn(glv, Mousebutton, NULL);

		nb = AG_NotebookNew(hb, AG_NOTEBOOK_VFILL);
		ntab = AG_NotebookAddTab(nb, "Ambient", AG_BOX_VERT);
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_VFILL);
		AG_WidgetBindFloat(pal, "RGBAv", ambient);
		ntab = AG_NotebookAddTab(nb, "Diffuse", AG_BOX_VERT);
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_VFILL);
		AG_WidgetBindFloat(pal, "RGBAv", diffuse);
		ntab = AG_NotebookAddTab(nb, "Specular", AG_BOX_VERT);
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_VFILL);
		AG_WidgetBindFloat(pal, "RGBAv", specular);
	}
	hb = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_FRAME);
	{
		AG_RadioNewInt(hb, 0, primitiveNames, (int *)&primitive);
		AG_ButtonNewInt(hb, AG_BUTTON_STICKY, "Wireframe", &wireframe);
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


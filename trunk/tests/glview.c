/*	Public domain	*/
/*
 * This program demonstrates the use of the AG_GLView widget.
 *
 * AG_GLView provides a low-level GL context. Under OpenGL-based Agar
 * display drivers, all AG_GLView does is to save and restore the relevant
 * GL matrices and states around calls to Draw().
 *
 * For high-level scene graph functionality, check out the Agar-based
 * FreeSG library at http://freesg.org/.
 */

#include "agartest.h"

#ifdef HAVE_OPENGL

#include <agar/gui/opengl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

const char *primitiveNames[] = { "Cube", "Sphere", NULL };
const char *shadingNames[] = { "Flat Shading", "Smooth Shading", NULL };

typedef struct {
	AG_TestInstance _inherit;
	enum { CUBE, SPHERE } primitive;
	enum { FLATSHADING, SMOOTHSHADING } shading;
	GLfloat spin;
	GLdouble vz;
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	int wireframe;
} MyTestInstance;

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

static __inline__ void
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
	MyTestInstance *ti = AG_PTR(1);
	GLfloat pos[4];
	int i;

	glLoadIdentity();
	glPushAttrib(GL_POLYGON_BIT|GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, ti->wireframe ? GL_LINE : GL_POLYGON);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ti->ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ti->diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ti->specular);
	glShadeModel(ti->shading == FLATSHADING ? GL_FLAT : GL_SMOOTH);

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

	glLightfv(GL_LIGHT1, GL_AMBIENT, ti->ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ti->diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, ti->specular);

	glPushMatrix();
	glTranslated(0.0, 0.0, ti->vz);
	glRotatef(ti->spin,0.0f,1.0f,0.0f);
	glRotatef(ti->spin,1.0f,1.0f,1.0f);

	switch (ti->primitive) {
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
}

static Uint32
UpdateRotation(AG_Timer *to, AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	
	if (++ti->spin > 360.0f) { ti->spin -= 360.0f; }
	return (to->ival);
}

/*
 * Overlay callback function. This type of callback is useful for rendering
 * things such as status text on top of the OpenGL context.
 */
static void
MyOverlayFunction(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();
	MyTestInstance *ti = AG_PTR(1);
	AG_Surface *myText;

	/* Render a text string using the font engine. */
	AG_PushTextState();
	AG_TextColorRGB(255, 255, 125);
	myText = AG_TextRenderf("Zoom using mouse wheel\n"
	                        "Spin = %.0f degrees, z = %.02f",
				ti->spin, ti->vz);
	AG_PopTextState();

	/*
	 * Blit the text at the lower left of the widget. Note that the
	 * AG_WidgetMapSurface() interface is much more efficient at this
	 * (hardware->hardware copy), unless the text changes frequently.
	 */
	if (myText != NULL) {
		AG_WidgetBlit(glv, myText,
		    0,
		    AGWIDGET(glv)->h - agTextFontHeight*2 - 5);
		AG_SurfaceFree(myText);
	}
}

/* Control the Z using the mouse wheel. */
static void
ButtonDown(AG_Event *event)
{
	MyTestInstance *ti = AG_PTR(1);
	int button = AG_INT(2);

	switch (button) {
	case AG_MOUSE_WHEELUP:
		ti->vz -= 0.1;
		break;
	case AG_MOUSE_WHEELDOWN:
		ti->vz += 0.1;
		break;
	}
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;
	GLfloat amb[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat dif[4] = { 0.0f, 1.0f, 0.5f, 1.0f };
	GLfloat spe[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	int i;

	ti->primitive = SPHERE;
	ti->shading = FLATSHADING;
	ti->spin = 0.0f;
	ti->vz = -5.0;
	for (i = 0; i < 4; i++) {
		ti->ambient[i] = amb[i];
		ti->diffuse[i] = dif[i];
		ti->specular[i] = spe[i];
	}
	ti->wireframe = 0;
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	MyTestInstance *ti = obj;
	AG_GLView *glv;
	AG_Box *hb;
	AG_HSVPal *pal;

	hb = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	{
		AG_Pane *pa;
		AG_Notebook *nb, *nbColor;
		AG_NotebookTab *ntab;
		int i;
		
		pa = AG_PaneNewHoriz(hb, AG_PANE_EXPAND|AG_PANE_DIV1FILL);
		AG_PaneResizeAction(pa, AG_PANE_EXPAND_DIV1);
		nb = AG_NotebookNew(pa->div[0], AG_NOTEBOOK_EXPAND);

		for (i = 0; i < 2; i++) {
			ntab = AG_NotebookAdd(nb, "Test tab", AG_BOX_VERT);
		
			/* Create the AG_GLView widget. */
			glv = AG_GLViewNew(ntab, AG_GLVIEW_EXPAND);

			/* Set a periodic redraw at 60fps. */
			AG_RedrawOnTick(glv, 1000/60);
			
			/* Set up our callback functions. */ 
			AG_GLViewScaleFn(glv, MyScaleFunction, NULL);
			AG_GLViewDrawFn(glv, MyDrawFunction, "%p", ti);
			AG_GLViewOverlayFn(glv, MyOverlayFunction, "%p", ti);
			AG_GLViewButtondownFn(glv, ButtonDown, "%p", ti);

			/* Update the rotation 30 times per second. */
			AG_AddTimerAuto(win, 1000/30, 
			    UpdateRotation, "%p", ti);
		}

		/* Edit ambient and diffuse color components. */
		nbColor = AG_NotebookNew(pa->div[1], AG_NOTEBOOK_EXPAND);
		{
			ntab = AG_NotebookAdd(nbColor, "Amb", AG_BOX_VERT);
			pal = AG_HSVPalNew(ntab,
			    AG_HSVPAL_NOALPHA|AG_HSVPAL_EXPAND);
			AG_BindFloat(pal, "RGBAv", ti->ambient);

			ntab = AG_NotebookAdd(nbColor, "Dif", AG_BOX_VERT);
			pal = AG_HSVPalNew(ntab,
			    AG_HSVPAL_NOALPHA|AG_HSVPAL_EXPAND);
			AG_BindFloat(pal, "RGBAv", ti->diffuse);

			ntab = AG_NotebookAdd(nbColor, "Spe", AG_BOX_VERT);
			pal = AG_HSVPalNew(ntab,
			    AG_HSVPAL_NOALPHA|AG_HSVPAL_EXPAND);
			AG_BindFloat(pal, "RGBAv", ti->specular);
		}
	}
	hb = AG_BoxNewHoriz(win, AG_BOX_FRAME|AG_BOX_HFILL);
	{
		AG_RadioNewInt(hb, 0, primitiveNames, (void *)&ti->primitive);
		AG_SeparatorNewVert(hb);
		AG_RadioNewInt(hb, 0, shadingNames, (void *)&ti->shading);
		AG_SeparatorNewVert(hb);
		AG_ButtonNewInt(hb, AG_BUTTON_STICKY, "Wireframe Mode",
		    &ti->wireframe);
	}
	return (0);
}

#endif /* HAVE_OPENGL */

const AG_TestCase glviewTest = {
	"glView",
	"Test the AG_GLView(3) widget (OpenGL required)",
	"1.4.2",
	AG_TEST_OPENGL,
#ifdef HAVE_OPENGL
	sizeof(MyTestInstance),
	Init,
	NULL,
	NULL,		/* test */
	TestGUI,
#else
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	NULL,		/* testGUI */
#endif
	NULL		/* bench */
};

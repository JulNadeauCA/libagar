/*	Public domain	*/

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/sg.h>

#include <math.h>

SG_Range xr = { -2.0, 0.05, 2.0 };
SG_Range ur = {  2.0, 0.05, 2.0 };

SG_Real spinX = 0.0, spinY = -0.0, spinZ = 0.0;
SG_Real urMin = -2.0;
int init = 1;
int plotMode = 1;

static void
Plot(void *pNode, SG_View *sgv)
{
	SG_Node *node = pNode;
	SG_Color color;
	SG_Vector v;
	SG_Real x, u;
	int mode = 0;
	int i;
	
	switch (plotMode) {
	case 0:	mode = SG_POINTS; break;
	case 1:	mode = SG_LINE_STRIP; break;
	case 2:	mode = SG_LINES; break;
	}
	
	SG_Begin(SG_LINES);
	SG_Vertex3(-1.0, 0.0, 0.0);
	SG_Vertex3(+1.0, 0.0, 0.0);
	SG_Vertex3(0.0, -1.0, 0.0);
	SG_Vertex3(0.0, +1.0, 0.0);
	SG_Vertex3(0.0, 0.0, -1.0);
	SG_Vertex3(0.0, 0.0, +1.0);
	SG_End();

	for (x = xr.a; x < xr.b; x += xr.d) {
		SG_Begin(mode);
		for (u = ur.a; u < ur.b; u += ur.d) {
			v.x = x;
			v.y = SG_Cbrt(-u/3.0) + u*SG_Sqrt(-x/2.0);
			v.z = u;
			SG_VectorScalev(&v, 30.0);
			color.r = 1.0;
			color.g = 1.0 - 1.0/SG_Fabs(u*100.0);
			color.b = 1.0 - 1.0/SG_Fabs(u*100.0);
			SG_Color3v(&color);
			SG_Vertex3v(&v);
		}
		SG_End();
	}
	for (x = xr.a; x < xr.b; x += xr.d) {
		SG_Begin(mode);
		for (u = ur.a; u < ur.b; u += ur.d) {
			v.x = x;
			v.y = SG_Cbrt(-u/3.0) - u*SG_Sqrt(-x/2.0);
			v.z = u;
			SG_VectorScalev(&v, 30.0);
			color.g = 1.0;
			color.r = 1.0 - 1.0/SG_Fabs(x*3.0);
			color.b = 1.0 - 1.0/SG_Fabs(x*3.0);
			SG_Color3v(&color);
			SG_Vertex3v(&v);
		}
		SG_End();
	}

	SG_RotateEul(node, spinX, spinY, spinZ);
	SG_RotateEul(node, spinX, spinY, spinZ);
	if (init) {
		if (ur.a > urMin) {
			ur.a -= 0.06;
		} else {
			init = 0;
			spinY = -0.006;
		}
	}
}

/* Define a new node type for the plot. */
SG_NodeOps CatastropheOps = {
	"Catastrophe",
	sizeof(SG_Node),
	0,
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
	NULL,	
	NULL,		
	Plot
};

AG_Window *
CreateParametersWindow(void)
{
	AG_Window *win;
	AG_FSpinbutton *fsb;
	AG_Spinbutton *sb;
	const char *modes[] = { "Points", "Lines", "Segments", NULL };
	AG_Radio *rad;
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Parameters");
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	rad = AG_RadioNew(win, AG_RADIO_EXPAND, modes);
	AG_WidgetBindInt(rad, "value", &plotMode);

	fsb = AG_FSpinbuttonNew(win, 0, NULL, "xStart:");
	AG_FSpinbuttonSetIncrement(fsb, 0.1);
	SG_WidgetBindReal(fsb, "value", &xr.a);
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "xInc:");
	AG_FSpinbuttonSetIncrement(fsb, 0.001);
	SG_WidgetBindReal(fsb, "value", &xr.d);
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "xEnd:");
	AG_FSpinbuttonSetIncrement(fsb, 0.1);
	SG_WidgetBindReal(fsb, "value", &xr.b);
	
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "uStart:");
	AG_FSpinbuttonSetIncrement(fsb, 0.1);
	SG_WidgetBindReal(fsb, "value", &ur.a);
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "uInc:");
	AG_FSpinbuttonSetIncrement(fsb, 0.001);
	SG_WidgetBindReal(fsb, "value", &ur.d);
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "uEnd:");
	AG_FSpinbuttonSetIncrement(fsb, 0.1);
	SG_WidgetBindReal(fsb, "value", &ur.b);
	
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "Pitch:");
	AG_FSpinbuttonSetIncrement(fsb, 0.0001);
	SG_WidgetBindReal(fsb, "value", &spinX);
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "Yaw:");
	AG_FSpinbuttonSetIncrement(fsb, 0.0001);
	SG_WidgetBindReal(fsb, "value", &spinY);
	fsb = AG_FSpinbuttonNew(win, 0, NULL, "Roll:");
	AG_FSpinbuttonSetIncrement(fsb, 0.0001);
	SG_WidgetBindReal(fsb, "value", &spinZ);
	
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	AG_Window *win;
	SG_View *sv;
	SG *sg;
	SG_Node *cat;
	char *s;

	if (AG_InitCore("catastrophe-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_OPENGL|AG_VIDEO_RESIZABLE)
	    == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitInput(0);
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 0,0,0);

	/* Create the scene. */
	sg = SG_New(agWorld, "scene");
	cat = SG_NodeAdd(sg->root, "MyCatastrophe", &CatastropheOps, 0);
	SG_Translate3(cat, 0.0, 0.0, 90.0);
	SG_Rotatevd(cat, 30.0, SG_VecJ);

	/* Create the user interface. */
	CreateParametersWindow();
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Catastrophe");
	sv = SG_ViewNew(win, sg, SG_VIEW_EXPAND|SG_VIEW_NO_LIGHTING);
	SG_Translate3(sv->cam, 10.0, 0.0, -100.0);
	SG_Rotatevd(sv->cam, 180.0, SG_VecI);
	
	AG_WindowSetGeometry(win, 64, 0, agView->w-64, agView->h);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

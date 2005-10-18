/*	$Csoft: glview.c,v 1.5 2005/10/08 02:45:25 vedge Exp $	*/
/*	Public domain	*/

/*
 * This application uses Agar's vector graphics library to generate
 * a simple sketch.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/core/view.h>

#include <agar/gui/gui.h>
#include <agar/vg/vg.h>

#include <string.h>
#include <unistd.h>

static AG_Timeout tUpdate, tRot;
double spin = 0.0;

Uint32
UpdateDisplay(void *obj, Uint32 ival, void *p)
{
	AG_Pixmap *px = obj;
	VG *vg = p;

	VG_Rasterize(vg);
	AG_PixmapUpdateSurface(px);
	return (ival);
}

Uint32
UpdateRotation(void *obj, Uint32 ival, void *p)
{
	VG_Matrix *rot = p;

	VG_LoadRotate(rot, spin);
	if ((spin += 1.0) > 360.0) { spin = spin - 360.0; }
	return (ival);
}

void
CreateDrawing(void)
{
	AG_Window *win;
	VG_Matrix *rot, *pos;
	VG_Style *bold;
	VG *vg;

	/*
	 * Create a new vector graphics object with antialiasing enabled,
	 * a visible origin and a visible grid.
	 */
	vg = VG_New(NULL, VG_ANTIALIAS|VG_VISORIGIN);

	/*
	 * Scale the drawing to 20x20 units with a scaling factor of 1.
	 * From this point on, it is safe to reference the raster surface
	 * using vg->su.
	 */
	VG_Scale(vg, 20.0, 20.0, 1.0);

	/* Create a thick green line style. */
	bold = VG_CreateStyle(vg, VG_LINE_STYLE, "bold");
	bold->vg_line_st.thickness = 2;

	/* Draw a triangle and apply some transformations. */
	VG_Begin(vg, VG_LINE_LOOP);
	{
		/* Request a green color for this element. */
		VG_Color3(vg, 0, 255, 0);

		/* Assign the "bold" line style. */
		VG_SetStyle(vg, "bold");

		/*
		 * Push a rotation and a translation matrix. If you think in
		 * terms relative to the drawing's origin, the transformations
		 * are specified in reverse order.
		 */
		VG_Rotate(vg, 90.0);
		rot = VG_PushIdentity(vg);
		pos = VG_Translate(vg, 10.0, 10.0);

		/* Draw a triangle. */
		VG_Vertex2(vg, 0.0, -5.0);
		VG_Vertex2(vg, -5.0, 5.0);
		VG_Vertex2(vg, 5.0, 5.0);
	}
	VG_End(vg);

	/* Use an AG_Pixmap widget to display the raster surface. */
	win = AG_WindowNew(AG_WINDOW_NORESIZE);
	{
		AG_Pixmap *px;
		AG_MFSpinbutton *fsb;

		VG_Rasterize(vg);
		px = AG_PixmapFromSurface(win, vg->su);

		/* Use a timer to rasterize the drawing every 2ms. */
		AG_SetTimeout(&tUpdate, UpdateDisplay, vg, 0);
		AG_AddTimeout(px, &tUpdate, 1);
		
		/* Use another timer to update the rotation matrix. */
		AG_SetTimeout(&tRot, UpdateRotation, rot, 0);
		AG_AddTimeout(NULL, &tRot, 2);

		/*
		 * Bind a spinbutton widget directly to the translation
		 * elements of the translation matrix.
		 */
		fsb = AG_MFSpinbuttonNew(win, NULL, ",", "Translation: ");
		AG_WidgetBindDouble(fsb, "xvalue", &pos->m[0][2]);
		AG_WidgetBindDouble(fsb, "yvalue", &pos->m[1][2]);
		AG_MFSpinbuttonSetIncrement(fsb, 0.2);

		AG_WindowShow(win);
	}
}

int
main(int argc, char *argv[])
{
	int c, i, fps = 60;
	char *s;

	if (AG_InitCore("vectordwg-demo", 0) == -1) {
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
	if (AG_InitVideo(640, 480, 32, 0) == -1 ||
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
	CreateDrawing();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


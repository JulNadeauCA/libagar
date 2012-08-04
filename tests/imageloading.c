/*	Public domain	*/
/*
 * Test built-in image load/export functions.
 */

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Surface *su, *su2;

	/* Load, display and save a PNG file. */
	AG_LabelNew(win, 0, "Loading agar.png:");
	if ((su = AG_SurfaceFromPNG("agar.png")) == NULL) {
		AG_LabelNew(win, 0, "Failed: %s", AG_GetError());
	} else {
		AG_PixmapFromSurface(win, 0, su);
	}
	AG_LabelNew(win, 0, "Saving agar.png:");
	if (AG_SurfaceExportPNG(su, "agar-save.png") == -1) {
		AG_LabelNew(win, 0, "Save failed: %s", AG_GetError());
	} else {
		if ((su2 = AG_SurfaceFromPNG("agar-save.png")) == NULL) {
			AG_LabelNew(win, 0, "Load failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(win, 0, su2);
		}
	}
	
	/* Load/save a PNG file in indexed color format. */
	AG_LabelNew(win, 0, "Loading agar-index.png:");
	if ((su = AG_SurfaceFromPNG("agar-index.png")) == NULL) {
		AG_LabelNew(win, 0, "Failed: %s", AG_GetError());
	} else {
		AG_PixmapFromSurface(win, 0, su);
	}
	AG_LabelNew(win, 0, "Saving agar-index.png:");
	if (AG_SurfaceExportPNG(su, "agar-index-save.png") == -1) {
		AG_LabelNew(win, 0, "Save failed: %s", AG_GetError());
	} else {
		if ((su2 = AG_SurfaceFromPNG("agar-index-save.png")) == NULL) {
			AG_LabelNew(win, 0, "Load failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(win, 0, su2);
		}
	}

	/* Load/save a JPEG file. */
	AG_LabelNew(win, 0, "Loading pepe.jpg:");
	if ((su = AG_SurfaceFromJPEG("pepe.jpg")) == NULL) {
		AG_LabelNew(win, 0, "Failed: %s", AG_GetError());
	} else {
		AG_PixmapFromSurface(win, 0, su);
	}
	AG_LabelNew(win, 0, "Saving pepe.jpg:");
	if (AG_SurfaceExportJPEG(su, "pepe-save.jpg") == -1) {
		AG_LabelNew(win, 0, "Save failed: %s", AG_GetError());
	} else {
		if ((su2 = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
			AG_LabelNew(win, 0, "Load failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(win, 0, su2);
		}
	}
	return (0);
}

const AG_TestCase imageLoadingTest = {
	"imageLoading",
	N_("Test the image loader / exporter routines"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI
};

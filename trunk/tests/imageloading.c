/*	Public domain	*/
/*
 * Test built-in image load/export functions.
 */

#include "agartest.h"

#include "config/datadir.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Surface *su, *su2;

	/* Load, display and save a PNG file. */
	AG_LabelNewS(win, 0, "Loading agar.png:");
	if (!AG_ConfigFile("load-path", "agar", "png", path, sizeof(path))) {
		if ((su = AG_SurfaceFromPNG(path)) == NULL) {
			AG_LabelNew(win, 0, "%s: %s", path, AG_GetError());
		} else {
			AG_PixmapFromSurface(win, 0, su);
		}
		AG_LabelNewS(win, 0, "Saving agar.png:");
		if (AG_SurfaceExportPNG(su, "agar-save.png") == -1) {
			AG_LabelNew(win, 0, "Save failed: %s", AG_GetError());
		} else {
			if ((su2 = AG_SurfaceFromPNG("agar-save.png")) == NULL) {
				AG_LabelNew(win, 0, "Load failed: %s", AG_GetError());
			} else {
				AG_PixmapFromSurface(win, 0, su2);
			}
		}
	} else {
		AG_LabelNewS(win, 0, AG_GetError());
	}

	/* Load/save a PNG file in indexed color format. */
	AG_LabelNewS(win, 0, "Loading agar-index.png:");
	if (!AG_ConfigFile("load-path", "agar-index", "png", path, sizeof(path))) {
		if ((su = AG_SurfaceFromPNG(path)) == NULL) {
			AG_LabelNew(win, 0, "Failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(win, 0, su);
		}
		AG_LabelNewS(win, 0, "Saving agar-index.png:");
		if (AG_SurfaceExportPNG(su, "agar-index-save.png") == -1) {
			AG_LabelNew(win, 0, "Save failed: %s", AG_GetError());
		} else {
			if ((su2 = AG_SurfaceFromPNG("agar-index-save.png")) == NULL) {
				AG_LabelNew(win, 0, "Load failed: %s", AG_GetError());
			} else {
				AG_PixmapFromSurface(win, 0, su2);
			}
		}
	} else {
		AG_LabelNewS(win, 0, AG_GetError());
	}

	/* Load/save a JPEG file. */
	AG_LabelNewS(win, 0, "Loading pepe.jpg:");
	if (!AG_ConfigFile("load-path", "pepe", "jpg", path, sizeof(path))) {
		if ((su = AG_SurfaceFromJPEG(path)) == NULL) {
			AG_LabelNew(win, 0, "Failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(win, 0, su);
		}
		AG_LabelNewS(win, 0, "Saving pepe.jpg:");
		if (AG_SurfaceExportJPEG(su, "pepe-save.jpg") == -1) {
			AG_LabelNew(win, 0, "Save failed: %s", AG_GetError());
		} else {
			if ((su2 = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
				AG_LabelNew(win, 0, "Load failed: %s", AG_GetError());
			} else {
				AG_PixmapFromSurface(win, 0, su2);
			}
		}
	} else {
		AG_LabelNewS(win, 0, AG_GetError());
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
	TestGUI,
	NULL		/* bench */
};

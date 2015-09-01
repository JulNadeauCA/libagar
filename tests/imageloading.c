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
	AG_Box *hBox, *vBox, *bppBox;
	AG_PixelFormat *pfTest;

	hBox = AG_BoxNewHoriz(win, AG_BOX_EXPAND|AG_BOX_FRAME);

	vBox = AG_BoxNewVert(hBox, AG_BOX_FRAME);
	
	/* Display a PNG file with transparency */
	AG_LabelNewS(vBox, 0, "Imported axe.png:");
	if (!AG_ConfigFile("load-path", "axe", "png", path, sizeof(path))) {
		if ((su = AG_SurfaceFromPNG(path)) != NULL) {
			AG_PixmapFromSurface(vBox, 0, su);
			AG_LabelNewS(vBox, 0, "Exported axe-save.png:");
			if (AG_SurfaceExportPNG(su, "axe-save.png", 0) == 0) {
				if ((su2 = AG_SurfaceFromPNG("axe-save.png")) != NULL) {
					AG_PixmapFromSurface(vBox, 0, su2);
					AG_SurfaceFree(su2);
				} else {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				}
			} else {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			}
			AG_SurfaceFree(su);
		} else {
			AG_LabelNew(vBox, 0, "%s: %s", path, AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	/* Load, display and save a PNG file. */
	AG_LabelNewS(vBox, 0, "Imported agar.png:");
	if (!AG_ConfigFile("load-path", "agar", "png", path, sizeof(path))) {
		if ((su = AG_SurfaceFromPNG(path)) != NULL) {
			AG_PixmapFromSurface(vBox, 0, su);
		
			AG_LabelNewS(vBox, 0, "Converted to {8,16,24,32}bpp:");
			bppBox = AG_BoxNewHoriz(vBox, 0 );
			{
				const int testDepths[4] = { 8,16,24,32 };
				int i;

				for (i = 0; i < 4; i++) {
					pfTest = AG_PixelFormatRGB(testDepths[i], 0xff000000, 0x00ff0000, 0x0000ff00);
					if ((su2 = AG_SurfaceConvert(su, pfTest)) != NULL) {
						AG_PixmapFromSurface(bppBox, 0, su2);
						AG_SurfaceFree(su2);
					} else {
						AG_LabelNew(bppBox, 0, "Convert failed: %s", AG_GetError());
					}
					AG_PixelFormatFree(pfTest);
				}
			}
			AG_LabelNewS(vBox, 0, "Exported agar-save.png:");
			if (AG_SurfaceExportPNG(su, "agar-save.png", 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((su2 = AG_SurfaceFromPNG("agar-save.png")) != NULL) {
					AG_PixmapFromSurface(vBox, 0, su2);
					AG_SurfaceFree(su2);
				} else {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				}
			}
			AG_SurfaceFree(su);
		} else {
			AG_LabelNew(vBox, 0, "%s: %s", path, AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	/* Load/save a PNG file in indexed color format. */
	AG_LabelNewS(vBox, 0, "Imported agar-index.png:");
	if (!AG_ConfigFile("load-path", "agar-index", "png", path, sizeof(path))) {
		if ((su = AG_SurfaceFromPNG(path)) != NULL) {
			AG_PixmapFromSurface(vBox, 0, su);

			AG_LabelNewS(vBox, 0, "Converted to {8,16,24,32}bpp:");
			bppBox = AG_BoxNewHoriz(vBox, 0 );
			{
				const int testDepths[4] = { 8,16,24,32 };
				int i;

				for (i = 0; i < 4; i++) {
					pfTest = AG_PixelFormatRGB(testDepths[i], 0xff000000, 0x00ff0000, 0x0000ff00);
					if ((su2 = AG_SurfaceConvert(su, pfTest)) != NULL) {
						AG_PixmapFromSurface(bppBox, 0, su2);
						AG_SurfaceFree(su2);
					} else {
						AG_LabelNew(bppBox, 0, "Convert failed: %s", AG_GetError());
					}
					AG_PixelFormatFree(pfTest);
				}
			}

			AG_LabelNewS(vBox, 0, "Exported agar-index-save.png:");
			if (AG_SurfaceExportPNG(su, "agar-index-save.png", 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((su2 = AG_SurfaceFromPNG("agar-index-save.png")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, su2);
					AG_SurfaceFree(su2);
				}
			}

			AG_SurfaceFree(su);
		} else {
			AG_LabelNew(vBox, 0, "Failed: %s", AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	vBox = AG_BoxNewVert(hBox, AG_BOX_FRAME);

	/* Load/save a JPEG file. */
	AG_LabelNewS(vBox, 0, "Imported pepe.jpg:");
	if (!AG_ConfigFile("load-path", "pepe", "jpg", path, sizeof(path))) {
		if ((su = AG_SurfaceFromJPEG(path)) == NULL) {
			AG_LabelNew(vBox, 0, "Failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(vBox, 0, su);

			AG_LabelNewS(vBox, 0, "Exported pepe.jpg (Quality = 10%):");
			if (AG_SurfaceExportJPEG(su, "pepe-save.jpg", 10, 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((su2 = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, su2);
					AG_SurfaceFree(su2);
				}
			}
			AG_LabelNewS(vBox, 0, "Exported pepe.jpg (Quality = 100%):");
			if (AG_SurfaceExportJPEG(su, "pepe-save.jpg", 100, 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((su2 = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, su2);
					AG_SurfaceFree(su2);
				}
			}
			AG_SurfaceFree(su);
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}
	return (0);
}

const AG_TestCase imageLoadingTest = {
	"imageLoading",
	N_("Test the image loader / exporter routines"),
	"1.5.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

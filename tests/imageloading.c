/*	Public domain	*/
/*
 * Test built-in image load/export functions.
 */

#include "agartest.h"

#include "config/datadir.h"

static void
Test_Format(const AG_Surface *_Nonnull S, AG_PixelFormat *_Nonnull pf,
    AG_Box *parent)
{
	AG_Surface *D;

	if ((D = AG_SurfaceConvert(S, pf)) != NULL) {
		AG_PixmapFromSurface(parent, 0, D);
		AG_SurfaceFree(D);
	} else {
		AG_LabelNew(parent, 0, "Convert failed: %s", AG_GetError());
	}
	AG_PixelFormatFree(pf);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_FILENAME_MAX];
	AG_Surface *S, *D;
	AG_Scrollview *sv;
	AG_Box *hBox, *vBox, *box;
	AG_PixelFormat pfTest;
	int i;

	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND |
	                           AG_SCROLLVIEW_BY_MOUSE |
				   AG_SCROLLVIEW_PAN_RIGHT);

	hBox = AG_BoxNewHoriz(sv, 0);
	vBox = AG_BoxNewVert(hBox, 0);
	
	/*
	 * Load BMP files in different flavors of the format.
	 * Demonstrate how AG_Printf() can be used to construct path names.
	 */
	for (i = 1; i <= 4; i++) {
		char pathExp[AG_FILENAME_MAX];

		if (AG_ConfigFind(AG_CONFIG_PATH_DATA,
		                  AG_Printf("agar-%d.bmp", i),
		                  path, sizeof(path)) == 0)
		{
			AG_LabelNew(vBox, 0, "Here is %s:", path);
			if ((S = AG_SurfaceFromBMP(path)) != NULL) {
				AG_PixmapFromSurface(vBox, 0, S);

				Snprintf(pathExp, sizeof(pathExp),
				    "agar-%d-save.png", i);
				AG_LabelNew(vBox, 0, "Exported to %s:", pathExp);
				if (AG_SurfaceExportPNG(S, pathExp, 0) == 0) {
					if ((D = AG_SurfaceFromPNG(pathExp)) != NULL) {
						AG_PixmapFromSurface(vBox, 0, D);
						AG_SurfaceFree(D);
					} else {
						AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
					}
				} else {
					AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
				}
				AG_SurfaceFree(S);
			} else {
				AG_LabelNew(vBox, 0, "%s: %s", path, AG_GetError());
			}
		} else
		{
			AG_LabelNewS(vBox, 0, AG_GetError());
		}
	}

	/*
	 * Load, display and export a PNG file in RGBA format.
	 * Export it and load it again.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "axe.png", path, sizeof(path))) {
		AG_LabelNew(vBox, 0, "Here is %s:", path);
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			AG_Surface *Sx;

			AG_PixmapFromSurface(vBox, 0, S);

			AG_LabelNew(vBox, 0, "Scaled to 64x64:");
			Sx = AG_SurfaceScale(S, 64,64, 0);
			AG_PixmapFromSurface(vBox, 0, Sx);

			AG_LabelNew(vBox, 0, "Exported to ave-save.png:");
			if (AG_SurfaceExportPNG(Sx, "axe-save.png", 0) == 0) {
				if ((D = AG_SurfaceFromPNG("axe-save.png")) != NULL) {
					AG_PixmapFromSurface(vBox, 0, D);
					AG_SurfaceFree(D);
				} else {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				}
			} else {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			}
			AG_SurfaceFree(S);
			AG_SurfaceFree(Sx);
		} else {
			AG_LabelNew(vBox, 0, "%s: %s", path, AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	/*
	 * Test conversion to different bit resolutions.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "agar.png", path, sizeof(path))) {
		AG_LabelNew(vBox, 0, "Here is %s:", path);
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			AG_PixmapFromSurface(vBox, 0, S);
		
			AG_LabelNewS(vBox, 0, "Converted to {16,24,32}bpp:");
			box = AG_BoxNewHoriz(vBox, 0 );
			{
				AG_PixelFormatRGB(&pfTest, 16,
				    0xf000, 0x0f00, 0x00f0);
				Test_Format(S, &pfTest, box);
				
				AG_PixelFormatRGB(&pfTest, 24,
				    0x00ff0000, 0x0000ff00, 0x000000ff);
				Test_Format(S, &pfTest, box);

				AG_PixelFormatRGB(&pfTest, 32,
				    0xff000000, 0x00ff0000, 0x0000ff00);
				Test_Format(S, &pfTest, box);
			}
#if AG_MODEL == AG_LARGE
			AG_LabelNewS(vBox, 0, "Converted to {48,64}bpp:");
			box = AG_BoxNewHoriz(vBox, 0);
			{
				AG_PixelFormatRGB(&pfTest, 48,
				    0xffff000000000000,
				    0x0000ffff00000000,
				    0x00000000ffff0000);
				Test_Format(S, &pfTest, box);
				
				AG_PixelFormatRGBA(&pfTest, 64,
				    0xffff000000000000,
				    0x0000ffff00000000,
				    0x00000000ffff0000,
				    0x000000000000ffff);
				Test_Format(S, &pfTest, box);
			}
#endif
			AG_LabelNewS(vBox, 0, "Converted to Grayscale:");
			box = AG_BoxNewHoriz(vBox, 0);
			{
				AG_PixelFormatGrayscale(&pfTest, 32);
				Test_Format(S, &pfTest, box);
#if AG_MODEL == AG_LARGE
				AG_PixelFormatGrayscale(&pfTest, 64);
				Test_Format(S, &pfTest, box);
#endif
			}

			AG_LabelNew(vBox, 0, "Exported to ave-save.png:");
			if (AG_SurfaceExportPNG(S, "agar-save.png", 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((D = AG_SurfaceFromPNG("agar-save.png")) != NULL) {
					AG_PixmapFromSurface(vBox, 0, D);
					AG_SurfaceFree(D);
				} else {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				}
			}
			AG_SurfaceFree(S);
		} else {
			AG_LabelNew(vBox, 0, "%s: %s", path, AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}
	
	vBox = AG_BoxNewVert(hBox, 0);

	/*
	 * Load/save a PNG file in palettized mode.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "agar-index.png", path, sizeof(path))) {
		AG_LabelNew(vBox, 0, "Here is %s:", path);
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			S->flags |= AG_SURFACE_TRACE;
			AG_PixmapFromSurface(vBox, 0, S);
			AG_LabelNew(vBox, 0, "Exported to agar-index-save.png:");
			if (AG_SurfaceExportPNG(S, "agar-index-save.png", 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((D = AG_SurfaceFromPNG("agar-index-save.png")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, D);
					AG_SurfaceFree(D);
				}
			}
			AG_SurfaceFree(S);
		} else {
			AG_LabelNew(vBox, 0, "Failed: %s", AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	/* Load/save a JPEG file. */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "pepe.jpg", path, sizeof(path))) {
		AG_LabelNew(vBox, 0, "Here is %s:", path);
		if ((S = AG_SurfaceFromJPEG(path)) == NULL) {
			AG_LabelNew(vBox, 0, "Failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(vBox, 0, S);

			AG_LabelNewS(vBox, 0, "Exported to pepe.jpg\n"
			                      "(at 10% quality):");
			if (AG_SurfaceExportJPEG(S, "pepe-save.jpg", 10, 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((D = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, D);
					AG_SurfaceFree(D);
				}
			}
			
			AG_LabelNewS(vBox, 0, "Exported to pepe-save.jpg\n"
			                      "(at 100% quality):");
			if (AG_SurfaceExportJPEG(S, "pepe-save.jpg", 100, 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((D = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, D);
					AG_SurfaceFree(D);
				}
			}
			AG_SurfaceFree(S);
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}
	
	AG_WindowSetGeometry(win, -1, -1, 500, 800);
	return (0);
}

const AG_TestCase imageloadingTest = {
	"imageloading",
	N_("Test image loader / exporter routines"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

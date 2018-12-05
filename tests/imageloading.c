/*	Public domain	*/
/*
 * Test built-in image load/export functions.
 */

#include "agartest.h"

#include "config/datadir.h"

static void
LabelLoaded(void *_Nullable parent, const char *_Nonnull path)
{
	AG_Label *lbl;

	lbl = AG_LabelNew(parent, 0, "Loaded from %s:", path);
	AG_SetStyle(lbl, "font-size", "80%");
}

static void
LabelSaved(void *_Nullable parent, const char *_Nonnull path)
{
	AG_Label *lbl;
	
	lbl = AG_LabelNew(parent, 0, "Exported to %s:", path);
	AG_SetStyle(lbl, "font-size", "80%");
}

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Surface *S, *Ssave;
/*	AG_Scrollview *sv; */
	AG_Box *hBox, *vBox, *bppBox;
	AG_PixelFormat pfTest;
	int i;

/*	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND); */
	hBox = AG_BoxNewHoriz(win, AG_BOX_FRAME);
	vBox = AG_BoxNewVert(hBox, AG_BOX_FRAME);
	
	/*
	 * Load BMP files in different flavors of the format.
	 */
	for (i = 1; i <= 4; i++) {
		if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, AG_Printf("agar-%d.bmp",i),
		    path, sizeof(path))) {
			LabelLoaded(vBox, path);
			if ((S = AG_SurfaceFromBMP(path)) != NULL) {
				AG_PixmapFromSurface(vBox, 0, S);
				AG_SurfaceFree(S);
			} else {
				AG_LabelNew(vBox, 0, "%s: %s", path, AG_GetError());
			}
		} else {
			AG_LabelNewS(vBox, 0, AG_GetError());
		}
	}

	/*
	 * Load, display and export a PNG file in RGBA format.
	 * Export it and load it again.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "axe.png", path, sizeof(path))) {
		LabelLoaded(vBox, path);
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			AG_PixmapFromSurface(vBox, 0, S);
			LabelSaved(vBox, "axe-save.png");
			if (AG_SurfaceExportPNG(S, "axe-save.png", 0) == 0) {
				if ((Ssave = AG_SurfaceFromPNG("axe-save.png")) != NULL) {
					AG_PixmapFromSurface(vBox, 0, Ssave);
					AG_SurfaceFree(Ssave);
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
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	/*
	 * Test conversion to different bit resolutions.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "agar.png", path, sizeof(path))) {
		LabelLoaded(vBox, path);
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			AG_PixmapFromSurface(vBox, 0, S);
		
			AG_LabelNewS(vBox, 0, "Converted to {8,16,24,32}bpp:");
			bppBox = AG_BoxNewHoriz(vBox, 0 );
			{
				const int testDepths[4] = { 8,16,24,32 };
				int i;

				for (i = 0; i < 4; i++) {
					AG_PixelFormatRGB(&pfTest, testDepths[i],
					    0xff000000,
					    0x00ff0000,
					    0x0000ff00);
					if ((Ssave = AG_SurfaceConvert(S, &pfTest)) != NULL) {
						AG_PixmapFromSurface(bppBox, 0, Ssave);
						AG_SurfaceFree(Ssave);
					} else {
						AG_LabelNew(bppBox, 0,
						    "Convert failed: %s", AG_GetError());
					}
					AG_PixelFormatFree(&pfTest);
				}
			}

			LabelSaved(vBox, "axe-save.png");
			if (AG_SurfaceExportPNG(S, "agar-save.png", 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((Ssave = AG_SurfaceFromPNG("agar-save.png")) != NULL) {
					AG_PixmapFromSurface(vBox, 0, Ssave);
					AG_SurfaceFree(Ssave);
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

	/*
	 * Load/save a PNG file in palettized mode.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "agar-index.png",
	    path, sizeof(path))) {
		LabelLoaded(vBox, path);
		if ((S = AG_SurfaceFromPNG(path)) != NULL) {
			S->flags |= AG_SURFACE_TRACE;
			AG_PixmapFromSurface(vBox, 0, S);
			LabelSaved(vBox, "axe-index-save.png");
			if (AG_SurfaceExportPNG(S, "agar-index-save.png", 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((Ssave = AG_SurfaceFromPNG("agar-index-save.png")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, Ssave);
					AG_SurfaceFree(Ssave);
				}
			}
			AG_SurfaceFree(S);
		} else {
			AG_LabelNew(vBox, 0, "Failed: %s", AG_GetError());
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}

	vBox = AG_BoxNewVert(hBox, AG_BOX_FRAME);

	/* Load/save a JPEG file. */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "pepe.jpg", path, sizeof(path))) {
		LabelLoaded(vBox, path);
		if ((S = AG_SurfaceFromJPEG(path)) == NULL) {
			AG_LabelNew(vBox, 0, "Failed: %s", AG_GetError());
		} else {
			AG_PixmapFromSurface(vBox, 0, S);

			LabelSaved(vBox, "pepe.jpg (10% quality)");
			if (AG_SurfaceExportJPEG(S, "pepe-save.jpg", 10, 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((Ssave = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, Ssave);
					AG_SurfaceFree(Ssave);
				}
			}
			
			LabelSaved(vBox, "pepe.jpg (100% quality)");
			if (AG_SurfaceExportJPEG(S, "pepe-save.jpg", 100, 0) == -1) {
				AG_LabelNew(vBox, 0, "Save failed: %s", AG_GetError());
			} else {
				if ((Ssave = AG_SurfaceFromJPEG("pepe-save.jpg")) == NULL) {
					AG_LabelNew(vBox, 0, "Load failed: %s", AG_GetError());
				} else {
					AG_PixmapFromSurface(vBox, 0, Ssave);
					AG_SurfaceFree(Ssave);
				}
			}
			AG_SurfaceFree(S);
		}
	} else {
		AG_LabelNewS(vBox, 0, AG_GetError());
	}
	return (0);
}

const AG_TestCase imageLoadingTest = {
	"imageLoading",
	N_("Test the image loader / exporter routines"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

/*	Public domain	*/
/*
 * Test built-in image load/export functions.
 */

#include <agar/core.h>
#include <agar/gui.h>

static void
Quit(AG_Event *event)
{
	AG_QuitGUI();
}

int
main(int argc, char *argv[])
{
	AG_Surface *su, *su2;
	AG_Window *win;

	if (AG_InitCore("agar-imageloading-demo", 0) == -1) {
		fprintf(stderr, "AG_InitCore: %s\n", AG_GetError());
		return (1);
	}
	if (AG_InitGraphics(NULL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		goto fail;
	}

	{
		AG_Surface *s;

		s = AG_SurfaceRGBA(32, 32, 16, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
		    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
		);
		printf("loss: 0x%x, 0x%x, 0x%x, 0x%x\n",
		    s->format->Rloss,
		    s->format->Gloss,
		    s->format->Bloss,
		    s->format->Aloss);
		printf("shift: %d, %d, %d, %d\n",
		    s->format->Rshift,
		    s->format->Gshift,
		    s->format->Bshift,
		    s->format->Ashift);
	}

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar image loading demo");

	
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

	AG_ButtonNewFn(win, 0, "Quit", Quit, NULL);
	AG_WindowShow(win);
	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}

/*	Public domain	*/
/*
 * This program demonstrates the use of the file loader widget,
 * AG_FileDlg.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

/* Callback routine for AG_FileDlg. */
static void
LoadImage(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	char *file = AG_STRING(1);
	AG_FileType *ft = AG_PTR(2);
	AG_Surface *s;
	AG_Window *win;
	AG_Scrollview *sv;
	Uint8 *pSrc;
	int i;

	if (strcmp(ft->exts[0], "*.bmp") == 0) {
		s = AG_SurfaceFromBMP(file);
	} else if (strcmp(ft->exts[0], "*.jpg") == 0) {
		s = AG_SurfaceFromJPEG(file);
	} else if (strcmp(ft->exts[0], "*.png") == 0) {
		s = AG_SurfaceFromPNG(file);
	} else {
		return;
	}
	if (s == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", file, AG_GetError());
		return;
	}

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Image <%s>", AG_ShortFilename(file));

	/* We use AG_FileOptionFoo() to retrieve per-type options. */
	if (AG_FileOptionBool(ft,"invert")) {
		pSrc = (Uint8 *)s->pixels;
		for (i = 0; i < s->w*s->h; i++) {
			Uint8 r, g, b;

			AG_GetPixelRGB(AG_GET_PIXEL(s,pSrc), s->format,
			    &r,&g,&b);
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			AG_PUT_PIXEL(s, pSrc,
			    AG_MapPixelRGB(s->format, r,g,b));
			pSrc += s->format->BytesPerPixel;
		}
	}

	/*
	 * Place an AG_Pixmap(3) widget inside of an AG_Scrollview(3) so
	 * the user can pan the view.
	 */
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE|AG_SCROLLVIEW_EXPAND);
	AG_PixmapFromSurfaceScaled(sv, 0, s, s->w, s->h);
	AG_SurfaceFree(s);

	AG_WindowSetGeometry(win, -1, -1, s->w+32, s->h+64);
	AG_WindowShow(win);
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_FileDlg *fd;
	AG_FileType *ft;
	AG_Box *box;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Agar loader dialog demo");

	/* Create the file loader widget. */
	fd = AG_FileDlgNew(win, AG_FILEDLG_EXPAND);
	
	/* Set some default directory. */
	AG_FileDlgSetDirectoryMRU(fd, "images-dir", "./Images");
	
	/* Set some default filename. */
	AG_FileDlgSetFilenameS(fd, "Meme.bmp");

	/*
	 * Register the loader functions. We can assign a set of user
	 * options to specific types as well.
	 */
	ft = AG_FileDlgAddType(fd, "Windows Bitmap", "*.bmp", LoadImage, NULL);
	AG_FileOptionNewBool(ft, "Invert colors", "invert", 0);
 
	ft = AG_FileDlgAddType(fd, "JPEG image", "*.jpg,*.jpeg", LoadImage, NULL);
	AG_FileOptionNewBool(ft, "Invert colors", "invert", 0);

	ft = AG_FileDlgAddType(fd, "Portable Network Graphics", "*.png",
	    LoadImage, NULL);
	AG_FileOptionNewBool(ft, "Invert colors", "invert", 0);

	/*
	 * As different file types are selected, FileDlg will automatically
	 * create various widgets for per-type options. We specify where those
	 * widgets will be created here.
	 */
	box = AG_BoxNewVert(win, AG_BOX_HFILL);
	AG_FileDlgSetOptionContainer(fd, box);

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	char *driverSpec = NULL, *optArg;
	int c;

	while ((c = AG_Getopt(argc, argv, "?hd:", &optArg, NULL)) != -1) {
		switch (c) {
		case 'd':
			driverSpec = optArg;
			break;
		case '?':
		case 'h':
		default:
			printf("Usage: loader [-d agar-driver-spec]\n");
			return (1);
		}
	}
	if (AG_InitCore("agar-loader-demo", 0) == -1 ||
	    AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_QuitGUI);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


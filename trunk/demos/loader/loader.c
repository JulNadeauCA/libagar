/*	Public domain	*/
/*
 * This program demonstrates the use of the file loader widget,
 * AG_FileDlg.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifndef _WIN32
#include "config/have_sdl_image.h"
#ifdef HAVE_SDL_IMAGE
#include <SDL_image.h>
#endif
#endif

/* Load a Windows bitmap using the built-in AG_SurfaceFromBMP(). */
static void
LoadBMP(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	char *file = AG_STRING(1);
	AG_FileType *ft = AG_PTR(2);
	AG_Surface *bmp;
	AG_Window *win;
	AG_Scrollview *sv;
	Uint8 *pSrc;
	char *title;
	int i;

	/* Load the bitmap file into a SDL surface. */
	if ((bmp = AG_SurfaceFromBMP(file)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", file, AG_GetError());
		return;
	}
	pSrc = (Uint8 *)bmp->pixels;

	if ((title = strrchr(file, '/')) == NULL) {
		title = file;
	} else {
		title++;
	}
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Image <%s>", title);

	/* We use AG_FileOptionFoo() to retrieve per-type options. */
	if (AG_FileOptionBool(ft,"bmp.invert")) {
		for (i = 0; i < bmp->w*bmp->h; i++) {
			Uint8 r, g, b;

			AG_GetPixelRGB(AG_GET_PIXEL(bmp,pSrc), bmp->format,
			    &r,&g,&b);
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			AG_PUT_PIXEL(bmp, pSrc,
			    AG_MapPixelRGB(bmp->format, r,g,b));
			pSrc += bmp->format->BytesPerPixel;
		}
	}

	/*
	 * Place an AG_Pixmap(3) widget inside of an AG_Scrollview(3) so
	 * the user can pan the view.
	 */
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE|AG_SCROLLVIEW_EXPAND);
	AG_PixmapFromSurfaceScaled(sv, 0, bmp, bmp->w, bmp->h);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win, -1, -1, bmp->w+20, bmp->h+20);
	
	AG_SurfaceFree(bmp);
}

#ifdef HAVE_SDL_IMAGE

/* Load an image using SDL_image. */
static void
LoadIMG(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	char *file = AG_STRING(1);
	AG_FileType *ft = AG_PTR(2);
	SDL_Surface *img;
	AG_Window *win;
	AG_Scrollview *sv;
	char *title;
	int i;

	/* Load the bitmap file into a SDL surface. */
	if ((img = IMG_Load(file)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", file, IMG_GetError());
		return;
	}

	if ((title = strrchr(file, '/')) == NULL) {
		title = file;
	} else {
		title++;
	}
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Image <%s>", title);

	/*
	 * Place an AG_Pixmap(3) widget inside of an AG_Scrollview(3) to
	 * display the image.
	 */
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE|AG_SCROLLVIEW_EXPAND);
	AG_PixmapFromSurface(sv, 0, AG_SurfaceFromSDL(img));

	AG_WindowShow(win);
	AG_WindowSetGeometry(win, -1, -1, img->w+20, img->h+20);
	
	SDL_FreeSurface(img);
}
#endif /* HAVE_SDL_IMAGE */

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
	 * options to each type.
	 */
	ft = AG_FileDlgAddType(fd, "Windows Bitmap", "*.bmp", LoadBMP, NULL);
	AG_FileOptionNewBool(ft, "Invert bitmap", "bmp.invert", 0);
#ifdef HAVE_SDL_IMAGE
	AG_FileDlgAddType(fd, "JFIF format", "*.jpg,*.jpeg", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "Portable Network Graphics", "*.png", LoadIMG,
	    NULL);
	AG_FileDlgAddType(fd, "Graphics Interchange", "*.gif", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "GIMP Native", "*.xcf", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "Tagged Image File", "*.tif,*.tiff", LoadIMG,
	    NULL);
	AG_FileDlgAddType(fd, "X11 Pixmap", "*.xpm", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "Portable Anymap", "*.pnm", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "IBM PC Paintbrush", "*.pcx", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "TrueVision Targa", "*.tga", LoadIMG, NULL);
	AG_FileDlgAddType(fd, "Interleaved Bitmap", "*.lbm,*.iff", LoadIMG,
	    NULL);
#endif /* HAVE_SDL_IMAGE */

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
	AG_BindGlobalKey(AG_KEY_ESCAPE, AG_KEYMOD_ANY, AG_Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);
	CreateWindow();
#ifndef HAVE_SDL_IMAGE
	AG_TextWarning("sdl-image-warning",
	   "The SDL_image library was not found.\n"
	   "Without SDL_image, this demo will only load files in .bmp format.");
#endif
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


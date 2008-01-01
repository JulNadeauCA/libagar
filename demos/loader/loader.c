/*	Public domain	*/
/*
 * This program demonstrates the use of the file loader widget,
 * AG_FileDlg.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

#include "config/have_sdl_image.h"
#ifdef HAVE_SDL_IMAGE
#include <SDL_image.h>
#endif

/* Load a Windows bitmap using the built-in SDL_LoadBMP(). */
static void
LoadBMP(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	char *file = AG_STRING(1);
	AG_FileType *ft = AG_PTR(2);
	SDL_Surface *bmp;
	AG_Window *win;
	Uint8 *pSrc;
	char *title;
	int i;

	/* Load the bitmap file into a SDL surface. */
	if ((bmp = SDL_LoadBMP(file)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", file, SDL_GetError());
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

			SDL_GetRGB(AG_GET_PIXEL(bmp,pSrc), bmp->format,
			    &r, &g, &b);
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			AG_PUT_PIXEL(bmp, pSrc, SDL_MapRGB(bmp->format,
			    r, g, b));
			pSrc += bmp->format->BytesPerPixel;
		}
	}

	/*
	 * Add a pixmap widget displaying the bitmap, scaled down to the
	 * maximum view size if needed.
	 */
	AG_PixmapFromSurfaceScaled(win, 0, bmp,
	    (bmp->w < (agView->w - 16)) ? bmp->w : agView->w - 16,
	    (bmp->h < (agView->h - 40)) ? bmp->h : agView->h - 40);

	SDL_FreeSurface(bmp);
	AG_WindowShow(win);
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
	 * Add a pixmap widget displaying the bitmap, scaled down to the
	 * maximum view size if needed.
	 */
	AG_PixmapFromSurfaceScaled(win, 0, img,
	    (img->w < (agView->w - 16)) ? img->w : agView->w - 16,
	    (img->h < (agView->h - 40)) ? img->h : agView->h - 40);
	SDL_FreeSurface(img);
	AG_WindowShow(win);
}
#endif /* HAVE_SDL_IMAGE */

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_FileDlg *fd;
	AG_FileType *ft;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Image loader");

	/* Create the file loader widget. */
	fd = AG_FileDlgNew(win, AG_FILEDLG_EXPAND);
	
	/* Set some default directory. */
	AG_FileDlgSetDirectoryMRU(fd, "images-dir", "./Images");
	
	/* Set some default filename. */
	AG_FileDlgSetFilename(fd, "Meme.bmp");

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
	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("loader-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}


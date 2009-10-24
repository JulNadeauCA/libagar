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

			AG_GetRGB(AG_GET_PIXEL(bmp,pSrc), bmp->format,
			    &r,&g,&b);
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			AG_PUT_PIXEL(bmp, pSrc, AG_MapRGB(bmp->format, r,g,b));
			pSrc += bmp->format->BytesPerPixel;
		}
	}

	/*
	 * Place an AG_Pixmap(3) widget inside of an AG_Scrollview(3) so
	 * the user can pan the view.
	 */
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE);
	AG_PixmapFromSurfaceScaled(sv, 0, bmp, bmp->w, bmp->h);
	AG_Expand(sv);

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 60);
	AG_WindowShow(win);
	
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
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE);
	AG_PixmapFromSurface(sv, 0, AG_SurfaceFromSDL(img));
	AG_Expand(sv);

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 60, 60);
	AG_WindowShow(win);
	
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
	AG_WindowSetCaption(win, "Image loader");

	/* Create the file loader widget. */
	fd = AG_FileDlgNew(win, 0);
	AG_Expand(fd);
	
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
	box = AG_BoxNewVert(win, 0);
	AG_ExpandHoriz(box);
	AG_FileDlgSetOptionContainer(fd, box);

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	Uint flags = AG_VIDEO_RESIZABLE;

	if (AG_InitCore("agar-loader-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (argc >= 2) {
		if (strcmp(argv[1], "-g") == 0) {
			printf("Forcing GL mode\n");
			flags |= AG_VIDEO_OPENGL;
		}
	}
	
	if (AG_InitVideo(640, 480, 32, flags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
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


/*	Public domain	*/
/*
 * This program demonstrates the use of the file loader widget,
 * AG_FileDlg(3).
 */

#include "agartest.h"

#include <string.h>

#include "config/datadir.h"

/* Callback routine for AG_FileDlg. */
static int
LoadImage(AG_Event *event)
{
/*	AG_FileDlg *fd = AG_SELF(); */
	AG_Window *winParent = AG_PTR(1);
	char *file = AG_STRING(2);
	AG_FileType *ft = AG_PTR(3);
	AG_Surface *S;
	AG_Window *win;
	AG_Scrollview *sv;

	if (strcmp(ft->exts[0], "*.bmp") == 0) {
		S = AG_SurfaceFromBMP(file);
	} else if (strcmp(ft->exts[0], "*.jpg") == 0) {
		S = AG_SurfaceFromJPEG(file);
	} else if (strcmp(ft->exts[0], "*.png") == 0) {
		S = AG_SurfaceFromPNG(file);
	} else {
		AG_SetError("Unrecognized format: %s", ft->exts[0]);
		return (-1);
	}
	if (S == NULL)
		return (-1);

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_SurfaceFree(S);
		return (-1);
	}
	AG_WindowSetCaption(win, "Image <%s>", AG_ShortFilename(file));

	/*
	 * Process type-specific options.
	 */
	if (AG_FileOptionBool(ft,"invert") == 1) {	/* Invert all pixels */
		Uint8 *pSrc = S->pixels;
		Uint8 *pEnd = &S->pixels[S->h*S->pitch];

		while (pSrc < pEnd) {
			AG_Color c;
			
			AG_GetColor(&c, AG_SurfaceGet_At(S,pSrc), &S->format);
			c.r = AG_COLOR_LAST - c.r;
			c.g = AG_COLOR_LAST - c.g;
			c.b = AG_COLOR_LAST - c.b;
			AG_SurfacePut_At(S,pSrc,
			    AG_MapPixel_RGBA(&S->format, c.r, c.g, c.b, c.a));
			pSrc += S->format.BytesPerPixel;
		}
	}

	/*
	 * Place an AG_Pixmap(3) widget inside of an AG_Scrollview(3) so
	 * the user can pan the view.
	 */
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE|AG_SCROLLVIEW_EXPAND);
	AG_PixmapFromSurfaceScaled(sv, 0, S, S->w, S->h);
	AG_SurfaceFree(S);

	AG_WindowSetGeometry(win, -1, -1, 320, 240);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_FileDlg *fd;
	AG_DirDlg *dd;
	AG_FileType *ft[3];
	AG_Box *box;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	int i;

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAdd(nb, "Load file", AG_BOX_VERT);
	{
		/* Create the file loader widget. */
		fd = AG_FileDlgNew(ntab, AG_FILEDLG_EXPAND);
	
		/* Set some default directory. */
		if (strcmp(DATADIR, "NONE") != 0) {
			AG_FileDlgSetDirectoryMRU(fd, "images-dir", DATADIR);
		} else {
			AG_FileDlgSetDirectoryMRU(fd, "images-dir", ".");
		}
	
		/* Set some default filename. */
		AG_FileDlgSetFilenameS(fd, "agar.bmp");

		/*
		 * Register the loader functions.
		 */
		ft[0] = AG_FileDlgAddType(fd, "Windows Bitmap", "*.bmp",		LoadImage, "%p", win);
		ft[1] = AG_FileDlgAddType(fd, "JPEG image", "*.jpg,*.jpeg",		LoadImage, "%p", win);
		ft[2] = AG_FileDlgAddType(fd, "Portable Network Graphics", "*.png",	LoadImage, "%p", win);

		/*
		 * Register a boolean option called "invert" on all types.
		 */
		for (i = 0; i < 3; i++)
			AG_FileOptionNewBool(ft[i], "Inverted", "invert", 0);
	
		/*
		 * As different file types are selected, FileDlg may create
		 * widgets for type specific options. We specify where those
		 * widgets will be created here.
		 */
		box = AG_BoxNewVert(ntab, AG_BOX_HFILL|AG_BOX_FRAME);
		AG_BoxSetPadding(box, 10);
		AG_FileDlgSetOptionContainer(fd, box);
	}
	ntab = AG_NotebookAdd(nb, "Select directory", AG_BOX_VERT);
	{
		/* Create the directory selector widget. */
		dd = AG_DirDlgNew(ntab, AG_DIRDLG_EXPAND);
	
		/* Set some default directory. */
		AG_DirDlgSetDirectoryMRU(dd, "images-dir", "./Images");
	}

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	return (0);
}

const AG_TestCase loaderTest = {
	"loader",
	N_("Test the AG_FileDlg(3) file selection widget"),
	"1.5.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

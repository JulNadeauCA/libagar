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
	AG_Surface *s;
	AG_Window *win;
	AG_Scrollview *sv;
	Uint8 *pSrc;
	Uint i;

	if (strcmp(ft->exts[0], "*.bmp") == 0) {
		s = AG_SurfaceFromBMP(file);
	} else if (strcmp(ft->exts[0], "*.jpg") == 0) {
		s = AG_SurfaceFromJPEG(file);
	} else if (strcmp(ft->exts[0], "*.png") == 0) {
		s = AG_SurfaceFromPNG(file);
	} else {
		AG_SetError("Unrecognized format: %s", ft->exts[0]);
		return (-1);
	}
	if (s == NULL)
		return (-1);

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_SurfaceFree(s);
		return (-1);
	}
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
		 * Register the loader functions. We can assign a set of user
		 * specified options to specific types as well.
		 */
		ft[0] = AG_FileDlgAddType(fd, "Windows Bitmap", "*.bmp",		LoadImage, "%p", win);
		ft[1] = AG_FileDlgAddType(fd, "JPEG image", "*.jpg,*.jpeg",		LoadImage, "%p", win);
		ft[2] = AG_FileDlgAddType(fd, "Portable Network Graphics", "*.png",	LoadImage, "%p", win);

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

/*	Public domain	*/
/*
 * This program demonstrates the use of the file loader widget,
 * AG_FileDlg(3).
 */

#include "agartest.h"

#include <string.h>

#include "config/datadir.h"

/* Callback routine for AG_FileDlg. */
static void
LoadImage(AG_Event *event)
{
/*	AG_FileDlg *fd = AG_FILEDLG_SELF(); */
	AG_Window *winParent = AG_WINDOW_PTR(1);
	char *file = AG_STRING(2);
	AG_FileType *ft = AG_PTR(3);
	AG_Surface *S;
	AG_Window *win;
	AG_Box *box;

	if ((S = AG_SurfaceFromFile(file)) == NULL) {
		AG_TextMsgFromError();
		return;
	}

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_SurfaceFree(S);
		AG_TextMsgFromError();
		return;
	}
	AG_WindowSetCaptionS(win, AG_ShortFilename(file));

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

	box = AG_BoxNewVert(win, AG_BOX_HFILL);
	AG_SetStyle(box, "font-size", "90%");
	{
		AG_LabelNew(box, 0,
		    AGSI_COURIER "%s" AGSI_RST " (%u x %u x %d bpp):",
		    AG_ShortFilename(file),
		    S->w, S->h, S->format.BitsPerPixel);
	}


	if (S->w > 320 && S->h > 240) { 			/* "Large" */
		AG_Scrollview *sv;

		sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_BY_MOUSE |
		                           AG_SCROLLVIEW_PAN_LEFT |
		                           AG_SCROLLVIEW_PAN_RIGHT |
		                           AG_SCROLLVIEW_EXPAND);
		AG_PixmapFromSurfaceScaled(sv, 0, S, S->w, S->h);
	} else {						/* Compact */
		AG_PixmapFromSurfaceScaled(win, 0, S, S->w, S->h);
	}
	
	AG_SurfaceFree(S);

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Box *box;
	AG_Notebook *nb;
	AG_NotebookTab *nt;

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Load file\n(a FileDlg)"), AG_BOX_VERT);
	{
		AG_FileDlg *fd;
		AG_FileType *ft;
		
		AG_LabelNewS(nt, 0, _("Load an image with AG_FileDlg(3):"));

		/* Create the file loader. Mask files by extension by default. */
		fd = AG_FileDlgNew(nt, AG_FILEDLG_MASK_EXT | AG_FILEDLG_EXPAND);
	
		/* Set some default directory. */
		if (strcmp(DATADIR, "NONE") != 0) {
			AG_FileDlgSetDirectoryMRU(fd, "images-dir", DATADIR);
		} else {
			AG_FileDlgSetDirectoryMRU(fd, "images-dir", ".");
		}
	
		/* Set some default filename. */
		AG_FileDlgSetFilenameS(fd, "agar.bmp");

		/*
		 * Register the supported file formats.
		 */
		AG_FileDlgAddImageTypes(fd, LoadImage, "%p", win);

		/*
		 * Register extra types:
		 * ft = AG_FileDlgAddType(fd, "My custom type", "*.foo",
		 *                        LoadFoo, "%p", win);
		 */

		/*
		 * Create a new boolean option "Invert color", applicable to
		 * all image formats.
		 */
		TAILQ_FOREACH(ft, &fd->types, types) {
			AG_FileOptionNewBool(ft, _("Invert color"), "invert", 0);
		}
	
		/*
		 * As different file types are selected, FileDlg may create
		 * widgets for type specific options. We specify where those
		 * widgets will be created here.
		 */
		box = AG_BoxNewVert(nt, AG_BOX_HFILL);
		AG_SetStyle(box, "padding", "10");
		AG_FileDlgSetOptionContainer(fd, box);
	}
	nt = AG_NotebookAdd(nb, _("Select directory\n(a DirDlg)"), AG_BOX_VERT);
	{
		AG_DirDlg *dd;

		AG_LabelNewS(nt, 0, _("Pick a directory with AG_DirDlg(3):"));

		/* Create the directory selector widget. */
		dd = AG_DirDlgNew(nt, AG_DIRDLG_EXPAND);
	
		/* Set some default directory. */
		AG_DirDlgSetDirectoryMRU(dd, "images-dir", "./Images");
	}

	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	return (0);
}

const AG_TestCase loaderTest = {
	"loader",
	N_("Test the AG_FileDlg(3) file selection widget"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

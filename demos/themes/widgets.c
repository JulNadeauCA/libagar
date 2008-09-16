/*	Public domain	*/
/*
 * This application displays a couple of Agar-GUI widgets. It is useful for
 * testing new themes (see AG_Style(3)), but it does not demonstrate widget
 * functionality in any useful way. If you have libjpeg installed, F8 will
 * generate a screenshot.
 *
 * See the README file for a description of the various command-line options
 * available.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include <stdio.h>
#include <math.h>

#include "config/have_getopt.h"

#include "rounded_style.h"
#include "doublebuf.h"

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_HBox *hbox;
	AG_VBox *vbox;
	AG_Pane *pane;
	AG_Combo *com;
	AG_UCombo *ucom;
	AG_Box *div1, *div2;
	AG_Textbox *tbox;
	int i;

	/*
	 * Create a new window and attach widgets to it. The Window object
	 * is simply a container widget that packs its children vertically.
	 */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Some Agar-GUI widgets");
	
	/*
	 * Pane provides two Box containers which can be resized using
	 * a control placed in the middle.
	 *
	 * The MPane widget also provides a set of preconfigured layouts
	 * for multiple pane views.
	 */
	pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	div1 = pane->div[0];
	div2 = pane->div[1];
	{
		AG_Label *lbl;

		/* The Pixmap widget displays a raster surface. */
		AG_PixmapFromBMP(div1, 0, "agar.bmp");
	
		/*
		 * The Label widget provides a simple static or polled label
		 * (polled labels use special format strings; see AG_Label(3)
		 * for details).
		 */
		AG_LabelNewStatic(div1, 0, "This is a static label");

		lbl = AG_LabelNewPolled(div2, AG_LABEL_FRAME,
		    "This is a polled label (x=%i)", &pane->dx);
		AG_LabelSizeHint(lbl, 1,
		    "This is a polled label (x=1234)");
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
	}

	/*
	 * HBox is a container which aligns its children horizontally. Both
	 * HBox and VBox are derived from the Box object.
	 */
	hbox = AG_HBoxNew(div1, AG_HBOX_HFILL|AG_HBOX_HOMOGENOUS);
	{
		/*
		 * The Button widget is a simple push-button. It is typically
		 * used to trigger events, but it can also bind its state to
		 * an boolean (integer) value or a bitmask.
		 */
		for (i = 0; i < 5; i++)
			AG_ButtonNew(hbox, 0, "x");
	}

	hbox = AG_HBoxNew(div1, AG_HBOX_HFILL);
	{
		/* The Radio checkbox is a group of radio buttons. */
		{
			const char *radioItems[] = {
				"Radio1",
				"Radio2",
				NULL
			};
			AG_RadioNew(hbox, AG_RADIO_EXPAND, radioItems);
		}
	
		vbox = AG_VBoxNew(hbox, 0);
		{
			/*
			 * The Checkbox widget can bind to boolean values
			 * and bitmasks.
			 */
			AG_CheckboxNew(vbox, 0, "Checkbox 1");
			AG_CheckboxNew(vbox, 0, "Checkbox 2");
		}
	}

	/* Separator simply renders horizontal or vertical line. */
	AG_SeparatorNew(div1, AG_SEPARATOR_HORIZ);

	/*
	 * The Combo widget is a textbox widget with a expander button next
	 * to it. The button triggers a popup window which displays a list
	 * (using the AG_Tlist(3) widget).
	 */
	com = AG_ComboNew(div1, AG_COMBO_HFILL, "Combo: ");
	AG_ComboSizeHint(com, "Item #00 ", 10);

	/* UCombo is a variant of Combo which looks like a single button. */
	ucom = AG_UComboNew(div1, AG_UCOMBO_HFILL);

	/* Populate the Tlist displayed by the combo widgets we just created. */
	for (i = 0; i < 50; i++) {
		AG_TlistAdd(com->list, NULL, "Item #%d", i);
		AG_TlistAdd(ucom->list, NULL, "Item #%d", i);
	}

	/*
	 * Numerical binds to an integral or floating-point number.
	 * It can also provides built-in unit conversion (see AG_Units(3)).
	 */
	{
		AG_Numerical *num;
		static float myFloat = 1.0;
		static int myMin = 0, myMax = 10, myInt = 1;

		num = AG_NumericalNew(div1, AG_NUMERICAL_HFILL, "cm", "Real: ");
		AG_WidgetBindFloat(num, "value", &myFloat);
		num = AG_NumericalNew(div1, AG_NUMERICAL_HFILL, NULL, "Int: ");
		AG_WidgetBindInt(num, "value", &myInt);
	}

	/*
	 * Textbox is a single or multiline text edition widget. It can bind
	 * to a fixed-size buffer and supports UTF-8.
	 */
	AG_TextboxNew(div1, AG_TEXTBOX_HFILL, "Enter text: ");

	/*
	 * Scrollbar provides three bindings, "value", "min" and "max",
	 * which we can bind to integers or floating-point variables.
	 * Progressbar and Slider have similar interfaces.
	 */
	{
		static int myVal = 50, myMin = -100, myMax = 100, myVisible = 0;
		AG_Scrollbar *sb;
		AG_Slider *sl;
		AG_Statusbar *st;
		AG_ProgressBar *pb;

		sb = AG_ScrollbarNewInt(div1, AG_SCROLLBAR_HORIZ,
		    AG_SCROLLBAR_HFILL|AG_SCROLLBAR_FOCUSABLE,
		    &myVal, &myMin, &myMax, &myVisible);
		AG_ScrollbarSetIntIncrement(sb, 10);

		sl = AG_SliderNewInt(div1, AG_SLIDER_HORIZ,
		    AG_SLIDER_HFILL|AG_SCROLLBAR_FOCUSABLE,
		    &myVal, &myMin, &myMax);
		AG_SliderSetIntIncrement(sl, 10);

		pb = AG_ProgressBarNewInt(div1, AG_PROGRESS_BAR_HORIZ,
		    AG_PROGRESS_BAR_SHOW_PCT,
		    &myVal, &myMin, &myMax);

		/* Statusbar displays one or more text labels. */
		st = AG_StatusbarNew(div1, 0);
		AG_StatusbarAddLabel(st, AG_LABEL_POLLED, "Value = %d",
		    &myVal);
	}

	/*
	 * Notebook provides multiple containers which can be selected by
	 * the user.
	 */
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Table *table;

		nb = AG_NotebookNew(div2, AG_NOTEBOOK_EXPAND);

		ntab = AG_NotebookAddTab(nb, "Table", AG_BOX_VERT);
		{
			float f;

			/*
			 * AG_Table displays a set of cells organized in
			 * rows and columns. It is optimized for cases where
			 * the table is static or needs to be repopulated
			 * periodically.
			 */
			table = AG_TableNew(ntab, AG_TABLE_EXPAND);
			AG_TableAddCol(table, "x", "<8888>", NULL);
			AG_TableAddCol(table, "sin(x)", "<8888>", NULL);
			AG_TableAddCol(table, "cos(x)", NULL, NULL);
			for (f = 0; f < 60; f += 0.3) {
				/*
				 * Insert a Table row for sin(f) and cos(f).
				 * The directives of the format string are
				 * documented in AG_Table(3).
				 */
				AG_TableAddRow(table, "%.02f:%.02f:%.02f",
				    f, sin(f), cos(f));
			}
		}
		
		ntab = AG_NotebookAddTab(nb, "Tlist", AG_BOX_VERT);
		{
			AG_Tlist *tl;
			AG_TlistItem *ti;

			/* The Tlist widget displays a tree or list of items. */
			tl = AG_TlistNew(ntab, AG_TLIST_EXPAND|AG_TLIST_TREE);
			ti = AG_TlistAdd(tl, agIconDoc.s, "Category");
			ti->depth = 0;
			ti = AG_TlistAdd(tl, agIconDoc.s, "Science");
			ti->depth = 1;
			ti = AG_TlistAdd(tl, agIconDoc.s, "Culture");
			ti->depth = 1;
			ti = AG_TlistAdd(tl, agIconDoc.s, "Art");
			ti->depth = 2;
			ti = AG_TlistAdd(tl, agIconDoc.s, "Craft");
			ti->depth = 2;
		}
		
		ntab = AG_NotebookAddTab(nb, "Text", AG_BOX_VERT);
		{
			char *someText;
			size_t size, bufSize;
			FILE *f;

			/*
			 * Textboxes with the MULTILINE flag provide basic
			 * text edition functionality. The CATCH_TAB flag
			 * causes the widget to receive TAB key events (which
			 * are normally used to focus other widget).
			 */
			tbox = AG_TextboxNew(ntab, AG_TEXTBOX_EXPAND|
			                           AG_TEXTBOX_MULTILINE|
						   AG_TEXTBOX_CATCH_TAB, NULL);
			AG_WidgetSetFocusable(tbox, 1);

			/*
			 * Load the contents of this file into a buffer. Make
			 * the buffer a bit larger so the user can try
			 * entering text.
			 */
			if ((f = fopen("widgets.c", "r")) != NULL) {
				fseek(f, 0, SEEK_END);
				size = ftell(f);
				fseek(f, 0, SEEK_SET);
				bufSize = size+1024;
				someText = AG_Malloc(bufSize);
				fread(someText, size, 1, f);
				fclose(f);
				someText[size] = '\0';
			} else {
				someText = AG_Strdup("Failed to load "
				                     "widgets.c");
			}
	
			/*
			 * Bind the buffer's contents to the Textbox. The
			 * size argument to AG_TextboxBindUTF8() must include
			 * space for the terminating NUL.
			 */
			AG_TextboxBindUTF8(tbox, someText, bufSize);
			AG_TextboxSetCursorPos(tbox, 0);
		}
	}

	/* Override default window sizing. */
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 80, 80);
	
	AG_WindowShow(win);
}

static void
Quit(AG_Event *event)
{
	SDL_Event nev;

	/* Terminate the application. */
	nev.type = SDL_USEREVENT;
	SDL_PushEvent(&nev);
}

static void
Preferences(AG_Event *event)
{
	DEV_ConfigShow();
}

static void
GuiDebugger(AG_Event *event)
{
	AG_WindowShow(DEV_GuiDebugger());
}

static void
SetTheme(AG_Event *event)
{
	AG_Style *style = AG_PTR(1);
	AG_SetStyle(agView, style);
}

int
main(int argc, char *argv[])
{
	AG_AgarVersion ver;
	extern AG_Style myRoundedStyle;
	AG_Menu *appMenu;
	AG_MenuItem *m;
	AG_Window *win;
	int w = 640, h = 480, fps = -1;
	int useDoubleBuf = 0;
	int guiFlags = AG_VIDEO_RESIZABLE;

	/* Initialize Agar-Core. */
	if (AG_InitCore("agar-widgets-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	
	/* Fetch Agar version information. */
	AG_GetVersion(&ver);

#ifdef HAVE_GETOPT
	{
		int c;

		while ((c = getopt(argc, argv, "?vgsw:h:fbBt:T:r:R")) != -1) {
			extern char *optarg;

			switch (c) {
			case 'v':
				/* Display Agar version information */
				printf("Agar version: %d.%d.%d\n", ver.major,
				    ver.minor, ver.patch);
				printf("Release name: \"%s\"\n", ver.release);
				exit(0);
			case 'g':
				/* Force OpenGL mode */
				AG_SetBool(agConfig, "view.opengl", 1);
				guiFlags |= AG_VIDEO_OPENGL;
				break;
			case 's':
				/* Force SDL mode */
				AG_SetBool(agConfig, "view.opengl", 0);
				break;
			case 'w':
				/* Set display width in pixels */
				w = atoi(optarg);
				break;
			case 'h':
				/* Set display height in pixels */
				h = atoi(optarg);
				break;
			case 'f':
				/* Force full screen */
				AG_SetBool(agConfig, "view.full-screen", 1);
				break;
			case 'r':
				/* Change default refresh rate */
				fps = atoi(optarg);
				break;
			case 'b':
				/* Force use of the FreeType font engine */
				AG_SetBool(agConfig, "font.freetype", 0);
				break;
			case 'B':
				/* Force use of the bitmap font engine */
				AG_SetBool(agConfig, "font.freetype", 1);
				break;
			case 'T':
				/* Set an alternate font directory */
				AG_SetString(agConfig, "font-path", "%s",
				    optarg);
				break;
			case 't':
				/* Change the default font */
				AG_TextParseFontSpec(optarg);
				break;
			case 'R':
				/* Disable resizable window. */
				guiFlags &= ~(AG_VIDEO_RESIZABLE);
				break;
			case '?':
			default:
				printf("%s [-vgsDdfbBR] [-r fps] [-t fontspec] "
				       "[-w width] [-h height] "
				       "[-T font-path]\n",
				       agProgName);
				exit(0);
			}
		}
	}
#endif
	
	/* Initialize Agar-GUI. */
	if (AG_InitVideo(w, h, 32, guiFlags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}

	/* Change the default refresh rate. */
	AG_SetRefreshRate(fps);

	/* Bind some useful accelerator keys. */
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Initialize the Agar-DEV library. */
	DEV_InitSubsystem(0);

	/* Initialize our custom theme. */
	InitMyRoundedStyle(&myRoundedStyle);

	/* Display the version and current graphics driver in use. */
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	{
		AG_Label *lbl;

		lbl = AG_LabelNew(win, 0,
		    "Agar version: %d.%d.%d\n(%s)\n"
		    "Graphics driver: %s",
		    ver.major, ver.minor, ver.patch, ver.release,
		    AG_GetBool(agConfig,"view.opengl") ? "OpenGL" : "SDL");
		AG_LabelJustify(lbl, AG_TEXT_CENTER);

		AG_WindowSetPosition(win, AG_WINDOW_LOWER_CENTER, 0);
		AG_WindowShow(win);
	}

	/* Create an application menu. */
	appMenu = AG_MenuNewGlobal(0);
	m = AG_MenuNode(appMenu->root, "File", NULL);
	{
		AG_MenuAction(m, "Preferences...", NULL, Preferences, NULL);
		AG_MenuAction(m, "GUI Debugger...", NULL, GuiDebugger, NULL);
		AG_MenuAction(m, "Quit", NULL, Quit, NULL);
	}
	m = AG_MenuNode(appMenu->root, "Themes", NULL);
	{
		AG_MenuAction(m, "Default", NULL,
		    SetTheme, "%p", &agStyleDefault);
		AG_MenuAction(m, "Rounded", NULL,
		    SetTheme, "%p", &myRoundedStyle);
	}

	/* Create our test window. */
	CreateWindow();
	
	/* Load our custom color scheme. */
	if (AG_ColorsLoad("green.acs") == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Failed to load color scheme: %s",
		    AG_GetError());
	}

	/* Use the stock event loop. */
	AG_EventLoop();

	AG_Destroy();
	return (0);
}


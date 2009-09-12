/*	Public domain	*/
/*
 * This application displays a couple of standard Agar-GUI widgets. It is
 * useful for testing the GUI display and trying different themes (see
 * AG_Style(3) for details). See README file for a description of the
 * various command-line options provided.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include <stdio.h>
#include <math.h>

#include "config/have_getopt.h"
#include <agar/config/ag_debug.h>

#include "mytheme.h"

static void
ComboSelected(AG_Event *event)
{
	AG_TlistItem *ti = AG_PTR(1);

	AG_TextMsg(AG_MSG_INFO, "Item %s", ti->text);
}

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_Box *hBox, *vBox;
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
	pane = AG_PaneNew(win, AG_PANE_HORIZ, 0);
	AG_PaneSetDivisionMin(pane, 0, 50, 100);
	AG_Expand(pane);
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
		AG_LabelNew(div1, 0, "This is a static label");

		lbl = AG_LabelNewPolled(div2, AG_LABEL_FRAME,
		    "This is a polled label (x=%i)", &pane->dx);
		AG_LabelSizeHint(lbl, 1,
		    "This is a polled label (x=1234)");
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
	}

	/*
	 * Box is a general-purpose widget container. We use AG_BoxNewHoriz()
	 * for horizontal widget packing.
	 */
	hBox = AG_BoxNewHoriz(div1, AG_BOX_HOMOGENOUS|AG_BOX_FRAME);
	AG_ExpandHoriz(hBox);
	{
		/*
		 * The Button widget is a simple push-button. It is typically
		 * used to trigger events, but it can also bind its state to
		 * an boolean (integer) value or a bitmask.
		 */
		for (i = 0; i < 5; i++)
			AG_ButtonNew(hBox, 0, "%c", 0x41+i);
	}

	hBox = AG_BoxNewHoriz(div1, 0);
	AG_ExpandHoriz(hBox);
	{
		/* The Radio checkbox is a group of radio buttons. */
		{
			const char *radioItems[] = {
				"Radio1",
				"Radio2",
				NULL
			};
			AG_Radio *rad;

			rad = AG_RadioNew(hBox, 0, radioItems);
			AG_Expand(rad);
		}
	
		vBox = AG_BoxNewVert(hBox, 0);
		{
			/*
			 * The Checkbox widget can bind to boolean values
			 * and bitmasks.
			 */
			AG_CheckboxNew(vBox, 0, "Checkbox 1");
			AG_CheckboxNew(vBox, 0, "Checkbox 2");
		}
	}

	/* Separator simply renders horizontal or vertical line. */
	AG_SeparatorNew(div1, AG_SEPARATOR_HORIZ);

	/*
	 * The Combo widget is a textbox widget with a expander button next
	 * to it. The button triggers a popup window which displays a list
	 * (using the AG_Tlist(3) widget).
	 */
	com = AG_ComboNew(div1, 0, "Combo: ");
	AG_ExpandHoriz(com);
	AG_ComboSizeHint(com, "Item #00 ", 10);
	AG_SetEvent(com, "combo-selected", ComboSelected, NULL);

	/* UCombo is a variant of Combo which looks like a single button. */
	ucom = AG_UComboNew(div1, 0);
	AG_ExpandHoriz(ucom);

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

		num = AG_NumericalNewS(div1, 0, "cm", "Real: ");
		AG_ExpandHoriz(num);
		AG_BindFloat(num, "value", &myFloat);

		num = AG_NumericalNewS(div1, 0, NULL, "Int: ");
		AG_ExpandHoriz(num);
		AG_BindInt(num, "value", &myInt);
	}

	/*
	 * Textbox is a single or multiline text edition widget. It can bind
	 * to a fixed-size buffer and supports UTF-8.
	 */
	AG_TextboxNew(div1, 0, "Enter text: ");

	/*
	 * Scrollbar provides three bindings, "value", "min" and "max",
	 * which we can bind to integers or floating-point variables.
	 * Progressbar and Slider have similar interfaces.
	 */
	{
		static int myVal = 50, myMin = -100, myMax = 100, myVisible = 0;
		AG_Scrollbar *sb;
		AG_Slider *sl;
		AG_ProgressBar *pb;

		sb = AG_ScrollbarNewInt(div1, AG_SCROLLBAR_HORIZ, 0,
		    &myVal, &myMin, &myMax, &myVisible);
		AG_ExpandHoriz(sb);
		AG_ScrollbarSetIntIncrement(sb, 10);

		sl = AG_SliderNewInt(div1, AG_SLIDER_HORIZ, 0,
		    &myVal, &myMin, &myMax);
		AG_ExpandHoriz(sl);
		AG_SliderSetIntIncrement(sl, 10);

		pb = AG_ProgressBarNewInt(div1, AG_PROGRESS_BAR_HORIZ,
		    AG_PROGRESS_BAR_SHOW_PCT,
		    &myVal, &myMin, &myMax);
	}

	/*
	 * Notebook provides multiple containers which can be selected by
	 * the user.
	 */
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Table *table;

		nb = AG_NotebookNew(div2, 0);
		AG_Expand(nb);

		ntab = AG_NotebookAddTab(nb, "Table", AG_BOX_VERT);
		{
			float f;

			/*
			 * AG_Table displays a set of cells organized in
			 * rows and columns. It is optimized for cases where
			 * the table is static or needs to be repopulated
			 * periodically.
			 */
			table = AG_TableNew(ntab, 0);
			AG_TableAddCol(table, "x", "33%", NULL);
			AG_TableAddCol(table, "sin(x)", "33%", NULL);
			AG_TableAddCol(table, "cos(x)", "33%", NULL);
			AG_Expand(table);
			for (f = 0.0f; f < 60.0f; f += 0.3f) {
				/*
				 * Insert a Table row for sin(f) and cos(f).
				 * The directives of the format string are
				 * documented in AG_Table(3).
				 */
				AG_TableAddRow(table, "%.02f:%.02f:%.02f",
				    f, sin(f), cos(f));
			}
		}
		
		ntab = AG_NotebookAddTab(nb, "Text", AG_BOX_VERT);
		{
			char *someText;
			size_t size, bufSize;
			FILE *f;

			/*
			 * Textboxes with the MULTILINE flag provide basic
			 * text edition functionality. The CATCH_TAB flag
			 * causes the widget to receive TAB key events
			 * (normally used to focus other widgets).
			 */
			tbox = AG_TextboxNew(ntab, AG_TEXTBOX_MULTILINE|
						   AG_TEXTBOX_CATCH_TAB, NULL);
			AG_Expand(tbox);
			AG_WidgetSetFocusable(tbox, 1);

			/*
			 * Load the contents of this file into a buffer. Make
			 * the buffer a bit larger so the user can try
			 * entering text.
			 */
			if ((f = fopen("themes.c", "r")) != NULL) {
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
				                     "themes.c");
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
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 80, 70);
	
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

#ifdef AG_DEBUG
static void
ShowGuiDebugger(AG_Event *event)
{
	AG_WindowShow(AG_GuiDebugger());
}
#endif

static void
SetTheme(AG_Event *event)
{
	AG_Style *style = AG_PTR(1);

	AG_SetStyle(agView, style);
}

static void
SetColorScheme(AG_Event *event)
{
	char *file = AG_STRING(1);

	if (AG_ColorsLoad(file) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Failed to load color scheme: %s",
		    AG_GetError());
	}
}

static void
TweakColorScheme(AG_Event *event)
{
	int darker = AG_INT(1);
	const int inc = 10;
	int i;

	for (i = 0; i < LAST_COLOR; i++) {
		Uint8 r, g, b, a;

		AG_ColorsGetRGBA(i, &r, &g, &b, &a);
		if (darker) {
			r = (r - inc) > 0 ? (r - inc) : 0;
			g = (g - inc) > 0 ? (g - inc) : 0;
			b = (b - inc) > 0 ? (b - inc) : 0;
		} else {
			r = (r + inc) < 255 ? (r + inc) : 255;
			g = (g + inc) < 255 ? (g + inc) : 255;
			b = (b + inc) < 255 ? (b + inc) : 255;
		}
		AG_ColorsSetRGBA(i, r, g, b, a);
	}
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
	char *colorFile = NULL;

	/* Initialize Agar-Core. */
	if (AG_InitCore("agar-themes-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	
	/* Fetch Agar version information. */
	AG_GetVersion(&ver);

#ifdef HAVE_GETOPT
	{
		int c;

		while ((c = getopt(argc, argv, "?vgsw:h:fbBt:T:r:Rc:")) != -1) {
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
				AG_SetString(agConfig, "font-path", optarg);
				break;
			case 't':
				/* Change the default font */
				AG_TextParseFontSpec(optarg);
				break;
			case 'R':
				/* Disable resizable window. */
				guiFlags &= ~(AG_VIDEO_RESIZABLE);
				break;
			case 'c':
				/* Load color scheme */
				colorFile = optarg;
				break;
			case '?':
			default:
				printf("%s [-vgsDdfbBR] [-r fps] [-t fontspec] "
				       "[-w width] [-h height] "
				       "[-T font-path] [-c colors.acs]\n",
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
		AG_Box *hBox;

		lbl = AG_LabelNew(win, 0,
		    "Using Agar version: %d.%d.%d (\"%s\")\n"
		    "Graphics mode: %s",
		    ver.major, ver.minor, ver.patch, ver.release,
		    AG_GetBool(agConfig,"view.opengl") ?
		    "OpenGL" : "Unaccelerated (framebuffer)");
		AG_ExpandHoriz(lbl);
		AG_LabelJustify(lbl, AG_TEXT_CENTER);

		hBox = AG_BoxNewHoriz(win, 0);
		AG_ExpandHoriz(hBox);
		{
			AG_ButtonNewFn(hBox, 0, "Default theme",
			    SetTheme, "%p", &agStyleDefault);
			AG_ButtonNewFn(hBox, 0, "Custom theme",
			    SetTheme, "%p", &myRoundedStyle);
			
			AG_SeparatorNewVert(hBox);
			
			AG_ButtonNewFn(hBox, 0, "Green",
			    SetColorScheme, "%s", "green.acs");
			AG_ButtonNewFn(hBox, 0, "Darker",
			    TweakColorScheme, "%i", 1);
			AG_ButtonNewFn(hBox, 0, "Lighter",
			    TweakColorScheme, "%i", 0);
		}

		AG_WindowSetPosition(win, AG_WINDOW_LOWER_CENTER, 0);
		AG_WindowShow(win);
	}

	/* Create an application menu. */
	appMenu = AG_MenuNewGlobal(0);
	m = AG_MenuNode(appMenu->root, "File", NULL);
	{
		AG_MenuAction(m, "Preferences...", agIconGear.s,
		    Preferences, NULL);
#ifdef AG_DEBUG
		AG_MenuAction(m, "GUI Debugger...", agIconMagnifier.s,
		    ShowGuiDebugger, NULL);
#endif
		AG_MenuSeparator(m);
		AG_MenuAction(m, "Quit", agIconClose.s,
		    Quit, NULL);
	}
	m = AG_MenuNode(appMenu->root, "Test", NULL);
	{
		AG_MenuNode(m, "Submenu A", NULL);
		AG_MenuSeparator(m);
		m = AG_MenuNode(m, "Submenu B", NULL);
		AG_MenuNode(m, "Submenu C", NULL);
		AG_MenuNode(m, "Submenu D", NULL);
		AG_MenuNode(m, "Submenu E", NULL);
	}

	/* Create our test window. */
	CreateWindow();
	
	/* Load our custom color scheme. */
	if (colorFile != NULL &&
	    AG_ColorsLoad(colorFile) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Failed to load color scheme: %s",
		    AG_GetError());
	}

	/* Use the stock event loop. */
	AG_EventLoop();

	AG_Destroy();
	return (0);
}


/*	Public domain	*/
/*
 * This application displays a bunch of Agar-GUI widgets. It is useful for
 * testing new themes (see AG_Style(3)). If you have libjpeg installed, F8
 * will generate a screenshot.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/dev.h>

#include <math.h>

#include "rounded_style.h"

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
		lbl = AG_LabelNewPolled(div2, 0,
		    "This is a polled label (x=%i)", &pane->dx);
		AG_LabelSizeHint(lbl, 1,
		    "This is a polled label (x=1234)");
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
	 * which we can bind to integers. Progressbar has a very similar
	 * interface.
	 */
	{
		static int myVal = 50, myMin = 0, myMax = 200;
		AG_Scrollbar *sb;
		AG_Statusbar *st;
		AG_ProgressBar *pb;

		sb = AG_ScrollbarNew(div1, AG_SCROLLBAR_HORIZ,
		    AG_SCROLLBAR_HFILL|AG_SCROLLBAR_FOCUSABLE);
		AG_WidgetBindInt(sb, "min", &myMin);
		AG_WidgetBindInt(sb, "max", &myMax);
		AG_WidgetBindInt(sb, "value", &myVal);

		pb = AG_ProgressBarNewHoriz(div1, AG_PROGRESS_BAR_SHOW_PCT);
		AG_WidgetBindInt(pb, "min", &myMin);
		AG_WidgetBindInt(pb, "max", &myMax);
		AG_WidgetBindInt(pb, "value", &myVal);

		/* Statusbar displays one or more AG_Label(3) objects. */
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
		}
		
		ntab = AG_NotebookAddTab(nb, "Graph", AG_BOX_VERT);
		{
			AG_Radio *rad;
			AG_FixedPlotter *g;
			AG_FixedPlotterItem *sinplot, *cosplot;
			double f;
			int i = 0;
			
			/*
			 * FixedPlotter displays a plot from a set of integer
			 * values (for floating point data, see SC_Plotter(3)).
			 */
			g = AG_FixedPlotterNew(ntab, AG_FIXED_PLOTTER_LINES,
			                             AG_FIXED_PLOTTER_EXPAND);
			sinplot = AG_FixedPlotterCurve(g, "sin", 0,150,0, 0);
			cosplot = AG_FixedPlotterCurve(g, "cos", 150,150,0, 0);
			for (f = 0; f < 60; f += 0.3) {
				AG_FixedPlotterDatum(sinplot, sin(f)*20.0);
				AG_FixedPlotterDatum(cosplot, cos(f)*20.0);
				/*
				 * Insert a Table row for sin(f) and cos(f).
				 * The directives of the format string are
				 * documented in AG_Table(3).
				 */
				AG_TableAddRow(table, "%.02f:%.02f:%.02f",
				    f, sin(f), cos(f));
			}

			/*
			 * Radio displays a group of radio buttons. It can
			 * bind to an integer value. In this case we bind it
			 * to the "type" enum of the FixedPlotter.
			 */
			rad = AG_RadioNew(ntab, AG_RADIO_HFILL, NULL);
			AG_RadioAddItemHK(rad, SDLK_p, "Points");
			AG_RadioAddItemHK(rad, SDLK_l, "Lines");
			AG_WidgetBindInt(rad, "value", &g->type);
		}
		
		ntab = AG_NotebookAddTab(nb, "Tlist", AG_BOX_VERT);
		{
			AG_Tlist *tl;
			AG_TlistItem *ti;

			/*
			 * The Tlist widget displays either lists or trees.
			 * For flat, polled lists, it is more efficient to use
			 * a Table with a single column, however.
			 */
			tl = AG_TlistNew(ntab, AG_TLIST_EXPAND|AG_TLIST_TREE);
			ti = AG_TlistAdd(tl, agIconDoc.s, "Foo");
			ti->depth = 0;
			ti = AG_TlistAdd(tl, agIconDoc.s, "Bar");
			ti->depth = 1;
			ti = AG_TlistAdd(tl, agIconDoc.s, "Baz");
			ti->depth = 2;
		}
		
		ntab = AG_NotebookAddTab(nb, "Color", AG_BOX_VERT);
		{
			/*
			 * HSVPal is an HSV color picker widget which can
			 * bind to RGB(A) or HSV(A) vectors (integral or real)
			 * or 32-bit pixel values (of a given format).
			 */
			AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		}
	}

#if 0
	/*
	 * AG_GLView provides an OpenGL rendering context. See the "glview",
	 * demo in the Agar ./demos directory for a sample application.
	 *
	 * The higher-level SG(3) scene graph interface also provides a
	 * visualization widget, SG_View(3). Some demos from the ./demos
	 * directory demonstrating its use include "sgview", "linear" and
	 * "lorenz".
	 */
	AG_GLViewNew(win, 0);
#endif
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
	extern AG_Style myRoundedStyle;
	AG_Menu *appMenu;
	AG_MenuItem *m;

	if (AG_InitCore("widgets-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	/* Initialize the Agar-DEV library. */
	DEV_InitSubsystem(0);

	/* Initialize our custom theme. */
	InitMyRoundedStyle(&myRoundedStyle);

	/* Create an application menu. */
	appMenu = AG_MenuNewGlobal(0);
	m = AG_MenuNode(appMenu->root, "File", NULL);
	AG_MenuAction(m, "Preferences...", NULL, Preferences, NULL);
	AG_MenuAction(m, "GUI Debugger...", NULL, GuiDebugger, NULL);
	AG_MenuAction(m, "Quit", NULL, Quit, NULL);
	
	m = AG_MenuNode(appMenu->root, "Themes", NULL);
	AG_MenuAction(m, "Default", NULL, SetTheme, "%p", &agStyleDefault);
	AG_MenuAction(m, "Rounded", NULL, SetTheme, "%p", &myRoundedStyle);
	
	CreateWindow();

	AG_EventLoop();
	AG_Destroy();
	return (0);
}


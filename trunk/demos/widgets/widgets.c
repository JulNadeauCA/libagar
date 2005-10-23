/*	$Csoft: gamemenu.c,v 1.6 2005/10/07 07:09:35 vedge Exp $	*/
/*	Public domain	*/

/*
 * This application displays a bunch of Agar-GUI widgets. It is useful for
 * customizing color schemes.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <unistd.h>
#include <math.h>

static void
CreateWindow(void)
{
	AG_Window *win;
	AG_HBox *hbox;
	AG_HPane *hp;
	AG_HPaneDiv *div;
	AG_Combo *com;
	AG_UCombo *ucom;
	int i;

	/*
	 * The AG_Window widget is a movable container. By default, its children
	 * are aligned vertically.
	 */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Widgets demo");
	
	/*
	 * AG_VPane provides two AG_Box containers, aligned vertically, which
	 * can be resized using a control placed in the middle. Multiple
	 * divisions are allowed. Widgets can attach to div->box1 and div->box2.
	 *
	 * An AG_HPane widget is also available for resizing two containers
	 * aligned horizontally.
	 */
	hp = AG_HPaneNew(win, AG_HPANE_EXPAND);
	div = AG_HPaneAddDiv(hp,
	    AG_BOX_VERT, AG_BOX_HFILL,
	    AG_BOX_VERT, AG_BOX_HFILL|AG_BOX_WFILL);
	AG_BoxSetSpacing(div->box1, 4);
	AG_BoxSetPadding(div->box1, 5);
	{
		/* The AG_Pixmap widget displays a raster surface. */
		AG_PixmapFromBMP(div->box1, "agar.bmp");
	
		/*
		 * The AG_Label widget provides a simple static or
		 * polled label. Note that AG_LABEL_POLLED labels use
		 * a specific type of format string to deal with
		 * pointer alignment issues.
		 */
		AG_LabelNewStatic(div->box1, "This is a static label");
		AG_LabelNew(div->box2, AG_LABEL_POLLED,
		    "This is a polled label (x=%i)", &div->x);
	}

	/*
	 * AG_HBox is a container which aligns its children horizontally. Both
	 * AG_HBox and AG_VBox are wrappers around AG_Box.
	 */
	hbox = AG_HBoxNew(div->box1, AG_HBOX_WFILL|AG_HBOX_HOMOGENOUS);
	{
		/*
		 * The AG_Button widget is a simple push-button. It is typically
		 * used to trigger events, but it can also bind to a boolean
		 * value or a flag.
		 */
		for (i = 0; i < 5; i++)
			AG_ButtonNew(hbox, 0, "x");
	}

	/*
	 * The AG_Checkbox widget is a check box. It can bind to a boolean
	 * value or a flag.
	 */
	AG_CheckboxNew(div->box1, 0, "Checkbox 1");
	AG_CheckboxNew(div->box1, 0, "Checkbox 2");

	/* AG_Separator is a simple visual separator. */
	AG_SeparatorNew(div->box1, AG_SEPARATOR_HORIZ);

	/*
	 * The AG_Combo widget is a textbox widget with a expander button next
	 * to it. The button triggers a popup window which displays a list
	 * (using the AG_Tlist widget).
	 */
	com = AG_ComboNew(div->box1, 0, "Combo: ");

	/*
	 * AG_UCombo is a variant of AG_Combo which looks like a single 
	 * button. It is used by the AG_FSpinbutton widget to set the
	 * conversion unit, for instance.
	 */
	ucom = AG_UComboNew(div->box1);

	/* Populate both combo widgets. */
	for (i = 0; i < 50; i++) {
		AG_TlistAdd(com->list, NULL, "Item #%d", i);
		AG_TlistAdd(ucom->list, NULL, "Item #%d", i);
	}

	/*
	 * AG_FSpinbutton can bind to an integral or floating-point number.
	 * It also provides built-in unit conversion. AG_Spinbutton is a
	 * variant which handles integer values only and provides no unit
	 * conversion.
	 */
	AG_FSpinbuttonNew(div->box1, 0, "cm", "Real number: ");
	AG_SpinbuttonNew(div->box1, "Integer: ");

	/*
	 * AG_MFSpinbutton and AG_MSpinbutton are variants used to conveniently
	 * edit two values, such as 2D coordinates.
	 */
	AG_MFSpinbuttonNew(div->box1, 0, "mm", "x", "Dimensions: ");
	AG_MSpinbuttonNew(div->box1, 0, ",", "Coordinates: ");

	/*
	 * AG_Textbox is a single-line text edition widget. It can bind to
	 * a sized buffer containing a string.
	 */
	AG_TextboxNew(div->box1, "Enter text: ");

	{
		static int value = 127;
		AG_Scrollbar *sb;
		AG_Statusbar *st;

		/*
		 * AG_Scrollbar provides three bindings, "value", "min" and
		 * "max". Note that this widget is not focusable by default.
		 */
		sb = AG_ScrollbarNew(div->box1, AG_SCROLLBAR_HORIZ,
		    AG_SCROLLBAR_WFILL|AG_SCROLLBAR_FOCUSABLE);
		AG_WidgetSetInt(sb, "min", 0);
		AG_WidgetSetInt(sb, "max", 255);
		AG_WidgetBindInt(sb, "value", &value);
	
		/*
		 * AG_Statusbar displays a label (either static or polled).
		 */
		st = AG_StatusbarNew(div->box1);
		AG_StatusbarAddLabel(st, AG_LABEL_POLLED, "Value = %d",
		    &value);
	}

	/*
	 * AG_Notebook provides multiple containers which can be selected by
	 * the user.
	 */
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Table *table;

		nb = AG_NotebookNew(div->box2, AG_NOTEBOOK_EXPAND);

		ntab = AG_NotebookAddTab(nb, "Color", AG_BOX_VERT);
		{
			/*
			 * AG_HSVPal is an HSV color picker widget which can
			 * bind to RGB(A) or HSV(A) vectors (integral or real)
			 * or 32-bit pixel values (of a given format).
			 */
			AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		}
		
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
			const char *radio_items[] = {
				"Points",
				"Lines",
				NULL
			};
			AG_Radio *rad;
			AG_Graph *g;
			AG_GraphItem *sinplot, *cosplot;
			double f;
			int i = 0;
			
			/* AG_Graph plots a set of values graphically. */
			g = AG_GraphNew(ntab, AG_GRAPH_LINES,
			    AG_GRAPH_WFILL|AG_GRAPH_HFILL);
			sinplot = AG_GraphAddItem(g, "sin", 0, 150, 0, 0);
			cosplot = AG_GraphAddItem(g, "cos", 150, 150, 0, 0);
			for (f = 0; f < 60; f += 0.3) {
				AG_GraphPlot(sinplot, sin(f)*20.0);
				AG_GraphPlot(cosplot, cos(f)*20.0);

				/*
				 * Insert an AG_Table row for sin(f) and cos(f).
				 * Note that the number of fields must >= the
				 * number of columns specified earlier.
				 */
				AG_TableAddRow(table, "%.02f:%.02f:%.02f",
				    f, sin(f), cos(f));
			}

			/*
			 * AG_Radio displays a group of radio buttons. It can
			 * bind to an integer value.
			 */
			rad = AG_RadioNew(ntab, radio_items);
			AG_WidgetBindInt(rad, "value", &g->type);
		}
		
		ntab = AG_NotebookAddTab(nb, "Tlist", AG_BOX_VERT);
		{
			AG_Tlist *tl;

			/*
			 * The AG_Tlist widget displays either lists or trees.
			 * For flat, polled lists, it is more efficient to use
			 * an AG_Table with a single column, however.
			 */
			tl = AG_TlistNew(ntab, 0);
			AG_TlistAdd(tl, NULL, "Foo");
			AG_TlistAdd(tl, NULL, "Bar");
			AG_TlistAdd(tl, NULL, "Baz");
		}
		
	}

#if 0
	/*
	 * AG_GLView provides an OpenGL rendering context. See the "glview"
	 * demo in the Agar ./demos directory for a sample application.
	 */
	AG_GLViewNew(win, 0);
#endif
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("widgets-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFgGr:bB")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case 'r':
			fps = atoi(optarg);
			break;
		case 'b':
			AG_SetBool(agConfig, "font.freetype", 0);
			AG_SetString(agConfig, "font-face", "minimal.xcf");
			AG_SetInt(agConfig, "font-size", 11);
			break;
		case 'B':
			AG_SetBool(agConfig, "font.freetype", 1);
			AG_SetString(agConfig, "font-face", "Vera.ttf");
			break;
		case '?':
		default:
			printf("%s [-vfFgGbB] [-r fps]\n", agProgName);
			exit(0);
		}
	}

	/* Initialize the display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(640, 480, 32, 0) == -1 ||
	    AG_InitInput(0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitConfigWin(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F1, KMOD_NONE, AG_ShowSettings);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	CreateWindow();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


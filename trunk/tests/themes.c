/*	Public domain	*/
/*
 * This application displays a couple of standard Agar-GUI widgets. It is
 * useful for testing the GUI display and trying different themes (see
 * AG_Style(3) for details). See README file for a description of the
 * various command-line options provided.
 */

#include "agartest.h"

#include <agar/dev.h>

#include <stdio.h>
#include <math.h>

#include <agar/core/types.h>

#include <agar/config/ag_debug.h>
#include <agar/config/version.h>

#include "themes_mytheme.h"

char textBuffer[30];

/* Apply a new theme to all windows. */
static void
SetTheme(AG_Event *event)
{
	AG_Style *style = AG_PTR(1);
	AG_Driver *drv;

	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver)
		AG_SetStyle(drv, style);
}

/* Load a color scheme file. */
static void
SetColorScheme(AG_Event *event)
{
	char *file = AG_STRING(1);

	if (AG_ColorsLoad(file) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Failed to load color scheme: %s",
		    AG_GetError());
	}
}

/* Tweak the current color scheme. */
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

/* Example callback for combo-selected. */
static void
ComboSelected(AG_Event *event)
{
	AG_TlistItem *ti = AG_PTR(1);

	AG_TextMsgS(AG_MSG_INFO, AG_Printf("Selected Item: %s", ti->text));
}

/* Show the agar-dev "Preferences" dialog. */
static void
Preferences(AG_Event *event)
{
	DEV_ConfigShow();
}

static void
SetWordWrap(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	int flag = AG_INT(2);

	AG_TextboxSetWordWrap(textbox, flag);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	AG_Box *hBox, *vBox;
	AG_Pane *pane;
	AG_Combo *com;
	AG_UCombo *ucom;
	AG_Box *div1, *div2;
	AG_Textbox *tbox;
	int i;

	/*
	 * Pane provides two Box containers which can be resized using
	 * a control placed in the middle.
	 */
	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	AG_PaneSetDivisionMin(pane, 0, 50, 100);
	AG_PaneMoveDividerPct(pane, 40);
	div1 = pane->div[0];
	div2 = pane->div[1];
	{
		char path[AG_PATHNAME_MAX];
		AG_Label *lbl;

		/* The Pixmap widget displays a raster surface. */
		if (!AG_ConfigFile("load-path", "agar", "bmp", path, sizeof(path)))
			AG_PixmapFromBMP(div1, 0, path);
	
		/*
		 * The Label widget provides a simple static or polled label
		 * (polled labels use special format strings; see AG_Label(3)
		 * for details).
		 */
		AG_LabelNewS(div1, 0, "This is a static label");

		lbl = AG_LabelNewPolled(div1, AG_LABEL_FRAME|AG_LABEL_EXPAND,
		    "This is a polled label.\n"
		    "Window is at %i,%i (%ux%u)",
		    &AGWIDGET(win)->x,
		    &AGWIDGET(win)->y,
		    &AGWIDGET(win)->w,
		    &AGWIDGET(win)->h);
		AG_LabelSizeHint(lbl, 1,
		    "This is a polled label\n"
		    "Window is at 0000,0000 (0000x0000)");
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
	}

	/*
	 * Box is a general-purpose widget container. We use AG_BoxNewHoriz()
	 * for horizontal widget packing.
	 */
	hBox = AG_BoxNewHoriz(div1, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		/*
		 * The Button widget is a simple push-button. It is typically
		 * used to trigger events, but it can also bind its state to
		 * an boolean (integer) value or a bitmask.
		 */
		for (i = 0; i < 5; i++)
			AG_ButtonNewS(hBox, 0, AG_Printf("%c", 0x41+i));
	}

	hBox = AG_BoxNewHoriz(div1, AG_BOX_HFILL);
	{
		/* The Radio checkbox is a group of radio buttons. */
		{
			const char *radioItems[] = {
				"Radio1",
				"Radio2",
				NULL
			};
			AG_RadioNew(hBox, 0, radioItems);
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
	com = AG_ComboNew(div1, AG_COMBO_HFILL, "Combo: ");
	AG_ComboSizeHint(com, "Item #00 ", 10);
	AG_SetEvent(com, "combo-selected", ComboSelected, NULL);

	/* UCombo is a variant of Combo which looks like a single button. */
	ucom = AG_UComboNew(div1, AG_UCOMBO_HFILL);
	AG_UComboSizeHint(ucom, "Item #1234", 5);

	/* Populate the Tlist displayed by the combo widgets we just created. */
	for (i = 0; i < 50; i++) {
		char text[32];

		/* This is more efficient than AG_Printf() */
		AG_Strlcpy(text, "Item #", sizeof(text));
		AG_StrlcatInt(text, i, sizeof(text));

		AG_TlistAddS(com->list, NULL, text);
		AG_TlistAddS(ucom->list, NULL, text);
	}

	/*
	 * Numerical binds to an integral or floating-point number.
	 * It can also provides built-in unit conversion (see AG_Units(3)).
	 */
	{
		AG_Numerical *num;
		static float myFloat = 1.0f;
		static int myInt = 123;

		num = AG_NumericalNewS(div1,
		    AG_NUMERICAL_EXCL|AG_NUMERICAL_HFILL,
		    "cm", "Real: ");
		AG_BindFloat(num, "value", &myFloat);
		AG_SetFloat(num, "incr", 1.0f);

		num = AG_NumericalNewS(div1,
		    AG_NUMERICAL_EXCL|AG_NUMERICAL_HFILL,
		    NULL, "Int: ");
		AG_BindInt(num, "value", &myInt);
		AG_SetInt(num, "min", -100);
		AG_SetInt(num, "max", +100);
	}

	/*
	 * Textbox is a single or multiline text edition widget. It can bind
	 * to fixed-size or dynamically-sized buffers in any character set
	 * encoding.
	 */
	{
		AG_Strlcpy(textBuffer, "Foo bar baz bezo", sizeof(textBuffer));

		/* Create a textbox bound to a fixed-size buffer */
		tbox = AG_TextboxNew(div1,
		    AG_TEXTBOX_EXCL|AG_TEXTBOX_HFILL,
		    "Fixed text buffer: ");
		AG_TextboxBindUTF8(tbox, textBuffer, sizeof(textBuffer));

		/* Create a textbox bound to a built-in AG_Text element */
		tbox = AG_TextboxNew(div1,
		    AG_TEXTBOX_EXCL|AG_TEXTBOX_MULTILINGUAL|AG_TEXTBOX_HFILL,
		    "AG_Text element: ");
		AG_TextboxSetString(tbox, "Foo");
	}

	AG_SeparatorNewHoriz(div1);

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

		sb = AG_ScrollbarNew(div1, AG_SCROLLBAR_HORIZ,
		    AG_SCROLLBAR_NOAUTOHIDE|AG_SCROLLBAR_EXCL|AG_SCROLLBAR_HFILL);
		AG_BindInt(sb, "value", &myVal);
		AG_BindInt(sb, "min", &myMin);
		AG_BindInt(sb, "max", &myMax);
		AG_BindInt(sb, "visible", &myVisible);
		AG_SetInt(sb, "inc", 10);

		sl = AG_SliderNew(div1, AG_SLIDER_HORIZ,
		    AG_SLIDER_EXCL|AG_SLIDER_HFILL);
		AG_BindInt(sl, "value", &myVal);
		AG_BindInt(sl, "min", &myMin);
		AG_BindInt(sl, "max", &myMax);
		AG_SetInt(sl, "inc", 10);

		pb = AG_ProgressBarNew(div1, AG_PROGRESS_BAR_HORIZ,
		    AG_PROGRESS_BAR_EXCL|AG_PROGRESS_BAR_SHOW_PCT);
		AG_BindInt(pb, "value", &myVal);
		AG_BindInt(pb, "min", &myMin);
		AG_BindInt(pb, "max", &myMax);
	}

	/*
	 * Notebook provides multiple containers which can be selected by
	 * the user.
	 */
	{
		AG_Notebook *nb;
		AG_NotebookTab *ntab;
		AG_Table *table;
		AG_Menu *menu;
		AG_MenuItem *m;
	
		/* Create a test menu */
		menu = AG_MenuNew(div2, AG_MENU_HFILL);
		m = AG_MenuNode(menu->root, "File", NULL);
		{
			AG_MenuAction(m, "Preferences...", agIconGear.s,
			    Preferences, NULL);
			AG_MenuSeparator(m);
			AG_MenuAction(m, "Close this test", agIconClose.s,
			    AGWINDETACH(win));
		}
		m = AG_MenuNode(menu->root, "Theme", NULL);
		{
			AG_MenuAction(m, "Default theme", NULL,
			    SetTheme, "%p", &agStyleDefault);
			AG_MenuAction(m, "Custom theme", NULL,
			    SetTheme, "%p", &myRoundedStyle);
			m = AG_MenuNode(m, "Color scheme", NULL);
			{
				AG_MenuAction(m, "Green (green.acs)", NULL,
				    SetColorScheme, "%s", "green.acs");
				AG_MenuAction(m, "Make Darker", NULL,
				    TweakColorScheme, "%i", 1);
				AG_MenuAction(m, "Make Lighter", NULL,
				    TweakColorScheme, "%i", 0);
			}
		}
		m = AG_MenuNode(menu->root, "Test", NULL);
		{
			AG_MenuNode(m, "Submenu A", NULL);
			AG_MenuSeparator(m);
			m = AG_MenuNode(m, "Submenu B", NULL);
			AG_MenuNode(m, "Submenu C", NULL);
			AG_MenuNode(m, "Submenu D", NULL);
			AG_MenuNode(m, "Submenu E", NULL);
		}

		nb = AG_NotebookNew(div2, AG_NOTEBOOK_EXPAND);

		ntab = AG_NotebookAddTab(nb, "Some table", AG_BOX_VERT);
		{
			float f;

			/*
			 * AG_Table displays a set of cells organized in
			 * rows and columns. It is optimized for cases where
			 * the table is static or needs to be repopulated
			 * periodically.
			 */
			table = AG_TableNew(ntab, AG_TABLE_EXPAND);
			AG_TableAddCol(table, "x", "33%", NULL);
			AG_TableAddCol(table, "sin(x)", "33%", NULL);
			AG_TableAddCol(table, "cos(x)", "33%", NULL);
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
		
		ntab = AG_NotebookAddTab(nb, "Some text", AG_BOX_VERT);
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
			tbox = AG_TextboxNewS(ntab,
			    AG_TEXTBOX_MULTILINE|AG_TEXTBOX_CATCH_TAB|
			    AG_TEXTBOX_EXPAND|AG_TEXTBOX_EXCL, NULL);
			AG_WidgetSetFocusable(tbox, 1);

			/*
			 * Load the contents of this file into a buffer. Make
			 * the buffer a bit larger so the user can try
			 * entering text.
			 */
			if (!AG_ConfigFile("load-path", "loss", "txt", path, sizeof(path)) &&
			    (f = fopen(path, "r")) != NULL) {
				fseek(f, 0, SEEK_END);
				size = ftell(f);
				fseek(f, 0, SEEK_SET);
				bufSize = size+1024;
				someText = AG_Malloc(bufSize);
				(void)fread(someText, size, 1, f);
				fclose(f);
				someText[size] = '\0';
			} else {
				someText = AG_Strdup("Failed to load loss.txt");
				bufSize = strlen(someText)+1;
			}
	
			/*
			 * Bind the buffer's contents to the Textbox. The
			 * size argument to AG_TextboxBindUTF8() must include
			 * space for the terminating NUL.
			 */
			AG_TextboxBindUTF8(tbox, someText, bufSize);
	
			/* Add a word wrapping control */
			AG_CheckboxNewFn(ntab, 0, "Word wrapping",
			    SetWordWrap, "%p", tbox);
		}
		
		ntab = AG_NotebookAddTab(nb, "Empty tab", AG_BOX_VERT);
	}
	return (0);
}

static int
Init(void *obj)
{
	/* Initialize our custom theme. */
	InitMyRoundedStyle(&myRoundedStyle);
	
	DEV_InitSubsystem(0);
	return (0);
}

static void
Destroy(void *obj)
{
	DEV_DestroySubsystem();
}

const AG_TestCase themesTest = {
	"themes",
	N_("Test a variety of widgets with different themes"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

/*	Public domain	*/
/*
 * This application displays a set of standard Agar-GUI widgets.
 */

#include "agartest.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <agar/core/types.h>

#include <agar/config/ag_debug.h>
#include <agar/config/version.h>

typedef struct {
	AG_TestInstance _inherit;
	char textBuffer[128];
	char *someText;
	AG_TextElement *textElement;
} MyTestInstance;

static void
ComboSelectedItem(AG_Event *event)
{
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(1);

	AG_TextTmsg(AG_MSG_INFO, 1000,
	    "Selected item: [ " AGSI_ITALIC "%s" AGSI_RST " ]", ti->text);
}

static void
ComboEnteredText(AG_Event *event)
{
	const char *text = AG_STRING(1);

	AG_TextTmsg(AG_MSG_INFO, 1000,
	    "Entered text: \" " AGSI_ITALIC "%s" AGSI_RST " \"", text);
}

/* Suggested "GUI Preferences" dialog. */
static void
Preferences(AG_Event *event)
{
	AG_DEV_ConfigShow();
}

static void
SetWordWrap(AG_Event *event)
{
	AG_Textbox *textbox = AG_TEXTBOX_PTR(1);
	int flag = AG_INT(2);

	AG_TextboxSetWordWrap(textbox, flag);
}

static void
TestMenuFn(AG_Event *event)
{
	const char *text = AG_STRING(1);

	AG_TextMsgS(AG_MSG_INFO, text);
}

static void
TableKeyDown(AG_Event *event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int keysym = AG_INT(1);
	int m;

	switch (keysym) {
	case AG_KEY_DELETE:
		for (m = 0; m < t->m; m++) {
			if (AG_TableRowSelected(t,m))
				break;
		}
		if (m < t->m) {
			AG_Debug(t, "Deleting row # %d\n", m);
		}
		break;
	}
}

static void
SayHello(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	char *who = AG_TextboxDupString(tb);

	AG_TextTmsg(AG_MSG_INFO, 2000, "Hello, %s!", who);

	Free(who);
}

/*
 * Sample autocomplete routine for Textbox. Parse "First Last" or "First,Last"
 * and provide different suggestions for first and last names based on two
 * separate dictionaries.
 */
static void
AutocompleteName(AG_Event *event)
{
	const char *dictFirst[] = {
		"Agnes", "Apu", "Artie", "Barbara", "Barry", "Bart", "Bernice",
		"Brandine", "Carl", "Cecil", "Cletus", "Count", "Disco",
		"Dondelinger", "Doris", "Drederick", "Eleanor", "Gerald", "Gil",
		"Gino", "Herman", "Homer", "Jasper", "Jebediah", "Jimbo",
		"Julius", "Kent", "Kirk", "Lenny", "Lisa", "Maggie", "Marge",
		"Martha", "Martin", "Manjula", "Marvin", "Maude", "Moe", "Murphy",
		"Ned", "Rainier", "Robert", "Rod", "Sarah", "Sideshow", "Todd",
		"\xC3\x9Cter", NULL
	}, **dp;
	const char *dictLast[] = {
		"Abernathy", "Beardly", "Bouvier", "Brockman", "Carlson",
		"Dracula", "Duffman", "Flanders", "Gunderson", "Harlan",
		"Hibbert", "Hermann", "Jones", "Krustofsky", "Moleman",
		"Monroe", "Nahasapeemapetilon", "Leonard", "Mel", "Prince",
		"Rollolinski", "Samson", "Simpson", "Skinner", "Springfield",
		"Spuckler", "Szyslak", "Stu", "Tatum", "Terwilliger",
		"Z\xC3\xB6rker", "Van Houten", "Wiggum", "Wolfcastle", "Ziff",
		NULL
	};
	AG_Editable *ed = AG_EDITABLE_SELF();
	AG_Tlist *tl = AG_TLIST_PTR(1);
	char *s = AG_EditableDupString(ed), *sp = s;
	const char *sFirst, *sLast;
	AG_TlistItem *it;
	int nSpaces=0;

	while (*sp == ' ' || *sp == '\t') {
		sp++;
	}
	sFirst = AG_Strsep(&sp, " ,");
	do {
		sLast = AG_Strsep(&sp, " ,");
		nSpaces++;
	} while (sLast != NULL && *sLast == '\0');

	AG_TlistBegin(tl);

	if (sFirst[0] == '\0' || sFirst[0] == '*') {
		for (dp = dictFirst; *dp != NULL; dp++)
			AG_TlistAddPtr(tl, NULL, *dp, (void *)*dp);
	} else if (sLast != NULL) {
		if (sLast[0] == '\0' || sLast[0] == '*') {
			for (dp = dictLast; *dp != NULL; dp++) {
				it = AG_TlistAdd(tl, NULL, "%s %s", sFirst, *dp);
				it->p1 = (void *)*dp;
			}
		} else {
			for (dp = dictLast; *dp != NULL; dp++) {
				if (AG_Strncasecmp(*dp, sLast, (AG_Size)strlen(sLast)) == 0) {
					it = AG_TlistAdd(tl, NULL, "%s %s", sFirst, *dp);
					it->p1 = (void *)*dp;
				}
			}
		}
	} else {
		if (nSpaces > 1) {
			for (dp = dictLast; *dp != NULL; dp++) {
				it = AG_TlistAdd(tl, NULL, "%s %s", sFirst, *dp);
				it->p1 = (void *)*dp;
			}
		} else {
			for (dp = dictFirst; *dp != NULL; dp++)
				if (AG_Strncasecmp(*dp, sFirst, (AG_Size)strlen(sFirst)) == 0)
					AG_TlistAddPtr(tl, NULL, *dp, (void *)*dp);
		}
	}

	AG_TlistEnd(tl);

	if (tl->nItems == 0) {
		AG_EditableCloseAutocomplete(ed);
	} else if (tl->nItems == 1) {
		char *sOrig = AG_EditableDupString(ed);

		if (AG_TlistFindText(tl, sOrig)) {
			AG_EditableCloseAutocomplete(ed);
		}
		Free(sOrig);
	}
	Free(s);
}

/* Create more checkboxes under "Some checkboxes" */
static void
CreateMoreCheckboxes(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Box *box = AG_BOX_PTR(1);
	AG_Pane *pane = AG_PANE_PTR(2);

	AG_WidgetDisable(bu);

	AG_CheckboxNew(box, 0, "George Liquor");
	AG_CheckboxNew(box, 0, "Haggis McHaggis");
	AG_CheckboxNew(box, 0, "Kowalski, Bubba\n"
	                       "and Jiminy Lummox");
	AG_CheckboxNew(box, 0, "Wilbern Cobb");

	AG_PaneMoveDivider(pane, pane->dx + 50);
	AG_SetFontSize(box, "80%");
}

static void
ComboExpanded(AG_Event *_Nonnull event)
{
	AG_Combo *com = AG_COMBO_SELF();
	AG_Tlist *tl = com->list;
	int i;

	for (i = 0; i < 50; i++) {
		char text[32];

		AG_Strlcpy(text, "Item #", sizeof(text));
		AG_StrlcatInt(text, i, sizeof(text));

		AG_TlistAddS(tl, NULL, text);
	}

}

static void
UComboExpanded(AG_Event *_Nonnull event)
{
	AG_UCombo *com = AG_UCOMBO_SELF();
	AG_Tlist *tl = com->list;

	AG_TlistAddS(tl, NULL, "George Liquor");
	AG_TlistAddS(tl, NULL, "Haggis McHaggis");
	AG_TlistAddS(tl, NULL, "Kowalski (Lummox)");
	AG_TlistAddS(tl, NULL, "Bubba (Lummox)");
	AG_TlistAddS(tl, NULL, "Jiminy (Lummox)");
	AG_TlistAddS(tl, NULL, "Wilbern Cobb");

	AG_TlistSizeHintLargest(tl, 6);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	char path[AG_PATHNAME_MAX];
	MyTestInstance *ti = obj;
	AG_Box *hBox, *vBox, *div;
	AG_Pane *hPane, *vPane;
	AG_Combo *com;
	AG_UCombo *ucom;
	AG_Textbox *tbox;
	AG_Surface *S;
	int i;

	/*
	 * Pane provides two Box containers which can be resized using
	 * a control placed in the middle.
	 */
	hPane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	AG_PaneSetDivisionMin(hPane, 0, 50, 100);
/*	AG_PaneMoveDividerPct(hPane, 45); */
	div = hPane->div[0];

	/* Load a Windows bitmap file and display it in an AG_Pixmap(3). */
	if (AG_ConfigFind(AG_CONFIG_PATH_DATA, "agar-1.bmp", path, sizeof(path)) == 0) {
		if ((S = AG_SurfaceFromBMP(path)) == NULL) {
			S = AG_TextRender(AG_GetError());
		}
		AG_PixmapFromSurface(div, AG_PIXMAP_HFILL | AG_PIXMAP_RESCALE, S);
		AG_SurfaceFree(S);
	} else {
		S = AG_TextRender(AG_GetError());
		AG_PixmapFromSurface(div, 0, S);
		AG_SurfaceFree(S);
	}

	/* Load a 16-bit/color PNG file and display it in an AG_Pixmap(3). */
#if AG_MODEL == AG_LARGE
	if (AG_ConfigFind(AG_CONFIG_PATH_DATA, "agar64.png", path, sizeof(path)) == 0) {
		if ((S = AG_SurfaceFromPNG(path)) == NULL) {
			S = AG_TextRender(AG_GetError());
		} else {
#if 0
			int x,y;

/*			S->flags |= AG_SURFACE_TRACE; */

			/*
			 * Extract components from surface-encoded pixels
			 * and re-encode the pixels from the extracted
			 * components.
			 * 
			 * If pixel operations are working as expected on
			 * 16-bit/color surfaces, the following code should be
			 * a no-op and the original image appear unchanged.
			 */
			for (y = 0; y < S->h; y++) {
				for (x = 0; x < S->w; x++) {
					AG_Pixel px = AG_SurfaceGet(S, x,y);
					AG_Color c;

					AG_GetColor(&c, px, &S->format);
#if 0
					if (c.a > 0) {
						c.r = AG_COLOR_LAST - (AG_Component)(((float)x / (float)S->w) * AG_COLOR_LASTD);
						c.g = AG_COLOR_LAST - (AG_Component)(((float)x / (float)S->w) * AG_COLOR_LASTD);
						c.b = AG_COLOR_LAST - (AG_Component)(((float)x / (float)S->w) * AG_COLOR_LASTD);
					}
#endif
					AG_SurfacePut(S, x,y,
					    AG_MapPixel(&S->format, &c));
				}
			}
#endif
		}
		AG_PixmapFromSurface(div, AG_PIXMAP_HFILL | AG_PIXMAP_RESCALE, S);
		AG_SurfaceFree(S);
	} else {
		S = AG_TextRender(AG_GetError());
		AG_PixmapFromSurface(div, 0, S);
		AG_SurfaceFree(S);
	}
#endif /* AG_LARGE */

	AG_SeparatorNewHoriz(div);

	/*
	 * AG_Label(3) displays either a static text label, or a dynamically
	 * updated one. Polled (dynamic) labels use a special format documented
	 * in AG_String(3).
	 */
	{
		AG_AgarVersion av;
		AG_Label *lbl;

		AG_GetVersion(&av);

		/* A static label */
		lbl = AG_LabelNew(div, AG_LABEL_HFILL,
		    "Agar v%d.%d.%d ("
		    AGSI_LEAGUE_SPARTAN AGSI_CYAN "%s" AGSI_RST ")",
		    av.major, av.minor, av.patch,
		    av.release ? av.release : "dev");
		AG_SetFontSize(lbl, "120%");
		AG_LabelJustify(lbl, AG_TEXT_CENTER);

		/* A dynamically-updated label. */
		lbl = AG_LabelNewPolled(div, AG_LABEL_HFILL,
		    "This "
		    AGSI_ALGUE AGSI_BLACK_AGAR AGSI_RST
		    " Window is at %i,%i (%u x %u).",
		    &AGWIDGET(win)->x,
		    &AGWIDGET(win)->y,
		    &AGWIDGET(win)->w,
		    &AGWIDGET(win)->h);
		AG_LabelSizeHint(lbl, 1, "<This [XXXX] Window is at 0000,0000 (0000 x 0000). >");
		AG_SetFontSize(lbl, "110%");

		/* Show an example of a bitmap font (agar-minimal.agbf) */
		AG_LabelNew(div, AG_LABEL_HFILL,
		    "Here is a bitmap font: " 
		    AGSI_AGAR_MINIMAL "Agar Minimal Hello!" AGSI_RST
		    " (12px)");
	}

	/*
	 * Pane provides two Box containers which can be resized using
	 * a control placed in the middle.
	 */
	vPane = AG_PaneNewVert(div, AG_PANE_EXPAND);
	div = vPane->div[0];

	/*
	 * Box is a general-purpose widget container. AG_BoxNewHoriz() creates
	 * a container which packs its widgets horizontally.
	 */
	hBox = AG_BoxNewHoriz(div, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		/*
		 * The Button widget is a simple push-button. It is typically
		 * used to trigger events, but it can also bind its state to
		 * an boolean (integer) value or a bitmask.
		 */
		for (i = 0; i < 5; i++)
			AG_ButtonNewS(hBox, 0, AG_Printf("%c", 0x41+i));
	}

	hBox = AG_BoxNewHoriz(div, AG_BOX_HFILL);
	AG_BoxSetHorizAlign(hBox, AG_BOX_CENTER);
	AG_BoxSetVertAlign(hBox, AG_BOX_CENTER);
	{
		/* Radio button group */
		vBox = AG_BoxNewVert(hBox, 0);
		AG_BoxSetLabel(vBox, "Radio group:");
		{
			const char *radioItems[] = {
				"Homer\n"
				"(" AGSI_ITALIC "Simpson" AGSI_RST ")",
				"Marge",
				"Bart",
				"Lisa",
				"Maggie",
				NULL
			};
			AG_RadioNew(vBox, 0, radioItems);
		}
	
		vBox = AG_BoxNewVert(hBox, 0);
		AG_BoxSetLabel(vBox, "Checkboxes:");
		{
			AG_Button *btn;

			/*
			 * The Checkbox widget can bind to boolean values
			 * and bitmasks.
			 */
			AG_CheckboxNew(vBox, 0, "Ren H" "\xC3\xB6" "ek");
			AG_CheckboxNew(vBox, 0, "Stimpson "
			                        AGSI_ITALIC "J." AGSI_RST
						" Cat");
			AG_CheckboxNew(vBox, 0, "Mr. " AGSI_BOLD "Horse" AGSI_RST);
			AG_CheckboxNew(vBox, 0, AGSI_YEL AGSI_BOLD "P" AGSI_RST
			                        "owdered "
						AGSI_YEL AGSI_BOLD "T" AGSI_RST
						"oast "
						AGSI_YEL AGSI_BOLD "M" AGSI_RST
						"an");

			btn = AG_ButtonNewFn(vBox, 0, "Create More ...",
			    CreateMoreCheckboxes, "%p,%p", vBox, vPane);

			AG_SetPadding(btn, "2");
		}
	}

	div = vPane->div[1];

	/*
	 * The Combo widget is a textbox widget with a expander button next
	 * to it. The button triggers a popup window which displays a list
	 * (using the AG_Tlist(3) widget).
	 */
	com = AG_ComboNew(div, AG_COMBO_ANY_TEXT | AG_COMBO_HFILL, "Combo: ");
	AG_ComboSizeHint(com, "<Item #000>", 10);
	AG_SetEvent(com, "combo-expanded", ComboExpanded, NULL);
	AG_SetEvent(com, "combo-selected", ComboSelectedItem, NULL);
	AG_SetEvent(com, "combo-text-entry", ComboEnteredText, NULL);

	/* UCombo is a variant of Combo which looks like a single button. */
	ucom = AG_UComboNew(div, AG_UCOMBO_HFILL);
	AG_UComboSizeHint(ucom, "<Item #1234>", 5);
	AG_SetEvent(ucom, "ucombo-expanded", UComboExpanded, NULL);

	/* Create a horizontal separator */
	AG_SeparatorNewHoriz(div);

	/*
	 * Textbox is a single or multiline text edition widget. It can bind
	 * to fixed-size or dynamically-sized buffers in any character set
	 * encoding.
	 */
	ti->textBuffer[0] = '\0';
	ti->textElement = AG_TextNew(100);

	/* Create a textbox bound to a fixed-size buffer */
	tbox = AG_TextboxNew(div, AG_TEXTBOX_EXCL | AG_TEXTBOX_HFILL, "Textbox: ");
	AG_TextboxSetPlaceholderS(tbox, "First & Last Name");
	AG_TextboxAutocomplete(tbox, AutocompleteName, NULL);
	AG_TextboxBindUTF8(tbox, ti->textBuffer, sizeof(ti->textBuffer));
	AG_SetEvent(tbox, "textbox-return", SayHello, "%p", tbox);

	/*
	 * Create a textbox connected to a dynamically-sized, multilingual
	 * text element.
	 */
	tbox = AG_TextboxNew(div, AG_TEXTBOX_EXCL | AG_TEXTBOX_MULTILINGUAL |
	                          AG_TEXTBOX_HFILL, "TextElement: ");
	AG_TextboxSetString(tbox, "Hello");
	AG_TextSetEntS(ti->textElement, AG_LANG_EN, "Hello");
	AG_TextSetEntS(ti->textElement, AG_LANG_FR, "Bonjour");
	AG_TextSetEntS(ti->textElement, AG_LANG_DE, "Guten tag");
	AG_TextboxBindText(tbox, ti->textElement);

	/* Create a horizontal separator */
	AG_SeparatorNewHoriz(div);

	/*
	 * Numerical binds to an integral or floating-point number.
	 * It can also provides built-in unit conversion (see AG_Units(3)).
	 */
	{
		AG_Numerical *num;
		static float myFloat = 1.0f;
		static int myInt = 50;

		num = AG_NumericalNewS(div,
		    AG_NUMERICAL_EXCL | AG_NUMERICAL_HFILL,
		    "cm", "Numerical (flt): ");
		AG_BindFloat(num, "value", &myFloat);
		AG_SetFloat(num, "inc", 1.0f);

		num = AG_NumericalNewS(div,
		    AG_NUMERICAL_EXCL | AG_NUMERICAL_HFILL,
		    NULL, "Numerical (int): ");
		AG_BindInt(num, "value", &myInt);
		AG_SetInt(num, "min", -100);
		AG_SetInt(num, "max", +100);
	}

	/* Create a horizontal separator */
	AG_SeparatorNewHoriz(div);

	/*
	 * Scrollbar provides "value", "min" and "max" bindings which can be
	 * connected to integers or floating-point variables. Progressbar and
	 * Slider offer similar interfaces.
	 */
	{
		static int myVal = 50, myMin = -100, myMax = 100, myVisible = 0;
		AG_Scrollbar *sb;
		AG_ProgressBar *pb;
		AG_Numerical *num;

		num = AG_NumericalNewS(div, AG_NUMERICAL_HFILL, NULL,
		    "Bound Integer: ");
		AG_BindInt(num, "value", &myVal);
		AG_SetInt(num, "min", -100);
		AG_SetInt(num, "max", +100);

		AG_LabelNewS(div, 0, "Scrollbar:");
		sb = AG_ScrollbarNewHoriz(div, AG_SCROLLBAR_EXCL |
		                               AG_SCROLLBAR_HFILL);
		AG_BindInt(sb, "value", &myVal);
		AG_BindInt(sb, "min", &myMin);
		AG_BindInt(sb, "max", &myMax);
		AG_BindInt(sb, "visible", &myVisible);
		AG_SetInt(sb, "inc", 10);

		AG_LabelNewS(div, 0, "ProgressBar (Horiz | Vert):");

		hBox = AG_BoxNewHoriz(div, AG_BOX_HFILL);

		pb = AG_ProgressBarNewHoriz(hBox, AG_PROGRESS_BAR_EXCL |
		                                  AG_PROGRESS_BAR_SHOW_PCT |
		                                  AG_PROGRESS_BAR_HFILL);
		AG_BindInt(pb, "value", &myVal);
		AG_BindInt(pb, "min", &myMin);
		AG_BindInt(pb, "max", &myMax);

		AG_SeparatorNewVert(hBox);

		pb = AG_ProgressBarNewVert(hBox, AG_PROGRESS_BAR_EXCL |
		                                 AG_PROGRESS_BAR_SHOW_PCT);
		AG_ProgressBarSetLength(pb, 50);
		AG_BindInt(pb, "value", &myVal);
		AG_BindInt(pb, "min", &myMin);
		AG_BindInt(pb, "max", &myMax);
	}

	/*
	 * Load the square agar logo, perform pixel manipulation on the loaded
	 * surface (divide alpha of red-dominant pixels) and display the results
	 * in AG_Pixmap(3) widgets.
	 */
	if (AG_ConfigFind(AG_CONFIG_PATH_DATA, "sq-agar.bmp", path, sizeof(path)) == 0) {
		AG_Surface *S;
		int i, x,y;

		S = AG_SurfaceFromFile(path);

		hBox = AG_BoxNewHoriz(div, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
		for (i = 0; i < 5; i++) {
			for (y = 0; y < (int)S->h; y++) {
				for (x = 0; x < (int)S->w; x++) {
					AG_Pixel px = AG_SurfaceGet(S, x,y);
					AG_Color c ;
						
					AG_GetColor(&c, px, &S->format);
					if (c.r > AG_MAX(c.g, c.b)) {
						c.a /= (1+i);
					}
					AG_SurfacePut(S, x,y,
					    AG_MapPixel(&S->format, &c));
				}
			}
			AG_PixmapFromSurface(hBox, 0, S);
		}
		AG_SurfaceFree(S);
	}

	/*
	 * Notebook provides multiple containers which can be selected by
	 * the user.
	 */
	{
		AG_Notebook *nb;
		AG_NotebookTab *nt;
		AG_Table *table;
		AG_Menu *menu;
		AG_MenuItem *m, *mSub;
		static int myInt[2];
	
		/* Create a test menu */
		menu = AG_MenuNew(hPane->div[1], AG_MENU_HFILL);
		m = AG_MenuNode(menu->root, "File", NULL);
		{
			AG_MenuActionKb(m,
			    AGSI_IDEOGRAM AGSI_CLOSE_X AGSI_RST
			    " Close Test", NULL,
			    AG_KEY_W, AGSI_WINCMD,
			    AGWINCLOSE(win));
		}

		m = AG_MenuNode(menu->root, "Edit", NULL);
		{
			AG_MenuActionKb(m,
			    AGSI_IDEOGRAM AGSI_GEAR AGSI_RST
			    " Preferences...", NULL,
			    AG_KEY_P, AGSI_WINCMD,
			    Preferences, NULL);
		}

		m = AG_MenuNode(menu->root, "Test Menu", NULL);
		{
			AG_MenuState(m, 0);
			AG_MenuNode(m,
			    AGSI_IDEOGRAM AGSI_DIP_CHIP AGSI_RST
			    " Disabled Submenu",
			    NULL);
			AG_MenuState(m, 1);
			AG_MenuSeparator(m);

			/* Menu item with child entries */
			mSub = AG_MenuNode(m,
			    AGSI_IDEOGRAM AGSI_GAME_CONTROLLER AGSI_RST
			    " Try Me!",
			    NULL);
			AG_MenuAction(mSub,
			    AGSI_IDEOGRAM AGSI_ALICE AGSI_RST
			    " Alice",
			    NULL,
			    TestMenuFn, "%s", "Hi Alice!");

			AG_MenuAction(mSub,
			    AGSI_IDEOGRAM AGSI_BOB AGSI_RST
			    " Bob", NULL,
			    TestMenuFn, "%s", "Hi Bob!");

			AG_MenuAction(mSub,
			    AGSI_IDEOGRAM AGSI_PILE_OF_POO AGSI_RST
			    " Pile of Poo", NULL,
			    TestMenuFn, "%s", "Pile of Poo!");

			AG_MenuIntBool(mSub, "Togglable Binding #1 ",
			    NULL, &myInt[0], 0);
			AG_MenuIntBool(mSub, "Inverted Binding #1 ",
			    NULL, &myInt[0], 1);

			AG_MenuSeparator(m);
			AG_MenuSectionS(m,
			    AGSI_IDEOGRAM AGSI_VACUUM_TUBE AGSI_RST
			    AGSI_ITALIC " Non Selectable Text");
			AG_MenuSeparator(m);

			AG_MenuIntBool(m,
			    "Togglable Binding #1\t"
			    AGSI_IDEOGRAM AGSI_AGAR_AG AGSI_RST,
			    NULL, &myInt[0], 0);
			AG_MenuIntBool(m,
			    "Togglable Binding #2\t"
			    AGSI_IDEOGRAM AGSI_AGAR_AR AGSI_RST,
			    NULL, &myInt[1], 0);

			AG_MenuIntBool(m, "Inverted Binding #2", NULL,
			    &myInt[1], 1);
		}

		nb = AG_NotebookNew(hPane->div[1], AG_NOTEBOOK_EXPAND);

		nt = AG_NotebookAdd(nb,
		    "Example Table\n"
		    AGSI_IDEOGRAM AGSI_MATH_X_EQUALS AGSI_RST
		    AGSI_ITALIC "sin" AGSI_RST "(x), "
		    AGSI_ITALIC "cos" AGSI_RST "(x)",
		    AG_BOX_VERT);
		{
			float f;

			/*
			 * AG_Table displays a set of cells organized in
			 * rows and columns. It is optimized for cases where
			 * the table is static or needs to be repopulated
			 * periodically.
			 */
			table = AG_TableNew(nt, AG_TABLE_EXPAND | AG_TABLE_MULTI);

			AG_TableAddCol(table, "x", "33%", NULL);
			AG_TableAddCol(table, "sin(x)", "33%", NULL);
			AG_TableAddCol(table, "cos(x)", "33%", NULL);

			AG_SetFontFamily(table, "charter");
			AG_SetFontSize(table, "120%");
			AG_SetColor(table, "#666");

			for (f = 0.0f; f < 60.0f; f += 0.3f) {
				/*
				 * Insert a Table row for sin(f) and cos(f).
				 * The directives of the format string are
				 * documented in AG_Table(3).
				 */
				AG_TableAddRow(table, "%.02f:%.02f:%.02f",
				    f, sin(f), cos(f));
			}
			{
				const char *selModes[] = {
				    "Select\nby\nRow",
				    "Select\nby\nCell",
				    "Select\nby\nColumn",
				    NULL
				};
				AG_Radio *rad;

				rad = AG_RadioNewUint(nt, 0, selModes,
				    (Uint *)&table->selMode);
				AG_RadioSetDisposition(rad, AG_RADIO_HORIZ);
				AG_SetFontSize(rad, "80%");
			}
			

			vBox = AG_BoxNewVert(nt, 0);
			AG_SetFontSize(vBox, "80%");
			{
				AG_CheckboxNewFlag(vBox, 0, "Select multiple\n(with ctrl/shift)",
				    &table->flags, AG_TABLE_MULTI);

				AG_CheckboxNewFlag(vBox, 0, "Select multiple always",
				    &table->flags, AG_TABLE_MULTITOGGLE);

				AG_CheckboxNewFlag(vBox, 0, "Highlight columns",
				    &table->flags, AG_TABLE_HIGHLIGHT_COLS);
			}

			/*
			 * Append an event handler for the DELETE function.
			 */
			AG_AddEvent(table, "key-down", TableKeyDown, NULL);
		}
		
		nt = AG_NotebookAdd(nb,
		    AGSI_WHT AGSI_ITALIC "\"Loss of Breath\"" AGSI_RST
		    AGSI_LEAGUE_GOTHIC "\nBy Edgar Allan Poe  " AGSI_RST
		    AGSI_IDEOGRAM AGSI_EDGAR_ALLAN_POE AGSI_RST,
		    AG_BOX_VERT);
		{
			char path[AG_PATHNAME_MAX];
			AG_Size size, bufSize;
			FILE *f;

			/*
			 * Textboxes with the MULTILINE flag provide basic
			 * text edition functionality. The CATCH_TAB flag
			 * causes the widget to receive TAB key events
			 * (normally used to focus other widgets).
			 */
			tbox = AG_TextboxNewS(nt,
			    AG_TEXTBOX_MULTILINE | AG_TEXTBOX_CATCH_TAB |
			    AG_TEXTBOX_EXPAND | AG_TEXTBOX_EXCL, NULL);
			AG_WidgetSetFocusable(tbox, 1);

			/*
			 * Load the contents of this file into a buffer. Make
			 * the buffer a bit larger so the user can try
			 * entering text.
			 */
			if (AG_ConfigFind(AG_CONFIG_PATH_DATA, "loss.txt",
			    path, sizeof(path)) == 0) {
				if ((f = fopen(path, "r")) != NULL) {
					fseek(f, 0, SEEK_END);
					size = ftell(f);
					fseek(f, 0, SEEK_SET);
					bufSize = size + 4096;
					ti->someText = AG_Malloc(bufSize);
					(void)fread(ti->someText, size, 1, f);
					fclose(f);
					ti->someText[size] = '\0';
				} else {
					ti->someText = Strdup(path);
					bufSize = (AG_Size)strlen(ti->someText)+1;
				}
			} else {
				ti->someText = AG_Strdup("loss.txt not found");
				bufSize = (AG_Size)strlen(ti->someText)+1;
			}
	
			/*
			 * Bind the buffer's contents to the Textbox. The
			 * size argument to AG_TextboxBindUTF8() must include
			 * space for the terminating NUL.
			 */
			AG_TextboxBindUTF8(tbox, ti->someText, bufSize);
	
			/* Add a word wrapping control */
			AG_CheckboxNewFn(nt, 0, "Word wrapping",
			    SetWordWrap, "%p", tbox);
		}
		AG_NotebookSelect(nb, nt);

		nt = AG_NotebookAdd(nb, "Colors\n"
		    AGSI_RED AGSI_BOLD "R" AGSI_RST
		    AGSI_GRN AGSI_BOLD "G" AGSI_RST
		    AGSI_BLU AGSI_BOLD "B" AGSI_RST,
		    AG_BOX_VERT);
		AGBOX(nt)->flags |= AG_BOX_HOMOGENOUS;
		AG_SetPadding(nt, "10");
		{
			AG_HSVPal *pal;
			const Uint flags = AG_HSVPAL_HFILL | AG_HSVPAL_SHOW_RGB;

			pal = AG_HSVPalNew(nt, flags);
			pal->h = 0.0f;
			pal->s = 1.0f;
			pal->v = 1.0f;
			pal = AG_HSVPalNew(nt, flags);
			pal->h = 120.0f;
			pal->s = 1.0f;
			pal->v = 1.0f;
			pal = AG_HSVPalNew(nt, flags);
			pal->h = 240.0f;
			pal->s = 1.0f;
			pal->v = 1.0f;
		}
	}
	return (0);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->textBuffer[0] = '\0';
	ti->someText = NULL;
	return (0);
}

static void
Destroy(void *obj)
{
	MyTestInstance *ti = obj;

	Free(ti->someText);
	AG_TextFree(ti->textElement);
}

const AG_TestCase widgetsTest = {
	AGSI_IDEOGRAM AGSI_POPULATED_WINDOW AGSI_RST,
	"widgets",
	N_("Display various Agar-GUI widgets"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	Destroy,
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

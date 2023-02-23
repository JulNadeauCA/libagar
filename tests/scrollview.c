/*	Public domain	*/
/*
 * This application demonstrates the use of the AG_Scrollview(3) widget.
 */

#include "agartest.h"

static void
Header(void *_Nonnull parent)
{
	AG_Label *lbl;

	lbl = AG_LabelNewS(parent, AG_LABEL_HFILL,
	    _(AGSI_IDEOGRAM AGSI_SCROLLVIEW AGSI_RST
	     " Test for AG_Scrollview "
	     AGSI_IDEOGRAM AGSI_SCROLLVIEW));
	AG_SetFontFamily(lbl, "league-spartan");
	AG_SetFontSize(lbl, "160%");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
}

static void
TestWithButtons(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1), *win;
	AG_Box *hBox;
	AG_Scrollview *sv;
	int x, y;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	Header(win);
	AG_LabelNewS(win, 0,
	    "An " AGSI_CYAN AGSI_CODE "AG_Scrollview" AGSI_RST
	    " with arrays of " AGSI_CYAN AGSI_CODE "AG_Button" AGSI_RST ":");

	AG_ButtonNew(win, AG_BUTTON_EXCL, "Foo");
	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND);
	for (y = 0; y < 20; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		AG_SetSpacing(hBox, "0");
		AG_SetPadding(hBox, "0");
		for (x = 0; x < 20; x++) {
			AG_ButtonNew(hBox, AG_BUTTON_EXCL,
			    AGSI_AGAR_MINIMAL "%c\n" "%02d",    /* Monospace */
			    (char)(0x41+x), y);
		}
	}
	AG_ButtonNew(win, AG_BUTTON_EXCL, "Bar");
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 400, 300);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
OnLabelShown(AG_Event *event)
{
	Debug(AG_LABEL_SELF(), "Shown!\n");
}

static void
OnLabelHidden(AG_Event *event)
{
	Debug(AG_LABEL_SELF(), "Hidden!\n");
}

static void
TestWithLabels(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1), *win;
	AG_Box *hBox;
	AG_Scrollview *sv;
	AG_Label *lbl;
	int x, y;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "Scrollview (array of Labels)");
	Header(win);
	AG_LabelNewS(win, 0,
	    "An " AGSI_CYAN AGSI_CODE "AG_Scrollview" AGSI_RST
	    " with arrays of " AGSI_CYAN AGSI_CODE "AG_Label" AGSI_RST "\n"
	    "(and " AGSI_CODE "AG_SCROLLVIEW_BY_MOUSE" AGSI_RST " set):");

	sv = AG_ScrollviewNew(win, AG_SCROLLVIEW_EXPAND |
	                           AG_SCROLLVIEW_BY_MOUSE |
	                           AG_SCROLLVIEW_PAN_LEFT |
	                           AG_SCROLLVIEW_PAN_RIGHT);

	for (y = 0; y < 40; y++) {
		hBox = AG_BoxNewHoriz(sv, 0);
		for (x = 0; x < 10; x++) {
			lbl = AG_LabelNew(hBox, 0, "Hello (%d,%d)", x,y);
			AG_AddEvent(lbl, "widget-shown", OnLabelShown, NULL);
			AG_AddEvent(lbl, "widget-hidden", OnLabelHidden, NULL);
		}
	}
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 320, 240);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Box *box;

	box = AG_BoxNewVert(win, AG_BOX_EXPAND);
	AG_SetPadding(box, "0");
	Header(box);

	AG_ButtonNewFn(box, AG_BUTTON_HFILL, _("Array of Buttons"),
	    TestWithButtons,"%p",win);
	AG_ButtonNewFn(box, AG_BUTTON_HFILL, _("Array of Labels"),
	    TestWithLabels,"%p",win);

	return (0);
}

const AG_TestCase scrollviewTest = {
	AGSI_IDEOGRAM AGSI_SCROLLVIEW AGSI_RST,
	"scrollview",
	N_("Test the AG_Scrollview(3) widget"),
	"1.6.0",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

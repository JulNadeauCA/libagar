/*	Public domain	*/

#include "agartest.h"

typedef struct {
	AG_TestInstance _inherit;
	Uint myValue;
} MyTestInstance;

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Label *lbl;
	MyTestInstance *ti = obj;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Radio *rad;

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, "Horizontal", AG_BOX_VERT);
	{
		const char *items[] = {
			"Bart", "Homer", "Lisa", "Maggie", "Marge", NULL
		};
		lbl = AG_LabelNewPolled(nt, AG_LABEL_HFILL | AG_LABEL_FRAME,
		    "Radio Group Test.  Current Value:" AGSI_BOLD "%i" AGSI_RST ".",
		    &ti->myValue);
		AG_LabelSizeHint(lbl, 1, "<Radio Group Test.  Current Value: XXXX.>");
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
		AG_LabelValign(lbl, AG_TEXT_TOP);

		rad = AG_RadioNewUint(nt, AG_RADIO_HFILL | AG_RADIO_HOMOGENOUS,
		    items, &ti->myValue);
		AG_SetPadding(rad, "10");
		AG_SetFontFamily(rad, "league-gothic");
		AG_RadioSetDisposition(rad, AG_RADIO_HORIZ);
	}

	nt = AG_NotebookAdd(nb, "Vertical", AG_BOX_VERT);
	{
		const char *items[] = {
			"Homer Simpson", "Marge Simpson", "Bart Simpson",
			"Lisa Simpson", "Maggie Simpson", "Santa's Little Helper",
			"Snowball II/V", "Abraham Simpson", "Abu Nahasapeemapetilon",
			"Barney Gumble", "Clancy Wiggum", "Dewey Largo", "Eddie",
			"Edna Krabappel", "Itchy", "Janey Powell", "Jasper Beardsley",
			"Kent Brockman", "Krusty The Clown", "Lenny Leonard",
			"Lou", "Martin Prince", "Milhouse Van Houten", "Moe Szyslak",
			"Mr. Burns", "Ned Flanders", "Otto Mann", "Patty Bouvier",
			"Ralph Wiggum", NULL
		};
		lbl = AG_LabelNewPolled(nt, AG_LABEL_HFILL | AG_LABEL_FRAME,
		    "Radio Group Test\n"
		    "Value: " AGSI_BOLD "%i" AGSI_RST,
		    &ti->myValue);
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
		AG_LabelValign(lbl, AG_TEXT_TOP);
		rad = AG_RadioNewUint(nt, 0, items, &ti->myValue);
		AG_SetFontSize(rad, "80%");
		AG_SetPadding(rad, "10");
	}

	return (0);
}

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;

	ti->myValue = 1;
	return (0);
}

const AG_TestCase radioTest = {
	AGSI_IDEOGRAM AGSI_RADIO_BUTTON AGSI_RST,
	"radio",
	N_("Test AG_Radio(3) group"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

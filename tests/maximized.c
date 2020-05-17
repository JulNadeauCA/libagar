/*	Public domain	*/
/*
 * This application displays a maximized window with zero padding.
 */

#include "agartest.h"

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Label *lbl;
	AG_Table *table;
	int i;

	AG_SetStyle(win, "padding", "0");

	lbl = AG_LabelNew(win, AG_LABEL_HFILL, "Testing AG_WindowMaximize(3)");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);
	AG_SpacerNewHoriz(win);

	/* Create an example table. */
	table = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableAddCol(table, "Foo", "<8888>", NULL);
	AG_TableAddCol(table, "Bar", NULL, NULL);
	for (i = 0; i < 100; i++)
		AG_TableAddRow(table, "%d:%s", i, "Foo");

	/* Maximize the window. */
	AG_WindowMaximize(win);
	return (0);
}

const AG_TestCase maximizedTest = {
	"maximized",
	N_("Test AG_WindowMaximize(3)"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

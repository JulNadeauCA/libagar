/*	Public domain	*/

/*
 * This application demonstrates some uses for the AG_Table(3) widget.
 *
 * In EXAMPLE 1, we populate a Table statically and we demonstrate how the
 * %[Ft] and %[Fn] elements can be used to provide dynamically-updated text
 * and image cells.
 *
 * In EXAMPLE 2, we construct a Table where the rows themselves are
 * dynamically updated (as opposed to simply the cell values).
 *
 * In EXAMPLE 3, we show how arbitrary widgets can be inserted into a Table
 * and just how conveniently Agar bindings can handle the situation.
 */

#include "agartest.h"

#include <string.h>

#include <agar/core/snprintf.h>

/* This function is called to sort the elements of a column (Ex.1) */
static int
MyCustomSortFn(const void *p1, const void *p2)
{
	const AG_TableCell *c1 = p1;
	const AG_TableCell *c2 = p2;

	return (c1->data.i - c2->data.i);
}

/* This is a custom cell function which returns text into s (Ex.1) */
static void
MyCustomDynamicTextFn(void *p, char *s, AG_Size len)
{
/*	AG_TableCell *cell = p; */
	
	AG_Snprintf(s, len, "Ticks: %lu",
	    (unsigned long)AG_GetTicks());
}

/*
 * This is a cell function (%[Fs]) which returns a newly-allocated surface.
 *
 * The alternate %[FS] specifier may also be used to set a function that
 * returns a surface to be reused without duplication.
 */
static AG_Surface *
MyCustomSurfaceFn(void *p, int x, int y)
{
	return AG_SurfaceDup(agIconLoad.s);
}

/*
 * EXAMPLE 1:
 * A statically constructed table.
 */
static void
CreateStaticTable(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1);
	AG_Window *win;
	AG_Table *table;
	int i;

	/* Create our window. */
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "Example 1: Static Table");

	/*
	 * Create the table. We could have used the AG_TABLE_MULTI flag if
	 * we wanted to allow multiple selections, or the AG_TABLE_POLL
	 * flag to update the table periodically.
	 */
	table = AG_TableNew(win, AG_TABLE_EXPAND);
	
	/*
	 * Create a column large enough to hold the string "<HIDDEN POINTER>".
	 * Other types of size specifications are possible, such as "123px"
	 * for a number of pixels, "n%" for a percentage and NULL to request
	 * that the column use all remaining space.
	 */
	AG_TableAddCol(table, "Some string", "<HIDDEN POINTER>", NULL);

	/* Create another column with a specific sorting function. */
	AG_TableAddCol(table, "Val", "<8888>", MyCustomSortFn);
	
	/* Create a third column using the remaining space. */
	AG_TableAddCol(table, "Stuff", NULL, NULL);

	/* Now begin adding rows to the table. */
	AG_TableBegin(table);

	/*
	 * The AG_TableAddRow() function accepts a format string which specifies
	 * the type of the following arguments. Columns can have cells of
	 * different types, but the sorting function must handle them.
	 */
	for (i = 0; i < 20; i++) {
		/* Display a string and two integers. */
		AG_TableAddRow(table, "%s:%d:%d", "Foo", i, i*2);
		
		/* Display a string and two floating point numbers. */
		AG_TableAddRow(table, "%s:%.03f:%.02f", "Bar", 1.0/(float)i,
		    2.0/3.0);
		
		/* The %[Ft] element is a function that returns text. */
		AG_TableAddRow(table, "%s:%d:%[Ft]", "Baz", i,
		    MyCustomDynamicTextFn);
		
		/*
		 * The %[Fs] element is a function that returns a surface
		 * (see AG_Surface(3)).
		 * The %[FS] variant sets the "NODUP" flag (so that Agar
		 * will never free the surface automatically).
		 */
		AG_TableAddRow(table, "%s:%d:%[Fs]", "Foo", i,
		    MyCustomSurfaceFn);
	}

	/*
	 * It is also possible to insert "hidden" fields in rows. This is
	 * typically used for user pointers.
	 */
	AG_TableAddRow(table, "%s:%d:%i:%p", "Hidden pointer", 1, 1, table);
	
	/* Make sure to call this when you're done adding rows. */
	AG_TableEnd(table);

	/* Display and resize our window. */
	AG_WindowSetGeometryAligned(win, AG_WINDOW_BC, 320, 240);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

#ifdef AG_TIMERS

/* This is our callback function for updating our dynamic table (Ex.2) */
static void
UpdateTable(AG_Event *event)
{
	AG_Table *t = AG_TABLE_SELF();
	static int prev = 0;
	static int dir = +1;
	int i;

	AG_TableBegin(t);
	for (i = 0; i < prev; i++) {
		AG_TableAddRow(t, "%d:Element %d", i, i);
	}
	AG_TableEnd(t);

	if (dir < 0) {
		if (--prev < 0) { prev = 0; dir = +1; }
	} else {
		if (++prev > 100) { prev = 100; dir = -1; }
	}
}

/*
 * EXAMPLE 2: A dynamically constructed table. As opposed to dynamically
 * updating cell values, the table is completely recreated at periodic
 * intervals.
 */
static void
PausePolling(AG_Event *event)
{
	AG_Table *tbl = AG_TABLE_PTR(1);
	int state = AG_INT(2);

	if (!state) {
		AG_TableSetPollInterval(tbl, 100);
	} else {
		AG_TableSetPollInterval(tbl, 0);
	}
}
static void
CreatePolledTable(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1);
	AG_Window *win;
	AG_Table *table;
	AG_Box *box;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "Example 2: Polled Table");

	table = AG_TableNewPolled(win, AG_TABLE_EXPAND, UpdateTable, NULL);
	AG_TableSizeHint(table, 200, 20);
	AG_TableSetPollInterval(table, 100);
	AG_TableAddCol(table, "A", "<8888888>", NULL);
	AG_TableAddCol(table, "B", "<888888888888>", NULL);

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_ButtonNewFn(box, AG_BUTTON_STICKY, "Pause polling",
		    PausePolling,"%p",table);
		AG_ButtonNewFn(box, 0, "Close", AGWINCLOSE(win));
	}

	AG_WindowSetPosition(win, AG_WINDOW_ML, 1);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}
#endif /* AG_TIMERS */

/* Report on the status of our test array (Ex.3) */
static void
CountSelected(AG_Event *event)
{
	int *MyTable = AG_PTR(1);
	int i, total = 0;

	for (i = 0; i < 20; i++) {
		if (MyTable[i] == 1)
			total++;
	}
	AG_TextMsg(AG_MSG_INFO, "%d rows are selected", total);
}

/* Clear all rows (Ex.3) */
static void
ClearAllRows(AG_Event *event)
{
	int *MyTable = AG_PTR(1);

	memset(MyTable, 0, 20*sizeof(int));
}

/*
 * EXAMPLE 3:
 * Table with embedded control widgets (%[W] row element).
 */
static void
CreateTableWithWidgets(AG_Event *event)
{
	AG_Window *winParent = AG_WINDOW_PTR(1);
	static int MyTable[20];
	AG_Window *win;
	AG_Table *table;
	AG_Box *box;
	int i;

	/* Create our window. */
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "Example 3: Table With Embedded Widgets");

	/* Create our table. */
	table = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableSizeHint(table, 200, 20);

	/* Create our columns. */
	AG_TableAddCol(table, AGSI_ALGUE AGSI_BLACK_AGAR, "<XX>", NULL);
	AG_TableAddCol(table, "Items", "<Example row #10>", NULL);

	/* Initialize our test array. */
	memset(MyTable, 0, 20*sizeof(int));

	/* Insert the rows. */
	AG_TableBegin(table);
	for (i = 0; i < 20; i++) {
		AG_Button *button;

		/*
		 * The %[W] specifier allows us to insert an arbitrary widget
		 * into the table. In this case, we will insert a Button
		 * bound to an entry in our MyTable array.
		 *
		 * It is important to pass NULL as the "parent" argument to
		 * ButtonNew() since TableAddRow() will attach the Button to
		 * the Table for us.
		 */
		button = AG_ButtonNewInt(NULL, AG_BUTTON_STICKY,
		    (i % 2) == 0 ? AGSI_ALGUE AGSI_SML_FACE_W_OM :
		                   AGSI_ALGUE AGSI_CAT_FACE,
		    &MyTable[i]);
		AG_TableAddRow(table, "%[W]:Example row #%d", button, i+1);
		AG_SetStyle(button, "color", (i % 2) == 0 ? "#83b5b5" :
		                                            "#d9b0de");
		AG_SetStyle(button, "text-color", "#000");
	}
	AG_TableEnd(table);

	/* Provide a function to report on the status of MyTable. */
	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_ButtonNewFn(box, 0, "Count Selected", CountSelected,"%p",MyTable);
		AG_ButtonNewFn(box, 0, "Clear", ClearAllRows,"%p",MyTable);
		AG_ButtonNewFn(box, 0, "Close", AGWINCLOSE(win));
	}

	/* Display and resize our window. */
	AG_WindowSetPosition(win, AG_WINDOW_MR, 1);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_Label *lbl;

	AG_SetStyle(win, "background-color", "#404045");

	lbl = AG_LabelNewS(win, 0, _("Tests for AG_Table(3)"));
	AG_SetStyle(lbl, "font-family", "cm-serif");
	AG_SetStyle(lbl, "font-weight", "cm-bold");
	AG_SetStyle(lbl, "font-size", "150%");

	AG_ButtonNewFn(win, AG_BUTTON_HFILL,
	    "Create a " AGSI_BOLD "static" AGSI_RST " table",
	    CreateStaticTable,"%p",win);
#ifdef AG_TIMERS
	AG_ButtonNewFn(win, AG_BUTTON_HFILL,
	    "Create a " AGSI_BOLD "polled" AGSI_RST " table",
	    CreatePolledTable,"%p",win);
#else
	AG_PushDisabledState(win);
	AG_ButtonNewS(win, 0, "Create polled table");
	AG_PopDisabledState(win);
#endif
	AG_ButtonNewFn(win, AG_BUTTON_HFILL,
	    "Create a table with " AGSI_BOLD "widgets",
	    CreateTableWithWidgets,"%p",win);

	return (0);
}

const AG_TestCase tableTest = {
	"table",
	N_("Test the AG_Table(3) widget"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};

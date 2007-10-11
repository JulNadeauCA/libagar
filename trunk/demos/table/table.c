/*	Public domain	*/
/*
 * This application demonstrates the use of the AG_Table widget for
 * displaying the contents of a list.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>
#include <unistd.h>

/* This function is called to sort the elements of a column. */
static int
my_sort_fn(const void *p1, const void *p2)
{
	const AG_TableCell *c1 = p1;
	const AG_TableCell *c2 = p2;

	return (c1->data.i - c2->data.i);
}

/* This is a custom cell function which returns text into s. */
static void
my_custom_cell_txt_fn(void *p, char *s, size_t len)
{
	AG_TableCell *cell = p;
	
	snprintf(s, len, "Ticks: %lu",
	    (unsigned long)SDL_GetTicks());
}

/* This function creates a static table. */
static void
CreateStaticTable(void)
{
	AG_Window *win;
	AG_Button *btn;
	AG_Table *table;
	int i;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Static Table");

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
	AG_TableAddCol(table, "Val", "<8888>", my_sort_fn);
	
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
		AG_TableAddRow(table, "%s:%.03f:%g", "Bar", 1.0/(float)i,
		    2.0/3.0);
		
		/* Provide a custom cell function that returns text. */
		AG_TableAddRow(table, "%s:%d:%[Ft]", "Baz", 1,
		    my_custom_cell_txt_fn);
	}

	/*
	 * It is also possible to associate "hidden" fields with rows. This
	 * is useful for storing user pointers, for instance.
	 */
	AG_TableAddRow(table, "%s:%d:%i:%p", "Hidden pointer", 1, 1,
	    (void *)0xdeadbeef);
	
	/* Make sure to call this when you're done adding rows. */
	AG_TableEnd(table);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win,
	    agView->w/2 - 320/2,
	    agView->h/2 - 240/2,
	    320, 240);
}

static void
UpdateTable(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	static int prev = 0;
	static int dir = +1;
	int i;

	AG_TableBegin(t);
	for (i = 0; i < prev; i++) {
		AG_TableAddRow(t, "%d:%u", i, (unsigned)SDL_GetTicks());
	}
	AG_TableEnd(t);

	if (dir < 0) {
		if (--prev < 0) { prev = 0; dir = +1; }
	} else {
		if (++prev > 100) { prev = 100; dir = -1; }
	}
}

/* This function creates a polled table. */
static void
CreatePolledTable(void)
{
	AG_Window *win;
	AG_Button *btn;
	AG_Table *table;
	int i;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Polled Table");

	/* Create a polled table. */
	table = AG_TableNewPolled(win, AG_TABLE_EXPAND, UpdateTable, NULL);
	AG_TableAddCol(table, "Foo", "<8888>", NULL);
	AG_TableAddCol(table, "Bar", "<888888888>", NULL);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win,
	    agView->w/2 - 320/3,
	    agView->h/2 - 240/3,
	    320, 240);
}

int
main(int argc, char *argv[])
{
	int c, i, fps = -1;
	char *s;

	if (AG_InitCore("table-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}

	while ((c = getopt(argc, argv, "?vfFgGr:")) != -1) {
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
		case '?':
		default:
			printf("%s [-vfFgG] [-r fps]\n", agProgName);
			exit(0);
		}
	}

	/* Initialize a 640x480x32 display. Respond to keyboard/mouse events. */
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitInput(0);
	AG_SetRefreshRate(fps);
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
	CreateStaticTable();
	CreatePolledTable();

	AG_EventLoop();
	AG_Destroy();
	return (0);
fail:
	AG_Destroy();
	return (1);
}


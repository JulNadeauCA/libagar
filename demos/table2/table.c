/*	Public domain	*/

/*
 * This application tests different uses for the AG_Table(3) widget.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include <string.h>

#include <agar/core/snprintf.h>

AG_Surface *PixMeme, *PixAgar;

/* This is a custom cell function which returns text into s (Ex.1) */
static void
MyCustomDynamicTextFn(void *p, char *s, size_t len)
{
	AG_TableCell *cell = p;
	
	AG_Snprintf(s, len, "Ticks: %lu",
	    (unsigned long)SDL_GetTicks());
}

/* This is a custom cell function which returns a surface to display (Ex.1) */
static AG_Surface *
MyCustomSurfaceFn(void *p, int x, int y)
{
	/* Return the surface of a built-in icon. */
	return (agIconLoad.s);
}

static void
CreateTable1(void)
{
	AG_Window *win;
	AG_Table *table;
	int i;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Example 1: Static Table");

	table = AG_TableNew(win, 0);
	AG_Expand(table);
	
	AG_TableAddCol(table, "This column", "<Foobar>", NULL);
	AG_TableAddCol(table, "Sorted column", "100px", NULL);
	AG_TableAddCol(table, "Expanded column", "-", NULL);

	AG_TableBegin(table);
	for (i = 0; i < 20; i++) {
		AG_TableAddRow(table, "%s:%d:%d", "Foo", i, i*2);
		AG_TableAddRow(table, "%s:%.03f:%.02f", "Bar", 1.0/(float)i,
		    2.0/3.0);
		AG_TableAddRow(table, "%s:%d:%[Ft]", "Baz", i,
		    MyCustomDynamicTextFn);
		AG_TableAddRow(table, "%s:%d:%[Fs]", "Foo", i,
		    MyCustomSurfaceFn);
	}
	AG_TableAddRow(table, "%s:%d:%i:%p", "Hidden pointer", 1, 1, table);
	AG_TableEnd(table);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_BC, 320, 240);
	AG_WindowShow(win);
}

static void
CreateTable3(void) 
{
        AG_Window *win;
        AG_Table *table;
        int x=0;

        win = AG_WindowNew(0);
	AG_WindowSetCaption(win,
	    "Similar Cases to %s by Morphological Analysis", "Your Case");
        AG_WindowSetGeometry(win, 0, 0, 320, 240);
        AG_WindowSetPosition(win, AG_WINDOW_LOWER_CENTER, 0);
            
        table = AG_TableNew(win, 0);
	AG_Expand(table);

        AG_LabelNew(win, 0, "%d total", 100);
    
        /* Create our columns. */
	AG_TableAddCol(table, "Morphology Results", "140px", NULL);
	AG_TableAddCol(table, " ", "64px", NULL);
	AG_TableAddCol(table, "Pix1", "100px", NULL);
	AG_TableAddCol(table, "Pix2", "-", NULL);
        AG_TableSetRowHeight(table,50);
        
	/* Insert the rows. */
	AG_TableBegin(table);
	for (x=0; x<50; x++) {

		AG_Button *button, *button2;
                AG_Pixmap *pixmap1, *pixmap2;
                char b[16384];
             
                AG_Snprintf(b, sizeof(b),
		    "%d: %s\n%d Agree\n #%d\n %s", x,
		    "hello", 12094, 12490, "world"); 

		button = AG_ButtonNewFn(NULL, 0, b, NULL, NULL);
                AG_ButtonJustify(button, AG_TEXT_LEFT);

		button2 = AG_ButtonNewFn(NULL, 0, "Some\nHow", NULL, NULL);
                AG_ButtonJustify(button, AG_TEXT_CENTER);

		pixmap1 = AG_PixmapFromSurfaceCopy(NULL, 0, PixAgar);
		pixmap2 = AG_PixmapFromSurfaceCopy(NULL, 0, PixMeme);
                
		AG_TableAddRow(table, "%[W]:%[W]:%[W]:%[W]",
		    button, button2, pixmap1, pixmap2, NULL);
	}
	AG_TableEnd(table);         

        AG_ButtonNewFn(win, 0, "Rank By ", NULL, NULL );
        AG_ButtonNewFn(win, 0, "Search again ", NULL, NULL );
                
	AG_WindowShow(win);           
}

/* Report on the status of our test array (Ex.3) */
static void
ReportSelectedRows(AG_Event *event)
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

static void
CreateTable2(void)
{
	static int MyTable[20];
	AG_Window *win;
	AG_Table *table;
	AG_Box *box;
	int i;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Example 3: Table With Embedded Widgets");

	table = AG_TableNew(win, 0);
	AG_TableAddCol(table, "Button", "20%", NULL);
	AG_TableAddCol(table, "Button2", "20%", NULL);
	AG_TableAddCol(table, "Items", "20%", NULL);
	AG_TableAddCol(table, "Pixmap1", "20%", NULL);
	AG_TableAddCol(table, "Pixmap2", "20%", NULL);
        AG_TableSetRowHeight(table,120);
	AG_Expand(table);

	memset(MyTable, 0, 20*sizeof(int));

	AG_TableBegin(table);
	for (i = 0; i < 20; i++) {
		AG_Button *button, *button2;

		button = AG_ButtonNewInt(NULL, AG_BUTTON_STICKY,
		    "This line of text\n"
		    "This Line of Text\n"
		    "This Line of Text\n"
		    "This Line of Text",
		    &MyTable[i]);
		button2 = AG_ButtonNewInt(NULL, AG_BUTTON_STICKY, "Select",
		    &MyTable[i]);

		AG_TableAddRow(table,
		    "%[W]:%[W]:Row %d:%[W]:%[W]", button, button2, i,
		    AG_PixmapFromSurfaceCopy(NULL,AG_PIXMAP_RESCALE,PixMeme),
		    AG_PixmapFromSurfaceCopy(NULL,AG_PIXMAP_RESCALE,PixAgar));
	}
	AG_TableEnd(table);

	box = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS);
	AG_ExpandHoriz(box);
	{
		AG_ButtonNewFn(box, 0, "Report selected rows",
		    ReportSelectedRows, "%p", MyTable);
		AG_ButtonNewFn(box, 0, "Clear rows",
		    ClearAllRows, "%p", MyTable);
	}

	/* Display and resize our window. */
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MR, 500, 300);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{	
	int guiFlags = AG_VIDEO_RESIZABLE;

	if (AG_InitCore("agar-table-demo", 0) == -1)  {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (argc > 1) {
		if (strcmp(argv[1], "-g") == 0) {
			printf("Forcing GL mode\n");
			AG_SetBool(agConfig, "view.opengl", 1);
			guiFlags |= AG_VIDEO_OPENGL;
		} else if (strcmp(argv[1], "-G") == 0) {
			printf("Forcing FB mode\n");
			AG_SetBool(agConfig, "view.opengl", 0);
		}
	}
	if (AG_InitVideo(640, 480, 32, guiFlags) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	
        if ((PixMeme = AG_SurfaceFromBMP("Meme.bmp")) == NULL)
		PixMeme = AG_TextRender(AG_GetError());
        if ((PixAgar = AG_SurfaceFromBMP("Agar.bmp")) == NULL)
		PixAgar = AG_TextRender(AG_GetError());

	CreateTable1();
	CreateTable2();
        CreateTable3();

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

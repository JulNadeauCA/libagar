/*	Public domain	*/

/* Structure describing an instance of the MY_Widget class. */
typedef struct my_widget {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	void *ti;			/* Agar test instance (for agartest) */
	int label;			/* Surface handle */
	const char *foo;		/* Some parameter */
	int x, y;			/* Last mouse coordinates */
	int radius;			/* Circle radius */
	AG_KeySym lastKey;		/* Last key pressed */
	AG_Char   lastUnicode;		/* Last key (in Unicode) */
	AG_Timer  clickTimer;		/* Timer for mousebuttondown */
} MY_Widget;

/* Accessor macros */
#define   MYWIDGET(o)        ((MY_Widget *)(o))
#define  MYcWIDGET(o)        ((const MY_Widget *)(o))
#define  MY_WIDGET_SELF()    MYWIDGET(  AG_OBJECT(0,         "AG_Widget:MY_Widget:*") )
#define  MY_WIDGET_PTR(n)    MYWIDGET(  AG_OBJECT((n),       "AG_Widget:MY_Widget:*") )
#define  MY_WIDGET_NAMED(n)  MYWIDGET(  AG_OBJECT_NAMED((n), "AG_Widget:MY_Widget:*") )
#define MY_cWIDGET_SELF()   MYcWIDGET( AG_cOBJECT(0,         "AG_Widget:MY_Widget:*") )
#define MY_cWIDGET_PTR(n)   MYcWIDGET( AG_cOBJECT((n),       "AG_Widget:MY_Widget:*") )
#define MY_cWIDGET_NAMED(n) MYcWIDGET( AG_cOBJECT_NAMED((n), "AG_Widget:MY_Widget:*") )

extern AG_WidgetClass myWidgetClass;
MY_Widget *MY_WidgetNew(void *, const char *);

/*	Public domain	*/

/* Structure describing an instance of the BezierWidget class. */
typedef struct bezier_widget {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	void *ti;			/* Agar test instance (for agartest) */
	int label;			/* Surface handle */
	const char *foo;		/* Some parameter */
	int x, y;			/* Last mouse coordinates */
	int radius;			/* Circle radius */
	AG_KeySym lastKey;		/* Last key pressed */
	AG_Char   lastUnicode;		/* Last key (in Unicode) */
	AG_Timer  clickTimer;		/* Timer for mousebuttondown */
} BezierWidget;

/* Accessor macros */
#define MYWIDGET(obj)           ((BezierWidget *)(obj))
#define MYCWIDGET(obj)          ((const BezierWidget *)(obj))
#define MYWIDGET_SELF()          MYWIDGET( AG_OBJECT(0,"AG_Widget:BezierWidget:*") )
#define MYWIDGET_PTR(n)          MYWIDGET( AG_OBJECT((n),"AG_Widget:BezierWidget:*") )
#define MYWIDGET_NAMED(n)        MYWIDGET( AG_OBJECT_NAMED((n),"AG_Widget:BezierWidget:*") )
#define CONST_MYWIDGET_SELF()   MYCWIDGET( AG_CONST_OBJECT(0,"AG_Widget:BezierWidget:*") )
#define CONST_MYWIDGET_PTR(n)   MYCWIDGET( AG_CONST_OBJECT((n),"AG_Widget:BezierWidget:*") )
#define CONST_MYWIDGET_NAMED(n) MYCWIDGET( AG_CONST_OBJECT_NAMED((n),"AG_Widget:BezierWidget:*") )

extern AG_WidgetClass bezierWidgetClass;
BezierWidget *BezierWidgetNew(void *, const char *);

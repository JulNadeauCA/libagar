/*	Public domain	*/

/* Structure describing an instance of the MyWidget class. */
typedef struct my_widget {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	void *ti;			/* Agar test instance (for agartest) */
	int mySurface;			/* Surface handle */
	const char *foo;		/* Some parameter */
	int x, y;			/* Last mouse coordinates */
} MyWidget;

/* Accessor macros */
#define MYWIDGET(obj)           ((MyWidget *)(obj))
#define MYCWIDGET(obj)          ((const MyWidget *)(obj))
#define MYWIDGET_SELF()          MYWIDGET( AG_OBJECT(0,"MyWidget:*") )
#define MYWIDGET_PTR(n)          MYWIDGET( AG_OBJECT((n),"MyWidget:*") )
#define MYWIDGET_NAMED(n)        MYWIDGET( AG_OBJECT_NAMED((n),"MyWidget:*") )
#define CONST_MYWIDGET_SELF()   MYCWIDGET( AG_CONST_OBJECT(0,"MyWidget:*") )
#define CONST_MYWIDGET_PTR(n)   MYCWIDGET( AG_CONST_OBJECT((n),"MyWidget:*") )
#define CONST_MYWIDGET_NAMED(n) MYCWIDGET( AG_CONST_OBJECT_NAMED((n),"MyWidget:*") )

extern AG_WidgetClass myWidgetClass;
MyWidget *MyWidgetNew(void *, const char *);

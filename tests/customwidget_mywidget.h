/*	Public domain	*/

/* Structure describing an instance of the MyWidget class. */
typedef struct my_widget {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	void *ti;			/* Agar test instance (for agartest) */
	int mySurface;			/* Surface handle */
	const char *foo;		/* Some parameter */
	int x, y;			/* Last mouse coordinates */
} MyWidget;

#define MYWIDGET(obj)           ((MyWidget *)(obj))
#define MYWIDGET_SELF()         AG_OBJECT(0,"MyWidget:*")
#define MYWIDGET_PTR(n)         AG_OBJECT((n),"MyWidget:*")
#define MYWIDGET_NAMED(n)       AG_OBJECT_NAMED((n),"MyWidget:*")
#define CONST_MYWIDGET_SELF()   AG_CONST_OBJECT(0,"MyWidget:*")
#define CONST_MYWIDGET_PTR(n)   AG_CONST_OBJECT((n),"MyWidget:*")
#define CONST_MYWIDGET_NAMED(n) AG_CONST_OBJECT_NAMED((n),"MyWidget:*")

extern AG_WidgetClass myWidgetClass;
MyWidget *MyWidgetNew(void *, const char *);

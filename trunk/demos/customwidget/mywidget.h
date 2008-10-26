/*	Public domain	*/

/* Structure describing an instance of the MyWidget class. */
typedef struct my_widget {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	int mySurface;			/* Surface handle */
	const char *foo;		/* Some parameter */
} MyWidget;

extern AG_WidgetClass myWidgetClass;
MyWidget *MyWidgetNew(void *, const char *);

/*	Public domain	*/

/*
 * Implementation of a typical Agar widget which uses surface mappings to
 * efficiently draw surfaces, regardless of the underlying graphics system.
 *
 * If you are not familiar with the way the Agar object system handles
 * inheritance, see demos/objsystem.
 */

#include "agartest.h"
#include "customwidget_mywidget.h"

/*
 * This is a generic constructor function. It is completely optional, but
 * customary of FooNew() functions to allocate, initialize and attach an
 * instance of the class.
 */
MyWidget *
MyWidgetNew(void *parent, const char *foo)
{
	MyWidget *my;

	/* Create a new instance of the MyWidget class */
	my = malloc(sizeof(MyWidget));
	AG_ObjectInit(my, &myWidgetClass);

	/* Set some constructor arguments */
	my->foo = foo;

	/* Attach the object to the parent (no-op if parent is NULL) */
	AG_ObjectAttach(parent, my);

	return (my);
}

/*
 * This function requests a minimal geometry for displaying the widget.
 * It is expected to return the width and height in pixels into r.
 *
 * Note: Some widgets will provide FooSizeHint() functions to allow the
 * programmer to request an initial size in pixels or some other metric
 * FooSizeHint() typically sets some structure variable, which are then
 * used here.
 */
static void
SizeRequest(void *p, AG_SizeReq *r)
{
	MyWidget *my = p;
	
	if (my->mySurface == -1) {
		/*
		 * We can use AG_TextSize() to return the dimensions of rendered
		 * text, without rendering it.
		 */
		AG_TextSize("Custom widget!", &r->w, &r->h);
	} else {
		/*
		 * We can use the geometry of the rendered surface. The
		 * AGWIDGET_SURFACE() macro returns the AG_Surface given a
		 * Widget surface handle.
		 */
		r->w = AGWIDGET_SURFACE(my,my->mySurface)->w;
		r->h = AGWIDGET_SURFACE(my,my->mySurface)->h;
	}
}

/*
 * This function is called by the parent widget after it decided how much
 * space to allocate to this widget. It is mostly useful to container
 * widgets, but other widgets generally use it to check if the allocated
 * geometry can be handled by Draw().
 */
static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	MyWidget *my = p;

	/* If we return -1, Draw() will not be called. */
	if (a->w < 5 || a->h < 5)
		return (-1);
	
	TestMsg(my->ti, "Allocated %dx%d pixels", a->w, a->h);
	return (0);
}

/*
 * Draw function. Invoked from GUI rendering context to draw the widget
 * at its current location. All primitive and surface operations operate
 * on widget coordinates.
 */
static void
Draw(void *p)
{
	MyWidget *my = p;
	AG_Color c;
	
	/*
	 * Draw a box spanning the widget area. Use the widget's
	 * default color (the "color" style property).
	 */
	AG_DrawBox(my,
	    AG_RECT(0, 0, AGWIDGET(my)->w, AGWIDGET(my)->h), 1,
	    AG_WCOLOR(my,0));

	/*
	 * Render some text onto a surface. The default text color
	 * (the "text-color" style property) will be used.
	 */
	if (my->mySurface == -1) {
		my->mySurface = AG_WidgetMapSurface(my,
		    AG_TextRender("Custom widget!"));
	}

	c = AG_ColorRGB(250, 250, 0);
	AG_DrawLine(my, 0, 0,					my->x, my->y, c);
	AG_DrawLine(my, AGWIDGET(my)->w, 0,			my->x, my->y, c);
	AG_DrawLine(my, 0, AGWIDGET(my)->h,			my->x, my->y, c);
	AG_DrawLine(my, AGWIDGET(my)->w, AGWIDGET(my)->h,	my->x, my->y, c);
	AG_DrawCircle(my, my->x, my->y, 50, c);

	/* Draw the mapped surface centered around the cursor. */
	AG_WidgetBlitSurface(my, my->mySurface,
	    my->x - AGWIDGET_SURFACE(my,my->mySurface)->w/2,
	    my->y - AGWIDGET_SURFACE(my,my->mySurface)->h/2);
}

/* Mouse motion event handler */
static void
MouseMotion(AG_Event *event)
{
	MyWidget *my = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	if (x != my->x || y != my->y) {
		AG_Redraw(my);
	}
	my->x = x;
	my->y = y;
}

/* Mouse click event handler */
static void
MouseButtonDown(AG_Event *event)
{
	MyWidget *my = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	TestMsg(my->ti, "Click at %d,%d", x, y);
	AG_WidgetFocus(my);
}

/* Mouse click event handler */
static void
MouseButtonUp(AG_Event *event)
{
/*	MyWidget *my = AG_SELF(); */
/*	int button = AG_INT(1); */
/*	int x = AG_INT(2); */
/*	int y = AG_INT(3); */

	/* ... */
}

/* Keystroke event handler */
static void
KeyDown(AG_Event *event)
{
	MyWidget *my = AG_SELF();
	int keysym = AG_INT(1);
/*	int keymod = AG_INT(2); */
	Uint32 unicode = AG_INT(3);

	TestMsg(my->ti, "Keystroke: 0x%x (Uni=%x)", keysym, unicode);
}

/* Keystroke event handler */
static void
KeyUp(AG_Event *event)
{
/*	MyWidget *my = AG_SELF(); */
/*	int keysym = AG_INT(1); */

	/* ... */
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void
Init(void *obj)
{
	MyWidget *my = obj;

	/* Allow this widget to grab focus. */
	AGWIDGET(my)->flags |= AG_WIDGET_FOCUSABLE;

	/* Receive mouse motion events unconditionally. */
	AGWIDGET(my)->flags |= AG_WIDGET_UNFOCUSED_MOTION;

	/* The widget will be using the font engine. */
	AGWIDGET(my)->flags |= AG_WIDGET_USE_TEXT;
	
	/* The widget will react to mouse hover events. */
	AGWIDGET(my)->flags |= AG_WIDGET_USE_MOUSEOVER;

	/* Initialize instance variables. */
	my->foo = "";
	my->x = 0;
	my->y = 0;

	/*
	 * We'll eventually need to create and map a surface, but we cannot
	 * do this from Init(), because it involves texture operations in
	 * GL mode which are thread-unsafe. We wait until Draw() to do that.
	 */
	my->mySurface = -1;

	/*
	 * Map our event handlers. For a list of all meaningful events
	 * we can handle, see AG_Object(3), AG_Widget(3) and AG_Window(3).
	 *
	 * Here we register handlers for the common AG_Window(3) events.
	 */
	AG_SetEvent(my, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(my, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(my, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(my, "key-up", KeyUp, NULL);
	AG_SetEvent(my, "key-down", KeyDown, NULL);
}

/*
 * This structure describes our widget class. It inherits from AG_ObjectClass.
 * Any of the function members may be NULL. See AG_Widget(3) for details.
 */
AG_WidgetClass myWidgetClass = {
	{
		"AG_Widget:MyWidget",	/* Name of class */
		sizeof(MyWidget),	/* Size of structure */
		{ 0,0 },		/* Version for load/save */
		Init,			/* Initialize dataset */
		NULL,			/* Free dataset */
		NULL,			/* Destroy widget */
		NULL,			/* Load widget (for GUI builder) */
		NULL,			/* Save widget (for GUI builder) */
		NULL			/* Edit (for GUI builder) */
	},
	Draw,				/* Render widget */
	SizeRequest,			/* Default size requisition */
	SizeAllocate			/* Size allocation callback */
};

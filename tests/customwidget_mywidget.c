/*	Public domain	*/

/*
 * Implementation of a typical Agar widget which uses surface mappings to
 * efficiently draw surfaces, regardless of the underlying graphics system.
 *
 * If you are not familiar with the way the Agar object system handles
 * inheritance, see demos/objsystem.
 */

#include <stdlib.h>

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
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	MyWidget *my = p;
	
	if (my->label == -1) {
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
		r->w = AGWIDGET_SURFACE(my,my->label)->w;
		r->h = AGWIDGET_SURFACE(my,my->label)->h;
	}
}

/*
 * This function is called by the parent widget after it decided how much
 * space to allocate to this widget. It is mostly useful to container
 * widgets, but other widgets generally use it to check if the allocated
 * geometry can be handled by Draw().
 */
static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
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
Draw(void *_Nonnull p)
{
	MyWidget *my = p;
	const int x = my->x;
	const int y = my->y;
	const AG_Surface *S;
	AG_Color c;
	AG_Rect r;
	
	/*
	 * Draw a box over the widget area; use "background-color".
	 */
	r.x = 0;
	r.y = 0;
	r.w = AGWIDGET(my)->w;
	r.h = AGWIDGET(my)->h;
	AG_DrawBox(my, &r, 1, &AG_WCOLOR(my, AG_BG_COLOR));

	/* Establish a clipping rectangle over the entire widget area. */
	AG_PushClipRect(my, &r);

	/* Render the last pressed key as a text surface. */
	if (my->label == -1) {
		AG_TextFontPts(36.0f);
		my->label = AG_WidgetMapSurface(my,
		    AG_TextRenderF("%c", my->lastKey));
	}

	/*
	 * Draw some lines and circles.
	 */
	AG_ColorRGB_8(&c, 250,250,0);
	AG_DrawLine(my, 0,   0,   x, y, &c);
	AG_DrawLine(my, r.w, 0,   x, y, &c);
	AG_DrawLine(my, 0,   r.h, x, y, &c);
	AG_DrawLine(my, r.w, r.h, x, y, &c);
	AG_DrawCircle(my, x,y, my->radius, &c);
	AG_DrawCircle(my, x + (my->radius >> 1),
	                  y - (my->radius >> 1),
			  my->radius/10, &c);

	/*
	 * Draw a mapped label surface centered at the cursor.
	 */
	AG_PushBlendingMode(my, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	S = AGWIDGET_SURFACE(my, my->label);
	AG_WidgetBlitSurface(my, my->label, x - (S->w >> 1),
	                                    y - (S->h >> 1) - (S->h >> 1));

	AG_PopBlendingMode(my);

	AG_PopClipRect(my);
}

/* Mouse motion event handler */
static void
MouseMotion(AG_Event *_Nonnull event)
{
	MyWidget *my = MYWIDGET_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	if (x != my->x || y != my->y) {
		AG_Redraw(my);
	}
	my->x = x;
	my->y = y;
}

static Uint32
ClickTimeout(AG_Timer *to, AG_Event *_Nonnull event)
{
	MyWidget *my = MYWIDGET_SELF();
	const int maxDia = AG_MAX(AGWIDGET(my)->w,
	                          AGWIDGET(my)->h);

	AG_Redraw(my);

	if ((my->radius += 5) > maxDia) {
		my->radius = 1;
		return (0);			/* Cancel timer */
	}
	return (1);				/* Reschedule in 1ms */
}

/* Mouse click event handler */
static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	MyWidget *my = MYWIDGET_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	TestMsg(my->ti, "Click at %d,%d", x, y);
	AG_WidgetFocus(my);

	/* Set the timer to expire in 1ms and call ClickTimeout() */
	AG_AddTimer(my, &my->clickTimer, 1, ClickTimeout, NULL);
}

/* Mouse click event handler */
static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	MyWidget *my = MYWIDGET_SELF();
/*	int button = AG_INT(1); */
/*	int x = AG_INT(2); */
/*	int y = AG_INT(3); */

	/* Deactivate the running timer. */
	AG_DelTimer(my, &my->clickTimer);
}

/* Keystroke event handler */
static void
KeyDown(AG_Event *_Nonnull event)
{
	MyWidget *my = MYWIDGET_SELF();
	int keysym = AG_INT(1);
/*	int keymod = AG_INT(2); */
	AG_Char unicode = AG_CHAR(3);

	TestMsg(my->ti, "Keystroke: 0x%x (Uni=%x)", keysym, unicode);
	my->lastKey = keysym;
	my->lastUnicode = unicode;

	if (my->label != -1) {				/* Invalidate cached */
		AG_WidgetUnmapSurface(my, my->label);
		my->label = -1;
		AG_Redraw(my);
	}
}

/* Keystroke event handler */
static void
KeyUp(AG_Event *_Nonnull event)
{
/*	MyWidget *my = MYWIDGET_SELF(); */
/*	int keysym = AG_INT(1); */

	/* ... */
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void
Init(void *_Nonnull obj)
{
	MyWidget *my = obj;

	/*
	 * May grab focus. Receive mousemotion events even if unfocused.
	 * Use the font engine (AG_TextRender() will be needed).
	 * Generate mouseover events.
	 */
	AGWIDGET(my)->flags |= (AG_WIDGET_FOCUSABLE | AG_WIDGET_UNFOCUSED_MOTION |
	                        AG_WIDGET_USE_TEXT | AG_WIDGET_USE_MOUSEOVER);

	/* Initialize instance variables. */
	my->foo = "";
	my->x = 0;
	my->y = 0;
	my->radius = 5;
	my->lastKey = 'X';

	/*
	 * We'll eventually need to create and map a surface, but we cannot
	 * do this from Init(), because it involves texture operations in
	 * GL mode which are thread-unsafe. We wait until Draw() to do that.
	 */
	my->label = -1;

	AG_InitTimer(&my->clickTimer, "clickTimer", 0);

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

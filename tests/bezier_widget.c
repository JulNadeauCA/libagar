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
#include "bezier_widget.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

/*
 * This is a generic constructor function. It is completely optional, but
 * customary of FooNew() functions to allocate, initialize and attach an
 * instance of the class.
 */
BezierWidget *
BezierWidgetNew(void *parent, const char *foo)
{
	BezierWidget *my;

	/* Create a new instance of the BezierWidget class */
	my = malloc(sizeof(BezierWidget));
	AG_ObjectInit(my, &bezierWidgetClass);

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
	BezierWidget *my = p;

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
	BezierWidget *my = p;

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
	BezierWidget *my = p;
	AG_Color c;
	AG_Rect r;

	/* Draw a raised 3D box over the widget area; use background-color. */
	r.x = 0;
	r.y = 0;
	r.w = AGWIDGET(my)->w;
	r.h = AGWIDGET(my)->h;
	AG_DrawBoxRaised(my, &r, &AG_WCOLOR(my, AG_BG_COLOR));

	if (my->label == -1) {
		AG_TextFontPct(500);
		my->label = AG_WidgetMapSurface(my,
		    AG_TextRenderF("%s", "M_Bezier2 test"));
	}

	AG_ColorRGB_8(&c, 250,250,0);

	/* Establish a clipping rectangle over the entire widget area. */
	AG_PushClipRect(my, &r);

	M_DrawBezier2(AGWIDGET(my),
			100, 20,
			120, 40,
			140, 25,
			160, 20,
			20, &c);

	M_DrawBezier2(AGWIDGET(my),
			200, 200,
			120, 40,
			140, 300,
			160, 20,
			20, &c);

	M_DrawBezier2(AGWIDGET(my),
			100, 200,
			120, 200,
			40, 300,
			160, 20,
			20, &c);

	M_DrawBezier2(AGWIDGET(my),
			30, 10,
			43, 34,
			40, 134,
			160, 212,
			5, &c);

	AG_PopClipRect(my);
}


/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void
Init(void *_Nonnull obj)
{
	BezierWidget *my = obj;

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

}

/*
 * This structure describes our widget class. It inherits from AG_ObjectClass.
 * Any of the function members may be NULL. See AG_Widget(3) for details.
 */
AG_WidgetClass bezierWidgetClass = {
	{
		"AG_Widget:BezierWidget",	/* Name of class */
		sizeof(BezierWidget),	/* Size of structure */
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

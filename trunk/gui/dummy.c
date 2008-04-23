/*	Public domain	*/

/*
 * This file demonstrates the implementation of a minimal Agar widget
 * that uses surface mappings in a way that allows for efficient use of
 * texture hardware (if we are using the GL display mode).
 */

#include <core/core.h>

#include "dummy.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

/*
 * This is a generic constructor function. It is completely optional, but
 * customary of FooNew() functions to allocate, initialize and attach an
 * instance of the class.
 */
AG_Dummy *
AG_DummyNew(void *parent, Uint flags, const char *caption)
{
	AG_Dummy *dum;

	/* Create a new instance of the AG_Dummy class */
	dum = Malloc(sizeof(AG_Dummy));
	AG_ObjectInit(dum, &agDummyClass);

	/* Set some constructor arguments */
	dum->flags |= flags;
	dum->caption = caption;

	/* It is customary for widgets to provide HFILL and VFILL flags. */
	if (flags & AG_DUMMY_HFILL) { AG_ExpandHoriz(dum); }
	if (flags & AG_DUMMY_VFILL) { AG_ExpandVert(dum); }

	/* Attach the object to the parent (if parent==NULL, this is a noop) */
	AG_ObjectAttach(parent, dum);

	return (dum);
}

/*
 * This function requests a minimal geometry for displaying the widget.
 * It is expected to return the width and height in pixels into r.
 */
static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Dummy *dum = p;

	if (dum->mySurface == -1) {
		/*
		 * We can use AG_TextSize() to return the dimensions of rendered
		 * text, without rendering it.
		 */
		AG_TextSize("Some text", &r->w, &r->h);
	} else {
		/*
		 * We can use the geometry of the rendered surface. The
		 * WSURFACE() macro returns the AG_Surface given a Widget
		 * surface handle.
		 */
		r->w = WSURFACE(dum,dum->mySurface)->w;
		r->h = WSURFACE(dum,dum->mySurface)->h;
	}
}

/*
 * This function is called by the parent widget after it decided how much
 * space to allocate to this widget. It is mostly useful to container
 * widgets, but other widgets generally use it to check if the allocated
 * geometry can be handled by Draw().
 */
static int
SizeAllocate(void *p, AG_SizeAlloc *r)
{
	AG_Dummy *dum = p;

	/* If we return -1, Draw() will not be called. */
	if (r->w < 5 || r->h < 5)
		return (-1);

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
	AG_Dummy *dum = p;
	
	/*
	 * Draw a box spanning the widget area. In order to allow themeing,
	 * you would generally use a STYLE() call here instead, see AG_Style(3)
	 * for more information on styles.
	 */
	AG_DrawBox(dum,
	    AG_RECT(0, 0, AGWIDGET(dum)->w, AGWIDGET(dum)->h), 1,
	    AG_COLOR(BUTTON_COLOR));

	/*
	 * Render "Foo" text into a new surface. In OpenGL mode, this involves
	 * a texture upload, so it is critical that static surfaces are mapped
	 * only when necessary.
	 */
	if (dum->mySurface == -1) {
		AG_Surface *mySurface;

		mySurface = AG_TextRender("Foo");
		dum->mySurface = AG_WidgetMapSurface(dum, mySurface);
	}

	/* Blit the mapped surface at [0,0]. */
	AG_WidgetBlitSurface(dum, dum->mySurface, 0, 0);
}

/* Mouse motion event handler */
static void
MouseMotion(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	/* ... */
}

/* Mouse click event handler */
static void
MouseButtonDown(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int button = AG_INT(1);
	
	if (button != SDL_BUTTON_LEFT) {
		return;
	}
	AG_WidgetFocus(dum);
}

/* Mouse click event handler */
static void
MouseButtonUp(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	/* ... */
}

/* Keystroke event handler */
static void
KeyDown(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

/* Keystroke event handler */
static void
KeyUp(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void
Init(void *obj)
{
	AG_Dummy *dum = obj;

	/* Allow this widget to grab focus. */
	AGWIDGET(dum)->flags |= AG_WIDGET_FOCUSABLE;

	/* Initialize the parent widget structure. */
	dum->flags = 0;

	/*
	 * We need to map a surface, but we cannot do this from this
	 * function, because it involves texture operations and would
	 * break thread safety in OpenGL mode. Surface operations are
	 * only safe from the draw() and scale() routines.
	 */
	dum->mySurface = -1;

	/*
	 * Map our event handlers. For a list of all meaningful events
	 * we can handle, see AG_Object(3), AG_Widget(3) and AG_Window(3).
	 *
	 * Here we register handlers for the common AG_Window(3) events.
	 */
	AG_SetEvent(dum, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(dum, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(dum, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(dum, "window-keyup", KeyUp, NULL);
	AG_SetEvent(dum, "window-keydown", KeyDown, NULL);
}

/*
 * Destructor routine. Note that the object system will automatically
 * invoke the destructors of the parent classes afterwards.
 */
static void
Destroy(void *obj)
{
	AG_Dummy *dum = obj;
}

/*
 * Load routine. For persistent widgets, this is typically used when
 * reading data files generated by the GUI builder.
 *
 * The object system will automatically invoke the load routines of
 * the parent beforehand.
 */
static int
Load(void *obj, AG_DataSource *ds, const AG_Version *ver)
{
	return (0);
}

/*
 * Save routine. For persistent widgets, this is typically used by
 * the GUI builder.
 *
 * The object system will automatically invoke the save routines of
 * the parent beforehand.
 */
static int
Save(void *obj, AG_DataSource *ds)
{
	return (0);
}

/*
 * This structure describes our widget class. It inherits from AG_ObjectClass.
 * Any of the function members may be NULL.
 */
AG_WidgetClass agDummyClass = {
	{
		"AG_Widget:AG_Dummy",	/* Name of class */
		sizeof(AG_Dummy),	/* Size of structure */
		{ 0,0 },		/* Version for load/save */
		Init,			/* Initialize dataset */
		NULL,			/* Free dataset */
		Destroy,		/* Destroy dataset */
		Load,			/* Load dataset */
		Save,			/* Save dataset */
		NULL			/* Generic edit operation */
	},
	Draw,				/* Render widget */
	SizeRequest,			/* Default size requisition */
	SizeAllocate			/* Size allocation callback */
};

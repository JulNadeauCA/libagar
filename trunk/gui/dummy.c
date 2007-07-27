/*	Public domain	*/

/*
 * This file demonstrates the implementation of a minimal Agar widget
 * that uses surface mappings.
 */

#include <core/core.h>
#include <core/view.h>

#include "dummy.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

#include <stdarg.h>

/*
 * This is the structure passed to AG_WidgetInit, which contains information
 * about this widget class, and its function mappings.
 */
const AG_WidgetOps agDummyOps = {
	{
		"AG_Widget:AG_Dummy",		/* Name of class */
		sizeof(AG_Dummy),		/* Size of structure */
		{ 0,0 },			/* Version */
		NULL,				/* init() */
		NULL,				/* reinit() */
		NULL,				/* destroy() */
		NULL,				/* load() */
		NULL,				/* save() */
		NULL				/* edit() */
	},
	AG_DummyDraw,		/* Draw function */
	AG_DummyScale		/* Scale/resize function */
};

/*
 * This is a generic constructor function. It is customary of FooNew()
 * functions in agar to allocate, initialize and attach an object. When
 * useful, is also customary to provide multiple alternative constructor
 * functions.
 */
AG_Dummy *
AG_DummyNew(void *parent, Uint flags, const char *caption)
{
	AG_Dummy *dum;

	dum = Malloc(sizeof(AG_Dummy), M_OBJECT);
	AG_DummyInit(dum, flags, caption);
	AG_ObjectAttach(parent, dum);
	return (dum);
}

/*
 * Scale routine. Invoked with w = -1 and h = -1 for initial sizing,
 * and with the actual geometry in pixels for resizing.
 */
void
AG_DummyScale(void *p, int w, int h)
{
	AG_Dummy *dum = p;
	SDL_Surface *label = AGWIDGET_SURFACE(dum,0);

	if (w == -1 && h == -1) {
		AGWIDGET(dum)->w = 16;
		AGWIDGET(dum)->h = 16;
	}
}

/*
 * Draw function. Invoked from GUI rendering context to draw the widget
 * at its current location. All primitive and surface operations operate
 * on widget coordinates.
 */
void
AG_DummyDraw(void *p)
{
	AG_Dummy *dum = p;
	
	if (AGWIDGET(dum)->w < 1 || AGWIDGET(dum)->h < 1)
		return;

	/* Draw a box spanning the widget area. */
	agPrim.box(dum,
	    0, 0,
	    AGWIDGET(dum)->w, AGWIDGET(dum)->h,
	    1,
	    AG_COLOR(BUTTON_COLOR));

	/*
	 * Render "Foo" text into a new surface. In OpenGL mode, this involves
	 * a texture upload, so it is critical that static surfaces are mapped
	 * only when necessary.
	 */
	if (dum->mySurface == -1) {
		SDL_Surface *mySurface;

		mySurface = AG_TextRender("Foo");
		dum->mySurface = AG_WidgetMapSurface(dum, mySurface);
	}

	/* Blit the mapped surface at [0,0]. */
	AG_WidgetBlitSurface(dum, dum->mySurface, 0, 0);
}

/* Mouse motion event handler */
static void
mousemotion(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	/* ... */
}

/* Mouse click event handler */
static void
mousebuttondown(AG_Event *event)
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
mousebuttonup(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	/* ... */
}

/* Keystroke event handler */
static void
keydown(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

/* Keystroke event handler */
static void
keyup(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

/* Initialization routine. */
void
AG_DummyInit(AG_Dummy *dum, Uint flags, const char *caption)
{
	Uint wFlags = AG_WIDGET_FOCUSABLE;

	/* It is customary for widgets to provide HFILL and VFILL flags. */
	if (flags & AG_DUMMY_HFILL) { wFlags |= AG_WIDGET_HFILL; }
	if (flags & AG_DUMMY_VFILL) { wFlags |= AG_WIDGET_VFILL; }

	/* Initialize the parent widget structure. */
	AG_WidgetInit(dum, &agDummyOps, wFlags);
	dum->flags = flags;

	/*
	 * We need to map a surface, but we cannot do this from this
	 * function, because it involves texture operations and would
	 * break thread safety in OpenGL mode. Surface operations are
	 * only safe from the draw() and scale() routines.
	 */
	dum->mySurface = -1;

	/* Map our event handlers. */
	AG_SetEvent(dum, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(dum, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(dum, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(dum, "window-keyup", keyup, NULL);
	AG_SetEvent(dum, "window-keydown", keydown, NULL);
}


/*	Public domain	*/

/*
 * This file demonstrates the implementation of a minimal Agar widget
 * that uses surface mappings.
 */

#include <core/core.h>

#include "dummy.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

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
		 * WSURFACE() macro returns the SDL_Surface given a
		 * Widget surface handle.
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
	
	/* Draw a box spanning the widget area. */
	AG_DrawBox(dum,
	    AG_RECT(0, 0, WIDGET(dum)->w, WIDGET(dum)->h), 1,
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
		NULL,				/* free_dataset() */
		NULL,				/* destroy() */
		NULL,				/* load() */
		NULL,				/* save() */
		NULL				/* edit() */
	},
	Draw,			/* Rendering function */
	SizeRequest,		/* Minimal size request */
	SizeAllocate		/* Allocated size callback */
};

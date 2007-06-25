/*	Public domain	*/

#include <core/core.h>
#include <core/view.h>

#include "button.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

#include <stdarg.h>

const AG_WidgetOps agDummyOps = {
	{
		"AG_Widget:AG_Dummy",
		sizeof(AG_Dummy),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_DummyDraw,
	AG_DummyScale
};

static void mousemotion(AG_Event *);
static void mousebuttonup(AG_Event *);
static void mousebuttondown(AG_Event *);
static void keyup(AG_Event *);
static void keydown(AG_Event *);

AG_Dummy *
AG_DummyNew(void *parent, Uint flags, const char *caption)
{
	AG_Dummy *dum;

	dum = Malloc(sizeof(AG_Dummy), M_OBJECT);
	AG_DummyInit(dum, flags, caption);
	AG_ObjectAttach(parent, dum);
	return (dum);
}

void
AG_DummyInit(AG_Dummy *dum, Uint flags, const char *caption)
{
	Uint wFlags = AG_WIDGET_FOCUSABLE;

	if (flags & AG_DUMMY_HFILL) { wFlags |= AG_WIDGET_HFILL; }
	if (flags & AG_DUMMY_VFILL) { wFlags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(dum, &agDummyOps, wFlags);
	dum->flags = flags;

	AG_SetEvent(dum, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(dum, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(dum, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(dum, "window-keyup", keyup, NULL);
	AG_SetEvent(dum, "window-keydown", keydown, NULL);
}

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

void
AG_DummyDraw(void *p)
{
	AG_Dummy *dum = p;
	
	if (AGWIDGET(dum)->w < 1 || AGWIDGET(dum)->h < 1)
		return;

	agPrim.box_dithered(dum,
	    0, 0,
	    AGWIDGET(dum)->w, AGWIDGET(dum)->h,
	    pressed ? -1 : 1,
	    AG_COLOR(BUTTON_COLOR),
	    AG_COLOR(DISABLED_COLOR));
}

static void
mousemotion(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	/* ... */
}

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

static void
mousebuttonup(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	/* ... */
}

static void
keydown(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

static void
keyup(AG_Event *event)
{
	AG_Dummy *dum = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}


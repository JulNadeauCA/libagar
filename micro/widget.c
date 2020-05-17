/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * MA_Widget: The base class for all micro-Agar widgets.
 */

#include <agar/core/core.h>

#include <agar/micro/gui.h>
#include <agar/micro/widget.h>
#include <agar/micro/window.h>
#include <agar/micro/primitive.h>

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/*
 * Test whether a widget (and its parent window) both hold focus.
 * The Widget and agDrivers VFS must be locked.
 */
Uint8
MA_WidgetIsFocused(const void *obj)
{
	MA_Widget *wid = WIDGET(obj);

	return ((wid->flags & MA_WIDGET_FOCUSED) &&
                (wid->window == NULL || maWindowFocused == wid->window));
}

/*
 * Test whether the point at display coordinates x,y overlaps a widget's
 * allocated area (inclusive).
 */
Uint8
MA_WidgetArea(const void *obj, Sint16 x, Sint16 y)
{
	const MA_Widget *wid = WIDGET(obj);
	MA_Widget *widParent = WIDGET(obj);
	Uint16 xWid=0, yWid=0;

	do {
		xWid += widParent->x;
		yWid += widParent->y;
	} while ((widParent = OBJECT(wid)->parent) != NULL);

	return (x >= xWid &&
	        y >= yWid &&
	        x <= xWid + wid->w &&
	        y <= yWid + wid->h);
}

/*
 * Draw an unmapped surface at given coordinates in the widget's coordinate
 * system. With hardware-accelerated drivers, this operation is slow compared
 * to drawing of mapped surfaces, since a software->hardware copy is done.
 */
void
MA_WidgetBlit(void *obj, MA_Surface *S, Uint8 x, Uint8 y)
{
	maDriverOps->blitSurface(WIDGET(obj), S, x,y);
}

void
MA_WidgetBlitFrom(void *obj, Uint8 s, MA_Rect *r, Uint8 x, Uint8 y)
{
	MA_Widget *wid = WIDGET(obj);
	
	if (s == 0 || wid->surfaces[s] == NULL)
		return;

	maDriverOps->blitSurfaceFrom(wid, s, r, x,y);
}

static void FocusWidget(MA_Widget *_Nonnull);
static void UnfocusWidget(MA_Widget *_Nonnull);

/*
 * Set the parent window back pointer on a widget and its children
 * recursively.
 */
static void
SetParentWindow(MA_Widget *_Nonnull wid, MA_Window *_Nullable win)
{
	MA_Widget *chld;
	
	wid->window = win;

	OBJECT_FOREACH_CHILD(chld, wid, ma_widget)
		SetParentWindow(chld, win);
}

static void
OnAttach(AG_Event *_Nonnull event)
{
	MA_Widget *widget = MA_WIDGET_SELF();
	const void *parent = AG_PTR(1);

	if (AG_OfClass(parent, "MA_Window:*") &&
	    AG_OfClass(widget, "MA_Widget:*")) {
		MA_Widget *wParent = WIDGET(parent);
		/*
		 * This is a widget attaching to a window.
		 */
		SetParentWindow(widget, MAWINDOW(wParent));
		if (MAWINDOW(wParent)->visible) {
/*			widget->flags |= MA_WIDGET_UPDATE_WINDOW; */
			AG_PostEvent(widget, "widget-shown", NULL);
		}
	} else if (AG_OfClass(parent, "MA_Widget:*") &&
	           AG_OfClass(widget, "MA_Widget:*")) {
		MA_Widget *wParent = WIDGET(parent);
		MA_Window *window = wParent->window;
		/*
		 * This is a widget attaching to another widget (not a window).
		 */
		SetParentWindow(widget, window);
		if (window && window->visible)
			AG_PostEvent(widget, "widget-shown", NULL);
	}
}

static void
OnDetach(AG_Event *_Nonnull event)
{
	MA_Widget *widget = MA_WIDGET_SELF();
	const void *parent = AG_PTR(1);
	
	if (AG_OfClass(parent, "MA_Widget:*") &&
	    AG_OfClass(widget, "MA_Widget:*"))
		SetParentWindow(widget, NULL);
}

/* "widget-shown" handler */
static void
OnShow(AG_Event *_Nonnull event)
{
	MA_Widget *wid = MA_WIDGET_SELF();
	MA_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ma_widget) {
		AG_ForwardEvent(chld, event);
	}
	wid->flags |= MA_WIDGET_VISIBLE;
}

/* "widget-hidden" handler */
static void
OnHide(AG_Event *_Nonnull event)
{
	MA_Widget *wid = MA_WIDGET_SELF();
	MA_Widget *chld;
#ifdef AG_TIMERS
	AG_RedrawTie *rt;
#endif
	OBJECT_FOREACH_CHILD(chld, wid, ma_widget) {
		AG_ForwardEvent(chld, event);
	}
	wid->flags &= ~(MA_WIDGET_VISIBLE);
}

static void
Init(void *_Nonnull obj)
{
	MA_Widget *wid = obj;

	OBJECT(wid)->flags |= AG_OBJECT_NAME_ONATTACH;

	wid->flags = 0;
	wid->x = 0;
	wid->y = 0;
	wid->w = 0;
	wid->h = 0;
	wid->nSurfaces = 0;
	wid->surfaces = NULL;
	wid->window = NULL;
	
	AG_SetEvent(wid, "attached", OnAttach, NULL);
	AG_SetEvent(wid, "detached", OnDetach, NULL);
	AG_SetEvent(wid, "widget-shown", OnShow, NULL);
	AG_SetEvent(wid, "widget-hidden", OnHide, NULL);
}

/* Set widget to "enabled" state for input. */
void
MA_WidgetEnable(void *obj)
{
	MA_Widget *wid = obj;

	if (wid->flags & MA_WIDGET_DISABLED) {
		wid->flags &= ~(MA_WIDGET_DISABLED);
		AG_PostEvent(wid, "widget-enabled", NULL);
/*		MA_Redraw(wid); */
	}
}

/* Set widget to "disabled" state for input. */
void
MA_WidgetDisable(void *obj)
{
	MA_Widget *wid = obj;

	if (!(wid->flags & MA_WIDGET_DISABLED)) {
		wid->flags |= MA_WIDGET_DISABLED;
		AG_PostEvent(wid, "widget-disabled", NULL);
/*		MA_Redraw(wid); */
	}
}

static void
Destroy(void *_Nonnull obj)
{
	MA_Widget *wid = obj;
	Uint8 i;

	/*
	 * Free surfaces. We can assume that drivers have already deleted
	 * any associated resources.
	 */
	for (i = 0; i < wid->nSurfaces; i++) {
		MA_Surface *S;

		if ((S = wid->surfaces[i]) != NULL) {
			S->flags &= ~(MA_SURFACE_MAPPED);
			MA_SurfaceFree(S);
		}
	}
	Free(wid->surfaces);
}

/* Remove focus from a widget and its children. */
void
MA_WidgetUnfocus(void *p)
{
	MA_Widget *wid = p, *cwid;
	
	if (wid->flags & MA_WIDGET_FOCUSED) {
		UnfocusWidget(wid);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ma_widget)
		MA_WidgetUnfocus(cwid);
}

static void
UnfocusWidget(MA_Widget *_Nonnull wid)
{
	MA_Window *win;

	wid->flags &= ~(MA_WIDGET_FOCUSED);
	if ((win = wid->window)) {
		AG_PostEvent(wid, "widget-lostfocus", NULL);
/*		win->nFocused--; */
		win->flags |= MA_WINDOW_REDRAW;
	}
}

/* Move the focus over a widget (and its parents). */
int
MA_WidgetFocus(void *obj)
{
	MA_Widget *wid = obj, *wParent = wid;
	MA_Window *win = wid->window;
	
	if (MA_WidgetIsFocused(wid)) {
		return (1);
	}
	if (!(wid->flags & MA_WIDGET_FOCUSABLE))
		return (0);

	/* Remove any existing focus. XXX inefficient */
	if (win)
		MA_WidgetUnfocus(win);

	/*
	 * Set the focus flag on the widget and its parents, up
	 * to the parent window.
	 */
	do {
		if (AG_OfClass(wParent, "MA_Window:*")) {
			MA_WindowFocus(MAWINDOW(wParent));
			break;
		}
		if ((wParent->flags & MA_WIDGET_FOCUSED) == 0) {
			FocusWidget(wParent);
		}
	} while ((wParent = OBJECT(wParent)->parent) != NULL);

	return (1);
}

/* Acquire widget focus */
static void
FocusWidget(MA_Widget *_Nonnull wid)
{
	MA_Window *win;

	wid->flags |= MA_WIDGET_FOCUSED;
	if ((win = wid->window) != NULL) {
		AG_PostEvent(wid, "widget-gainfocus", NULL);
/*		win->nFocused++; */
		win->flags |= MA_WINDOW_REDRAW;
	}
}

static void
SizeRequest(void *_Nonnull p, MA_SizeReq *_Nonnull r)
{
	(void) p;
	(void) r;
	r->w = 0;
	r->h = 0;
}

static Sint8
SizeAllocate(void *_Nonnull p, const MA_SizeAlloc *_Nonnull a)
{
	(void) p;
	(void) a;
	return (0);
}

void
MA_WidgetSizeReq(void *obj, MA_SizeReq *r)
{
	MA_Widget *wid = obj;

	r->w = 0;
	r->h = 0;

	if (WIDGET_OPS(wid)->size_request != NULL)
		WIDGET_OPS(wid)->size_request(wid, r);
}

void
MA_WidgetSizeAlloc(void *obj, MA_SizeAlloc *a)
{
	MA_Widget *wid = obj;

	if (a->w <= 0 || a->h <= 0) {
		a->w = 0;
		a->h = 0;
		wid->flags |= MA_WIDGET_UNDERSIZE;
	} else {
		wid->flags &= ~(MA_WIDGET_UNDERSIZE);
	}
	wid->x = a->x;
	wid->y = a->y;
	wid->w = a->w;
	wid->h = a->h;

	if (WIDGET_OPS(wid)->size_allocate != NULL) {
		if (WIDGET_OPS(wid)->size_allocate(wid, a) == -1) {
			wid->flags |= MA_WIDGET_UNDERSIZE;
		} else {
			wid->flags &= ~(MA_WIDGET_UNDERSIZE);
		}
	}
}

/*
 * Search for a focused widget inside a window. Return value is only valid
 * as long as the Driver VFS is locked.
 */
MA_Widget *
MA_WidgetFindFocused(void *p)
{
	MA_Widget *wid = p;
	MA_Widget *cwid, *fwid;

	if (!AG_OfClass(wid, "MA_Window:*")) {
		if ((wid->flags & MA_WIDGET_FOCUSED) == 0 ||
		    (wid->flags & MA_WIDGET_VISIBLE) == 0 ||
		    (wid->flags & MA_WIDGET_DISABLED))
			return (0);
	}
	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, ma_widget) {
		if ((fwid = MA_WidgetFindFocused(cwid)) != NULL)
			return (fwid);
	}
	return (wid);
}

/* Show a widget */
void
MA_WidgetShow(void *obj)
{
	MA_Widget *wid = obj;

	wid->flags |= MA_WIDGET_VISIBLE;
	AG_PostEvent(wid, "widget-shown", NULL);
	if (wid->window)
		MA_WindowUpdate(wid->window);
}

/* Hide a widget */
void
MA_WidgetHide(void *obj)
{
	MA_Widget *wid = obj;

	wid->flags &= ~(MA_WIDGET_VISIBLE);
	AG_PostEvent(wid, "widget-hidden", NULL);
	if (wid->window)
		MA_WindowUpdate(wid->window);
}

/* Make a widget and all of its children visible. */
void
MA_WidgetShowAll(void *p)
{
	MA_Widget *wid = p;
	MA_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ma_widget)
		MA_WidgetShowAll(chld);

	AG_PostEvent(wid, "widget-shown", NULL);
}

/* Make a widget and all of its children invisible. */
void
MA_WidgetHideAll(void *p)
{
	MA_Widget *wid = p;
	MA_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ma_widget)
		MA_WidgetHideAll(chld);

	AG_PostEvent(wid, "widget-hidden", NULL);
}

/* Generic inherited draw() routine. */
void
MA_WidgetInheritDraw(void *obj)
{
	WIDGET_SUPER_OPS(obj)->draw(obj);
}

/* Generic inherited size_request() routine. */
void
MA_WidgetInheritSizeRequest(void *obj, MA_SizeReq *r)
{
	WIDGET_SUPER_OPS(obj)->size_request(obj, r);
}

/* Generic inherited size_allocate() routine. */
int
MA_WidgetInheritSizeAllocate(void *obj, const MA_SizeAlloc *a)
{
	return WIDGET_SUPER_OPS(obj)->size_allocate(obj, a);
}

/*
 * Attach a surface to a Widget and return an integer surface handle.
 *
 * The returned handle is unique to the Widget, and is also an index into its
 * surfaces[] array.
 *
 * The surface will be freed automatically when the widget is destroyed.
 */
Uint8
MA_WidgetMapSurface(void *obj, MA_Surface *S)
{
	MA_Widget *wid = obj;
	Uint8 i, n, s=0;

	n = wid->nSurfaces;
	for (i = 0; i < n; i++) {
		if (wid->surfaces[i] == NULL) {
			s = i;
			break;
		}
	}
	if (i == n) {
		++n;
		wid->surfaces = Realloc(wid->surfaces, n*sizeof(MA_Surface *));
		s = wid->nSurfaces++;
	}
	wid->surfaces[s] = S;

	if (S) {
		S->flags |= MA_SURFACE_MAPPED;
	}
	return (s);
}

/*
 * Replace the contents of a mapped surface. Passing S => NULL is equivalent
 * to calling MA_WidgetUnmapSurface().
 */
void
MA_WidgetReplaceSurface(void *obj, Uint8 s, MA_Surface *S)
{
	MA_Widget *wid = obj;
	MA_Surface *Sprev;

#ifdef AG_DEBUG
	if (s >= wid->nSurfaces)
		AG_FatalError("No such surface");
#endif
	if ((Sprev = wid->surfaces[s]) != NULL) {
		MA_SurfaceFree(Sprev);
	}
	if (S) {
		S->flags |= MA_SURFACE_MAPPED;
	}
	wid->surfaces[s] = S;
}

MA_WidgetClass maWidgetClass = {
	{
		"MA_Widget",
		sizeof(MA_Widget),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	SizeRequest,
	SizeAllocate
};

/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Generic interface to mice. Most graphics drivers will register a single
 * mouse object, but multiple instances are supported.
 */

#include <core/core.h>
#include <core/config.h>

#include "window.h"
#include "cursors.h"

AG_Mouse *
AG_MouseNew(void *drv, const char *desc)
{
	AG_Mouse *ms;
	
	AG_ASSERT_CLASS(drv, "AG_Driver:*");
	
	if ((ms = TryMalloc(sizeof(AG_Mouse))) == NULL) {
		return (NULL);
	}
	AG_ObjectInit(ms, &agMouseClass);
	AGINPUTDEV(ms)->drv = drv;
	if ((AGINPUTDEV(ms)->desc = TryStrdup(desc)) == NULL) {
		goto fail;
	}
	AGDRIVER(drv)->mouse = ms;
	AG_ObjectAttach(&agInputDevices, ms);
	return (ms);
fail:
	AG_ObjectDestroy(ms);
	return (NULL);
}

static void
Init(void *obj)
{
	AG_Mouse *ms = obj;

	OBJECT(ms)->flags |= AG_OBJECT_NAME_ONATTACH;
	ms->nButtons = 0;
	ms->btnState = 0;
	ms->x = 0;
	ms->y = 0;
	ms->xRel = 0;
	ms->yRel = 0;
}

/* Update the mouse state following a cursor motion event. */
void
AG_MouseMotionUpdate(AG_Mouse *ms, int x, int y)
{
	ms->xRel = x - ms->x;
	ms->yRel = y - ms->y;

	if (ms->xRel != 0 || ms->yRel != 0) {
		ms->x = x;
		ms->y = y;
	}
}

/*
 * Change the cursor if overlapping a registered cursor area.
 * Generally called in response to mousemotion events.
 */
void
AG_MouseCursorUpdate(AG_Window *win, int x, int y)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_CursorArea *ca;
	AG_Rect r;

	TAILQ_FOREACH(ca, &win->cursorAreas, cursorAreas) {
		if (ca->wid->flags & AG_WIDGET_HIDE) {
			continue;
		}
		r.x = ca->r.x + ca->wid->rView.x1;
		r.y = ca->r.y + ca->wid->rView.y1;
		r.w = ca->r.w;
		r.h = ca->r.h;
		if (AG_RectInside(&r, x,y))
			break;
	}
	if (ca == NULL) {
		if (drv->activeCursor != &drv->cursors[0])
			AGDRIVER_CLASS(drv)->unsetCursor(drv);
	} else if (ca->c != drv->activeCursor) {
		AGDRIVER_CLASS(drv)->setCursor(drv, ca->c);
	}
}

/* Update the mouse state following a button press/release event. */
void
AG_MouseButtonUpdate(AG_Mouse *ms, AG_MouseButtonAction a, int which)
{
	if (a == AG_BUTTON_PRESSED) {
		ms->btnState |= AG_MOUSE_BUTTON(which);
	} else {
		ms->btnState &= ~(AG_MOUSE_BUTTON(which));
	}
}

/* Recursively deliver a mouse-motion event to widgets. */
static void
PostMouseMotion(AG_Window *win, AG_Widget *wid, int x, int y, int xRel,
    int yRel, Uint state)
{
	AG_Widget *chld;

	AG_ObjectLock(wid);
	if ((AG_WidgetIsFocusedInWindow(wid)) ||
	    (wid->flags & AG_WIDGET_UNFOCUSED_MOTION)) {
		AG_PostEvent(NULL, wid, "mouse-motion",
		    "%i(x),%i(y),%i(xRel),%i(yRel),%i(buttons)",
		    x - wid->rView.x1,
		    y - wid->rView.y1,
		    xRel,
		    yRel,
		    (int)state);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-mousemotion",
		    "%i,%i,%i,%i,%i",
		    x - wid->rView.x1,
		    y - wid->rView.y1,
		    xRel,
		    yRel,
		    (int)state);
#endif
		if (wid == win->widExclMotion)
			goto out;			/* Skip other widgets */
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		PostMouseMotion(win, chld, x, y, xRel, yRel, state);
out:
	AG_ObjectUnlock(wid);
}

/*
 * Recursively deliver a mouse-button-up event to widgets that either hold
 * focus or have the UNFOCUSED_BUTTONUP flag set.
 */
static void
PostMouseButtonUp(AG_Window *win, AG_Widget *wid, int x, int y,
    AG_MouseButton button)
{
	AG_Widget *chld;

	AG_ObjectLock(wid);
	if ((AG_WidgetIsFocusedInWindow(wid)) ||
	    (wid->flags & AG_WIDGET_UNFOCUSED_BUTTONUP)) {
		AG_PostEvent(NULL, wid, "mouse-button-up",
		    "%i(button),%i(x),%i(y)",
		    (int)button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-mousebuttonup",
		    "%i,%i,%i",
		    (int)button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#endif
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		PostMouseButtonUp(win, chld, x, y, button);
	}
	AG_ObjectUnlock(wid);
}

/*
 * Recursively deliver a mouse-button-down event to widgets based on
 * widget sensitivity rectangles.
 */
static int
PostMouseButtonDown(AG_Window *win, AG_Widget *wid, int x, int y,
    AG_MouseButton button)
{
	AG_Widget *chld;
	AG_Event *ev;
	
	AG_ObjectLock(wid);

	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		if (PostMouseButtonDown(win, chld, x, y, button))
			goto match;
	}
	if (!AG_WidgetSensitive(wid, x, y)) {
		goto out;
	}
	TAILQ_FOREACH(ev, &OBJECT(wid)->events, events) {
		if (strcmp(ev->name, "mouse-button-down") == 0)
			break;
#ifdef AG_LEGACY
		if (strcmp(ev->name, "window-mousebuttondown") == 0)
			break;
#endif
	}
	if (ev != NULL) {
		AG_PostEvent(NULL, wid, "mouse-button-down",
		    "%i(button),%i(x),%i(y)",
		    (int)button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-mousebuttondown",
		    "%i,%i,%i",
		    (int)button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#endif
		goto match;
	}
out:
	AG_ObjectUnlock(wid);
	return (0);
match:
	AG_ObjectUnlock(wid);
	return (1);
}

/*
 * Process a mouse-motion event relative to the given window;
 * this is called from low-level event handling code.
 */
void
AG_ProcessMouseMotion(AG_Window *win, int x, int y, int xRel, int yRel,
    Uint state)
{
	AG_Widget *wid;
	
	/* Allow a widget (e.g., AG_Pane) to absorb all mousemotion events */
	if ((wid = win->widExclMotion) != NULL) {
		AG_ObjectLock(wid);
		PostMouseMotion(win, wid, x, y, xRel, yRel, state);
		AG_ObjectUnlock(wid);
		return;
	}

	/* Recursively post mouse-motion to all widgets. */
	WIDGET_FOREACH_CHILD(wid, win)
		PostMouseMotion(win, wid, x, y, xRel, yRel, state);
}

/*
 * Process a mouse-button event relative to the given window;
 * this is called from low-level event handling code.
 */
void
AG_ProcessMouseButtonUp(AG_Window *win, int x, int y, AG_MouseButton button)
{
	AG_Widget *wid;

	WIDGET_FOREACH_CHILD(wid, win)
		PostMouseButtonUp(win, wid, x, y, button);
}

/*
 * Process a mouse-button event relative to the given window;
 * this is called from low-level event handling code.
 */
void
AG_ProcessMouseButtonDown(AG_Window *win, int x, int y, AG_MouseButton button)
{
	AG_Widget *wid;

	WIDGET_FOREACH_CHILD(wid, win)
		PostMouseButtonDown(win, wid, x, y, button);
}

AG_ObjectClass agMouseClass = {
	"Agar(InputDevice:Mouse)",
	sizeof(AG_Mouse),
	{ 0,0 },
	Init,
	NULL,		/* reinit */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};

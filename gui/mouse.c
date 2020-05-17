/*
 * Copyright (c) 2009-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/gui/window.h>
#include <agar/gui/cursors.h>

static void PostMouseMotion(AG_Window *_Nonnull _Restrict,
    AG_Widget *_Nonnull _Restrict, int,int, int,int, Uint);
static void PostMouseButtonUp(AG_Window *_Nonnull _Restrict,
    AG_Widget *_Nonnull _Restrict, int,int, AG_MouseButton);
static int PostMouseButtonDown(AG_Window *_Nonnull _Restrict,
    AG_Widget *_Nonnull _Restrict, int,int, AG_MouseButton);

AG_Mouse *
AG_MouseNew(void *drv, const char *desc)
{
	AG_Mouse *ms;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	
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

Uint8
AG_MouseGetState(AG_Mouse *ms, int *x, int *y)
{
	if (x != NULL) { *x = ms->x; }
	if (y != NULL) { *y = ms->y; }
	return (ms->btnState);
}

static void
Init(void *_Nonnull obj)
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

	TAILQ_FOREACH(ca, &win->pvt.cursorAreas, cursorAreas) {
		if (!(ca->wid->flags & AG_WIDGET_VISIBLE)) {
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
		if (drv->activeCursor != TAILQ_FIRST(&drv->cursors)) {
			AGDRIVER_CLASS(drv)->unsetCursor(drv);
		}
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

/*
 * Process a `mouse-motion' event relative to the given window.
 * 
 * This is generally called from the event-handling section of low-level
 * driver code. The agDrivers VFS must be locked.
 */
void
AG_ProcessMouseMotion(AG_Window *win, int x, int y, int xRel, int yRel,
    Uint state)
{
	AG_Widget *wid;
	
	/*
	 * If needed, we give a particular widget exclusivity over all
	 * `mouse-motion' events. This is used notably by AG_Pane(3).
	 */
	if ((wid = win->widExclMotion) != NULL) {
		PostMouseMotion(win, wid, x,y, xRel,yRel, state);
		return;
	}

	OBJECT_FOREACH_CHILD(wid, win, ag_widget)
		PostMouseMotion(win, wid, x,y, xRel,yRel, state);
}

/*
 * Deliver a `mouse-motion' event to all active widgets which are
 * either focused or have UNFOCUSED_MOTION set.
 *
 * Also, deliver `mouse-over' event (and update MOUSEOVER flag) to
 * all widgets with USE_MOUSEOVER enabled.
 */
static void
PostMouseMotion(AG_Window *_Nonnull _Restrict win,
    AG_Widget *_Nonnull _Restrict wid, int x, int y, int xRel, int yRel,
    Uint state)
{
	AG_Widget *chld;
	Uint flags;

	AG_ObjectLock(wid);
	flags = wid->flags;
	if (flags & AG_WIDGET_VISIBLE) {
		if (flags & AG_WIDGET_USE_MOUSEOVER) {
			if (AG_WidgetArea(wid, x,y)) {
				if ((flags & AG_WIDGET_MOUSEOVER) == 0) {
					wid->flags |= AG_WIDGET_MOUSEOVER;
					AG_PostEvent(wid, "mouse-over", NULL);
					AG_Redraw(wid);
				}
			} else {
				if (flags & AG_WIDGET_MOUSEOVER) {
					wid->flags &= ~(AG_WIDGET_MOUSEOVER);
					AG_PostEvent(wid, "mouse-over", NULL);
					AG_Redraw(wid);
				}
			}
		}
		if ((flags & AG_WIDGET_FOCUSED) ||
		    (flags & AG_WIDGET_UNFOCUSED_MOTION)) {
			AG_PostEvent(wid, "mouse-motion",
			    "%i(x),%i(y),%i(xRel),%i(yRel),%i(buttons)",
			    x - wid->rView.x1,
			    y - wid->rView.y1,
			    xRel,
			    yRel,
			    (int)state);

			if (wid == win->widExclMotion)
				goto out;		/* Skip other widgets */
		}
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		PostMouseMotion(win, chld, x, y, xRel, yRel, state);
out:
	AG_ObjectUnlock(wid);
}

/*
 * Process a mouse-button event relative to the given window.
 * 
 * This is generally called from the event-handling section of low-level
 * driver code. The agDrivers VFS must be locked.
 */
void
AG_ProcessMouseButtonUp(AG_Window *win, int x, int y, AG_MouseButton button)
{
	AG_Widget *wid;

	OBJECT_FOREACH_CHILD(wid, win, ag_widget)
		PostMouseButtonUp(win, wid, x, y, button);
}

/*
 * Deliver a `mouse-button-up' event to all active widgets which are
 * either focused or have UNFOCUSED_BUTTONUP set. 
 */
static void
PostMouseButtonUp(AG_Window *_Nonnull _Restrict win,
    AG_Widget *_Nonnull _Restrict wid, int x, int y, AG_MouseButton button)
{
	AG_Widget *chld;
	Uint flags;

	AG_ObjectLock(wid);
	flags = wid->flags;
	if (flags & AG_WIDGET_VISIBLE) {
		if ((flags & AG_WIDGET_FOCUSED) ||
		    (flags & AG_WIDGET_UNFOCUSED_BUTTONUP)) {
			AG_PostEvent(wid, "mouse-button-up",
			    "%i(button),%i(x),%i(y)",
			    (int)button,
			    x - wid->rView.x1,
			    y - wid->rView.y1);
		}
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		PostMouseButtonUp(win, chld, x, y, button);
	}
	AG_ObjectUnlock(wid);
}

/*
 * Process a mouse-button event relative to the given window.
 * 
 * This is generally called from the event-handling section of low-level
 * driver code. The agDrivers VFS must be locked.
 */
void
AG_ProcessMouseButtonDown(AG_Window *win, int x, int y, AG_MouseButton button)
{
	AG_Driver *drv;
	AG_Window *winModal;
	AG_Widget *wid;
	
	if ((win->flags & AG_WINDOW_MODAL) == 0) {
		OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_OBJECT_ISA(drv, "AG_Driver:*");

			OBJECT_FOREACH_CHILD(winModal, drv, ag_window) {
				if (winModal == win || !winModal->visible ||
				   (winModal->flags & AG_WINDOW_MODAL) == 0) {
					continue;
				}
				AG_OBJECT_ISA(winModal, "AG_Widget:AG_Window:*");
				AG_PostEvent(winModal, "window-close", NULL);
			}
		}
	}

	OBJECT_FOREACH_CHILD(wid, win, ag_widget)
		PostMouseButtonDown(win, wid, x, y, button);
}

/*
 * Deliver a `mouse-button-down' event to the active widget at specified
 * window coordinates (if multiple widgets overlap, deliver to the topmost
 * widget which has a `mouse-button-down' handler defined).
 */
static int
PostMouseButtonDown(AG_Window *_Nonnull _Restrict win,
    AG_Widget *_Nonnull _Restrict wid, int x, int y, AG_MouseButton button)
{
	AG_Widget *chld;
	AG_Event *ev;
	
	AG_ObjectLock(wid);

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		if (PostMouseButtonDown(win, chld, x, y, button))
			goto match;
	}
	if ((wid->flags & AG_WIDGET_VISIBLE) == 0) {
		goto no_match;
	}
	if ((wid->flags & AG_WIDGET_UNFOCUSED_BUTTONDOWN) == 0) {
		if (!AG_WidgetSensitive(wid, x,y)) {
			goto no_match;
		}
	}
	TAILQ_FOREACH(ev, &OBJECT(wid)->events, events) {
		if (strcmp(ev->name, "mouse-button-down") == 0)
			break;
	}
	if (ev != NULL) {
		AG_PostEvent(wid, "mouse-button-down",
		    "%i(button),%i(x),%i(y)",
		    (int)button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
		goto match;
	}
no_match:
	AG_ObjectUnlock(wid);
	return (0);
match:
	AG_ObjectUnlock(wid);
	return (1);
}

AG_ObjectClass agMouseClass = {
	"Agar(InputDevice:Mouse)",
	sizeof(AG_Mouse),
	{ 0,0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};

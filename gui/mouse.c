/*
 * Copyright (c) 2009-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Interface to mice.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>
#include <agar/gui/cursors.h>
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/separator.h>
#endif

static void PostMouseMotion(AG_Window *_Nonnull, AG_Widget *_Nonnull, int,int, int,int);
static void PostMouseButtonUp(AG_Window *_Nonnull, AG_Widget *_Nonnull, int,int, AG_MouseButton);
static void PostMouseButtonDown(AG_Window *_Nonnull, AG_Widget *_Nonnull, int,int, AG_MouseButton);

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

/*
 * Process a Mouse Motion event.
 * 
 * Called from AG_EventLoop(3) or from a user-defined AG_CustomEventLoop(3).
 * The agDrivers VFS must be locked.
 */
void
AG_ProcessMouseMotion(AG_Window *win, int x, int y, int xRel, int yRel)
{
	AG_Widget *wid;
	
	/*
	 * If requested, give a particular widget exclusivity over all
	 * "mouse-motion" events. This is used notably by AG_Pane(3).
	 */
	if ((wid = win->widExclMotion) != NULL) {
		PostMouseMotion(win, wid, x,y, xRel,yRel);
		return;
	}

	OBJECT_FOREACH_CHILD(wid, win, ag_widget)
		PostMouseMotion(win, wid, x,y, xRel,yRel);
}

static void
PostMouseMotion(AG_Window *_Nonnull win, AG_Widget *_Nonnull wid, int x, int y,
    int xRel, int yRel)
{
	AG_Widget *chld;
	Uint flags;

	AG_ObjectLock(wid);
	flags = wid->flags;
	if ((flags & AG_WIDGET_VISIBLE) == 0) {
		/*
		 * If a widget is hidden, it is implied that its children are
		 * hidden as well. TODO: Lockless visibility determination.
		 */
		goto out;
	}
	if (flags & AG_WIDGET_USE_MOUSEOVER) {          /* Update MOUSEOVER */
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

		if (WIDGET_OPS(wid)->mouse_motion != NULL) {
			WIDGET_OPS(wid)->mouse_motion(wid,
			    x - wid->rView.x1,
		            y - wid->rView.y1,
			    xRel, yRel);
		}
		AG_PostEvent(wid, "mouse-motion",
		    "%i(x),%i(y),%i(xRel),%i(yRel),%i(buttons)",
		    x - wid->rView.x1,
		    y - wid->rView.y1,
		    xRel, yRel,
		    (int)wid->drv->mouse->btnState);

		if (wid == win->widExclMotion) {
			/* Receives mouse-motion exclusively. */
			goto out;
		}
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		PostMouseMotion(win, chld, x, y, xRel, yRel);
out:
	AG_ObjectUnlock(wid);
}

/*
 * Process a Mouse Button Up (Button Released) event.
 * 
 * Called from AG_EventLoop(3) or from a user-defined AG_CustomEventLoop(3).
 * The agDrivers VFS must be locked.
 */
void
AG_ProcessMouseButtonUp(AG_Window *win, int x, int y, AG_MouseButton button)
{
	AG_Widget *wid;

	OBJECT_FOREACH_CHILD(wid, win, ag_widget)
		PostMouseButtonUp(win, wid, x, y, button);
}

static void
PostMouseButtonUp(AG_Window *_Nonnull win, AG_Widget *_Nonnull wid,
    int x, int y, AG_MouseButton button)
{
	AG_Widget *chld;
	Uint flags;

	AG_ObjectLock(wid);
	flags = wid->flags;

	if ((flags & AG_WIDGET_VISIBLE) == 0) {
		/*
		 * If a widget is hidden, it is implied that its children are
		 * hidden as well. TODO: Lockless visibility determination.
		 */
		goto out;
	}

	if ((flags & AG_WIDGET_FOCUSED) ||
	    (flags & AG_WIDGET_UNFOCUSED_BUTTONUP)) {

		if (WIDGET_OPS(wid)->mouse_button_up != NULL) {
			WIDGET_OPS(wid)->mouse_button_up(wid,
			    button,
		            x - wid->rView.x1,
		            y - wid->rView.y1);
		} else {
			AG_PostEvent(wid, "mouse-button-up",
			    "%i(button),%i(x),%i(y)",
			    (int)button,
			    x - wid->rView.x1,
			    y - wid->rView.y1);
		}
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		PostMouseButtonUp(win, chld, x, y, button);
out:
	AG_ObjectUnlock(wid);
}

/*
 * Process a Mouse Button Down (Button Pressed) event.
 *
 * Called from AG_EventLoop(3) or from a user-defined AG_CustomEventLoop(3).
 * The agDrivers VFS must be locked.
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

static void
PostMouseButtonDown(AG_Window *_Nonnull win, AG_Widget *_Nonnull wid,
    int x, int y, AG_MouseButton button)
{
	AG_Widget *chld;
	AG_Event *ev;
	
	AG_ObjectLock(wid);

	if ((wid->flags & AG_WIDGET_VISIBLE) == 0)
		goto out;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		PostMouseButtonDown(win, chld, x, y, button);

	if ((wid->flags & AG_WIDGET_UNFOCUSED_BUTTONDOWN) == 0) {
		if (!AG_WidgetSensitive(wid, x,y))
			goto out;
	}

	if (WIDGET_OPS(wid)->mouse_button_down != NULL) {
		WIDGET_OPS(wid)->mouse_button_down(wid,
		    button,
	            x - wid->rView.x1,
	            y - wid->rView.y1);
	} else {
		TAILQ_FOREACH(ev, &OBJECT(wid)->events, events) {
			if (strcmp(ev->name, "mouse-button-down") == 0)
				break;
		}
		if (ev != NULL)
			AG_PostEvent(wid, "mouse-button-down",
			    "%i(button),%i(x),%i(y)",
			    (int)button,
			    x - wid->rView.x1,
			    y - wid->rView.y1);
	}
out:
	AG_ObjectUnlock(wid);
}

#if defined(AG_WIDGETS) && defined(AG_DEBUG)
static void *
Edit(void *obj)
{
	AG_Mouse *mouse = obj;
	AG_Box *box;
	AG_Label *lbl;

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);

	lbl = AG_LabelNewS(box, 0, AGINPUTDEV(mouse)->desc);
	AG_SetFontFamily(lbl, "league-spartan");
	AG_SetFontSize(lbl, "130%");

	lbl = AG_LabelNewPolledMT(box,
	    AG_LABEL_HFILL,
	    &OBJECT(mouse)->lock,
	    _("Button Count: %u\n"
	      "Button State: 0x%x\n"
	      "Cursor Position: %d,%d [%d %d]"),
	    AGINPUTDEV(mouse)->desc,
	    &mouse->nButtons, &mouse->btnState,
	    &mouse->x, &mouse->y,
	    &mouse->xRel, &mouse->yRel);
	AG_RedrawOnTick(lbl, 100);

	return (box);
}
#endif /* AG_WIDGETS and AG_DEBUG */

AG_ObjectClass agMouseClass = {
	"Agar(InputDevice:Mouse)",
	sizeof(AG_Mouse),
	{ 0,0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
	Edit
#else
	NULL		/* edit */
#endif
};

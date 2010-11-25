/*
 * Copyright (c) 2001-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "window.h"
#include "titlebar.h"
#include "icon.h"

#include "primitive.h"
#include "icons.h"
#include "cursors.h"
#include "label.h"

#include <string.h>
#include <stdarg.h>

static void Shown(AG_Event *);
static void Hidden(AG_Event *);
static void GainFocus(AG_Event *);
static void LostFocus(AG_Event *);

int agWindowSideBorderDefault = 0;
int agWindowBotBorderDefault = 6;
AG_Window *agWindowToFocus = NULL;
AG_Window *agWindowFocused = NULL;
struct ag_windowq agWindowDetachQ;
struct ag_widgetq agWidgetDetachQ;

void
AG_InitWindowSystem(void)
{
	TAILQ_INIT(&agWindowDetachQ);
	TAILQ_INIT(&agWidgetDetachQ);
	agWindowToFocus = NULL;
	agWindowFocused = NULL;
}

void
AG_DestroyWindowSystem(void)
{
}

/* Create a generic window. */
AG_Window *
AG_WindowNew(Uint flags)
{
	AG_Driver *drv;
	AG_Window *win;

	win = Malloc(sizeof(AG_Window));
	AG_ObjectInit(win, &agWindowClass);
	AG_ObjectSetNameS(win, "generic");
	OBJECT(win)->flags &= ~(AG_OBJECT_NAME_ONATTACH);
	WIDGET(win)->window = win;

	win->flags |= flags;
	
	if (win->flags & AG_WINDOW_MODAL)
		win->flags |= AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOMINIMIZE;
	if (win->flags & AG_WINDOW_NORESIZE)
		win->flags |= AG_WINDOW_NOMAXIMIZE;

	AG_SetEvent(win, "window-close", AGWINDETACH(win));

	switch (agDriverOps->wm) {
	case AG_WM_SINGLE:
		/*
		 * In single-display mode, windows are attached to the driver
		 * object.
		 * XXX todo: WindowNew() variant with parent driver arg.
		 */
		AG_ObjectAttach(agDriverSw, win);
	
		if (!(win->flags & AG_WINDOW_NOTITLE)) {
			win->hMin += agTextFontHeight;
		}
		if (win->flags & AG_WINDOW_NOBORDERS) {
			win->wBorderSide = 0;
			win->wBorderBot = 0;
		}
		break;
	case AG_WM_MULTIPLE:
		/*
		 * In multiple-display mode, one driver instance is created
		 * for each window.
		 */
		if ((drv = AG_DriverOpen(agDriverOps)) == NULL) {
			/* XXX TODO provide an error-check constructor */
			AG_FatalError(NULL);
		}
		AG_ObjectAttach(drv, win);
		AGDRIVER_MW(drv)->win = win;

		win->wBorderSide = 0;
		win->wBorderBot = 0;
		break;
	}
	return (win);
}

/* Create a named window (C string). */
AG_Window *
AG_WindowNewNamedS(Uint flags, const char *name)
{
	AG_Window *win;

#ifdef AG_DEBUG
	const char *p;
	if ((p = strchr(name, '/')) != NULL)
		AG_FatalError("Window names cannot contain `/' (near \"%s\")", p);
#endif

	AG_LockVFS(&agDrivers);
	if (AG_WindowFocusNamed(name)) {
		win = NULL;
		goto out;
	}
	win = AG_WindowNew(flags);
	AG_ObjectSetNameS(win, name);
	AG_SetEvent(win, "window-close", AGWINHIDE(win));
out:
	AG_UnlockVFS(&agDrivers);
	return (win);
}

/* Create a named window (format string). */
AG_Window *
AG_WindowNewNamed(Uint flags, const char *fmt, ...)
{
	char s[AG_OBJECT_NAME_MAX];
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);
	return AG_WindowNewNamedS(flags, s);
}

/*
 * Window attach function (we don't use the default Object attach function
 * because AG_WINDOW_KEEPBELOW windows have to be inserted at the head of
 * the list).
 */
static void
Attach(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv = OBJECT(win)->parent;
	
	/* Attach the window. */
	if (win->flags & AG_WINDOW_KEEPBELOW) {
		TAILQ_INSERT_HEAD(&OBJECT(drv)->children, OBJECT(win), cobjs);
	} else {
		TAILQ_INSERT_TAIL(&OBJECT(drv)->children, OBJECT(win), cobjs);
	}
	if (AGDRIVER_MULTIPLE(drv))
		AGDRIVER_MW(drv)->win = win;

	/*
	 * Notify the objects. This will cause the the "drv" and "drvOps"
	 * pointers of all widgets to be updated.
	 */
	AG_PostEvent(drv, win, "attached", NULL);
	
	if (AGDRIVER_SINGLE(drv)) {
		/*
		 * Initialize the built-in window titlebar and icon now that
		 * we have an attached driver. We could not do this earlier
		 * because surface operations are involved.
		 */
		if (win->tbar == NULL &&
		    !(win->flags & AG_WINDOW_NOTITLE)) {
			Uint titlebarFlags = 0;

			if (win->flags & AG_WINDOW_NOCLOSE)
				titlebarFlags |= AG_TITLEBAR_NO_CLOSE;
			if (win->flags & AG_WINDOW_NOMINIMIZE)
				titlebarFlags |= AG_TITLEBAR_NO_MINIMIZE;
			if (win->flags & AG_WINDOW_NOMAXIMIZE)
				titlebarFlags |= AG_TITLEBAR_NO_MAXIMIZE;

			win->tbar = AG_TitlebarNew(win, titlebarFlags);
		}
		if (win->icon == NULL) {
			win->icon = AG_IconNew(NULL, 0);
		}
		WIDGET(win->icon)->drv = drv;
		WIDGET(win->icon)->drvOps = AGDRIVER_CLASS(drv);
		AG_IconSetSurfaceNODUP(win->icon, agIconWindow.s);
		AG_IconSetBackgroundFill(win->icon, 1, agColors[BG_COLOR]);
	}
	
	if (win->flags & AG_WINDOW_FOCUSONATTACH)
		AG_WindowFocus(win);
}

/* Window detach function. */
static void
Detach(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv = OBJECT(win)->parent, *odrv;
	AG_Window *subwin, *nsubwin;
	AG_Window *owin;

#ifdef AG_DEBUG
	if (drv == NULL || !AG_OfClass(drv, "AG_Driver:*"))
		AG_FatalError("Window is not attached to a Driver");
#endif
	AG_LockVFS(&agDrivers);

	/* Implicitely hide the window. */
	if (win->visible)
		AG_WindowHide(win);
	
	/* Window is being detached. */
	win->flags |= AG_WINDOW_DETACHING;

	/* The window's titlebar and icon are no longer safe to use. */
	if (win->tbar != NULL)
		win->tbar = NULL;
	if (win->icon != NULL)
		win->icon = NULL;
	
	/* Cancel any planned focus change to this window. */
	if (win == agWindowToFocus) {
		Debug(NULL, "%s has been detached; cancelling focus change\n",
		    OBJECT(win)->name);
		agWindowToFocus = NULL;
	}

	/* Remove any reference from another window to this one. */
	AGOBJECT_FOREACH_CHILD(odrv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(owin, odrv) {
			if (owin == win) {
				continue;
			}
			TAILQ_FOREACH(subwin, &owin->subwins, swins) {
				if (subwin == win)
					break;
			}
			if (subwin != NULL)
				TAILQ_REMOVE(&owin->subwins, subwin, swins);
		}
	}
	
	/*
	 * Notify the objects. This will cause the the "drv" and "drvOps"
	 * pointers of all widgets to be reset to NULL.
	 */
	AG_PostEvent(drv, win, "detached", NULL);

 	/*
	 * For a window detach to be safe in event context, the window
	 * list cannot be directly altered. We place the window in the
	 * driver's "detach" queue, to be destroyed at the end of the
	 * current event processing cycle.
	 */
	TAILQ_INSERT_TAIL(&agWindowDetachQ, win, detach);

	/* Implicitely queue the sub-windows for detachment as well */
	for (subwin = TAILQ_FIRST(&win->subwins);
	     subwin != TAILQ_END(&win->subwins);
	     subwin = nsubwin) {
		nsubwin = TAILQ_NEXT(subwin, swins);
		AG_ObjectDetach(subwin);
	}
	TAILQ_INIT(&win->subwins);
	
	AG_UnlockVFS(&agDrivers);
}

static void
Init(void *obj)
{
	AG_Window *win = obj;
	AG_Event *ev;

	win->flags = 0;
	win->visible = 0;
	win->alignment = AG_WINDOW_ALIGNMENT_NONE;
	win->spacing = 3;
	win->lPad = 2;
	win->rPad = 2;
	win->tPad = 2;
	win->bPad = 2;
	win->wReq = 0;
	win->hReq = 0;
	win->wMin = win->lPad + win->rPad + 16;
	win->hMin = win->tPad + win->bPad + 16;
	win->minPct = 50;
	win->wBorderBot = agWindowBotBorderDefault;
	win->wBorderSide = agWindowSideBorderDefault;
	win->wResizeCtrl = 16;
	win->rSaved = AG_RECT(-1,-1,-1,-1);
	win->r = AG_RECT(0,0,0,0);
	win->caption[0] = '\0';
	win->tbar = NULL;
	win->icon = NULL;
	win->nFocused = 0;
	win->parent = NULL;
	win->widExclMotion = NULL;
	TAILQ_INIT(&win->subwins);
	TAILQ_INIT(&win->cursorAreas);

	AG_SetEvent(win, "window-gainfocus", GainFocus, NULL);
	AG_SetEvent(win, "window-lostfocus", LostFocus, NULL);

	/*
	 * Arrange for propagation of the widget-shown, widget-hidden and
	 * detached events to attached widgets.
	 */
	ev = AG_SetEvent(win, "widget-shown", Shown, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "widget-hidden", Hidden, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "detached", NULL, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;

	/* Custom attach/detach hooks are needed by the window stack. */
	AG_ObjectSetAttachFn(win, Attach, NULL);
	AG_ObjectSetDetachFn(win, Detach, NULL);

#ifdef AG_DEBUG
	AG_BindUint(win, "flags", &win->flags);
	AG_BindString(win, "caption", win->caption, sizeof(win->caption));
	AG_BindInt(win, "visible", &win->visible);
	AG_BindUint(win, "alignment", &win->alignment);
	AG_BindInt(win, "spacing", &win->spacing);
	AG_BindInt(win, "tPad", &win->tPad);
	AG_BindInt(win, "bPad", &win->bPad);
	AG_BindInt(win, "lPad", &win->lPad);
	AG_BindInt(win, "rPad", &win->rPad);
	AG_BindInt(win, "wReq", &win->wReq);
	AG_BindInt(win, "hReq", &win->hReq);
	AG_BindInt(win, "wMin", &win->wMin);
	AG_BindInt(win, "hMin", &win->hMin);
	AG_BindInt(win, "wBorderBot", &win->wBorderBot);
	AG_BindInt(win, "wBorderSide", &win->wBorderSide);
	AG_BindInt(win, "wResizeCtrl", &win->wResizeCtrl);
	AG_BindInt(win, "rSaved.x", &win->rSaved.x);
	AG_BindInt(win, "rSaved.y", &win->rSaved.y);
	AG_BindInt(win, "rSaved.w", &win->rSaved.w);
	AG_BindInt(win, "rSaved.h", &win->rSaved.h);
	AG_BindInt(win, "r.x", &win->r.x);
	AG_BindInt(win, "r.y", &win->r.y);
	AG_BindInt(win, "r.w", &win->r.w);
	AG_BindInt(win, "r.h", &win->r.h);
	AG_BindInt(win, "minPct", &win->minPct);
	AG_BindInt(win, "nFocused", &win->nFocused);
#endif /* AG_DEBUG */
}

/* Attach a sub-window. */
void
AG_WindowAttach(AG_Window *win, AG_Window *subwin)
{
	AG_Driver *drvWin = WIDGET(win)->drv;

	if (win == NULL)
		return;

	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);

	if (subwin->parent != NULL) {			/* Reparent? */
		if (subwin->parent == win) {
			goto out;
		}
		AG_WindowDetach(subwin->parent, subwin);
	}

	/* Update the pointers */
	subwin->parent = win;
	TAILQ_INSERT_HEAD(&win->subwins, subwin, swins);

	/* Pass the "transient for" hint to window managers where supported. */
	if (AGDRIVER_MULTIPLE(drvWin) &&
	    AGDRIVER_MW_CLASS(drvWin)->setTransientFor != NULL)
		AGDRIVER_MW_CLASS(drvWin)->setTransientFor(win, subwin);
out:
	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Detach a sub-window. */
void
AG_WindowDetach(AG_Window *win, AG_Window *subwin)
{
	AG_Driver *drvWin;

	if (win == NULL)
		return;

	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);

#ifdef AG_DEBUG
	if (subwin->parent != win)
		AG_FatalError("Inconsistent AG_WindowDetach()");
#endif
	drvWin = WIDGET(win)->drv;

	/* Pass the "transient for" hint to window managers where supported. */
	if (AGDRIVER_MULTIPLE(drvWin) &&
	    AGDRIVER_MW_CLASS(drvWin)->setTransientFor != NULL)
		AGDRIVER_MW_CLASS(drvWin)->setTransientFor(win, NULL);

	/* Update the pointers */
	TAILQ_REMOVE(&win->subwins, subwin, swins);
	subwin->parent = NULL;

	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Evaluate whether a widget is requesting geometry update. */
static int
UpdateNeeded(AG_Widget *wid)
{
	AG_Widget *chld;

	if (wid->flags & AG_WIDGET_UPDATE_WINDOW) {
		return (1);
	}
	WIDGET_FOREACH_CHILD(chld, wid) {
		if (UpdateNeeded(chld))
			return (1);
	}
	return (0);
}

static void
Draw(void *obj)
{
	AG_Window *win = obj;
	AG_Widget *chld;

	if (UpdateNeeded(WIDGET(win)))
		AG_WindowUpdate(win);
	
	STYLE(win)->Window(win);

	if (!(win->flags & AG_WINDOW_NOCLIPPING)) {
		AG_PushClipRect(win, win->r);
	}
	WIDGET_FOREACH_CHILD(chld, win) {
		AG_WidgetDraw(chld);
	}
	if (!(win->flags & AG_WINDOW_NOCLIPPING))
		AG_PopClipRect(win);
}

static void
Shown(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv = WIDGET(win)->drv;
	AG_SizeReq r;
	AG_SizeAlloc a;
	int xPref, yPref;
	Uint mwFlags = 0;

	if (WIDGET(win)->x == -1 && WIDGET(win)->y == -1) {
		AG_WidgetSizeReq(win, &r);
		if (AGDRIVER_SINGLE(drv)) {
			AG_WM_GetPrefPosition(win, &xPref, &yPref, r.w, r.h);
			a.x = xPref;
			a.y = yPref;
		} else {
			mwFlags |= AG_DRIVER_MW_ANYPOS;
			a.x = 0;
			a.y = 0;
		}
		a.w = r.w;
		a.h = r.h;
	} else {
		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDTH(win);
		a.h = HEIGHT(win);
	}
	AG_WidgetSizeAlloc(win, &a);

	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_SINGLE:
		/* Append to modal window list */
		if (win->flags & AG_WINDOW_MODAL) {
			AG_Variable Vmodal;
			AG_InitPointer(&Vmodal, win);
			AG_ListAppend(AGDRIVER_SW(drv)->Lmodal, &Vmodal);
		}
		AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);
		break;
	case AG_WM_MULTIPLE:
		/*
		 * Create/map a native window of requested dimensions.
		 * We expect the driver will call AG_WidgetUpdateCoords().
		 */
		if (!(AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN)) {
			if (AGDRIVER_MW_CLASS(drv)->openWindow(win,
			    AG_RECT(a.x, a.y, a.w, a.h), 0,
			    mwFlags) == -1) {
				AG_FatalError(NULL);
			}
		}
		if (AGDRIVER_MW_CLASS(drv)->mapWindow(win) == -1) {
			AG_FatalError(NULL);
		}
		if (AGDRIVER_MW_CLASS(drv)->setWindowCaption != NULL) {
			AGDRIVER_MW_CLASS(drv)->setWindowCaption(win,
			    win->caption);
		}
		break;
	}

	if (!(win->flags & AG_WINDOW_DENYFOCUS))
		AG_WindowFocus(win);

	/* Notify that the window is now visible. */
	AG_PostEvent(NULL, win, "window-shown", NULL);

	/* Assume we gained focus. XXX */
	AG_PostEvent(NULL, win, "window-gainfocus", NULL);

	/* Apply initial alignment setting */
	if (win->alignment != AG_WINDOW_ALIGNMENT_NONE) {
		AG_WindowSetGeometryAligned(win,
		    win->alignment,
		    WIDTH(win),
		    HEIGHT(win));
	}

	/* Mark for redraw */
	win->dirty = 1;
}

static void
Hidden(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverSw *dsw;
	int i;

	/* Cancel any pending redraw. */
	win->dirty = 0;

	/* Cancel any pending focus change to this window. */
	if (win == agWindowToFocus)
		agWindowToFocus = NULL;

	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_SINGLE:
		dsw = (AG_DriverSw *)drv;

		if (OBJECT(drv)->parent == NULL)
			break;

		/* Remove from the modal window list if modal. */
		if (win->flags & AG_WINDOW_MODAL) {
			for (i = 0; i < dsw->Lmodal->n; i++) {
				if (dsw->Lmodal->v[i].data.p == win)
					break;
			}
			if (i < dsw->Lmodal->n)
				AG_ListRemove(dsw->Lmodal, i);
		}

		/* Update the background. */
		/* XXX XXX XXX no need for the fill rect? */
		if (AGDRIVER_CLASS(drv)->type == AG_FRAMEBUFFER) {
			AG_DrawRectFilled(win,
			    AG_RECT(0,0, WIDTH(win), HEIGHT(win)),
			    agColors[BG_COLOR]);
			if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
				AGDRIVER_CLASS(drv)->updateRegion(drv,
				    AG_RECT(WIDGET(win)->x, WIDGET(win)->y,
				            WIDTH(win), HEIGHT(win)));
		}
		break;
	case AG_WM_MULTIPLE:
		if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
			if (AGDRIVER_MW_CLASS(drv)->unmapWindow(win) == -1)
				AG_FatalError(NULL);
		}
		break;
	}
	AG_PostEvent(NULL, win, "window-hidden", NULL);
}

static void
WidgetGainFocus(AG_Widget *wid)
{
	AG_Widget *chld;

	WIDGET_FOREACH_CHILD(chld, wid) {
		AG_ObjectLock(chld);
		WidgetGainFocus(chld);
		AG_ObjectUnlock(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(NULL, wid, "widget-gainfocus", NULL);
}

static void
WidgetLostFocus(AG_Widget *wid)
{
	AG_Widget *chld;

	WIDGET_FOREACH_CHILD(chld, wid) {
		AG_ObjectLock(chld);
		WidgetLostFocus(chld);
		AG_ObjectUnlock(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(NULL, wid, "widget-lostfocus", NULL);
}

static void
GainFocus(AG_Event *event)
{
	WidgetGainFocus(WIDGET(AG_SELF()));
}

static void
LostFocus(AG_Event *event)
{
	WidgetLostFocus(WIDGET(AG_SELF()));
}

/* Set the visibility bit of a window. */
void
AG_WindowShow(AG_Window *win)
{
	AG_ObjectLock(win);
	if (!win->visible) {
		AG_PostEvent(NULL, win, "widget-shown", NULL);
		win->visible++;
	}
	AG_ObjectUnlock(win);
}

/* Clear the visibility bit of a window. */
void
AG_WindowHide(AG_Window *win)
{
	AG_ObjectLock(win);
	if (win->visible) {
		win->visible = 0;
		AG_PostEvent(NULL, win, "widget-hidden", NULL);	/* Hidden() */
	}
	AG_ObjectUnlock(win);
}

/* Build an ordered list of the focusable widgets in a window. */
static void
ListFocusableWidgets(AG_List *L, AG_Widget *wid)
{
	AG_Widget *chld;
	AG_Variable V;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_FOCUSABLE) {
		AG_InitPointer(&V, wid->focusFwd ? wid->focusFwd : wid);
		AG_ListAppend(L, &V);
	}
	AG_ObjectUnlock(wid);

	WIDGET_FOREACH_CHILD(chld, wid)
		ListFocusableWidgets(L, chld);
}

/*
 * Move the widget focus inside a window.
 * The window must be locked.
 */
void
AG_WindowCycleFocus(AG_Window *win, int reverse)
{
	AG_List *Lfoc, *Luniq;
	int i, j;

	/* Generate a list of focusable widgets; eliminate duplicates. */
	Lfoc = AG_ListNew();
	Luniq = AG_ListNew();
	ListFocusableWidgets(Lfoc, WIDGET(win));
	for (i = 0; i < Lfoc->n; i++) {
		for (j = 0; j < Luniq->n; j++) {
			if (Lfoc->v[i].data.p == Luniq->v[j].data.p)
				break;
		}
		if (j == Luniq->n)
			AG_ListAppend(Luniq, &Lfoc->v[i]);
	}
	if (Luniq->n == 0)
		goto out;

	/* Move focus after/before the currently focused widget. */
	if (reverse) {
		for (i = 0; i < Luniq->n; i++) {
			if (WIDGET(Luniq->v[i].data.p)->flags & AG_WIDGET_FOCUSED)
				break;
		}
		if (i == -1) {
			AG_WidgetFocus(Luniq->v[0].data.p);
		} else {
			if (i-1 < 0) {
				AG_WidgetFocus(Luniq->v[Luniq->n - 1].data.p);
			} else {
				AG_WidgetFocus(Luniq->v[i - 1].data.p);
			}
		}
	} else {
		for (i = Luniq->n-1; i >= 0; i--) {
			if (WIDGET(Luniq->v[i].data.p)->flags & AG_WIDGET_FOCUSED)
				break;
		}
		if (i == Luniq->n) {
			AG_WidgetFocus(Luniq->v[0].data.p);
		} else {
			if (i+1 < Luniq->n) {
				AG_WidgetFocus(Luniq->v[i + 1].data.p);
			} else {
				AG_WidgetFocus(Luniq->v[0].data.p);
			}
		}
	}
out:
	AG_ListDestroy(Lfoc);
	AG_ListDestroy(Luniq);
	win->dirty = 1;
}

/*
 * Give focus to a window. For single-window drivers, the operation only takes
 * effect at the end of the current event cycle. For multiple-window drivers,
 * the change takes effect immediately. If the window is not attached to a
 * driver, the operation is deferred until the next attach. If NULL is given,
 * cancel any planned focus change.
 */
void
AG_WindowFocus(AG_Window *win)
{
	AG_LockVFS(&agDrivers);

	if (win == NULL) {
		agWindowToFocus = NULL;
		goto out;
	}
	AG_ObjectLock(win);
	if (OBJECT(win)->parent == NULL) {
		/* Will focus on future attach */
		win->flags |= AG_WINDOW_FOCUSONATTACH;
	} else {
		agWindowToFocus = win;
	}
	win->dirty = 1;
	AG_ObjectUnlock(win);
out:
	AG_UnlockVFS(&agDrivers);
}

/* Give focus to a window by name, and show it is if is hidden. */
int
AG_WindowFocusNamed(const char *name)
{
	AG_Driver *drv;
	AG_Window *owin;
	int rv = 0;

	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(owin, drv) {
			if (strcmp(OBJECT(owin)->name, name) == 0) {
				AG_WindowShow(owin);
				AG_WindowFocus(owin);
			    	rv = 1;
				goto out;
			}
		}
	}
out:
	AG_UnlockVFS(&agDrivers);
	return (rv);
}

/*
 * Place focus on a Window following a click at the given coordinates,
 * in the video context of a specified single-display driver.
 *
 * Returns 1 if the focus state has changed as a result.
 */
int
AG_WindowFocusAtPos(AG_DriverSw *dsw, int x, int y)
{
	AG_Window *win;

	AG_ASSERT_CLASS(dsw, "AG_Driver:AG_DriverSw:*");
	agWindowToFocus = NULL;
	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		AG_ObjectLock(win);
		if (!win->visible ||
		    !AG_WidgetArea(win, x,y) ||
		    (win->flags & AG_WINDOW_DENYFOCUS)) {
			AG_ObjectUnlock(win);
			continue;
		}
		agWindowToFocus = win;
		AG_ObjectUnlock(win);
		return (1);
	}
	return (0);
}

/* Update window background after geometry change in single-display mode. */
/* XXX TODO Avoid drawing over KEEPABOVE windows */
static void
UpdateWindowBG(AG_Window *win, AG_Rect rPrev)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_Rect r;

	if (WIDGET(win)->x > rPrev.x) {			/* L-resize */
		r.x = rPrev.x;
		r.y = rPrev.y;
		r.w = WIDGET(win)->x - rPrev.x;
		r.h = HEIGHT(win);
	} else if (WIDTH(win) < rPrev.w) {		/* R-resize */
		r.x = WIDGET(win)->x + WIDTH(win);
		r.y = WIDGET(win)->y;
		r.w = rPrev.w - WIDTH(win);
		r.h = rPrev.h;
	} else {
		r.w = 0;
		r.h = 0;
	}
	if (r.w > 0 && r.h > 0) {
		AGDRIVER_CLASS(drv)->fillRect(drv, r, agColors[BG_COLOR]);
		if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
			AGDRIVER_CLASS(drv)->updateRegion(drv, r);
	}
	if (HEIGHT(win) < rPrev.h) {				/* H-resize */
		r.x = rPrev.x;
		r.y = WIDGET(win)->y + HEIGHT(win);
		r.w = rPrev.w;
		r.h = rPrev.h - HEIGHT(win);
			
		AGDRIVER_CLASS(drv)->fillRect(drv, r, agColors[BG_COLOR]);
		if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
			AGDRIVER_CLASS(drv)->updateRegion(drv, r);
	}
}

/* 
 * Set window coordinates and geometry. This should be used instead of
 * a direct WidgetSizeAlloc() since extra processing is done according
 * to the current driver in use.
 *
 * If "bounded" is non-zero, the window is bounded to the display area.
 */
int
AG_WindowSetGeometryRect(AG_Window *win, AG_Rect r, int bounded)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_SizeReq rWin;
	AG_SizeAlloc a;
	AG_Rect rPrev;
	int new;
	int nw, nh;
	int wMin, hMin;
	Uint wDisp, hDisp;

	if (AG_GetDisplaySize(drv, &wDisp, &hDisp) == -1) {
		wDisp = 0;
		hDisp = 0;
	}

	AG_ObjectLock(win);
	rPrev = AG_RECT(WIDGET(win)->x, WIDGET(win)->y,
	                WIDGET(win)->w, WIDGET(win)->h);
	new = ((WIDGET(win)->x == -1 || WIDGET(win)->y == -1));

	/* Compute the final window size. */
	if (r.w == -1 || r.h == -1) {
		AG_WidgetSizeReq(win, &rWin);
		nw = (r.w == -1) ? rWin.w : r.w;
		nh = (r.h == -1) ? rWin.h : r.h;
	} else {
		nw = r.w;
		nh = r.h;
	}
	if (win->flags & AG_WINDOW_MINSIZEPCT) {
		wMin = win->minPct*win->wReq/100;
		hMin = win->minPct*win->hReq/100;
	} else {
		wMin = win->wMin;
		hMin = win->hMin;
	}
	if (nw < wMin) { nw = wMin; }
	if (nh < hMin) { nh = hMin; }
	if (WIDGET(win)->x == -1) {
		WIDGET(win)->x = wDisp/2 - nw/2;
	}
	if (WIDGET(win)->y == -1) {
		WIDGET(win)->y = hDisp/2 - nh/2;
	}
	a.x = (r.x == -1) ? WIDGET(win)->x : r.x;
	a.y = (r.y == -1) ? WIDGET(win)->y : r.y;
	a.w = nw;
	a.h = nh;
	if (bounded)
		AG_WM_LimitWindowToDisplaySize(drv, &a);

	/* Invoke the driver-specific pre-resize callback routine. */
	if (win->visible && AGDRIVER_MULTIPLE(drv))
		AGDRIVER_MW_CLASS(drv)->preResizeCallback(win);
	
	/*
	 * Resize the widgets and update their coordinates; update
	 * the geometry of the window's Widget structure.
	 */
	if (AG_WidgetSizeAlloc(win, &a) == -1) {
		goto fail;
	}
	AG_WidgetUpdateCoords(win, a.x, a.y);

	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_SINGLE:
		if (win->visible && !new) {
			UpdateWindowBG(win, rPrev);
		}
		break;
	case AG_WM_MULTIPLE:
		if ((AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) &&
		    AGDRIVER_MW_CLASS(drv)->moveResizeWindow(win, &a) == -1) {
			goto fail;
		}
		break;
	}

	win->dirty = 1;
	AG_ObjectUnlock(win);
	return (0);
fail:
	if (!new) {						/* Revert */
		a.x = rPrev.x;
		a.y = rPrev.y;
		a.w = rPrev.w;
		a.h = rPrev.h;
		AG_WidgetSizeAlloc(win, &a);
		AG_WidgetUpdateCoords(win, a.x, a.y);
	}
	AG_ObjectUnlock(win);
	return (-1);
}

/* Configure minimum window size in percentage of computed geometry. */
void
AG_WindowSetMinSizePct(AG_Window *win, int pct)
{
	AG_ObjectLock(win);
	win->flags |= AG_WINDOW_MINSIZEPCT;
	win->minPct = pct;
	win->dirty = 1;
	AG_ObjectUnlock(win);
}

/* Configure minimum window size in pixels. */
void
AG_WindowSetMinSize(AG_Window *win, int w, int h)
{
	AG_ObjectLock(win);
	win->flags &= ~(AG_WINDOW_MINSIZEPCT);
	win->wMin = w;
	win->hMin = h;
	win->dirty = 1;
	AG_ObjectUnlock(win);
}

/* Assign a window a specific alignment and size in pixels. */
int
AG_WindowSetGeometryAligned(AG_Window *win, enum ag_window_alignment alignment,
    int w, int h)
{
	Uint wMax, hMax;
	int x, y;

	win->alignment = alignment;
	AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax);

	switch (alignment) {
	case AG_WINDOW_ALIGNMENT_NONE:
		return (0);
	case AG_WINDOW_TL:
		x = 0;
		y = 0;
		break;
	case AG_WINDOW_TC:
		x = wMax/2 - w/2;
		y = 0;
		break;
	case AG_WINDOW_TR:
		x = wMax - w;
		y = 0;
		break;
	case AG_WINDOW_ML:
		x = 0;
		y = hMax/2 - h/2;
		break;
	case AG_WINDOW_MR:
		x = wMax - w;
		y = hMax/2 - h/2;
		break;
	case AG_WINDOW_BL:
		x = 0;
		y = hMax - h;
		break;
	case AG_WINDOW_BC:
		x = wMax/2 - w/2;
		y = hMax - h;
		break;
	case AG_WINDOW_BR:
		x = wMax - w;
		y = hMax - h;
		break;
	case AG_WINDOW_MC:
	default:
		x = wMax/2 - w/2;
		y = hMax/2 - h/2;
		break;
	}
	
	/* XXX hack to compensate for titlebars */
	if (AGDRIVER_MULTIPLE(WIDGET(win)->drv)) {
		switch (alignment) {
		case AG_WINDOW_TL:
		case AG_WINDOW_TC:
		case AG_WINDOW_TR:
			y += 34;
			if (y > hMax) { y = 0; }
			break;
		case AG_WINDOW_BL:
		case AG_WINDOW_BC:
		case AG_WINDOW_BR:
			y -= 34;
			if (y < 0) { y = 0; }
			break;
		default:
			break;
		}
	}
	return AG_WindowSetGeometry(win, x, y, w, h);
}

/* Assign a window a specific alignment and size in percentage of view area. */
int
AG_WindowSetGeometryAlignedPct(AG_Window *win, enum ag_window_alignment align,
    int wPct, int hPct)
{
	Uint wMax = 0, hMax = 0;

	AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax);

	return AG_WindowSetGeometryAligned(win, align,
	                                   wPct*wMax/100,
	                                   hPct*hMax/100);
}

/* Backup the current window geometry (i.e., before a minimize) */
void
AG_WindowSaveGeometry(AG_Window *win)
{
	win->rSaved.x = WIDGET(win)->x;
	win->rSaved.y = WIDGET(win)->y;
	win->rSaved.w = WIDTH(win);
	win->rSaved.h = HEIGHT(win);
}

/* Restore saved geometry (i.e., after an unminimize operation) */
int
AG_WindowRestoreGeometry(AG_Window *win)
{
	return AG_WindowSetGeometryRect(win, win->rSaved, 0);
}

/* Maximize a window */
void
AG_WindowMaximize(AG_Window *win)
{
	Uint wMax, hMax;

	AG_WindowSaveGeometry(win);
	AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax);
	if (AG_WindowSetGeometry(win, 0, 0, wMax, hMax) == 0)
		win->flags |= AG_WINDOW_MAXIMIZED;
}

/* Restore a window's geometry prior to maximization. */
void
AG_WindowUnmaximize(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_Rect r;

	if (AG_WindowRestoreGeometry(win) == 0) {
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
		/* XXX 1.4 */
		if (drv != NULL && AGDRIVER_SINGLE(drv)) {
			r.x = 0;
			r.y = 0;
			r.w = AGDRIVER_SW(drv)->w;
			r.h = AGDRIVER_SW(drv)->h;
			AGDRIVER_CLASS(drv)->fillRect(drv, r,
			    agColors[BG_COLOR]);
			if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
				AGDRIVER_CLASS(drv)->updateRegion(drv, r);
		}
	}
}

static void
IconMotion(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	AG_Driver *drv = WIDGET(icon)->drv;
	int xRel = AG_INT(3);
	int yRel = AG_INT(4);
	AG_Window *wDND = icon->wDND;

	if (icon->flags & AG_ICON_DND) {
		if (drv != NULL && AGDRIVER_SINGLE(drv)) {
			AG_Rect r;
			r.x = 0;
			r.y = 0;
			r.w = AGDRIVER_SW(drv)->w;
			r.h = AGDRIVER_SW(drv)->h;
			AGDRIVER_CLASS(drv)->fillRect(drv, r,
			    agColors[BG_COLOR]);
			r = AG_Rect2ToRect(WIDGET(wDND)->rView);
			if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
				AGDRIVER_CLASS(drv)->updateRegion(drv, r);
		}
		AG_WindowSetGeometryRect(wDND,
		    AG_RECT(WIDGET(wDND)->x + xRel,
		            WIDGET(wDND)->y + yRel,
			    WIDTH(wDND),
			    HEIGHT(wDND)), 1);
		 
		icon->xSaved = WIDGET(wDND)->x;
		icon->ySaved = WIDGET(wDND)->y;
		icon->wSaved = WIDTH(wDND);
		icon->hSaved = HEIGHT(wDND);
	}
}

static void
IconButtonDown(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	AG_Window *win = AG_PTR(1);

	WIDGET(icon)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                       AG_WIDGET_UNFOCUSED_BUTTONUP;
	if (icon->flags & AG_ICON_DBLCLICKED) {
		AG_CancelEvent(icon, "dblclick-expire");
		AG_WindowUnminimize(win);
		AG_ObjectDetach(win->icon);
		AG_ObjectDetach(icon->wDND);
		icon->wDND = NULL;
		icon->flags &= (AG_ICON_DND|AG_ICON_DBLCLICKED);
	} else {
		icon->flags |= (AG_ICON_DND|AG_ICON_DBLCLICKED);
		AG_SchedEvent(NULL, icon, agMouseDblclickDelay,
		    "dblclick-expire", NULL);
	}
}

static void
IconButtonUp(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	
	WIDGET(icon)->flags &= ~(AG_WIDGET_UNFOCUSED_MOTION);
	WIDGET(icon)->flags &= ~(AG_WIDGET_UNFOCUSED_BUTTONUP);
	icon->flags &= ~(AG_ICON_DND);
}

static void
DoubleClickTimeout(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	icon->flags &= ~(AG_ICON_DBLCLICKED);
}

/* Minimize a window */
void
AG_WindowMinimize(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;

	if (win->flags & AG_WINDOW_MINIMIZED) {
		return;
	}
	win->flags |= AG_WINDOW_MINIMIZED;
	AG_WindowHide(win);

	if (AGDRIVER_SINGLE(drv)) {
		AG_Window *wDND;
		AG_Icon *icon = win->icon;

		wDND = AG_WindowNew(AG_WINDOW_PLAIN|AG_WINDOW_KEEPBELOW|
		                    AG_WINDOW_DENYFOCUS|AG_WINDOW_NOBACKGROUND);
		AG_ObjectAttach(wDND, icon);
		icon->wDND = wDND;
		icon->flags &= ~(AG_ICON_DND|AG_ICON_DBLCLICKED);

		AG_SetEvent(icon, "dblclick-expire", DoubleClickTimeout, NULL);
		AG_SetEvent(icon, "mouse-motion", IconMotion, NULL);
		AG_SetEvent(icon, "mouse-button-up", IconButtonUp, NULL);
		AG_SetEvent(icon, "mouse-button-down", IconButtonDown, "%p", win);

		if (icon->xSaved != -1) {
			AG_WindowShow(wDND);
			AG_WindowSetGeometry(wDND, icon->xSaved, icon->ySaved,
			                     icon->wSaved, icon->hSaved);
		} else {
			AG_WindowSetPosition(wDND, AG_WINDOW_LOWER_LEFT, 1);
			AG_WindowShow(wDND);
			icon->xSaved = WIDGET(wDND)->x;
			icon->ySaved = WIDGET(wDND)->y;
			icon->wSaved = WIDTH(wDND);
			icon->hSaved = HEIGHT(wDND);
		}
	}
	/* TODO MW: send a WM_CHANGE_STATE */
}

/* Unminimize a window */
void
AG_WindowUnminimize(AG_Window *win)
{
	if (!win->visible) {
		AG_WindowShow(win);
		win->flags &= ~(AG_WINDOW_MINIMIZED);
	} else {
		AG_WindowFocus(win);
	}
	/* TODO MW: send a WM_CHANGE_STATE */
}

/* AGWINDETACH(): General-purpose "detach window" event handler. */
void
AG_WindowDetachGenEv(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_ObjectDetach(win);
}

/* AGWINHIDE(): General-purpose "hide window" event handler. */
void
AG_WindowHideGenEv(AG_Event *event)
{
	AG_WindowHide(AG_PTR(1));
}

/* AGWINCLOSE(): General-purpose "close window" event handler. */
void
AG_WindowCloseGenEv(AG_Event *event)
{
	AG_PostEvent(NULL, AG_PTR(1), "window-close", NULL);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Window *win = obj;
	AG_Driver *drv = WIDGET(win)->drv;
	AG_Widget *chld;
	AG_SizeReq rChld, rTbar;
	int nWidgets;
	int wTot;
	
	wTot = win->lPad + win->rPad + win->wBorderSide*2;
	r->w = wTot;
	r->h = win->bPad+win->tPad + win->wBorderBot;

	if (win->tbar != NULL) {
		AG_WidgetSizeReq(win->tbar, &rTbar);
		r->w = MAX(r->w, rTbar.w);
		r->h += rTbar.h;
	}
	nWidgets = 0;
	WIDGET_FOREACH_CHILD(chld, win) {
		if (chld == WIDGET(win->tbar)) {
			continue;
		}
		AG_WidgetSizeReq(chld, &rChld);
		r->w = MAX(r->w, rChld.w + wTot);
		r->h += rChld.h + win->spacing;
		nWidgets++;
	}
	if (nWidgets > 0 && r->h >= win->spacing)
		r->h -= win->spacing;

	win->wReq = r->w;
	win->hReq = r->h;

	if (AGDRIVER_SINGLE(drv))
		AG_WM_LimitWindowToView(win);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Window *win = obj;
	AG_Driver *drv = WIDGET(win)->drv;
	AG_Widget *chld;
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	int wAvail, hAvail;
	int totFixed;
	int nWidgets;

	/* Calculate total space available for widgets. */
	wAvail = a->w - win->lPad - win->rPad - win->wBorderSide*2;
	hAvail = a->h - win->bPad - win->tPad - win->wBorderBot;

	/* Calculate the space occupied by non-fill widgets. */
	nWidgets = 0;
	totFixed = 0;
	WIDGET_FOREACH_CHILD(chld, win) {
		AG_WidgetSizeReq(chld, &rChld);
		if ((chld->flags & AG_WIDGET_VFILL) == 0) {
			totFixed += rChld.h;
		}
		if (chld != WIDGET(win->tbar)) {
			totFixed += win->spacing;
		}
		nWidgets++;
	}
	if (nWidgets > 0 && totFixed >= win->spacing)
		totFixed -= win->spacing;

	/* Position the widgets. */
	if (win->tbar != NULL) {
		AG_WidgetSizeReq(win->tbar, &rChld);
		aChld.x = 0;
		aChld.y = 0;
		aChld.w = a->w;
		aChld.h = rChld.h;
		AG_WidgetSizeAlloc(win->tbar, &aChld);
		aChld.x = win->lPad + win->wBorderSide;
		aChld.y = rChld.h + win->tPad;
	} else {
		aChld.x = win->lPad + win->wBorderSide;
		aChld.y = win->tPad;
	}
	WIDGET_FOREACH_CHILD(chld, win) {
		AG_WidgetSizeReq(chld, &rChld);
		if (chld == WIDGET(win->tbar)) {
			continue;
		}
		if (chld->flags & AG_WIDGET_NOSPACING) {
			AG_SizeAlloc aTmp;
			AG_Widget *chldNext;

			aTmp.x = 0;
			aTmp.y = aChld.y;
			aTmp.w = a->w;
			aTmp.h = rChld.h;
			AG_WidgetSizeAlloc(chld, &aTmp);
			aChld.y += aTmp.h;

			chldNext = WIDGET_NEXT_CHILD(chld);
			if (chldNext == NULL ||
			    !(chldNext->flags & AG_WIDGET_NOSPACING)) {
				aChld.y += win->spacing;
			}
			continue;
		} else {
			aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
			          wAvail : MIN(wAvail,rChld.w);
		}
		aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
		          hAvail-totFixed : rChld.h;
		AG_WidgetSizeAlloc(chld, &aChld);
		aChld.y += aChld.h + win->spacing;
	}
	if (AGDRIVER_SINGLE(drv))
		AG_WM_LimitWindowToView(win);

	win->r.x = 0;
	win->r.y = 0;
	win->r.w = a->w;
	win->r.h = a->h;

	return (0);
}

/* Set the width of the window side borders. */
void
AG_WindowSetSideBorders(AG_Window *win, int pixels)
{
	if (win != NULL) {
		AG_ObjectLock(win);
		win->wBorderSide = pixels;
		win->dirty = 1;
		AG_ObjectUnlock(win);
	} else {
		agWindowSideBorderDefault = pixels;
	}
}

/* Set the width of the window bottom border. */
void
AG_WindowSetBottomBorder(AG_Window *win, int pixels)
{
	if (win != NULL) {
		AG_ObjectLock(win);
		win->wBorderBot = pixels;
		win->dirty = 1;
		AG_ObjectUnlock(win);
	} else {
		agWindowBotBorderDefault = pixels;
	}
}

/* Change the spacing between child widgets. */
void
AG_WindowSetSpacing(AG_Window *win, int spacing)
{
	AG_ObjectLock(win);
	win->spacing = spacing;
	win->dirty = 1;
	AG_ObjectUnlock(win);
}

/* Change the padding around child widgets. */
void
AG_WindowSetPadding(AG_Window *win, int lPad, int rPad, int tPad, int bPad)
{
	AG_ObjectLock(win);
	if (lPad != -1) { win->lPad = lPad; }
	if (rPad != -1) { win->rPad = rPad; }
	if (tPad != -1) { win->tPad = tPad; }
	if (bPad != -1) { win->bPad = bPad; }
	win->dirty = 1;
	AG_ObjectUnlock(win);
}

/* Request a specific initial window position. */
void
AG_WindowSetPosition(AG_Window *win, enum ag_window_alignment alignment,
    int cascade)
{
	AG_ObjectLock(win);
	win->alignment = alignment;
	AG_SETFLAGS(win->flags, AG_WINDOW_CASCADE, cascade);
	if (win->visible) {
		(void)AG_WindowSetGeometry(win, -1, -1, -1, -1);
	}
	AG_ObjectUnlock(win);
}

/* Set a default window-close handler. */
void
AG_WindowSetCloseAction(AG_Window *win, enum ag_window_close_action mode)
{
	AG_ObjectLock(win);

	switch (mode) {
	case AG_WINDOW_HIDE:
		AG_SetEvent(win, "window-close", AGWINHIDE(win));
		break;
	case AG_WINDOW_DETACH:
		AG_SetEvent(win, "window-close", AGWINDETACH(win));
		break;
	default:
		AG_UnsetEvent(win, "window-close");
		break;
	}

	AG_ObjectUnlock(win);
}

/* Update Agar's built-in titlebar from window caption. */
static void
UpdateTitlebar(AG_Window *win)
{
	AG_Titlebar *tbar = win->tbar;

	AG_ObjectLock(tbar);
	AG_LabelTextS(tbar->label, (win->caption != NULL) ? win->caption : "");
	AG_ObjectUnlock(tbar);
}

/* Update the window's minimized icon caption. */
static void
UpdateIconCaption(AG_Window *win)
{
	AG_Icon *icon = win->icon;
	char s[16], *c;

	if (Strlcpy(s, win->caption, sizeof(s)) >= sizeof(s)) {	/* Truncate */
		for (c = &s[0]; *c != '\0'; c++) {
			if (*c == ' ')
				*c = '\n';
		}
		AG_IconSetText(icon, "%s...", s);
	} else {
		AG_IconSetTextS(icon, s);
	}
}

/* Set the text to show inside a window's titlebar (C string). */
void
AG_WindowSetCaptionS(AG_Window *win, const char *s)
{
	AG_Driver *drv = WIDGET(win)->drv;

	AG_ObjectLock(win);
	Strlcpy(win->caption, s, sizeof(win->caption));

	if (win->tbar != NULL)
		UpdateTitlebar(win);
	if (win->icon != NULL)
		UpdateIconCaption(win);

	if (AGDRIVER_MULTIPLE(drv) &&
	    (AGDRIVER_MW(drv)->flags&AG_DRIVER_MW_OPEN) &&
	    (AGDRIVER_MW_CLASS(drv)->setWindowCaption != NULL)) {
		AGDRIVER_MW_CLASS(drv)->setWindowCaption(win, s);
	}
	win->dirty = 1;
	AG_ObjectUnlock(win);
}

/* Set the text to show inside a window's titlebar (format string). */
void
AG_WindowSetCaption(AG_Window *win, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;
	
	va_start(ap, fmt);
	Vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);

	AG_WindowSetCaptionS(win, s);
}

/*
 * Destroy windows queued for detach. Usually called by drivers, at the
 * end of the current event processing cycle.
 */
void
AG_FreeDetachedWindows(void)
{
	AG_Window *win, *winNext;
	AG_Driver *drv;

	for (win = TAILQ_FIRST(&agWindowDetachQ);
	     win != TAILQ_END(&agWindowDetachQ);
	     win = winNext) {
		winNext = TAILQ_NEXT(win, detach);

		/* Cancel any planned focus change to this window. */
		if (win == agWindowFocused)
			agWindowFocused = NULL;

		/* Release the cursor areas and associated cursors. */
		AG_UnmapAllCursors(win, NULL);

		/* Close the associated window in MW mode. */
		drv = WIDGET(win)->drv;
		if (AGDRIVER_MULTIPLE(drv) &&
		    AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN)
			AGDRIVER_MW_CLASS(drv)->closeWindow(win);

		/* Remove the Window detach handler and free the object. */
		AG_ObjectSetDetachFn(win, NULL, NULL);
		AG_ObjectDetach(win);

		/* Free the driver instance as well in MW mode. */
		if (AGDRIVER_MULTIPLE(drv))
			AG_DriverClose(drv);
		
		AG_ObjectDestroy(win);
	}
	TAILQ_INIT(&agWindowDetachQ);
}

/*
 * Configure a new cursor-change area with a specified cursor. The provided
 * cursor will be freed automatically on window detach.
 */
AG_CursorArea *
AG_MapCursor(void *obj, AG_Rect r, AG_Cursor *c)
{
	AG_Window *win = WIDGET(obj)->window;
	AG_CursorArea *ca;

	if (win == NULL) {
		AG_SetError("Unattached widget");
		return (NULL);
	}
	if ((ca = TryMalloc(sizeof(AG_CursorArea))) == NULL) {
		return (NULL);
	}
	ca->stock = -1;
	ca->r = r;
	ca->c = c;
	ca->wid = WIDGET(obj);
	TAILQ_INSERT_TAIL(&win->cursorAreas, ca, cursorAreas);
	return (ca);
}

/* Configure a new cursor-change area with a stock Agar cursor. */
AG_CursorArea *
AG_MapStockCursor(void *obj, AG_Rect r, int name)
{
	AG_Window *win = WIDGET(obj)->window;
	AG_Driver *drv = WIDGET(win)->drv;
	AG_CursorArea *ca;

	if (win == NULL || drv == NULL) {
		AG_SetError("Unattached widget");
		return (NULL);
	}
	if (name < 0 || name >= drv->nCursors) {
		AG_SetError("No such cursor");
		AG_Verbose("No such cursor: %d in %s\n", name, OBJECT(drv)->name);
		return (NULL);
	}
	if ((ca = TryMalloc(sizeof(AG_CursorArea))) == NULL) {
		return (NULL);
	}
	ca->stock = name;
	ca->r = r;
	ca->wid = WIDGET(obj);
	ca->c = &drv->cursors[name];
	TAILQ_INSERT_TAIL(&win->cursorAreas, ca, cursorAreas);
	return (ca);
}

/*
 * Remove a cursor-change area and release the associated cursor (unless it
 * is a stock Agar cursor).
 */
void
AG_UnmapCursor(void *obj, AG_CursorArea *ca)
{
	AG_Window *win = WIDGET(obj)->window;
	AG_Driver *drv = WIDGET(win)->drv;

	if (win == NULL) {
		return;
	}
	if (ca->c == drv->activeCursor) {
		if (ca->stock == -1) {
			/* XXX TODO it would be safer to defer this operation */
			AGDRIVER_CLASS(drv)->unsetCursor(drv);
			AG_CursorFree(drv, ca->c);
		}
		TAILQ_REMOVE(&win->cursorAreas, ca, cursorAreas);
		Free(ca);
	}
}

/* Destroy all cursors (or all cursors associated with a given widget). */
void
AG_UnmapAllCursors(AG_Window *win, void *wid)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_CursorArea *ca, *caNext;

	if (wid == NULL) {
		for (ca = TAILQ_FIRST(&win->cursorAreas);
		     ca != TAILQ_END(&win->cursorAreas);
		     ca = caNext) {
			caNext = TAILQ_NEXT(ca, cursorAreas);
			if (ca->stock == -1) {
				AG_CursorFree(drv, ca->c);
			}
			Free(ca);
		}
		TAILQ_INIT(&win->cursorAreas);
	} else {
scan:
		TAILQ_FOREACH(ca, &win->cursorAreas, cursorAreas) {
			if (ca->wid != wid) {
				continue;
			}
			if (ca->stock == -1) {
				AG_CursorFree(drv, ca->c);
			}
			TAILQ_REMOVE(&win->cursorAreas, ca, cursorAreas);
			Free(ca);
			goto scan;
		}
	}
}

#ifdef AG_LEGACY
/* Pre-1.4 */
AG_Window *
AG_FindWindow(const char *name)
{
	AG_Driver *drv;
	AG_Window *win;

	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if ((win = AG_ObjectFindS(drv, name)) != NULL)
			return (win);
	}
	return (NULL);
}
void
AG_ViewAttach(struct ag_window *pWin)
{
	AG_ObjectAttach(agView, pWin);
}
void
AG_ViewDetach(AG_Window *win)
{
	AG_ObjectDetach(win);
}

void
AG_WindowSetVisibility(AG_Window *win, int flag)
{
	AG_ObjectLock(win);
	if (win->visible) {
		AG_WindowHide(win);
	} else {
		AG_WindowShow(win);
	}
	AG_ObjectUnlock(win);
}
#endif /* AG_LEGACY */

AG_WidgetClass agWindowClass = {
	{
		"Agar(Widget:Window)",
		sizeof(AG_Window),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

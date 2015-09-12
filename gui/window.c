/*
 * Copyright (c) 2001-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/titlebar.h>
#include <agar/gui/icon.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>
#include <agar/gui/cursors.h>
#include <agar/gui/label.h>

#include <agar/config/ag_debug_gui.h>

#include <string.h>
#include <stdarg.h>

static void OnShow(AG_Event *);
static void OnHide(AG_Event *);
static void OnFocusGain(AG_Event *);
static void OnFocusLoss(AG_Event *);

int agWindowSideBorderDefault = 0;
int agWindowBotBorderDefault = 8;

/* Protected by agDrivers VFS lock */
AG_WindowQ agWindowDetachQ;			/* Windows to detach */
AG_WindowQ agWindowShowQ;			/* Windows to show */
AG_WindowQ agWindowHideQ;			/* Windows to hide */
AG_Window *agWindowToFocus = NULL;		/* Window to focus */
AG_Window *agWindowFocused = NULL;		/* Window holding focus */

/* Map enum ag_window_wm_type to EWMH window type */
const char *agWindowWmTypeNames[] = {
	"_NET_WM_WINDOW_TYPE_NORMAL",
	"_NET_WM_WINDOW_TYPE_DESKTOP",
	"_NET_WM_WINDOW_TYPE_DOCK",
	"_NET_WM_WINDOW_TYPE_TOOLBAR",
	"_NET_WM_WINDOW_TYPE_MENU",
	"_NET_WM_WINDOW_TYPE_UTILITY",
	"_NET_WM_WINDOW_TYPE_SPLASH",
	"_NET_WM_WINDOW_TYPE_DIALOG",
	"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
	"_NET_WM_WINDOW_TYPE_POPUP_MENU",
	"_NET_WM_WINDOW_TYPE_TOOLTIP",
	"_NET_WM_WINDOW_TYPE_NOTIFICATION",
	"_NET_WM_WINDOW_TYPE_COMBO",
	"_NET_WM_WINDOW_TYPE_DND"
};

void
AG_InitWindowSystem(void)
{
	TAILQ_INIT(&agWindowDetachQ);
	TAILQ_INIT(&agWindowShowQ);
	TAILQ_INIT(&agWindowHideQ);
	agWindowToFocus = NULL;
	agWindowFocused = NULL;
}

void
AG_DestroyWindowSystem(void)
{
}

static void
InitWindow(AG_Window *win, Uint flags)
{
	AG_ObjectInit(win, &agWindowClass);
	AG_ObjectSetNameS(win, "generic");

	/* We use a specific naming system. */
	OBJECT(win)->flags &= ~(AG_OBJECT_NAME_ONATTACH);

	WIDGET(win)->window = win;
	win->flags |= flags;

	if (win->flags & AG_WINDOW_MODAL)
		win->flags |= AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOMINIMIZE;
	if (win->flags & AG_WINDOW_NORESIZE)
		win->flags |= AG_WINDOW_NOMAXIMIZE;
#ifdef AG_LEGACY
	if (win->flags & AG_WINDOW_POPUP) { win->wmType = AG_WINDOW_WM_POPUP_MENU; }
	if (win->flags & AG_WINDOW_DIALOG) { win->wmType = AG_WINDOW_WM_DIALOG; }
#endif

	/* Default window "close" action should be detach/free. */
	AG_SetEvent(win, "window-close", AGWINDETACH(win));
}

/*
 * Create a generic window under a specific single-window driver. Return
 * a pointer to the newly-allocated window, or NULL on failure.
 */
AG_Window *
AG_WindowNewSw(void *pDrv, Uint flags)
{
	AG_Driver *drv = pDrv;
	AG_Window *win;

	if (!AG_OfClass(drv, "AG_Driver:AG_DriverSw:*")) {
		return (NULL);
	}
	
	if ((win = TryMalloc(sizeof(AG_Window))) == NULL) {
		return (NULL);
	}
	InitWindow(win, flags);
	AG_ObjectAttach(drv, win);
	
	if (!(win->flags & AG_WINDOW_NOTITLE)) {
		win->hMin += agTextFontHeight;
	}
	if (win->flags & AG_WINDOW_NOBORDERS) {
		win->wBorderSide = 0;
		win->wBorderBot = 0;
	}
	return (win);
}

/*
 * Create a generic window using the default window driver; return
 * a pointer to the newly-allocated window, or NULL on failure.
 *
 * - With single-window drivers, all Window objects are attached to
 *   the main Driver object.
 * - With multiple-window drivers: A parent Driver object is created
 *   for every Window object.
 */
AG_Window *
AG_WindowNew(Uint flags)
{
	AG_Driver *drv;
	AG_Window *win;

	if ((win = TryMalloc(sizeof(AG_Window))) == NULL) {
		return (NULL);
	}
	InitWindow(win, flags);

	switch (agDriverOps->wm) {
	case AG_WM_SINGLE:
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
		if ((drv = AG_DriverOpen(agDriverOps)) == NULL) {
			return (NULL);
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

/* Special implementation of AG_ObjectAttach() for AG_Window. */
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
		 * Initialize the built-in AG_Titlebar and dekstop AG_Icon now
		 * that we have an attached driver (we could not do this
		 * earlier because surface operations are involved).
		 */
		if (win->tbar == NULL && !(win->flags & AG_WINDOW_NOTITLE)) {
			Uint titlebarFlags = 0;

			if (win->flags & AG_WINDOW_NOCLOSE) { titlebarFlags |= AG_TITLEBAR_NO_CLOSE; }
			if (win->flags & AG_WINDOW_NOMINIMIZE) { titlebarFlags |= AG_TITLEBAR_NO_MINIMIZE; }
			if (win->flags & AG_WINDOW_NOMAXIMIZE) { titlebarFlags |= AG_TITLEBAR_NO_MAXIMIZE; }

			win->tbar = AG_TitlebarNew(win, titlebarFlags);
		}
		if (win->icon == NULL) {
			win->icon = AG_IconNew(NULL, 0);
		}
		WIDGET(win->icon)->drv = drv;
		WIDGET(win->icon)->drvOps = AGDRIVER_CLASS(drv);
		AG_IconSetSurfaceNODUP(win->icon, agIconWindow.s);
		AG_IconSetBackgroundFill(win->icon, 1, AGDRIVER_SW(drv)->bgColor);
		AG_SetStyle(win->icon, "font-size", "80%");
		AG_WidgetCompileStyle(win->icon);
	}

	if (win->flags & AG_WINDOW_FOCUSONATTACH)
		AG_WindowFocus(win);
}

/* Special implementation of AG_ObjectDetach() for AG_Window. */
static void
Detach(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv;
	AG_Window *other, *subwin;
	AG_Timer *to, *toNext;

#ifdef AG_DEBUG_GUI
	Debug(NULL, "AG_ObjectDetach(Window %s, \"%s\")\n", OBJECT(win)->name,
	    win->caption);
#endif

	AG_LockVFS(&agDrivers);

	/* Mark window detach in progress */
	win->flags |= AG_WINDOW_DETACHING;

	/* Cancel any running timer attached to the window. */
	AG_LockTiming();
	for (to = TAILQ_FIRST(&OBJECT(win)->timers);
	     to != TAILQ_END(&OBJECT(win)->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, timers);
		AG_DelTimer(win, to);
	}
	AG_UnlockTiming();

	/* Implicitely detach window dependencies. */
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(other, drv) {
			if (other == win) {
				continue;
			}
			AG_ObjectLock(other);
			TAILQ_FOREACH(subwin, &other->subwins, swins) {
				if (subwin == win)
					break;
			}
			if (subwin != NULL) {
				TAILQ_REMOVE(&other->subwins, subwin, swins);
			}
			if (other->pinnedTo == win) {
				AG_WindowUnpin(other);
			}
			if (other->parent == win ||
			    other->transientFor == win) {
				AG_ObjectDetach(other);
			}
			AG_ObjectUnlock(other);
		}
	}

 	/*
	 * For the AG_ObjectDetach() call to be safe in (free-threaded) event
	 * context, we must defer the actual window hide / detach operation
	 * until the end of the current event processing cycle.
	 */
	TAILQ_INSERT_TAIL(&agWindowDetachQ, win, detach);

	/* Queued Show/Hide operations would be redundant. */
	TAILQ_FOREACH(other, &agWindowHideQ, visibility) {
		if (other == win) {
			TAILQ_REMOVE(&agWindowHideQ, win, visibility);
			break;
		}
	}
	TAILQ_FOREACH(other, &agWindowShowQ, visibility) {
		if (other == win) {
			TAILQ_REMOVE(&agWindowShowQ, win, visibility);
			break;
		}
	}
	
	AG_UnlockVFS(&agDrivers);
}

/* Timer callback for fade-in/fade-out */
static Uint32
FadeTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Window *win = AG_SELF();
	int dir = AG_INT(1);

	if (dir == 1) {					/* Fade in */
		if (win->fadeOpacity < 1.0) {
			win->fadeOpacity += win->fadeInIncr;
			AG_WindowSetOpacity(win, win->fadeOpacity);
			return (to->ival);
		} else {
			return (0);
		}
	} else {					/* Fade out */
		if (win->fadeOpacity > 0.0) {
			win->fadeOpacity -= win->fadeOutIncr;
			AG_WindowSetOpacity(win, win->fadeOpacity);
			return (to->ival);
		} else {
			AG_WindowSetOpacity(win, 1.0);

			/* Defer operation until AG_WindowProcessQueued(). */
			AG_LockVFS(&agDrivers);
			TAILQ_INSERT_TAIL(&agWindowHideQ, win, visibility);
			AG_UnlockVFS(&agDrivers);
			return (0);
		}
	}
}

static void
Init(void *obj)
{
	AG_Window *win = obj;
	AG_Event *ev;
	int i;

	win->wmType = AG_WINDOW_WM_NORMAL;
	win->flags = AG_WINDOW_NOCURSORCHG;
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
	win->transientFor = NULL;
	win->pinnedTo = NULL;
	win->widExclMotion = NULL;
	win->fadeInTime = 0.06f;
	win->fadeInIncr = 0.2f;
	win->fadeOutTime = 0.06f;
	win->fadeOutIncr = 0.2f;
	win->fadeOpacity = 1.0f;
	win->zoom = AG_ZOOM_DEFAULT;
	TAILQ_INIT(&win->subwins);
	TAILQ_INIT(&win->cursorAreas);
	for (i = 0; i < 5; i++)
		win->caResize[i] = NULL;

	AG_InitTimer(&win->fadeTo, "fade", 0);

	AG_SetEvent(win, "window-gainfocus", OnFocusGain, NULL);
	AG_SetEvent(win, "window-lostfocus", OnFocusLoss, NULL);

	/*
	 * We wish to forward incoming `widget-shown', `widget-hidden' and
	 * `detached' events to all attached child widgets.
	 */
	ev = AG_SetEvent(win, "widget-shown", OnShow, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "widget-hidden", OnHide, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "detached", NULL, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;

	/* Custom attach/detach hooks are needed by the window stack. */
	AG_ObjectSetAttachFn(win, Attach, NULL);
	AG_ObjectSetDetachFn(win, Detach, NULL);
	
	/* Set the inheritable style defaults. */
	AG_SetString(win, "font-family", OBJECT(agDefaultFont)->name);
	AG_SetString(win, "font-size", AG_Printf("%.02fpts", agDefaultFont->spec.size));
	AG_SetString(win, "font-weight", (agDefaultFont->flags & AG_FONT_BOLD) ? "bold" : "normal");
	AG_SetString(win, "font-style", (agDefaultFont->flags & AG_FONT_ITALIC) ? "italic" : "normal");
	WIDGET(win)->font = agDefaultFont;
	WIDGET(win)->pal = agDefaultPalette;

#ifdef AG_DEBUG
	AG_BindUint(win, "flags", &win->flags);
	AG_BindString(win, "caption", win->caption, sizeof(win->caption));
	AG_BindInt(win, "visible", &win->visible);
	AG_BindInt(win, "spacing", &win->spacing);
	AG_BindInt(win, "wReq", &win->wReq);
	AG_BindInt(win, "hReq", &win->hReq);
	AG_BindInt(win, "wMin", &win->wMin);
	AG_BindInt(win, "hMin", &win->hMin);
	AG_BindInt(win, "r.x", &win->r.x);
	AG_BindInt(win, "r.y", &win->r.y);
	AG_BindInt(win, "r.w", &win->r.w);
	AG_BindInt(win, "r.h", &win->r.h);
#endif /* AG_DEBUG */
}

/*
 * Make a window a logical child of the specified window. If the logical
 * parent window is detached, its child windows will be automatically
 * detached along with it.
 */
void
AG_WindowAttach(AG_Window *winParent, AG_Window *winChld)
{
	if (winParent == NULL)
		return;

	AG_LockVFS(&agDrivers);
	AG_ObjectLock(winParent);
	AG_ObjectLock(winChld);
	if (winChld->parent != NULL) {
		if (winChld->parent == winParent) {
			goto out;
		}
		AG_WindowDetach(winChld->parent, winChld);
	}
	winChld->parent = winParent;
	winChld->zoom = winParent->zoom;
	AG_WidgetCopyStyle(winChld, winParent);
	TAILQ_INSERT_HEAD(&winParent->subwins, winChld, swins);
out:
	AG_ObjectUnlock(winChld);
	AG_ObjectUnlock(winParent);
	AG_UnlockVFS(&agDrivers);
}

/* Detach a window from its logical parent. */
void
AG_WindowDetach(AG_Window *winParent, AG_Window *winChld)
{
	if (winParent == NULL)
		return;

	AG_LockVFS(&agDrivers);
	AG_ObjectLock(winParent);
	AG_ObjectLock(winChld);

#ifdef AG_DEBUG
	if (winChld->parent != winParent)
		AG_FatalError("Inconsistent AG_WindowDetach()");
#endif
	TAILQ_REMOVE(&winParent->subwins, winChld, swins);
	winChld->parent = NULL;

	AG_ObjectUnlock(winChld);
	AG_ObjectUnlock(winParent);
	AG_UnlockVFS(&agDrivers);
}

/*
 * Make a window a transient window of another window. The effect of
 * this setting is WM-dependent (see AG_Window(3) for details).
 */
void
AG_WindowMakeTransient(AG_Window *winParent, AG_Window *winTrans)
{
	AG_Driver *drv = WIDGET(winTrans)->drv;
	
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(winTrans);

	if (winParent == NULL) {
		if (AGDRIVER_MULTIPLE(drv) &&
		    AGDRIVER_MW_CLASS(drv)->setTransientFor != NULL) {
			AGDRIVER_MW_CLASS(drv)->setTransientFor(NULL, winTrans);
		}
		winTrans->transientFor = NULL;
	} else {
		AG_ObjectLock(winParent);
		if (AGDRIVER_MULTIPLE(drv) &&
		    AGDRIVER_MW_CLASS(drv)->setTransientFor != NULL) {
			AGDRIVER_MW_CLASS(drv)->setTransientFor(winParent, winTrans);
		}
		winTrans->transientFor = winParent;
		AG_ObjectUnlock(winParent);
	}

	AG_ObjectUnlock(winTrans);
	AG_UnlockVFS(&agDrivers);
}

/* Pin a window against another. */
void
AG_WindowPin(AG_Window *winParent, AG_Window *win)
{
#ifdef AG_DEBUG
	if (win == winParent) { AG_FatalError("AG_WindowPin"); }
#endif
	AG_ObjectLock(win);
	win->pinnedTo = winParent;
	AG_ObjectUnlock(win);
}

/* Unpin a window. */
void
AG_WindowUnpin(AG_Window *win)
{
	AG_ObjectLock(win);
	win->pinnedTo = NULL;
	AG_ObjectUnlock(win);
}

static void
MovePinnedRecursive(AG_Window *win, AG_Window *winParent, int xRel, int yRel)
{
	AG_Rect r;
	AG_Window *winOther;
	AG_Driver *drv;

	r.x = WIDGET(win)->x + xRel;
	r.y = WIDGET(win)->y + yRel;
	r.w = WIDGET(win)->w;
	r.h = WIDGET(win)->h;
	AG_WindowSetGeometryRect(win, r, 0);
	
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(winOther, drv) {
			if (winOther->pinnedTo == win)
				MovePinnedRecursive(winOther, win, xRel, yRel);
		}
	}
}

void
AG_WindowMovePinned(AG_Window *winParent, int xRel, int yRel)
{
	AG_Driver *drv;
	AG_Window *win;

	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(win, drv) {
			if (win->pinnedTo == winParent)
				MovePinnedRecursive(win, winParent, xRel, yRel);
		}
	}
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
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
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
	AG_Rect r;
	int hBar = (win->tbar != NULL) ? HEIGHT(win->tbar) : 0;

	if (UpdateNeeded(WIDGET(win)))
		AG_WindowUpdate(win);

	/* Render window background. */
	if (!(win->flags & AG_WINDOW_NOBACKGROUND) &&
	    !(WIDGET(win)->drv->flags & AG_DRIVER_WINDOW_BG)) {
		AG_DrawRect(win,
		    AG_RECT(0, hBar-1, WIDTH(win), HEIGHT(win)-hBar),
		    WCOLOR(win,0));
	}

	/* Render decorative borders. */
	if (win->wBorderBot > 0) {
		r.x = 0;
		r.y = HEIGHT(win) - win->wBorderBot;
		r.h = win->wBorderBot;
		if (!(win->flags & AG_WINDOW_NORESIZE) &&
		    WIDTH(win) > win->wResizeCtrl*2) {
			r.w = win->wResizeCtrl;
			AG_DrawBox(win, r,
			    AG_WindowSelectedWM(win,AG_WINOP_LRESIZE) ? -1 : 1,
			    WCOLOR(win,BORDER_COLOR));
			r.x = WIDTH(win) - win->wResizeCtrl;
			AG_DrawBox(win, r,
			    AG_WindowSelectedWM(win,AG_WINOP_RRESIZE) ? -1 : 1,
			    WCOLOR(win,BORDER_COLOR));
			r.x = win->wResizeCtrl;
			r.w = WIDTH(win) - win->wResizeCtrl*2;
			AG_DrawBox(win, r,
			    AG_WindowSelectedWM(win,AG_WINOP_HRESIZE) ? -1 : 1,
			    WCOLOR(win,BORDER_COLOR));
		} else {
			r.w = WIDTH(win);
			AG_DrawBox(win, r, 1, WCOLOR(win,BORDER_COLOR));
		}
	}
	if (win->wBorderSide > 0) {
		r.x = 0;
		r.y = hBar;
		r.w = win->wBorderSide;
		r.h = HEIGHT(win) - win->wBorderBot - hBar;
		AG_DrawBox(win, r, 1, WCOLOR(win,BORDER_COLOR));
		r.x = WIDTH(win) - win->wBorderSide;
		AG_DrawBox(win, r, 1, WCOLOR(win,BORDER_COLOR));
	}

	if (!(win->flags & AG_WINDOW_NOCLIPPING)) {
		AG_PushClipRect(win, win->r);
	}
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
		AG_WidgetDraw(chld);
	}
	if (!(win->flags & AG_WINDOW_NOCLIPPING))
		AG_PopClipRect(win);
}

static void
OnShow(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverMw *dmw = AGDRIVER_MW(drv);
	AG_SizeReq r;
	AG_SizeAlloc a;
	int xPref, yPref;
	Uint mwFlags = 0;
	AG_Variable V;

	win->visible = 1;
	WIDGET(win)->flags |= AG_WIDGET_VISIBLE;

	/* Compile the globally inheritable style attributes. */
	AG_WidgetCompileStyle(win);

	if (WIDGET(win)->x == -1 && WIDGET(win)->y == -1) {
		/*
		 * No explicit window geometry was provided; compute a
		 * default size from the widget sizeReq() operations,
		 * and apply initial window alignment settings.
		 */
		AG_WidgetSizeReq(win, &r);
		if (AGDRIVER_SINGLE(drv)) {
			AG_WM_GetPrefPosition(win, &xPref, &yPref, r.w, r.h);
			a.x = xPref;
			a.y = yPref;
		} else {
			a.x = 0;
			a.y = 0;
		}
		a.w = r.w;
		a.h = r.h;

		if (win->alignment != AG_WINDOW_ALIGNMENT_NONE) {
			if (!AGDRIVER_SINGLE(drv))
				AG_WindowComputeAlignment(win, &a);
		} else {
			if (dmw->flags & AG_DRIVER_MW_ANYPOS_AVAIL) {
				/* Let the WM choose a default position */
				mwFlags |= AG_DRIVER_MW_ANYPOS;
			} else {
				win->alignment = AG_WINDOW_MC;
				AG_WindowComputeAlignment(win, &a);
			}
		}
	} else {
		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDTH(win);
		a.h = HEIGHT(win);
	}
	AG_WidgetSizeAlloc(win, &a);
	
	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_SINGLE:
		if (win->flags & AG_WINDOW_MODAL) {	/* Per-driver stack */
			AG_InitPointer(&V, win);
			AG_ListAppend(AGDRIVER_SW(drv)->Lmodal, &V);
		}
		AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);
		break;
	case AG_WM_MULTIPLE:
		if (win->flags & AG_WINDOW_MODAL) {	/* Global stack */
			AG_InitPointer(&V, win);
			AG_ListAppend(agModalWindows, &V);
		}
		/* We expect the driver will call AG_WidgetUpdateCoords(). */
		if (!(dmw->flags & AG_DRIVER_MW_OPEN)) {
			if (AGDRIVER_MW_CLASS(drv)->openWindow(win,
			    AG_RECT(a.x, a.y, a.w, a.h), 0,
			    mwFlags) == -1) {
				AG_FatalError(NULL);
			}
			dmw->flags |= AG_DRIVER_MW_OPEN;
		}
		if (win->flags & AG_WINDOW_FADEIN) {
			AG_WindowSetOpacity(win, 0.0);
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
	
	/* Notify widgets that the window is now visible. */
	AG_PostEvent(NULL, win, "window-shown", NULL);

	/* Implicit focus change. */
	if (!(win->flags & AG_WINDOW_DENYFOCUS))
		agWindowToFocus = win;

	/* Mark for redraw */
	win->dirty = 1;

	/* We can now allow cursor changes. */
	win->flags &= ~(AG_WINDOW_NOCURSORCHG);
	
	if (win->flags & AG_WINDOW_FADEIN) {
		AG_AddTimer(win, &win->fadeTo,
		    (Uint32)((win->fadeInTime*1000.0)/(1.0/win->fadeInIncr)),
		    FadeTimeout, "%i", 1);
	}
}

static void
OnHide(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverSw *dsw;
	int i;

	win->visible = 0;
	WIDGET(win)->flags &= ~(AG_WIDGET_VISIBLE);
	win->dirty = 0;
	win->flags |= AG_WINDOW_NOCURSORCHG;
	
	/* Cancel focus state or any focus change requests. */
	if (win == agWindowToFocus)
		agWindowToFocus = NULL;

	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_SINGLE:
		dsw = (AG_DriverSw *)drv;
#ifdef AG_DEBUG	
		if (OBJECT(drv)->parent == NULL) 
			AG_FatalError("NULL parent");
#endif
		if (win == agWindowFocused) {
			AG_Window *wOther;

			AG_PostEvent(NULL, win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		
			AG_FOREACH_WINDOW_REVERSE(wOther, dsw) {
				if (wOther->visible &&
				  !(wOther->flags & AG_WINDOW_DENYFOCUS))
					break;
			}
			if (wOther != NULL)
				agWindowToFocus = wOther;
		}

		if (win->flags & AG_WINDOW_MODAL) {
			for (i = 0; i < dsw->Lmodal->n; i++) {
				if (dsw->Lmodal->v[i].data.p == win)
					break;
			}
			if (i < dsw->Lmodal->n)
				AG_ListRemove(dsw->Lmodal, i);
		}
		if (AGDRIVER_CLASS(drv)->type == AG_FRAMEBUFFER) {
			AG_DrawRectFilled(win,
			    AG_RECT(0,0, WIDTH(win), HEIGHT(win)), dsw->bgColor);
			if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
				AGDRIVER_CLASS(drv)->updateRegion(drv,
				    AG_RECT(WIDGET(win)->x, WIDGET(win)->y,
				            WIDTH(win), HEIGHT(win)));
		}
		break;
	case AG_WM_MULTIPLE:
		if (win == agWindowFocused) {
			AG_PostEvent(NULL, win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		if (win->flags & AG_WINDOW_MODAL) {
			for (i = 0; i < agModalWindows->n; i++) {
				if (agModalWindows->v[i].data.p == win)
					break;
			}
			if (i < agModalWindows->n)
				AG_ListRemove(agModalWindows, i);
		}
		if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
			if (AGDRIVER_MW_CLASS(drv)->unmapWindow(win) == -1)
				AG_FatalError(NULL);
		}
		break;
	}
	
	/* Notify widgets that the window is now hidden. */
	AG_PostEvent(NULL, win, "window-hidden", NULL);
}

static void
WidgetGainFocus(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
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

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_ObjectLock(chld);
		WidgetLostFocus(chld);
		AG_ObjectUnlock(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(NULL, wid, "widget-lostfocus", NULL);
}

static void
OnFocusGain(AG_Event *event)
{
	AG_Window *win = AG_SELF();

/*	Verbose("%s (\"%s\"): Gained Focus\n", OBJECT(win)->name, win->caption); */
	WidgetGainFocus(WIDGET(win));
}

static void
OnFocusLoss(AG_Event *event)
{
	AG_Window *win = AG_SELF();

/*	Verbose("%s (\"%s\"): Lost Focus\n", OBJECT(win)->name, win->caption); */
	WidgetLostFocus(WIDGET(win));
}

/* Make a window visible to the user. */
void
AG_WindowShow(AG_Window *win)
{
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);
	if (!win->visible) {
#ifdef AG_THREADS
		if (!AG_ThreadEqual(AG_ThreadSelf(), agEventThread)) {
			AG_LockVFS(&agDrivers);
			TAILQ_INSERT_TAIL(&agWindowShowQ, win, visibility);
			AG_UnlockVFS(&agDrivers);
		} else
#endif
		{
			AG_PostEvent(NULL, win, "widget-shown", NULL);
		}
	}
	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Make a window invisible to the user. */
void
AG_WindowHide(AG_Window *win)
{
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);

	if (!win->visible) {
		goto out;
	}
	if ((win->flags & AG_WINDOW_FADEOUT) &&
	   !(win->flags & AG_WINDOW_DETACHING)) {
		AG_AddTimer(win, &win->fadeTo,
		    (Uint32)((win->fadeOutTime*1000.0)/(1.0/win->fadeOutIncr)),
		    FadeTimeout, "%i", -1);
	} else {
#ifdef AG_THREADS
		if (!AG_ThreadEqual(AG_ThreadSelf(), agEventThread)) {
			AG_LockVFS(&agDrivers);
			TAILQ_INSERT_TAIL(&agWindowHideQ, win, visibility);
			AG_UnlockVFS(&agDrivers);
		} else
#endif
		{
			AG_PostEvent(NULL, win, "widget-hidden", NULL);
			win->visible = 0;
			WIDGET(win)->flags &= ~(AG_WIDGET_VISIBLE);
		}
	}
out:
	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/*
 * Render all windows that need to be redrawn. This is typically invoked
 * by the main event loop, once events have been processed.
 */ 
void
AG_WindowDrawQueued(void)
{
	AG_Driver *drv;
	AG_Window *win;

	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		switch (AGDRIVER_CLASS(drv)->wm) {
		case AG_WM_MULTIPLE:
			if ((win = AGDRIVER_MW(drv)->win) != NULL) {
				AG_ObjectLock(win);
				if (win->visible && win->dirty) {
					AG_BeginRendering(drv);
					AGDRIVER_CLASS(drv)->renderWindow(win);
					AG_EndRendering(drv);
					win->dirty = 0;
				}
				AG_ObjectUnlock(win);
			}
			break;
		case AG_WM_SINGLE:
			{
				AG_DriverSw *dsw = (AG_DriverSw *)drv;
				Uint32 t;

				t = AG_GetTicks();
				if ((t - dsw->rLast) < dsw->rNom) {
					AG_Delay(1);
					goto out;
				}
				dsw->rLast = t;
				
				AG_FOREACH_WINDOW(win, drv) {
					if (win->visible && win->dirty)
						break;
				}
				if (win != NULL ||
				    (dsw->flags & AG_DRIVER_SW_REDRAW)) {
					dsw->flags &= ~(AG_DRIVER_SW_REDRAW);
					AG_BeginRendering(drv);
					AG_FOREACH_WINDOW(win, drv) {
						AG_ObjectLock(win);
						AG_WindowDraw(win);
						AG_ObjectUnlock(win);
					}
					AG_EndRendering(drv);
				}
			}
			break;
		}
	}
out:
	AG_UnlockVFS(&agDrivers);
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

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
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
 * Give input focus to a window. The actual focus change will take effect at
 * the end of the current event cycle.
 */
void
AG_WindowFocus(AG_Window *win)
{
	AG_LockVFS(&agDrivers);

	if (win == NULL) {
		agWindowToFocus = NULL;
		goto out;
	}

	/*
	 * Avoid clobbering modal windows (single-window drivers are
	 * expected to perform this test internally).
	 */
	if (agDriverOps->wm == AG_WM_MULTIPLE) {
		if (agModalWindows->n > 0)
			goto out;
	}

	AG_ObjectLock(win);
	if (win->flags & AG_WINDOW_DENYFOCUS) {
		AG_ObjectUnlock(win);
		goto out;
	}
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
 * Focus the window at specified display coordinates x,y
 * (single-window drivers only).
 *
 * Returns 1 if the focus state has changed as a result.
 */
int
AG_WindowFocusAtPos(AG_DriverSw *dsw, int x, int y)
{
	AG_Window *win;

	AG_LockVFS(&agDrivers);

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
		AG_UnlockVFS(&agDrivers);
		return (1);
	}
	AG_UnlockVFS(&agDrivers);
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
		AGDRIVER_CLASS(drv)->fillRect(drv, r, AGDRIVER_SW(drv)->bgColor);
		if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
			AGDRIVER_CLASS(drv)->updateRegion(drv, r);
	}
	if (HEIGHT(win) < rPrev.h) {				/* H-resize */
		r.x = rPrev.x;
		r.y = WIDGET(win)->y + HEIGHT(win);
		r.w = rPrev.w;
		r.h = rPrev.h - HEIGHT(win);
			
		AGDRIVER_CLASS(drv)->fillRect(drv, r, AGDRIVER_SW(drv)->bgColor);
		if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
			AGDRIVER_CLASS(drv)->updateRegion(drv, r);
	}
}

/* Set the coordinates and geometry of a window. */
int
AG_WindowSetGeometryRect(AG_Window *win, AG_Rect r, int bounded)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);
	AG_SizeReq rWin;
	AG_SizeAlloc a;
	AG_Rect rPrev;
	int new;
	int nw, nh;
	int wMin, hMin;
	Uint wDisp = 0, hDisp = 0;

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

	if (WIDGET(win)->x == -1 ||
	    WIDGET(win)->y == -1) {
		if (AG_GetDisplaySize(drv, &wDisp, &hDisp) == -1) {
			wDisp = 0;
			hDisp = 0;
		}
		if (WIDGET(win)->x == -1)
			WIDGET(win)->x = wDisp/2 - nw/2;
		if (WIDGET(win)->y == -1)
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
	AG_WidgetSizeAlloc(win, &a);
	AG_WidgetUpdateCoords(win, a.x, a.y);

	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_SINGLE:
		if (dc->type == AG_FRAMEBUFFER && win->visible && !new) {
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

/*
 * Update the x,y of the a to produce the required alignment for the
 * window's current display.
 */
void
AG_WindowComputeAlignment(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	Uint wMax, hMax;
	int w = a->w;
	int h = a->h;

	if (AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax) == -1)
		return;

	switch (win->alignment) {
	case AG_WINDOW_TL:
		a->x = 0;
		a->y = 0;
		break;
	case AG_WINDOW_TC:
		a->x = wMax/2 - w/2;
		a->y = 0;
		break;
	case AG_WINDOW_TR:
		a->x = wMax - w;
		a->y = 0;
		break;
	case AG_WINDOW_ML:
		a->x = 0;
		a->y = hMax/2 - h/2;
		break;
	case AG_WINDOW_MR:
		a->x = wMax - w;
		a->y = hMax/2 - h/2;
		break;
	case AG_WINDOW_BL:
		a->x = 0;
		a->y = hMax - h;
		break;
	case AG_WINDOW_BC:
		a->x = wMax/2 - w/2;
		a->y = hMax - h;
		break;
	case AG_WINDOW_BR:
		a->x = wMax - w;
		a->y = hMax - h;
		break;
	case AG_WINDOW_MC:
	case AG_WINDOW_ALIGNMENT_NONE:
	default:
		a->x = wMax/2 - w/2;
		a->y = hMax/2 - h/2;
		break;
	}
	if (AGDRIVER_MULTIPLE(drv) &&
	    AGDRIVER_MW_CLASS(drv)->tweakAlignment != NULL)
		AGDRIVER_MW_CLASS(drv)->tweakAlignment(win, a, wMax, hMax);
}

/* Assign a window a specific alignment and size in pixels. */
int
AG_WindowSetGeometryAligned(AG_Window *win, enum ag_window_alignment alignment,
    int w, int h)
{
	AG_SizeAlloc a;
	int rv;

	AG_ObjectLock(win);
	win->alignment = alignment;
	a.x = 0;
	a.y = 0;
	a.w = (w != -1) ? w : WIDGET(win)->w;
	a.h = (h != -1) ? h : WIDGET(win)->h;
	AG_WindowComputeAlignment(win, &a);
	rv = AG_WindowSetGeometry(win, a.x, a.y, a.w, a.h);
	AG_ObjectUnlock(win);
	return (rv);
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
	if (AG_WindowRestoreGeometry(win) == 0) {
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
		win->dirty = 1;
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
			    AGDRIVER_SW(drv)->bgColor);
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

/* Timer for double click on minimized icon. */
static Uint32
IconDoubleClickTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Icon *icon = AG_SELF();

	icon->flags &= ~(AG_ICON_DBLCLICKED);
	return (0);
}

static void
IconButtonDown(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	AG_Window *win = AG_PTR(1);

	WIDGET(icon)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                       AG_WIDGET_UNFOCUSED_BUTTONUP;
	if (icon->flags & AG_ICON_DBLCLICKED) {
		AG_DelTimer(icon, &icon->toDblClick);
		AG_WindowUnminimize(win);
		AG_ObjectDetach(win->icon);
		AG_ObjectDetach(icon->wDND);
		icon->wDND = NULL;
		icon->flags &= (AG_ICON_DND|AG_ICON_DBLCLICKED);
	} else {
		icon->flags |= (AG_ICON_DND|AG_ICON_DBLCLICKED);
		AG_AddTimer(icon, &icon->toDblClick, agMouseDblclickDelay,
		    IconDoubleClickTimeout, NULL);
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

		AG_SetEvent(icon, "mouse-motion", IconMotion, NULL);
		AG_SetEvent(icon, "mouse-button-up", IconButtonUp, NULL);
		AG_SetEvent(icon, "mouse-button-down", IconButtonDown, "%p", win);

#if 0
		if (icon->xSaved != -1) {
			AG_WindowShow(wDND);
			AG_WindowSetGeometry(wDND, icon->xSaved, icon->ySaved,
			                     icon->wSaved, icon->hSaved);
		} else {
#endif
		AG_WindowSetPosition(wDND, AG_WINDOW_LOWER_LEFT, 1);
		AG_WindowShow(wDND);
		icon->xSaved = WIDGET(wDND)->x;
		icon->ySaved = WIDGET(wDND)->y;
		icon->wSaved = WIDTH(wDND);
		icon->hSaved = HEIGHT(wDND);
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

/* Close the actively focused window. */
void
AG_CloseFocusedWindow(void)
{
	AG_LockVFS(&agDrivers);
	if (agWindowFocused != NULL) {
		AG_PostEvent(NULL, agWindowFocused, "window-close", NULL);
	}
	AG_UnlockVFS(&agDrivers);
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
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
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
	AG_Rect r;

	if (win->wBorderSide > 0) {
		r.x = 0;
		r.y = 0;
		r.w = win->wBorderSide;
		r.h = a->h - win->wBorderBot;
		AG_SetStockCursor(win, &win->caResize[0], r, AG_HRESIZE_CURSOR);
		r.x = a->w - win->wBorderSide;
		AG_SetStockCursor(win, &win->caResize[4], r, AG_HRESIZE_CURSOR);
	}
	if (win->wBorderBot > 0) {
		r.x = 0;
		r.y = a->h - win->wBorderBot;
		r.w = win->wResizeCtrl;
		r.h = win->wBorderBot;
		AG_SetStockCursor(win, &win->caResize[1], r, AG_LRDIAG_CURSOR);
		r.x = win->wResizeCtrl;
		r.w = a->w - win->wResizeCtrl*2;
		AG_SetStockCursor(win, &win->caResize[2], r, AG_VRESIZE_CURSOR);
		r.x = a->w - win->wResizeCtrl;
		r.w = win->wResizeCtrl;
		AG_SetStockCursor(win, &win->caResize[3], r, AG_LLDIAG_CURSOR);
	}

	/* Calculate total space available for widgets. */
	wAvail = a->w - win->lPad - win->rPad - win->wBorderSide*2;
	hAvail = a->h - win->bPad - win->tPad - win->wBorderBot;

	/* Calculate the space occupied by non-fill widgets. */
	nWidgets = 0;
	totFixed = 0;
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
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
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
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

			chldNext = OBJECT_NEXT_CHILD(chld, ag_widget);
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
    int tiling)
{
	AG_ObjectLock(win);
	win->alignment = alignment;
	AG_SETFLAGS(win->flags, AG_WINDOW_TILING, tiling);
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
	char s[20], *c;

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
	    (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) &&
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

/* Commit any pending focus change. The agDrivers VFS must be locked. */
void
AG_WindowProcessFocusChange(void)
{
	AG_Driver *drv;
	
	switch (agDriverOps->wm) {
	case AG_WM_SINGLE:
		AG_WM_CommitWindowFocus(agWindowToFocus);
		break;
	case AG_WM_MULTIPLE:
		if ((drv = AGWIDGET(agWindowToFocus)->drv) != NULL) {
			AGDRIVER_MW_CLASS(drv)->raiseWindow(agWindowToFocus);
			AGDRIVER_MW_CLASS(drv)->setInputFocus(agWindowToFocus);
		}
		break;
	}
	agWindowToFocus = NULL;
}

/*
 * Make windows on the show queue visible.
 * The agDrivers VFS must be locked.
 */
void
AG_WindowProcessShowQueue(void)
{
	AG_Window *win;

	TAILQ_FOREACH(win, &agWindowShowQ, visibility) {
		AG_PostEvent(NULL, win, "widget-shown", NULL);
	}
	TAILQ_INIT(&agWindowShowQ);
}

/*
 * Make windows on the hide queue invisible.
 * The agDrivers VFS must be locked.
 */
void
AG_WindowProcessHideQueue(void)
{
	AG_Window *win;

	TAILQ_FOREACH(win, &agWindowHideQ, visibility) {
		AG_PostEvent(NULL, win, "widget-hidden", NULL);
	}
	TAILQ_INIT(&agWindowHideQ);
}

/*
 * Close and destroy windows on the detach queue.
 * The agDrivers VFS must be locked.
 */
void
AG_WindowProcessDetachQueue(void)
{
	AG_Window *win, *winNext;
	AG_Driver *drv;
	int closedMain = 0, nHidden = 0;

#ifdef AG_DEBUG_GUI
	Debug(NULL, "AG_WindowProcessDetachQueue() Begin\n");
#endif
	TAILQ_FOREACH(win, &agWindowDetachQ, detach) {
		if (!win->visible) {
			continue;
		}
#ifdef AG_DEBUG_GUI
		Debug(NULL, "Hiding: %s (\"%s\")\n", OBJECT(win)->name, win->caption);
#endif
		/*
		 * Note: `widget-hidden' event handlers may cause new windows
		 * to be added to agWindowDetachQ.
		 */
		AG_PostEvent(NULL, win, "widget-hidden", NULL);
		nHidden++;
	}
	if (nHidden > 0) {
		/*
		 * Windows were hidden - defer detach operation until the next
		 * event cycle, just in case the underlying WM cannot hide and
		 * unmap a window in the same event cycle.
		 */
#ifdef AG_DEBUG_GUI
		Debug(NULL, "Defer Detach (%d windows hidden)\n", nHidden);
#endif
		return;
	}

	for (win = TAILQ_FIRST(&agWindowDetachQ);
	     win != TAILQ_END(&agWindowDetachQ);
	     win = winNext) {
		winNext = TAILQ_NEXT(win, detach);
		drv = WIDGET(win)->drv;
		
#ifdef AG_DEBUG_GUI
		Debug(NULL, "Detach: %s (\"%s\")\n", OBJECT(win)->name, win->caption);
#endif
		/* Notify all widgets of the window detach. */
		AG_PostEvent(drv, win, "detached", NULL);

		/* Release the cursor areas and associated cursors. */
		AG_UnmapAllCursors(win, NULL);
	
		if (AGDRIVER_MULTIPLE(drv)) {
			if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
				AGDRIVER_MW_CLASS(drv)->closeWindow(win);
				AGDRIVER_MW(drv)->flags &= ~(AG_DRIVER_MW_OPEN);
			}
		} else {
			win->tbar = NULL;
			if (win->icon != NULL) {
				AG_ObjectDestroy(win->icon);
				win->icon = NULL;
			}
			AGDRIVER_SW(drv)->flags |= AG_DRIVER_SW_REDRAW;
		}

		/* Do a standard AG_ObjectDetach(). */
		AG_ObjectSetDetachFn(win, NULL, NULL);
		AG_ObjectDetach(win);

		if (AGDRIVER_MULTIPLE(drv)) {
			/* Destroy the AG_Driver object. */
			AG_UnlockVFS(&agDrivers);
			AG_DriverClose(drv);
			AG_LockVFS(&agDrivers);
		}
		if (win->flags & AG_WINDOW_MAIN) {
			closedMain++;
		}
		AG_PostEvent(NULL, win, "window-detached", NULL);
		AG_ObjectDestroy(win);
	}
	TAILQ_INIT(&agWindowDetachQ);
	
	/* Terminate if the last AG_WINDOW_MAIN window was closed. */
	if (closedMain > 0) {
		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_FOREACH_WINDOW(win, drv) {
				if (win->flags & AG_WINDOW_MAIN)
					break;
			}
			if (win != NULL)
				break;
		}
		if (drv == NULL) {
#ifdef AG_DEBUG_GUI
			Debug(NULL, "AG_WindowProcessDetachQueue() Exit Normally\n");
#endif
			AG_Terminate(0);
		}
#ifdef AG_DEBUG_GUI
		Debug(NULL, "AG_WindowProcessDetachQueue() End (MAIN window remains)\n");
#endif
	} else {
#ifdef AG_DEBUG_GUI
		Debug(NULL, "AG_WindowProcessDetachQueue() End\n");
#endif
	}
}

/*
 * Configure a new cursor-change area with a specified cursor. The provided
 * cursor will be freed automatically on window detach.
 */
AG_CursorArea *
AG_MapCursor(void *obj, AG_Rect r, AG_Cursor *c)
{
	AG_Widget *wid = obj;
	AG_Window *win = wid->window;
	AG_CursorArea *ca;

	if ((ca = TryMalloc(sizeof(AG_CursorArea))) == NULL) {
		return (NULL);
	}
	ca->stock = -1;
	ca->r = r;
	ca->c = c;
	ca->wid = wid;

	if (win != NULL) {
		TAILQ_INSERT_TAIL(&win->cursorAreas, ca, cursorAreas);
	} else {
		TAILQ_INSERT_TAIL(&wid->cursorAreas, ca, cursorAreas);
	}
	return (ca);
}

/* Configure a new cursor-change area with a stock Agar cursor. */
AG_CursorArea *
AG_MapStockCursor(void *obj, AG_Rect r, int name)
{
	AG_Widget *wid = obj;
	AG_Window *win = wid->window;
	AG_Cursor *ac;
	AG_CursorArea *ca;
	int i = 0;

	TAILQ_FOREACH(ac, &wid->drv->cursors, cursors) {
		if (i++ == name)
			break;
	}
	if (ac == NULL) {
		AG_SetError("No such cursor");
		return (NULL);
	}

	if ((ca = TryMalloc(sizeof(AG_CursorArea))) == NULL) {
		return (NULL);
	}
	ca->stock = name;
	ca->r = r;
	ca->wid = WIDGET(obj);

	if (win != NULL) {
		ca->c = ac;
		TAILQ_INSERT_TAIL(&win->cursorAreas, ca, cursorAreas);
	} else {
		/* Will resolve cursor name when widget is later attached. */
		ca->c = NULL;
		TAILQ_INSERT_TAIL(&wid->cursorAreas, ca, cursorAreas);
	}
	return (ca);
}

/*
 * Remove a cursor-change area and release the associated cursor (unless it
 * is a stock Agar cursor).
 */
void
AG_UnmapCursor(void *obj, AG_CursorArea *ca)
{
	AG_Widget *wid = obj;
	AG_Window *win = wid->window;

	if (win == NULL) {
		TAILQ_REMOVE(&wid->cursorAreas, ca, cursorAreas);
		free(ca);
	} else {
		AG_Driver *drv = WIDGET(win)->drv;

		if (ca->c == drv->activeCursor) {
			if (ca->stock == -1) {
				/* XXX TODO it would be safer to defer this operation */
				if (AGDRIVER_CLASS(drv)->unsetCursor != NULL) {
					AGDRIVER_CLASS(drv)->unsetCursor(drv);
				}
				AG_CursorFree(drv, ca->c);
			}
			TAILQ_REMOVE(&win->cursorAreas, ca, cursorAreas);
			free(ca);
		}
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
			free(ca);
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
			free(ca);
			goto scan;
		}
	}
}

/* Configure per-window opacity (for compositing window managers). */
int
AG_WindowSetOpacity(AG_Window *win, float f)
{
	AG_Driver *drv = OBJECT(win)->parent;
	int rv = -1;

	AG_ObjectLock(win);
	win->fadeOpacity = (f > 1.0) ? 1.0 : f;

	if (AGDRIVER_MULTIPLE(drv) &&
	    AGDRIVER_MW_CLASS(drv)->setOpacity != NULL)
		rv = AGDRIVER_MW_CLASS(drv)->setOpacity(win, win->fadeOpacity);
	
	/* TODO: support compositing under single-window drivers. */
	AG_ObjectUnlock(win);
	return (rv);
}

/* Configure fade-in/fade-out parameters. */
void
AG_WindowSetFadeIn(AG_Window *win, float fadeTime, float fadeIncr)
{
	AG_ObjectLock(win);
	win->fadeInTime = fadeTime;
	win->fadeInIncr = fadeIncr;
	AG_ObjectUnlock(win);
}
void
AG_WindowSetFadeOut(AG_Window *win, float fadeTime, float fadeIncr)
{
	AG_ObjectLock(win);
	win->fadeOutTime = fadeTime;
	win->fadeOutIncr = fadeIncr;
	AG_ObjectUnlock(win);
}

/* Set the window zoom level. The scales are defined in agZoomValues[]. */
void
AG_WindowSetZoom(AG_Window *win, int zoom)
{
	AG_Window *winChld;

	AG_ObjectLock(win);
	if (zoom < 0 || zoom >= AG_ZOOM_RANGE || zoom == win->zoom) {
		AG_ObjectUnlock(win);
		return;
	}
	AG_SetStyle(win, "font-size", AG_Printf("%.02f%%", agZoomValues[zoom]));
	AG_WindowUpdate(win);
	win->zoom = zoom;
	if (WIDGET(win)->drv != NULL) {
		AG_TextClearGlyphCache(WIDGET(win)->drv);
	}
	TAILQ_FOREACH(winChld, &win->subwins, swins) {
		AG_WindowSetZoom(winChld, zoom);
	}
	AG_ObjectUnlock(win);
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
	AG_ObjectAttach(agDriverSw, pWin);
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
int
AG_WindowIntersect(AG_DriverSw *drv, int x, int y)
{
	AG_Window *win;
	int rv = 0;

	AG_FOREACH_WINDOW(win, drv) {
		if (win->visible &&
		    AG_WidgetArea(win, x, y))
			rv++;
	}
	return (rv);
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

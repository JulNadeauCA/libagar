/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
#include <agar/gui/checkbox.h>
#include <agar/gui/scrollview.h>
#include <agar/gui/textbox.h>
#include <agar/gui/radio.h>
#include <agar/gui/numerical.h>
#include <agar/gui/separator.h>
#endif

#include <string.h>
#include <stdarg.h>

/* #define DEBUG_FOCUS */
/* #define DEBUG_REDRAW */
/* #define DEBUG_RAISE_LOWER */

int agWindowSideBorderDefault = 0;
int agWindowBotBorderDefault = 8;

/* Protected by agDrivers VFS lock */
AG_WindowQ agWindowDetachQ;			/* Windows to detach */
AG_WindowQ agWindowShowQ;			/* Windows to show */
AG_WindowQ agWindowHideQ;			/* Windows to hide */
AG_Window *agWindowToFocus = NULL;		/* Window to focus */
AG_Window *agWindowFocused = NULL;		/* Window holding focus */

#if defined(AG_DEBUG) && defined(AG_WIDGETS)
AG_Window *_Nullable agTargetWindow = NULL;     /* For GUI debugger */
#endif

#ifdef AG_WM_HINTS
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
	"_NET_WM_WINDOW_TYPE_DND",
	NULL
};
#endif /* AG_WM_HINTS */

const char *agWindowAlignmentNames[] = {
	N_("None"),
	N_("Top Left"),		/* TL */
	N_("Top Center"),	/* TC */
	N_("Top Right"),	/* TR */
	N_("Middle Left"),	/* ML */
	N_("Middle Center"),	/* MC */
	N_("Middle Right"),	/* MR */
	N_("Bottom Left"),	/* BL */
	N_("Bottom Center"),	/* BC */
	N_("Bottom Right"),	/* BR */
	NULL
};

static int agWindowIconCounter = 0;

#ifdef AG_WIDGETS
static int CreateMinimizedIcon(AG_Window *_Nonnull);
static void AttachTitlebarAndIcons(AG_Driver *_Nonnull, AG_Window *_Nonnull);
static void HideMinimizedIcon(AG_Window *_Nonnull);
#endif

/*
 * Initialize a new window.
 * Called internally by AG_WindowNew*().
 */
static __inline__ void
InitWindow(AG_Window *win, Uint flags)
{
	static Uint winNo = 0;

	AG_ObjectInit(win, &agWindowClass);
	AG_ObjectSetName(win, "win%u", winNo++);

	/* We use a specific naming system. */
	OBJECT(win)->flags &= ~(AG_OBJECT_NAME_ONATTACH);

	WIDGET(win)->window = win;
	win->flags |= flags;

	if (win->flags & AG_WINDOW_NORESIZE)
		win->flags |= AG_WINDOW_NOMAXIMIZE;
	/*
	 * Set the default "close" action to AGWINDETACH(), a canned
	 * event handler which calls AG_ObjectDetach(win).
	 */
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

	if (!AG_OBJECT_VALID(drv) || !AG_OfClass(drv, "AG_Driver:AG_DriverSw:*")) {
		AG_SetErrorS("Bad driver argument");
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
 * Create a new Agar window. On success return a pointer to the newly
 * allocated window. If not successful, fail and return NULL.
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

/*
 * Create a new, uniquely named Agar window (string name).
 * On success return a pointer to the new window.
 * If the named window exists, focus the existing window and return NULL.
 */
AG_Window *
AG_WindowNewNamedS(Uint flags, const char *name)
{
	AG_Window *win;

	AG_LockVFS(&agDrivers);
	if (AG_WindowFocusNamed(name)) {
		win = NULL;
		goto out;
	}
	if ((win = AG_WindowNew(flags)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_ObjectSetNameS(win, name);
	AG_SetEvent(win, "window-close", AGWINHIDE(win));
out:
	AG_UnlockVFS(&agDrivers);
	return (win);
}

/*
 * Create a new, uniquely named Agar window (format string name).
 * On success return a pointer to the new window.
 * If the named window exists, focus the existing window and return NULL.
 */
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
Attach(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Driver *drv = OBJECT(win)->parent;

	AG_OBJECT_ISA(drv, "AG_Driver:*");

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
	AG_PostEvent(win, "attached", "%p", drv);

#ifdef AG_WIDGETS
	if (AGDRIVER_SINGLE(drv)) {
		/*
		 * Initialize built-in titlebar and desktop icon now that
		 * we have an attached driver (we could not do this earlier
		 * because surface operations require a driver).
		 */
		AttachTitlebarAndIcons(drv, win);
	}
#endif
	if (win->flags & AG_WINDOW_FOCUSONATTACH)
		AG_WindowFocus(win);
}

#ifdef AG_WIDGETS
static void
AttachTitlebarAndIcons(AG_Driver *_Nonnull drv, AG_Window *_Nonnull win)
{
	AG_Icon *icon;

	if (win->tbar == NULL && !(win->flags & AG_WINDOW_NOTITLE)) {
		Uint titlebarFlags = 0;

		if (win->flags & AG_WINDOW_NOCLOSE) { titlebarFlags |= AG_TITLEBAR_NO_CLOSE; }
		if (win->flags & AG_WINDOW_NOMINIMIZE) { titlebarFlags |= AG_TITLEBAR_NO_MINIMIZE; }
		if (win->flags & AG_WINDOW_NOMAXIMIZE) { titlebarFlags |= AG_TITLEBAR_NO_MAXIMIZE; }

		win->tbar = AG_TitlebarNew(win, titlebarFlags);
	}
	if ((icon = win->icon) == NULL) {
		icon = win->icon = AG_IconNew(NULL, 0);
		AG_ObjectSetNameS(icon, "icon");
	}
	WIDGET(icon)->drv = drv;
	WIDGET(icon)->drvOps = AGDRIVER_CLASS(drv);
	AG_IconSetSurface(icon, agIconWindow.s);
	AG_IconSetBackgroundFill(icon, 1, &AGDRIVER_SW(drv)->bgColor);
	AG_SetStyle(icon, "font-size", "80%");
	AG_WidgetCompileStyle(icon);
}
#endif /* AG_WIDGETS */

/* Special implementation of AG_ObjectDetach() for AG_Window. */
static void
Detach(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Driver *drv;
	AG_Window *other, *subwin;
	AG_Timer *to, *toNext;

	AG_LockVFS(&agDrivers);

	/* Mark window detach in progress */
	win->flags |= AG_WINDOW_DETACHING;

	/* Cancel any running timer attached to the window. */
	AG_LockTiming();
	for (to = TAILQ_FIRST(&OBJECT(win)->timers);
	     to != TAILQ_END(&OBJECT(win)->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, pvt.timers);
		AG_DelTimer(win, to);
	}
	AG_UnlockTiming();

	/* Implicitely detach window dependencies. */
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_OBJECT_ISA(drv, "AG_Driver:*");

		AG_FOREACH_WINDOW(other, drv) {
			if (other == win) {
				continue;
			}
			AG_OBJECT_ISA(other, "AG_Widget:AG_Window:*");
			AG_ObjectLock(other);
			TAILQ_FOREACH(subwin, &other->pvt.subwins, pvt.swins) {
				if (subwin == win)
					break;
			}
			if (subwin) {
				TAILQ_REMOVE(&other->pvt.subwins, subwin, pvt.swins);
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
	TAILQ_INSERT_TAIL(&agWindowDetachQ, win, pvt.detach);

	/* Queued Show/Hide operations would be redundant. */
	TAILQ_FOREACH(other, &agWindowHideQ, pvt.visibility) {
		if (other == win) {
			TAILQ_REMOVE(&agWindowHideQ, win, pvt.visibility);
			break;
		}
	}
	TAILQ_FOREACH(other, &agWindowShowQ, pvt.visibility) {
		if (other == win) {
			TAILQ_REMOVE(&agWindowShowQ, win, pvt.visibility);
			break;
		}
	}
	
	AG_UnlockVFS(&agDrivers);
}

static Uint32
FadeTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	int dir = AG_INT(1);

	if (dir == 1) {					/* Fade in */
		if (win->pvt.fadeOpacity < 1.0f) {
			win->pvt.fadeOpacity += win->pvt.fadeInIncr;
			AG_WindowSetOpacity(win, win->pvt.fadeOpacity);
			return (to->ival);
		} else {
			return (0);
		}
	} else {					/* Fade out */
		if (win->pvt.fadeOpacity > 0.0f) {
			win->pvt.fadeOpacity -= win->pvt.fadeOutIncr;
			AG_WindowSetOpacity(win, win->pvt.fadeOpacity);
			return (to->ival);
		} else {
			AG_WindowSetOpacity(win, 1.0f);

			/* Defer operation until AG_WindowProcessQueued(). */
			AG_LockVFS(&agDrivers);
			TAILQ_INSERT_TAIL(&agWindowHideQ, win, pvt.visibility);
			AG_UnlockVFS(&agDrivers);
			return (0);
		}
	}
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

	AG_OBJECT_ISA(winParent, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(winChld, "AG_Widget:AG_Window:*");

	AG_LockVFS(&agDrivers);
	AG_ObjectLock(winParent);
	AG_ObjectLock(winChld);
	if (winChld->parent) {
		if (winChld->parent == winParent) {
			goto out;
		}
		AG_WindowDetach(winChld->parent, winChld);
	}
	winChld->parent = winParent;
	winChld->zoom = winParent->zoom;
	AG_WidgetCopyStyle(winChld, winParent);
	TAILQ_INSERT_HEAD(&winParent->pvt.subwins, winChld, pvt.swins);
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

	AG_OBJECT_ISA(winParent, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(winChld, "AG_Widget:AG_Window:*");

	AG_LockVFS(&agDrivers);
	AG_ObjectLock(winParent);
	AG_ObjectLock(winChld);

#ifdef AG_DEBUG
	if (winChld->parent != winParent)
		AG_FatalError("Inconsistent AG_WindowDetach()");
#endif
	TAILQ_REMOVE(&winParent->pvt.subwins, winChld, pvt.swins);
	winChld->parent = NULL;

	AG_ObjectUnlock(winChld);
	AG_ObjectUnlock(winParent);
	AG_UnlockVFS(&agDrivers);
}

/*
 * Make a window a transient window for another window. The effect of
 * this setting is WM-dependent (see AG_Window(3) for details).
 */
void
AG_WindowMakeTransient(AG_Window *forParent, AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);

	if (forParent == NULL) {
		if (drv && AGDRIVER_MULTIPLE(drv) &&
		    AGDRIVER_MW_CLASS(drv)->setTransientFor != NULL) {
			AGDRIVER_MW_CLASS(drv)->setTransientFor(win, NULL);
		}
		win->transientFor = NULL;
	} else {
		AG_ObjectLock(forParent);
		if (drv && AGDRIVER_MULTIPLE(drv) &&
		    AGDRIVER_MW_CLASS(drv)->setTransientFor != NULL) {
			AGDRIVER_MW_CLASS(drv)->setTransientFor(win, forParent);
		}
		win->transientFor = forParent;
		AG_ObjectUnlock(forParent);
	}

	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Pin a window against another. */
void
AG_WindowPin(AG_Window *winParent, AG_Window *win)
{
#ifdef AG_DEBUG
	if (win == winParent) { AG_FatalError("AG_WindowPin"); }
#endif
	AG_OBJECT_ISA(winParent, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);
	win->pinnedTo = winParent;
	AG_ObjectUnlock(win);
}

/* Unpin a window. */
void
AG_WindowUnpin(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);
	win->pinnedTo = NULL;
	AG_ObjectUnlock(win);
}

static void
MovePinnedRecursive(AG_Window *_Nonnull win, int xRel, int yRel)
{
	AG_Rect r;
	AG_Window *winOther;
	AG_Driver *drv;

	r.x = WIDGET(win)->x + xRel;
	r.y = WIDGET(win)->y + yRel;
	r.w = WIDGET(win)->w;
	r.h = WIDGET(win)->h;
	AG_WindowSetGeometryRect(win, &r, 0);
	
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(winOther, drv) {
			if (winOther->pinnedTo == win)
				MovePinnedRecursive(winOther, xRel,yRel);
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
				MovePinnedRecursive(win, xRel, yRel);
		}
	}
	AG_UnlockVFS(&agDrivers);
}

/*
 * Test whether a widget (or one of its children) is requesting a
 * geometry update.
 */
static int _Pure_Attribute
UpdateNeeded(AG_Widget *_Nonnull wid)
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

/*
 * Test whether a window is currently selected for a given WM operation.
 * The agDrivers VFS must be locked.
 */
static __inline__ int _Pure_Attribute
Selected_WM_Op(AG_Window *_Nonnull win, enum ag_wm_operation op)
{
	AG_Driver *drv = WIDGET(win)->drv;

	return (AGDRIVER_SINGLE(drv) &&
	        AGDRIVER_SW(drv)->winSelected == win &&
	        AGDRIVER_SW(drv)->winop == op);
}

#define DRAW_BORDER(winop,c)			\
	if (Selected_WM_Op(win, winop)) {	\
		AG_DrawBoxSunk(win, &r, c);	\
	} else {				\
		AG_DrawBoxRaised(win, &r, c);	\
	}

static void
Draw(void *_Nonnull obj)
{
	AG_Window *win = obj;
	const AG_Color *cFg = &WCOLOR(win, FG_COLOR);
	AG_Widget *chld;
	AG_Rect r;
	const int w = WIDTH(win);
	const int h = HEIGHT(win);
	const int hBar = (win->tbar) ? HEIGHT(win->tbar) : 0;
	const int wBorderBot = win->wBorderBot;
	int wBorderSide;

	if (UpdateNeeded(WIDGET(win)))
		AG_WindowUpdate(win);

	/* Render window background. */
	if ((win->flags & AG_WINDOW_NOBACKGROUND) == 0 &&
	    (WIDGET(win)->drv->flags & AG_DRIVER_WINDOW_BG) == 0) {
		r.x = 0;
		r.y = hBar-1;
		r.w = w;
		r.h = h-hBar;
		AG_DrawRect(win, &r, &WCOLOR(win, BG_COLOR));
	}

	/* Render decorative borders. */
	if (wBorderBot > 0) {
		const int wResizeCtrl = win->wResizeCtrl;
		const int wResizeCtrl2 = (wResizeCtrl << 1);

		r.x = 0;
		r.y = h - wBorderBot;
		r.h = wBorderBot;
		if ((win->flags & AG_WINDOW_NORESIZE) == 0 && w > wResizeCtrl2) {
			r.w = wResizeCtrl;
			DRAW_BORDER(AG_WINOP_LRESIZE, cFg);
			r.x = w - wResizeCtrl;
			DRAW_BORDER(AG_WINOP_RRESIZE, cFg);
			r.x = wResizeCtrl;
			r.w = w - wResizeCtrl2;
			DRAW_BORDER(AG_WINOP_HRESIZE, cFg);
		} else {
			r.w = w;
			AG_DrawBoxRaised(win, &r, cFg);
		}
	}
	if ((wBorderSide = win->wBorderSide) > 0) {
		r.x = 0;
		r.y = hBar;
		r.w = wBorderSide;
		r.h = h - wBorderBot - hBar;
		AG_DrawBoxRaised(win, &r, cFg);
		r.x = w - wBorderSide;
		AG_DrawBoxRaised(win, &r, cFg);
	}

	OBJECT_FOREACH_CHILD(chld, win, ag_widget)
		AG_WidgetDraw(chld);
}

#undef DRAW_BORDER

/* Handler for "widget-shown", generated when the window becomes visible. */
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Driver *drv = WIDGET(win)->drv;
	AG_Object *chld;
	AG_SizeReq r;
	AG_SizeAlloc a;
	int xPref, yPref;
	Uint mwFlags = 0;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");

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
			if (AGDRIVER_MULTIPLE(drv))
				AG_WindowComputeAlignment(win, &a);
		} else {
#ifdef AG_WM_HINTS
			if (AGDRIVER_MULTIPLE(drv) &&
			   (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_ANYPOS_AVAIL)) {
				/* Let the WM choose a default position */
				mwFlags |= AG_DRIVER_MW_ANYPOS;
			} else
#endif
			{
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
		AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);
		break;
	case AG_WM_MULTIPLE:
		/* We expect the driver will call AG_WidgetUpdateCoords(). */
		if (!(AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN)) {
			AG_Rect rw;

			rw.x = a.x;
			rw.y = a.y;
			rw.w = a.w;
			rw.h = a.h;
			if (AGDRIVER_MW_CLASS(drv)->openWindow(win, &rw, 0,
			    mwFlags) == -1) {
				AG_FatalError(NULL);
			}
			AGDRIVER_MW(drv)->flags |= AG_DRIVER_MW_OPEN;
		}
		if (win->flags & AG_WINDOW_FADEIN)
			AG_WindowSetOpacity(win, 0.0f);

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
	OBJECT_FOREACH_CHILD(chld, win, ag_object) {
/*		Debug(win, "Propagating \"widget-shown\"() to %s\n", chld->name); */
		AG_ForwardEvent(chld, event);
	}
	AG_PostEvent(win, "window-shown", NULL);

	/* Implicit focus change. */
	if (!(win->flags & AG_WINDOW_DENYFOCUS))
		agWindowToFocus = win;

	/* Mark for redraw */
	win->dirty = 1;

	/* We can now allow cursor changes. */
	win->flags &= ~(AG_WINDOW_NOCURSORCHG);

	if (win->flags & AG_WINDOW_FADEIN) {
		AG_AddTimer(win, &win->pvt.fadeTo,
		    (Uint32)((win->pvt.fadeInTime*1000.0) /
		             (1.0/win->pvt.fadeInIncr)),
		    FadeTimeout, "%i", 1);
	}
}

/* Handler for "widget-hidden", generated when the window is to be unmapped. */
static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverSw *dsw;
	AG_Object *chld;

	AG_OBJECT_ISA(drv, "AG_Driver:*");

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

			AG_PostEvent(win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		
			AG_FOREACH_WINDOW_REVERSE(wOther, dsw) {
				if (wOther->visible &&
				  !(wOther->flags & AG_WINDOW_DENYFOCUS))
					break;
			}
			if (wOther)
				agWindowToFocus = wOther;
		}
		if (AGDRIVER_CLASS(drv)->type == AG_FRAMEBUFFER) {
			AG_Rect r;

			r.x = 0;
			r.y = 0;
			r.w = WIDTH(win);
			r.h = HEIGHT(win);
			AG_DrawRectFilled(win, &r, &dsw->bgColor);

			if (AGDRIVER_CLASS(drv)->updateRegion != NULL) {
				r.x = WIDGET(win)->x;
				r.y = WIDGET(win)->y;
				AGDRIVER_CLASS(drv)->updateRegion(drv, &r);
			}
		}
		break;
	case AG_WM_MULTIPLE:
		if (win == agWindowFocused) {
			AG_PostEvent(win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
			if (AGDRIVER_MW_CLASS(drv)->unmapWindow(win) == -1)
				AG_FatalError(NULL);
		}
		break;
	}
	
	/* Notify widgets that the window is now hidden. */
	OBJECT_FOREACH_CHILD(chld, win, ag_object) {
/*		Debug(win, "Propagating \"widget-hidden\"() to %s\n", chld->name); */
		AG_ForwardEvent(chld, event);
	}
	AG_PostEvent(win, "window-hidden", NULL);
}

/* Handler for "detached", generated by AG_ObjectDetach(). */
static void
OnDetach(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Object *chld;

#if defined(AG_DEBUG) && defined(AG_WIDGETS)
	if (win == agTargetWindow)
		AG_GuiDebuggerDetachWindow();
#endif

	/* Propagate to child objects */
	OBJECT_FOREACH_CHILD(chld, win, ag_object) {
/*		AG_Debug(win, "Broadcasting \"detached\"() to %s\n", chld->name); */

		event->argv[1].data.p = win;	/* Make SENDER */
		AG_ForwardEvent(chld, event);
	}
}

static void
WidgetGainFocus(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_OBJECT_ISA(chld, "AG_Widget:*");
		AG_ObjectLock(chld);
		WidgetGainFocus(chld);
		AG_ObjectUnlock(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(wid, "widget-gainfocus", NULL);
}

static void
WidgetLostFocus(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_OBJECT_ISA(chld, "AG_Widget:*");
		AG_ObjectLock(chld);
		WidgetLostFocus(chld);
		AG_ObjectUnlock(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(wid, "widget-lostfocus", NULL);
}

static void
OnGainFocus(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();

#ifdef DEBUG_FOCUS
	Debug(win, "Focus Gained (\"%s\")\n", win->caption);
#endif
	WidgetGainFocus(WIDGET(win));
}

static void
OnLostFocus(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();

#ifdef DEBUG_FOCUS
	Debug(win, "Focus Lost (\"%s\")\n", win->caption);
#endif
	WidgetLostFocus(WIDGET(win));
}

/* Lower a window to the bottom of the stack. */
void
AG_WindowLower(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(drv, "AG_Driver:*");
#ifdef DEBUG_RAISE_LOWER
	Debug(drv, "Lowering %s to bottom\n", OBJECT(win)->name);
#endif
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(drv);
	AG_ObjectLock(win);
	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_MULTIPLE:
		AGDRIVER_MW_CLASS(drv)->lowerWindow(win);
		break;
	case AG_WM_SINGLE:
		TAILQ_REMOVE(&OBJECT(drv)->children, OBJECT(win), cobjs);
		TAILQ_INSERT_HEAD(&OBJECT(drv)->children, OBJECT(win), cobjs);
		break;
	}
	AG_ObjectUnlock(drv);
	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Raise a window to the top of the stack. */
void
AG_WindowRaise(AG_Window *win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(drv, "AG_Driver:*");
#ifdef DEBUG_RAISE_LOWER
	Debug(drv, "Raising %s to top\n", OBJECT(win)->name);
#endif
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(drv);
	AG_ObjectLock(win);
	switch (AGDRIVER_CLASS(drv)->wm) {
	case AG_WM_MULTIPLE:
		AGDRIVER_MW_CLASS(drv)->raiseWindow(win);
		break;
	case AG_WM_SINGLE:
		TAILQ_REMOVE(&OBJECT(drv)->children, OBJECT(win), cobjs);
		TAILQ_INSERT_TAIL(&OBJECT(drv)->children, OBJECT(win), cobjs);
		break;
	}
	AG_ObjectUnlock(drv);
	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Make a window visible to the user. */
void
AG_WindowShow(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);
	if (!win->visible) {
#ifdef AG_THREADS
		if (!AG_ThreadEqual(AG_ThreadSelf(), agEventThread)) {
			AG_LockVFS(&agDrivers);
			TAILQ_INSERT_TAIL(&agWindowShowQ, win, pvt.visibility);
			AG_UnlockVFS(&agDrivers);
		} else
#endif
		{
			AG_PostEvent(win, "widget-shown", NULL);
		}
	}
	AG_ObjectUnlock(win);
	AG_UnlockVFS(&agDrivers);
}

/* Make a window invisible to the user. */
void
AG_WindowHide(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_LockVFS(&agDrivers);
	AG_ObjectLock(win);

	if (!win->visible) {
		goto out;
	}
	if ((win->flags & AG_WINDOW_FADEOUT) &&
	   !(win->flags & AG_WINDOW_DETACHING)) {
		AG_AddTimer(win, &win->pvt.fadeTo,
		    (Uint32)((win->pvt.fadeOutTime * 1000.0) /
		             (1.0 / win->pvt.fadeOutIncr)),
		    FadeTimeout, "%i", -1);
	} else {
#ifdef AG_THREADS
		if (!AG_ThreadEqual(AG_ThreadSelf(), agEventThread)) {
			AG_LockVFS(&agDrivers);
			TAILQ_INSERT_TAIL(&agWindowHideQ, win, pvt.visibility);
			AG_UnlockVFS(&agDrivers);
		} else
#endif
		{
			AG_PostEvent(win, "widget-hidden", NULL);
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

/*
 * Build an ordered list (with duplicates allowed) of all potentially
 * focusable widgets in a window. Honor focusFwd and "tabindex" attributes.
 */
static void
FindFocusableWidgets(AG_WidgetVec *W, AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;
	
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_FOCUSABLE) {
		AG_VEC_PUSH(W, wid->focusFwd ? wid->focusFwd : wid);
	}
#if 0
	if (AG_Defined(wid, "tabindex"))
		/* TODO move to proper index. */
#endif
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		FindFocusableWidgets(W, chld);
	}
	AG_ObjectUnlock(wid);
}

/*
 * Move the widget focus inside a window.
 * The window must be locked.
 */
void
AG_WindowCycleFocus(AG_Window *win, int reverse)
{
	AG_WidgetVec W;		/* Ordered list of potential candidates */
	AG_WidgetVec WU;	/* W with duplicates removed */
	int i, j;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");

	AG_VEC_INIT(&W);
	AG_VEC_INIT(&WU);
	FindFocusableWidgets(&W, WIDGET(win));
	for (i = 0; i < W.length; i++) {
		for (j = 0; j < WU.length; j++) {
			if (W.data[i] == WU.data[j])
				break;
		}
		if (j == WU.length)
			AG_VEC_PUSH(&WU, W.data[i]);
	}
	if (WU.length == 0)
		goto out;

	/* Move focus after/before the currently focused widget. */
	if (reverse) {
		for (i = 0; i < WU.length; i++) {
			if (WIDGET(WU.data[i])->flags & AG_WIDGET_FOCUSED)
				break;
		}
		if (i == -1) {
			AG_WidgetFocus(WU.data[0]);
		} else {
			if (i-1 < 0) {
				AG_WidgetFocus(WU.data[WU.length-1]);
			} else {
				AG_WidgetFocus(WU.data[i-1]);
			}
		}
	} else {
		for (i = WU.length-1; i >= 0; i--) {
			if (WIDGET(WU.data[i])->flags & AG_WIDGET_FOCUSED)
				break;
		}
		if (i == WU.length) {
			AG_WidgetFocus(WU.data[0]);
		} else {
			if (i+1 < WU.length) {
				AG_WidgetFocus(WU.data[i+1]);
			} else {
				AG_WidgetFocus(WU.data[0]);
			}
		}
	}
out:
	AG_VEC_DESTROY(&WU);
	AG_VEC_DESTROY(&W);
	win->dirty = 1;
}

/*
 * Return a pointer to the currently focused window.
 * The agDrivers VFS must be locked.
 */
AG_Window *
AG_WindowFindFocused(void)
{
	return (agWindowFocused);
}

/*
 * Test whether the given window is focused.
 * The agDrivers VFS must be locked.
 */
int
AG_WindowIsFocused(AG_Window *_Nullable win)
{
	return (agWindowFocused == win);
}

/*
 * Give input focus to a window. The actual focus change will take effect at
 * the end of the current event cycle.
 */
void
AG_WindowFocus(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");

	AG_LockVFS(&agDrivers);

	if (win == NULL || agWindowFocused == win) {
		agWindowToFocus = NULL;
		goto out;
	}

	/* Abort if there are any visible modal windows. */
	if (agDriverOps->wm == AG_WM_MULTIPLE) {
		AG_Driver *drv;
		AG_Window *other;

		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_OBJECT_ISA(drv, "AG_Driver:*");
			AG_FOREACH_WINDOW(other, drv) {
				if (other == win) {
					continue;
				}
				AG_OBJECT_ISA(other, "AG_Widget:AG_Window:*");
				AG_ObjectLock(other);
				if (other->flags & AG_WINDOW_MODAL) {
					AG_ObjectUnlock(other);
					goto out;
				}
				AG_ObjectUnlock(other);
			}
		}
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

/*
 * Give focus to a window by name, and show it is if is hidden.
 */
int
AG_WindowFocusNamed(const char *name)
{
	AG_Driver *drv;
	AG_Window *owin;
	int rv = 0;

	AG_LockVFS(&agDrivers);
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_OBJECT_ISA(drv, "AG_Driver:*");
		AG_FOREACH_WINDOW(owin, drv) {
			AG_OBJECT_ISA(owin, "AG_Widget:AG_Window:*");
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
	
	AG_OBJECT_ISA(dsw, "AG_Driver:AG_DriverSw:*");

	AG_LockVFS(&agDrivers);
	agWindowToFocus = NULL;
	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");

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
UpdateWindowBG(AG_Window *_Nonnull win, const AG_Rect *_Nonnull rPrev)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_Rect r;

	if (WIDGET(win)->x > rPrev->x) {		/* L-resize */
		r.x = rPrev->x;
		r.y = rPrev->y;
		r.w = WIDGET(win)->x - rPrev->x;
		r.h = HEIGHT(win);
	} else if (WIDTH(win) < rPrev->w) {		/* R-resize */
		r.x = WIDGET(win)->x + WIDTH(win);
		r.y = WIDGET(win)->y;
		r.w = rPrev->w - WIDTH(win);
		r.h = rPrev->h;
	} else {
		r.w = 0;
		r.h = 0;
	}
	if (r.w > 0 && r.h > 0) {
		AGDRIVER_CLASS(drv)->fillRect(drv, &r,
		    &AGDRIVER_SW(drv)->bgColor);
		if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
			AGDRIVER_CLASS(drv)->updateRegion(drv, &r);
	}
	if (HEIGHT(win) < rPrev->h) {				/* H-resize */
		r.x = rPrev->x;
		r.y = WIDGET(win)->y + HEIGHT(win);
		r.w = rPrev->w;
		r.h = rPrev->h - HEIGHT(win);
			
		AGDRIVER_CLASS(drv)->fillRect(drv, &r,
		    &AGDRIVER_SW(drv)->bgColor);
		if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
			AGDRIVER_CLASS(drv)->updateRegion(drv, &r);
	}
}

/* Set window position and size (no bounds) */
int
AG_WindowSetGeometry(AG_Window *win, int x, int y, int w, int h)
{
	AG_Rect r;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return AG_WindowSetGeometryRect(win, &r, 0);
}

/* Set the coordinates and geometry of a window. */
int
AG_WindowSetGeometryRect(AG_Window *win, const AG_Rect *r, int bounded)
{
	AG_Driver *drv = WIDGET(win)->drv;
	const AG_DriverClass *dc = AGDRIVER_CLASS(drv);
	AG_SizeReq rWin;
	AG_SizeAlloc a;
	AG_Rect rPrev;
	int new;
	int nw, nh;
	int wMin, hMin;
	Uint wDisp = 0, hDisp = 0;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	AG_ObjectLock(win);

	rPrev.x = WIDGET(win)->x;
	rPrev.y = WIDGET(win)->y;
	rPrev.w = WIDGET(win)->w;
	rPrev.h = WIDGET(win)->h;
	new = ((WIDGET(win)->x == -1 || WIDGET(win)->y == -1));

	/* Compute the final window size. */
	if (r->w == -1 || r->h == -1) {
		AG_WidgetSizeReq(win, &rWin);
		nw = (r->w == -1) ? rWin.w : r->w;
		nh = (r->h == -1) ? rWin.h : r->h;
	} else {
		nw = r->w;
		nh = r->h;
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
			WIDGET(win)->x = (wDisp >> 1) - (nw >> 1);
		if (WIDGET(win)->y == -1)
			WIDGET(win)->y = (hDisp >> 1) - (nh >> 1);
	}
	a.x = (r->x == -1) ? WIDGET(win)->x : r->x;
	a.y = (r->y == -1) ? WIDGET(win)->y : r->y;
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
			UpdateWindowBG(win, &rPrev);
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
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
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
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
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
 *
 * The window must be locked.
 */
void
AG_WindowComputeAlignment(AG_Window *win, AG_SizeAlloc *a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	Uint wMax, hMax;
	int w = a->w;
	int h = a->h;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_OBJECT_ISA(drv, "AG_Driver:*");

	if (AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax) == -1)
		return;

	switch (win->alignment) {
	case AG_WINDOW_TL:
		a->x = 0;
		a->y = 0;
		break;
	case AG_WINDOW_TC:
		a->x = (wMax >> 1) - (w >> 1);
		a->y = 0;
		break;
	case AG_WINDOW_TR:
		a->x = wMax - w;
		a->y = 0;
		break;
	case AG_WINDOW_ML:
		a->x = 0;
		a->y = (hMax >> 1) - (h >> 1);
		break;
	case AG_WINDOW_MR:
		a->x = wMax - w;
		a->y = (hMax >> 1) - (h >> 1);
		break;
	case AG_WINDOW_BL:
		a->x = 0;
		a->y = hMax - h;
		break;
	case AG_WINDOW_BC:
		a->x = (wMax >> 1) - (w >> 1);
		a->y = hMax - h;
		break;
	case AG_WINDOW_BR:
		a->x = wMax - w;
		a->y = hMax - h;
		break;
	case AG_WINDOW_MC:
	case AG_WINDOW_ALIGNMENT_NONE:
	default:
		a->x = (wMax >> 1) - (w >> 1);
		a->y = (hMax >> 1) - (h >> 1);
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
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
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
	Uint wMax=0, hMax=0;
	AG_Driver *drv;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	drv = WIDGET(win)->drv;
	AG_OBJECT_ISA(drv, "AG_Driver:*");

	AG_GetDisplaySize(drv, &wMax, &hMax);

	return AG_WindowSetGeometryAligned(win, align,
	                                   wPct * wMax / 100,
	                                   hPct * hMax / 100);
}

/* Backup the current window geometry (i.e., before a minimize) */
void
AG_WindowSaveGeometry(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");

	win->rSaved.x = WIDGET(win)->x;
	win->rSaved.y = WIDGET(win)->y;
	win->rSaved.w = WIDTH(win);
	win->rSaved.h = HEIGHT(win);
}

/* Restore saved geometry (i.e., after an unminimize operation) */
int
AG_WindowRestoreGeometry(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");

	return AG_WindowSetGeometryRect(win, &win->rSaved, 0);
}

/* Maximize a window */
void
AG_WindowMaximize(AG_Window *win)
{
	Uint wMax, hMax;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	AG_WindowSaveGeometry(win);
	AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax);

	if (AG_WindowSetGeometry(win, 0, 0, wMax, hMax) == 0)
		win->flags |= AG_WINDOW_MAXIMIZED;
	
	AG_ObjectUnlock(win);
}

/* Restore a window's geometry prior to maximization. */
void
AG_WindowUnmaximize(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	if (AG_WindowRestoreGeometry(win) == 0) {
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
		win->dirty = 1;
	}
	
	AG_ObjectUnlock(win);
}

#ifdef AG_WIDGETS
/*
 * Mouse-motion callback for a minimized window icon in SW mode.
 */
static void
IconMotion(AG_Event *_Nonnull event)
{
	AG_Icon *icon = AG_ICON_SELF();
	AG_Driver *drv = WIDGET(icon)->drv;
	int xRel = AG_INT(3);
	int yRel = AG_INT(4);

	AG_OBJECT_ISA(drv, "AG_Driver:*");

	if (icon->flags & AG_ICON_DND) {
		AG_Rect r;
		AG_Window *wDND = icon->wDND;

		AG_OBJECT_ISA(wDND, "AG_Widget:AG_Window:*");

		if (drv && AGDRIVER_SINGLE(drv)) {
			r.x = 0;
			r.y = 0;
			r.w = AGDRIVER_SW(drv)->w;
			r.h = AGDRIVER_SW(drv)->h;
			AGDRIVER_CLASS(drv)->fillRect(drv, &r,
			    &AGDRIVER_SW(drv)->bgColor);
			AG_Rect2ToRect(&r, &WIDGET(wDND)->rView);

			if (AGDRIVER_CLASS(drv)->updateRegion != NULL)
				AGDRIVER_CLASS(drv)->updateRegion(drv, &r);
		}
		r.x = WIDGET(wDND)->x + xRel;
		r.y = WIDGET(wDND)->y + yRel;
		r.w = WIDTH(wDND);
		r.h = HEIGHT(wDND);
		AG_WindowSetGeometryRect(wDND, &r, 1);
		 
		icon->xSaved = WIDGET(wDND)->x;
		icon->ySaved = WIDGET(wDND)->y;
		icon->wSaved = WIDTH(wDND);
		icon->hSaved = HEIGHT(wDND);
	}
}
#endif /* AG_WIDGETS */

/* Timer for double click on minimized icon. */
static Uint32
IconDoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Icon *icon = AG_ICON_SELF();

	icon->flags &= ~(AG_ICON_DBLCLICKED);
	return (0);
}

static void
IconButtonDown(AG_Event *_Nonnull event)
{
	AG_Icon *icon = AG_ICON_SELF();

	WIDGET(icon)->flags |= AG_WIDGET_UNFOCUSED_MOTION |
	                       AG_WIDGET_UNFOCUSED_BUTTONUP;
	if (icon->flags & AG_ICON_DBLCLICKED) {
		AG_Window *win = AG_WINDOW_PTR(1);

		AG_DelTimer(icon, &icon->toDblClick);
		AG_WindowUnminimize(win);
		icon->flags &= (AG_ICON_DND|AG_ICON_DBLCLICKED);
	} else {
		icon->flags |= (AG_ICON_DND|AG_ICON_DBLCLICKED);
		AG_AddTimer(icon, &icon->toDblClick, agMouseDblclickDelay,
		    IconDoubleClickTimeout, NULL);
	}
}

static void
IconButtonUp(AG_Event *_Nonnull event)
{
	AG_Icon *icon = AG_ICON_SELF();
	
	WIDGET(icon)->flags &= ~(AG_WIDGET_UNFOCUSED_MOTION);
	WIDGET(icon)->flags &= ~(AG_WIDGET_UNFOCUSED_BUTTONUP);
	icon->flags &= ~(AG_ICON_DND);
}

/* Minimize a window */
void
AG_WindowMinimize(AG_Window *win)
{
	AG_Driver *drv;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	if (win->flags & AG_WINDOW_MINIMIZED) {
		Debug(win, "Already minimized\n");
		goto out;
	}
	Debug(win, "Minimizing\n");
	win->flags |= AG_WINDOW_MINIMIZED;
	AG_WindowHide(win);
#ifdef AG_WIDGETS
	if ((drv = WIDGET(win)->drv) && AGDRIVER_SINGLE(drv)) {
		if (CreateMinimizedIcon(win) == -1)
			goto out;
	}
#endif
	/* TODO MW: send a WM_CHANGE_STATE */
out:
	AG_ObjectUnlock(win);
}

#ifdef AG_WIDGETS
static int
CreateMinimizedIcon(AG_Window *_Nonnull win)
{
	AG_Window *wDND;
	AG_Icon *icon = win->icon;

	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	if ((wDND = icon->wDND) == NULL) {
		wDND = icon->wDND = AG_WindowNew(
		    AG_WINDOW_PLAIN | AG_WINDOW_KEEPBELOW |
		    AG_WINDOW_DENYFOCUS | AG_WINDOW_NOBACKGROUND);
		if (wDND == NULL) {
			return (-1);
		}
		AG_ObjectAttach(wDND, icon);
		AG_ObjectSetName(wDND, "icon%u", agWindowIconCounter++);
	} else {
		AG_WindowShow(wDND);
	}
	icon->flags &= ~(AG_ICON_DND | AG_ICON_DBLCLICKED);

	AG_SetEvent(icon, "mouse-motion", IconMotion, NULL);
	AG_SetEvent(icon, "mouse-button-up", IconButtonUp, NULL);
	AG_SetEvent(icon, "mouse-button-down", IconButtonDown, "%p", win);

	if (icon->xSaved != -1) {
		AG_WindowShow(wDND);
		AG_WindowSetGeometry(wDND,
		    icon->xSaved, icon->ySaved,
		    icon->wSaved, icon->hSaved);
	} else {
		AG_WindowSetPosition(wDND, AG_WINDOW_LOWER_LEFT, 1);
		AG_WindowShow(wDND);
		icon->xSaved = WIDGET(wDND)->x;
		icon->ySaved = WIDGET(wDND)->y;
		icon->wSaved = WIDTH(wDND);
		icon->hSaved = HEIGHT(wDND);
	}
	return (0);
}
#endif /* AG_WIDGETS */

/* Unminimize a window */
void
AG_WindowUnminimize(AG_Window *win)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	if (!win->visible) {
		AG_Driver *drv;
#ifdef AG_WIDGETS
		if ((drv = WIDGET(win)->drv) && AGDRIVER_SINGLE(drv))
			HideMinimizedIcon(win);
#endif
		AG_WindowShow(win);
		win->flags &= ~(AG_WINDOW_MINIMIZED);
	} else {
		AG_WindowFocus(win);
	}
	/* TODO MW: send a WM_CHANGE_STATE */
	AG_ObjectUnlock(win);
}

#ifdef AG_WIDGETS
static void
HideMinimizedIcon(AG_Window *_Nonnull win)
{
	AG_Icon *icon;

	if ((icon = win->icon) == NULL) {
		return;
	}
	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	AG_OBJECT_ISA(icon->wDND, "AG_Widget:AG_Window:*");
	AG_WindowHide(icon->wDND);
}
#endif /* AG_WIDGETS */

/* AGWINDETACH(): General-purpose "detach window" event handler. */
void
AG_WindowDetachGenEv(AG_Event *event)
{
	AG_ObjectDetach(AG_WINDOW_PTR(1));
}

/* AGWINHIDE(): General-purpose "hide window" event handler. */
void
AG_WindowHideGenEv(AG_Event *event)
{
	AG_WindowHide(AG_WINDOW_PTR(1));
}

/* AGWINCLOSE(): General-purpose "close window" event handler. */
void
AG_WindowCloseGenEv(AG_Event *event)
{
	AG_PostEvent(AG_WINDOW_PTR(1), "window-close", NULL);
}

/* Close the actively focused window. */
void
AG_CloseFocusedWindow(void)
{
	AG_LockVFS(&agDrivers);
	if (agWindowFocused) {
		AG_OBJECT_ISA(agWindowFocused, "AG_Widget:AG_Window:*");
		AG_PostEvent(agWindowFocused, "window-close", NULL);
	}
	AG_UnlockVFS(&agDrivers);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Window *win = obj;
	AG_Widget *chld;
	AG_SizeReq rChld, rTbar;
	AG_Titlebar *tbar = win->tbar;
	const int spacing = win->spacing;
	int nWidgets;
	int wTot;
	
	wTot = win->lPad + win->rPad + (win->wBorderSide << 1);
	r->w = wTot;
	r->h = win->bPad+win->tPad + win->wBorderBot;

	if (tbar) {
		AG_WidgetSizeReq(tbar, &rTbar);
		r->w = MAX(r->w, rTbar.w);
		r->h += rTbar.h;
	}
	nWidgets = 0;
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
		if (chld == WIDGET(tbar)) {
			continue;
		}
		AG_OBJECT_ISA(chld, "AG_Widget:*");
		AG_WidgetSizeReq(chld, &rChld);
		r->w = MAX(r->w, rChld.w + wTot);
		r->h += rChld.h + spacing;
		nWidgets++;
	}
	if (nWidgets > 0 && r->h >= spacing)
		r->h -= spacing;

	win->wReq = r->w;
	win->hReq = r->h;

	if (WIDGET(win)->drv && AGDRIVER_SINGLE(WIDGET(win)->drv))
		AG_WM_LimitWindowToView(win);
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Window *win = obj;
	AG_Widget *chld;
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	AG_Rect r;
	const int spacing = win->spacing;
	const int wBorderSide = win->wBorderSide;
	const int wBorderBot = win->wBorderBot;
	int wAvail, hAvail, totFixed, nWidgets;

	if (wBorderSide > 0) {
		r.x = 0;
		r.y = 0;
		r.w = wBorderSide;
		r.h = a->h - wBorderBot;
		AG_SetStockCursor(win, &win->pvt.caResize[0], &r,
		    AG_HRESIZE_CURSOR);
		r.x = a->w - wBorderSide;
		AG_SetStockCursor(win, &win->pvt.caResize[4], &r,
		    AG_HRESIZE_CURSOR);
	}
	if (wBorderBot > 0) {
		int wResizeCtrl = win->wResizeCtrl;

		r.x = 0;
		r.y = a->h - wBorderBot;
		r.w = wResizeCtrl;
		r.h = wBorderBot;
		AG_SetStockCursor(win, &win->pvt.caResize[1], &r,
		    AG_LRDIAG_CURSOR);

		r.x = wResizeCtrl;
		r.w = a->w - (wResizeCtrl << 1);
		AG_SetStockCursor(win, &win->pvt.caResize[2], &r,
		    AG_VRESIZE_CURSOR);

		r.x = a->w - wResizeCtrl;
		r.w = wResizeCtrl;
		AG_SetStockCursor(win, &win->pvt.caResize[3], &r,
		    AG_LLDIAG_CURSOR);
	}

	/* Calculate total space available for widgets. */
	wAvail = a->w - win->lPad - win->rPad - (wBorderSide << 1);
	hAvail = a->h - win->bPad - win->tPad - wBorderBot;

	/* Calculate the space occupied by non-fill widgets. */
	nWidgets = 0;
	totFixed = 0;
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
		AG_OBJECT_ISA(chld, "AG_Widget:*");
		AG_WidgetSizeReq(chld, &rChld);
		if ((chld->flags & AG_WIDGET_VFILL) == 0) {
			totFixed += rChld.h;
		}
		if (chld != WIDGET(win->tbar)) {
			totFixed += spacing;
		}
		nWidgets++;
	}
	if (nWidgets > 0 && totFixed >= spacing)
		totFixed -= spacing;

	if (win->tbar) {					/* Titlebar */
		AG_WidgetSizeReq(win->tbar, &rChld);
		aChld.x = 0;
		aChld.y = 0;
		aChld.w = a->w;
		aChld.h = rChld.h;
		AG_WidgetSizeAlloc(win->tbar, &aChld);
		aChld.x = win->lPad + wBorderSide;
		aChld.y = rChld.h + win->tPad;
	} else {
		aChld.x = win->lPad + wBorderSide;
		aChld.y = win->tPad;
	}
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {		/* Widgets */
		AG_OBJECT_ISA(chld, "AG_Widget:*");
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
				aChld.y += spacing;
			}
			continue;
		}
		aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
		          wAvail : MIN(wAvail,rChld.w);
		aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
		          hAvail-totFixed : rChld.h;
		AG_WidgetSizeAlloc(chld, &aChld);
		aChld.y += aChld.h + spacing;
	}

	if (WIDGET(win)->drv && AGDRIVER_SINGLE(WIDGET(win)->drv))
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
	if (win) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		win->wBorderSide = pixels;
		win->dirty = 1;
	} else {
		agWindowSideBorderDefault = pixels;
	}
}

/* Set the width of the window bottom border. */
void
AG_WindowSetBottomBorder(AG_Window *win, int pixels)
{
	if (win) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		win->wBorderBot = pixels;
		win->dirty = 1;
	} else {
		agWindowBotBorderDefault = pixels;
	}
}

/* Change the spacing between child widgets. */
void
AG_WindowSetSpacing(AG_Window *win, int spacing)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	win->spacing = spacing;
	win->dirty = 1;
}

/* Change the padding around child widgets. */
void
AG_WindowSetPadding(AG_Window *win, int lPad, int rPad, int tPad, int bPad)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
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
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	win->alignment = alignment;
	AG_SETFLAGS(win->flags, AG_WINDOW_TILING, tiling);

	AG_ObjectUnlock(win);
}

/* Set a default window-close handler. */
void
AG_WindowSetCloseAction(AG_Window *win, enum ag_window_close_action mode)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
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
UpdateTitlebar(AG_Window *_Nonnull win)
{
	AG_Titlebar *tbar;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	tbar = win->tbar;
	AG_OBJECT_ISA(tbar, "AG_Widget:AG_Box:AG_Titlebar:*");
	AG_ObjectLock(tbar);
	AG_LabelTextS(tbar->label, win->caption);
	AG_ObjectUnlock(tbar);
}

/* Update the window's minimized icon caption. */
static void
UpdateIconCaption(AG_Window *_Nonnull win)
{
	AG_Icon *icon = win->icon;
	char s[20], *c;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	icon = win->icon;
	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");

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

/* Set the text to show inside a window's titlebar (format string). */
void
AG_WindowSetCaption(AG_Window *win, const char *fmt, ...)
{
	char *s;
	va_list ap;
	
	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_WindowSetCaptionS(win, s);

	free(s);
}

/* Set the text to show inside a window's titlebar (C string). */
void
AG_WindowSetCaptionS(AG_Window *win, const char *s)
{
	AG_Driver *drv;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	Strlcpy(win->caption, s, sizeof(win->caption));

	if (win->tbar)
		UpdateTitlebar(win);
	if (win->icon)
		UpdateIconCaption(win);

	if ((drv = WIDGET(win)->drv) && AGDRIVER_MULTIPLE(drv) &&
	    (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) &&
	    (AGDRIVER_MW_CLASS(drv)->setWindowCaption != NULL)) {
		AGDRIVER_MW_CLASS(drv)->setWindowCaption(win, s);
	}
	win->dirty = 1;
	AG_ObjectUnlock(win);
}

/* Set the icon of a window from the contents of a surface. */
void
AG_WindowSetIcon(AG_Window *win, const AG_Surface *S)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_IconSetSurface(win->icon, S);
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
#ifdef AG_TYPE_SAFETY
		if (agWindowToFocus)
			AG_OBJECT_ISA(agWindowToFocus, "AG_Widget:AG_Window:*");
#endif
		if ((drv = WIDGET(agWindowToFocus)->drv) != NULL) {
			AG_OBJECT_ISA(drv, "AG_Driver:*");
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

	TAILQ_FOREACH(win, &agWindowShowQ, pvt.visibility) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		AG_PostEvent(win, "widget-shown", NULL);
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

	TAILQ_FOREACH(win, &agWindowHideQ, pvt.visibility) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		AG_PostEvent(win, "widget-hidden", NULL);
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
#ifdef AG_DEBUG
	int debugLvlSave;
#endif
	TAILQ_FOREACH(win, &agWindowDetachQ, pvt.detach) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		if (!win->visible) {
			continue;
		}
		/*
		 * Note: `widget-hidden' event handlers may cause new windows
		 * to be added to agWindowDetachQ.
		 */
		AG_PostEvent(win, "widget-hidden", NULL);
		nHidden++;
	}
	if (nHidden > 0) {
		/*
		 * Windows were hidden - defer detach operation until the next
		 * event cycle, just in case the underlying WM cannot hide and
		 * unmap a window in the same event cycle.
		 */
		return;
	}

	for (win = TAILQ_FIRST(&agWindowDetachQ);
	     win != TAILQ_END(&agWindowDetachQ);
	     win = winNext) {
		winNext = TAILQ_NEXT(win, pvt.detach);
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		drv = WIDGET(win)->drv;
		AG_OBJECT_ISA(drv, "AG_Driver:*");

		/* Notify all widgets of the window detach. */
		AG_PostEvent(win, "detached", "%p", drv);

		/* Release the cursor areas and associated cursors. */
		AG_UnmapAllCursors(win, NULL);
	
		if (AGDRIVER_MULTIPLE(drv)) {
			if (AGDRIVER_MW(drv)->flags & AG_DRIVER_MW_OPEN) {
				AGDRIVER_MW_CLASS(drv)->closeWindow(win);
				AGDRIVER_MW(drv)->flags &= ~(AG_DRIVER_MW_OPEN);
			}
		} else {
			win->tbar = NULL;
			AGDRIVER_SW(drv)->flags |= AG_DRIVER_SW_REDRAW;
		}

		/* Unset detach-fn and do a standard AG_ObjectDetach(). */
		Debug_Mute(debugLvlSave);
		AG_SetFn(win, "detach-fn", NULL, NULL);
		Debug_Unmute(debugLvlSave);
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
		AG_PostEvent(win, "window-detached", NULL);
		AG_ObjectDestroy(win);
	}
	TAILQ_INIT(&agWindowDetachQ);
	
	/* Terminate if the last AG_WINDOW_MAIN window was closed. */
	if (closedMain > 0) {
		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_OBJECT_ISA(drv, "AG_Driver:*");
			AG_FOREACH_WINDOW(win, drv) {
				AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
				if (win->flags & AG_WINDOW_MAIN)
					break;
			}
			if (win != NULL)
				break;
		}
		if (drv == NULL) {
#ifdef AG_EVENT_LOOP
			AG_Terminate(0);
#endif
		}
	}
}

/*
 * Configure a new cursor-change area with a specified cursor. The provided
 * cursor will be freed automatically on window detach.
 */
AG_CursorArea *
AG_MapCursor(void *obj, const AG_Rect *r, AG_Cursor *c)
{
	AG_Widget *wid = obj;
	AG_Window *win;
	AG_CursorArea *ca;
	
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	
	if ((ca = TryMalloc(sizeof(AG_CursorArea))) == NULL) {
		return (NULL);
	}
	ca->stock = -1;
	ca->r = *r;
	ca->c = c;
	ca->wid = wid;
	
	if ((win = wid->window) != NULL) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		TAILQ_INSERT_TAIL(&win->pvt.cursorAreas, ca, cursorAreas);
	} else {
		TAILQ_INSERT_TAIL(&wid->pvt.cursorAreas, ca, cursorAreas);
	}
	return (ca);
}

/* Configure a new cursor-change area with a stock Agar cursor. */
AG_CursorArea *
AG_MapStockCursor(void *obj, const AG_Rect *_Nonnull r, int name)
{
	AG_Widget *wid = obj;
	AG_Window *win;
	AG_Driver *drv;
	AG_Cursor *ac;
	AG_CursorArea *ca;
	int i = 0;
	
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	win = wid->window;
	drv = wid->drv;
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	TAILQ_FOREACH(ac, &drv->cursors, cursors) {
		if (i++ == name)
			break;
	}
	if (ac == NULL)
		return (NULL);

	if ((ca = TryMalloc(sizeof(AG_CursorArea))) == NULL) {
		return (NULL);
	}
	ca->stock = name;
	ca->r = *r;
	ca->wid = WIDGET(obj);

	if (win != NULL) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		ca->c = ac;
		TAILQ_INSERT_TAIL(&win->pvt.cursorAreas, ca, cursorAreas);
	} else {
		/* Will resolve cursor name when widget is later attached. */
		ca->c = NULL;
		TAILQ_INSERT_TAIL(&wid->pvt.cursorAreas, ca, cursorAreas);
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
		TAILQ_REMOVE(&wid->pvt.cursorAreas, ca, cursorAreas);
		free(ca);
	} else {
		AG_Driver *drv;
		
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		drv = WIDGET(win)->drv;
		AG_OBJECT_ISA(drv, "AG_Driver:*");

		if (ca->c == drv->activeCursor) {
			if (ca->stock == -1) {
				/* XXX TODO it would be safer to defer this operation */
				if (AGDRIVER_CLASS(drv)->unsetCursor != NULL) {
					AGDRIVER_CLASS(drv)->unsetCursor(drv);
				}
				AG_CursorFree(drv, ca->c);
			}
			TAILQ_REMOVE(&win->pvt.cursorAreas, ca, cursorAreas);
			free(ca);
		}
	}
}

/*
 * Destroy all cursors associated with a widget wid
 * (or all cursors altogether if wid is NULL).
 */
void
AG_UnmapAllCursors(AG_Window *win, void *wid)
{
	AG_Driver *drv;
	AG_CursorArea *ca, *caNext;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	drv = WIDGET(win)->drv;
	AG_OBJECT_ISA(drv, "AG_Driver:*");

	if (wid == NULL) {
		for (ca = TAILQ_FIRST(&win->pvt.cursorAreas);
		     ca != TAILQ_END(&win->pvt.cursorAreas);
		     ca = caNext) {
			caNext = TAILQ_NEXT(ca, cursorAreas);
			if (ca->stock == -1) {
				AG_CursorFree(drv, ca->c);
			}
			free(ca);
		}
		TAILQ_INIT(&win->pvt.cursorAreas);
	} else {
scan:
		TAILQ_FOREACH(ca, &win->pvt.cursorAreas, cursorAreas) {
			if (ca->wid != wid) {
				continue;
			}
			if (ca->stock == -1) {
				AG_CursorFree(drv, ca->c);
			}
			TAILQ_REMOVE(&win->pvt.cursorAreas, ca, cursorAreas);
			free(ca);
			goto scan;
		}
	}
}

/*
 * Set the per-window opacity (for compositing window managers).
 */
int
AG_WindowSetOpacity(AG_Window *win, float f)
{
	AG_Driver *drv;
	int rv = -1;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);
	drv = OBJECT(win)->parent;
	AG_OBJECT_ISA(drv, "AG_Driver:*");

	win->pvt.fadeOpacity = (f > 1.0f) ? 1.0f : f;
	if (drv && AGDRIVER_MULTIPLE(drv) &&
	    AGDRIVER_MW_CLASS(drv)->setOpacity != NULL)
		rv = AGDRIVER_MW_CLASS(drv)->setOpacity(win, win->pvt.fadeOpacity);
	
	/* TODO: support compositing under single-window drivers. */
	AG_ObjectUnlock(win);
	return (rv);
}

/*
 * Set window fade-in/out parameters (for compositing window managers).
 */
void
AG_WindowSetFadeIn(AG_Window *win, float fadeTime, float fadeIncr)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	win->pvt.fadeInTime = fadeTime;
	win->pvt.fadeInIncr = fadeIncr;
}
void
AG_WindowSetFadeOut(AG_Window *win, float fadeTime, float fadeIncr)
{
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	win->pvt.fadeOutTime = fadeTime;
	win->pvt.fadeOutIncr = fadeIncr;
}

/* Set the window zoom level. The scales are defined in agZoomValues[]. */
void
AG_WindowSetZoom(AG_Window *win, int zoom)
{
	AG_Window *winChld;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_ObjectLock(win);

	if (zoom < 0 || zoom >= AG_ZOOM_MAX || zoom == win->zoom) {
		AG_ObjectUnlock(win);
		return;
	}

	AG_SetStyleF(win, "font-size", "%.02f%%", agZoomValues[zoom]);

	win->zoom = zoom;

	AG_WindowUpdate(win);

	if (WIDGET(win)->drv) {
		AG_TextClearGlyphCache(WIDGET(win)->drv);
	}
	TAILQ_FOREACH(winChld, &win->pvt.subwins, pvt.swins) {
		AG_WindowSetZoom(winChld, zoom);
	}
	AG_ObjectUnlock(win);
}

/*
 * Render a window to the display (must be enclosed between calls to
 * AG_BeginRendering() and AG_EndRendering()).
 * The agDrivers VFS and Window object must be locked.
 */
void
AG_WindowDraw(AG_Window *_Nonnull win)
{
	AG_Driver *drv;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	drv = WIDGET(win)->drv;
	AG_OBJECT_ISA(drv, "AG_Driver:*");

	if (!win->visible) {
		return;
	}
	AGDRIVER_CLASS(drv)->renderWindow(win);
	win->dirty = 0;
}

/*
 * Recompute the coordinates and geometries of all widgets attached to the
 * window. This is used following AG_ObjectAttach() and AG_ObjectDetach()
 * calls made in event context, or direct modifications to the x,y,w,h
 * fields of the Widget structure.
 *
 * The agDrivers VFS and Window must be locked.
 */
void
AG_WindowUpdate(AG_Window *_Nonnull win)
{
	AG_SizeAlloc a;
	
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	
	if (WIDGET(win)->x != -1 && WIDGET(win)->y != -1) {
		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDGET(win)->w;
		a.h = WIDGET(win)->h;
		AG_WidgetSizeAlloc(win, &a);
	}
	AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);
}

/*
 * Return visibility status of window.
 * The agDrivers VFS and Window object must be locked.
 */
int
AG_WindowIsVisible(AG_Window *_Nonnull win)
{
	return (win->visible);
}

/*
 * Return a pointer to a widget's parent window.
 * The agDrivers VFS must be locked.
 */
AG_Window *
AG_ParentWindow(void *_Nonnull obj)
{
	AG_OBJECT_ISA(obj, "AG_Widget:*");
	return (WIDGET(obj)->window);
}

/* Set an explicit widget position in pixels. */
void
AG_WidgetSetPosition(void *_Nonnull obj, int x, int y)
{
	AG_Widget *wid = obj;

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);
	wid->x = x;
	wid->y = y;
	AG_WidgetUpdate(wid);
	AG_ObjectUnlock(wid);
}

/* Set an explicit widget geometry in pixels. */
void
AG_WidgetSetSize(void *_Nonnull obj, int w, int h)
{
	AG_Widget *wid = obj;

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);
	wid->w = w;
	wid->h = h;
	AG_WidgetUpdate(wid);
	AG_ObjectUnlock(wid);
}

/* Set an explicit widget geometry from an AG_Rect argument. */
void
AG_WidgetSetGeometry(void *obj, const AG_Rect *r)
{
	AG_Widget *wid = obj;

	AG_OBJECT_ISA(wid, "AG_Widget:*");
	AG_ObjectLock(wid);
	wid->x = r->x;
	wid->y = r->y;
	wid->w = r->w;
	wid->h = r->h;
	AG_WidgetUpdate(wid);
	AG_ObjectUnlock(wid);
}

/* Set the largest allowable window size. */
void
AG_WindowSetGeometryMax(AG_Window *_Nonnull win)
{
	Uint wMax, hMax;

	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	AG_GetDisplaySize((void *)WIDGET(win)->drv, &wMax, &hMax);
	AG_WindowSetGeometry(win, 0, 0, wMax, hMax);
}

/* Request widget redraw. */
void
AG_Redraw(void *_Nonnull obj)
{
	AG_Window *win;

	AG_OBJECT_ISA(obj, "AG_Widget:*");
#ifdef DEBUG_REDRAW
	Debug(obj, "Redraw %s @ t=%u\n",
	    WIDGET(obj)->window ? OBJECT(WIDGET(obj)->window)->name : "(null)",
	    AG_GetTicks());
#endif
	if ((win = WIDGET(obj)->window) != NULL) {
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
		win->dirty = 1;
	}
}

/*
 * Alternate interface to AG_MapCursor(). Create a new mapping or
 * update the rectangle of an existing one.
 */
void
AG_SetCursor(void *obj, AG_CursorArea **ca, const AG_Rect *_Nonnull r,
    struct ag_cursor *c)
{
	AG_OBJECT_ISA(obj, "AG_Widget:*");

	if (*ca == NULL) {
		*ca = AG_MapCursor(obj, r, c);
	} else {
		(*ca)->r = *r;
	}
}

/*
 * Select one of the standard cursors. If the given cursor doesn't exist,
 * return NULL into *ca without any error message.
 */
void
AG_SetStockCursor(void *obj, AG_CursorArea **ca, const AG_Rect *r, int cName)
{
	AG_OBJECT_ISA(obj, "AG_Widget:*");

	if (*ca == NULL) {
		*ca = AG_MapStockCursor(obj, r, cName);
	} else {
		(*ca)->r = *r;
	}
}

/*
 * Process synchronous window operations. This includes focus changes,
 * visibility changes and the detach operation. Called from custom event
 * loops or driver code, after all queued events have been processed.
 */
void
AG_WindowProcessQueued(void)
{
	AG_LockVFS(&agDrivers);
	if (agWindowToFocus != NULL) { AG_WindowProcessFocusChange(); }
	if (!AG_TAILQ_EMPTY(&agWindowShowQ)) { AG_WindowProcessShowQueue(); }
	if (!AG_TAILQ_EMPTY(&agWindowHideQ)) { AG_WindowProcessHideQueue(); }
	if (!AG_TAILQ_EMPTY(&agWindowDetachQ)) { AG_WindowProcessDetachQueue(); }
	AG_UnlockVFS(&agDrivers);
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

static void
Init(void *_Nonnull obj)
{
	AG_Window *win = obj;
	int i;
#ifdef AG_DEBUG
	int debugLvlSave;
#endif

	win->flags = AG_WINDOW_NOCURSORCHG;
#ifdef AG_DEBUG
	memset(win->caption, 0, sizeof(win->caption));
#else
	win->caption[0] = '\0';
#endif
	win->visible = 0;
	win->dirty = 0;
	win->alignment = AG_WINDOW_ALIGNMENT_NONE;
	win->tbar = NULL;
	win->icon = NULL;
	win->spacing = 3;
	win->tPad = 2;
	win->bPad = 2;
	win->lPad = 2;
	win->rPad = 2;
	win->wReq = 0;
	win->hReq = 0;
	win->wMin = win->lPad + win->rPad + 16;
	win->hMin = win->tPad + win->bPad + 16;
	win->wBorderBot = agWindowBotBorderDefault;
	win->wBorderSide = agWindowSideBorderDefault;
	win->wResizeCtrl = 16;
	win->r.x = 0;
	win->r.y = 0;
	win->r.w = 0;
	win->r.h = 0;
	win->rSaved.x = -1;
	win->rSaved.y = -1;
	win->rSaved.w = -1;
	win->rSaved.h = -1;
	win->minPct = 50;
	win->nFocused = 0;
	win->widExclMotion = NULL;
	win->wmType = AG_WINDOW_WM_NORMAL;
	win->zoom = AG_ZOOM_DEFAULT;
	win->parent = NULL;
	win->transientFor = NULL;
	win->pinnedTo = NULL;

	TAILQ_INIT(&win->pvt.subwins);

	win->pvt.fadeInTime = 0.06f;
	win->pvt.fadeInIncr = 0.2f;
	win->pvt.fadeOutTime = 0.06f;
	win->pvt.fadeOutIncr = 0.2f;
	win->pvt.fadeOpacity = 1.0f;
	AG_InitTimer(&win->pvt.fadeTo, "fade", 0);

	TAILQ_INIT(&win->pvt.cursorAreas);
	for (i = 0; i < 5; i++)
		win->pvt.caResize[i] = NULL;

	AG_SetEvent(win, "window-gainfocus", OnGainFocus, NULL);
	AG_SetEvent(win, "window-lostfocus", OnLostFocus, NULL);

	/*
	 * We wish to forward incoming `widget-shown', `widget-hidden' and
	 * `detached' events to all attached child widgets.
	 */
	AG_SetEvent(win, "widget-shown", OnShow, NULL);
	AG_SetEvent(win, "widget-hidden", OnHide, NULL);
	AG_SetEvent(win, "detached", OnDetach, NULL);

	Debug_Mute(debugLvlSave);

	/* Use custom attach/detach hooks to keep the window stack in order. */
	AG_SetFn(win, "attach-fn", Attach, NULL);
	AG_SetFn(win, "detach-fn", Detach, NULL);
	
	/* Set the inheritable style defaults. */
	AG_SetString(win,  "font-family", OBJECT(agDefaultFont)->name);
	AG_SetStringF(win, "font-size",   "%.02fpts", agDefaultFont->spec.size);
	AG_SetString(win,  "font-weight", "normal");
	AG_SetString(win,  "font-style",  "normal");
	
	Debug_Unmute(debugLvlSave);

	WIDGET(win)->font = agDefaultFont;
	WIDGET(win)->pal = agDefaultPalette;
}

#if defined(AG_WIDGETS) && defined(AG_DEBUG)
static void
WindowCaptionChanged(AG_Event *event)
{
	char caption[AG_WINDOW_CAPTION_MAX];
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_TextboxCopyString(tb, caption, sizeof(caption));
	AG_WindowSetCaptionS(win, caption);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	static const AG_FlagDescr flagDescr[] = {
	    { AG_WINDOW_MODAL,		N_("Application-modal"),	0 },
	    { AG_WINDOW_MAXIMIZED,	N_("Maximized"),		0 },
	    { AG_WINDOW_MINIMIZED,	N_("Minimized"),		0 },
	    { AG_WINDOW_KEEPABOVE,	N_("Keep above others"),	1 },
	    { AG_WINDOW_KEEPBELOW,	N_("Keep below others"),	1 },
	    { AG_WINDOW_DENYFOCUS,	N_("Deny focus"),		1 },
	    { AG_WINDOW_NOHRESIZE,	N_("No horizontal resize"),	1 },
	    { AG_WINDOW_NOVRESIZE,	N_("No vertical resize"),	1 },
	    { AG_WINDOW_NOBACKGROUND,	N_("Disable background"),	1 },
	    { AG_WINDOW_MAIN,		N_("Main window"),		1 },
	    { AG_WINDOW_NOMOVE,		N_("Unmoveable"),		1 },
	    { AG_WINDOW_MODKEYEVENTS,	N_("Mod keys generate events"),	1 },
	    { AG_WINDOW_NOCURSORCHG,	N_("Inhibit cursor changes"),	1 },
	    { AG_WINDOW_USE_TEXT,	N_("Using the font engine"),	0 },
	    { 0,			NULL,				0 }
	};
	AG_Window *tgt = obj;
	AG_Box *box, *hBox, *lBox, *rBox;
	AG_Textbox *tb;
	const Uint nuFl = AG_NUMERICAL_SLOW;

	box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	tb = AG_TextboxNewS(box, AG_TEXTBOX_HFILL, _("Caption: "));
	AG_TextboxSizeHint(tb, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXX>");
	AG_TextboxBindUTF8(tb, tgt->caption, sizeof(tgt->caption));
	AG_SetEvent(tb, "textbox-postchg", WindowCaptionChanged, "%p", tgt);
	AG_SetStyle(tb, "font-size", "140%");

	hBox = AG_BoxNewHoriz(box, AG_BOX_EXPAND);
	lBox = AG_BoxNewVert(hBox, 0);

	AG_WidgetDisable(AG_CheckboxNewInt(lBox, 0, _("Is visible"), &tgt->visible));
	AG_WidgetDisable(AG_CheckboxNewInt(lBox, 0, _("Is dirty"), &tgt->dirty));

	AG_CheckboxSetFromFlags(lBox, 0, &tgt->flags, flagDescr);

	rBox = AG_BoxNewVert(hBox, AG_BOX_EXPAND);

	AG_LabelNewPolled(rBox, AG_LABEL_SLOW | AG_LABEL_HFILL,
	                  "X=%i Y=%i W=%i H=%i Zoom=%i",
			  &WIDGET(tgt)->x,
			  &WIDGET(tgt)->y,
			  &tgt->r.w,
			  &tgt->r.h,
			  &tgt->zoom);

	AG_LabelNewPolledMT(rBox, AG_LABEL_SLOW | AG_LABEL_HFILL,
	                    &OBJECT(tgt)->pvt.lock,
	                    _("Parent: %[objName] @ (AG_Window *)%p"),
	                    &tgt->parent, &tgt->parent);

	AG_SeparatorNewHoriz(rBox);

#if 0
	AG_LabelNewPolledMT(rBox, AG_LABEL_SLOW | AG_LABEL_HFILL,
	                    &OBJECT(tgt)->pvt.lock,
	                    _("Transient for: %[objName] @ (AG_Window *)%p"),
	                    &tgt->transientFor, &tgt->transientFor);
	AG_LabelNewPolledMT(rBox, AG_LABEL_SLOW | AG_LABEL_HFILL,
	                    &OBJECT(tgt)->pvt.lock,
	                   _("Pinned to: %[objName] @ (AG_Window *)%p"),
	                   &tgt->pinnedTo, &tgt->pinnedTo);
#endif

#if 0
	AG_NumericalNewInt(rBox, nuFl, NULL, _("View X: "), &WIDGET(tgt)->x);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("View Y: "), &WIDGET(tgt)->y);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("View W: "), &tgt->r.w);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("View H: "), &tgt->r.h);
	AG_NumericalNewInt(rBox, 0, NULL, _("Saved X: "), &tgt->rSaved.x);
	AG_NumericalNewInt(rBox, 0, NULL, _("Saved Y: "), &tgt->rSaved.y);
	AG_NumericalNewInt(rBox, 0, NULL, _("Saved W: "), &tgt->rSaved.w);
	AG_NumericalNewInt(rBox, 0, NULL, _("Saved H: "), &tgt->rSaved.h);
#endif
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Spacing: "), &tgt->spacing);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Padding Left: "), &tgt->lPad);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Padding Right: "), &tgt->rPad);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Padding Top: "), &tgt->tPad);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Padding Bottom: "), &tgt->bPad);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Minimum W: "), &tgt->wMin);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Minimum H: "), &tgt->hMin);
#if 0
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Bottom Border W: "), &tgt->wBorderBot);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Side Border W: "), &tgt->wBorderSide);
	AG_NumericalNewInt(rBox, nuFl, NULL, _("Resize Ctrl W: "), &tgt->wResizeCtrl);
#endif

	if (tgt->flags & AG_WINDOW_MINSIZEPCT)
		AG_NumericalNewInt(rBox, nuFl, "%", _("Minimum Pct: "), &tgt->minPct);
#if 0	
	AG_SpacerNewHoriz(rBox);

	AG_LabelNewS(rBox, 0, _("Initial window position:"));
	AG_RadioNewUint(rBox, 0, agWindowAlignmentNames, &tgt->alignment);
# ifdef AG_WM_HINTS
	AG_LabelNewS(rBox, 0, _("EWMH Window Type:"));
	AG_RadioNewUint(rBox, 0, agWindowWmTypeNames, &tgt->wmType);
# endif
#endif
	AG_SetStyle(box, "font-size", "80%");
	return (box);
}
#endif /* AG_WIDGETS and AG_DEBUG */

AG_WidgetClass agWindowClass = {
	{
		"Agar(Widget:Window)",
		sizeof(AG_Window),
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
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

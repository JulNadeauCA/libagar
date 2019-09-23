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

/*
 * Micro-Agar Window.
 */

#include <agar/core/core.h>

#include <agar/micro/gui.h>
#include <agar/micro/window.h>
#include <agar/micro/primitive.h>

#include <string.h>
#include <stdarg.h>

MA_WindowQ maWindowDetachQ;			/* Windows to detach */
MA_WindowQ maWindowShowQ;			/* Windows to show */
MA_WindowQ maWindowHideQ;			/* Windows to hide */
MA_Window *maWindowToFocus = NULL;		/* Window to focus */
MA_Window *maWindowFocused = NULL;		/* Window holding focus */

void
MA_InitWindowSystem(void)
{
	TAILQ_INIT(&maWindowDetachQ);
	TAILQ_INIT(&maWindowShowQ);
	TAILQ_INIT(&maWindowHideQ);
	maWindowToFocus = NULL;
	maWindowFocused = NULL;
}

/*
 * Initialize a new window.
 * Called internally by MA_WindowNew().
 */
static __inline__ void
InitWindow(MA_Window *win, Uint flags)
{
	static Uint winNo = 0;

	AG_ObjectInit(win, &maWindowClass);
	AG_ObjectSetName(win, "win%u", winNo++);

	win->flags |= flags;

	/* NORESIZE implies NOMAXIMIZE */
	if (flags & MA_WINDOW_NORESIZE)
		win->flags |= MA_WINDOW_NOMAXIMIZE;

	/*
	 * Set the default "close" action to MAWINDETACH(), a canned
	 * event handler which calls AG_ObjectDetach(win).
	 */
	AG_SetEvent(win, "window-close", MAWINDETACH(win));
}

/*
 * Create a new Agar window. On success return a pointer to the newly
 * allocated window. If not successful, fail and return NULL.
 */
MA_Window *
MA_WindowNew(Uint16 flags)
{
	MA_Window *win;

	if ((win = TryMalloc(sizeof(MA_Window))) == NULL) {
		return (NULL);
	}
	InitWindow(win, flags);

	if (maDriver)
		AG_ObjectAttach(maDriver, win);

	return (win);
}

/* Special implementation of AG_ObjectAttach() for MA_Window. */
static void
Attach(AG_Event *_Nonnull event)
{
	MA_Window *win = MA_WINDOW_SELF();

	if (win->flags & MA_WINDOW_KEEPBELOW) {
		TAILQ_INSERT_HEAD(&OBJECT(maDriver)->children,
		                   OBJECT(win), cobjs);
	} else {
		TAILQ_INSERT_TAIL(&OBJECT(maDriver)->children,
		                   OBJECT(win), cobjs);
	}

	AG_PostEvent(win, "attached", NULL);
}

/* Special implementation of AG_ObjectDetach() for MA_Window. */
static void
Detach(AG_Event *_Nonnull event)
{
	MA_Window *win = MA_WINDOW_SELF();
	MA_Window *other, *subwin;
#ifdef AG_TIMERS
	AG_Timer *to, *toNext;
#endif
	/* Mark window detach in progress */
	win->flags |= MA_WINDOW_DETACHING;

#ifdef AG_TIMERS
	/* Cancel any running timer attached to the window. */
	for (to = TAILQ_FIRST(&OBJECT(win)->timers);
	     to != TAILQ_END(&OBJECT(win)->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, timers);
		AG_DelTimer(win, to);
	}
#endif
	MA_FOREACH_WINDOW(other, maDriver) {
		if (other == win) {
			continue;
		}
		TAILQ_FOREACH(subwin, &other->subwins, swins) {
			if (subwin == win)
				break;
		}
		if (subwin) {
			TAILQ_REMOVE(&other->subwins, subwin, swins);
		}
		if (other->parent == win)
			AG_ObjectDetach(other);
	}

 	/*
	 * We must defer the actual window hide / detach operation
	 * until the end of the current event processing cycle.
	 */
	TAILQ_INSERT_TAIL(&maWindowDetachQ, win, detach);

	/* Queued Show/Hide operations would be redundant. */
	TAILQ_FOREACH(other, &maWindowHideQ, visibility) {
		if (other == win) {
			TAILQ_REMOVE(&maWindowHideQ, win, visibility);
			break;
		}
	}
	TAILQ_FOREACH(other, &maWindowShowQ, visibility) {
		if (other == win) {
			TAILQ_REMOVE(&maWindowShowQ, win, visibility);
			break;
		}
	}
}

/*
 * Make a window a logical child of the specified window. If the logical
 * parent window is detached, its child windows will be automatically
 * detached along with it.
 */
void
MA_WindowAttach(MA_Window *winParent, MA_Window *winChld)
{
	if (winParent == NULL)
		return;

	if (winChld->parent) {
		if (winChld->parent == winParent) {
			return;
		}
		MA_WindowDetach(winChld->parent, winChld);
	}
	winChld->parent = winParent;
	TAILQ_INSERT_HEAD(&winParent->subwins, winChld, swins);
}

/* Detach a window from its logical parent. */
void
MA_WindowDetach(MA_Window *winParent, MA_Window *winChld)
{
	if (winParent == NULL)
		return;

	TAILQ_REMOVE(&winParent->subwins, winChld, swins);
	winChld->parent = NULL;
}

static void
Draw(void *_Nonnull obj)
{
#if 0
	MA_Window *win = obj;
	MA_Widget *chld;

	/* TODO Title */
	/* TODO background */
	/* TODO border */

	OBJECT_FOREACH_CHILD(chld, win, ma_widget)
		MA_WidgetDraw(chld);
#endif
	(void) obj;
}

/* Handler for "widget-shown", generated when the window becomes visible. */
static void
OnShow(AG_Event *_Nonnull event)
{
	MA_Window *win = MA_WINDOW_SELF();
	AG_Object *chld;
	MA_SizeReq r;
	MA_SizeAlloc a;
	
	win->visible = 1;

	if (win->x == -1 || win->y == -1) {
		/*
		 * No explicit window geometry was provided; compute a
		 * default size from the widget sizeReq() operations,
		 * and apply initial window alignment settings.
		 */
		MA_WidgetSizeReq(win, &r);
		a.x = 0;
		a.y = 0;
		a.w = r.w;
		a.h = r.h;
	} else {
		a.x = win->x;
		a.y = win->y;
		a.w = win->w;
		a.h = win->h;
	}
	MA_WidgetSizeAlloc(win, &a);
	
	OBJECT_FOREACH_CHILD(chld, win, ag_object) {	/* Notify all widgets */
		AG_ForwardEvent(chld, event);
	}
	AG_PostEvent(win, "window-shown", NULL);

	if (!(win->flags & MA_WINDOW_DENYFOCUS))
		maWindowToFocus = win;

	/* Mark for redraw */
	win->flags |= MA_WINDOW_REDRAW;
}

/* Handler for "widget-hidden", generated when the window is to be unmapped. */
static void
OnHide(AG_Event *_Nonnull event)
{
	MA_Window *win = MA_WINDOW_SELF();
	AG_Object *chld;

	win->visible = 0;
	WIDGET(win)->flags &= ~(MA_WIDGET_VISIBLE);
	win->flags &= ~(MA_WINDOW_REDRAW);
	
	/* Cancel focus state or any focus change requests. */
	if (win == maWindowToFocus)
		maWindowToFocus = NULL;

	if (win == maWindowFocused) {
		MA_Window *wOther;

		AG_PostEvent(win, "window-lostfocus", NULL);
		maWindowFocused = NULL;
		
		MA_FOREACH_WINDOW_REVERSE(wOther, maDriver) {
			if (wOther->visible &&
			  !(wOther->flags & MA_WINDOW_DENYFOCUS)) {
				break;
			}
			if (wOther)
				maWindowToFocus = wOther;
		}
		/* TODO wipe BG */
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
	MA_Window *win = MA_WINDOW_SELF();
	AG_Object *chld;

	/* Propagate to child objects */
	OBJECT_FOREACH_CHILD(chld, win, ag_object) {
/*		AG_Debug(win, "Broadcasting \"detached\"() to %s\n", chld->name); */

		event->argv[1].data.p = win;	/* Make SENDER */
		AG_ForwardEvent(chld, event);
	}
}

static void
WidgetGainFocus(MA_Widget *_Nonnull wid)
{
	MA_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ma_widget) {
		WidgetGainFocus(chld);
	}
	if (wid->flags & MA_WIDGET_FOCUSED)
		AG_PostEvent(wid, "widget-gainfocus", NULL);
}

static void
WidgetLostFocus(MA_Widget *_Nonnull wid)
{
	MA_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ma_widget) {
		WidgetLostFocus(chld);
	}
	if (wid->flags & MA_WIDGET_FOCUSED)
		AG_PostEvent(wid, "widget-lostfocus", NULL);
}

static void
OnGainFocus(AG_Event *_Nonnull event)
{
	MA_Window *win = MA_WINDOW_SELF();

/*	Verbose("%s (\"%s\"): Gained Focus\n", OBJECT(win)->name, win->caption); */
	WidgetGainFocus(WIDGET(win));
}

static void
OnLostFocus(AG_Event *_Nonnull event)
{
	MA_Window *win = MA_WINDOW_SELF();

/*	Verbose("%s (\"%s\"): Lost Focus\n", OBJECT(win)->name, win->caption); */
	WidgetLostFocus(WIDGET(win));
}

/* Lower a window to the bottom of the stack. */
void
MA_WindowLower(MA_Window *win)
{
	Debug(maDriver, "Lowering %s to bottom\n", OBJECT(win)->name);
	TAILQ_REMOVE(&OBJECT(maDriver)->children, OBJECT(win), cobjs);
	TAILQ_INSERT_HEAD(&OBJECT(maDriver)->children, OBJECT(win), cobjs);
}

/* Raise a window to the top of the stack. */
void
MA_WindowRaise(MA_Window *win)
{
	Debug(maDriver, "Raising %s to top\n", OBJECT(win)->name);
	TAILQ_REMOVE(&OBJECT(maDriver)->children, OBJECT(win), cobjs);
	TAILQ_INSERT_TAIL(&OBJECT(maDriver)->children, OBJECT(win), cobjs);
}

/* Make a window visible to the user. */
void
MA_WindowShow(MA_Window *win)
{
	if (win->visible)
		return;
	
	AG_PostEvent(win, "widget-shown", NULL);
}

/* Make a window invisible to the user. */
void
MA_WindowHide(MA_Window *win)
{
	if (!win->visible)
		return;

	AG_PostEvent(win, "widget-hidden", NULL);
	win->visible = 0;
	WIDGET(win)->flags &= ~(MA_WIDGET_VISIBLE);
}

/*
 * Render all windows that need to be redrawn. This is typically invoked
 * by the main event loop, once events have been processed.
 */ 
void
MA_WindowDrawQueued(void)
{
	MA_Window *win;

	MA_FOREACH_WINDOW(win, maDriver) {
		if (win->visible && (win->flags & MA_WINDOW_REDRAW))
			break;
	}
	if (win) {
		maRenderingContext = 1;
		maDriverOps->beginRendering();

		MA_FOREACH_WINDOW(win, maDriver)
			MA_WindowDraw(win);

		maDriverOps->endRendering();
		maRenderingContext = 0;
	}
}

/*
 * Give input focus to a window. The actual focus change will take effect at
 * the end of the current event cycle.
 */
void
MA_WindowFocus(MA_Window *win)
{
	if (win == NULL) {
		maWindowToFocus = NULL;
		return;
	}
	if (win->flags & MA_WINDOW_DENYFOCUS) {
		return;
	}
	if (OBJECT(win)->parent != NULL) {
		maWindowToFocus = win;
	}
	win->flags |= MA_WINDOW_REDRAW;
}

/* Set the coordinates and geometry of a window. */
Sint8
MA_WindowSetGeometry(MA_Window *win, int x, int y, int w, int h)
{
	MA_SizeReq rWin;
	MA_SizeAlloc a;
	Uint8 new;
	Uint16 nw, nh;
	Uint16 wMin, hMin;
	
	new = ((win->x == 0 || win->y == 0));

	/* Compute the final window size. */
	if (w == 0 || h == 0) {					/* Auto */
		MA_WidgetSizeReq(win, &rWin);
		nw = (w == 0) ? rWin.w : w;
		nh = (h == 0) ? rWin.h : h;
	} else {
		nw = w;
		nh = h;
	}

	wMin = MA_WINDOW_MIN_W(win->min) << 1;
	hMin = MA_WINDOW_MIN_H(win->min) << 1;
	if (nw < wMin) { nw = wMin; }
	if (nh < hMin) { nh = hMin; }

	if (win->x == -1 || win->y == -1) {
		int wDisp, hDisp;

		MA_GetDisplaySize(&wDisp, &hDisp);
		if (win->x == -1) { win->x = (wDisp >> 1) - (nw >> 1); }
		if (win->y == -1) { win->y = (hDisp >> 1) - (nh >> 1); }
	}
	a.x = x;
	a.y = y;
	a.w = nw;
	a.h = nh;

	/*
	 * Resize the widgets and update their coordinates; update
	 * the geometry of the window's Widget structure.
	 */
	MA_WidgetSizeAlloc(win, &a);

	win->flags |= MA_WINDOW_REDRAW;
	return (0);
}

/* Maximize a window */
void
MA_WindowMaximize(MA_Window *win)
{
	int wMax, hMax;

	MA_GetDisplaySize(&wMax, &hMax);

	if (MA_WindowSetGeometry(win, 0,0, wMax,hMax) == 0)
		win->flags |= MA_WINDOW_MAXIMIZED;
}

/* Restore a window's geometry prior to maximization. */
void
MA_WindowUnmaximize(MA_Window *win)
{
	/* TODO restore geometry */

	win->flags &= ~(MA_WINDOW_MAXIMIZED);
	win->flags |= MA_WINDOW_REDRAW;
}

/* Minimize a window */
void
MA_WindowMinimize(MA_Window *win)
{
	if (win->flags & MA_WINDOW_MINIMIZED) {
		return;
	}
	Debug(win, "Minimizing\n");
	win->flags |= MA_WINDOW_MINIMIZED;
	MA_WindowHide(win);
}

/* Unminimize a window */
void
MA_WindowUnminimize(MA_Window *win)
{
	if (!win->visible) {
		MA_WindowShow(win);
		win->flags &= ~(MA_WINDOW_MINIMIZED);
	} else {
		MA_WindowFocus(win);
	}
}

/* MAWINDETACH(): General-purpose "detach window" event handler. */
void
MA_WindowDetachGenEv(AG_Event *event)
{
	AG_ObjectDetach(MA_WINDOW_PTR(1));
}

/* MAWINHIDE(): General-purpose "hide window" event handler. */
void
MA_WindowHideGenEv(AG_Event *event)
{
	MA_WindowHide(MA_WINDOW_PTR(1));
}

/* AGWINCLOSE(): General-purpose "close window" event handler. */
void
MA_WindowCloseGenEv(AG_Event *event)
{
	AG_PostEvent(MA_WINDOW_PTR(1), "window-close", NULL);
}

/* Close the actively focused window. */
void
MA_CloseFocusedWindow(void)
{
	if (maWindowFocused)
		AG_PostEvent(maWindowFocused, "window-close", NULL);
}

void
MA_WindowSizeRequest(void *_Nonnull obj, MA_SizeReq *_Nonnull r)
{
	MA_Window *win = obj;
	MA_Widget *chld;
	MA_SizeReq rChld;
	Uint8 nWidgets=0;

	r->w = 0;
	r->h = 0;

	nWidgets = 0;
	OBJECT_FOREACH_CHILD(chld, win, ma_widget) {
		MA_WidgetSizeReq(chld, &rChld);
		r->w = MAX(r->w, rChld.w);
		r->h += rChld.h + win->spacing;
		nWidgets++;
	}
	if (nWidgets > 0 && r->h >= win->spacing)
		r->h -= win->spacing;
}

Sint8
MA_WindowSizeAllocate(void *_Nonnull obj, const MA_SizeAlloc *_Nonnull a)
{
	MA_Window *win = obj;
	MA_Widget *chld;
	MA_SizeReq rChld;
	MA_SizeAlloc aChld;
	const Uint8  lPad =                 MA_WINDOW_PAD_LEFT(win->padding);
	const Uint16 wAvail = a->w - lPad - MA_WINDOW_PAD_RIGHT(win->padding);
	const Uint8  tPad =                 MA_WINDOW_PAD_TOP(win->padding);
	const Uint16 hAvail = a->h - tPad - MA_WINDOW_PAD_BOTTOM(win->padding);
	Uint16 totFixed=0;
	Uint8 nWidgets=0;

	/* Calculate the space occupied by non-[HV]FILL widgets. */
	OBJECT_FOREACH_CHILD(chld, win, ma_widget) {
		MA_WidgetSizeReq(chld, &rChld);
		if ((chld->flags & MA_WIDGET_VFILL) == 0) {
			totFixed += rChld.h;
		}
		totFixed += win->spacing;
		nWidgets++;
	}
	if (nWidgets > 0 && totFixed >= win->spacing)
		totFixed -= win->spacing;

	aChld.x = lPad;
	aChld.y = tPad;

	OBJECT_FOREACH_CHILD(chld, win, ma_widget) {		/* Widgets */
		MA_WidgetSizeReq(chld, &rChld);

		aChld.w = (chld->flags & MA_WIDGET_HFILL) ?
		          wAvail : MIN(wAvail,rChld.w);

		aChld.h = (chld->flags & MA_WIDGET_VFILL) ?
		          hAvail-totFixed : rChld.h;

		MA_WidgetSizeAlloc(chld, &aChld);

		aChld.y += aChld.h + win->spacing;
	}
	win->x = 0;
	win->y = 0;
	win->w = a->w;
	win->h = a->h;
	return (0);
}

/*
 * Make windows on the show queue visible.
 */
void
MA_WindowProcessShowQueue(void)
{
	MA_Window *win;

	TAILQ_FOREACH(win, &maWindowShowQ, visibility) {
		AG_PostEvent(win, "widget-shown", NULL);
	}
	TAILQ_INIT(&maWindowShowQ);
}

/*
 * Make windows on the hide queue invisible.
 */
void
MA_WindowProcessHideQueue(void)
{
	MA_Window *win;

	TAILQ_FOREACH(win, &maWindowHideQ, visibility) {
		AG_PostEvent(win, "widget-hidden", NULL);
	}
	TAILQ_INIT(&maWindowHideQ);
}

/*
 * Close and destroy windows on the detach queue.
 */
void
MA_WindowProcessDetachQueue(void)
{
	MA_Window *win, *winNext;
	int nHidden=0;

	TAILQ_FOREACH(win, &maWindowDetachQ, detach) {
		if (!win->visible) {
			continue;
		}
		/*
		 * Note: `widget-hidden' event handlers may cause new windows
		 * to be added to maWindowDetachQ.
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

	for (win = TAILQ_FIRST(&maWindowDetachQ);
	     win != TAILQ_END(&maWindowDetachQ);
	     win = winNext) {
		winNext = TAILQ_NEXT(win, detach);

		/* Notify all widgets of the window detach. */
		AG_PostEvent(win, "detached", "%p", maDriver);

		/* Unset detach-fn and do a standard AG_ObjectDetach(). */
		AG_SetFn(win, "detach-fn", NULL, NULL);
		AG_ObjectDetach(win);

		AG_PostEvent(win, "window-detached", NULL);
		AG_ObjectDestroy(win);
	}
	TAILQ_INIT(&maWindowDetachQ);
}

/*
 * Render a window to the display (must be enclosed between calls to
 * MA_BeginRendering() and MA_EndRendering()).
 */
void
MA_WindowDraw(MA_Window *_Nonnull win)
{
	if (!win->visible) {
		return;
	}
	maDriverOps->renderWindow(win);
	win->flags &= ~(MA_WINDOW_REDRAW);
}

/*
 * Recompute the coordinates and geometries of all widgets attached to the
 * window. This is used following AG_ObjectAttach() and AG_ObjectDetach()
 * calls made in event context, or direct modifications to the x,y,w,h
 * fields of the Widget structure.
 */
void
MA_WindowUpdate(MA_Window *_Nonnull win)
{
	MA_SizeAlloc a;
	
	if (WIDGET(win)->w != 0 && WIDGET(win)->h != 0) {
		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDGET(win)->w;
		a.h = WIDGET(win)->h;
		MA_WidgetSizeAlloc(win, &a);
	}
}

/*
 * Process synchronous window operations. This includes focus changes,
 * visibility changes and the detach operation. Called from custom event
 * loops or driver code, after all queued events have been processed.
 */
void
MA_WindowProcessQueued(void)
{
	if (maWindowToFocus != NULL) {
		/* TODO */
/*		AG_WM_CommitWindowFocus(maWindowToFocus); */
		maWindowToFocus = NULL;
	}
	if (!AG_TAILQ_EMPTY(&maWindowShowQ)) { MA_WindowProcessShowQueue(); }
	if (!AG_TAILQ_EMPTY(&maWindowHideQ)) { MA_WindowProcessHideQueue(); }
	if (!AG_TAILQ_EMPTY(&maWindowDetachQ)) { MA_WindowProcessDetachQueue(); }
}

static void
Init(void *_Nonnull obj)
{
	MA_Window *win = obj;

	win->flags = 0;
	win->caption[0] = '\0';
	win->visible = 0;
	win->spacing = 4;
	win->padding = 0x4444;
	win->min = 0x88;
	win->w = 8;
	win->h = 8;
	win->parent = NULL;
	TAILQ_INIT(&win->subwins);

	AG_SetEvent(win, "window-gainfocus", OnGainFocus, NULL);
	AG_SetEvent(win, "window-lostfocus", OnLostFocus, NULL);

	/*
	 * We wish to forward incoming `widget-shown', `widget-hidden' and
	 * `detached' events to all attached child widgets.
	 */
	AG_SetEvent(win, "widget-shown", OnShow, NULL);
	AG_SetEvent(win, "widget-hidden", OnHide, NULL);
	AG_SetEvent(win, "detached", OnDetach, NULL);

	/* Use custom attach/detach hooks to keep the window stack in order. */
	AG_SetFn(win, "attach-fn", Attach, NULL);
	AG_SetFn(win, "detach-fn", Detach, NULL);
}

AG_ObjectClass maWindowClass = {
	"MA_Window",
	sizeof(MA_Window),
	{ 0,0 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};

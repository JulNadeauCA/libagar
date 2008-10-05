/*
 * Copyright (c) 2001-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "window.h"
#include "titlebar.h"
#include "icon.h"

#include "primitive.h"
#include "icons.h"
#include "cursors.h"

#include <string.h>
#include <stdarg.h>

static void ClampToView(AG_Window *);
static void Resize(int, AG_Window *, SDL_MouseMotionEvent *);
static void Move(AG_Window *, SDL_MouseMotionEvent *);
static void Shown(AG_Event *);
static void Hidden(AG_Event *);
static void GainFocus(AG_Event *);
static void LostFocus(AG_Event *);

AG_Mutex agWindowLock;
int agWindowCurX[AG_WINDOW_ALIGNMENT_LAST];
int agWindowCurY[AG_WINDOW_ALIGNMENT_LAST];
int agWindowXOutLimit = 32;
int agWindowBotOutLimit = 32;
int agWindowIconWidth = 32;
int agWindowIconHeight = 32;

void
AG_InitWindowSystem(void)
{
	int i;

	AG_MutexInit(&agWindowLock);
	for (i = 0; i < AG_WINDOW_ALIGNMENT_LAST; i++) {
		agWindowCurX[i] = 0;
		agWindowCurY[i] = 0;
	}
}

void
AG_DestroyWindowSystem(void)
{
	AG_MutexDestroy(&agWindowLock);
}

/* Create a generic window. */
AG_Window *
AG_WindowNew(Uint flags)
{
	AG_Window *win;
	Uint titlebarFlags = 0;

	win = Malloc(sizeof(AG_Window));
	AG_ObjectInit(win, &agWindowClass);
	AG_ObjectSetName(win, "win-generic");
	OBJECT(win)->flags &= ~(AG_OBJECT_NAME_ONATTACH);

	win->flags |= flags;

	if ((win->flags & AG_WINDOW_NOTITLE) == 0)
		win->hMin += agTextFontHeight;
	if (win->flags & AG_WINDOW_MODAL)
		win->flags |= AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOMINIMIZE;
	if (win->flags & AG_WINDOW_NORESIZE)
		win->flags |= AG_WINDOW_NOMAXIMIZE;
	if (win->flags & AG_WINDOW_NOCLOSE)
		titlebarFlags |= AG_TITLEBAR_NO_CLOSE;
	if (win->flags & AG_WINDOW_NOMINIMIZE)
		titlebarFlags |= AG_TITLEBAR_NO_MINIMIZE;
	if (win->flags & AG_WINDOW_NOMAXIMIZE)
		titlebarFlags |= AG_TITLEBAR_NO_MAXIMIZE;
	if ((win->flags & AG_WINDOW_NOTITLE) == 0)
		win->tbar = AG_TitlebarNew(win, titlebarFlags);
	if (win->flags & AG_WINDOW_NOBORDERS) {
		win->wBorderSide = 0;
		win->wBorderBot = 0;
	}

	AG_SetEvent(win, "window-close", AGWINDETACH(win));
	AG_ViewAttach(win);
	return (win);
}

/* Create a named window */
AG_Window *
AG_WindowNewNamed(Uint flags, const char *fmt, ...)
{
	char name[AG_OBJECT_NAME_MAX], *c;
	AG_Window *win;
	va_list ap;

	AG_LockVFS(agView);
	va_start(ap, fmt);
	Vsnprintf(name, sizeof(name), fmt, ap);
	va_end(ap);
	for (c = &name[0]; *c != '\0'; c++) {
		if (*c == '/')
			*c = '_';
	}
	if (AG_WindowFocusNamed(name)) {
		win = NULL;
		goto out;
	}
	win = AG_WindowNew(flags);
	AG_ObjectSetName(win, "%s", name);
	AG_SetEvent(win, "window-close", AGWINHIDE(win));
out:
	AG_UnlockVFS(agView);
	return (win);
}

static void
ChildAttached(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Widget *wid = AG_PTR(1);

	if (win->visible) {
		AG_WindowUpdate(win);
		AG_PostEvent(NULL, wid, "widget-shown", NULL);
	}
}

static void
Init(void *obj)
{
	AG_Window *win = obj;
	AG_Event *ev;

	win->flags = 0;
	win->visible = 0;
	win->alignment = AG_WINDOW_CENTER;
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
	win->wBorderBot = 6;
	win->wBorderSide = 0;
	win->wResizeCtrl = 16;

	win->rSaved = AG_RECT(-1,-1,-1,-1);
	win->caption[0] = '\0';
	win->tbar = NULL;
	win->icon = AG_IconNew(NULL, 0);
	TAILQ_INIT(&win->subwins);
	AG_IconSetSurfaceNODUP(win->icon, agIconWindow.s);
	AG_IconSetBackgroundFill(win->icon, 1, AG_COLOR(BG_COLOR));

	AG_SetEvent(win, "window-gainfocus", GainFocus, NULL);
	AG_SetEvent(win, "window-lostfocus", LostFocus, NULL);
	ev = AG_SetEvent(win, "widget-shown", Shown, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "widget-hidden", Hidden, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "detached", NULL, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	AG_SetEvent(win, "child-attached", ChildAttached, NULL);
}

/* Attach a sub-window. */
void
AG_WindowAttach(AG_Window *win, AG_Window *subwin)
{
	if (win == NULL) {
		return;
	}
	AG_LockVFS(agView);
	AG_ObjectLock(win);
	TAILQ_INSERT_HEAD(&win->subwins, subwin, swins);
	AG_ObjectUnlock(win);
	AG_UnlockVFS(agView);
}

/* Detach a sub-window. */
void
AG_WindowDetach(AG_Window *win, AG_Window *subwin)
{
	if (win == NULL) {
		return;
	}
	AG_LockVFS(agView);
	AG_ObjectLock(win);
	TAILQ_REMOVE(&win->subwins, subwin, swins);
	AG_ObjectUnlock(win);
	AG_UnlockVFS(agView);
}

static void
Draw(void *obj)
{
	AG_Window *win = obj;
	AG_Widget *chld;
	int hTitle = 0;
	
	STYLE(win)->Window(win);

	hTitle = (win->tbar != NULL) ? HEIGHT(win->tbar) : 0;
	AG_PushClipRect(win,
	    AG_RECT(0, 0,
	            WIDTH(win) - win->wBorderSide*2,
		    HEIGHT(win) - win->wBorderBot));

	WIDGET_FOREACH_CHILD(chld, win)
		AG_WidgetDraw(chld);
	
	AG_PopClipRect();
}

/* Apply initial alignment parameter. */
static void
ApplyAlignment(AG_Window *win)
{
	int xOffs = 0, yOffs = 0;

	AG_MutexLock(&agWindowLock);
	
	if (win->flags & AG_WINDOW_CASCADE) {
		xOffs = agWindowCurX[win->alignment];
		yOffs = agWindowCurY[win->alignment];
		agWindowCurX[win->alignment] += 16;
		agWindowCurY[win->alignment] += 16;
		if (agWindowCurX[win->alignment] > agView->w)
			agWindowCurX[win->alignment] = 0;
		if (agWindowCurY[win->alignment] > agView->h)
			agWindowCurY[win->alignment] = 0;
	}

	switch (win->alignment) {
	case AG_WINDOW_TL:
		WIDGET(win)->x = xOffs;
		WIDGET(win)->y = yOffs;
		break;
	case AG_WINDOW_TC:
		WIDGET(win)->x = agView->w/2 - WIDTH(win)/2 + xOffs;
		WIDGET(win)->y = 0;
		break;
	case AG_WINDOW_TR:
		WIDGET(win)->x = agView->w - WIDTH(win) - xOffs;
		WIDGET(win)->y = yOffs;
		break;
	case AG_WINDOW_ML:
		WIDGET(win)->x = xOffs;
		WIDGET(win)->y = agView->h/2 - HEIGHT(win)/2 + yOffs;
		break;
	case AG_WINDOW_MC:
		WIDGET(win)->x = agView->w/2 - WIDTH(win)/2 + xOffs;
		WIDGET(win)->y = agView->h/2 - HEIGHT(win)/2 + yOffs;
		break;
	case AG_WINDOW_MR:
		WIDGET(win)->x = agView->w - WIDTH(win) - xOffs;
		WIDGET(win)->y = agView->h/2 - HEIGHT(win)/2 + yOffs;
		break;
	case AG_WINDOW_BL:
		WIDGET(win)->x = xOffs;
		WIDGET(win)->y = agView->h - HEIGHT(win) - yOffs;
		break;
	case AG_WINDOW_BC:
		WIDGET(win)->x = agView->w/2 - WIDTH(win)/2 + xOffs;
		WIDGET(win)->y = agView->h - HEIGHT(win);
		break;
	case AG_WINDOW_BR:
		WIDGET(win)->x = agView->w - WIDTH(win) - xOffs;
		WIDGET(win)->y = agView->h - HEIGHT(win) - yOffs;
		break;
	default:
		break;
	}
	
	ClampToView(win);
	AG_MutexUnlock(&agWindowLock);
}

static void
Shown(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_SizeReq r;
	AG_SizeAlloc a;

	if ((win->flags & AG_WINDOW_DENYFOCUS) == 0) {
		AG_WindowFocus(win);
	}
	if (win->flags & AG_WINDOW_MODAL) {
		agView->winModal = Realloc(agView->winModal,
		    (agView->nModal+1) * sizeof(AG_Window *));
		agView->winModal[agView->nModal] = win;
		agView->nModal++;
	}
	if (WIDGET(win)->x == -1 && WIDGET(win)->y == -1) {
		AG_WidgetSizeReq(win, &r);
		if (r.w > agView->w) { r.w = agView->w; }
		if (r.h > agView->h) { r.h = agView->h; }
		a.x = 0;
		a.y = 0;
		a.w = r.w;
		a.h = r.h;
		AG_WidgetSizeAlloc(win, &a);
		ApplyAlignment(win);
	} else {
		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDTH(win);
		a.h = HEIGHT(win);
		AG_WidgetSizeAlloc(win, &a);
	}
	AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);
	AG_PostEvent(NULL, win, "window-shown", NULL);
	AG_PostEvent(NULL, win, "window-gainfocus", NULL);
}

static void
Hidden(AG_Event *event)
{
	AG_Window *win = AG_SELF();

	if ((win->flags & AG_WINDOW_DENYFOCUS) == 0) {
		/* Remove the focus. XXX cycle */
		agView->winToFocus = NULL;
	}
	if (win->flags & AG_WINDOW_MODAL) {
		agView->nModal--;
	}

	/* Update the background if necessary. */
	/* XXX TODO Avoid drawing over KEEPABOVE windows */
//	if (!AG_WindowIsSurrounded(win)) {
		if (!agView->opengl) {
			AG_DrawRectFilled(win,
			    AG_RECT(0,0, WIDTH(win), HEIGHT(win)),
			    AG_COLOR(BG_COLOR));
			AG_QueueVideoUpdate(
			    WIDGET(win)->x,
			    WIDGET(win)->y,
			    WIDTH(win),
			    HEIGHT(win));
		}
//	}
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

/*
 * Verify whether a given window resides inside the area of some other
 * window, to see whether is it necessary to update the background.
 *
 * The window's parent View must be locked.
 */
int
AG_WindowIsSurrounded(AG_Window *win)
{
	AG_Window *owin;

	TAILQ_FOREACH(owin, &agView->windows, windows) {
		AG_ObjectLock(owin);
		if (owin->visible &&
		    AG_WidgetArea(owin, WIDGET(win)->x, WIDGET(win)->y) &&
		    AG_WidgetArea(owin,
		     WIDGET(win)->x + WIDTH(win),
		     WIDGET(win)->y + HEIGHT(win))) {
			AG_ObjectUnlock(owin);
			return (1);
		}
		AG_ObjectUnlock(owin);
	}
	return (0);
}

/* Set the visibility state of a window. */
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
		AG_PostEvent(NULL, win, "widget-hidden", NULL);
	}
	AG_ObjectUnlock(win);
}

static void
AG_WindowCountWidgets(AG_Widget *wid, Uint *nwidgets)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_FOCUSABLE) {
		(*nwidgets)++;
	}
	AG_ObjectUnlock(wid);

	WIDGET_FOREACH_CHILD(cwid, wid)
		AG_WindowCountWidgets(cwid, nwidgets);
}

static void
AG_WindowMapWidgets(AG_Widget *wid, AG_Widget **widgets, Uint *i)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_FOCUSABLE) {
		widgets[(*i)++] = wid;
	}
	AG_ObjectUnlock(wid);

	WIDGET_FOREACH_CHILD(cwid, wid)
		AG_WindowMapWidgets(cwid, widgets, i);
}

/*
 * Move the widget focus inside a window.
 * The window must be locked.
 */
void
AG_WindowCycleFocus(AG_Window *win, int reverse)
{
	AG_Widget **widgets;
	AG_Widget *olfocus;
	Uint nwidgets = 0;
	Uint i = 0;

	if ((olfocus = AG_WidgetFindFocused(win)) == NULL) {
		return;
	}
	AG_WindowCountWidgets(WIDGET(win), &nwidgets);
	widgets = Malloc(nwidgets*sizeof(AG_Widget *));
	AG_WindowMapWidgets(WIDGET(win), widgets, &i);

	for (i = 0; i < nwidgets; i++) {
		if (widgets[i] == olfocus) {
			if (reverse) {
				if (i-1 >= 0) {
					AG_WidgetFocus(widgets[i-1]);
				} else if (i-1 < 0) {
					AG_WidgetFocus(widgets[nwidgets-1]);
				}
			} else {
				if (i+1 < nwidgets) {
					AG_WidgetFocus(widgets[i+1]);
				} else if (i+1 >= nwidgets) {
					AG_WidgetFocus(widgets[0]);
				}
			}
			break;
		}
	}
	Free(widgets);
}

/*
 * Clamp the window geometry/coordinates down to the view area.
 * The window must be locked.
 */
static void
ClampToView(AG_Window *win)
{
	AG_Widget *w = WIDGET(win);

	if (w->x < agWindowXOutLimit - w->w) {
		w->x = agWindowXOutLimit - w->w;
	} else if (w->x > agView->w - agWindowXOutLimit) {
		w->x = agView->w - agWindowXOutLimit;
	}
	if (w->y < 0) {
		w->y = 0;
	} else if (w->y > agView->h - agWindowBotOutLimit) {
		w->y = agView->h - agWindowBotOutLimit;
	}
#if 0
	if (w->x + w->w > agView->w) { w->x = agView->w - w->w; }
	if (w->y + w->h > agView->h) { w->y = agView->h - w->h; }
	if (w->x < 0) { w->x = 0; }
	if (w->y < 0) { w->y = 0; }

	if (w->x+w->w > agView->w) {
		w->x = 0;
		w->w = agView->w - 1;
	}
	if (w->y+w->h > agView->h) {
		w->y = 0;
		w->h = agView->h - 1;
	}
#endif
}

/*
 * Move a window using the mouse.
 * The view and window must be locked.
 */
static void
Move(AG_Window *win, SDL_MouseMotionEvent *motion)
{
	AG_Rect rPrev, rNew;
	SDL_Rect rFill1, rFill2;

	rPrev.x = WIDGET(win)->x;
	rPrev.y = WIDGET(win)->y;
	rPrev.w = WIDTH(win);
	rPrev.h = HEIGHT(win);

	WIDGET(win)->x += motion->xrel;
	WIDGET(win)->y += motion->yrel;
	ClampToView(win);

	AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);

	/* Update the background. */
	/* XXX TODO Avoid drawing over KEEPABOVE windows */
	rNew.x = WIDGET(win)->x;
	rNew.y = WIDGET(win)->y;
	rNew.w = WIDTH(win);
	rNew.h = HEIGHT(win);
	rFill1.w = 0;
	rFill2.w = 0;
	if (rNew.x > rPrev.x) {		/* Right */
		rFill1.x = rPrev.x;
		rFill1.y = rPrev.y;
		rFill1.w = rNew.x - rPrev.x;
		rFill1.h = rNew.h;
	} else if (rNew.x < rPrev.x) {	/* Left */
		rFill1.x = rNew.x + rNew.w;
		rFill1.y = rNew.y;
		rFill1.w = rPrev.x - rNew.x;
		rFill1.h = rPrev.h;
	}
	if (rNew.y > rPrev.y) {		/* Downward */
		rFill2.x = rPrev.x;
		rFill2.y = rPrev.y;
		rFill2.w = rNew.w;
		rFill2.h = rNew.y - rPrev.y;
	} else if (rNew.y < rPrev.y) {	/* Upward */
		rFill2.x = rPrev.x;
		rFill2.y = rNew.y + rNew.h;
		rFill2.w = rPrev.w;
		rFill2.h = rPrev.y - rNew.y;
	}
	if (!agView->opengl &&
	    !(win->flags & AG_WINDOW_NOUPDATERECT)) {
		if (rFill1.w > 0) {
			SDL_FillRect(agView->v, &rFill1, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rFill1);
		}
		if (rFill2.w > 0) {
			SDL_FillRect(agView->v, &rFill2, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rFill2);
		}
	}
	AG_PostEvent(NULL, win, "window-user-move", "%d,%d",
	    WIDGET(win)->x, WIDGET(win)->y);
}

/*
 * Give focus to a window. This will only take effect at the end
 * of the current event cycle.
 */
void
AG_WindowFocus(AG_Window *win)
{
	AG_LockVFS(agView);
	if (win == NULL) {
		agView->winToFocus = NULL;
		goto out;
	}
	AG_ObjectLock(win);
	if ((win->flags & AG_WINDOW_DENYFOCUS) == 0) {
		win->flags |= AG_WINDOW_FOCUSONATTACH;
		agView->winToFocus = win;
	}
	AG_ObjectUnlock(win);
out:
	AG_UnlockVFS(agView);
}

/* Give focus to a window by name, and show it is if is hidden. */
int
AG_WindowFocusNamed(const char *name)
{
	AG_Window *owin;
	int rv = 0;

	AG_LockVFS(agView);
	TAILQ_FOREACH(owin, &agView->windows, windows) {
		if (strlen(OBJECT(owin)->name) >= 4 &&
		    strcmp(OBJECT(owin)->name+4, name) == 0) {
			AG_WindowShow(owin);
			AG_WindowFocus(owin);
		    	rv = 1;
			goto out;
		}
	}
out:
	AG_UnlockVFS(agView);
	return (0);
}

/*
 * Check if the given coordinates overlap a window control, and if so
 * which operation is related to it.
 */
static __inline__ int
AG_WindowMouseOverCtrl(AG_Window *win, int x, int y)
{
	if ((y - WIDGET(win)->y) > (HEIGHT(win) - win->wBorderBot)) {
		int xRel = x - WIDGET(win)->x;
	    	if (xRel < win->wResizeCtrl) {
			return (AG_WINOP_LRESIZE);
		} else if (xRel > (WIDTH(win) - win->wResizeCtrl)) {
			return (AG_WINOP_RRESIZE);
		} else if ((win->flags & AG_WINDOW_NOVRESIZE) == 0) {
			return (AG_WINOP_HRESIZE);
		}
	}
	return (AG_WINOP_NONE);
}

/*
 * Dispatch events to widgets.
 * The view must be locked, and the window list must be nonempty.
 * Returns 1 if the event was processed, otherwise 0.
 */
int
AG_WindowEvent(SDL_Event *ev)
{
	extern SDL_Cursor *agCursorToSet;
	static AG_Window *keydown_win = NULL;	/* XXX hack */
	AG_Window *win;
	AG_Widget *wid, *wFoc;
	int focus_changed = 0, tabCycle;
	AG_Window *focus_saved = agView->winToFocus;
	int rv = 0;

	agCursorToSet = NULL;
	
	if (agView->nModal > 0) {
		/* Skip window manager operations if there is a modal window. */
		goto process;
	}
	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		/*
		 * Focus on the highest overlapping window, and continue
		 * with normal processing of the MOUSEBUTTONDOWN event.
		 */
		agView->winToFocus = NULL;
		TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq,
		    windows) {
			AG_ObjectLock(win);
			if (!win->visible ||
			    !AG_WidgetArea(win, ev->button.x, ev->button.y)) {
				AG_ObjectUnlock(win);
				continue;
			}
			if (win->flags & AG_WINDOW_DENYFOCUS) {
				agView->winToFocus = focus_saved;
			} else {
				agView->winToFocus = win;
				focus_changed++;
			}
			AG_ObjectUnlock(win);
			break;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		/* Terminate any window operation in progress. */
		agView->winop = AG_WINOP_NONE;
		break;
	}
process:
	/*
	 * Iterate over the visible windows and deliver the appropriate Agar
	 * events.
	 */
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
		AG_ObjectLock(win);
		/*
		 * If a modal window exists and a click is made outside of
		 * its area, send it a "window-modal-close" event.
		 */
		if (agView->nModal > 0) {
			if (win == agView->winModal[agView->nModal-1]) {
				if ((ev->type == SDL_MOUSEBUTTONDOWN) &&
				    !AG_WidgetArea(win, ev->button.x,
				    ev->button.y)) {
					AG_PostEvent(NULL, win,
					    "window-modal-close", NULL);
					rv = 1;
					AG_ObjectUnlock(win);
					goto out;
				}
			} else {
				AG_ObjectUnlock(win);
				continue;
			}
		}
		if (!win->visible) {
			AG_ObjectUnlock(win);
			continue;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			/* Process active MOVE or RESIZE operations. */
			if (agView->winop != AG_WINOP_NONE &&
			    agView->winSelected != win) {
				AG_ObjectUnlock(win);
				continue;
			}
			switch (agView->winop) {
			case AG_WINOP_NONE:
				break;
			case AG_WINOP_MOVE:
				Move(win, &ev->motion);
				AG_ObjectUnlock(win);
				rv = 1;
				goto out;
			case AG_WINOP_LRESIZE:
				Resize(AG_WINOP_LRESIZE, win, &ev->motion);
				AG_ObjectUnlock(win);
				goto out;
			case AG_WINOP_RRESIZE:
				Resize(AG_WINOP_RRESIZE, win, &ev->motion);
				AG_ObjectUnlock(win);
				goto out;
			case AG_WINOP_HRESIZE:
				Resize(AG_WINOP_HRESIZE, win, &ev->motion);
				AG_ObjectUnlock(win);
				goto out;
			default:
				break;
			}
			/*
			 * Forward this MOUSEMOTION to all widgets that either
			 * hold focus or have the AG_WIDGET_UNFOCUSED_MOTION
			 * flag set.
			 */
			WIDGET_FOREACH_CHILD(wid, win) {
				AG_ObjectLock(wid);
				if (wid->flags & AG_WIDGET_PRIO_MOTION) {
					AG_WidgetMouseMotion(win, wid,
					    ev->motion.x, ev->motion.y,
					    ev->motion.xrel, ev->motion.yrel,
					    ev->motion.state);
					AG_ObjectUnlock(wid);
					AG_ObjectUnlock(win);
					goto out;
				}
				AG_ObjectUnlock(wid);
			}
			WIDGET_FOREACH_CHILD(wid, win) {
				AG_WidgetMouseMotion(win, wid,
				    ev->motion.x, ev->motion.y,
				    ev->motion.xrel, ev->motion.yrel,
				    ev->motion.state);
			}
			/* Change the cursor if a RESIZE op is in progress. */
			if (agCursorToSet == NULL &&
			    (win->wBorderBot > 0) &&
			    !(win->flags & AG_WINDOW_NORESIZE) &&
			    AG_WidgetArea(win, ev->motion.x, ev->motion.y))
			{
				switch (AG_WindowMouseOverCtrl(win,
				    ev->motion.x, ev->motion.y))
				{
				case AG_WINOP_LRESIZE:
					AG_SetCursor(AG_LLDIAG_CURSOR);
					break;
				case AG_WINOP_RRESIZE:
					AG_SetCursor(AG_LRDIAG_CURSOR);
					break;
				case AG_WINOP_HRESIZE:
					AG_SetCursor(AG_VRESIZE_CURSOR);
					break;
				default:
					break;
				}
			}
			if (agCursorToSet == NULL) {
				/*
				 * Prevent widgets in other windows from
				 * changing the cursor.
				 */
				agCursorToSet = agDefaultCursor;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			/* Terminate active window operations. */
			/* XXX redundant? */
			if (agView->winop != AG_WINOP_NONE) {
				agView->winop = AG_WINOP_NONE;
				agView->winSelected = NULL;
			}
			/*
			 * Forward to all widgets that either hold focus or have
			 * the AG_WIDGET_UNFOCUSED_BUTTONUP flag set.
			 */
			WIDGET_FOREACH_CHILD(wid, win) {
				AG_WidgetMouseButtonUp(win, wid,
				    ev->button.button,
				    ev->button.x, ev->button.y);
			}
			if (focus_changed) {
				AG_ObjectUnlock(win);
				rv = 1;
				goto out;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!AG_WidgetArea(win, ev->button.x, ev->button.y)) {
				AG_ObjectUnlock(win);
				continue;
			}
			if (win->wBorderBot > 0 &&
			    !(win->flags & AG_WINDOW_NORESIZE)) {
				agView->winop = AG_WindowMouseOverCtrl(win,
				    ev->button.x, ev->button.y);
				if (agView->winop != AG_WINOP_NONE)
					agView->winSelected = win;
			}
			/* Forward to overlapping widgets. */
			WIDGET_FOREACH_CHILD(wid, win) {
				if (AG_WidgetMouseButtonDown(win, wid,
				    ev->button.button, ev->button.x,
				    ev->button.y)) {
					AG_ObjectUnlock(win);
					rv = 1;
					goto out;
				}
			}
			if (focus_changed) {
				AG_ObjectUnlock(win);
				rv = 1;
				goto out;
			}
			break;
		case SDL_KEYUP:
			if (keydown_win != NULL && keydown_win != win) {
				/*
				 * Key was initially pressed while another
				 * window was holding focus, ignore.
				 */
				keydown_win = NULL;
				break;
			}
			/* FALLTHROUGH */
		case SDL_KEYDOWN:
			switch (ev->type) {
			case SDL_KEYUP:
				AG_WidgetUnfocusedKeyUp(WIDGET(win),
				    (int)ev->key.keysym.sym,
				    (int)ev->key.keysym.mod,
				    (int)ev->key.keysym.unicode);
				break;
			case SDL_KEYDOWN:
				AG_WidgetUnfocusedKeyDown(WIDGET(win),
				    (int)ev->key.keysym.sym,
				    (int)ev->key.keysym.mod,
				    (int)ev->key.keysym.unicode);
				break;
			}
			switch (ev->key.keysym.sym) {		/* XXX */
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				/* Always ignore modifiers */
				AG_ObjectUnlock(win);
				return (0);
			default:
				break;
			}
			tabCycle = 1;
			if (AG_WINDOW_FOCUSED(win) &&
			   (wFoc = AG_WidgetFindFocused(win)) != NULL) {
				AG_ObjectLock(wFoc);
				if (ev->key.keysym.sym != SDLK_TAB ||
			            wFoc->flags & AG_WIDGET_CATCH_TAB) {
					if (wFoc->flags & AG_WIDGET_CATCH_TAB) {
						tabCycle = 0;
					}
					AG_PostEvent(NULL, wFoc,
					    (ev->type == SDL_KEYUP) ?
					    "window-keyup" : "window-keydown",
					    "%i, %i, %i",
					    (int)ev->key.keysym.sym,
					    (int)ev->key.keysym.mod,
					    (int)ev->key.keysym.unicode);
					/*
					 * Ensure the keyup event is posted to
					 * this window when the key is released,
					 * in case a keydown event handler
					 * changes the window focus.
					 */
					keydown_win = win;
					rv = 1;
				}
				AG_ObjectUnlock(wFoc);
			}
			if (tabCycle &&
			    ev->key.keysym.sym == SDLK_TAB &&
			    ev->type == SDL_KEYUP) {
				AG_WindowCycleFocus(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				rv = 1;
			}
			AG_ObjectUnlock(win);
			goto out;
		}
		AG_ObjectUnlock(win);
	}
	if (agCursorToSet != NULL && SDL_GetCursor() != agCursorToSet) {
		SDL_SetCursor(agCursorToSet);
	}
out:
	/*
	 * If requested, reorder the window list such that a new window
	 * holds focus.
	 * 
	 * The focus_changed flag is set if there was a focus change
	 * in reaction to a window operation (winToFocus can be NULL
	 * in that case).
	 */
	if (focus_changed || agView->winToFocus != NULL) {
		AG_Window *lastwin;

		lastwin = TAILQ_LAST(&agView->windows, ag_windowq);
		if (agView->winToFocus != NULL &&
		    agView->winToFocus == lastwin) {
			AG_PostEvent(NULL, agView->winToFocus,
			    "window-gainfocus", NULL);
			agView->winToFocus = NULL;
			goto outf;
		}

		if (lastwin != NULL) {
			if (lastwin->flags & AG_WINDOW_KEEPABOVE) {
				goto outf;
			}
			AG_PostEvent(NULL, lastwin, "window-lostfocus", NULL);
		}
		if (agView->winToFocus != NULL &&
		    agView->winToFocus != TAILQ_LAST(&agView->windows,
		    ag_windowq)) {
			TAILQ_REMOVE(&agView->windows, agView->winToFocus,
			    windows);
			TAILQ_INSERT_TAIL(&agView->windows, agView->winToFocus,
			    windows);
			AG_PostEvent(NULL, agView->winToFocus,
			    "window-gainfocus", NULL);
		}
		agView->winToFocus = NULL;
	}
outf:
	return (rv);
}

/* 
 * Set window coordinates and geometry. This should be used instead of
 * a direct WidgetSizeAlloc() since it redraws portions of the window
 * background when needed.
 */
int
AG_WindowSetGeometryRect(AG_Window *win, AG_Rect r, int bounded)
{
	AG_SizeReq rWin;
	AG_SizeAlloc aWin;
	SDL_Rect rFill;
	int new;
	int ox, oy, ow, oh;
	int nw, nh;
	int wMin, hMin;

	AG_ObjectLock(win);
	ox = WIDGET(win)->x;
	oy = WIDGET(win)->y;
	ow = WIDTH(win);
	oh = HEIGHT(win);
	new = ((WIDGET(win)->x == -1 || WIDGET(win)->y == -1));

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

	/* Limit the window to the view boundaries. */
	aWin.x = (r.x == -1) ? WIDGET(win)->x : r.x;
	aWin.y = (r.y == -1) ? WIDGET(win)->y : r.y;
	aWin.w = nw;
	aWin.h = nh;
	
	if (bounded) {
		if (aWin.x+aWin.w > agView->w) {
			aWin.w = agView->w - aWin.x;
		}
		if (aWin.y+aWin.h > agView->h) {
			aWin.h = agView->h - aWin.y;
		}
		if (aWin.w < 0) {
			aWin.x = 0;
			aWin.w = agView->w;
		}
		if (aWin.h < 0) {
			aWin.y = 0;
			aWin.h = agView->h;
		}
	}
	
	/* Size the widgets and update their coordinates. */
	if (AG_WidgetSizeAlloc(win, &aWin) == -1) {
		if (!new) {				/* Revert */
			aWin.x = ox;
			aWin.y = oy;
			aWin.w = ow;
			aWin.h = oh;
			AG_WidgetSizeAlloc(win, &aWin);
		}
		goto fail;
	}
	AG_WidgetUpdateCoords(win, aWin.x, aWin.y);

	/* Update the background. */
	/* XXX TODO Avoid drawing over KEEPABOVE windows */
	if (win->visible && !new && !agView->opengl &&
	    !(win->flags & AG_WINDOW_NOUPDATERECT)) {
		if (WIDGET(win)->x > ox) {			/* L-resize */
			rFill.x = ox;
			rFill.y = oy;
			rFill.w = WIDGET(win)->x - ox;
			rFill.h = HEIGHT(win);
		} else if (WIDTH(win) < ow) {			/* R-resize */
			rFill.x = WIDGET(win)->x + WIDTH(win);
			rFill.y = WIDGET(win)->y;
			rFill.w = ow - WIDTH(win);
			rFill.h = oh;
		} else {
			rFill.w = 0;
			rFill.h = 0;
		}
		if (rFill.w > 0 && rFill.h > 0) {
			SDL_FillRect(agView->v, &rFill, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rFill);
		}
		if (HEIGHT(win) < oh) {				/* H-resize */
			rFill.x = ox;
			rFill.y = WIDGET(win)->y + HEIGHT(win);
			rFill.w = ow;
			rFill.h = oh - HEIGHT(win);
			SDL_FillRect(agView->v, &rFill, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rFill);
		}
	}
	AG_ObjectUnlock(win);
	return (0);
fail:
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
	AG_ObjectUnlock(win);
}

/* Assign a window a specific alignment and size in pixels. */
int
AG_WindowSetGeometryAligned(AG_Window *win, enum ag_window_alignment align,
    int w, int h)
{
	int x, y;

	switch (align) {
	case AG_WINDOW_TL:
		x = 0;
		y = 0;
		break;
	case AG_WINDOW_TC:
		x = agView->w/2 - w/2;
		y = 0;
		break;
	case AG_WINDOW_TR:
		x = agView->w - w;
		y = 0;
		break;
	case AG_WINDOW_ML:
		x = 0;
		y = agView->h/2 - h/2;
		break;
	case AG_WINDOW_MR:
		x = agView->w - w;
		y = agView->h/2 - h/2;
		break;
	case AG_WINDOW_BL:
		x = 0;
		y = agView->h - h;
		break;
	case AG_WINDOW_BC:
		x = agView->w/2 - w/2;
		y = agView->h - h;
		break;
	case AG_WINDOW_BR:
		x = agView->w - w;
		y = agView->h - h;
		break;
	case AG_WINDOW_MC:
	default:
		x = agView->w/2 - w/2;
		y = agView->h/2 - h/2;
		break;
	}
	return AG_WindowSetGeometry(win, x, y, w, h);
}

int
AG_WindowSetGeometryAlignedPct(AG_Window *win, enum ag_window_alignment align,
    int wPct, int hPct)
{
	return AG_WindowSetGeometryAligned(win, align,
	                                   wPct*agView->w/100,
	                                   hPct*agView->h/100);
}

void
AG_WindowSaveGeometry(AG_Window *win)
{
	win->rSaved.x = WIDGET(win)->x;
	win->rSaved.y = WIDGET(win)->y;
	win->rSaved.w = WIDTH(win);
	win->rSaved.h = HEIGHT(win);
}

int
AG_WindowRestoreGeometry(AG_Window *win)
{
	return AG_WindowSetGeometryRect(win, win->rSaved, 0);
}

void
AG_WindowMaximize(AG_Window *win)
{
	AG_WindowSaveGeometry(win);
	if (AG_WindowSetGeometry(win, 0, 0, agView->w, agView->h) == 0)
		win->flags |= AG_WINDOW_MAXIMIZED;
}

void
AG_WindowUnmaximize(AG_Window *win)
{
	if (AG_WindowRestoreGeometry(win) == 0) {
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
		if (!agView->opengl &&
		    !(win->flags & AG_WINDOW_NOUPDATERECT)) {
			SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
			SDL_UpdateRect(agView->v, 0, 0, agView->v->w,
			    agView->v->h);
		}
	}
}

static void
IconMotion(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	int xRel = AG_INT(3);
	int yRel = AG_INT(4);
	AG_Window *wDND = icon->wDND;

	if (icon->flags & AG_ICON_DND) {
		if (!agView->opengl) {
			SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
			AG_QueueVideoUpdate(
			    WIDGET(wDND)->x,
			    WIDGET(wDND)->y,
			    WIDTH(wDND), HEIGHT(wDND));
		}
		AG_WindowSetGeometryRect(wDND,
		    AG_RECT(WIDGET(wDND)->x + xRel,
		            WIDGET(wDND)->y + yRel,
			    WIDTH(wDND), HEIGHT(wDND)), 1);
		    
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
		AG_ViewDetach(icon->wDND);
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

void
AG_WindowMinimize(AG_Window *win)
{
	AG_Window *wDND;
	AG_Icon *icon = win->icon;

	if (win->flags & AG_WINDOW_MINIMIZED) {
		return;
	}
	win->flags |= AG_WINDOW_MINIMIZED;
	AG_WindowHide(win);

	wDND = AG_WindowNew(AG_WINDOW_PLAIN|AG_WINDOW_KEEPBELOW|
	                    AG_WINDOW_DENYFOCUS|AG_WINDOW_NOBACKGROUND);
	AG_ObjectAttach(wDND, icon);
	icon->wDND = wDND;
	icon->flags &= ~(AG_ICON_DND|AG_ICON_DBLCLICKED);

	AG_SetEvent(icon, "dblclick-expire", DoubleClickTimeout, NULL);
	AG_SetEvent(icon, "window-mousemotion", IconMotion, NULL);
	AG_SetEvent(icon, "window-mousebuttonup", IconButtonUp, NULL);
	AG_SetEvent(icon, "window-mousebuttondown", IconButtonDown, "%p", win);

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

void
AG_WindowUnminimize(AG_Window *win)
{
	if (!win->visible) {
		AG_WindowShow(win);
		win->flags &= ~(AG_WINDOW_MINIMIZED);
	} else {
		AG_WindowFocus(win);
	}
}

/*
 * Resize a window with the mouse.
 * The window must be locked.
 */
static void
Resize(int op, AG_Window *win, SDL_MouseMotionEvent *motion)
{
	int x = WIDGET(win)->x;
	int y = WIDGET(win)->y;
	int w = WIDTH(win);
	int h = HEIGHT(win);

	switch (op) {
	case AG_WINOP_LRESIZE:
		if (!(win->flags & AG_WINDOW_NOHRESIZE)) {
			if (motion->xrel < 0) {
				w -= motion->xrel;
				x += motion->xrel;
			} else if (motion->xrel > 0) {
				w -= motion->xrel;
				x += motion->xrel;
			}
		}
		if (!(win->flags & AG_WINDOW_NOVRESIZE)) {
			if (motion->yrel < 0 || motion->yrel > 0)
				h += motion->yrel;
		}
		break;
	case AG_WINOP_RRESIZE:
		if (!(win->flags & AG_WINDOW_NOHRESIZE)) {
			if (motion->xrel < 0 || motion->xrel > 0)
				w += motion->xrel;
		}
		if (!(win->flags & AG_WINDOW_NOVRESIZE)) {
			if (motion->yrel < 0 || motion->yrel > 0)
				h += motion->yrel;
		}
		break;
	case AG_WINOP_HRESIZE:
		if (!(win->flags & AG_WINDOW_NOHRESIZE)) {
			if (motion->yrel < 0 || motion->yrel > 0)
				h += motion->yrel;
		}
		break;
	default:
		break;
	}
	AG_WindowSetGeometry(win, x, y, w, h);
	AG_PostEvent(NULL, win, "window-user-resize", "%d,%d", w, h);
}

void
AG_WindowDetachGenEv(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_ViewDetach(win);
}

void
AG_WindowShowGenEv(AG_Event *event)
{
	AG_WindowShow(AG_PTR(1));
}

void
AG_WindowHideGenEv(AG_Event *event)
{
	AG_WindowHide(AG_PTR(1));
}

void
AG_WindowCloseGenEv(AG_Event *event)
{
	AG_PostEvent(NULL, AG_PTR(1), "window-close", NULL);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Window *win = obj;
	AG_Widget *chld;
	AG_SizeReq rChld, rTbar;
	int nWidgets;
	
	r->w = win->lPad+win->rPad + win->wBorderSide*2;
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
		r->w = MAX(r->w, rChld.w + (win->lPad+win->rPad) +
		                 win->wBorderSide*2);
		r->h += rChld.h + win->spacing;
		nWidgets++;
	}
	if (nWidgets > 0 && r->h >= win->spacing)
		r->h -= win->spacing;

	win->wReq = r->w;
	win->hReq = r->h;
	ClampToView(win);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Window *win = obj;
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
	ClampToView(win);
	return (0);
}

/* Change the spacing between child widgets. */
void
AG_WindowSetSpacing(AG_Window *win, int spacing)
{
	AG_ObjectLock(win);
	win->spacing = spacing;
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

/* Set the text to show inside a window's titlebar. */
void
AG_WindowSetCaption(AG_Window *win, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

	AG_ObjectLock(win);
	if (win->tbar != NULL) {
		va_start(ap, fmt);
		Vsnprintf(s, sizeof(s), fmt, ap);
		va_end(ap);
		Strlcpy(win->caption, s, sizeof(win->caption));
		AG_WindowUpdateCaption(win);
	}
	AG_ObjectUnlock(win);
}

void
AG_WindowUpdateCaption(AG_Window *win)
{
	char iconCap[16], *c;

	AG_ObjectLock(win);
	if (win->tbar != NULL) {
		AG_TitlebarSetCaption(win->tbar, win->caption);
		if (Strlcpy(iconCap, win->caption, sizeof(iconCap)) >=
		    sizeof(iconCap)) {
			for (c = &iconCap[0]; *c != '\0'; c++) {
				if (*c == ' ')
					*c = '\n';
			}
			AG_IconSetText(win->icon, "%s...", iconCap);
		} else {
			AG_IconSetText(win->icon, "%s", iconCap);
		}
	}
	AG_ObjectUnlock(win);
}

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

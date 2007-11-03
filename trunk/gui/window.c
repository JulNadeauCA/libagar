/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "primitive.h"
#include "cursors.h"

#include <string.h>
#include <stdarg.h>

static void Resize(int, AG_Window *, SDL_MouseMotionEvent *);
static void Move(AG_Window *, SDL_MouseMotionEvent *);
static void Shown(AG_Event *);
static void Hidden(AG_Event *);
static void GainFocus(AG_Event *);
static void LostFocus(AG_Event *);

AG_Mutex agWindowLock = AG_MUTEX_INITIALIZER;
int	 agWindowXOutLimit = 32;
int	 agWindowBotOutLimit = 32;

/* Create a generic window. */
AG_Window *
AG_WindowNew(Uint flags)
{
	AG_Window *win;

	win = Malloc(sizeof(AG_Window), M_OBJECT);

	AG_MutexLock(&agView->lock);			/* XXX needed?? */
	AG_WindowInit(win, NULL, flags);
	AG_SetEvent(win, "window-close", AGWINDETACH(win));
	AG_ViewAttach(win);
	AG_MutexUnlock(&agView->lock);
	return (win);
}

/* Create a named window */
AG_Window *
AG_WindowNewNamed(Uint flags, const char *fmt, ...)
{
	char name[AG_OBJECT_NAME_MAX], *c;
	AG_Window *win;
	va_list ap;

	AG_MutexLock(&agView->lock);
	va_start(ap, fmt);
	vsnprintf(name, sizeof(name), fmt, ap);
	va_end(ap);
	for (c = &name[0]; *c != '\0'; c++) {
		if (*c == '/')
			*c = '_';
	}
	if (AG_WindowFocusNamed(name)) {
		win = NULL;
		goto out;
	}
	win = Malloc(sizeof(AG_Window), M_OBJECT);
	AG_WindowInit(win, name, flags);
	AG_SetEvent(win, "window-close", AGWINHIDE(win));
	AG_ViewAttach(win);
out:
	AG_MutexUnlock(&agView->lock);
	return (win);
}

void
AG_WindowInit(void *p, const char *name, int flags)
{
	AG_Window *win = p;
	int titlebar_flags = 0;
	AG_Event *ev;

	AG_WidgetInit(win, &agWindowOps, 0);
	AG_ObjectSetName(win, (name != NULL) ? name : "win-generic");
	OBJECT(win)->flags &= ~(AG_OBJECT_NAME_ONATTACH);
	win->flags = flags;
	win->visible = 0;
	win->alignment = AG_WINDOW_CENTER;
	win->spacing = 3;
	win->lPad = 2;
	win->rPad = 2;
	win->tPad = 2;
	win->bPad = 2;
	win->minw = win->lPad + win->rPad;
	win->minh = win->tPad + win->bPad +
	            ((win->flags&AG_WINDOW_NOTITLE) ? 0 : agTextFontHeight);
	win->savx = -1;
	win->savy = -1;
	win->savw = -1;
	win->savh = -1;
	win->caption[0] = '\0';
	TAILQ_INIT(&win->subwins);
	AG_MutexInitRecursive(&win->lock);

	if (!agView->opengl)
		WIDGET(win)->flags |= AG_WIDGET_CLIPPING;

	if (win->flags & AG_WINDOW_MODAL)
		win->flags |= AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOMINIMIZE;
	if (win->flags & AG_WINDOW_NORESIZE)
		win->flags |= AG_WINDOW_NOMAXIMIZE;

	if (win->flags & AG_WINDOW_NOCLOSE)
		titlebar_flags |= AG_TITLEBAR_NO_CLOSE;
	if (win->flags & AG_WINDOW_NOMINIMIZE)
		titlebar_flags |= AG_TITLEBAR_NO_MINIMIZE;
	if (win->flags & AG_WINDOW_NOMAXIMIZE)
		titlebar_flags |= AG_TITLEBAR_NO_MAXIMIZE;

	win->tbar = (flags & AG_WINDOW_NOTITLE) ? NULL :
	    AG_TitlebarNew(win, titlebar_flags);
	
	AG_SetEvent(win, "window-gainfocus", GainFocus, NULL);
	AG_SetEvent(win, "window-lostfocus", LostFocus, NULL);

	ev = AG_SetEvent(win, "widget-shown", Shown, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "widget-hidden", Hidden, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "detached", NULL, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
}

/* Attach a sub-window. */
void
AG_WindowAttach(AG_Window *win, AG_Window *subwin)
{
	if (win == NULL) {
		return;
	}
	AG_MutexLock(&agView->lock);
	TAILQ_INSERT_HEAD(&win->subwins, subwin, swins);
	AG_MutexUnlock(&agView->lock);
}

/* Detach a sub-window. */
void
AG_WindowDetach(AG_Window *win, AG_Window *subwin)
{
	if (win == NULL) {
		return;
	}
	AG_MutexLock(&agView->lock);
	TAILQ_REMOVE(&win->subwins, subwin, swins);
	AG_MutexUnlock(&agView->lock);
}

static void
Draw(void *p)
{
	AG_Window *win = p;

	if ((win->flags & AG_WINDOW_NOBACKGROUND) == 0)
		STYLE(win)->WindowBackground(win);
	if ((win->flags & AG_WINDOW_NOBORDERS) == 0)
		STYLE(win)->WindowBorders(win);
}

static void
Destroy(void *p)
{
	AG_Window *win = p;

	AG_MutexDestroy(&win->lock);
	/* AG_ViewDetachQueued() will free the sub-windows */
	AG_WidgetDestroy(win);
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
		AG_WindowApplyAlignment(win, win->alignment);
	} else {
		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDGET(win)->w;
		a.h = WIDGET(win)->h;
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
		AG_DrawRectFilled(win,
		    AG_RECT(0,0, WIDTH(win), HEIGHT(win)),
		    AG_COLOR(BG_COLOR));
		if (!agView->opengl) {
			AG_QueueVideoUpdate(
			    WIDGET(win)->x,
			    WIDGET(win)->y,
			    WIDGET(win)->w,
			    WIDGET(win)->h);
		}
//	}
	AG_PostEvent(NULL, win, "window-hidden", NULL);
}

static void
WidgetGainFocus(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		WidgetGainFocus(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(NULL, wid, "widget-gainfocus", NULL);
}

static void
WidgetLostFocus(AG_Widget *wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		WidgetLostFocus(chld);
	}
	if (wid->flags & AG_WIDGET_FOCUSED)
		AG_PostEvent(NULL, wid, "widget-lostfocus", NULL);
}

static void GainFocus(AG_Event *event) { WidgetGainFocus(WIDGET(AG_SELF())); }
static void LostFocus(AG_Event *event) { WidgetLostFocus(WIDGET(AG_SELF())); }

/*
 * Verify whether a given window resides inside the area of some other
 * window, to see whether is it necessary to update the background.
 */
int
AG_WindowIsSurrounded(AG_Window *win)
{
	AG_Window *owin;

	TAILQ_FOREACH(owin, &agView->windows, windows) {
		AG_MutexLock(&owin->lock);
		if (owin->visible &&
		    AG_WidgetArea(owin, WIDGET(win)->x, WIDGET(win)->y) &&
		    AG_WidgetArea(owin,
		     WIDGET(win)->x + WIDGET(win)->w,
		     WIDGET(win)->y + WIDGET(win)->h)) {
			AG_MutexUnlock(&owin->lock);
			return (1);
		}
		AG_MutexUnlock(&owin->lock);
	}
	return (0);
}

/* Toggle the visibility of a window. */
int
AG_WindowToggleVisibility(AG_Window *win)
{
	int oldvis;

	AG_MutexLock(&win->lock);
	oldvis = win->visible;
	if (win->visible) {
		AG_WindowHide(win);
	} else {
		AG_WindowShow(win);
	}
	AG_MutexUnlock(&win->lock);
	return (oldvis);
}

/* Set the visibility bit of a window. */
void
AG_WindowShow(AG_Window *win)
{
	AG_MutexLock(&win->lock);
	if (!win->visible) {
		AG_PostEvent(NULL, win, "widget-shown", NULL);
		win->visible++;
	}
	AG_MutexUnlock(&win->lock);
}

/* Clear the visibility bit of a window. */
void
AG_WindowHide(AG_Window *win)
{
	AG_MutexLock(&win->lock);
	if (win->visible) {
		win->visible = 0;
		AG_PostEvent(NULL, win, "widget-hidden", NULL);
	}
	AG_MutexUnlock(&win->lock);
}

static void
AG_WindowCountWidgets(AG_Widget *wid, Uint *nwidgets)
{
	AG_Widget *cwid;

	if (wid->flags & AG_WIDGET_FOCUSABLE)
		(*nwidgets)++;

	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WindowCountWidgets(cwid, nwidgets);
}

static void
AG_WindowMapWidgets(AG_Widget *wid, AG_Widget **widgets, Uint *i)
{
	AG_Widget *cwid;

	if (wid->flags & AG_WIDGET_FOCUSABLE)
		widgets[(*i)++] = wid;

	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
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
	widgets = Malloc(nwidgets * sizeof(AG_Widget *), M_WIDGET);
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
	Free(widgets, M_WIDGET);
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
	SDL_Rect oldpos, newpos, rfill1, rfill2;

	oldpos.x = WIDGET(win)->x;
	oldpos.y = WIDGET(win)->y;
	oldpos.w = WIDGET(win)->w;
	oldpos.h = WIDGET(win)->h;

	WIDGET(win)->x += motion->xrel;
	WIDGET(win)->y += motion->yrel;
	ClampToView(win);

	AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);

	/* Update the background. */
	/* XXX TODO Avoid drawing over KEEPABOVE windows */
	newpos.x = WIDGET(win)->x;
	newpos.y = WIDGET(win)->y;
	newpos.w = WIDGET(win)->w;
	newpos.h = WIDGET(win)->h;
	rfill1.w = 0;
	rfill2.w = 0;
	if (newpos.x > oldpos.x) {		/* Right */
		rfill1.x = oldpos.x;
		rfill1.y = oldpos.y;
		rfill1.w = newpos.x - oldpos.x;
		rfill1.h = newpos.h;
	} else if (newpos.x < oldpos.x) {	/* Left */
		rfill1.x = newpos.x + newpos.w;
		rfill1.y = newpos.y;
		rfill1.w = oldpos.x - newpos.x;
		rfill1.h = oldpos.h;
	}
	if (newpos.y > oldpos.y) {		/* Downward */
		rfill2.x = oldpos.x;
		rfill2.y = oldpos.y;
		rfill2.w = newpos.w;
		rfill2.h = newpos.y - oldpos.y;
	} else if (newpos.y < oldpos.y) {	/* Upward */
		rfill2.x = oldpos.x;
		rfill2.y = newpos.y + newpos.h;
		rfill2.w = oldpos.w;
		rfill2.h = oldpos.y - newpos.y;
	}
	if (!agView->opengl &&
	    !(win->flags & AG_WINDOW_NOUPDATERECT)) {
		if (rfill1.w > 0) {
			SDL_FillRect(agView->v, &rfill1, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rfill1);
		}
		if (rfill2.w > 0) {
			SDL_FillRect(agView->v, &rfill2, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rfill2);
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
	AG_MutexLock(&agView->lock);
	if (win == NULL) {
		agView->winToFocus = NULL;
		goto out;
	}
	AG_MutexLock(&win->lock);
	if ((win->flags & AG_WINDOW_DENYFOCUS) == 0) {
		win->flags |= AG_WINDOW_FOCUSONATTACH;
		agView->winToFocus = win;
	}
	AG_MutexUnlock(&win->lock);
out:	AG_MutexUnlock(&agView->lock);
}

/* Give focus to a window by name, and show it is if is hidden. */
int
AG_WindowFocusNamed(const char *name)
{
	AG_Window *owin;
	int rv = 0;

	AG_MutexLock(&agView->lock);
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
	AG_MutexUnlock(&agView->lock);
	return (0);
}

/*
 * Check if the given coordinates overlap a window control, and if so
 * which operation is related to it.
 */
static __inline__ int
AG_WindowMouseOverCtrl(AG_Window *win, int x, int y)
{
	if ((y - WIDGET(win)->y) > (WIDGET(win)->h - agColorsBorderSize)) {
	    	if ((x - WIDGET(win)->x) < 17) {
			return (AG_WINOP_LRESIZE);
		} else if ((x - WIDGET(win)->x) > (WIDGET(win)->w - 17)) {
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
			AG_MutexLock(&win->lock);
			if (!win->visible ||
			    !AG_WidgetArea(win, ev->button.x, ev->button.y)) {
				AG_MutexUnlock(&win->lock);
				continue;
			}
			if (win->flags & AG_WINDOW_DENYFOCUS) {
				agView->winToFocus = focus_saved;
			} else {
				agView->winToFocus = win;
				focus_changed++;
			}
			AG_MutexUnlock(&win->lock);
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
					goto out;
				}
			} else {
				continue;
			}
		}
		AG_MutexLock(&win->lock);
		if (!win->visible) {
			AG_MutexUnlock(&win->lock);
			continue;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			/* Process active MOVE or RESIZE operations. */
			if (agView->winop != AG_WINOP_NONE &&
			    agView->winSelected != win) {
				AG_MutexUnlock(&win->lock);
				continue;
			}
			switch (agView->winop) {
			case AG_WINOP_NONE:
				break;
			case AG_WINOP_MOVE:
				Move(win, &ev->motion);
				AG_MutexUnlock(&win->lock);
				rv = 1;
				goto out;
			case AG_WINOP_LRESIZE:
				Resize(AG_WINOP_LRESIZE, win, &ev->motion);
				AG_MutexUnlock(&win->lock);
				goto out;
			case AG_WINOP_RRESIZE:
				Resize(AG_WINOP_RRESIZE, win, &ev->motion);
				AG_MutexUnlock(&win->lock);
				goto out;
			case AG_WINOP_HRESIZE:
				Resize(AG_WINOP_HRESIZE, win, &ev->motion);
				AG_MutexUnlock(&win->lock);
				goto out;
			default:
				break;
			}
			/*
			 * Forward this MOUSEMOTION to all widgets that either
			 * hold focus or have the AG_WIDGET_UNFOCUSED_MOTION
			 * flag set.
			 */
			OBJECT_FOREACH_CHILD(wid, win, ag_widget) {
				if (wid->flags & AG_WIDGET_PRIO_MOTION) {
					AG_WidgetMouseMotion(win, wid,
					    ev->motion.x, ev->motion.y,
					    ev->motion.xrel, ev->motion.yrel,
					    ev->motion.state);
					AG_MutexUnlock(&win->lock);
					goto out;
				}
			}
			OBJECT_FOREACH_CHILD(wid, win, ag_widget) {
				AG_WidgetMouseMotion(win, wid,
				    ev->motion.x, ev->motion.y,
				    ev->motion.xrel, ev->motion.yrel,
				    ev->motion.state);
			}
			/* Change the cursor if a RESIZE op is in progress. */
			if (agCursorToSet == NULL &&
			    (win->flags & AG_WINDOW_NORESIZE) == 0 &&
			    AG_WidgetArea(win, ev->motion.x, ev->motion.y)) {
				if ((win->flags & AG_WINDOW_NOBORDERS) == 0) {
					switch (AG_WindowMouseOverCtrl(win,
					    ev->motion.x, ev->motion.y)) {
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
			OBJECT_FOREACH_CHILD(wid, win, ag_widget) {
				AG_WidgetMouseButtonUp(win, wid,
				    ev->button.button,
				    ev->button.x, ev->button.y);
			}
			if (focus_changed) {
				AG_MutexUnlock(&win->lock);
				rv = 1;
				goto out;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!AG_WidgetArea(win, ev->button.x, ev->button.y)) {
				AG_MutexUnlock(&win->lock);
				continue;
			}
			if ((win->flags & AG_WINDOW_NOBORDERS) == 0 &&
			    (agView->winop = AG_WindowMouseOverCtrl(win,
			    ev->button.x, ev->button.y)) != AG_WINOP_NONE) {
				agView->winSelected = win;
			}
			/* Forward to overlapping widgets. */
			OBJECT_FOREACH_CHILD(wid, win, ag_widget) {
				if (AG_WidgetMouseButtonDown(win, wid,
				    ev->button.button, ev->button.x,
				    ev->button.y)) {
					AG_MutexUnlock(&win->lock);
					rv = 1;
					goto out;
				}
			}
			if (focus_changed) {
				AG_MutexUnlock(&win->lock);
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
			/* XXX */
			switch (ev->key.keysym.sym) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				/* Always ignore modifiers */
				AG_MutexUnlock(&win->lock);
				return (0);
			default:
				break;
			}
			tabCycle = 1;
			if (AG_WINDOW_FOCUSED(win) &&
			   (wFoc = AG_WidgetFindFocused(win)) != NULL &&
			   (ev->key.keysym.sym != SDLK_TAB ||
			    wFoc->flags & AG_WIDGET_CATCH_TAB)) {
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
			if (tabCycle &&
			    ev->key.keysym.sym == SDLK_TAB &&
			    ev->type == SDL_KEYUP) {
				AG_WindowCycleFocus(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				AG_MutexUnlock(&win->lock);
				rv = 1;
			}
			goto out;
		}
		AG_MutexUnlock(&win->lock);
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

void
AG_WindowUpdate(AG_Window *win)
{
	AG_SizeAlloc a;
	
	if (win == NULL) {
		return;
	}
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
 * Set window coordinates and geometry. This should be used instead of
 * a direct WidgetSizeAlloc() since it redraws portions of the window
 * background when needed.
 */
int
AG_WindowSetGeometryParam(AG_Window *win, int x, int y, int w, int h,
    int bounded)
{
	AG_SizeReq rWin;
	AG_SizeAlloc aWin;
	SDL_Rect rFill;
	int new;
	int ox, oy, ow, oh;
	int nw, nh;

	AG_MutexLock(&win->lock);
	ox = WIDGET(win)->x;
	oy = WIDGET(win)->y;
	ow = WIDGET(win)->w;
	oh = WIDGET(win)->h;
	new = ((WIDGET(win)->x == -1 || WIDGET(win)->y == -1));

	if (w == -1 || h == -1) {
		AG_WidgetSizeReq(win, &rWin);
		nw = (w == -1) ? rWin.w : w;
		nh = (h == -1) ? rWin.h : h;
	} else {
		nw = w;
		nh = h;
	}
	if (nw < win->minw) { nw = win->minw; }
	if (nh < win->minh) { nh = win->minh; }

	/* Limit the window to the view boundaries. */
	aWin.x = (x == -1) ? WIDGET(win)->x : x;
	aWin.y = (y == -1) ? WIDGET(win)->y : y;
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
			rFill.h = WIDGET(win)->h;
		} else if (WIDGET(win)->w < ow) {		/* R-resize */
			rFill.x = WIDGET(win)->x + WIDGET(win)->w;
			rFill.y = WIDGET(win)->y;
			rFill.w = ow - WIDGET(win)->w;
			rFill.h = oh;
		} else {
			rFill.w = 0;
			rFill.h = 0;
		}
		if (rFill.w > 0 && rFill.h > 0) {
			SDL_FillRect(agView->v, &rFill, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rFill);
		}
		if (WIDGET(win)->h < oh) {			/* H-resize */
			rFill.x = ox;
			rFill.y = WIDGET(win)->y + WIDGET(win)->h;
			rFill.w = ow;
			rFill.h = oh - WIDGET(win)->h;
			SDL_FillRect(agView->v, &rFill, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rFill);
		}
	}
	AG_MutexUnlock(&win->lock);
	return (0);
fail:
	AG_MutexUnlock(&win->lock);
	return (-1);
}

void
AG_WindowSaveGeometry(AG_Window *win)
{
	win->savx = WIDGET(win)->x;
	win->savy = WIDGET(win)->y;
	win->savw = WIDGET(win)->w;
	win->savh = WIDGET(win)->h;
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
	if (AG_WindowSetGeometry(win, win->savx, win->savy, win->savw,
	    win->savh) == 0) {
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
		if (!agView->opengl &&
		    !(win->flags & AG_WINDOW_NOUPDATERECT)) {
			SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
			SDL_UpdateRect(agView->v, 0, 0, agView->v->w,
			    agView->v->h);
		}
	}
}

void
AG_WindowMinimize(AG_Window *win)
{
	win->flags |= AG_WINDOW_MINIMIZED;
	AG_WindowHide(win);
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
	int w = WIDGET(win)->w;
	int h = WIDGET(win)->h;

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
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Window *win = p;
	AG_Widget *chld;
	AG_SizeReq rChld, rTbar;
	int nWidgets, wPad;
	
	AG_MutexLock(&win->lock);

	r->w = win->lPad + win->rPad;
	r->h = win->bPad + win->tPad;

	if ((win->flags & AG_WINDOW_NOBORDERS) == 0) {
		r->w += agColorsBorderSize*2;
		r->h += agColorsBorderSize;
		if (win->tbar == NULL) {
			r->h += agColorsBorderSize;
		}
		wPad = win->lPad + win->rPad + agColorsBorderSize*2;
	} else {
		wPad = win->lPad + win->rPad;
	}
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
		r->w = MAX(r->w, rChld.w + wPad);
		r->h += rChld.h + win->spacing;
		nWidgets++;
	}
	if (nWidgets > 0 && r->h >= win->spacing)
		r->h -= win->spacing;

	if (!agWindowAnySize) {
		win->minw = r->w;
		win->minh = r->h;
	} else {
		win->minw = win->lPad + win->rPad + agColorsBorderSize*2;
		win->minh = win->tPad + win->bPad + agColorsBorderSize;
		if (win->tbar != NULL) {
			win->minw = MAX(win->minw,rTbar.w);
			win->minh += rTbar.h;
		} else {
			win->minh += agColorsBorderSize;
		}
	}
	ClampToView(win);
	AG_MutexUnlock(&win->lock);
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Window *win = p;
	AG_Widget *chld;
	AG_SizeReq rChld;
	AG_SizeAlloc aChld;
	int wAvail, hAvail;
	int totFixed;
	int nWidgets;

	AG_MutexLock(&win->lock);

	/* Calculate total space available for widgets. */
	wAvail = a->w - win->lPad - win->rPad;
	hAvail = a->h - win->tPad - win->bPad;
	if ((win->flags & AG_WINDOW_NOBORDERS) == 0) {
		wAvail -= agColorsBorderSize*2;
		hAvail -= agColorsBorderSize;
		if (win->tbar == NULL)
			hAvail -= agColorsBorderSize;
	}

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
		aChld.x = win->lPad;
		aChld.y = rChld.h + win->tPad;
		if ((win->flags & AG_WINDOW_NOBORDERS) == 0) {
			aChld.x += agColorsBorderSize;
		}
	} else {
		aChld.x = win->lPad;
		aChld.y = win->tPad;
		if ((win->flags & AG_WINDOW_NOBORDERS) == 0) {
			aChld.x += agColorsBorderSize;
			aChld.y += agColorsBorderSize;
		}
	}
	OBJECT_FOREACH_CHILD(chld, win, ag_widget) {
		AG_WidgetSizeReq(chld, &rChld);
		if (chld == WIDGET(win->tbar)) {
			continue;
		}
		if (chld->flags & AG_WIDGET_IGNORE_PADDING) {
			AG_SizeAlloc aTmp;

			aTmp.x = 0;
			aTmp.y = aChld.y;
			aTmp.w = a->w;
			aTmp.h = rChld.h;
			AG_WidgetSizeAlloc(chld, &aTmp);
			aChld.y += aTmp.h + win->spacing;
			continue;
		} else {
			aChld.w = (chld->flags & AG_WIDGET_HFILL) ?
			          wAvail : rChld.w;
		}
		aChld.h = (chld->flags & AG_WIDGET_VFILL) ?
		          hAvail-totFixed : rChld.h;
		AG_WidgetSizeAlloc(chld, &aChld);
		aChld.y += aChld.h + win->spacing;
	}
	ClampToView(win);
	AG_MutexUnlock(&win->lock);
	return (0);
}

/* Change the spacing between child widgets. */
void
AG_WindowSetSpacing(AG_Window *win, int spacing)
{
	AG_MutexLock(&win->lock);
	win->spacing = spacing;
	AG_MutexUnlock(&win->lock);
}

/* Change the padding around child widgets. */
void
AG_WindowSetPadding(AG_Window *win, int lPad, int rPad, int tPad, int bPad)
{
	AG_MutexLock(&win->lock);
	if (lPad != -1) { win->lPad = lPad; }
	if (rPad != -1) { win->rPad = rPad; }
	if (tPad != -1) { win->tPad = tPad; }
	if (bPad != -1) { win->bPad = bPad; }
	AG_MutexUnlock(&win->lock);
}

/* Request a specific initial window position. */
void
AG_WindowSetPosition(AG_Window *win, enum ag_window_alignment alignment,
    int cascade)
{
	AG_MutexLock(&win->lock);
	win->alignment = alignment;
	/* Ignore cascade for now */
	AG_MutexUnlock(&win->lock);
}

/* Set a default window-close handler. */
void
AG_WindowSetCloseAction(AG_Window *win, enum ag_window_close_action mode)
{
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
}

/*
 * Set the position of a window assuming its size is known.
 * The window must be locked.
 */
void
AG_WindowApplyAlignment(AG_Window *win, enum ag_window_alignment alignment)
{
	switch (alignment) {
	case AG_WINDOW_UPPER_LEFT:
		WIDGET(win)->x = 0;
		WIDGET(win)->y = 0;
		break;
	case AG_WINDOW_MIDDLE_LEFT:
		WIDGET(win)->x = 0;
		WIDGET(win)->y = agView->h/2 - WIDGET(win)->h/2;
		break;
	case AG_WINDOW_LOWER_LEFT:
		WIDGET(win)->x = 0;
		WIDGET(win)->y = agView->h - WIDGET(win)->h;
		break;
	case AG_WINDOW_UPPER_RIGHT:
		WIDGET(win)->x = agView->w - WIDGET(win)->w;
		WIDGET(win)->y = 0;
		break;
	case AG_WINDOW_MIDDLE_RIGHT:
		WIDGET(win)->x = agView->w - WIDGET(win)->w;
		WIDGET(win)->y = agView->h/2 - WIDGET(win)->h/2;
		break;
	case AG_WINDOW_LOWER_RIGHT:
		WIDGET(win)->x = agView->w - WIDGET(win)->w;
		WIDGET(win)->y = agView->h - WIDGET(win)->h;
		break;
	case AG_WINDOW_CENTER:
		WIDGET(win)->x = agView->w/2 - WIDGET(win)->w/2;
		WIDGET(win)->y = agView->h/2 - WIDGET(win)->h/2;
		break;
	case AG_WINDOW_LOWER_CENTER:
		WIDGET(win)->x = agView->w/2 - WIDGET(win)->w/2;
		WIDGET(win)->y = agView->h - WIDGET(win)->h;
		break;
	case AG_WINDOW_UPPER_CENTER:
		WIDGET(win)->x = agView->w/2 - WIDGET(win)->w/2;
		WIDGET(win)->y = 0;
		break;
	}
	ClampToView(win);
}

/* Set the text to show inside a window's titlebar. */
void
AG_WindowSetCaption(AG_Window *win, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

#ifndef DEBUG
	if (win->tbar == NULL)
		return;
#endif
	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);
	
	strlcpy(win->caption, s, sizeof(win->caption));
	AG_WindowUpdateCaption(win);
}

void
AG_WindowUpdateCaption(AG_Window *win)
{
	if (win->tbar != NULL)
		AG_TitlebarSetCaption(win->tbar, win->caption);
}

const AG_WidgetOps agWindowOps = {
	{
		"AG_Widget:AG_Window",
		sizeof(AG_Window),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

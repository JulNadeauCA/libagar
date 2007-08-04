/*
 * Copyright (c) 2001-2006 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include <core/view.h>

#include "window.h"
#include "titlebar.h"

#include "primitive.h"
#include "cursors.h"

#include <string.h>
#include <stdarg.h>

const AG_WidgetOps agWindowOps = {
	{
		"AG_Widget:AG_Window",
		sizeof(AG_Window),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		AG_WindowDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_WindowDraw,
	AG_WindowScale
};

const AG_WidgetStyleMod agWindowDefaultStyle = {
	"default",
	{
		NULL,			/* bggradient */
		NULL,			/* bgtexture */
		AG_NOTCH_RESIZE_STYLE
	},
	NULL				/* misc fn */
};

static void AG_WindowResizeOp(int, AG_Window *, SDL_MouseMotionEvent *);
static void AG_WindowMoveOp(AG_Window *, SDL_MouseMotionEvent *);
static void AG_WindowShownEv(AG_Event *);
static void AG_WindowHiddenEv(AG_Event *);

AG_Mutex agWindowLock = AG_MUTEX_INITIALIZER;
int	 agWindowXOffs = 0;
int	 agWindowYOffs = 0;
int	 agWindowAnySize = 0;

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
	char wname[AG_OBJECT_NAME_MAX];
	AG_Window *win = p;
	int titlebar_flags = 0;
	AG_Event *ev;
	int i;

	strlcpy(wname, "win-", sizeof(wname));
	strlcat(wname, (name != NULL) ? name : "generic", sizeof(wname));
	AG_WidgetInit(win, &agWindowOps, 0);
	AG_ObjectSetName(win, wname);
	win->flags = flags;
	win->visible = 0;
	win->alignment = AG_WINDOW_CENTER;
	win->spacing = 2;
	win->xpadding = agColorsBorderSize;
	win->ypadding_top = 1;
	win->ypadding_bot = agColorsBorderSize + 1;
	win->minw = win->xpadding*2 + 16;
	win->minh = win->ypadding_top + win->ypadding_bot +
	            agTextFontHeight + 16;
	win->savx = -1;
	win->savy = -1;
	win->savw = -1;
	win->savh = -1;
	win->caption[0] = '\0';
	TAILQ_INIT(&win->subwins);
	AG_MutexInitRecursive(&win->lock);
	AG_WindowSetStyle(win, &agWindowDefaultStyle);
	
	if (win->flags & AG_WINDOW_MODAL) {
		win->flags |= AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOMINIMIZE;
	}
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

	/* Automatically notify children of visibility changes. */
	ev = AG_SetEvent(win, "widget-shown", AG_WindowShownEv, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "widget-hidden", AG_WindowHiddenEv, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(win, "widget-lostfocus", NULL, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;

	/* Notify children prior to destruction. */
	ev = AG_SetEvent(win, "detached", NULL, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
}

/* Apply style settings to a window, which will be inherited by children. */
void
AG_WindowSetStyle(AG_Window *win, const AG_WidgetStyleMod *style)
{
	AGWIDGET(win)->style = style;
}

/* Attach a sub-window. */
void
AG_WindowAttach(AG_Window *win, AG_Window *subwin)
{
	AG_MutexLock(&agView->lock);
	TAILQ_INSERT_HEAD(&win->subwins, subwin, swins);
	AG_MutexUnlock(&agView->lock);
}

/* Detach a sub-window. */
void
AG_WindowDetach(AG_Window *win, AG_Window *subwin)
{
	AG_MutexLock(&agView->lock);
	TAILQ_REMOVE(&win->subwins, subwin, swins);
	AG_MutexUnlock(&agView->lock);
}

void
AG_WindowDraw(void *p)
{
	AG_Window *win = p;
	int i;

	if ((win->flags & AG_WINDOW_NOBACKGROUND) == 0) {
		agPrim.rect_filled(win, 0, 0,
		    AGWIDGET(win)->w, AGWIDGET(win)->h,
		    AG_COLOR(WINDOW_BG_COLOR));
	}
	if (win->flags & AG_WINDOW_NOBORDERS)
		return;

	/* Draw the window frame (expected to fit inside padding). */
	for (i = 1; i < agColorsBorderSize-1; i++) {
		agPrim.hline(win,
		    i, AGWIDGET(win)->w - i,
		    i,
		    agColorsBorder[i-1]);
		agPrim.hline(win,
		    i,  AGWIDGET(win)->w - i,
		    AGWIDGET(win)->h - i,
		    agColorsBorder[i-1]);
	}
	for (i = 1; i < agColorsBorderSize-1; i++) {
		agPrim.vline(win,
		    i,
		    i, AGWIDGET(win)->h - i,
		    agColorsBorder[i-1]);
		agPrim.vline(win,
		    AGWIDGET(win)->w - i,
		    i,
		    AGWIDGET(win)->h - i,
		    agColorsBorder[i-1]);
	}

	/* Draw the resize controls. */
	if ((win->flags & AG_WINDOW_NOHRESIZE) == 0) {
		agPrim.vline(win,
		    18,
		    AGWIDGET(win)->h - agColorsBorderSize,
		    AGWIDGET(win)->h - 2,
		    AG_COLOR(WINDOW_LO_COLOR));
		agPrim.vline(win,
		    19,
		    AGWIDGET(win)->h - agColorsBorderSize,
		    AGWIDGET(win)->h - 2,
		    AG_COLOR(WINDOW_HI_COLOR));
		
		agPrim.vline(win,
		    AGWIDGET(win)->w - 19,
		    AGWIDGET(win)->h - agColorsBorderSize,
		    AGWIDGET(win)->h - 2,
		    AG_COLOR(WINDOW_LO_COLOR));
		agPrim.vline(win,
		    AGWIDGET(win)->w - 18,
		    AGWIDGET(win)->h - agColorsBorderSize,
		    AGWIDGET(win)->h - 2,
		    AG_COLOR(WINDOW_HI_COLOR));
	}
	
	if ((win->flags & AG_WINDOW_NOVRESIZE) == 0) {
		agPrim.hline(win,
		    2,
		    agColorsBorderSize,
		    AGWIDGET(win)->h - 20,
		    AG_COLOR(WINDOW_LO_COLOR));
		agPrim.hline(win,
		    2,
		    agColorsBorderSize,
		    AGWIDGET(win)->h - 19,
		    AG_COLOR(WINDOW_HI_COLOR));
		
		agPrim.hline(win,
		    AGWIDGET(win)->w - agColorsBorderSize,
		    AGWIDGET(win)->w - 2,
		    AGWIDGET(win)->h - 20,
		    AG_COLOR(WINDOW_LO_COLOR));
		agPrim.hline(win,
		    AGWIDGET(win)->w - agColorsBorderSize,
		    AGWIDGET(win)->w - 2,
		    AGWIDGET(win)->h - 19,
		    AG_COLOR(WINDOW_HI_COLOR));
	}
}

void
AG_WindowDestroy(void *p)
{
	AG_Window *win = p;

	AG_MutexDestroy(&win->lock);
	/* AG_ViewDetachQueued() will free the sub-windows */
	AG_WidgetDestroy(win);
}

static void
AG_WindowShownEv(AG_Event *event)
{
	AG_Window *win = AG_SELF();

	if ((win->flags & AG_WINDOW_DENYFOCUS) == 0) {
		AG_WindowFocus(win);
	}
	if (win->flags & AG_WINDOW_MODAL) {
		dprintf("%s: modal #%d\n", win->caption, agView->nModal);
		agView->winModal = Realloc(agView->winModal,
		    (agView->nModal+1) * sizeof(AG_Window *));
		agView->winModal[agView->nModal] = win;
		agView->nModal++;
	}

	if (AGWIDGET(win)->x == -1 && AGWIDGET(win)->y == -1) {
		/* First pass: initial sizing. */
		AGWIDGET_OPS(win)->scale(win, AGWIDGET(win)->w,
		    AGWIDGET(win)->h);

		/* Second pass: [wh]fill and homogenous divisions. */
		AGWIDGET_OPS(win)->scale(win, AGWIDGET(win)->w,
		    AGWIDGET(win)->h);
	
		/* Position the window and cache the absolute widget coords. */
		AG_WindowApplyAlignment(win, win->alignment);
		AG_WidgetUpdateCoords(win, AGWIDGET(win)->x, AGWIDGET(win)->y);
	}
	AG_PostEvent(NULL, win, "window-shown", NULL);
}

static void
AG_WindowHiddenEv(AG_Event *event)
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
//	if (!AG_WindowIsSurrounded(win)) {
		agPrim.rect_filled(win, 0, 0,
		    AGWIDGET(win)->w,
		    AGWIDGET(win)->h,
		    AG_COLOR(BG_COLOR));
		if (!agView->opengl) {
			SDL_UpdateRect(agView->v,
			    AGWIDGET(win)->x, AGWIDGET(win)->y,
			    AGWIDGET(win)->w, AGWIDGET(win)->h);
		}
//	}
	AG_PostEvent(NULL, win, "window-hidden", NULL);
}

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
		    AG_WidgetArea(owin, AGWIDGET(win)->x, AGWIDGET(win)->y) &&
		    AG_WidgetArea(owin,
		     AGWIDGET(win)->x + AGWIDGET(win)->w,
		     AGWIDGET(win)->y + AGWIDGET(win)->h)) {
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

	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WindowCountWidgets(cwid, nwidgets);
}

static void
AG_WindowMapWidgets(AG_Widget *wid, AG_Widget **widgets, Uint *i)
{
	AG_Widget *cwid;

	if (wid->flags & AG_WIDGET_FOCUSABLE)
		widgets[(*i)++] = wid;

	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
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
	int i = 0;

	if ((olfocus = AG_WidgetFindFocused(win)) == NULL) {
		dprintf("no focus!\n");
		return;
	}

	AG_WindowCountWidgets(AGWIDGET(win), &nwidgets);
	widgets = Malloc(nwidgets * sizeof(AG_Widget *), M_WIDGET);
	AG_WindowMapWidgets(AGWIDGET(win), widgets, &i);

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
				} else if (i+1 == nwidgets) {
					AG_WidgetFocus(widgets[0]);
				}
			}
			break;
		}
	}
	Free(widgets, M_WIDGET);
}

/*
 * Move a window using the mouse.
 * The view and window must be locked.
 */
static void
AG_WindowMoveOp(AG_Window *win, SDL_MouseMotionEvent *motion)
{
	SDL_Rect oldpos, newpos, rfill1, rfill2;

	oldpos.x = AGWIDGET(win)->x;
	oldpos.y = AGWIDGET(win)->y;
	oldpos.w = AGWIDGET(win)->w;
	oldpos.h = AGWIDGET(win)->h;

	AGWIDGET(win)->x += motion->xrel;
	AGWIDGET(win)->y += motion->yrel;
	AG_WindowClamp(win);

	AG_WidgetUpdateCoords(win, AGWIDGET(win)->x, AGWIDGET(win)->y);

	/* Update the background. */
	newpos.x = AGWIDGET(win)->x;
	newpos.y = AGWIDGET(win)->y;
	newpos.w = AGWIDGET(win)->w;
	newpos.h = AGWIDGET(win)->h;
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
	if (!agView->opengl) {
		if (rfill1.w > 0) {
			SDL_FillRect(agView->v, &rfill1, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rfill1);
		}
		if (rfill2.w > 0) {
			SDL_FillRect(agView->v, &rfill2, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rfill2);
		}
	}
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
		if (strlen(AGOBJECT(owin)->name) >= 4 &&
		    strcmp(AGOBJECT(owin)->name+4, name) == 0) {
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
	if ((y - AGWIDGET(win)->y) > (AGWIDGET(win)->h - agColorsBorderSize)) {
	    	if ((x - AGWIDGET(win)->x) < 17) {
			return (AG_WINOP_LRESIZE);
		} else if ((x - AGWIDGET(win)->x) > (AGWIDGET(win)->w - 17)) {
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
	AG_Widget *wid;
	int focus_changed = 0;
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
				AG_WindowMoveOp(win, &ev->motion);
				AG_MutexUnlock(&win->lock);
				rv = 1;
				goto out;
			case AG_WINOP_LRESIZE:
				AG_WindowResizeOp(AG_WINOP_LRESIZE, win,
				    &ev->motion);
				AG_MutexUnlock(&win->lock);
				goto out;
			case AG_WINOP_RRESIZE:
				AG_WindowResizeOp(AG_WINOP_RRESIZE, win,
				    &ev->motion);
				AG_MutexUnlock(&win->lock);
				goto out;
			case AG_WINOP_HRESIZE:
				AG_WindowResizeOp(AG_WINOP_HRESIZE, win,
				    &ev->motion);
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
			AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
				if (wid->flags & AG_WIDGET_PRIO_MOTION) {
					AG_WidgetMouseMotion(win, wid,
					    ev->motion.x, ev->motion.y,
					    ev->motion.xrel, ev->motion.yrel,
					    ev->motion.state);
					AG_MutexUnlock(&win->lock);
					goto out;
				}
			}
			AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
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
			AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
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
			AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
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
			if (ev->key.keysym.sym == SDLK_TAB &&	/* Move focus */
			    ev->type == SDL_KEYUP) {
				AG_WindowCycleFocus(win,
				    (ev->key.keysym.mod & KMOD_SHIFT));
				AG_MutexUnlock(&win->lock);
				rv = 1;
				goto out;
			}
			if (AG_WINDOW_FOCUSED(win)) {
				AG_Widget *fwid;

				if ((fwid = AG_WidgetFindFocused(win))!= NULL) {
					AG_PostEvent(NULL, fwid,
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
				} else {
					dprintf("no focused widget in %s\n",
					    AGOBJECT(win)->name);
				}
			}
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
		    agView->winToFocus == lastwin) 	/* Already focused? */
			goto outf;

		if (lastwin != NULL) {
			AG_PostEvent(NULL, lastwin, "widget-lostfocus", NULL);
			if (lastwin->flags & AG_WINDOW_KEEPABOVE)
				goto outf;
		}
		if (agView->winToFocus != NULL &&
		    agView->winToFocus != TAILQ_LAST(&agView->windows,
		    ag_windowq)) {
			TAILQ_REMOVE(&agView->windows, agView->winToFocus,
			    windows);
			TAILQ_INSERT_TAIL(&agView->windows, agView->winToFocus,
			    windows);
		}
		agView->winToFocus = NULL;
	}
outf:
	return (rv);
}

/*
 * Clamp the window geometry/coordinates down to the view area.
 * The window must be locked.
 */
void
AG_WindowClamp(AG_Window *win)
{
	AG_Widget *w = AGWIDGET(win);
	
	if (w->x + w->w > agView->w)
		w->x = agView->w - w->w;
	if (w->y + w->h > agView->h)
		w->y = agView->h - w->h;

	if (w->x < 0)
		w->x = 0;
	if (w->y < 0)
		w->y = 0;

	if (w->x+w->w > agView->w) {
		w->x = 0;
		w->w = agView->w - 1;
	}
	if (w->y+w->h > agView->h) {
		w->y = 0;
		w->h = agView->h - 1;
	}
}

/* Set the window coordinates and geometry. The window must be locked. */
void
AG_WindowSetGeometry(AG_Window *win, int x, int y, int w, int h)
{
	SDL_Rect rfill1, rfill2;
	int ox = AGWIDGET(win)->x;
	int oy = AGWIDGET(win)->y;
	int ow = AGWIDGET(win)->w;
	int oh = AGWIDGET(win)->h;
	
	if (AGWIDGET(win)->x == -1 && AGWIDGET(win)->y == -1) {
		/* Find the minimum geometry. */
		AG_WindowScale(win, -1, -1);
	}

	/* Limit the window within the view boundaries. */
	if (x+w > agView->w) {
		AGWIDGET(win)->x = ox;
		AGWIDGET(win)->w = ow;
	} else {
		AGWIDGET(win)->x = x >= 0 ? x : 0;
		AGWIDGET(win)->w = w < win->minw ? win->minw : w;
	}
	if (y+h > agView->h) {
		AGWIDGET(win)->y = oy;
		AGWIDGET(win)->h = oh;
	} else {
		AGWIDGET(win)->y = y >= 0 ? y : 0;
		AGWIDGET(win)->h = h < win->minh ? win->minh : h;
	}

	/* Effect the possible changes in geometry. */
	AG_WINDOW_UPDATE(win);

	/* Update the background. */
	rfill1.w = 0;
	rfill2.w = 0;

	if (AGWIDGET(win)->x > ox) {			/* L-resize */
		rfill1.x = ox;
		rfill1.y = oy;
		rfill1.w = AGWIDGET(win)->x - ox;
		rfill1.h = AGWIDGET(win)->h;
	} else if (AGWIDGET(win)->w < ow) {		/* R-resize */
		rfill1.x = AGWIDGET(win)->x + AGWIDGET(win)->w;
		rfill1.y = AGWIDGET(win)->y;
		rfill1.w = ow - AGWIDGET(win)->w;
		rfill1.h = oh;
	}
	if (AGWIDGET(win)->h < oh) {			/* H-resize */
		rfill2.x = ox;
		rfill2.y = AGWIDGET(win)->y + AGWIDGET(win)->h;
		rfill2.w = ow;
		rfill2.h = oh - AGWIDGET(win)->h;
	}
	if (!agView->opengl) {
		if (rfill1.w > 0) {
			SDL_FillRect(agView->v, &rfill1, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rfill1);
		}
		if (rfill2.w > 0) {
			SDL_FillRect(agView->v, &rfill2, AG_COLOR(BG_COLOR));
			SDL_UpdateRects(agView->v, 1, &rfill2);
		}
	}
}

void
AG_WindowSaveGeometry(AG_Window *win)
{
	win->savx = AGWIDGET(win)->x;
	win->savy = AGWIDGET(win)->y;
	win->savw = AGWIDGET(win)->w;
	win->savh = AGWIDGET(win)->h;
}

void
AG_WindowMaximize(AG_Window *win)
{
	AG_WindowSaveGeometry(win);
	AG_WindowSetGeometry(win, 0, 0, agView->w, agView->h);
	win->flags |= AG_WINDOW_MAXIMIZED;
}

void
AG_WindowUnmaximize(AG_Window *win)
{
	AG_WindowSetGeometry(win, win->savx, win->savy, win->savw, win->savh);
	win->flags &= ~(AG_WINDOW_MAXIMIZED);
	if (!agView->opengl) {
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, agView->v->w, agView->v->h);
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
AG_WindowResizeOp(int op, AG_Window *win, SDL_MouseMotionEvent *motion)
{
	int x = AGWIDGET(win)->x;
	int y = AGWIDGET(win)->y;
	int w = AGWIDGET(win)->w;
	int h = AGWIDGET(win)->h;

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
#if 0
	if (!agWindowAnySize) {
		if (w < win->minw)
			w = win->minw;
		if (h < win->minh)
			h = win->minh;
	}
	if (x < 0) x = 0;
	if (y < 0) y = 0;
#endif
	AG_WindowSetGeometry(win, x, y, w, h);
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

void
AG_WindowScale(void *p, int w, int h)
{
	AG_Window *win = p;
	AG_Widget *wid;
	int totfixed = 0;
	int x = win->xpadding, dx;
	int y, dy;
	int nwidgets = 0;
	int isTitlebar = 0;

	AG_MutexLock(&win->lock);

	if ((win->flags & AG_WINDOW_NOTITLE) &&
	    (win->flags & AG_WINDOW_NOBORDERS) == 0) {
		y = win->ypadding_top + win->spacing;
	} else {
		y = 0;
	}

	if (w == -1 && h == -1) {
		int maxw = 0;

		AGWIDGET(win)->w = win->xpadding*2;
		AGWIDGET(win)->h = win->ypadding_top + win->ypadding_bot;
		if (win->flags & AG_WINDOW_NOTITLE)
			AGWIDGET(win)->h += win->spacing;

		AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
			wid->x = x;
			wid->y = y;
			AGWIDGET_OPS(wid)->scale(wid, -1, -1);
			if (maxw < wid->w)
				maxw = wid->w;

			dx = maxw + win->xpadding*2;
			if (AGWIDGET(win)->w < dx)
				AGWIDGET(win)->w = dx;

			isTitlebar = AG_ObjectIsClass(wid,
			    "AG_Widget:AG_Box:AG_Titlebar");
			dy = wid->h + (isTitlebar ? win->ypadding_top :
				       win->spacing);
			y += dy;
			AGWIDGET(win)->h += dy;
		}
		AGWIDGET(win)->h -= win->spacing;

		win->minw = AGWIDGET(win)->w;
		win->minh = AGWIDGET(win)->h;
		goto out;
	}

	/* Sum the space requested by fixed widgets. */
	AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
		AGWIDGET_OPS(wid)->scale(wid, -1, -1);
		if ((wid->flags & AG_WIDGET_VFILL) == 0)
			totfixed += wid->h + win->spacing;
	}
	if (totfixed > win->spacing)
		totfixed -= win->spacing;

	AGOBJECT_FOREACH_CHILD(wid, win, ag_widget) {
		wid->x = x;
		wid->y = y;
			
		isTitlebar = AG_ObjectIsClass(wid,
		    "AG_Widget:AG_Box:AG_Titlebar");

		if (wid->flags & AG_WIDGET_HFILL) {
			if (isTitlebar) {
				wid->w = w - 1;
			} else {
				wid->w = w - win->xpadding*2 - 1;
			}
		}
		if (wid->flags & AG_WIDGET_VFILL) {
			wid->h = h - totfixed - win->ypadding_bot -
			                        win->ypadding_top - 2;
		}
		AGWIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
		y += wid->h;
		y += isTitlebar ? win->ypadding_top : win->spacing;
		nwidgets++;
	}

	if (win->tbar != NULL) {
		AGWIDGET(win->tbar)->x = 0;
		AGWIDGET(win->tbar)->y = 0;
		AGWIDGET(win->tbar)->w = w;
	}
out:
	AG_WindowClamp(win);
	AG_MutexUnlock(&win->lock);
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
AG_WindowSetPadding(AG_Window *win, int xpadding, int ypadding_top,
    int ypadding_bot)
{
	AG_MutexLock(&win->lock);
	if (xpadding >= 0)	{ win->xpadding = xpadding; }
	if (ypadding_top >= 0)	{ win->ypadding_top = ypadding_top; }
	if (ypadding_bot >= 0)	{ win->ypadding_bot = ypadding_bot; }
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
		AGWIDGET(win)->x = 0;
		AGWIDGET(win)->y = 0;
		break;
	case AG_WINDOW_MIDDLE_LEFT:
		AGWIDGET(win)->x = 0;
		AGWIDGET(win)->y = agView->h/2 - AGWIDGET(win)->h/2;
		break;
	case AG_WINDOW_LOWER_LEFT:
		AGWIDGET(win)->x = 0;
		AGWIDGET(win)->y = agView->h - AGWIDGET(win)->h;
		break;
	case AG_WINDOW_UPPER_RIGHT:
		AGWIDGET(win)->x = agView->w - AGWIDGET(win)->w;
		AGWIDGET(win)->y = 0;
		break;
	case AG_WINDOW_MIDDLE_RIGHT:
		AGWIDGET(win)->x = agView->w - AGWIDGET(win)->w;
		AGWIDGET(win)->y = agView->h/2 - AGWIDGET(win)->h/2;
		break;
	case AG_WINDOW_LOWER_RIGHT:
		AGWIDGET(win)->x = agView->w - AGWIDGET(win)->w;
		AGWIDGET(win)->y = agView->h - AGWIDGET(win)->h;
		break;
	case AG_WINDOW_CENTER:
		AGWIDGET(win)->x = agView->w/2 - AGWIDGET(win)->w/2;
		AGWIDGET(win)->y = agView->h/2 - AGWIDGET(win)->h/2;
		break;
	case AG_WINDOW_LOWER_CENTER:
		AGWIDGET(win)->x = agView->w/2 - AGWIDGET(win)->w/2;
		AGWIDGET(win)->y = agView->h - AGWIDGET(win)->h;
		break;
	case AG_WINDOW_UPPER_CENTER:
		AGWIDGET(win)->x = agView->w/2 - AGWIDGET(win)->w/2;
		AGWIDGET(win)->y = 0;
		break;
	}
	AG_WindowClamp(win);
}

/* Set the text to show inside a window's titlebar. */
void
AG_WindowSetCaption(AG_Window *win, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;
	SDL_Surface *su;

#ifndef DEBUG
	if (win->tbar == NULL)
		return;
#endif
	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);

	if (win->tbar != NULL) {
		AG_TitlebarSetCaption(win->tbar, s);
	}
	strlcpy(win->caption, s, sizeof(win->caption));
}

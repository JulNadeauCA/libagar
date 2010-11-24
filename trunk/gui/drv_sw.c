/*
 * Copyright (c) 2009-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Single-window graphics driver framework. Under this framework, the driver
 * creates a single "native" display context and Agar emulates a window
 * manager internally.
 */

#include <config/have_sdl.h>
#include <config/have_opengl.h>
#include <config/have_glx.h>

#include <core/core.h>

#include "gui.h"
#include "window.h"
#include "menu.h"
#include "icons.h"

static void (*agVideoResizeCallback)(Uint w, Uint h) = NULL;

static void
Init(void *obj)
{
	AG_DriverSw *dsw = obj;
	Uint i;

	dsw->w = 0;
	dsw->h = 0;
	dsw->depth = 0;
	dsw->flags = 0;
	dsw->winop = AG_WINOP_NONE;
	dsw->winSelected = NULL;
	dsw->winLastKeydown = NULL;
	dsw->style = &agStyleDefault;
	dsw->rNom = 20;
	dsw->rCur = 0;

	for (i = 0; i < AG_WINDOW_ALIGNMENT_LAST; i++) {
		dsw->windowCurX[i] = 0;
		dsw->windowCurY[i] = 0;
	}
	dsw->windowXOutLimit = 32;
	dsw->windowBotOutLimit = 32;
	dsw->windowIconWidth = 32;
	dsw->windowIconHeight = 32;

	if ((dsw->Lmodal = AG_ListNew()) == NULL)
		AG_FatalError(NULL);
}

static void
Destroy(void *obj)
{
	AG_DriverSw *dsw = obj;
	
	if (dsw->Lmodal != NULL)
		AG_ListDestroy(dsw->Lmodal);
}

/*
 * Generic background popup menu.
 */

static void
UnminimizeWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	AG_WindowUnminimize(win);
}

#ifdef AG_DEBUG
static void
OpenGuiDebugger(AG_Event *event)
{
	AG_Window *win;

	if ((win = AG_GuiDebugger()) != NULL)
		AG_WindowShow(win);
}
#endif /* AG_DEBUG */

static void
ExitApplication(AG_Event *event)
{
	AG_Quit();
}

/* Display a generic popup menu for the display background. */
void
AG_WM_BackgroundPopupMenu(AG_DriverSw *dsw)
{
	AG_Menu *me;
	AG_MenuItem *mi;
	AG_Window *win;
	int nWindows = 0;

	me = AG_MenuNew(NULL, 0);
	mi = me->itemSel = AG_MenuNode(me->root, NULL, NULL);

	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		if (strcmp(win->caption, "win-popup") == 0) {
			continue;
		}
		AG_MenuAction(mi,
		    win->caption[0] != '\0' ? win->caption : _("Untitled"),
		    agIconWinMaximize.s,
		    UnminimizeWindow, "%p", win);
		nWindows++;
	}
	if (nWindows > 0) {
		AG_MenuSeparator(mi);
	}
#ifdef AG_DEBUG
	AG_MenuAction(mi, _("GUI debugger"), agIconMagnifier.s,
	    OpenGuiDebugger, NULL);
#endif
	AG_MenuAction(mi, _("Exit application"), agIconWinClose.s,
	    ExitApplication, NULL);
				
	AG_MenuExpand(NULL, mi,
	    AGDRIVER(dsw)->mouse->x + 4,
	    AGDRIVER(dsw)->mouse->y + 4);
}

/* Resize a video display to the specified dimensions. */
int
AG_ResizeDisplay(int w, int h)
{
	AG_Window *win;

	if (agDriverSw == NULL) {
		AG_SetError("AG_ResizeDisplay() is only applicable to "
		            "single-window graphics drivers");
		return (-1);
	}
	if (AGDRIVER_SW_CLASS(agDriverSw)->videoResize(agDriverSw,
	    (Uint)w, (Uint)h) == -1)
		return (-1);

	/* Update the Agar window geometries. */
	AG_FOREACH_WINDOW(win, agDriverSw) {
		AG_SizeAlloc a;

		a.x = WIDGET(win)->x;
		a.y = WIDGET(win)->y;
		a.w = WIDGET(win)->w;
		a.h = WIDGET(win)->h;

		AG_ObjectLock(win);

		if (win->flags & AG_WINDOW_MAXIMIZED) {
			AG_WindowSetGeometryMax(win);
		} else {
			if (win->flags & AG_WINDOW_HMAXIMIZE) {
				a.x = 0;
				a.w = agDriverSw->w;
			} else {
				if (a.x+a.w > agDriverSw->w) {
					a.x = agDriverSw->w - a.w;
					if (a.x < 0) {
						a.x = 0;
						a.w = agDriverSw->w;
					}
				}
			}
			if (win->flags & AG_WINDOW_VMAXIMIZE) {
				a.y = 0;
				a.h = agDriverSw->h;
			} else {
				if (a.y+a.h > agDriverSw->h) {
					a.y = agDriverSw->h - a.h;
					if (a.y < 0) {
						a.y = 0;
						a.h = agDriverSw->w;
					}
				}
			}
			AG_WidgetSizeAlloc(win, &a);
			AG_WindowUpdate(win);
		}
		AG_ObjectUnlock(win);
	}
	if (agVideoResizeCallback != NULL) {
		agVideoResizeCallback(w, h);
	}
	return (0);
}

/* Configure a video resize callback routine. */
void
AG_SetVideoResizeCallback(void (*fn)(Uint w, Uint h))
{
	agVideoResizeCallback = fn;
}

/* Process a window move initiated by the WM. */
static void
WM_Move(AG_Window *win, int xRel, int yRel)
{
	AG_DriverSw *dsw = (AG_DriverSw *)WIDGET(win)->drv;
	AG_Rect rPrev, rNew;
	AG_Rect rFill1, rFill2;

	rPrev.x = WIDGET(win)->x;
	rPrev.y = WIDGET(win)->y;
	rPrev.w = WIDTH(win);
	rPrev.h = HEIGHT(win);

	WIDGET(win)->x += xRel;
	WIDGET(win)->y += yRel;
	AG_WM_LimitWindowToView(win);

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

	if (rFill1.w > 0) {
		AGDRIVER_CLASS(dsw)->fillRect(dsw, rFill1, agColors[BG_COLOR]);
		if (AGDRIVER_CLASS(dsw)->updateRegion != NULL)
			AGDRIVER_CLASS(dsw)->updateRegion(dsw, rFill1);
	}
	if (rFill2.w > 0) {
		AGDRIVER_CLASS(dsw)->fillRect(dsw, rFill2, agColors[BG_COLOR]);
		if (AGDRIVER_CLASS(dsw)->updateRegion != NULL)
			AGDRIVER_CLASS(dsw)->updateRegion(dsw, rFill2);
	}

	AG_PostEvent(NULL, win, "window-user-move", "%d,%d",
	    WIDGET(win)->x, WIDGET(win)->y);
}

/* Process a window resize operation initiated by the WM. */
static void
WM_Resize(int op, AG_Window *win, int xRel, int yRel)
{
	int x = WIDGET(win)->x;
	int y = WIDGET(win)->y;
	int w = WIDTH(win);
	int h = HEIGHT(win);

	switch (op) {
	case AG_WINOP_LRESIZE:
		if (!(win->flags & AG_WINDOW_NOHRESIZE)) {
			if (xRel < 0) {
				w -= xRel;
				x += xRel;
			} else if (xRel > 0) {
				w -= xRel;
				x += xRel;
			}
		}
		if (!(win->flags & AG_WINDOW_NOVRESIZE)) {
			if (yRel < 0 || yRel > 0)
				h += yRel;
		}
		break;
	case AG_WINOP_RRESIZE:
		if (!(win->flags & AG_WINDOW_NOHRESIZE)) {
			if (xRel < 0 || xRel > 0)
				w += xRel;
		}
		if (!(win->flags & AG_WINDOW_NOVRESIZE)) {
			if (yRel < 0 || yRel > 0)
				h += yRel;
		}
		break;
	case AG_WINOP_HRESIZE:
		if (!(win->flags & AG_WINDOW_NOHRESIZE)) {
			if (yRel < 0 || yRel > 0)
				h += yRel;
		}
		break;
	default:
		break;
	}
	AG_WindowSetGeometry(win, x, y, w, h);
	AG_PostEvent(NULL, win, "window-user-resize", "%d,%d", w, h);
}


/*
 * Window-manager specific processing of mouse-motion events. This is
 * called by low-level event processing code whenever a WM operation is
 * in effect.
 */
void
AG_WM_MouseMotion(AG_DriverSw *dsw, AG_Window *win, int xRel, int yRel)
{
	switch (dsw->winop) {
	case AG_WINOP_MOVE:
		WM_Move(win, xRel, yRel);
		break;
	case AG_WINOP_LRESIZE:
		WM_Resize(AG_WINOP_LRESIZE, win, xRel, yRel);
		break;
	case AG_WINOP_RRESIZE:
		WM_Resize(AG_WINOP_RRESIZE, win, xRel, yRel);
		break;
	case AG_WINOP_HRESIZE:
		WM_Resize(AG_WINOP_HRESIZE, win, xRel, yRel);
		break;
	default:
		break;
	}
}

/*
 * Reorder the window list and post the appropriate Window events following
 * a change in focus (single-display drivers only).
 */
void
AG_WM_CommitWindowFocus(AG_Window *win)
{
	if (agWindowFocused != NULL) {
		if (win != NULL &&
		    win == agWindowFocused) {		/* Nothing to do */
			return;
		}
		AG_PostEvent(NULL, agWindowFocused, "window-lostfocus", NULL);
	}
	if (win != NULL) {
		AG_ObjectLock(win);
		if (!(win->flags & AG_WINDOW_KEEPBELOW)) {
			AG_ObjectMoveToTail(win);
		}
		agWindowFocused = win;
		AG_ObjectUnlock(win);
		AG_PostEvent(NULL, win, "window-gainfocus", NULL);
	} else {
		agWindowFocused = NULL;
	}
}

/* Limit window size to display size. */
void
AG_WM_LimitWindowToDisplaySize(AG_Driver *drv, AG_SizeAlloc *a)
{
	Uint wMax = 0, hMax = 0;

	AG_GetDisplaySize(drv, &wMax, &hMax);

	if (a->x+a->w > wMax) {
		a->w = wMax - a->x;
	}
	if (a->y+a->h > hMax) {
		a->h = hMax - a->y;
	}
	if (a->w < 0) {
		a->x = 0;
		a->w = wMax;
	}
	if (a->h < 0) {
		a->y = 0;
		a->h = hMax;
	}
}


/*
 * Limit the window geometry/coordinates to the view area.
 * The window must be locked.
 */
void
AG_WM_LimitWindowToView(AG_Window *win)
{
	AG_DriverSw *dsw = OBJECT(win)->parent;
	AG_Widget *w = WIDGET(win);

	if (w->x < 0) {
		w->x = 0;
	}
	if (w->y < 0) {
		w->y = 0;
	} else if (w->y > dsw->h - dsw->windowBotOutLimit) {
		w->y = dsw->h - dsw->windowBotOutLimit;
	}
	
#if 0
	if (w->x + w->w > dsw->w) { w->x = dsw->w - w->w; }
	if (w->y + w->h > dsw->h) { w->y = dsw->h - w->h; }
	if (w->x < 0) { w->x = 0; }
	if (w->y < 0) { w->y = 0; }

	if (w->x+w->w > dsw->w) {
		w->x = 0;
		w->w = dsw->w - 1;
	}
	if (w->y+w->h > dsw->h) {
		w->y = 0;
		w->h = dsw->h - 1;
	}
#endif
}

/* Compute default window coordinates from requested alignment settings. */
void
AG_WM_GetPrefPosition(AG_Window *win, int *x, int *y, int w, int h)
{
	AG_DriverSw *dsw = AGDRIVER_SW(WIDGET(win)->drv);
	int xOffs = 0, yOffs = 0;

	if (win->flags & AG_WINDOW_CASCADE) {
		xOffs = dsw->windowCurX[win->alignment];
		yOffs = dsw->windowCurY[win->alignment];
		dsw->windowCurX[win->alignment] += 16;
		dsw->windowCurY[win->alignment] += 16;
		if (dsw->windowCurX[win->alignment] > dsw->w)
			dsw->windowCurX[win->alignment] = 0;
		if (dsw->windowCurY[win->alignment] > dsw->h)
			dsw->windowCurY[win->alignment] = 0;
	}
	switch (win->alignment) {
	case AG_WINDOW_TL:
		*x = xOffs;
		*y = yOffs;
		break;
	case AG_WINDOW_TC:
		*x = dsw->w/2 - w/2 + xOffs;
		*y = 0;
		break;
	case AG_WINDOW_TR:
		*x = dsw->w - w - xOffs;
		*y = yOffs;
		break;
	case AG_WINDOW_ML:
		*x = xOffs;
		*y = dsw->h/2 - h/2 + yOffs;
		break;
	case AG_WINDOW_ALIGNMENT_NONE:
	case AG_WINDOW_MC:
		*x = dsw->w/2 - w/2 + xOffs;
		*y = dsw->h/2 - h/2 + yOffs;
		break;
	case AG_WINDOW_MR:
		*x = dsw->w - w - xOffs;
		*y = dsw->h/2 - h/2 + yOffs;
		break;
	case AG_WINDOW_BL:
		*x = xOffs;
		*y = dsw->h - h - yOffs;
		break;
	case AG_WINDOW_BC:
		*x = dsw->w/2 - w/2 + xOffs;
		*y = dsw->h - h;
		break;
	case AG_WINDOW_BR:
		*x = dsw->w - w - xOffs;
		*y = dsw->h - h - yOffs;
		break;
	default:
		break;
	}
}

AG_ObjectClass agDriverSwClass = {
	"AG_Driver:AG_DriverSw",
	sizeof(AG_DriverSw),
	{ 1,4 },
	Init,
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};

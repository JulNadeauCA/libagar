/*
 * Copyright (c) 2009-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/menu.h>
#include <agar/gui/icons.h>

AG_DriverSw *agDriverSw = NULL;		/* Root driver instance */

static void (*agVideoResizeCallback)(Uint w, Uint h) = NULL;

static void
Init(void *obj)
{
	AG_DriverSw *dsw = obj;

	dsw->w = 0;
	dsw->h = 0;
	dsw->depth = 0;
	dsw->flags = 0;
	dsw->winop = AG_WINOP_NONE;
	dsw->winSelected = NULL;
	dsw->winLastKeydown = NULL;
	dsw->rNom = 1000/60;
	dsw->rCur = 0;
	dsw->rLast = 0;
	dsw->windowXOutLimit = 32;
	dsw->windowBotOutLimit = 32;
	dsw->windowIconWidth = 32;
	dsw->windowIconHeight = 32;

	if ((dsw->Lmodal = AG_ListNew()) == NULL)
		AG_FatalError(NULL);

	AG_SetString(dsw, "bgColor", "rgb(0,0,0)");
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

	if ((win = AG_GuiDebugger(NULL)) != NULL)
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

/*
 * Update Agar window geometries following a display resize (for
 * single-window drivers).
 */
void
AG_PostResizeDisplay(AG_DriverSw *dsw)
{
	AG_Window *win;

	AG_FOREACH_WINDOW(win, dsw) {
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
				a.w = dsw->w;
			} else {
				if (a.x+a.w > dsw->w) {
					a.x = dsw->w - a.w;
					if (a.x < 0) {
						a.x = 0;
						a.w = dsw->w;
					}
				}
			}
			if (win->flags & AG_WINDOW_VMAXIMIZE) {
				a.y = 0;
				a.h = dsw->h;
			} else {
				if (a.y+a.h > dsw->h) {
					a.y = dsw->h - a.h;
					if (a.y < 0) {
						a.y = 0;
						a.h = dsw->w;
					}
				}
			}
			AG_WidgetSizeAlloc(win, &a);
			AG_WindowUpdate(win);
		}
		win->dirty = 1;
		AG_ObjectUnlock(win);
	}
	if (agVideoResizeCallback != NULL)
		agVideoResizeCallback(dsw->w, dsw->h);
}

/* Resize a video display to the specified dimensions. */
int
AG_ResizeDisplay(int w, int h)
{
	if (agDriverSw == NULL) {
		AG_SetError("AG_ResizeDisplay() is only applicable to "
		            "single-window graphics drivers");
		return (-1);
	}
	if (AGDRIVER_SW_CLASS(agDriverSw)->videoResize(agDriverSw,
	    (Uint)w, (Uint)h) == -1) {
		return (-1);
	}
	AG_PostResizeDisplay(agDriverSw);
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
	AG_DriverClass *dc = AGDRIVER_CLASS(dsw);
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

	if (dc->type == AG_FRAMEBUFFER) {
		/* Update the background. */
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
			AGDRIVER_CLASS(dsw)->fillRect(dsw, rFill1, dsw->bgColor);
			if (AGDRIVER_CLASS(dsw)->updateRegion != NULL)
				AGDRIVER_CLASS(dsw)->updateRegion(dsw, rFill1);
		}
		if (rFill2.w > 0) {
			AGDRIVER_CLASS(dsw)->fillRect(dsw, rFill2, dsw->bgColor);
			if (AGDRIVER_CLASS(dsw)->updateRegion != NULL)
				AGDRIVER_CLASS(dsw)->updateRegion(dsw, rFill2);
		}
	}

	win->dirty = 1;

	AG_WindowMovePinned(win, xRel, yRel);
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
	}
	AG_WindowSetGeometry(win, x, y, w, h);
}

/* Start window move operation. */
void
AG_WM_MoveBegin(AG_Window *win)
{
	AG_DriverSw *dsw = (AG_DriverSw *)WIDGET(win)->drv;
	
	if (!(win->flags & AG_WINDOW_DENYFOCUS)) {
		agWindowToFocus = win;
	}
	dsw->winSelected = win;
	if (!(win->flags & AG_WINDOW_NOMOVE))
		dsw->winop = AG_WINOP_MOVE;
}

/* Terminate window move operation. */
void
AG_WM_MoveEnd(AG_Window *win)
{
	AG_DriverSw *dsw = (AG_DriverSw *)WIDGET(win)->drv;

	dsw->winop = AG_WINOP_NONE;
	dsw->winSelected = NULL;
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
#ifdef AG_DEBUG
	if (win->flags & AG_WINDOW_DENYFOCUS)
		AG_FatalError("Window is not focusable");
#endif
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
		AG_PostEvent(NULL, win, "window-gainfocus", NULL);
		win->dirty = 1;
		AG_ObjectUnlock(win);
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

/* Compute default positions for AG_WINDOW_TILING windows */
static void
GetTilingPosition(AG_Window *win, int *xDst, int *yDst, int w, int h)
{
	AG_DriverSw *dsw = AGDRIVER_SW(WIDGET(win)->drv);
	AG_Window *wOther;
	const int maxTests = 10000, dx = 16;
	int nTest = 0;
	int x = 0, y = 0, xo, yo, wo, ho;
	int xd = 0, yd = 0;

	switch (win->alignment) {
	case AG_WINDOW_TL:	xd = 0;			yd = 0;			break;
	case AG_WINDOW_TC:	xd = dsw->w/2 - w/2;	yd = 0;			break;
	case AG_WINDOW_TR:	xd = dsw->w - w;	yd = 0;			break;
	case AG_WINDOW_ML:	xd = 0;			yd = dsw->h/2 - h/2;	break;
	case AG_WINDOW_MR:	xd = dsw->w - w;	yd = dsw->h/2 - h/2;	break;
	case AG_WINDOW_BL:	xd = 0;			yd = dsw->h - h;	break;
	case AG_WINDOW_BC:	xd = dsw->w/2 - w/2;	yd = dsw->h - h;	break;
	case AG_WINDOW_BR:	xd = dsw->w - w;	yd = dsw->h - h;	break;
	default:
	case AG_WINDOW_ALIGNMENT_NONE:
	case AG_WINDOW_MC:	xd = dsw->w/2 - w/2;	yd = dsw->h/2 - h/2;	break;
	}
	x = xd;
	y = yd;

	for (;;) {
		OBJECT_FOREACH_CHILD(wOther, dsw, ag_window) {
			if (wOther == win ||
			    (wOther->flags & AG_WINDOW_TILING) == 0) {
				continue;
			}
			xo = WIDGET(wOther)->x;
			yo = WIDGET(wOther)->y;
			wo = WIDGET(wOther)->w;
			ho = WIDGET(wOther)->h;
			if (x <= xo+wo && x+w >= xo &&
			    y <= yo+ho && y+h >= yo) {
				switch (win->alignment) {
				case AG_WINDOW_TL:
				case AG_WINDOW_TC:
				case AG_WINDOW_ML:
				case AG_WINDOW_MC:
				default:
					x += dx;
					if (x+w > dsw->w) {
						x = 0;
						y += dx;
						if (y > dsw->h)
							goto fail;
					}
					break;
				case AG_WINDOW_TR:
				case AG_WINDOW_MR:
					x -= dx;
					if (x < 0) {
						x = dsw->w - w;
						y += dx;
						if (y > dsw->h)
							goto fail;
					}
					break;
				case AG_WINDOW_BL:
				case AG_WINDOW_BC:
					x += dx;
					if (x+w > dsw->w) {
						x = 0;
						y -= dx;
						if (y < 0)
							goto fail;
					}
					break;
				case AG_WINDOW_BR:
					x -= dx;
					if (x < 0) {
						x = dsw->w - w;
						y -= dx;
						if (y < 0)
							goto fail;
					}
					break;
				}
				break;
			}
		}
		if (wOther == NULL) {
			goto out;
		}
		if (++nTest > maxTests)
			goto fail;
	}
out:
	*xDst = x;
	*yDst = y;
	return;
fail:
	Verbose("Window tiling failed\n");
	*xDst = xd;
	*yDst = yd;
}

/* Compute default window coordinates from requested alignment settings. */
void
AG_WM_GetPrefPosition(AG_Window *win, int *x, int *y, int w, int h)
{
	AG_DriverSw *dsw = AGDRIVER_SW(WIDGET(win)->drv);
	int xOffs = 0, yOffs = 0;

	if (win->flags & AG_WINDOW_TILING) {
		GetTilingPosition(win, x, y, w, h);
		return;
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
	default:
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

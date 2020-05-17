/*
 * Copyright (c) 2009-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <string.h>
#include <ctype.h>

AG_DriverSw *agDriverSw = NULL;		/* Root driver instance */

static void (*_Nullable agVideoResizeCallback)(Uint w, Uint h) = NULL;

static void
Init(void *_Nonnull obj)
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
	dsw->windowIconWidth = 32;
	dsw->windowIconHeight = 32;
	dsw->bgPopup = NULL;

	AG_SetString(dsw, "bgColor", "rgb(0,0,0)");
}

/*
 * Generic background popup menu.
 */

static void
UnminimizeWindow(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_WindowUnminimize(win);
}

#if defined(AG_DEBUG)
static void
OpenDebugger(AG_Event *_Nonnull event)
{
	AG_Window *win;

	if ((win = AG_GuiDebugger(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}
#endif /* AG_DEBUG */

static void
OpenStyleEditor(AG_Event *_Nonnull event)
{
	AG_Window *win;

	if ((win = AG_StyleEditor(agWindowFocused)) != NULL)
		AG_WindowShow(win);
}

static void
ExitApplication(AG_Event *_Nonnull event)
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

	if (dsw->bgPopup) {
		AG_MenuCollapseAll(dsw->bgPopup);
	}
	me = dsw->bgPopup = AG_MenuNew(NULL, 0);
	mi = me->itemSel = AG_MenuNode(me->root, NULL, NULL);

	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		switch (win->wmType) {
		case AG_WINDOW_WM_DND:
		case AG_WINDOW_WM_DOCK:
		case AG_WINDOW_WM_COMBO:
		case AG_WINDOW_WM_POPUP_MENU:
		case AG_WINDOW_WM_DROPDOWN_MENU:
			continue;
		default:
			break;
		}
		AG_MenuAction(mi,
		    (win->caption[0] != '\0') ? win->caption : _("Untitled"),
		    agIconWinMaximize.s,
		    UnminimizeWindow, "%p", win);
		nWindows++;
	}
	if (nWindows > 0) {
		AG_MenuSeparator(mi);
	}
	AG_MenuAction(mi, _("Style editor"), agIconGear.s, OpenStyleEditor, NULL);
#ifdef AG_DEBUG
	AG_MenuAction(mi, _("GUI debugger"), agIconMagnifier.s, OpenDebugger, NULL);
#else
	AG_MenuDisable(AG_MenuNode(mi, _("GUI debugger"), agIconMagnifier.s));
#endif
	AG_MenuSeparator(mi);
	AG_MenuAction(mi, _("Exit application"), agIconWinClose.s,
	    ExitApplication, NULL);
				
	win = AG_MenuExpand(NULL, mi,
	    AGDRIVER(dsw)->mouse->x + 4,
	    AGDRIVER(dsw)->mouse->y + 4);
	if (win == NULL) {
		Verbose("%s; ignoring\n", AG_GetError());
		AG_ObjectDestroy(dsw->bgPopup);		/* Undo */
		dsw->bgPopup = NULL;
	}
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

/* Handle a window displacement initiated by the WM. */
static void
WM_Move(AG_Window *_Nonnull win, int xRel, int yRel)
{
	AG_DriverSw *dsw = (AG_DriverSw *)WIDGET(win)->drv;
	AG_DriverClass *dc = AGDRIVER_CLASS(dsw);
	AG_Rect rPrev, rNew;
	AG_Rect a, b;

	rPrev.x = WIDGET(win)->x;
	rPrev.y = WIDGET(win)->y;
	rPrev.w = WIDTH(win);
	rPrev.h = HEIGHT(win);

	WIDGET(win)->x += xRel;
	WIDGET(win)->y += yRel;

	AG_WidgetUpdateCoords(win, WIDGET(win)->x, WIDGET(win)->y);

	if (dc->type == AG_FRAMEBUFFER) {          /* Update the background. */
		rNew.x = WIDGET(win)->x;
		rNew.y = WIDGET(win)->y;
		rNew.w = WIDTH(win);
		rNew.h = HEIGHT(win);
		a.w = 0;
		b.w = 0;
		if (rNew.x > rPrev.x) {				/* Right */
			a.x = rPrev.x - 2;
			a.y = rPrev.y;
			a.w = rNew.x - rPrev.x + 1;
			a.h = rNew.h + 1;
		} else if (rNew.x < rPrev.x) {			/* Left */
			a.x = rNew.x + rNew.w - 1;
			a.y = rNew.y - 1;
			a.w = rPrev.x - rNew.x + 2;
			a.h = rPrev.h + 2;
		}
		if (rNew.y > rPrev.y) {				/* Down */
			b.x = rPrev.x - 1;
			b.y = rPrev.y;
			b.w = rNew.w + 2;
			b.h = rNew.y - rPrev.y;
		} else if (rNew.y < rPrev.y) {			/* Up */
			b.x = rPrev.x - 1;
			b.y = rNew.y + rNew.h;
			b.w = rPrev.w + 2;
			b.h = rPrev.y - rNew.y + 1;
		}
		if (a.w > 0) {
			AGDRIVER_CLASS(dsw)->fillRect(dsw, &a, &dsw->bgColor);
			if (AGDRIVER_CLASS(dsw)->updateRegion != NULL)
				AGDRIVER_CLASS(dsw)->updateRegion(dsw, &a);
		}
		if (b.w > 0) {
			AGDRIVER_CLASS(dsw)->fillRect(dsw, &b, &dsw->bgColor);
			if (AGDRIVER_CLASS(dsw)->updateRegion != NULL)
				AGDRIVER_CLASS(dsw)->updateRegion(dsw, &b);
		}
	}

	win->dirty = 1;

	if (agWindowPinnedCount > 0)
		AG_WindowMovePinned(win, xRel, yRel);
}

/* Handle a window resize operation initiated by the WM. */
static void
WM_Resize(int op, AG_Window *_Nonnull win, int xRel, int yRel)
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
	if (agWindowFocused) {
		if (win &&
		    win == agWindowFocused) {		/* Nothing to do */
			return;
		}
		AG_PostEvent(agWindowFocused, "window-lostfocus", NULL);
	}
	if (win) {
		AG_ObjectLock(win);
		if (!(win->flags & AG_WINDOW_KEEPBELOW)) {
			AG_ObjectMoveToTail(win);
		}
		agWindowFocused = win;
		AG_PostEvent(win, "window-gainfocus", NULL);
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


/* Compute default positions for AG_WINDOW_TILING windows */
/* TODO optimize this */
static void
GetTilingPosition(AG_Window *_Nonnull win, int *_Nonnull xDst,
    int *_Nonnull yDst, int w, int h)
{
	AG_DriverSw *dsw = AGDRIVER_SW(WIDGET(win)->drv);
	AG_Window *wOther;
	const int maxTests = 10000, dx = 16;
	int nTest = 0;
	int dw = dsw->w;
	int dh = dsw->h;
	int x,y, xd, yd;

	switch (win->alignment) {
	case AG_WINDOW_TL: xd = 0;		yd = 0;			break;
	case AG_WINDOW_TC: xd = (dw>>1)-(w>>1);	yd = 0;			break;
	case AG_WINDOW_TR: xd = dw-w;		yd = 0;			break;
	case AG_WINDOW_ML: xd = 0;		yd = (dh>>1)-(h>>1);	break;
	case AG_WINDOW_MR: xd = dw-w;		yd = (dh>>1)-(h>>1);	break;
	case AG_WINDOW_BL: xd = 0;		yd = dh - h;		break;
	case AG_WINDOW_BC: xd = (dw>>1)-(w>>1);	yd = dh - h;		break;
	case AG_WINDOW_BR: xd = dw-w;		yd = dh - h;		break;
	default:
	case AG_WINDOW_ALIGNMENT_NONE:
	case AG_WINDOW_MC: xd = (dw>>1)-(w>>1);	yd = (dh>>1)-(h>>1);	break;
	}
	x = xd;
	y = yd;

	for (;;) {
		OBJECT_FOREACH_CHILD(wOther, dsw, ag_window) {
			int xo,yo, wo,ho;

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
					if (x+w > dw) {
						x = 0;
						y += dx;
						if (y > dh)
							goto fail;
					}
					break;
				case AG_WINDOW_TR:
				case AG_WINDOW_MR:
					x -= dx;
					if (x < 0) {
						x = dw - w;
						y += dx;
						if (y > dh)
							goto fail;
					}
					break;
				case AG_WINDOW_BL:
				case AG_WINDOW_BC:
					x += dx;
					if (x+w > dw) {
						x = 0;
						y -= dx;
						if (y < 0)
							goto fail;
					}
					break;
				case AG_WINDOW_BR:
					x -= dx;
					if (x < 0) {
						x = dw - w;
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
		*x = (dsw->w >> 1) - (w >> 1) + xOffs;
		*y = 0;
		break;
	case AG_WINDOW_TR:
		*x = dsw->w - w - xOffs;
		*y = yOffs;
		break;
	case AG_WINDOW_ML:
		*x = xOffs;
		*y = (dsw->h >> 1) - (h >> 1) + yOffs;
		break;
	default:
	case AG_WINDOW_ALIGNMENT_NONE:
	case AG_WINDOW_MC:
		*x = (dsw->w >> 1) - (w >> 1) + xOffs;
		*y = (dsw->h >> 1) - (h >> 1) + yOffs;
		break;
	case AG_WINDOW_MR:
		*x = dsw->w - w - xOffs;
		*y = (dsw->h >> 1) - (h >> 1) + yOffs;
		break;
	case AG_WINDOW_BL:
		*x = xOffs;
		*y = dsw->h - h - yOffs;
		break;
	case AG_WINDOW_BC:
		*x = (dsw->w >> 1) - (w >> 1) + xOffs;
		*y = dsw->h - h;
		break;
	case AG_WINDOW_BR:
		*x = dsw->w - w - xOffs;
		*y = dsw->h - h - yOffs;
		break;
	}
}

/* Blank the display background. */
void
AG_ClearBackground(void)
{
	AG_DriverSw *dsw;

	if ((dsw = agDriverSw) == NULL) {
		return;
	}
	AGDRIVER_SW_CLASS(dsw)->videoClear(dsw, &dsw->bgColor);
}

/* Configure the display refresh rate (driver-dependent). */
int
AG_SetRefreshRate(int fps)
{
	if (agDriverOps->setRefreshRate == NULL) {
		AG_SetError("Refresh rate not applicable to graphics driver");
		return (-1);
	}
	return agDriverOps->setRefreshRate(agDriverSw, fps);
}

/* Evaluate whether there are pending events to be processed. */
int
AG_PendingEvents(AG_Driver *drv)
{
	if (drv) {
		return AGDRIVER_CLASS(drv)->pendingEvents(drv);
	} else {
		return agDriverOps->pendingEvents(agDriverSw);
	}
}

/* Retrieve the next pending event, translated to generic AG_DriverEvent form. */
int
AG_GetNextEvent(AG_Driver *drv, AG_DriverEvent *dev)
{
	if (drv) {
		return AGDRIVER_CLASS(drv)->getNextEvent(drv, dev);
	} else {
		return agDriverOps->getNextEvent(agDriverSw, dev);
	}
}

/* Process the next pending event in generic manner. */
int
AG_ProcessEvent(AG_Driver *drv, AG_DriverEvent *dev)
{
	if (drv) {
		return AGDRIVER_CLASS(drv)->processEvent(drv, dev);
	} else {
		return agDriverOps->processEvent(agDriverSw, dev);
	}
}

AG_ObjectClass agDriverSwClass = {
	"AG_Driver:AG_DriverSw",
	sizeof(AG_DriverSw),
	{ 1,4 },
	Init,
	NULL,		/* reset */
	NULL,		/* destroy */
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};

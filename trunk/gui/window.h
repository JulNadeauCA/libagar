/*	Public domain	*/

#ifndef _AGAR_GUI_WINDOW_H_
#define _AGAR_GUI_WINDOW_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

#define AG_WINDOW_CAPTION_MAX 512

struct ag_titlebar;
struct ag_icon;
struct ag_widget;
struct ag_cursor;

#define AG_WINDOW_UPPER_LEFT	AG_WINDOW_TL
#define AG_WINDOW_UPPER_CENTER	AG_WINDOW_TC
#define AG_WINDOW_UPPER_RIGHT	AG_WINDOW_TR
#define AG_WINDOW_MIDDLE_LEFT	AG_WINDOW_ML
#define AG_WINDOW_CENTER	AG_WINDOW_MC
#define AG_WINDOW_MIDDLE_RIGHT	AG_WINDOW_MR
#define AG_WINDOW_LOWER_LEFT	AG_WINDOW_BL
#define AG_WINDOW_LOWER_CENTER	AG_WINDOW_BC
#define AG_WINDOW_LOWER_RIGHT	AG_WINDOW_BR

/* For AG_WindowSetCloseAction() */
enum ag_window_close_action {
	AG_WINDOW_HIDE,
	AG_WINDOW_DETACH,
	AG_WINDOW_NONE
};

/* Cursor-change area */
typedef struct ag_cursor_area {
	AG_Rect r;					/* Area in window */
	struct ag_cursor *c;				/* Associated cursor */
	struct ag_widget *wid;				/* Associated widget */
	int stock;					/* Stock Agar cursor? */
	AG_TAILQ_ENTRY(ag_cursor_area) cursorAreas;
} AG_CursorArea;

/* Window instance */
typedef struct ag_window {
	struct ag_widget wid;

	Uint flags;
#define AG_WINDOW_MODAL		0x0000001 /* Place in foreground */
#define AG_WINDOW_MAXIMIZED	0x0000002 /* Window is maximized */
#define AG_WINDOW_MINIMIZED	0x0000004 /* Window is minimized */
#define AG_WINDOW_KEEPABOVE	0x0000008 /* Keep window above */
#define AG_WINDOW_KEEPBELOW	0x0000010 /* Keep window below */
#define AG_WINDOW_DENYFOCUS	0x0000020 /* Widgets cannot gain focus */
#define AG_WINDOW_NOTITLE	0x0000040 /* Disable the titlebar */
#define AG_WINDOW_NOBORDERS	0x0000080 /* Disable the window borders */
#define AG_WINDOW_NOHRESIZE	0x0000100 /* Disable horizontal resize */
#define AG_WINDOW_NOVRESIZE	0x0000200 /* Disable vertical resize */
#define AG_WINDOW_NOCLOSE	0x0000400 /* Disable close button */
#define AG_WINDOW_NOMINIMIZE	0x0000800 /* Disable minimize button */
#define AG_WINDOW_NOMAXIMIZE	0x0001000 /* Disable maximize button */
#define AG_WINDOW_CASCADE	0x0002000 /* For AG_WindowSetPosition() */
#define AG_WINDOW_MINSIZEPCT	0x0004000 /* Set minimum size in % */
#define AG_WINDOW_NOBACKGROUND	0x0008000 /* Don't fill the background */
#define AG_WINDOW_NOUPDATERECT	0x0010000 /* Unused */
#define AG_WINDOW_FOCUSONATTACH	0x0020000 /* Automatic focus on attach */
#define AG_WINDOW_HMAXIMIZE	0x0040000 /* Keep maximized horizontally */
#define AG_WINDOW_VMAXIMIZE	0x0080000 /* Keep maximized vertically */
#define AG_WINDOW_NOMOVE	0x0100000 /* Disallow movement of window */
#define AG_WINDOW_NOCLIPPING	0x0200000 /* Don't set a clipping rectangle over the window area */
#define AG_WINDOW_MODKEYEVENTS	0x0400000 /* Generate key{up,down} events for
                                            keypresses on modifier keys */
#define AG_WINDOW_DETACHING	0x0800000 /* Window is being detached */
#define AG_WINDOW_POPUP		0x1000000 /* "Popup" style (WM-dependent) */
#define AG_WINDOW_DIALOG	0x2000000 /* "Dialog" style (WM-dependent) */

#define AG_WINDOW_NORESIZE	(AG_WINDOW_NOHRESIZE|AG_WINDOW_NOVRESIZE)
#define AG_WINDOW_NOBUTTONS	(AG_WINDOW_NOCLOSE|AG_WINDOW_NOMINIMIZE|\
				 AG_WINDOW_NOMAXIMIZE)
#define AG_WINDOW_PLAIN		(AG_WINDOW_NOTITLE|AG_WINDOW_NOBORDERS)

	char caption[AG_WINDOW_CAPTION_MAX];	/* Window caption */
	int visible;				/* Window is visible */
	int dirty;				/* Window needs redraw */

	struct ag_titlebar *tbar;		/* Titlebar (if any) */
	enum ag_window_alignment alignment;	/* Initial position */
	int spacing;				/* Default spacing (px) */
	int tPad, bPad, lPad, rPad;		/* Window padding (px) */
	int wReq, hReq;				/* Requested geometry (px) */
	int wMin, hMin;				/* Minimum geometry (px) */
	int wBorderBot;				/* Bottom border size (px) */
	int wBorderSide;			/* Side border size (px) */
	int wResizeCtrl;			/* Resize controls size (px) */
	AG_Rect rSaved;				/* Saved geometry */
	int minPct;				/* For MINSIZEPCT */

	struct ag_window *parent;		/* Parent window */
	AG_TAILQ_HEAD_(ag_window) subwins;	/* For AG_WindowAttach() */
	AG_TAILQ_ENTRY(ag_window) swins;	/* In parent's subwins */
	AG_TAILQ_ENTRY(ag_window) detach;	/* In agWindowDetachQ */

	struct ag_icon *icon;			/* Window icon (for internal WM) */
	AG_Rect r;				/* View area */
	int nFocused;				/* Widgets in focus chain */
	AG_Widget *widExclMotion;		/* Widget exclusively receiving mousemotion */
	AG_TAILQ_HEAD_(ag_cursor_area) cursorAreas; /* Cursor-change areas */
} AG_Window;

AG_TAILQ_HEAD(ag_windowq, ag_window);

__BEGIN_DECLS
extern AG_WidgetClass agWindowClass;
extern struct ag_widgetq agWidgetDetachQ;	/* Widget pre-detach queue */
extern struct ag_windowq agWindowDetachQ;	/* Window detach queue */
extern AG_Window        *agWindowToFocus;	/* Window to focus next */
extern AG_Window        *agWindowFocused;	/* Window holding focus */

void       AG_InitWindowSystem(void);
void       AG_DestroyWindowSystem(void);
AG_Window *AG_WindowNew(Uint);
AG_Window *AG_WindowNewNamedS(Uint, const char *);
AG_Window *AG_WindowNewNamed(Uint, const char *, ...)
			     FORMAT_ATTRIBUTE(printf,2,3);

void	 AG_WindowSetCaptionS(AG_Window *, const char *);
void	 AG_WindowSetCaption(AG_Window *, const char *, ...)
			     FORMAT_ATTRIBUTE(printf,2,3)
			     NONNULL_ATTRIBUTE(2);

void	 AG_WindowUpdateCaption(AG_Window *);
#define  AG_WindowSetIcon(win,su) AG_IconSetSurface((win)->icon,(su))
#define  AG_WindowSetIconNODUP(win,su) AG_IconSetSurfaceNODUP((win)->icon,(su))

void	 AG_WindowSetSpacing(AG_Window *, int);
void	 AG_WindowSetPadding(AG_Window *, int, int, int, int);
#define	 AG_WindowSetPaddingLeft(w,p)   AG_WindowSetPadding((w),(p),-1,-1,-1)
#define	 AG_WindowSetPaddingRight(w,p)  AG_WindowSetPadding((w),-1,(p),-1,-1)
#define	 AG_WindowSetPaddingTop(w,p)    AG_WindowSetPadding((w),-1,-1,(p),-1)
#define	 AG_WindowSetPaddingBottom(w,p) AG_WindowSetPadding((w),-1,-1,-1,(p))
void     AG_WindowSetSideBorders(AG_Window *, int);
void     AG_WindowSetBottomBorder(AG_Window *, int);

void	 AG_WindowSetPosition(AG_Window *, enum ag_window_alignment, int);
void	 AG_WindowSetCloseAction(AG_Window *, enum ag_window_close_action);

void	 AG_WindowSetMinSize(AG_Window *, int, int);
void	 AG_WindowSetMinSizePct(AG_Window *, int);
int	 AG_WindowSetGeometryRect(AG_Window *, AG_Rect, int);
int	 AG_WindowSetGeometryAligned(AG_Window *, enum ag_window_alignment,
                                     int, int);
int	 AG_WindowSetGeometryAlignedPct(AG_Window *, enum ag_window_alignment,
                                        int, int);
#define  AG_WindowSetGeometry(win,x,y,w,h) \
	 AG_WindowSetGeometryRect((win),AG_RECT((x),(y),(w),(h)),0)
#define  AG_WindowSetGeometryBounded(win,x,y,w,h) \
	 AG_WindowSetGeometryRect((win),AG_RECT((x),(y),(w),(h)),1)

void	 AG_WindowSaveGeometry(AG_Window *);
int	 AG_WindowRestoreGeometry(AG_Window *);
void	 AG_WindowMaximize(AG_Window *);
void	 AG_WindowUnmaximize(AG_Window *);
void	 AG_WindowMinimize(AG_Window *);
void	 AG_WindowUnminimize(AG_Window *);

void	 AG_WindowAttach(AG_Window *, AG_Window *);
void	 AG_WindowDetach(AG_Window *, AG_Window *);
void	 AG_WindowShow(AG_Window *);
void	 AG_WindowHide(AG_Window *);
void	 AG_WindowResize(AG_Window *);

void	 AG_WindowFocus(AG_Window *);
int      AG_WindowFocusAtPos(AG_DriverSw *, int, int);
int	 AG_WindowFocusNamed(const char *);
void	 AG_WindowCycleFocus(AG_Window *, int);
#define  AG_WindowFindFocused() agWindowFocused
#define  AG_WindowIsFocused(win) (agWindowFocused == win)
void	 AG_WindowDetachGenEv(AG_Event *);
void	 AG_WindowHideGenEv(AG_Event *);
void	 AG_WindowCloseGenEv(AG_Event *);
void	 AG_FreeDetachedWindows(void);

AG_CursorArea *AG_MapCursor(void *, AG_Rect, struct ag_cursor *);
AG_CursorArea *AG_MapStockCursor(void *, AG_Rect, int);
void           AG_UnmapCursor(void *, AG_CursorArea *);
void           AG_UnmapAllCursors(AG_Window *, void *);

#define AGWINDOW(win)        ((AG_Window *)(win))
#define AGWINDETACH(win)     AG_WindowDetachGenEv, "%p", (win)
#define AGWINHIDE(win)       AG_WindowHideGenEv, "%p", (win)
#define AGWINCLOSE(win)      AG_WindowCloseGenEv, "%p", (win)

/*
 * Render a window to the display (must be enclosed between calls to
 * AG_BeginRendering() and AG_EndRendering()).
 * The View VFS and Window object must be locked.
 */
static __inline__ void
AG_WindowDraw(AG_Window *win)
{
	AG_Driver *drv = AGWIDGET(win)->drv;

	if (!win->visible) {
		return;
	}
	AGDRIVER_CLASS(drv)->renderWindow(win);
	win->dirty = 0;
}

/*
 * Return the effective focus state of a widget.
 * The Widget and View VFS must be locked.
 */
static __inline__ int
AG_WidgetIsFocused(void *p)
{
	AG_Widget *wid = (AG_Widget *)p;

	return ((AGWIDGET(p)->flags & AG_WIDGET_FOCUSED) &&
                (wid->window == NULL || AG_WindowIsFocused(wid->window)));
}

/*
 * Recompute the coordinates and geometries of all widgets attached to the
 * window. This is used following AG_ObjectAttach() and AG_ObjectDetach()
 * calls made in event context, or direct modifications to the x,y,w,h
 * fields of the Widget structure.
 *
 * The View VFS and Window must be locked.
 */
static __inline__ void
AG_WindowUpdate(AG_Window *win)
{
	AG_SizeAlloc a;
	
	if (win == NULL) {
		return;
	}
	if (AGWIDGET(win)->x != -1 && AGWIDGET(win)->y != -1) {
		a.x = AGWIDGET(win)->x;
		a.y = AGWIDGET(win)->y;
		a.w = AGWIDGET(win)->w;
		a.h = AGWIDGET(win)->h;
		AG_WidgetSizeAlloc(win, &a);
	}
	AG_WidgetUpdateCoords(win, AGWIDGET(win)->x, AGWIDGET(win)->y);
}

/*
 * Return visibility status of window.
 * The View VFS and Window object must be locked.
 */
static __inline__ int
AG_WindowIsVisible(AG_Window *win)
{
	return (win->visible);
}

/*
 * Test whether a window is currently selected for a given WM operation.
 * The View VFS must be locked.
 */
static __inline__ int
AG_WindowSelectedWM(AG_Window *win, enum ag_wm_operation op)
{
	AG_Driver *drv = AGWIDGET(win)->drv;

	return (AGDRIVER_SINGLE(drv) &&
	        AGDRIVER_SW(drv)->winSelected == win &&
	        AGDRIVER_SW(drv)->winop == op);
}

/*
 * Return a pointer to a widget's parent window.
 * The View VFS must be locked.
 */
static __inline__ AG_Window *
AG_ParentWindow(void *obj)
{
	return (AGWIDGET(obj)->window);
}

/* Set an explicit widget position in pixels. */
static __inline__ void
AG_WidgetSetPosition(void *wid, int x, int y)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->x = x;
	AGWIDGET(wid)->y = y;
	AG_WidgetUpdate(wid);
	AG_ObjectUnlock(wid);
}

/* Set an explicit widget geometry in pixels. */
static __inline__ void
AG_WidgetSetSize(void *wid, int w, int h)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->w = w;
	AGWIDGET(wid)->h = h;
	AG_WidgetUpdate(wid);
	AG_ObjectUnlock(wid);
}

/* Set an explicit widget geometry from an AG_Rect argument. */
static __inline__ void
AG_WidgetSetGeometry(void *wid, AG_Rect r)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->x = r.x;
	AGWIDGET(wid)->y = r.y;
	AGWIDGET(wid)->w = r.w;
	AGWIDGET(wid)->h = r.h;
	AG_WidgetUpdate(wid);
	AG_ObjectUnlock(wid);
}

static __inline__ void
AG_WindowSetGeometryMax(AG_Window *win)
{
	Uint wMax, hMax;

	AG_GetDisplaySize((void *)AGWIDGET(win)->drv, &wMax, &hMax);
	AG_WindowSetGeometry(win, 0, 0, wMax, hMax);
}

#ifdef AG_LEGACY
void	   AG_WindowSetVisibility(AG_Window *, int) DEPRECATED_ATTRIBUTE;
AG_Window *AG_FindWindow(const char *) DEPRECATED_ATTRIBUTE;
void       AG_ViewAttach(AG_Window *) DEPRECATED_ATTRIBUTE;
void       AG_ViewDetach(AG_Window *) DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */

/* Request widget redraw. */
static __inline__ void
AG_Redraw(void *obj)
{
	if (!agRenderingContext && AGWIDGET(obj)->window != NULL)
		AGWIDGET(obj)->window->dirty = 1;
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_WINDOW_H_ */

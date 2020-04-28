/*	Public domain	*/

#ifndef _AGAR_GUI_WINDOW_H_
#define _AGAR_GUI_WINDOW_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

#ifndef AG_WINDOW_CAPTION_MAX
#define AG_WINDOW_CAPTION_MAX (AG_MODEL+64)
#endif

struct ag_titlebar;
struct ag_font;
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

typedef enum ag_window_close_action {
	AG_WINDOW_HIDE,               /* Hide the window */
	AG_WINDOW_DETACH,             /* Detach (destroy) the window */
	AG_WINDOW_IGNORE              /* Ignore the close request */
} AG_WindowCloseAction;

/*
 * Window function (used by the underlying window manager to set decoration,
 * stacking position and other window behavior settings). Maps to EWMH types.
 * SYNC: agWindowWmTypeNames[].
 */
enum ag_window_wm_type {
	AG_WINDOW_WM_NORMAL,		/* Normal, top-level window */
	AG_WINDOW_WM_DESKTOP,		/* Desktop feature (e.g., fullscreen) */
	AG_WINDOW_WM_DOCK,		/* Dock or panel feature */
	AG_WINDOW_WM_TOOLBAR,		/* Toolbar "torn off" from main window */
	AG_WINDOW_WM_MENU,		/* Pinnable menu window */
	AG_WINDOW_WM_UTILITY,		/* Persistent utility window (e.g.,
					   a palette or a toolbox). */
	AG_WINDOW_WM_SPLASH,		/* Introductory splash screen */
	AG_WINDOW_WM_DIALOG,		/* Dialog window */
	AG_WINDOW_WM_DROPDOWN_MENU,	/* Menubar-triggered drop-down menu */
	AG_WINDOW_WM_POPUP_MENU,	/* Contextual popup menu */
	AG_WINDOW_WM_TOOLTIP,		/* Mouse hover triggered tooltip */
	AG_WINDOW_WM_NOTIFICATION,	/* Notification bubble */
	AG_WINDOW_WM_COMBO,		/* Combo-box triggered window */
	AG_WINDOW_WM_DND		/* Draggable object */
};

typedef AG_TAILQ_HEAD(ag_cursor_areaq, ag_cursor_area) AG_CursorAreaQ;

typedef struct ag_window_fade_ctx {
	float inTime, outTime;                /* Total fade time (in s) */
	float inIncr, outIncr;                /* Delta (in opacity units) */
	float opacity;                        /* Current opacity */
	Uint32 _pad;
	AG_Timer timer;                       /* Fade timer */
} AG_WindowFadeCtx;

typedef struct ag_window_pvt {
	AG_TAILQ_ENTRY(ag_window) detach;     /* in agWindowDetachQ */
	AG_TAILQ_ENTRY(ag_window) visibility; /* in agWindow{Show,Hide}Q */
	AG_TAILQ_HEAD_(ag_window) subwins;    /* For AG_WindowAttach() */
	AG_TAILQ_ENTRY(ag_window) swins;      /* in logical parent window */
	AG_WindowFadeCtx *fade;               /* Fadein/fadeout context */
	AG_CursorAreaQ cursorAreas;           /* Cursor-change areas */
	AG_CursorArea *_Nullable caResize[5]; /* Window-resize areas */
} AG_WindowPvt;

/* Window instance */
typedef struct ag_window {
	struct ag_widget wid;		/* AG_Widget -> AG_Window */

	Uint flags;
#define AG_WINDOW_MODAL         0x00000001 /* Application-modal window */
#define AG_WINDOW_MAXIMIZED     0x00000002 /* Window is maximized (read-only) */
#define AG_WINDOW_MINIMIZED     0x00000004 /* Window is minimized (read-only) */
#define AG_WINDOW_KEEPABOVE     0x00000008 /* Keep window above others */
#define AG_WINDOW_KEEPBELOW     0x00000010 /* Keep window below others */
#define AG_WINDOW_DENYFOCUS     0x00000020 /* Prevent focus gain if possible */
#define AG_WINDOW_NOTITLE       0x00000040 /* Disable titlebar */
#define AG_WINDOW_NOBORDERS     0x00000080 /* Disable window borders */
#define AG_WINDOW_NOHRESIZE     0x00000100 /* Disable horizontal resize ctrl */
#define AG_WINDOW_NOVRESIZE     0x00000200 /* Disable vertical resize ctrl */
#define AG_WINDOW_NOCLOSE       0x00000400 /* Disable close button */
#define AG_WINDOW_NOMINIMIZE    0x00000800 /* Disable minimize button */
#define AG_WINDOW_NOMAXIMIZE    0x00001000 /* Disable maximize button */
#define AG_WINDOW_TILING        0x00002000 /* Subject to WM tiling */
#define AG_WINDOW_MINSIZEPCT    0x00004000 /* Min size is in % (read-only) */
#define AG_WINDOW_NOBACKGROUND  0x00008000 /* Don't fill the background */
#define AG_WINDOW_MAIN          0x00010000 /* Break from AG_EventLoop() on close */
#define AG_WINDOW_FOCUSONATTACH 0x00020000 /* Focus on attach (read-only) */
#define AG_WINDOW_HMAXIMIZE     0x00040000 /* Keep maximized horizontally */
#define AG_WINDOW_VMAXIMIZE     0x00080000 /* Keep maximized vertically */
#define AG_WINDOW_NOMOVE        0x00100000 /* Disallow movement of window */
#define AG_WINDOW_UPDATECAPTION 0x00200000 /* Caption text was updated */
#define AG_WINDOW_MODKEYEVENTS  0x00400000 /* Mod key events (LEGACY, unused) */
#define AG_WINDOW_DETACHING     0x00800000 /* Being detached (read-only) */
#define AG_WINDOW_NOCURSORCHG   0x04000000 /* Inhibit any cursor change */
#define AG_WINDOW_FADEIN        0x08000000 /* Fade-in (compositing WMs) */
#define AG_WINDOW_FADEOUT       0x10000000 /* Fade-out (compositing WMs) */
#define AG_WINDOW_USE_TEXT      0x20000000 /* At least one widget has USE_TEXT */

#define AG_WINDOW_NORESIZE     (AG_WINDOW_NOHRESIZE | AG_WINDOW_NOVRESIZE)
#define AG_WINDOW_NOBUTTONS    (AG_WINDOW_NOCLOSE | AG_WINDOW_NOMINIMIZE | \
                                AG_WINDOW_NOMAXIMIZE)
#define AG_WINDOW_PLAIN        (AG_WINDOW_NOTITLE | AG_WINDOW_NOBORDERS)

	char caption[AG_WINDOW_CAPTION_MAX];	/* Window caption */
	int visible;				/* Window is visible */
	int dirty;				/* Window needs redraw */
	enum ag_window_alignment alignment;	/* Initial position */

	struct ag_titlebar *_Nullable tbar;	/* Titlebar (or NULL) */
	struct ag_icon *_Nullable     icon;	/* Window icon (internal WM) */

	Uint32 _pad;
	int wReq, hReq;				/* Requested geometry (px) */
	int wMin, hMin;				/* Minimum geometry (px) */
	int wBorderBot;				/* Bottom border size (px) */
	int wBorderSide;			/* Side border size (px) */
	int wResizeCtrl;			/* Resize controls size (px) */
	AG_Rect r;				/* View area */
	AG_Rect rSaved;				/* Saved geometry */

	int minPct;				/* For MINSIZEPCT */
	int nFocused;				/* Widgets in focus chain */
	AG_Widget *_Nullable widExclMotion;	/* Hog all mousemotion events */
	enum ag_window_wm_type wmType;		/* Window function */
	int zoom;				/* Effective zoom level */
	
	struct ag_window *_Nullable parent;	  /* Logical parent window */
	struct ag_window *_Nullable transientFor; /* Transient parent window */
	struct ag_window *_Nullable pinnedTo;	  /* Pinned to parent window */

	AG_TAILQ_ENTRY(ag_window) user;		/* In user list */

	AG_WindowPvt pvt;			/* Private data */
} AG_Window;

typedef AG_TAILQ_HEAD(ag_windowq, ag_window) AG_WindowQ;
typedef AG_VEC_HEAD(AG_Window *) AG_WindowVec;

__BEGIN_DECLS
extern const char *_Nonnull agWindowWmTypeNames[];
extern const char *_Nonnull agWindowAlignmentNames[];
extern AG_WidgetClass agWindowClass;

/* Protected by agDrivers VFS lock */
extern AG_WindowQ agWindowDetachQ;		/* AG_ObjectDetach() queue */
extern AG_WindowQ agWindowShowQ;		/* AG_WindowShow() queue */
extern AG_WindowQ agWindowHideQ;		/* AG_WindowHide() queue */
extern AG_Window *_Nullable agWindowToFocus;	/* Window to focus next */
extern AG_Window *_Nullable agWindowFocused;	/* Window holding focus */
extern Uint agWindowPinnedCount;                /* Number of pinned windows */

#if defined(AG_DEBUG) && defined(AG_WIDGETS)
extern AG_Window *_Nullable agDebuggerTgtWindow;     /* For GUI debugger */
#endif
#if defined(AG_WIDGETS)
extern AG_Window *_Nullable agStyleEditorTgtWindow;  /* For Style Editor */
#endif

AG_Window *_Nullable AG_WindowFind(const char *_Nonnull)
                                  _Pure_Attribute
	                          _Warn_Unused_Result;

AG_Window *_Nullable AG_WindowNew(Uint);
AG_Window *_Nullable AG_WindowNewUnder(void *_Nonnull, Uint);

AG_Window *_Nullable AG_WindowNewNamedS(Uint, const char *_Nonnull);
AG_Window *_Nullable AG_WindowNewNamed(Uint, const char *_Nonnull, ...)
			              FORMAT_ATTRIBUTE(printf,2,3);

void AG_WindowSetCaption(AG_Window *_Nonnull, const char *_Nonnull, ...)
                        FORMAT_ATTRIBUTE(printf,2,3);
void AG_WindowSetCaptionS(AG_Window *_Nonnull, const char *_Nonnull);
void AG_WindowSetIcon(AG_Window *_Nonnull, const AG_Surface *_Nonnull);

void AG_WindowSetSideBorders(AG_Window *_Nonnull, int);
void AG_WindowSetBottomBorder(AG_Window *_Nonnull, int);

void AG_WindowSetPosition(AG_Window *_Nonnull, enum ag_window_alignment, int);
void AG_WindowSetCloseAction(AG_Window *_Nonnull, enum ag_window_close_action);

int  AG_WindowMove(AG_Window *, int, int);

void AG_WindowSetMinSize(AG_Window *_Nonnull, int, int);
void AG_WindowSetMinSizePct(AG_Window *_Nonnull, int);

int  AG_WindowSetGeometry(AG_Window *_Nonnull, int,int, int,int);
int  AG_WindowSetGeometryRect(AG_Window *_Nonnull, const AG_Rect *_Nonnull, int);
int  AG_WindowSetGeometryAligned(AG_Window *_Nonnull, enum ag_window_alignment, int, int);
int  AG_WindowSetGeometryAlignedPct(AG_Window *_Nonnull, enum ag_window_alignment,
                                    int,int);
void AG_WindowComputeAlignment(AG_Window *_Nonnull, AG_SizeAlloc *_Nonnull);

int  AG_WindowSetOpacity(AG_Window *_Nonnull, float);
void AG_WindowSetFadeIn(AG_Window *_Nonnull, float, float);
void AG_WindowSetFadeOut(AG_Window *_Nonnull, float, float);
void AG_WindowSetZoom(AG_Window *_Nonnull, int);

void AG_WindowSaveGeometry(AG_Window *_Nonnull);
int  AG_WindowRestoreGeometry(AG_Window *_Nonnull);
void AG_WindowMaximize(AG_Window *_Nonnull);
void AG_WindowUnmaximize(AG_Window *_Nonnull);
void AG_WindowMinimize(AG_Window *_Nonnull);
void AG_WindowUnminimize(AG_Window *_Nonnull);

void AG_WindowAttach(AG_Window *_Nullable, AG_Window *_Nonnull);
void AG_WindowDetach(AG_Window *_Nullable, AG_Window *_Nonnull);
void AG_WindowMakeTransient(AG_Window *_Nullable, AG_Window *_Nonnull);
void AG_WindowPin(AG_Window *_Nonnull, AG_Window *_Nonnull);
void AG_WindowUnpin(AG_Window *_Nonnull);
void AG_WindowMovePinned(AG_Window *_Nonnull, int, int);
void AG_WindowLower(AG_Window *_Nonnull);
void AG_WindowRaise(AG_Window *_Nonnull);
void AG_WindowShow(AG_Window *_Nonnull);
void AG_WindowHide(AG_Window *_Nonnull);
void AG_WindowDrawQueued(void);
void AG_WindowResize(AG_Window *_Nonnull);

AG_Window *_Nullable AG_WindowFindFocused(void)
                                         _Pure_Attribute
                                         _Warn_Unused_Result;

int AG_WindowIsFocused(AG_Window *_Nonnull)
                      _Pure_Attribute
                      _Warn_Unused_Result;

void AG_WindowFocus(AG_Window *_Nonnull);
int  AG_WindowFocusNamed(const char *_Nonnull);
int  AG_WindowFocusAtPos(AG_DriverSw *_Nonnull, int,int);
void AG_WindowCycleFocus(AG_Window *_Nonnull, int);

void AG_WindowDetachGenEv(AG_Event *_Nonnull);
void AG_WindowHideGenEv(AG_Event *_Nonnull);
void AG_WindowCloseGenEv(AG_Event *_Nonnull);

void AG_CloseFocusedWindow(void);

void AG_WindowProcessFocusChange(void);
void AG_WindowProcessShowQueue(void);
void AG_WindowProcessHideQueue(void);
void AG_WindowProcessDetachQueue(void);

AG_CursorArea *_Nullable AG_MapCursor(void *_Nonnull, const AG_Rect *_Nonnull,
                                      struct ag_cursor *_Nonnull);

AG_CursorArea *_Nullable AG_MapStockCursor(void *_Nonnull,
                                           const AG_Rect *_Nonnull, int);

void AG_UnmapCursor(void *_Nonnull, AG_CursorArea *_Nonnull);
void AG_UnmapAllCursors(AG_Window *_Nonnull, void *_Nullable);

void AG_WindowDraw(AG_Window *_Nonnull);
void AG_WindowUpdate(AG_Window *_Nonnull);
int  AG_WindowIsVisible(AG_Window *_Nonnull) _Pure_Attribute;

AG_Window *_Nullable AG_ParentWindow(void *_Nonnull);

void AG_WidgetSetPosition(void *_Nonnull, int,int);
void AG_WidgetSetSize(void *_Nonnull, int, int);
void AG_WidgetSetGeometry(void *_Nonnull, const AG_Rect *_Nonnull);

void AG_WindowSetGeometryMax(AG_Window *_Nonnull);

void AG_Redraw(void *_Nonnull);

void AG_SetCursor(void *_Nonnull, AG_CursorArea *_Nonnull *_Nullable,
                  const AG_Rect *_Nonnull, struct ag_cursor *_Nonnull);

void AG_SetStockCursor(void *_Nonnull, AG_CursorArea *_Nonnull *_Nullable,
                       const AG_Rect *_Nonnull, int);

void AG_WindowProcessQueued(void);

AG_Window *AG_SettingsWindow(void);

#define AGWINDOW(p)              ((AG_Window *)(p))
#define AGCWINDOW(p)             ((const AG_Window *)(p))
#define AG_WINDOW_SELF()          AGWINDOW( AG_OBJECT(0,"AG_Widget:AG_Window:*") )
#define AG_WINDOW_PTR(n)          AGWINDOW( AG_OBJECT((n),"AG_Widget:AG_Window:*") )
#define AG_WINDOW_NAMED(n)        AGWINDOW( AG_OBJECT_NAMED((n),"AG_Widget:AG_Window:*") )
#define AG_CONST_WINDOW_SELF()   AGCWINDOW( AG_CONST_OBJECT(0,"AG_Widget:AG_Window:*") )
#define AG_CONST_WINDOW_PTR(n)   AGCWINDOW( AG_CONST_OBJECT((n),"AG_Widget:AG_Window:*") )
#define AG_CONST_WINDOW_NAMED(n) AGCWINDOW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Window:*") )

/* Canned fn,args arguments to AG_{Add,Set,Post}Event(3). */
#define AGWINDETACH(win) AG_WindowDetachGenEv, "%p", (win)
#define AGWINHIDE(win)   AG_WindowHideGenEv, "%p", (win)
#define AGWINCLOSE(win)  AG_WindowCloseGenEv, "%p", (win)

/* Iterators over direct descendants widgets. */
#define AG_FOREACH_WINDOW(var, ob) \
	AGOBJECT_FOREACH_CHILD(var, ob, ag_window)

#define AG_FOREACH_WINDOW_REVERSE(var, ob) \
	AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, ag_window)

#ifdef AG_LEGACY
# define AG_WINDOW_CASCADE              AG_WINDOW_TILING
# define AG_FindWindow(name)            AG_WindowFind(name)
# define AG_WindowNewSw(drv,flags)      AG_WindowNewUnder((drv),(flags))
# define AG_WindowSetPaddingLeft(w,p)   AG_WindowSetPadding((w),(p),-1,-1,-1)
# define AG_WindowSetPaddingRight(w,p)  AG_WindowSetPadding((w),-1,(p),-1,-1)
# define AG_WindowSetPaddingTop(w,p)    AG_WindowSetPadding((w),-1,-1,(p),-1)
# define AG_WindowSetPaddingBottom(w,p) AG_WindowSetPadding((w),-1,-1,-1,(p))
 
void AG_WindowSetSpacing(AG_Window *_Nonnull, int) DEPRECATED_ATTRIBUTE;
void AG_WindowSetPadding(AG_Window *_Nonnull, int,int,int,int) DEPRECATED_ATTRIBUTE;
void AG_ViewAttach(AG_Window *_Nonnull) DEPRECATED_ATTRIBUTE;
void AG_ViewDetach(AG_Window *_Nonnull) DEPRECATED_ATTRIBUTE;
void AG_WindowSetVisibility(AG_Window *_Nonnull, int) DEPRECATED_ATTRIBUTE;
int AG_WindowIntersect(AG_DriverSw *_Nonnull, int, int) DEPRECATED_ATTRIBUTE _Pure_Attribute;
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_WINDOW_H_ */

/*	Public domain	*/

#ifndef _AGAR_WIDGET_WINDOW_H_
#define _AGAR_WIDGET_WINDOW_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

#define AG_WINDOW_CAPTION_MAX 512

struct ag_titlebar;
struct ag_icon;

enum ag_window_alignment {
	AG_WINDOW_TL,
	AG_WINDOW_TC,
	AG_WINDOW_TR,
	AG_WINDOW_ML,
	AG_WINDOW_MC,
	AG_WINDOW_MR,
	AG_WINDOW_BL,
	AG_WINDOW_BC,
	AG_WINDOW_BR
};

#define AG_WINDOW_UPPER_LEFT	AG_WINDOW_TL
#define AG_WINDOW_UPPER_CENTER	AG_WINDOW_TC
#define AG_WINDOW_UPPER_RIGHT	AG_WINDOW_TR
#define AG_WINDOW_MIDDLE_LEFT	AG_WINDOW_ML
#define AG_WINDOW_CENTER	AG_WINDOW_MC
#define AG_WINDOW_MIDDLE_RIGHT	AG_WINDOW_MR
#define AG_WINDOW_LOWER_LEFT	AG_WINDOW_BL
#define AG_WINDOW_LOWER_CENTER	AG_WINDOW_BC
#define AG_WINDOW_LOWER_RIGHT	AG_WINDOW_BR

enum ag_window_close_action {
	AG_WINDOW_HIDE,
	AG_WINDOW_DETACH,
	AG_WINDOW_NONE
};

struct ag_widget;

typedef struct ag_window {
	struct ag_widget wid;

	Uint flags;
#define AG_WINDOW_MODAL		0x00001	/* Place in foreground */
#define AG_WINDOW_MAXIMIZED	0x00002	/* Window is maximized */
#define AG_WINDOW_MINIMIZED	0x00004	/* Window is minimized */
#define AG_WINDOW_KEEPABOVE	0x00008	/* Keep window above */
#define AG_WINDOW_KEEPBELOW	0x00010	/* Keep window below */
#define AG_WINDOW_DENYFOCUS	0x00020	/* Widgets cannot gain focus */
#define AG_WINDOW_NOTITLE	0x00040	/* Disable the titlebar */
#define AG_WINDOW_NOBORDERS	0x00080	/* Disable the window borders */
#define AG_WINDOW_NOHRESIZE	0x00100	/* Disable horizontal resize */
#define AG_WINDOW_NOVRESIZE	0x00200	/* Disable vertical resize */
#define AG_WINDOW_NOCLOSE	0x00400	/* Disable close button */
#define AG_WINDOW_NOMINIMIZE	0x00800	/* Disable minimize button */
#define AG_WINDOW_NOMAXIMIZE	0x01000	/* Disable maximize button */
#define AG_WINDOW_NOBACKGROUND	0x08000	/* Don't fill the background */
#define AG_WINDOW_NOUPDATERECT	0x10000	/* Don't update rectangle */
#define AG_WINDOW_FOCUSONATTACH	0x20000	/* Automatic focus on attach */
#define AG_WINDOW_HMAXIMIZE	0x40000	/* Keep maximized horizontally */
#define AG_WINDOW_VMAXIMIZE	0x80000	/* Keep maximized vertically */
#define AG_WINDOW_NORESIZE	(AG_WINDOW_NOHRESIZE|AG_WINDOW_NOVRESIZE)
#define AG_WINDOW_PLAIN		(AG_WINDOW_NOTITLE|AG_WINDOW_NOBORDERS)

	char caption[AG_WINDOW_CAPTION_MAX];	/* Window caption */
	int visible;				/* Window is visible */

	struct ag_titlebar *tbar;		/* Titlebar (if any) */
	enum ag_window_alignment alignment;	/* Initial position */
	int spacing;				/* Default spacing (px) */
	int tPad, bPad, lPad, rPad;		/* Window padding (px) */
	int minw, minh;				/* Minimum geometry (px) */
	int savx, savy;				/* Saved coordinates (px) */
	int savw, savh;				/* Saved geometry (px) */
	
	AG_TAILQ_HEAD(,ag_window) subwins;	/* Sub-windows */
	AG_TAILQ_ENTRY(ag_window) windows;	/* Active window list */
	AG_TAILQ_ENTRY(ag_window) swins;	/* Sub-window list */
	AG_TAILQ_ENTRY(ag_window) detach;	/* Zombie window list */

	struct ag_icon *icon;			/* Window icon */
} AG_Window;

#define AG_WINDOW_FOCUSED(w) (AG_TAILQ_LAST(&agView->windows,ag_windowq)==(w))

__BEGIN_DECLS
extern AG_WidgetClass agWindowClass;
extern int agWindowIconWidth;
extern int agWindowIconHeight;

AG_Window *AG_WindowNew(Uint);
AG_Window *AG_WindowNewNamed(Uint, const char *, ...)
			     FORMAT_ATTRIBUTE(printf, 2, 3);

void	 AG_WindowSetCaption(AG_Window *, const char *, ...)
			     FORMAT_ATTRIBUTE(printf, 2, 3)
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

void	 AG_WindowSetPosition(AG_Window *, enum ag_window_alignment, int);
void	 AG_WindowSetCloseAction(AG_Window *, enum ag_window_close_action);
int	 AG_WindowSetGeometryParam(AG_Window *, int, int, int, int, int);
int	 AG_WindowSetGeometryAligned(AG_Window *, enum ag_window_alignment,
                                     int, int);
int	 AG_WindowSetGeometryAlignedPct(AG_Window *, enum ag_window_alignment,
                                        int, int);
#define  AG_WindowSetGeometry(win,x,y,w,h) \
	 AG_WindowSetGeometryParam((win),(x),(y),(w),(h),0)
#define  AG_WindowSetGeometryBounded(win,x,y,w,h) \
	 AG_WindowSetGeometryParam((win),(x),(y),(w),(h),1)
#define	 AG_WindowSetGeometryMax(win) \
	 AG_WindowSetGeometry((win),0,0,agView->w,agView->h)

void	 AG_WindowUpdate(AG_Window *);
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
int	 AG_WindowToggleVisibility(AG_Window *);
int	 AG_WindowEvent(SDL_Event *);
void	 AG_WindowResize(AG_Window *);
void	 AG_WindowFocus(AG_Window *);
int	 AG_WindowFocusNamed(const char *);
void	 AG_WindowCycleFocus(AG_Window *, int);
int	 AG_WindowIsSurrounded(AG_Window *);
void	 AG_WindowApplyAlignment(AG_Window *, enum ag_window_alignment);

/* Generic event handlers */
void	 AG_WindowDetachGenEv(AG_Event *);
void	 AG_WindowHideGenEv(AG_Event *);
void	 AG_WindowShowGenEv(AG_Event *);
void	 AG_WindowCloseGenEv(AG_Event *);

#define AGWINDETACH(win)	AG_WindowDetachGenEv, "%p", (win)
#define AGWINHIDE(win)		AG_WindowHideGenEv, "%p", (win)
#define AGWINCLOSE(win)		AG_WindowCloseGenEv, "%p", (win)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_WINDOW_H_ */

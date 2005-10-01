/*	$Csoft: window.h,v 1.94 2005/09/27 00:25:25 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_WINDOW_H_
#define _AGAR_WIDGET_WINDOW_H_

#include <engine/widget/widget.h>
#include <engine/widget/titlebar.h>

#include "begin_code.h"

enum ag_window_alignment {
	AG_WINDOW_UPPER_LEFT,
	AG_WINDOW_MIDDLE_LEFT,
	AG_WINDOW_LOWER_LEFT,
	AG_WINDOW_UPPER_RIGHT,
	AG_WINDOW_MIDDLE_RIGHT,
	AG_WINDOW_LOWER_RIGHT,
	AG_WINDOW_CENTER,
	AG_WINDOW_LOWER_CENTER,
	AG_WINDOW_UPPER_CENTER
};

struct ag_widget;

typedef struct ag_window {
	struct ag_widget wid;

	int flags;
#define AG_WINDOW_CASCADE	0x0002	/* Increment position slightly */
#define AG_WINDOW_NO_TITLEBAR	0x0004	/* Disable the titlebar */
#define AG_WINDOW_NO_DECORATIONS 0x0008	/* Disable the window borders */
#define AG_WINDOW_HIDE		0x0010	/* Hide on window-close */
#define AG_WINDOW_DETACH	0x0020	/* Detach on window-close */
#define AG_WINDOW_MAXIMIZED	0x0040	/* Window is maximized */
#define AG_WINDOW_ICONIFIED	0x0080	/* Window is minimized */
#define AG_WINDOW_NO_HRESIZE	0x0100	/* Disable horizontal resize */
#define AG_WINDOW_NO_VRESIZE	0x0200	/* Disable vertical resize */
#define AG_WINDOW_NO_CLOSE	0x0400	/* Disable close button */
#define AG_WINDOW_NO_MINIMIZE	0x0800	/* Disable minimize button */
#define AG_WINDOW_NO_MAXIMIZE	0x1000	/* Disable maximize button */
#define AG_WINDOW_MODAL		0x2000	/* Modal window behavior */
#define AG_WINDOW_INHIBIT_FOCUS	0x4000	/* Widgets cannot gain focus */
#define AG_WINDOW_NO_BACKGROUND 0x8000	/* Don't fill the background */
#define AG_WINDOW_NO_RESIZE	(AG_WINDOW_NO_HRESIZE|AG_WINDOW_NO_VRESIZE)

	char caption[128];
	int visible;				/* Window is visible */

	pthread_mutex_t lock;
	AG_Titlebar *tbar;			/* Titlebar (if any) */
	enum ag_window_alignment alignment;	/* Initial position */
	int spacing;				/* Default spacing (px) */
	int xpadding;				/* Window padding (px) */
	int ypadding_top, ypadding_bot;
	int minw, minh;				/* Minimum geometry (px) */
	int savx, savy;				/* Saved coordinates (px) */
	int savw, savh;				/* Saved geometry (px) */
	
	TAILQ_HEAD(,ag_window) subwins;		/* Sub-windows */
	TAILQ_ENTRY(ag_window) windows;		/* Active window list */
	TAILQ_ENTRY(ag_window) swins;		/* Sub-window list */
	TAILQ_ENTRY(ag_window) detach;		/* Zombie window list */
} AG_Window;

#define AG_WINDOW_FOCUSED(w) (TAILQ_LAST(&agView->windows, ag_windowq) == (w))
#define AG_WINDOW_UPDATE(win) \
	do { \
		AGWIDGET_OPS(win)->scale((win), AGWIDGET(win)->w, \
		    AGWIDGET(win)->h); \
		AG_WidgetUpdateCoords((win), AGWIDGET(win)->x, \
		    AGWIDGET(win)->y); \
	} while (/*CONSTCOND*/0)

__BEGIN_DECLS
AG_Window *AG_WindowNew(int, const char *, ...)
			FORMAT_ATTRIBUTE(printf, 2, 3);

void	 AG_WindowInit(void *, const char *, int);
void	 AG_WindowDestroy(void *);
void	 AG_WindowDraw(void *);
void	 AG_WindowScale(void *, int, int);

void	 AG_WindowSetCaption(AG_Window *, const char *, ...)
			     FORMAT_ATTRIBUTE(printf, 2, 3)
			     NONNULL_ATTRIBUTE(2);
void	 AG_WindowSetSpacing(AG_Window *, int);
void	 AG_WindowSetPadding(AG_Window *, int, int, int);
void	 AG_WindowSetPosition(AG_Window *, enum ag_window_alignment, int);
void	 AG_WindowSetCloseAction(AG_Window *, int);
void	 AG_WindowSetStyle(AG_Window *, const AG_WidgetStyleMod *);
void	 AG_WindowSetGeometry(AG_Window *, int, int, int, int);

void	 AG_WindowAttach(AG_Window *, AG_Window *);
void	 AG_WindowDetach(AG_Window *, AG_Window *);
void	 AG_WindowShow(AG_Window *);
void	 AG_WindowHide(AG_Window *);
int	 AG_WindowToggleVisibility(AG_Window *);
int	 AG_WindowEvent(SDL_Event *);
void	 AG_WindowResize(AG_Window *);
void	 AG_WindowFocus(AG_Window *);
void	 AG_WindowCycleFocus(AG_Window *, int);
void	 AG_WindowClamp(AG_Window *);
int	 AG_WindowIsSurrounded(AG_Window *);
void	 AG_WindowApplyAlignment(AG_Window *, enum ag_window_alignment);

/* Generic event handlers */
void	 AG_WindowDetachGenEv(int, union evarg *);
void	 AG_WindowHideGenEv(int, union evarg *);
void	 AG_WindowShowGenEv(int, union evarg *);
void	 AG_WindowCloseGenEv(int, union evarg *);

#define AGWINDETACH(win)	AG_WindowDetachGenEv, "%p", (win)
#define AGWINHIDE(win)		AG_WindowHideGenEv, "%p", (win)
#define AGWINCLOSE(win)		AG_WindowCloseGenEv, "%p", (win)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_WINDOW_H_ */

/*	Public domain	*/

#ifndef _AGAR_MICRO_WINDOW_H_
#define _AGAR_MICRO_WINDOW_H_

#include <agar/micro/widget.h>
#include <agar/micro/begin.h>

#ifndef MA_WINDOW_CAPTION_MAX
#define MA_WINDOW_CAPTION_MAX 16
#endif

struct ma_widget;

#define MA_WINDOW_UPPER_LEFT	MA_WINDOW_TL
#define MA_WINDOW_UPPER_CENTER	MA_WINDOW_TC
#define MA_WINDOW_UPPER_RIGHT	MA_WINDOW_TR
#define MA_WINDOW_MIDDLE_LEFT	MA_WINDOW_ML
#define MA_WINDOW_CENTER	MA_WINDOW_MC
#define MA_WINDOW_MIDDLE_RIGHT	MA_WINDOW_MR
#define MA_WINDOW_LOWER_LEFT	MA_WINDOW_BL
#define MA_WINDOW_LOWER_CENTER	MA_WINDOW_BC
#define MA_WINDOW_LOWER_RIGHT	MA_WINDOW_BR

/* Micro-Agar Window instance */
typedef struct ma_window {
	struct ag_object wid;              /* AG_Object -> MA_Window */

	Uint16 flags;
#define MA_WINDOW_MODAL		0x0001 /* Application-modal window */
#define MA_WINDOW_MAXIMIZED	0x0002 /* Window is maximized (read-only) */
#define MA_WINDOW_MINIMIZED	0x0004 /* Window is minimized (read-only) */
#define MA_WINDOW_KEEPABOVE	0x0008 /* Keep window above others */
#define MA_WINDOW_KEEPBELOW	0x0010 /* Keep window below others */
#define MA_WINDOW_DENYFOCUS	0x0020 /* Prevent focus gain if possible */
#define MA_WINDOW_NOTITLE	0x0040 /* Disable titlebar */
#define MA_WINDOW_NOBORDERS	0x0080 /* Disable window borders */
#define MA_WINDOW_NORESIZE	0x0100 /* Not resizable */
#define MA_WINDOW_NOCLOSE	0x0200 /* Not button-closable */
#define MA_WINDOW_NOBACKGROUND	0x0400 /* No BG fill */
#define MA_WINDOW_NOMINIMIZE	0x0800 /* Not minimizable */
#define MA_WINDOW_NOMAXIMIZE	0x1000 /* Not maximizable */
#define MA_WINDOW_NOMOVE	0x2000 /* Not movable */
#define MA_WINDOW_DETACHING	0x4000 /* Being detached (read-only) */
#define MA_WINDOW_REDRAW	0x8000 /* Request redraw and video update */
#define MA_WINDOW_PLAIN		(MA_WINDOW_NOTITLE | MA_WINDOW_NOBORDERS)
#define MA_WINDOW_NOBUTTONS	(MA_WINDOW_NOCLOSE | MA_WINDOW_NOMINIMIZE | \
				 MA_WINDOW_NOMAXIMIZE)

	char caption[MA_WINDOW_CAPTION_MAX];	/* Caption text */

	Uint8 visible;				/* Visibility flag */

	Uint8  spacing;				/* Widget spacing (px) */
	Uint16 padding;				/* Padding around widgets (px) */
#define MA_WINDOW_PAD_TOP(x)    (((x) & 0xf000) >> 12)
#define MA_WINDOW_PAD_BOTTOM(x) (((x) & 0x0f00) >> 8)
#define MA_WINDOW_PAD_LEFT(x)   (((x) & 0x00f0) >> 4)
#define MA_WINDOW_PAD_RIGHT(x)   ((x) & 0x000f)

	Uint8 min;				/* Minimum geometry (px/2) */
#define MA_WINDOW_MIN_W(x) ((x) & 0xf0) >> 4
#define MA_WINDOW_MIN_H(x) ((x) & 0x0f)

	Sint16 x, y;				/* Display position (px) or -1 */
	Uint16 w, h;				/* Geometry (px) */

	struct ma_window *_Nullable parent;	/* Logical parent window */

	AG_TAILQ_ENTRY(ma_window) detach;	/* In agWindowDetachQ */
	AG_TAILQ_ENTRY(ma_window) visibility;	/* In agWindow{Show,Hide}Q */
	AG_TAILQ_HEAD_(ma_window) subwins;	/* Logical sub-windows */
	AG_TAILQ_ENTRY(ma_window) swins;	/* Entry in logical parent */
} MA_Window;

typedef AG_TAILQ_HEAD(ma_windowq, ma_window) MA_WindowQ;
typedef AG_VEC_HEAD(MA_Window *) MA_WindowVec;

__BEGIN_DECLS
extern AG_ObjectClass maWindowClass;

extern MA_WindowQ maWindowDetachQ;		/* AG_ObjectDetach() queue */
extern MA_WindowQ maWindowShowQ;		/* MA_WindowShow() queue */
extern MA_WindowQ maWindowHideQ;		/* MA_WindowHide() queue */
extern MA_Window *_Nullable maWindowToFocus;	/* Window to focus next */
extern MA_Window *_Nullable maWindowFocused;	/* Window holding focus */

void MA_InitWindowSystem(void);

MA_Window *_Nullable MA_WindowNew(Uint16);

Sint8 MA_WindowSetGeometry(MA_Window *_Nonnull, int,int, int,int);
void  MA_WindowMaximize(MA_Window *_Nonnull);
void  MA_WindowUnmaximize(MA_Window *_Nonnull);
void  MA_WindowMinimize(MA_Window *_Nonnull);
void  MA_WindowUnminimize(MA_Window *_Nonnull);

void MA_WindowAttach(MA_Window *_Nullable, MA_Window *_Nonnull);
void MA_WindowDetach(MA_Window *_Nullable, MA_Window *_Nonnull);
void MA_WindowLower(MA_Window *_Nonnull);
void MA_WindowRaise(MA_Window *_Nonnull);
void MA_WindowShow(MA_Window *_Nonnull);
void MA_WindowHide(MA_Window *_Nonnull);
void MA_WindowDrawQueued(void);
void MA_WindowResize(MA_Window *_Nonnull);

void MA_WindowFocus(MA_Window *_Nonnull);
void MA_WindowCycleFocus(MA_Window *_Nonnull, int);

void MA_WindowDetachGenEv(AG_Event *_Nonnull);
void MA_WindowHideGenEv(AG_Event *_Nonnull);
void MA_WindowCloseGenEv(AG_Event *_Nonnull);

void AG_CloseFocusedWindow(void);

void MA_WindowProcessShowQueue(void);
void MA_WindowProcessHideQueue(void);
void MA_WindowProcessDetachQueue(void);

void MA_WindowDraw(MA_Window *_Nonnull);
void MA_WindowUpdate(MA_Window *_Nonnull);

void MA_WindowProcessQueued(void);

#define MAWINDOW(p)              ((MA_Window *)(p))
#define MACWINDOW(p)             ((const MA_Window *)(p))
#define MA_WINDOW_SELF()          MAWINDOW( AG_OBJECT(0,"MA_Window:*") )
#define MA_WINDOW_PTR(n)          MAWINDOW( AG_OBJECT((n),"MA_Window:*") )
#define MA_WINDOW_NAMED(n)        MAWINDOW( AG_OBJECT_NAMED((n),"MA_Window:*") )
#define MA_CONST_WINDOW_SELF()   MACWINDOW( AG_CONST_OBJECT(0,"MA_Window:*") )
#define MA_CONST_WINDOW_PTR(n)   MACWINDOW( AG_CONST_OBJECT((n),"MA_Window:*") )
#define MA_CONST_WINDOW_NAMED(n) MACWINDOW( AG_CONST_OBJECT_NAMED((n),"MA_Window:*") )

/* Canned fn,args arguments to AG_{Add,Set,Post}Event(3). */
#define MAWINDETACH(win) MA_WindowDetachGenEv, "%p", (win)
#define MAWINHIDE(win)   MA_WindowHideGenEv, "%p", (win)
#define MAWINCLOSE(win)  MA_WindowCloseGenEv, "%p", (win)

/* Iterators over direct descendants widgets. */
#define MA_FOREACH_WINDOW(var, ob) \
	AGOBJECT_FOREACH_CHILD(var, ob, ma_window)

#define MA_FOREACH_WINDOW_REVERSE(var, ob) \
	AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, ma_window)
__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_WINDOW_H_ */

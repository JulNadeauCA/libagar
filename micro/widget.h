/*	Public domain	*/

#ifndef _AGAR_MICRO_WIDGET_H_
#define _AGAR_MICRO_WIDGET_H_

#include <agar/micro/gui.h>
#include <agar/micro/geometry.h>
#include <agar/micro/colors.h>
#include <agar/micro/surface.h>
#include <agar/micro/drv.h>
#include <agar/micro/begin.h>

struct ma_widget;

/* Widget size requisition and allocation. */
typedef struct ma_size_req {
	Uint8 w, h;			/* Requested geometry in pixels */
} MA_SizeReq;
typedef struct ma_size_alloc {
	Uint8 w, h;			/* Allocated geometry in pixels */
	Uint8 x, y;			/* Allocated position in pixels */
} MA_SizeAlloc;

/* Micro-Agar Widget class description. */
typedef struct ma_widget_class {
	struct ag_object_class _inherit;     /* [AG_Object] -> [MA_Widget] */

	void  (*_Nullable draw)(void *_Nonnull);
	void  (*_Nullable size_request)(void *_Nonnull, MA_SizeReq *_Nonnull);
	Sint8 (*_Nullable size_allocate)(void *_Nonnull,
	                                 const MA_SizeAlloc *_Nonnull);
} MA_WidgetClass;

/* Flag description */
typedef struct ma_flag_descr {
	Uint bitmask;			/* Bitmask */
	const char *_Nonnull descr;	/* Description (UTF-8) */
	Uint8 writeable;		/* Write flag (1=R/W, 0=RO) */
} MA_FlagDescr;

/* Micro-Agar Widget instance */
typedef struct ma_widget {
	struct ag_object obj;		/* AG_Object -> MA_Widget */

	Uint8 flags;
#define MA_WIDGET_FOCUSABLE 0x01	/* Can grab focus */
#define MA_WIDGET_FOCUSED   0x02	/* Holds focus in parent */
#define MA_WIDGET_VISIBLE   0x04	/* Widget is exposed */
#define MA_WIDGET_HFILL     0x08	/* Expand to remaining parent height */
#define MA_WIDGET_VFILL     0x10	/* Expand to remaining parent width */
#define MA_WIDGET_EXPAND (MA_WIDGET_HFILL | MA_WIDGET_VFILL)
#define MA_WIDGET_DISABLED  0x20	/* Disable input */
#define MA_WIDGET_MOUSEOVER 0x40	/* Mouse cursor is over the widget */
#define MA_WIDGET_UNDERSIZE 0x80	/* Widget is too small to draw */

	Uint8 x, y;			/* Position in parent */
	Uint8 w, h;			/* Allocated size */

	Uint8                           nSurfaces;
	MA_Surface *_Nullable *_Nullable surfaces; /* Mapped surfaces */

	struct ma_window *_Nullable window; /* Parent window (or null) */
} MA_Widget;

typedef AG_VEC_HEAD(MA_Widget *) MA_WidgetVec;

#define MAWIDGET(p)             ((MA_Widget *)(p))
#define MACWIDGET(p)            ((const MA_Widget *)(p))
#define MA_WIDGET_SELF()          MAWIDGET( AG_OBJECT(0,"MA_Widget:*") )
#define MA_WIDGET_PTR(n)          MAWIDGET( AG_OBJECT((n),"MA_Widget:*") )
#define MA_WIDGET_NAMED(n)        MAWIDGET( AG_OBJECT_NAMED((n),"MA_Widget:*") )
#define MA_CONST_WIDGET_SELF()   MACWIDGET( AG_CONST_OBJECT(0,"MA_Widget:*") )
#define MA_CONST_WIDGET_PTR(n)   MACWIDGET( AG_CONST_OBJECT((n),"MA_Widget:*") )
#define MA_CONST_WIDGET_NAMED(n) MACWIDGET( AG_CONST_OBJECT((n),"MA_Widget:*") )

#define MAWIDGET_OPS(wi)	((MA_WidgetClass *)AGOBJECT(wi)->cls)
#define MAWIDGET_SUPER_OPS(wi)	((MA_WidgetClass *)AGOBJECT(wi)->cls->super)
#define MAWIDGET_SURFACE(wi,idx) MAWIDGET(wi)->surfaces[idx]

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MICRO)
# define WIDGET(wi)		MAWIDGET(wi)
# define WIDGET_OPS(wi)		MAWIDGET_OPS(wi)
# define WIDGET_SUPER_OPS(wi)	MAWIDGET_SUPER_OPS(wi)
# define WSURFACE(wi,ind)	MAWIDGET_SURFACE((wi),(ind))
# define WIDTH(p)		MAWIDGET(p)->w
# define HEIGHT(p)		MAWIDGET(p)->h
#endif

struct ma_window;
AG_TAILQ_HEAD(ma_widgetq, ma_widget);

__BEGIN_DECLS
extern MA_WidgetClass maWidgetClass;

void MA_WidgetSizeReq(void *_Nonnull, MA_SizeReq *_Nonnull);
void MA_WidgetSizeAlloc(void *_Nonnull, MA_SizeAlloc *_Nonnull);
int  MA_WidgetSetFocusable(void *_Nonnull, int);
int  MA_WidgetFocus(void *_Nonnull);
void MA_WidgetUnfocus(void *_Nonnull);

Uint8   MA_WidgetMapSurface(void *_Nonnull, MA_Surface *_Nullable);
void    MA_WidgetReplaceSurface(void *_Nonnull, Uint8, MA_Surface *_Nullable);
#define MA_WidgetUnmapSurface(o,n) MA_WidgetReplaceSurface((o),(n),NULL)
#define MA_WidgetBlitSurface(o,n,x,y) MA_WidgetBlitFrom((o),(n),NULL,(x),(y))

void MA_WidgetShow(void *_Nonnull);
void MA_WidgetHide(void *_Nonnull);
void MA_WidgetShowAll(void *_Nonnull);
void MA_WidgetHideAll(void *_Nonnull);

void MA_WidgetInheritDraw(void *_Nonnull);
void MA_WidgetInheritSizeRequest(void *_Nonnull, MA_SizeReq *_Nonnull);
int  MA_WidgetInheritSizeAllocate(void *_Nonnull, const MA_SizeAlloc *_Nonnull);

void MA_WidgetEnable(void *_Nonnull);
void MA_WidgetDisable(void *_Nonnull);

Uint8 MA_WidgetIsFocused(const void *_Nonnull) _Pure_Attribute;

Uint8 MA_WidgetArea(const void *_Nonnull, Sint16,Sint16) _Pure_Attribute;
void  MA_WidgetBlit(void *_Nonnull, MA_Surface *_Nonnull, Uint8,Uint8);
void  MA_WidgetBlitFrom(void *_Nonnull, Uint8, MA_Rect *_Nullable, Uint8,Uint8);

#define MA_WidgetEnabled(o)           !(MAWIDGET(o)->flags & MA_WIDGET_DISABLED)
#define MA_WidgetDisabled(o)           (MAWIDGET(o)->flags & MA_WIDGET_DISABLED)
#define MA_WidgetVisible(o)            (MAWIDGET(o)->flags & MA_WIDGET_VISIBLE)
#define MA_WidgetIsFocusedInWindow(o)  (MAWIDGET(o)->flags & MA_WIDGET_FOCUSED)
__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_WIDGET_H_ */

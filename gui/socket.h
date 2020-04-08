/*	Public domain	*/

#ifndef _AGAR_WIDGET_SOCKET_H_
#define _AGAR_WIDGET_SOCKET_H_

#include <agar/gui/widget.h>
#include <agar/gui/text.h>
#include <agar/gui/begin.h>

struct ag_icon;

enum ag_socket_bg_type {
	AG_SOCKET_PIXMAP,	/* Pixmap background */
	AG_SOCKET_RECT,		/* Generic rectangular background */
	AG_SOCKET_CIRCLE	/* Generic circular background */
};

typedef struct ag_socket {
	struct ag_widget wid;		/* AG_Widget -> AG_Socket */
	int state;			/* Default boolean state binding */
	int count;			/* Default stack count binding */
	Uint flags;
#define AG_SOCKET_HFILL		0x01	/* Fill available width */
#define AG_SOCKET_VFILL		0x02	/* Fill available height */
#define AG_SOCKET_EXPAND	(AG_SOCKET_HFILL|AG_SOCKET_VFILL)
#define AG_SOCKET_MOUSEOVER	0x04
#define AG_SOCKET_STICKY_STATE	0x08
	enum ag_socket_bg_type bgType;
	union {
		struct {
			int s;		/* Pixmap surface handle */
		} pixmap;
		struct {
			int w, h;	/* Dimensions */
		} rect;
		struct {
			int r;		/* Radius */
		} circle;
	} bgData;
	enum ag_text_justify lblJustify; /* Label justification */
	Uint32 _pad;
	struct ag_icon *_Nullable icon;	 /* Icon in socket if any */

	/* Icon inserted callback */
	int  (*_Nullable insertFn)(struct ag_socket *_Nonnull,
	                           struct ag_icon   *_Nonnull);
	/* Icon removed callback */
	void (*_Nullable removeFn)(struct ag_socket *_Nonnull,
	                           struct ag_icon   *_Nonnull);

	AG_Event *_Nullable overlayFn;	/* Rendering overlay callback */
} AG_Socket;

#define AGSOCKET(obj)            ((AG_Socket *)(obj))
#define AGCSOCKET(obj)           ((const AG_Socket *)(obj))
#define AG_SOCKET_SELF()          AGSOCKET( AG_OBJECT(0,"AG_Widget:AG_Socket:*") )
#define AG_SOCKET_PTR(n)          AGSOCKET( AG_OBJECT((n),"AG_Widget:AG_Socket:*") )
#define AG_SOCKET_NAMED(n)        AGSOCKET( AG_OBJECT_NAMED((n),"AG_Widget:AG_Socket:*") )
#define AG_CONST_SOCKET_SELF()   AGCSOCKET( AG_CONST_OBJECT(0,"AG_Widget:AG_Socket:*") )
#define AG_CONST_SOCKET_PTR(n)   AGCSOCKET( AG_CONST_OBJECT((n),"AG_Widget:AG_Socket:*") )
#define AG_CONST_SOCKET_NAMED(n) AGCSOCKET( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Socket:*") )

__BEGIN_DECLS
extern AG_WidgetClass agSocketClass;

AG_Socket *_Nonnull AG_SocketNew(void *_Nullable, Uint);
AG_Socket *_Nonnull AG_SocketFromBMP(void *_Nullable, Uint, const char *_Nonnull);
AG_Socket *_Nonnull AG_SocketFromSurface(void *_Nullable, Uint,
                                         AG_Surface *_Nullable);

void AG_SocketInsertFn(AG_Socket *_Nonnull,
                       int (*_Nullable)(AG_Socket *_Nonnull, struct ag_icon *_Nonnull));
void AG_SocketRemoveFn(AG_Socket *_Nonnull,
                       void (*_Nullable)(AG_Socket *_Nonnull, struct ag_icon *_Nonnull));
void AG_SocketOverlayFn(AG_Socket *_Nonnull,
                        _Nullable AG_EventFn, const char *_Nullable, ...);

void AG_SocketBgRect(AG_Socket *_Nonnull, Uint,Uint);
void AG_SocketBgCircle(AG_Socket *_Nonnull, Uint);
void AG_SocketBgPixmap(AG_Socket *_Nonnull, const AG_Surface *_Nullable);
void AG_SocketBgPixmapNODUP(AG_Socket *_Nonnull, AG_Surface *_Nullable);

void AG_SocketInsertIcon(AG_Socket *_Nonnull, struct ag_icon *_Nonnull);
void AG_SocketRemoveIcon(AG_Socket *_Nonnull);

#ifdef AG_LEGACY
void    AG_SocketSetPadding(AG_Socket *_Nonnull, int,int,int,int)
                           DEPRECATED_ATTRIBUTE;
#define	AG_SocketSetPaddingLeft(b,v)   AG_SetStyleF((b),"padding","0 0 0 %d",(v))
#define	AG_SocketSetPaddingRight(b,v)  AG_SetStyleF((b),"padding","0 %d 0 0",(v))
#define AG_SocketSetPaddingTop(b,v)    AG_SetStyleF((b),"padding","%d 0 0 0",(v))
#define	AG_SocketSetPaddingBottom(b,v) AG_SetStyleF((b),"padding","0 0 %d 0",(v))
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_SOCKET_H_ */

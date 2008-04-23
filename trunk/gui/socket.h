/*	Public domain	*/

#ifndef _AGAR_WIDGET_SOCKET_H_
#define _AGAR_WIDGET_SOCKET_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/text.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/text.h>
#endif

#include "begin_code.h"

struct ag_icon;

enum ag_socket_bg_type {
	AG_SOCKET_PIXMAP,	/* Pixmap background */
	AG_SOCKET_RECT,		/* Generic rectangular background */
	AG_SOCKET_CIRCLE	/* Generic circular background */
};

typedef struct ag_socket {
	struct ag_widget wid;
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
	int lPad, rPad, tPad, bPad;	/* Padding around contained Icon */
	struct ag_icon *icon;		/* Icon in socket or NULL */

	/* Callbacks */
	int (*insertFn)(struct ag_socket *, struct ag_icon *);
	void (*removeFn)(struct ag_socket *, struct ag_icon *);
	AG_Event *overlayFn;
} AG_Socket;

__BEGIN_DECLS
extern AG_WidgetClass agSocketClass;

AG_Socket *AG_SocketNew(void *, Uint);
AG_Socket *AG_SocketFromBMP(void *, Uint, const char *);
AG_Socket *AG_SocketFromSurface(void *, Uint, AG_Surface *);
void	   AG_SocketInsertFn(AG_Socket *,
	                     int (*)(AG_Socket *, struct ag_icon *));
void	   AG_SocketRemoveFn(AG_Socket *,
	                     void (*)(AG_Socket *, struct ag_icon *));
void       AG_SocketOverlayFn(AG_Socket *, AG_EventFn, const char *, ...);

void	   AG_SocketSetPadding(AG_Socket *, int, int, int, int);
#define	AG_SocketSetPaddingLeft(b,v)   AG_SocketSetPadding((b),(v),-1,-1,-1)
#define	AG_SocketSetPaddingRight(b,v)  AG_SocketSetPadding((b),-1,(v),-1,-1)
#define AG_SocketSetPaddingTop(b,v)    AG_SocketSetPadding((b),-1,-1,(v),-1)
#define	AG_SocketSetPaddingBottom(b,v) AG_SocketSetPadding((b),-1,-1,-1,(v))

void AG_SocketBgRect(AG_Socket *, Uint, Uint);
void AG_SocketBgCircle(AG_Socket *, Uint);
void AG_SocketBgPixmap(AG_Socket *, AG_Surface *);
void AG_SocketBgPixmapNODUP(AG_Socket *, AG_Surface *);

void AG_SocketInsertIcon(AG_Socket *, struct ag_icon *);
void AG_SocketRemoveIcon(AG_Socket *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SOCKET_H_ */

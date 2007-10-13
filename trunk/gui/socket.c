/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "socket.h"
#include "icon.h"

#include "window.h"
#include "primitive.h"
#include "label.h"
#include "pixmap.h"

#include <stdarg.h>

static void SetState(AG_WidgetBinding *, void *, int);
static void MouseMotion(AG_Event *);
static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);

AG_Socket *
AG_SocketNew(void *parent, Uint flags)
{
	AG_Socket *sock;

	sock = Malloc(sizeof(AG_Socket), M_OBJECT);
	AG_SocketInit(sock, flags);
	AG_ObjectAttach(parent, sock);
	return (sock);
}

AG_Socket *
AG_SocketFromSurface(void *parent, Uint flags, SDL_Surface *su)
{
	AG_Socket *sock;
	
	sock = Malloc(sizeof(AG_Socket), M_OBJECT);
	AG_SocketInit(sock, flags);
	AG_ObjectAttach(parent, sock);
	AG_SocketBgPixmap(sock, su);
	return (sock);
}

AG_Socket *
AG_SocketFromBMP(void *parent, Uint flags, const char *bmpfile)
{
	AG_Socket *sock;
	SDL_Surface *bmp;
	
	if ((bmp = SDL_LoadBMP(bmpfile)) == NULL) {
		AG_SetError("%s: %s", bmpfile, SDL_GetError());
		return (NULL);
	}
	sock = Malloc(sizeof(AG_Socket), M_OBJECT);
	AG_SocketInit(sock, flags);
	AG_ObjectAttach(parent, sock);
	AG_SocketBgPixmapNODUP(sock, bmp);
	return (sock);
}

void
AG_SocketInit(AG_Socket *sock, Uint flags)
{
	AG_WidgetInit(sock, &agSocketOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION|
	    AG_WIDGET_UNFOCUSED_BUTTONUP);
	AG_WidgetBindBool(sock, "state", &sock->state);
	AG_WidgetBindInt(sock, "count", &sock->count);
	sock->state = 0;
	sock->count = 0;
	sock->flags = flags;
	sock->bgType = AG_SOCKET_RECT;
	sock->bgData.rect.w = 32;
	sock->bgData.rect.h = 32;
	sock->lblJustify = AG_TEXT_LEFT;
	sock->lPad = 2;
	sock->rPad = 2;
	sock->tPad = 2;
	sock->bPad = 2;
	sock->icon = NULL;
	sock->insertFn = NULL;
	sock->removeFn = NULL;

	AG_SetEvent(sock, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(sock, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(sock, "window-mousemotion", MouseMotion, NULL);

	if (flags & AG_SOCKET_HFILL) { WIDGET(sock)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_SOCKET_VFILL) { WIDGET(sock)->flags |= AG_WIDGET_VFILL; }
}

void
AG_SocketInsertFn(AG_Socket *sock, int (*fn)(AG_Socket *, AG_Icon *))
{
	sock->insertFn = fn;
}

void
AG_SocketRemoveFn(AG_Socket *sock, void (*fn)(AG_Socket *, AG_Icon *))
{
	sock->removeFn = fn;
}

static void
Destroy(void *p)
{
	AG_Socket *sock = p;

	AG_WidgetDestroy(sock);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Socket *sock = p;

	switch (sock->bgType) {
	case AG_SOCKET_PIXMAP:
		r->w = WSURFACE(sock,sock->bgData.pixmap.s)->w;
		r->h = WSURFACE(sock,sock->bgData.pixmap.s)->h;
		break;
	case AG_SOCKET_RECT:
		r->w = sock->bgData.rect.w;
		r->h = sock->bgData.rect.h;
		break;
	case AG_SOCKET_CIRCLE:
		r->w = sock->bgData.circle.r*2;
		r->h = sock->bgData.circle.r*2;
		break;
	}
	r->w += sock->lPad + sock->rPad;
	r->h += sock->tPad + sock->bPad;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Socket *sock = p;

	if (a->w < (sock->lPad + sock->rPad) ||
	    a->h < (sock->tPad + sock->bPad)) {
		return (-1);
	}
	return (0);
}

static __inline__ int
GetState(AG_WidgetBinding *binding, void *p)
{
	switch (binding->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
		return *(int *)p;
	case AG_WIDGET_UINT8:
		return (int)(*(Uint8 *)p);
	case AG_WIDGET_UINT16:
		return (int)(*(Uint16 *)p);
	case AG_WIDGET_UINT32:
		return (int)(*(Uint32 *)p);
	case AG_WIDGET_FLAG:
		return (*(int *)p & (int)binding->data.bitmask);
	case AG_WIDGET_FLAG8:
		return (int)(*(Uint8 *)p & (Uint8)binding->data.bitmask);
	case AG_WIDGET_FLAG16:
		return (int)(*(Uint16 *)p & (Uint16)binding->data.bitmask);
	case AG_WIDGET_FLAG32:
		return (int)(*(Uint32 *)p & (Uint32)binding->data.bitmask);
	}
	return (-1);
}

static __inline__ int
GetCount(AG_WidgetBinding *binding, void *p)
{
	switch (binding->vtype) {
	case AG_WIDGET_UINT:	return (Uint)(*(int *)p);
	case AG_WIDGET_INT:	return *(int *)p;
	case AG_WIDGET_UINT8:	return (int)(*(Uint8 *)p);
	case AG_WIDGET_SINT8:	return (int)(*(Sint8 *)p);
	case AG_WIDGET_UINT16:	return (int)(*(Uint16 *)p);
	case AG_WIDGET_SINT16:	return (int)(*(Sint16 *)p);
	case AG_WIDGET_UINT32:	return (int)(*(Uint32 *)p);
	case AG_WIDGET_SINT32:	return (int)(*(Sint32 *)p);
	}
	return (-1);
}

static void
Draw(void *p)
{
	AG_Socket *sock = p;
	AG_WidgetBinding *binding;
	void *pBinding;
	int state, count;
	
	binding = AG_WidgetGetBinding(sock, "state", &pBinding);
	state = GetState(binding, pBinding);
	AG_WidgetUnlockBinding(binding);
	binding = AG_WidgetGetBinding(sock, "count", &pBinding);
	count = GetCount(binding, pBinding);
	AG_WidgetUnlockBinding(binding);
	
	switch (sock->bgType) {
	case AG_SOCKET_PIXMAP:
		AG_WidgetBlitSurface(sock, sock->bgData.pixmap.s, 0, 0);
		if (sock->icon != NULL) {
			AG_WidgetBlitSurface(sock->icon, sock->icon->surface,
			    0, 0);
		}
		if (state) {
			agPrim.rect_outlined(sock, sock->lPad, sock->tPad,
			    WIDGET(sock)->w - sock->lPad - sock->rPad,
			    WIDGET(sock)->h - sock->tPad - sock->bPad,
			    AG_COLOR(SOCKET_HIGHLIGHT_COLOR));
		}
		break;
	case AG_SOCKET_RECT:
		if (AG_WidgetEnabled(sock)) {
			agPrim.box(sock,
			    0, 0,
			    WIDGET(sock)->w, WIDGET(sock)->h, -1,
			    AG_COLOR(SOCKET_COLOR));
		} else {
			agPrim.box_dithered(sock,
			    0, 0,
			    WIDGET(sock)->w, WIDGET(sock)->h, -1,
			    AG_COLOR(SOCKET_COLOR),
			    AG_COLOR(DISABLED_COLOR));
		}
		if (sock->icon != NULL) {
			AG_WidgetBlitSurface(sock->icon, sock->icon->surface,
			    0, 0);
		}
		if (state) {
			agPrim.rect_outlined(sock, sock->lPad, sock->tPad,
			    WIDGET(sock)->w - sock->lPad - sock->rPad,
			    WIDGET(sock)->h - sock->tPad - sock->bPad,
			    AG_COLOR(SOCKET_HIGHLIGHT_COLOR));
		}
		break;
	case AG_SOCKET_CIRCLE:
		agPrim.circle(sock,
		    WIDGET(sock)->w/2,
		    WIDGET(sock)->h/2,
		    sock->bgData.circle.r,
		    AG_COLOR(SOCKET_COLOR));
		if (sock->icon != NULL) {
			AG_WidgetBlitSurface(sock->icon, sock->icon->surface,
			    0, 0);
		}
		if (state) {
			agPrim.circle(sock,
			    WIDGET(sock)->w/2,
			    WIDGET(sock)->h/2,
			    sock->bgData.circle.r - sock->lPad,
			    AG_COLOR(SOCKET_HIGHLIGHT_COLOR));
		}
		break;
	}
}

static void
SetState(AG_WidgetBinding *binding, void *p, int v)
{
	switch (binding->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
		*(int *)p = v;
		break;
	case AG_WIDGET_UINT8:
		*(Uint8 *)p = v;
		break;
	case AG_WIDGET_UINT16:
		*(Uint16 *)p = v;
		break;
	case AG_WIDGET_UINT32:
		*(Uint32 *)p = v;
		break;
	case AG_WIDGET_FLAG:
		{
			int *state = (int *)p;
			if (*state & (int)binding->data.bitmask) {
				*state &= ~(int)binding->data.bitmask;
			} else {
				*state |= (int)binding->data.bitmask;
			}
		}
		break;
	case AG_WIDGET_FLAG8:
		{
			Uint8 *state = (Uint8 *)p;
			if (*state & (Uint8)binding->data.bitmask) {
				*state &= ~(Uint8)binding->data.bitmask;
			} else {
				*state |= (Uint8)binding->data.bitmask;
			}
		}
		break;
	case AG_WIDGET_FLAG16:
		{
			Uint16 *state = (Uint16 *)p;
			if (*state & (Uint16)binding->data.bitmask) {
				*state &= ~(Uint16)binding->data.bitmask;
			} else {
				*state |= (Uint16)binding->data.bitmask;
			}
		}
		break;
	case AG_WIDGET_FLAG32:
		{
			Uint32 *state = (Uint32 *)p;
			if (*state & (Uint32)binding->data.bitmask) {
				*state &= ~(Uint32)binding->data.bitmask;
			} else {
				*state |= (Uint32)binding->data.bitmask;
			}
		}
		break;
	}
}

#if 0
static void
SetCount(AG_WidgetBinding *binding, void *p, int v)
{
	switch (binding->vtype) {
	case AG_WIDGET_UINT:	*(Uint *)p = v;		break;
	case AG_WIDGET_INT:	*(int *)p = v;		break;
	case AG_WIDGET_UINT8:	*(Uint8 *)p = v;	break;
	case AG_WIDGET_SINT8:	*(Sint8 *)p = v;	break;
	case AG_WIDGET_UINT16:	*(Uint16 *)p = v;	break;
	case AG_WIDGET_SINT16:	*(Sint16 *)p = v;	break;
	case AG_WIDGET_UINT32:	*(Uint32 *)p = v;	break;
	case AG_WIDGET_SINT32:	*(Sint32 *)p = v;	break;
	}
}
#endif

static void
MouseMotion(AG_Event *event)
{
	AG_Socket *sock = AG_SELF();
	AG_WidgetBinding *binding;
	int x = AG_INT(1);
	int y = AG_INT(2);
	void *pState;

	if (AG_WidgetDisabled(sock))
		return;

	binding = AG_WidgetGetBinding(sock, "state", &pState);
	if (!AG_WidgetRelativeArea(sock, x, y)) {
		if ((sock->flags & AG_SOCKET_STICKY_STATE) == 0 &&
		    GetState(binding, pState) == 1) {
			SetState(binding, pState, 0);
			AG_WidgetBindingChanged(binding);
		}
		if (sock->flags & AG_SOCKET_MOUSEOVER) {
			sock->flags &= ~(AG_SOCKET_MOUSEOVER);
			AG_PostEvent(NULL, sock, "socket-mouseoverlap",
			    "%i", 0);
		}
	} else {
		sock->flags |= AG_SOCKET_MOUSEOVER;
		AG_PostEvent(NULL, sock, "socket-mouseoverlap", "%i", 1);
	}
	AG_WidgetUnlockBinding(binding);
}

static void
IconMotion(AG_Event *event)
{
	AG_Icon *icon = AG_PTR(1);
	int xRel = AG_INT(4);
	int yRel = AG_INT(5);
	AG_Window *wDND = icon->wDND;

	AG_WindowSetGeometryParam(wDND,
	    WIDGET(wDND)->x + xRel,
	    WIDGET(wDND)->y + yRel,
	    WIDGET(wDND)->w,
	    WIDGET(wDND)->h,
	    1);
}

static void
IconButtonUp(AG_Event *event)
{
	AG_Icon *icon = AG_PTR(1);
	AG_Window *wDND = icon->wDND;
	int x = WIDGET(wDND)->cx;
	int y = WIDGET(wDND)->cy;
	AG_Socket *sock;
	int detach = 1;

	sock = AG_WidgetFindRect("AG_Widget:AG_Socket:*", x, y,
	    WIDGET(wDND)->w,
	    WIDGET(wDND)->h);
	if (sock != NULL) {
		if (sock->insertFn != NULL) {
			detach = sock->insertFn(sock, icon);
		} else {
			if (icon->sock != NULL) {
				AG_SocketRemoveIcon(icon->sock);
			}
			AG_SocketInsertIcon(sock, icon);
		}
	}
	if (detach)
		AG_ViewDetach(wDND);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Socket *sock = AG_SELF();
	int button = AG_INT(1);
	AG_WidgetBinding *binding;
	void *pState;
	int newState;
	AG_Icon *icon;
	
	if (AG_WidgetDisabled(sock))
		return;

	AG_WidgetFocus(sock);

	if (button != SDL_BUTTON_LEFT)
		return;
	
	binding = AG_WidgetGetBinding(sock, "state", &pState);
	if (!(sock->flags & AG_SOCKET_STICKY_STATE)) {
		SetState(binding, pState, 1);
	} else {
		newState = !GetState(binding, pState);
		SetState(binding, pState, newState);
		AG_PostEvent(NULL, sock, "socket-click", "%i", newState);
	}
	AG_WidgetBindingChanged(binding);
	AG_WidgetUnlockBinding(binding);

	if ((icon = sock->icon) != NULL) {
		AG_Pixmap *px;
		
		icon->wDND = AG_WindowNew(AG_WINDOW_PLAIN|
		                          AG_WINDOW_NOBACKGROUND|
					  AG_WINDOW_NOUPDATERECT);
		px = AG_PixmapFromSurfaceCopy(icon->wDND, 0,
		    WSURFACE(icon,icon->surface));
		WIDGET(px)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
		                     AG_WIDGET_UNFOCUSED_BUTTONUP;
		AG_SetEvent(px, "window-mousemotion", IconMotion,"%p",icon);
		AG_SetEvent(px, "window-mousebuttonup", IconButtonUp,"%p",icon);

		AG_WindowSetGeometry(icon->wDND,
		    WIDGET(icon)->cx, WIDGET(icon)->cy,
		    WIDGET(icon)->w,  WIDGET(icon)->h);
		AG_WindowShow(icon->wDND);
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Socket *sock = AG_SELF();
	int button = AG_INT(1);
	AG_WidgetBinding *binding;
	void *pState;
	int x = AG_INT(2);
	int y = AG_INT(3);
		
	if (AG_WidgetDisabled(sock) ||
	    x < 0 || y < 0 ||
	    x > WIDGET(sock)->w || y > WIDGET(sock)->h) {
		return;
	}
	
	binding = AG_WidgetGetBinding(sock, "state", &pState);
	if (GetState(binding, pState) && button == SDL_BUTTON_LEFT &&
	    !(sock->flags & AG_SOCKET_STICKY_STATE)) {
	    	SetState(binding, pState, 0);
		AG_PostEvent(NULL, sock, "socket-click", "%i", 0);
		AG_WidgetBindingChanged(binding);
	}
	AG_WidgetUnlockBinding(binding);
}

void
AG_SocketSetPadding(AG_Socket *sock, int lPad, int rPad, int tPad, int bPad)
{
	if (lPad != -1) { sock->lPad = lPad; }
	if (rPad != -1) { sock->rPad = rPad; }
	if (tPad != -1) { sock->tPad = tPad; }
	if (bPad != -1) { sock->bPad = bPad; }
}

void
AG_SocketBgRect(AG_Socket *sock, Uint w, Uint h)
{
	sock->bgType = AG_SOCKET_RECT;
	sock->bgData.rect.w = w;
	sock->bgData.rect.h = h;
}

void
AG_SocketBgCircle(AG_Socket *sock, Uint r)
{
	sock->bgType = AG_SOCKET_RECT;
	sock->bgData.circle.r = r;
}

void
AG_SocketBgPixmap(AG_Socket *sock, SDL_Surface *su)
{
	SDL_Surface *suDup = (su != NULL) ? AG_DupSurface(su) : NULL;

	sock->bgType = AG_SOCKET_PIXMAP;
	sock->bgData.pixmap.s = AG_WidgetMapSurface(sock, suDup);
}

void
AG_SocketBgPixmapNODUP(AG_Socket *sock, SDL_Surface *su)
{
	sock->bgType = AG_SOCKET_PIXMAP;
	sock->bgData.pixmap.s = AG_WidgetMapSurface(sock, su);
}

void
AG_SocketInsertIcon(AG_Socket *sock, AG_Icon *icon)
{
	AG_SizeAlloc a;
	
	sock->icon = icon;
	icon->sock = sock;

	a.w = WIDGET(sock)->w;
	a.h = WIDGET(sock)->h;
	a.x = WIDGET(sock)->cx;
	a.y = WIDGET(sock)->cy;
	AG_WidgetSizeAlloc(icon, &a);
	AG_WidgetUpdateCoords(icon, a.x, a.y);
}

void
AG_SocketRemoveIcon(AG_Socket *sock)
{
	if (sock->icon != NULL) {
		if (sock->removeFn != NULL) {
			sock->removeFn(sock, sock->icon);
			return;
		}
		sock->icon->sock = NULL;
	}
	sock->icon = NULL;
}

const AG_WidgetOps agSocketOps = {
	{
		"AG_Widget:AG_Socket",
		sizeof(AG_Socket),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

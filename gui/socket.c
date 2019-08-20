/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/socket.h>
#include <agar/gui/icon.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/label.h>
#include <agar/gui/pixmap.h>

#include <stdarg.h>

AG_Socket *
AG_SocketNew(void *parent, Uint flags)
{
	AG_Socket *sock;

	sock = Malloc(sizeof(AG_Socket));
	AG_ObjectInit(sock, &agSocketClass);
	sock->flags |= flags;
	
	if (flags & AG_SOCKET_HFILL) { AG_ExpandHoriz(sock); }
	if (flags & AG_SOCKET_VFILL) { AG_ExpandVert(sock); }

	AG_ObjectAttach(parent, sock);
	return (sock);
}

AG_Socket *
AG_SocketFromSurface(void *parent, Uint flags, AG_Surface *S)
{
	AG_Socket *sock;
	
	sock = AG_SocketNew(parent, flags);
	AG_SocketBgPixmap(sock, S);
	return (sock);
}

#ifdef AG_SERIALIZATION
AG_Socket *
AG_SocketFromBMP(void *parent, Uint flags, const char *bmpfile)
{
	AG_Socket *sock;
	AG_Surface *bmp;
	
	if ((bmp = AG_SurfaceFromBMP(bmpfile)) == NULL) {
		AG_FatalError(NULL);
	}
	sock = AG_SocketNew(parent, flags);
	AG_SocketBgPixmapNODUP(sock, bmp);
	return (sock);
}
#endif /* AG_SERIALIZATION */

void
AG_SocketInsertFn(AG_Socket *sock, int (*fn)(AG_Socket *, AG_Icon *))
{
	AG_ObjectLock(sock);
	sock->insertFn = fn;
	AG_ObjectUnlock(sock);
}

void
AG_SocketRemoveFn(AG_Socket *sock, void (*fn)(AG_Socket *, AG_Icon *))
{
	AG_ObjectLock(sock);
	sock->removeFn = fn;
	AG_ObjectUnlock(sock);
}

void
AG_SocketOverlayFn(AG_Socket *sock, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sock);
	if (fn) {
		sock->overlayFn = AG_SetEvent(sock, NULL, fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(sock->overlayFn, fmt, ap);
			va_end(ap);
		}
	} else {
		sock->overlayFn = NULL;
	}
	AG_ObjectUnlock(sock);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Socket *sock = obj;

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
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Socket *sock = obj;

	if (a->w < (sock->lPad + sock->rPad) ||
	    a->h < (sock->tPad + sock->bPad)) {
		return (-1);
	}
	return (0);
}

static __inline__ int
GetState(AG_Variable *_Nonnull binding, void *_Nonnull p)
{
	switch (AG_VARIABLE_TYPE(binding)) {
	case AG_VARIABLE_INT:
		return *(int *)p;
	case AG_VARIABLE_UINT8:
		return (int)(*(Uint8 *)p);
	case AG_VARIABLE_UINT16:
		return (int)(*(Uint16 *)p);
	case AG_VARIABLE_UINT32:
		return (int)(*(Uint32 *)p);
	case AG_VARIABLE_P_FLAG:
		return (int)(*(Uint *)p & binding->info.bitmask.u);
	case AG_VARIABLE_P_FLAG8:
		return (int)(*(Uint8 *)p & binding->info.bitmask.u8);
	case AG_VARIABLE_P_FLAG16:
		return (int)(*(Uint16 *)p & binding->info.bitmask.u16);
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG32:
		return (int)(*(Uint32 *)p & binding->info.bitmask.u32);
#endif
	default:
		return (0);
	}
	return (0);
}

#if 0
static __inline__ int
GetCount(AG_Variable *_Nonnull binding, void *_Nonnull p)
{
	switch (AG_VARIABLE_TYPE(binding)) {
	case AG_VARIABLE_UINT:		return (Uint)(*(int *)p);
	case AG_VARIABLE_INT:		return *(int *)p;
	case AG_VARIABLE_UINT8:		return (int)(*(Uint8 *)p);
	case AG_VARIABLE_SINT8:		return (int)(*(Sint8 *)p);
	case AG_VARIABLE_UINT16:	return (int)(*(Uint16 *)p);
	case AG_VARIABLE_SINT16:	return (int)(*(Sint16 *)p);
	case AG_VARIABLE_UINT32:	return (int)(*(Uint32 *)p);
	case AG_VARIABLE_SINT32:	return (int)(*(Sint32 *)p);
	default:
	}
	return (-1);
}
#endif

static void
Draw(void *_Nonnull obj)
{
	AG_Socket *sock = obj;
	AG_Variable *binding;
	void *pBinding;
	AG_Rect r;
	int state;
	
	binding = AG_GetVariable(sock, "state", &pBinding);
	state = GetState(binding, pBinding);
	AG_UnlockVariable(binding);
#if 0
	binding = AG_GetVariable(sock, "count", &pBinding);
	count = GetCount(binding, pBinding);
	AG_UnlockVariable(binding);
#endif
	/* Draw background */
	switch (sock->bgType) {
	case AG_SOCKET_PIXMAP:
		AG_WidgetBlitSurface(sock, sock->bgData.pixmap.s, 0, 0);
		break;
	case AG_SOCKET_RECT:
		r.x = 0;
		r.y = 0;
		r.w = WIDTH(sock);
		r.h = HEIGHT(sock);
		if (AG_WidgetEnabled(sock)) {
			AG_DrawBox(sock, &r, -1, &WCOLOR(sock,0));
		} else {
			AG_DrawBoxDisabled(sock, &r, -1,
			    &WCOLOR(sock,0),
			    &WCOLOR_DIS(sock,0));
		}
		break;
	case AG_SOCKET_CIRCLE:
		AG_DrawCircle(sock,
		    WIDTH(sock) >> 1,
		    HEIGHT(sock) >> 1,
		    sock->bgData.circle.r, &WCOLOR(sock,0));
		break;
	}

	if (sock->icon) {
		AGWIDGET_OPS(sock->icon)->draw(sock->icon);
	}
	if (sock->overlayFn) {
		AG_PostEventByPtr(sock, sock->overlayFn, NULL);
	} else {
		switch (sock->bgType) {
		case AG_SOCKET_PIXMAP:
			/* TODO */
		case AG_SOCKET_RECT:
			if (state) {
				r.x = sock->lPad;
				r.y = sock->tPad;
				r.w = WIDTH(sock) - sock->lPad - sock->rPad;
				r.h = HEIGHT(sock) - sock->tPad - sock->bPad;
				AG_DrawRectOutline(sock, &r,
				    &WCOLOR_SEL(sock,LINE_COLOR));
			}
			break;
		case AG_SOCKET_CIRCLE:
			if (state) {
				AG_DrawCircle(sock,
				    WIDTH(sock) >> 1,
				    HEIGHT(sock) >> 1,
				    sock->bgData.circle.r - sock->lPad,
				    &WCOLOR_SEL(sock,LINE_COLOR));
			}
			break;
		}
	}
}

static void
SetState(AG_Socket *_Nonnull sock, AG_Variable *_Nonnull binding,
    void *_Nonnull p, int v)
{
	switch (AG_VARIABLE_TYPE(binding)) {
	case AG_VARIABLE_INT:
		*(int *)p = v;
		break;
	case AG_VARIABLE_UINT8:
		*(Uint8 *)p = v;
		break;
	case AG_VARIABLE_UINT16:
		*(Uint16 *)p = v;
		break;
	case AG_VARIABLE_UINT32:
		*(Uint32 *)p = v;
		break;
	case AG_VARIABLE_P_FLAG:
		AG_SETFLAGS(*(int *)p, binding->info.bitmask.u, v);
		break;
	case AG_VARIABLE_P_FLAG8:
		AG_SETFLAGS(*(Uint8 *)p, binding->info.bitmask.u8, v);
		break;
	case AG_VARIABLE_P_FLAG16:
		AG_SETFLAGS(*(Uint16 *)p, binding->info.bitmask.u16, v);
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG32:
		AG_SETFLAGS(*(Uint32 *)p, binding->info.bitmask.u32, v);
		break;
#endif
	default:
		break;
	}
	AG_Redraw(sock);
}

#if 0
static void
SetCount(AG_Variable *_Nonnull binding, void *_Nonnull p, int v)
{
	switch (AG_VARIABLE_TYPE(binding)) {
	case AG_VARIABLE_UINT:	 *(Uint *)p = v;	break;
	case AG_VARIABLE_INT:	 *(int *)p = v;		break;
	case AG_VARIABLE_UINT8:	 *(Uint8 *)p = v;	break;
	case AG_VARIABLE_SINT8:	 *(Sint8 *)p = v;	break;
	case AG_VARIABLE_UINT16: *(Uint16 *)p = v;	break;
	case AG_VARIABLE_SINT16: *(Sint16 *)p = v;	break;
	case AG_VARIABLE_UINT32: *(Uint32 *)p = v;	break;
	case AG_VARIABLE_SINT32: *(Sint32 *)p = v;	break;
	}
}
#endif

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Socket *sock = AG_SOCKET_SELF();
	AG_Variable *binding;
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	void *pState;

	if (AG_WidgetDisabled(sock))
		return;

	binding = AG_GetVariable(sock, "state", &pState);
	if (!AG_WidgetRelativeArea(sock, x, y)) {
		if ((sock->flags & AG_SOCKET_STICKY_STATE) == 0 &&
		    GetState(binding, pState) == 1) {
			SetState(sock, binding, pState, 0);
		}
		if (sock->flags & AG_SOCKET_MOUSEOVER) {
			sock->flags &= ~(AG_SOCKET_MOUSEOVER);
			AG_PostEvent(sock, "socket-mouseoverlap", "%i", 0);
			AG_Redraw(sock);
		}
	} else {
		sock->flags |= AG_SOCKET_MOUSEOVER;
		AG_PostEvent(sock, "socket-mouseoverlap", "%i", 1);
		AG_Redraw(sock);
	}
	AG_UnlockVariable(binding);
}

static void
IconMotion(AG_Event *_Nonnull event)
{
	AG_Icon *icon = AG_ICON_PTR(1);
	const int xRel = AG_INT(4);
	const int yRel = AG_INT(5);
	AG_Window *wDND = icon->wDND;
	AG_Rect r;

	r.x = WIDGET(wDND)->x + xRel;
	r.y = WIDGET(wDND)->y + yRel;
	r.w = WIDTH(wDND);
	r.h = HEIGHT(wDND);
	AG_WindowSetGeometryRect(wDND, &r, 1);
}

static void
IconButtonUp(AG_Event *_Nonnull event)
{
	AG_Icon *icon = AG_ICON_PTR(1);
	AG_Window *wDND = icon->wDND;
	int x = WIDGET(wDND)->rView.x1;
	int y = WIDGET(wDND)->rView.y1;
	AG_Socket *sock;
	int detach = 1;

	sock = AG_WidgetFindRect("AG_Widget:AG_Socket:*", x,y, WIDTH(wDND),
	                                                       HEIGHT(wDND));
	if (sock) {
		AG_ObjectLock(sock);
		if (sock->insertFn) {
			detach = sock->insertFn(sock, icon);
		} else {
			if (icon->sock) {
				AG_SocketRemoveIcon(icon->sock);
			}
			AG_SocketInsertIcon(sock, icon);
		}
		AG_ObjectUnlock(sock);
	}
	if (detach)
		AG_ObjectDetach(wDND);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Socket *sock = AG_SOCKET_SELF();
	const int button = AG_INT(1);
	AG_Variable *binding;
	void *pState;
	int newState;
	AG_Icon *icon;
	
	if (AG_WidgetDisabled(sock) ||
	    button != AG_MOUSE_LEFT) {
		return;
	}
	if (!AG_WidgetIsFocused(sock))
		AG_WidgetFocus(sock);
	
	binding = AG_GetVariable(sock, "state", &pState);
	if (!(sock->flags & AG_SOCKET_STICKY_STATE)) {
		SetState(sock, binding, pState, 1);
	} else {
		newState = !GetState(binding, pState);
		SetState(sock, binding, pState, newState);
		AG_PostEvent(sock, "socket-click", "%i", newState);
	}
	AG_UnlockVariable(binding);

	if ((icon = sock->icon) != NULL) {
		AG_Pixmap *px;
		
		icon->wDND = AG_WindowNew(AG_WINDOW_PLAIN |
		                          AG_WINDOW_NOBACKGROUND);
		px = AG_PixmapFromSurface(icon->wDND, 0,
		    WSURFACE(icon,icon->surface));

		AG_ObjectLock(px);
		WIDGET(px)->flags |= AG_WIDGET_UNFOCUSED_MOTION |
		                     AG_WIDGET_UNFOCUSED_BUTTONUP;
		AG_SetEvent(px, "mouse-motion", IconMotion,"%p",icon);
		AG_SetEvent(px, "mouse-button-up", IconButtonUp,"%p",icon);
		AG_ObjectUnlock(px);

		AG_WindowSetGeometry(icon->wDND,
		    WIDGET(icon)->rView.x1,
		    WIDGET(icon)->rView.y1,
		    WIDTH(icon),
		    HEIGHT(icon));
		AG_WindowShow(icon->wDND);
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Socket *sock = AG_SOCKET_SELF();
	const int button = AG_INT(1);
	AG_Variable *binding;
	void *pState;
	const int x = AG_INT(2);
	const int y = AG_INT(3);
		
	if (AG_WidgetDisabled(sock) ||
	    x < 0 || y < 0 ||
	    x > WIDTH(sock) || y > HEIGHT(sock)) {
		return;
	}
	
	binding = AG_GetVariable(sock, "state", &pState);
	if (GetState(binding, pState) && button == AG_MOUSE_LEFT &&
	    !(sock->flags & AG_SOCKET_STICKY_STATE)) {
	    	SetState(sock, binding, pState, 0);
		AG_PostEvent(sock, "socket-click", "%i", 0);
	}
	AG_UnlockVariable(binding);
}

void
AG_SocketSetPadding(AG_Socket *sock, int lPad, int rPad, int tPad, int bPad)
{
	if (lPad != -1) { sock->lPad = lPad; }
	if (rPad != -1) { sock->rPad = rPad; }
	if (tPad != -1) { sock->tPad = tPad; }
	if (bPad != -1) { sock->bPad = bPad; }
	AG_Redraw(sock);
}

void
AG_SocketBgRect(AG_Socket *sock, Uint w, Uint h)
{
	AG_ObjectLock(sock);
	sock->bgType = AG_SOCKET_RECT;
	sock->bgData.rect.w = w;
	sock->bgData.rect.h = h;
	AG_ObjectUnlock(sock);
	AG_Redraw(sock);
}

void
AG_SocketBgCircle(AG_Socket *sock, Uint r)
{
	AG_ObjectLock(sock);
	sock->bgType = AG_SOCKET_CIRCLE;
	sock->bgData.circle.r = r;
	AG_ObjectUnlock(sock);
	AG_Redraw(sock);
}

void
AG_SocketBgPixmap(AG_Socket *sock, AG_Surface *S)
{
	AG_Surface *Sdup = S ? AG_SurfaceDup(S) : NULL;

	AG_ObjectLock(sock);
	sock->bgType = AG_SOCKET_PIXMAP;
	sock->bgData.pixmap.s = AG_WidgetMapSurface(sock, Sdup);
	AG_ObjectUnlock(sock);
	AG_Redraw(sock);
}

void
AG_SocketBgPixmapNODUP(AG_Socket *sock, AG_Surface *S)
{
	AG_ObjectLock(sock);
	sock->bgType = AG_SOCKET_PIXMAP;
	sock->bgData.pixmap.s = AG_WidgetMapSurface(sock, S);
	AG_ObjectUnlock(sock);
	AG_Redraw(sock);
}

void
AG_SocketInsertIcon(AG_Socket *sock, AG_Icon *icon)
{
	AG_SizeAlloc a;
	
	AG_ObjectLock(sock);
	AG_ObjectLock(icon);

	AG_ObjectAttach(sock, icon);
	sock->icon = icon;
	icon->sock = sock;

	a.w = WIDTH(sock);
	a.h = HEIGHT(sock);
	a.x = WIDGET(sock)->rView.x1;
	a.y = WIDGET(sock)->rView.y1;
	AG_WidgetSizeAlloc(icon, &a);
	AG_WidgetUpdateCoords(icon, a.x, a.y);
	
	AG_ObjectUnlock(icon);
	AG_ObjectUnlock(sock);
	AG_Redraw(sock);
}

void
AG_SocketRemoveIcon(AG_Socket *sock)
{
	AG_ObjectLock(sock);

	if (sock->icon != NULL) {
		AG_ObjectLock(sock->icon);
		if (sock->removeFn != NULL) {
			sock->removeFn(sock, sock->icon);
			AG_ObjectUnlock(sock->icon);
			goto out;
		}
		sock->icon->sock = NULL;
		AG_ObjectDetach(sock->icon);
		AG_ObjectUnlock(sock->icon);
	}
	sock->icon = NULL;
out:
	AG_ObjectUnlock(sock);
	AG_Redraw(sock);
}

static void
Init(void *_Nonnull obj)
{
	AG_Socket *sock = obj;

	WIDGET(sock)->flags |= AG_WIDGET_FOCUSABLE |
	                       AG_WIDGET_UNFOCUSED_MOTION |
	                       AG_WIDGET_UNFOCUSED_BUTTONUP;
	sock->flags = 0;
	sock->state = 0;
	sock->count = 0;
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
	sock->overlayFn = NULL;

	AG_SetEvent(sock, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(sock, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(sock, "mouse-motion", MouseMotion, NULL);
	
	AG_BindInt(sock, "state", &sock->state);
	AG_BindInt(sock, "count", &sock->count);

	AG_RedrawOnChange(sock, 100, "state");
	AG_RedrawOnChange(sock, 500, "count");
}

AG_WidgetClass agSocketClass = {
	{
		"Agar(Widget:Socket)",
		sizeof(AG_Socket),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */

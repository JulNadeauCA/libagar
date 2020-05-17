/*
 * Copyright (c) 2007-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Drag-and-Drop icon widget. Used with AG_Socket(3).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/socket.h>
#include <agar/gui/icon.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

#include <stdarg.h>

AG_Icon *
AG_IconNew(void *parent, Uint flags)
{
	AG_Icon *icon;

	icon = Malloc(sizeof(AG_Icon));
	AG_ObjectInit(icon, &agIconClass);
	icon->flags |= flags;
	AG_ObjectAttach(parent, icon);
	return (icon);
}

AG_Icon *
AG_IconFromSurface(AG_Surface *su)
{
	AG_Icon *icon;

	icon = Malloc(sizeof(AG_Icon));
	AG_ObjectInit(icon, &agIconClass);
	AG_IconSetSurface(icon, su);
	return (icon);
}

AG_Icon *
AG_IconFromBMP(const char *bmpfile)
{
	AG_Icon *icon;
	AG_Surface *bmp;

	if ((bmp = AG_SurfaceFromBMP(bmpfile)) == NULL) {
		return (NULL);
	}
	icon = Malloc(sizeof(AG_Icon));
	AG_ObjectInit(icon, &agIconClass);
	AG_IconSetSurfaceNODUP(icon, bmp);
	return (icon);
}

static void
Init(void *_Nonnull obj)
{
	AG_Icon *icon = obj;

	WIDGET(icon)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_TEXT;

	icon->flags = 0;
	icon->surface = -1;
	icon->wDND = NULL;
	icon->sock = NULL;
	icon->labelTxt[0] = '\0';
	icon->labelSurface = -1;
	icon->xSaved = -1;
	icon->ySaved = -1;
	icon->wSaved = -1;
	icon->hSaved = -1;
	AG_ColorNone(&icon->cBackground);
	AG_InitTimer(&icon->toDblClick, "dblClick", 0);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Icon *icon = obj;
	const int paddingHoriz = WIDGET(icon)->paddingLeft +
	                         WIDGET(icon)->paddingRight;
	int wLbl, hLbl, su;

	r->w = paddingHoriz;
	r->h = WIDGET(icon)->paddingTop + WIDGET(icon)->paddingBottom;

	if ((su = icon->surface) != -1) {
		r->w += WSURFACE(icon,su)->w;
		r->h += WSURFACE(icon,su)->h;
	}
	if (icon->labelTxt[0] != '\0') {
		if (icon->labelSurface == -1) {
			AG_TextSize(icon->labelTxt, &wLbl, &hLbl);
		} else {
			wLbl = WSURFACE(icon,icon->labelSurface)->w;
			hLbl = WSURFACE(icon,icon->labelSurface)->h;
		}
		r->h += WIDGET(icon)->spacingVert + hLbl;
		r->w = MAX(r->w, wLbl + paddingHoriz);
	}
}

static void
Draw(void *_Nonnull obj)
{
	AG_Icon *icon = obj;
	AG_Surface *S;
	AG_Rect r;
	int w_2;

	if (icon->surface == -1) {
		return;
	}
	S = WSURFACE(icon, icon->surface);
	r.w = WIDTH(icon);
	w_2 = r.w >> 1;

	AG_PushBlendingMode(icon, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	AG_WidgetBlitSurface(icon, icon->surface, w_2 - (S->w >> 1), 0);

	if (icon->labelTxt[0] != '\0') {
		if (icon->labelSurface != -1 &&
		    icon->flags & AG_ICON_REGEN_LABEL) {
			AG_WidgetUnmapSurface(icon, icon->labelSurface);
			icon->labelSurface = -1;
		}
		if (icon->labelSurface == -1) {
			icon->labelSurface = AG_WidgetMapSurface(icon,
			    AG_TextRender(icon->labelTxt));
		}
		if (icon->flags & AG_ICON_BGFILL) {
			r.x = 0;
			r.y = S->h;
			r.h = HEIGHT(icon) - r.y;
			AG_DrawRect(icon, &r, &icon->cBackground);
		}
		AG_WidgetBlitSurface(icon, icon->labelSurface,
		    w_2 - (WSURFACE(icon,icon->labelSurface)->w >> 1),
		    S->h + WIDGET(icon)->spacingVert);
	}

	AG_PopBlendingMode(icon);
}

void
AG_IconSetBackgroundFill(AG_Icon *icon, int enable, const AG_Color *c)
{
	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	AG_ObjectLock(icon);

	AG_SETFLAGS(icon->flags, AG_ICON_BGFILL, enable);
	memcpy(&icon->cBackground, c, sizeof(AG_Color));
	AG_Redraw(icon);

	AG_ObjectUnlock(icon);
}

void
AG_IconSetSurface(AG_Icon *icon, const AG_Surface *S)
{
	AG_Surface *Sdup = (S != NULL) ? AG_SurfaceDup(S) : NULL;

	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	AG_ObjectLock(icon);

	if (icon->surface != -1) {
		AG_WidgetReplaceSurface(icon, icon->surface, Sdup);
	} else {
		icon->surface = AG_WidgetMapSurface(icon, Sdup);
	}

	AG_Redraw(icon);
	AG_ObjectUnlock(icon);
}

void
AG_IconSetSurfaceNODUP(AG_Icon *icon, AG_Surface *su)
{
	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	AG_ObjectLock(icon);

	if (icon->surface != -1) {
		AG_WidgetReplaceSurfaceNODUP(icon, icon->surface, su);
	} else {
		icon->surface = AG_WidgetMapSurfaceNODUP(icon, su);
	}

	AG_Redraw(icon);
	AG_ObjectUnlock(icon);
}

void
AG_IconSetText(AG_Icon *icon, const char *fmt, ...)
{
	va_list ap;

	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	AG_ObjectLock(icon);

	if (fmt[0] == '\0') {
		if (icon->labelSurface != -1) {
			AG_WidgetUnmapSurface(icon, icon->labelSurface);
			icon->labelSurface = -1;
		}
		icon->labelTxt[0] = '\0';
	} else {
		va_start(ap, fmt);
		Vsnprintf(icon->labelTxt, sizeof(icon->labelTxt), fmt, ap);
		va_end(ap);
		icon->flags |= AG_ICON_REGEN_LABEL;
	}

	AG_Redraw(icon);
	AG_ObjectUnlock(icon);
}

void
AG_IconSetTextS(AG_Icon *icon, const char *s)
{
	AG_OBJECT_ISA(icon, "AG_Widget:AG_Icon:*");
	AG_ObjectLock(icon);

	if (s == NULL || s[0] == '\0') {
		if (icon->labelSurface != -1) {
			AG_WidgetUnmapSurface(icon, icon->labelSurface);
			icon->labelSurface = -1;
		}
		icon->labelTxt[0] = '\0';
	} else {
		Strlcpy(icon->labelTxt, s, sizeof(icon->labelTxt));
		icon->flags |= AG_ICON_REGEN_LABEL;
	}

	AG_Redraw(icon);
	AG_ObjectUnlock(icon);
}

AG_WidgetClass agIconClass = {
	{
		"Agar(Widget:Icon)",
		sizeof(AG_Icon),
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
	NULL			/* size_allocate */
};

#endif /* AG_WIDGETS */

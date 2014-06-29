/*
 * Copyright (c) 2007-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
Init(void *obj)
{
	AG_Icon *icon = obj;

	WIDGET(icon)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_USE_TEXT;

	icon->flags = 0;
	icon->surface = -1;
	icon->wDND = NULL;
	icon->sock = NULL;
	icon->labelTxt[0] = '\0';
	icon->labelSurface = -1;
	icon->labelPad = 4;
	icon->xSaved = -1;
	icon->ySaved = -1;
	icon->wSaved = -1;
	icon->hSaved = -1;
	icon->cBackground = AG_ColorRGBA(0,0,0,0);
	AG_InitTimer(&icon->toDblClick, "dblClick", 0);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Icon *icon = obj;
	int wLbl, hLbl;

	if (icon->surface == -1) {
		r->w = 0;
		r->h = 0;
	}
	r->w = WSURFACE(icon,icon->surface)->w;
	r->h = WSURFACE(icon,icon->surface)->h;
	if (icon->labelTxt[0] != '\0') {
		if (icon->labelSurface != -1) {
			wLbl = WSURFACE(icon,icon->labelSurface)->w;
			hLbl = WSURFACE(icon,icon->labelSurface)->h;
		} else {
			AG_TextSize(icon->labelTxt, &wLbl, &hLbl);
		}
		r->h += icon->labelPad + hLbl;
		r->w = MAX(r->w, wLbl);
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	if (a->w < 1 || a->h < 1) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *obj)
{
	AG_Icon *icon = obj;
	int hIcon;

	if (icon->surface == -1) {
		return;
	}
	AG_WidgetBlitSurface(icon, icon->surface,
	    WIDGET(icon)->w/2 - WSURFACE(icon,icon->surface)->w/2,
	    0);
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
		hIcon = WSURFACE(icon,icon->surface)->h;
		if (icon->flags & AG_ICON_BGFILL) {
			AG_DrawRect(icon,
			    AG_RECT(0, hIcon, WIDTH(icon), HEIGHT(icon)-hIcon),
			    icon->cBackground);
		}
		AG_WidgetBlitSurface(icon, icon->labelSurface,
		    WIDTH(icon)/2 - WSURFACE(icon,icon->labelSurface)->w/2,
		    WSURFACE(icon,icon->surface)->h + icon->labelPad);
	}
}

void
AG_IconSetBackgroundFill(AG_Icon *icon, int enable, AG_Color C)
{
	AG_SETFLAGS(icon->flags, AG_ICON_BGFILL, enable);
	icon->cBackground = C;
	AG_Redraw(icon);
}

void
AG_IconSetSurface(AG_Icon *icon, AG_Surface *su)
{
	AG_Surface *suDup = (su != NULL) ? AG_SurfaceDup(su) : NULL;

	AG_ObjectLock(icon);
	if (icon->surface != -1) {
		AG_WidgetReplaceSurface(icon, icon->surface, suDup);
	} else {
		icon->surface = AG_WidgetMapSurface(icon, suDup);
	}
	AG_ObjectUnlock(icon);
	AG_Redraw(icon);
}

void
AG_IconSetSurfaceNODUP(AG_Icon *icon, AG_Surface *su)
{
	AG_ObjectLock(icon);
	if (icon->surface != -1) {
		AG_WidgetReplaceSurfaceNODUP(icon, icon->surface, su);
	} else {
		icon->surface = AG_WidgetMapSurfaceNODUP(icon, su);
	}
	AG_ObjectUnlock(icon);
	AG_Redraw(icon);
}

void
AG_IconSetText(AG_Icon *icon, const char *fmt, ...)
{
	va_list ap;

	AG_ObjectLock(icon);
	if (fmt == NULL || fmt[0] == '\0') {
		AG_WidgetUnmapSurface(icon, icon->labelSurface);
		icon->labelSurface = -1;
		icon->labelTxt[0] = '\0';
	} else {
		va_start(ap, fmt);
		Vsnprintf(icon->labelTxt, sizeof(icon->labelTxt), fmt, ap);
		va_end(ap);
		icon->flags |= AG_ICON_REGEN_LABEL;
	}
	AG_ObjectUnlock(icon);
	AG_Redraw(icon);
}

void
AG_IconSetTextS(AG_Icon *icon, const char *s)
{
	AG_ObjectLock(icon);
	if (s == NULL || s[0] == '\0') {
		AG_WidgetUnmapSurface(icon, icon->labelSurface);
		icon->labelSurface = -1;
		icon->labelTxt[0] = '\0';
	} else {
		Strlcpy(icon->labelTxt, s, sizeof(icon->labelTxt));
		icon->flags |= AG_ICON_REGEN_LABEL;
	}
	AG_ObjectUnlock(icon);
	AG_Redraw(icon);
}

AG_WidgetClass agIconClass = {
	{
		"Agar(Widget:Icon)",
		sizeof(AG_Icon),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

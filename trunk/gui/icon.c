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

#include <stdarg.h>

AG_Icon *
AG_IconNew(void)
{
	AG_Icon *icon;

	icon = Malloc(sizeof(AG_Icon));
	AG_ObjectInit(icon, &agIconClass);
	return (icon);
}

AG_Icon *
AG_IconFromSurface(SDL_Surface *su)
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
	SDL_Surface *bmp;

	if ((bmp = SDL_LoadBMP(bmpfile)) == NULL) {
		AG_SetError("%s: %s", bmpfile, SDL_GetError());
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

	WIDGET(icon)->flags |= AG_WIDGET_FOCUSABLE;

	icon->flags = 0;
	icon->surface = -1;
	icon->wDND = NULL;
	icon->sock = NULL;
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Icon *icon = obj;

	if (icon->surface != -1) {
		r->w = WSURFACE(icon,icon->surface)->w;
		r->h = WSURFACE(icon,icon->surface)->h;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	if (a->w < 1 ||
	    a->h < 1) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *obj)
{
	AG_Icon *icon = obj;

	AG_WidgetBlitSurface(icon, icon->surface, 0, 0);
}

void
AG_IconSetSurface(AG_Icon *icon, SDL_Surface *su)
{
	SDL_Surface *suDup = (su != NULL) ? AG_DupSurface(su) : NULL;

	if (icon->surface != -1) {
		AG_WidgetReplaceSurface(icon, icon->surface, suDup);
	} else {
		icon->surface = AG_WidgetMapSurface(icon, suDup);
	}
}

void
AG_IconSetSurfaceNODUP(AG_Icon *icon, SDL_Surface *su)
{
	if (icon->surface != -1) {
		AG_WidgetReplaceSurface(icon, icon->surface, su);
	} else {
		icon->surface = AG_WidgetMapSurface(icon, su);
	}
}

AG_WidgetClass agIconClass = {
	{
		"AG_Widget:AG_Icon",
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

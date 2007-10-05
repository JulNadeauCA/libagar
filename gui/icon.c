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
#include <core/view.h>

#include "icon.h"
#include "socket.h"

#include "window.h"
#include "primitive.h"

#include <stdarg.h>

static void SetState(AG_WidgetBinding *, void *, int);
static void MouseMotion(AG_Event *);
static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);

AG_Icon *
AG_IconNew(void *parent, Uint flags)
{
	AG_Icon *icon;

	icon = Malloc(sizeof(AG_Icon), M_OBJECT);
	AG_IconInit(icon, flags);
	AG_ObjectAttach(parent, icon);
	return (icon);
}

void
AG_IconInit(AG_Icon *icon, Uint flags)
{
	AG_WidgetInit(icon, &agSocketOps, AG_WIDGET_FOCUSABLE);
	icon->flags = 0;
	icon->surface = -1;

	AG_SetEvent(icon, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(icon, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(icon, "window-mousemotion", MouseMotion, NULL);
}

static void
Destroy(void *p)
{
	AG_Icon *icon = p;

	AG_WidgetDestroy(icon);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Icon *icon = p;

	r->w = WSURFACE(icon,icon->surface)->w;
	r->h = WSURFACE(icon,icon->surface)->h;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Icon *icon = p;

	if (a->w < 1 ||
	    a->h < 1) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *p)
{
	AG_Icon *icon = p;

	AG_WidgetBlitSurface(icon, icon->surface, 0, 0);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	int xRel = AG_INT(3);
	int yRel = AG_INT(4);

	printf("move %d,%d\n", xRel, yRel);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	int button = AG_INT(1);
	AG_WidgetBinding *binding;
	void *pState;
	int newState;

	if (button == SDL_BUTTON_RIGHT) {
		/* TODO popup */
	} else if (button != SDL_BUTTON_LEFT) {
		return;
	}
	if (AG_WidgetDisabled(icon)) {
		return;
	}
	AG_WidgetFocus(icon);

}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Icon *icon = AG_SELF();
	int button = AG_INT(1);

	/* TODO check socket overlap */
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

const AG_WidgetOps agIconOps = {
	{
		"AG_Widget:AG_Icon",
		sizeof(AG_Icon),
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

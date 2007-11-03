/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "radio.h"
#include "window.h"
#include "primitive.h"

AG_Radio *
AG_RadioNew(void *parent, Uint flags, const char **items)
{
	AG_Radio *rad;

	rad = Malloc(sizeof(AG_Radio), M_OBJECT);
	AG_RadioInit(rad, flags, items);
	AG_ObjectAttach(parent, rad);
	return (rad);
}

static void
Draw(void *p)
{
	AG_Radio *rad = p;
	int i, val;
	int x = rad->xPadding + rad->radius*2 + rad->xSpacing;
	int y = rad->yPadding;

	STYLE(rad)->RadioGroupBackground(rad,
	    AG_RECT(0, 0, WIDTH(rad), HEIGHT(rad)));
	val = AG_WidgetInt(rad, "value");
	for (i = 0; i < rad->nitems;
	     i++, y += (rad->radius*2 + rad->ySpacing)) {
		STYLE(rad)->RadioButton(rad, x, y,
		    (i == val),
		    (i == rad->oversel));
		AG_WidgetBlitSurface(rad, rad->labels[i], x, y);
	}
}

static void
Destroy(void *p)
{
	AG_Radio *rad = p;

	Free(rad->labels, M_WIDGET);
	AG_WidgetDestroy(rad);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Radio *rad = p;

	if (rad->nitems == 0) {
		r->w = 0;
		r->h = 0;
	} else {
		r->w = rad->xPadding*2 + rad->xSpacing*2 + rad->radius*2 +
		       rad->max_w;
		r->h = rad->yPadding*2 + rad->nitems*rad->radius*2 +
		       (rad->nitems-1)*rad->ySpacing;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Radio *rad = p;
	
	if (a->w < rad->xPadding*2 + rad->xSpacing*2 + rad->radius*2 +
	    rad->max_w ||
	    a->h < rad->yPadding*2 + rad->nitems*rad->radius*2 +
	           (rad->nitems-1)*rad->ySpacing) {
		WIDGET(rad)->flags |= AG_WIDGET_CLIPPING;
	} else {
		WIDGET(rad)->flags &= ~(AG_WIDGET_CLIPPING);
	}
	return (0);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	int y = AG_INT(2) - rad->yPadding;

	rad->oversel = (y/(rad->radius*2 + rad->ySpacing));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	AG_WidgetBinding *valueb;
	int button = AG_INT(1);
	int y = AG_INT(3);
	int *sel, selNew = -1;

	valueb = AG_WidgetGetBinding(rad, "value", &sel);
	switch (button) {
	case SDL_BUTTON_LEFT:
		selNew = ((y - rad->yPadding)/(rad->radius*2 + rad->ySpacing));
		if (selNew >= rad->nitems) {
			selNew = rad->nitems - 1;
		} else if (selNew < 0) {
			selNew = 0;
		}
		AG_WidgetFocus(rad);
		break;
	default:
		break;
	}
	if (selNew != -1 && selNew != *sel) {
		*sel = selNew;
		AG_PostEvent(NULL, rad, "radio-changed", "%i", *sel);
	}
	AG_WidgetUnlockBinding(valueb);
}

static void
KeyDown(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	AG_WidgetBinding *valueb;
	int keysym = AG_INT(1);
	int *sel, selNew = -1;

	valueb = AG_WidgetGetBinding(rad, "value", &sel);
	switch ((SDLKey)keysym) {
	case SDLK_DOWN:
		selNew = *sel;
		if (++selNew >= rad->nitems)
			selNew = rad->nitems-1;
		break;
	case SDLK_UP:
		selNew = *sel;
		if (--selNew < 0)
			selNew = 0;
		break;
	default:
		break;
	}
	if (selNew != -1 && selNew != *sel) {
		*sel = selNew;
		AG_PostEvent(NULL, rad, "radio-changed", "%i", *sel);
	}
	AG_WidgetUnlockBinding(valueb);
}

void
AG_RadioInit(AG_Radio *rad, Uint flags, const char **items)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;
	const char *s, **itemsp = items;
	int i;

	if (flags & AG_RADIO_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_RADIO_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(rad, &agRadioOps, wflags);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &rad->value);

	rad->flags = flags;
	rad->value = -1;
	rad->max_w = 0;
	rad->oversel = -1;
	rad->xPadding = 3;
	rad->yPadding = 4;
	rad->xSpacing = 7;
	rad->ySpacing = 2;
	rad->radius = 6;

	for (rad->nitems = 0; (s = *itemsp++) != NULL; rad->nitems++)
		;;
	rad->labels = Malloc(sizeof(int)*rad->nitems, M_WIDGET);
	for (i = 0; i < rad->nitems; i++) {
		SDL_Surface *su;

		AG_TextColor(RADIO_TXT_COLOR);
		su = AG_TextRender(_(items[i]));
		rad->labels[i] = AG_WidgetMapSurface(rad, su);
		if (su->w > rad->max_w)
			rad->max_w = su->w;
	}

	AG_SetEvent(rad, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(rad, "window-keydown", KeyDown, NULL);
	AG_SetEvent(rad, "window-mousemotion", MouseMotion, NULL);
}


const AG_WidgetOps agRadioOps = {
	{
		"AG_Widget:AG_Radio",
		sizeof(AG_Radio),
		{ 0,0, },
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

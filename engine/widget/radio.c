/*	$Csoft: radio.c,v 1.49 2005/05/13 09:21:47 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include "radio.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static AG_WidgetOps radio_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_RadioDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_RadioDraw,
	AG_RadioScale
};

enum {
	XSPACING =	7,
	YSPACING =	2,
	XPADDING =	3,
	YPADDING =	4,
	RADIUS =	6,
	SEL_RADIUS =	3
};

enum {
	MOUSEBUTTONDOWN_EVENT,
	KEYDOWN_EVENT
};

static void radio_event(int, union evarg *);
static void mousemotion(int, union evarg *);

AG_Radio *
AG_RadioNew(void *parent, const char **items)
{
	AG_Radio *rad;

	rad = Malloc(sizeof(AG_Radio), M_OBJECT);
	AG_RadioInit(rad, items);
	AG_ObjectAttach(parent, rad);
	return (rad);
}

void
AG_RadioInit(AG_Radio *rad, const char **items)
{
	const char *s, **itemsp = items;
	int i;

	AG_WidgetInit(rad, "radio", &radio_ops, AG_WIDGET_FOCUSABLE|
		                                AG_WIDGET_WFILL);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &rad->value);

	rad->value = -1;
	rad->max_w = 0;
	rad->oversel = -1;

	for (rad->nitems = 0; (s = *itemsp++) != NULL; rad->nitems++)
		;;
	rad->labels = Malloc(sizeof(int)*rad->nitems, M_WIDGET);
	for (i = 0; i < rad->nitems; i++) {
		SDL_Surface *su;

		su = AG_TextRender(NULL, -1, AG_COLOR(RADIO_TXT_COLOR),
		    _(items[i]));
		rad->labels[i] = AG_WidgetMapSurface(rad, su);
		if (su->w > rad->max_w)
			rad->max_w = su->w;
	}

	AG_SetEvent(rad, "window-mousebuttondown", radio_event, "%i",
	    MOUSEBUTTONDOWN_EVENT);
	AG_SetEvent(rad, "window-keydown", radio_event, "%i", KEYDOWN_EVENT);
	AG_SetEvent(rad, "window-mousemotion", mousemotion, NULL);
}

static const int highlight[18] = {
	-5, +1,
	-5,  0,
	-5, -1,
	-4, -2,
	-4, -3,
	-3, -4,
	-2, -4,
	 0, -5,
	-1, -5
};

void
AG_RadioDraw(void *p)
{
	AG_Radio *rad = p;
	int i, val, j;
	int x = XPADDING + RADIUS*2 + XSPACING;
	int y = YPADDING;

	agPrim.frame(rad,
	    0,
	    0,
	    AGWIDGET(rad)->w,
	    AGWIDGET(rad)->h,
	    AG_COLOR(FRAME_COLOR));

	val = AG_WidgetInt(rad, "value");

	for (i = 0;
	     i < rad->nitems;
	     i++, y += (RADIUS*2 + YSPACING)) {
		int xc = XPADDING + RADIUS;
		int yc = y + RADIUS;

		for (j = 0; j < 18; j+=2) {
			AG_WidgetPutPixel(rad,
			    xc + highlight[j],
			    yc + highlight[j+1],
			    AG_COLOR(RADIO_HI_COLOR));
			AG_WidgetPutPixel(rad,
			    xc - highlight[j],
			    yc - highlight[j+1],
			    AG_COLOR(RADIO_LO_COLOR));
		}

		if (i == val) {
			agPrim.circle(rad,
			    XPADDING + RADIUS,
			    y + RADIUS,
			    SEL_RADIUS,
			    AG_COLOR(RADIO_SEL_COLOR));
		} else if (i == rad->oversel) {
			agPrim.circle(rad,
			    XPADDING + RADIUS,
			    y + RADIUS,
			    SEL_RADIUS,
			    AG_COLOR(RADIO_OVER_COLOR));
		}
		AG_WidgetBlitSurface(rad, rad->labels[i], x, y);
	}
}

void
AG_RadioDestroy(void *p)
{
	AG_Radio *rad = p;
	int i;

	Free(rad->labels, M_WIDGET);
	AG_WidgetDestroy(rad);
}

void
AG_RadioScale(void *p, int rw, int rh)
{
	AG_Radio *rad = p;

	if (rw == -1)
		AGWIDGET(rad)->w = XPADDING*2 + XSPACING + RADIUS*2+rad->max_w;
	if (rh == -1)
		AGWIDGET(rad)->h = rad->nitems*(YSPACING + RADIUS*2)+YPADDING*2;
}

static void
mousemotion(int argc, union evarg *argv)
{
	AG_Radio *rad = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i - YPADDING;

	rad->oversel = (y/(RADIUS*2 + YSPACING));
}

static void
radio_event(int argc, union evarg *argv)
{
	AG_Radio *rad = argv[0].p;
	AG_WidgetBinding *valueb;
	int type = argv[1].i;
	int button, keysym;
	int y;
	int *sel;

	valueb = AG_WidgetGetBinding(rad, "value", &sel);
	switch (type) {
	case MOUSEBUTTONDOWN_EVENT:
		button = argv[2].i;
		if (button == SDL_BUTTON_LEFT) {
			y = argv[4].i - YPADDING;
			*sel = (y/(RADIUS*2 + YSPACING));
			AG_WidgetFocus(rad);
		}
		break;
	case KEYDOWN_EVENT:
		keysym = argv[2].i;
		switch ((SDLKey)keysym) {
		case SDLK_DOWN:
			if (++(*sel) > rad->nitems)
				*sel = 0;
			break;
		case SDLK_UP:
			if (--(*sel) < 0)
				*sel = 0;
			break;
		default:
			break;
		}
		break;
	default:
		return;
	}
	if (*sel >= rad->nitems) {
		*sel = rad->nitems - 1;
	}
	AG_PostEvent(NULL, rad, "radio-changed", "%i", *sel);
	AG_WidgetUnlockBinding(valueb);
}


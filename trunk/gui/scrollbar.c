/*
 * Copyright (c) 2002-2006 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
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

#include "scrollbar.h"

#include "window.h"
#include "primitive.h"

const AG_WidgetOps agScrollbarOps = {
	{
		"AG_Widget:AG_Scrollbar",
		sizeof(AG_Scrollbar),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_ScrollbarDraw,
	AG_ScrollbarScale
};

enum ag_button_which {
	AG_BUTTON_NONE,
	AG_BUTTON_UP,
	AG_BUTTON_DOWN,
	AG_BUTTON_SCROLL
};

static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);
static void MouseMotion(AG_Event *);

AG_Scrollbar *
AG_ScrollbarNew(void *parent, enum ag_scrollbar_type type, Uint flags)
{
	AG_Scrollbar *sb;

	sb = Malloc(sizeof(AG_Scrollbar), M_WIDGET);
	AG_ScrollbarInit(sb, type, flags);
	AG_ObjectAttach(parent, sb);
	if (flags & AG_SCROLLBAR_FOCUS) {
		AG_WidgetFocus(sb);
	}
	return (sb);
}

void
AG_ScrollbarInit(AG_Scrollbar *sb, enum ag_scrollbar_type type, Uint flags)
{
	Uint wflags = AG_WIDGET_UNFOCUSED_BUTTONUP|AG_WIDGET_UNFOCUSED_MOTION;

	if (flags & AG_SCROLLBAR_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_SCROLLBAR_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(sb, &agScrollbarOps, wflags);
	AG_WidgetBindInt(sb, "value", &sb->value);
	AG_WidgetBindInt(sb, "min", &sb->min);
	AG_WidgetBindInt(sb, "max", &sb->max);
	AG_WidgetBindInt(sb, "visible", &sb->visible);

	sb->flags = flags;
	sb->value = 0;
	sb->min = 0;
	sb->max = 0;
	sb->visible = 0;
	sb->type = type;
	sb->curbutton = AG_BUTTON_NONE;
	sb->bw = 15;				/* XXX resolution-dependent */
	sb->barSz = 30;
	sb->arrowSz = 9;

	AG_SetEvent(sb, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(sb, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(sb, "window-mousemotion", MouseMotion, NULL);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	sb->curbutton = AG_BUTTON_NONE;
}

/* Clicked or dragged mouse to coord, so adjust value */
static void
AG_ScrollbarMove(AG_Scrollbar *sb, int x, int totalsize)
{
	const int scrArea = totalsize - (sb->bw*2);
	int min, max;

	if (sb->curbutton != AG_BUTTON_SCROLL) { return; }
	if (sb->barSz == -1) { return; }
		
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	if (max < min)
		return;
	
	if (x <= sb->bw) {				/* Below min */
		AG_WidgetSetInt(sb, "value", min);
	} else if (x >= sb->bw + scrArea) {		/* Above max */
		AG_WidgetSetInt(sb, "value", max);
	} else {					/* Between */
		AG_WidgetSetInt(sb, "value",
		    (x - sb->bw)*(max-min+1) / scrArea);
	}
	
	AG_PostEvent(NULL, sb, "scrollbar-changed", "%i",
	    AG_WidgetInt(sb, "value"));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = (sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(2) : AG_INT(3);
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		AGWIDGET(sb)->w : AGWIDGET(sb)->h;
	int min, value, max, nvalue;

	if (button != SDL_BUTTON_LEFT)
		return;

	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	value = AG_WidgetInt(sb, "value");

	if (max < min)
		return;
	
	AG_WidgetFocus(sb);
	
	if (x <= sb->bw) {				/* Up button */
		sb->curbutton = AG_BUTTON_UP;
		if (value > min) {
			AG_WidgetSetInt(sb, "value", value - 1);
		}
	} else if (x >= totalsize - sb->bw) {		/* Down button */
		sb->curbutton = AG_BUTTON_DOWN;
		if (value < max) {
			AG_WidgetSetInt(sb, "value", value + 1);
		}
	} else {					/* In between */
		sb->curbutton = AG_BUTTON_SCROLL;
		AG_ScrollbarMove(sb, x, totalsize);
	}
	
	/* Generate an event if value changed. */
	if (value != (nvalue = AG_WidgetInt(sb, "value")))
		AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", nvalue);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int x = (sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(1) : AG_INT(2);
	int state = AG_INT(5);
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		AGWIDGET(sb)->w : AGWIDGET(sb)->h;

	if (state & SDL_BUTTON_LMASK)
		AG_ScrollbarMove(sb, x, totalsize);
}

void
AG_ScrollbarScale(void *p, int rw, int rh)
{
	AG_Scrollbar *sb = p;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		if (rw == -1) {
			AGWIDGET(sb)->w = sb->bw*4;
		}
		if (rh == -1) {
			AGWIDGET(sb)->h = sb->bw;
		} else {
			sb->bw = AGWIDGET(sb)->h;	/* Square */
		}
		break;
	case AG_SCROLLBAR_VERT:
		if (rw == -1) {
			AGWIDGET(sb)->w = sb->bw;
		} else {
			sb->bw = AGWIDGET(sb)->w;	/* Square */
		}
		if (rh == -1) {
			AGWIDGET(sb)->h = sb->bw*4;
		}
		break;
	}
}

void
AG_ScrollbarDraw(void *p)
{
	AG_Scrollbar *sb = p;
	int value, min, max;
	int w, h, x, y;
	int maxcoord;

	if ((max = (AG_WidgetInt(sb,"max") - AG_WidgetInt(sb,"visible")))== 0 ||
	     max < (min = AG_WidgetInt(sb, "min")) ||
	     (max - min) <= 0) {
		return;
	}
	value = AG_WidgetInt(sb, "value");
	
	if (AGWIDGET(sb)->w < sb->bw || AGWIDGET(sb)->w < sb->bw)
		return;

	if (value < min) {
		value = min;
		AG_WidgetSetInt(sb, "value", min);
	}
	if (value > max) {
		value = max-1;
		AG_WidgetSetInt(sb, "value", max-1);
	}
	
	agPrim.box(sb, 0, 0, AGWIDGET(sb)->w, AGWIDGET(sb)->h, -1,
	    AG_COLOR(SCROLLBAR_COLOR));

	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		if (AGWIDGET(sb)->h < sb->bw*2 + 6) {
			return;
		}
		maxcoord = AGWIDGET(sb)->h - sb->bw*2 - sb->barSz;

		agPrim.box(sb,
		    0, 0,
		    sb->bw, sb->bw, (sb->curbutton == AG_BUTTON_UP) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		agPrim.arrow_up(sb, sb->bw/2, sb->bw/2, sb->arrowSz,
		    AG_COLOR(SCROLLBAR_ARR1_COLOR),
		    AG_COLOR(SCROLLBAR_ARR2_COLOR));

		y = AGWIDGET(sb)->h - sb->bw;
		agPrim.box(sb,
		    0, y,
		    sb->bw, sb->bw, (sb->curbutton == AG_BUTTON_DOWN) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		agPrim.arrow_down(sb, sb->bw/2, (y + sb->bw/2), sb->arrowSz,
		    AG_COLOR(SCROLLBAR_ARR1_COLOR),
		    AG_COLOR(SCROLLBAR_ARR2_COLOR));
		
		if (sb->barSz == -1) {
			y = 0;
			h = AGWIDGET(sb)->h - sb->bw*2;
		} else {
			y = value * maxcoord / (max-min);
			h = sb->barSz;
			if (sb->bw + y + h >
			    AGWIDGET(sb)->h - sb->bw)
				y = AGWIDGET(sb)->h - sb->bw*2 - h;
		}
		agPrim.box(sb, 0, sb->bw+y, sb->bw, h,
		    (sb->curbutton == AG_BUTTON_SCROLL) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		break;
	case AG_SCROLLBAR_HORIZ:
		if (AGWIDGET(sb)->w < sb->bw*2 + 6) {
			return;
		}
		maxcoord = AGWIDGET(sb)->w - sb->bw*2 - sb->barSz;

		agPrim.box(sb,
		    0, 0,
		    sb->bw, sb->bw, (sb->curbutton == AG_BUTTON_UP) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		agPrim.arrow_left(sb, sb->bw/2, sb->bw/2, sb->arrowSz,
		    AG_COLOR(SCROLLBAR_ARR1_COLOR),
		    AG_COLOR(SCROLLBAR_ARR2_COLOR));

		x = AGWIDGET(sb)->w - sb->bw;
		agPrim.box(sb,
		    x, 0,
		    sb->bw, sb->bw, (sb->curbutton == AG_BUTTON_DOWN) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		agPrim.arrow_right(sb, (x + sb->bw/2), sb->bw/2, sb->arrowSz,
		    AG_COLOR(SCROLLBAR_ARR1_COLOR),
		    AG_COLOR(SCROLLBAR_ARR2_COLOR));
		
		/* Calculate disabled bar */
		if (sb->barSz == -1) {
			x = 0;
			w = AGWIDGET(sb)->w - sb->bw*2;
		} else {
			x = value * maxcoord / (max-min);
			w = sb->barSz;
			if (sb->bw + x + w >
			    AGWIDGET(sb)->w - sb->bw)
				x = AGWIDGET(sb)->w - sb->bw*2 - w;
		}
		agPrim.box(sb, sb->bw+x, 0, w, sb->bw,
		    (sb->curbutton == AG_BUTTON_SCROLL) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		break;
	}
#if 0
	{
		SDL_Surface *txt;
		char label[32];

		snprintf(label, sizeof(label), "%d/%d/%d", value, min, max);
		txt = AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), label);
		AG_WidgetBlit(sb, txt,
		    AGWIDGET(sb)->w/2 - txt->w/2,
		    AGWIDGET(sb)->h/2 - txt->h/2);
		SDL_FreeSurface(txt);
		    
	}
#endif
}

void
AG_ScrollbarSetBarSize(AG_Scrollbar *sb, int bsize)
{
	sb->barSz = (bsize > 10 || bsize == -1) ? bsize : 10;
}

void
AG_ScrollbarGetBarSize(AG_Scrollbar *sb, int *bsize)
{
	*bsize = sb->barSz;
}

/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "scrollbar.h"
#include "window.h"
#include "primitive.h"

enum ag_button_which {
	AG_BUTTON_NONE = 0,
	AG_BUTTON_UP = 1,
	AG_BUTTON_DOWN = 2,
	AG_BUTTON_SCROLL = 3
};

static void mousebuttonup(AG_Event *);
static void mousebuttondown(AG_Event *);
static void mousemotion(AG_Event *);

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

	AG_SetEvent(sb, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(sb, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(sb, "window-mousemotion", mousemotion, NULL);
}

/* Clicked or dragged mouse to coord, so adjust value */
static void
MoveBar(AG_Scrollbar *sb, int x, int totalsize)
{
	int scrArea = totalsize - (sb->bw*2);
	int min, max;
	int nVal;

	if (sb->curbutton != AG_BUTTON_SCROLL ||
	    sb->barSz == -1) {
		return;
	}
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	
	if (x <= sb->bw) {				/* Below min */
		nVal = min;
	} else if (x >= sb->bw + scrArea) {		/* Above max */
		nVal = max;
	} else {					/* Between */
		nVal = (x - sb->bw)*(max-min+1) / scrArea;
	}
	AG_WidgetSetInt(sb, "value", nVal);
	AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", nVal);
}

static void
mousebuttonup(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL)
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		return;
	}
	sb->curbutton = AG_BUTTON_NONE;
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = (sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(2) : AG_INT(3);
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		WIDGET(sb)->w : WIDGET(sb)->h;
	int min, value, max, nvalue;

	if (button != SDL_BUTTON_LEFT) {
		return;
	}
	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL)
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		return;
	}
	
	AG_WidgetFocus(sb);

	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	value = AG_WidgetInt(sb, "value");
	
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
		MoveBar(sb, x, totalsize);
	}
	
	/* Generate an event if value changed. */
	if (value != (nvalue = AG_WidgetInt(sb, "value")))
		AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", nvalue);
}

static void
mousemotion(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int x = (sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(1) : AG_INT(2);
	int state = AG_INT(5);
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		WIDGET(sb)->w : WIDGET(sb)->h;
#if 0	
	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL)
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		return;
	}
#endif
	if (state & SDL_BUTTON_LMASK)
		MoveBar(sb, x, totalsize);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Scrollbar *sb = p;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		r->w = sb->bw*2;
		r->h = sb->bw;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = sb->bw;
		r->h = sb->bw*2;
		break;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *p)
{
	AG_Scrollbar *sb = p;
	int min, max, val;
	int x, size;
	int maxcoord;

	if (!AG_ScrollbarVisible(sb)) {
		return;
	}
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	val = AG_WidgetInt(sb, "value");
	
	if (val < min) {
		val = min;
		AG_WidgetSetInt(sb, "value", min);
	}
	if (val > max) {
		val = max-1;
		AG_WidgetSetInt(sb, "value", max-1);
	}

	STYLE(sb)->ScrollbarBackground(sb);

	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		maxcoord = WIDGET(sb)->h - sb->bw*2 - sb->barSz;
		if (sb->barSz == -1) {
			x = 0;
			size = WIDGET(sb)->h - sb->bw*2;
		} else {
			x = val * maxcoord / (max-min);
			size = sb->barSz;
			if (sb->bw+x+size > WIDGET(sb)->h - sb->bw)
				x = WIDGET(sb)->h - sb->bw*2 - size;
		}
		STYLE(sb)->ScrollbarVertButtons(sb, x, size);
		break;
	case AG_SCROLLBAR_HORIZ:
		maxcoord = WIDGET(sb)->w - sb->bw*2 - sb->barSz;
		if (sb->barSz == -1) {
			x = 0;
			size = WIDGET(sb)->w - sb->bw*2;
		} else {
			x = val * maxcoord / (max-min);
			size = sb->barSz;
			if (sb->bw+x+size > WIDGET(sb)->w - sb->bw)
				x = WIDGET(sb)->w - sb->bw*2 - size;
		}
		STYLE(sb)->ScrollbarHorizButtons(sb, x, size);
		break;
	}
#if 0
	{
		SDL_Surface *txt;
		char label[32];

		snprintf(label, sizeof(label), "%d/%d/%d", val, min, max);
		AG_TextColor(TEXT_COLOR);
		txt = AG_TextRender(label);
		AG_WidgetBlit(sb, txt,
		    WIDGET(sb)->w/2 - txt->w/2,
		    WIDGET(sb)->h/2 - txt->h/2);
		SDL_FreeSurface(txt);
		    
	}
#endif
}

const AG_WidgetOps agScrollbarOps = {
	{
		"AG_Widget:AG_Scrollbar",
		sizeof(AG_Scrollbar),
		{ 0,0 },
		NULL,		/* init */
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

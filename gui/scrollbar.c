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

static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);
static void MouseMotion(AG_Event *);

AG_Scrollbar *
AG_ScrollbarNew(void *parent, enum ag_scrollbar_type type, Uint flags)
{
	AG_Scrollbar *sb;

	sb = Malloc(sizeof(AG_Scrollbar));
	AG_ObjectInit(sb, &agScrollbarClass);
	sb->type = type;
	sb->flags |= flags;

	if (flags & AG_SCROLLBAR_HFILL) { AG_ExpandHoriz(sb); }
	if (flags & AG_SCROLLBAR_VFILL) { AG_ExpandVert(sb); }

	AG_ObjectAttach(parent, sb);
	return (sb);
}

static void
Init(void *obj)
{
	AG_Scrollbar *sb = obj;

	WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION;

	AG_WidgetBindInt(sb, "value", &sb->value);
	AG_WidgetBindInt(sb, "min", &sb->min);
	AG_WidgetBindInt(sb, "max", &sb->max);
	AG_WidgetBindInt(sb, "visible", &sb->visible);

	sb->type = AG_SCROLLBAR_HORIZ;
	sb->flags = 0;
	sb->value = 0;
	sb->min = 0;
	sb->max = 0;
	sb->visible = 0;
	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->bwDefault = 16;			/* XXX */
	sb->bw = 16;
	sb->barSz = 8;
	sb->buttonIncFn = NULL;
	sb->buttonDecFn = NULL;

	AG_SetEvent(sb, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(sb, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(sb, "window-mousemotion", MouseMotion, NULL);
}

/* Return 1 if the scrollbar is any useful for the current values. */
int
AG_ScrollbarVisible(AG_Scrollbar *sb)
{
	int min, max, rv;

	AG_ObjectLock(sb);
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	rv = (max > 0 && max >= min);
	AG_ObjectUnlock(sb);
	return (rv);
}

/* Set an alternate handler for UP/LEFT button click. */
void
AG_ScrollbarSetIncFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sb);
	sb->buttonIncFn = AG_SetEvent(sb, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(sb->buttonIncFn, fmt);
	AG_ObjectUnlock(sb);
}

/* Set an alternate handler for DOWN/RIGHT button click. */
void
AG_ScrollbarSetDecFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sb);
	sb->buttonDecFn = AG_SetEvent(sb, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(sb->buttonDecFn, fmt);
	AG_ObjectUnlock(sb);
}

/*
 * Clicked or dragged mouse to coord, so adjust value.
 * Widget must be locked.
 */
static void
MoveBar(AG_Scrollbar *sb, int x, int totalsize)
{
	int scrArea = totalsize - (sb->bw*2);
	int min, max;
	int nVal;

	if (sb->curBtn != AG_SCROLLBAR_BUTTON_SCROLL ||
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
MouseButtonUp(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL) {
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		}
		return;
	}
	switch (sb->curBtn) {
	case AG_SCROLLBAR_BUTTON_DEC:
		if (sb->buttonDecFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonDecFn->name, "%i", 0);
		}
		break;
	case AG_SCROLLBAR_BUTTON_INC:
		if (sb->buttonIncFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonIncFn->name, "%i", 0);
		}
		break;
	default:
		break;
	}
	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
	AG_PostEvent(NULL, sb, "scrollbar-drag-end", "%i",
	    AG_WidgetInt(sb,"value"));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = (sb->type == AG_SCROLLBAR_HORIZ) ?
	        AG_INT(2) : AG_INT(3);
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		        WIDGET(sb)->w : WIDGET(sb)->h;
	int min, value, max, nvalue;

	if (button != SDL_BUTTON_LEFT) {
		return;
	}
	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL) {
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		}
		return;
	}
	
	AG_WidgetFocus(sb);

	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");
	value = AG_WidgetInt(sb, "value");
	
	if (x <= sb->bw) {				/* Up button */
		sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
		if (sb->buttonDecFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonDecFn->name, "%i", 1);
		} else {
			if (value > min)
				AG_WidgetSetInt(sb, "value", value-1);
		}
	} else if (x >= totalsize - sb->bw) {		/* Down button */
		sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
		if (sb->buttonIncFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonIncFn->name, "%i", 1);
		} else {
			if (value < max)
				AG_WidgetSetInt(sb, "value", value+1);
		}
	} else {					/* In between */
		sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
		MoveBar(sb, x, totalsize);
	}
	AG_PostEvent(NULL, sb, "scrollbar-drag-begin", "%i", value);
	
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
		r->w = sb->bwDefault*2;
		r->h = sb->bwDefault;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = sb->bwDefault;
		r->h = sb->bwDefault*2;
		break;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Scrollbar *sb = p;

	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		if (a->h < sb->bwDefault*2) {
			sb->bw = a->h/2;
		} else {
			sb->bw = sb->bwDefault;
		}
		break;
	case AG_SCROLLBAR_HORIZ:
		if (a->w < sb->bwDefault*2) {
			sb->bw = a->w/2;
		} else {
			sb->bw = sb->bwDefault;
		}
		break;
	}
	sb->arrowSz = sb->bw*5/9;
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
		if (AGWIDGET(sb)->h < sb->bw*2 + sb->barSz) {
			size = 0;
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
		if (AGWIDGET(sb)->w < sb->bw*2 + sb->barSz) {
			size = 0;
		}
		STYLE(sb)->ScrollbarHorizButtons(sb, x, size);
		break;
	}
#if 0
	{
		SDL_Surface *txt;
		char label[32];

		Snprintf(label, sizeof(label), "%d/%d/%d", val, min, max);
		AG_TextColor(TEXT_COLOR);
		txt = AG_TextRender(label);
		AG_WidgetBlit(sb, txt,
		    WIDGET(sb)->w/2 - txt->w/2,
		    WIDGET(sb)->h/2 - txt->h/2);
		SDL_FreeSurface(txt);
		    
	}
#endif
}

AG_WidgetClass agScrollbarClass = {
	{
		"AG_Widget:AG_Scrollbar",
		sizeof(AG_Scrollbar),
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

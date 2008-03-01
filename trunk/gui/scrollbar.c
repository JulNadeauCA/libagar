/*
 * Copyright (c) 2002-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "scrollbar.h"
#include "window.h"
#include "primitive.h"

#include "gui_math.h"

static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);
static void MouseMotion(AG_Event *);

#define TOTSIZE(sb) (((sb)->type==AG_SCROLLBAR_VERT) ? HEIGHT(sb) : WIDTH(sb))

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
Decrement(AG_Scrollbar *sb, int v)
{
	int value = AG_WidgetInt(sb, "value");
	int min = AG_WidgetInt(sb, "min");

	if (value > min) {
		AG_WidgetSetInt(sb, "value", value-v);
		AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", value-v);
	}
}
		
static void
Increment(AG_Scrollbar *sb, int v)
{
	int value = AG_WidgetInt(sb, "value");
	int max = AG_WidgetInt(sb, "max") - AG_WidgetInt(sb, "visible");

	if (value < max) {
		AG_WidgetSetInt(sb, "value", value+v);
		AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", value+v);
	}
}

static Uint32
ScrollTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Scrollbar *sb = obj;

	switch (sb->curBtn) {
	case AG_SCROLLBAR_BUTTON_DEC:
		Decrement(sb, 1);
		break;
	case AG_SCROLLBAR_BUTTON_INC:
		Increment(sb, 1);
		break;
	default:
		break;
	}
	return (agMouseSpinIval);
}

static void
LostFocus(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	AG_DelTimeout(sb, &sb->scrollTo);
}

static void
BoundValue(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	AG_WidgetBinding *bNew = AG_PTR(1);
	AG_WidgetBinding *bValue;
	void *pValue;

	/*
	 * Require that "min", "max" and "visible" be of the same binding
	 * type as "value" to avoid inefficient conversions. Mixing of types
	 * can always be implemented if some application requires it.
	 */
	if (!strcmp(bNew->name, "min") || !strcmp(bNew->name, "max") ||
	    !strcmp(bNew->name, "visible")) {
		bValue = AG_WidgetGetBinding(sb, "value", &pValue);
		if (bValue->vtype != bNew->vtype) {
			AG_FatalError("Scrollbar \"%s\" binding type disagree "
			              "with \"value\" binding", bNew->name);
		}
		AG_WidgetUnlockBinding(bValue);
	}
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
	sb->wButtonDef = 16;			/* XXX */
	sb->wButton = 16;
	sb->wBar = 8;
	sb->buttonIncFn = NULL;
	sb->buttonDecFn = NULL;
	sb->xRef = -1;

	AG_SetEvent(sb, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(sb, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(sb, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(sb, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(sb, "widget-hidden", LostFocus, NULL);
	AG_SetEvent(sb, "widget-bound", BoundValue, NULL);

	AG_SetTimeout(&sb->scrollTo, ScrollTimeout, NULL, 0);
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

/* Return the total width/height available for scrolling, in pixels. */
static __inline__ int
GetExtent(AG_Scrollbar *sb)
{
	return ((sb->type == AG_SCROLLBAR_VERT) ? HEIGHT(sb) : WIDTH(sb)) -
	       sb->wButton*2 - sb->wBar;
}

/*
 * Return current integer value and range. We subtract the value of the
 * "visible" binding from the range to accomodate most applications where
 * the Scrollbar is used to scroll a view of a certain size.
 */
static __inline__ void
GetIntValueRange(AG_Scrollbar *sb, int *min, int *max, int *val)
{
	AG_WidgetBinding *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_WidgetGetBinding(sb, "value", &pVal);
	bMin = AG_WidgetGetBinding(sb, "min", &pMin);
	bMax = AG_WidgetGetBinding(sb, "max", &pMax);
	bVis = AG_WidgetGetBinding(sb, "visible", &pVis);

	/* We are guaranteed that the types match. */
	switch (bVal->vtype) {
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
		*val = *(int *)pVal;
		*min = *(int *)pMin;
		*max = *(int *)pMax - *(int *)pVis;
		break;
	case AG_WIDGET_UINT32:
	case AG_WIDGET_SINT32:
		*val = (int)(*(Uint32 *)pVal);
		*min = (int)(*(Uint32 *)pMin);
		*max = (int)(*(Uint32 *)pMax) - (int)(*(Uint32 *)pVis);
		break;
	case AG_WIDGET_UINT16:
	case AG_WIDGET_SINT16:
		*val = (int)(*(Uint16 *)pVal);
		*min = (int)(*(Uint16 *)pMin);
		*max = (int)(*(Uint16 *)pMax) - (int)(*(Uint16 *)pVis);
		break;
	case AG_WIDGET_UINT8:
	case AG_WIDGET_SINT8:
		*val = (int)(*(Uint8 *)pVal);
		*min = (int)(*(Uint8 *)pMin);
		*max = (int)(*(Uint8 *)pMax) - (int)(*(Uint8 *)pVis);
		break;
	default:
		*val = 0;
		*min = 0;
		*max = 0;
		break;
	}

	/*
	 * Not supposed to happen but if the value gets out of bounds,
	 * pretend it is not, for the sake of our internal calculations.
	 */
	if (*val < *min) {
		*val = *min;
	} else if (*val > *max) {
		*val = *max;
	}
	AG_WidgetUnlockBinding(bVis);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
}

/* Return 1 if the scrollbar is any useful for the current values. */
int
AG_ScrollbarVisible(AG_Scrollbar *sb)
{
	int min, max, val, rv;

	AG_ObjectLock(sb);
	GetIntValueRange(sb, &min, &max, &val);
	rv = (min < max);
	AG_ObjectUnlock(sb);
	return (rv);
}

/*
 * Return the current position of the scrollbar, in terms of the position of
 * the Left (or Top) edge in pixels.
 */
static __inline__ int
GetPosition(AG_Scrollbar *sb, int min, int max, int val, int extent)
{
	return (sb->wBar == -1) ? 0 : val*extent/(max-min);
}

/* Set the value based on an absolute cursor position. */
static __inline__ void
SeekToPosition(AG_Scrollbar *sb, int x, int min, int max, int extent)
{
	int vNew;

	if (x <= 0) {
		vNew = min;
	} else if (x >= extent) {				/* Above max */
		vNew = max;
	} else {						/* Between */
		vNew = x*(max-min)/extent;
	}
	AG_WidgetSetInt(sb, "value", vNew);
	AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", vNew);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int min, max, val;

	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL) {
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		}
		return;
	}
	
	AG_DelTimeout(sb, &sb->scrollTo);

	if (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC && sb->buttonDecFn != NULL) {
		AG_PostEvent(NULL, sb, sb->buttonDecFn->name, "%i", 0);
	}
	if (sb->curBtn == AG_SCROLLBAR_BUTTON_INC && sb->buttonIncFn != NULL) {
		AG_PostEvent(NULL, sb, sb->buttonIncFn->name, "%i", 0);
	}
	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;

	if (sb->xRef != -1) {
		GetIntValueRange(sb, &min, &max, &val);
		AG_PostEvent(NULL, sb, "scrollbar-drag-end", "%i", val);
		sb->xRef = -1;
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = ((sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(2) : AG_INT(3)) -
	        sb->wButton;
	int min, value, max;
	int extent, pos;

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
	extent = GetExtent(sb);
	GetIntValueRange(sb, &min, &max, &value);
	pos = GetPosition(sb, min, max, value, extent);
	if (x < 0) {
		/*
		 * Click on DECREMENT button. Unless user provided a handler
		 * function, we decrement once and start the timer.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
		if (sb->buttonDecFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonDecFn->name, "%i", 1);
		} else {
			Decrement(sb, 1);
			AG_ReplaceTimeout(sb, &sb->scrollTo, agMouseSpinDelay);
		}
	} else if (x > TOTSIZE(sb) - sb->wButton*2) {
		/*
		 * Click on INCREMENT button. Unless user provided a handler
		 * function, we increment once and start the timer.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
		if (sb->buttonIncFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonIncFn->name, "%i", 1);
		} else {
			Increment(sb, 1);
			AG_ReplaceTimeout(sb, &sb->scrollTo, agMouseSpinDelay);
		}
	} else if (x >= pos && x <= (pos + sb->wBar)) {
		/*
		 * Click on the scrollbar itself. We don't do anything except
		 * saving the cursor position which we will use in future
		 * mousemotion events.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
		sb->xRef = x - pos;
		AG_PostEvent(NULL, sb, "scrollbar-drag-begin", "%i", value);
	} else if (sb->wBar != -1) {
		/*
		 * Click outside of scrollbar. We seek to the absolute position
		 * described by the cursor.
		 *
		 * XXX TODO: Provide an option to scroll progressively to the
		 * position since many users will expect that.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
		sb->xRef = sb->wBar/2;
		SeekToPosition(sb, x - sb->xRef, min, max, extent);
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int x, extent, min, max, val;

#if 0	
	if (!AG_ScrollbarVisible(sb)) {
		if (OBJECT(sb)->parent != NULL)
			AG_ForwardEvent(NULL, OBJECT(sb)->parent, event);
		return;
	}
#endif
	if (sb->xRef == -1) {
		return;
	}
	x = (sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(1) : AG_INT(2);
	x -= sb->wButton;
	extent = GetExtent(sb);
	GetIntValueRange(sb, &min, &max, &val);
	SeekToPosition(sb, x - sb->xRef, min, max, extent);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Scrollbar *sb = p;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		r->w = sb->wButtonDef*2;
		r->h = sb->wButtonDef;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = sb->wButtonDef;
		r->h = sb->wButtonDef*2;
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
		if (a->h < sb->wButtonDef*2) {
			sb->wButton = a->h/2;
		} else {
			sb->wButton = sb->wButtonDef;
		}
		break;
	case AG_SCROLLBAR_HORIZ:
		if (a->w < sb->wButtonDef*2) {
			sb->wButton = a->w/2;
		} else {
			sb->wButton = sb->wButtonDef;
		}
		break;
	}
	sb->arrowSz = sb->wButton*5/9;
	return (0);
}

static void
Draw(void *p)
{
	AG_Scrollbar *sb = p;
	int extent, min, max, val;
	int x, size;

	GetIntValueRange(sb, &min, &max, &val);
	if (min >= max)
		return;

	STYLE(sb)->ScrollbarBackground(sb);

	extent = GetExtent(sb);
	if (sb->wBar == -1) {
		x = 0;
		size = extent;
	} else {
		x = val*extent/(max-min);
		size = sb->wBar;
		if (size < 0) { size = 0; }
	}
	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		STYLE(sb)->ScrollbarVertButtons(sb, x, size);
		break;
	case AG_SCROLLBAR_HORIZ:
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
		    WIDTH(sb)/2 - txt->w/2,
		    HEIGHT(sb)/2 - txt->h/2);
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

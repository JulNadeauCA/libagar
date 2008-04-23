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
#include "text.h"

AG_Radio *
AG_RadioNew(void *parent, Uint flags, const char **itemText)
{
	AG_Radio *rad;

	rad = Malloc(sizeof(AG_Radio));
	AG_ObjectInit(rad, &agRadioClass);
	rad->flags |= flags;

	if (flags & AG_RADIO_HFILL) { AG_ExpandHoriz(rad); }
	if (flags & AG_RADIO_VFILL) { AG_ExpandVert(rad); }

	if (itemText != NULL) {
		AG_RadioItemsFromArray(rad, itemText);
	}
	AG_ObjectAttach(parent, rad);
	return (rad);
}

AG_Radio *
AG_RadioNewFn(void *parent, Uint flags, const char **itemText, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Radio *rad;
	AG_Event *ev;

	rad = AG_RadioNew(parent, flags, itemText);
	ev = AG_SetEvent(rad, "radio-changed", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	return (rad);
}

/* Create a set of items from an array of strings. */
void
AG_RadioItemsFromArray(AG_Radio *rad, const char **itemText)
{
	const char *s, **pItems;
	AG_RadioItem *ri;
	int i, w;
	
	AG_ObjectLock(rad);
	for (i = 0, pItems = itemText;
	     (s = *pItems++) != NULL;
	     i++) {
		rad->items = Realloc(rad->items, (rad->nItems+1) *
		                                 sizeof(AG_RadioItem));
		ri = &rad->items[rad->nItems++];
		Strlcpy(ri->text, s, sizeof(ri->text));
		ri->surface = -1;
		ri->hotkey = SDLK_UNKNOWN;
		AG_TextSize(s, &w, NULL);
		if (w > rad->max_w) { rad->max_w = w; }
	}
	AG_ObjectUnlock(rad);
}

/* Create a radio item and return its index. */
int
AG_RadioAddItem(AG_Radio *rad, const char *fmt, ...)
{
	AG_RadioItem *ri;
	va_list ap;
	int w, rv;

	AG_ObjectLock(rad);
	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = SDLK_UNKNOWN;
	va_start(ap, fmt);
	Vsnprintf(ri->text, sizeof(ri->text), fmt, ap);
	va_end(ap);

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->max_w) { rad->max_w = w; }
	rv = rad->nItems++;

	AG_ObjectUnlock(rad);
	return (rv);
}

/* Create a radio item and return its index (with hotkey). */
int
AG_RadioAddItemHK(AG_Radio *rad, SDLKey hotkey, const char *fmt, ...)
{
	AG_RadioItem *ri;
	va_list ap;
	int w, rv;

	AG_ObjectLock(rad);
	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = hotkey;
	va_start(ap, fmt);
	Vsnprintf(ri->text, sizeof(ri->text), fmt, ap);
	va_end(ap);

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->max_w) { rad->max_w = w; }
	rv = rad->nItems++;

	AG_ObjectUnlock(rad);
	return (rv);
}

/* Remove all radio items */
void
AG_RadioClearItems(AG_Radio *rad)
{
	int i;

	AG_ObjectLock(rad);
	for (i = 0; i < rad->nItems; i++) {
		if (rad->items[i].surface != -1)
			AG_WidgetUnmapSurface(rad, rad->items[i].surface);
	}
	Free(rad->items);
	rad->items = Malloc(sizeof(AG_RadioItem));
	rad->nItems = 0;
	rad->max_w = 0;
	AG_ObjectUnlock(rad);
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
	AG_PushTextState();
	for (i = 0; i < rad->nItems;
	     i++, y += (rad->radius*2 + rad->ySpacing)) {
		AG_RadioItem *ri = &rad->items[i];

		STYLE(rad)->RadioButton(rad, x, y,
		    (i == val),
		    (i == rad->oversel));
		if (ri->surface == -1) {
			AG_TextColor(RADIO_TXT_COLOR);
			ri->surface = AG_WidgetMapSurface(rad,
			    AG_TextRender(ri->text));
		}
		AG_WidgetBlitSurface(rad, ri->surface, x, y);
	}
	AG_PopTextState();
}

static void
Destroy(void *p)
{
	AG_Radio *rad = p;

	Free(rad->items);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Radio *rad = p;

	if (rad->nItems == 0) {
		r->w = 0;
		r->h = 0;
	} else {
		r->w = rad->xPadding*2 + rad->xSpacing*2 + rad->radius*2 +
		       rad->max_w;
		r->h = rad->yPadding*2 + rad->nItems*rad->radius*2 +
		       (rad->nItems-1)*rad->ySpacing;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Radio *rad = p;
	
	if (a->w < rad->xPadding*2 + rad->xSpacing*2 + rad->radius*2 +
	    rad->max_w ||
	    a->h < rad->yPadding*2 + rad->nItems*rad->radius*2 +
	           (rad->nItems-1)*rad->ySpacing) {
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
		if (selNew >= rad->nItems) {
			selNew = rad->nItems - 1;
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
	int i;

	valueb = AG_WidgetGetBinding(rad, "value", &sel);
	switch ((SDLKey)keysym) {
	case SDLK_DOWN:
		selNew = *sel;
		if (++selNew >= rad->nItems)
			selNew = rad->nItems-1;
		break;
	case SDLK_UP:
		selNew = *sel;
		if (--selNew < 0)
			selNew = 0;
		break;
	default:
		for (i = 0; i < rad->nItems; i++) {
			if (rad->items[i].hotkey != SDLK_UNKNOWN &&
			    rad->items[i].hotkey == keysym) {
				selNew = i;
				break;
			}
		}
		break;
	}
	if (selNew != -1 && selNew != *sel) {
		*sel = selNew;
		AG_PostEvent(NULL, rad, "radio-changed", "%i", *sel);
	}
	AG_WidgetUnlockBinding(valueb);
}

static void
Init(void *obj)
{
	AG_Radio *rad = obj;

	WIDGET(rad)->flags |= AG_WIDGET_FOCUSABLE;

	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &rad->value);

	rad->flags = 0;
	rad->value = -1;
	rad->max_w = 0;
	rad->oversel = -1;
	rad->xPadding = 3;
	rad->yPadding = 4;
	rad->xSpacing = 7;
	rad->ySpacing = 2;
	rad->radius = 6;
	rad->items = NULL;
	rad->nItems = 0;

	AG_SetEvent(rad, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(rad, "window-keydown", KeyDown, NULL);
	AG_SetEvent(rad, "window-mousemotion", MouseMotion, NULL);
}

AG_WidgetClass agRadioClass = {
	{
		"AG_Widget:AG_Radio",
		sizeof(AG_Radio),
		{ 0,0, },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

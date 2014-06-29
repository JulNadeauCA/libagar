/*
 * Copyright (c) 2002-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/radio.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>

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
AG_RadioNewInt(void *parent, Uint flags, const char **itemText, int *v)
{
	AG_Radio *rad;
	rad = AG_RadioNew(parent, flags, itemText);
	AG_BindInt(rad, "value", v);
	return (rad);
}

AG_Radio *
AG_RadioNewUint(void *parent, Uint flags, const char **itemText, Uint *v)
{
	AG_Radio *rad;
	rad = AG_RadioNew(parent, flags, itemText);
	AG_BindUint(rad, "value", v);
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
		ri->hotkey = 0;
		AG_TextSize(s, &w, NULL);
		if (w > rad->max_w) { rad->max_w = w; }
	}
	AG_ObjectUnlock(rad);
	AG_Redraw(rad);
}

/* Create a radio item and return its index (C string). */
int
AG_RadioAddItemS(AG_Radio *rad, const char *s)
{
	AG_RadioItem *ri;
	int w, rv;

	AG_ObjectLock(rad);
	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = 0;
	Strlcpy(ri->text, s, sizeof(ri->text));

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->max_w) { rad->max_w = w; }
	rv = rad->nItems++;

	AG_ObjectUnlock(rad);
	AG_Redraw(rad);
	return (rv);
}

/* Create a radio item and return its index (format string). */
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
	ri->hotkey = 0;

	va_start(ap, fmt);
	Vsnprintf(ri->text, sizeof(ri->text), fmt, ap);
	va_end(ap);

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->max_w) { rad->max_w = w; }
	rv = rad->nItems++;

	AG_ObjectUnlock(rad);
	AG_Redraw(rad);
	return (rv);
}

/* Create a radio item and return its index (with hotkey; C string). */
int
AG_RadioAddItemHKS(AG_Radio *rad, AG_KeySym hotkey, const char *s)
{
	AG_RadioItem *ri;
	int w, rv;

	AG_ObjectLock(rad);
	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = hotkey;
	Strlcpy(ri->text, s, sizeof(ri->text));

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->max_w) { rad->max_w = w; }
	rv = rad->nItems++;

	AG_ObjectUnlock(rad);
	AG_Redraw(rad);
	return (rv);
}

/* Create a radio item and return its index (with hotkey; format string). */
int
AG_RadioAddItemHK(AG_Radio *rad, AG_KeySym hotkey, const char *fmt, ...)
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
	AG_Redraw(rad);
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
	AG_Redraw(rad);
}

static void
Draw(void *obj)
{
	AG_Radio *rad = obj;
	int i, val;
	int x = rad->xPadding + rad->radius*2 + rad->xSpacing;
	int y = rad->yPadding;
	
	AG_DrawBox(rad,
	    AG_RECT(0, 0, WIDTH(rad), HEIGHT(rad)),
	    -1,
	    WCOLOR(rad,AG_COLOR));
	
	val = AG_GetInt(rad, "value");
	AG_PushClipRect(rad, rad->r);
	for (i = 0; i < rad->nItems; i++) {
		AG_RadioItem *ri = &rad->items[i];
		int xc, yc;
	
		if (ri->surface == -1) {
			ri->surface = AG_WidgetMapSurface(rad,
			    AG_TextRender(ri->text));
		}
		if (i == rad->oversel) {
			AG_Rect r = AG_RECT(rad->xPadding, y,
			    WIDGET(rad)->w - rad->xPadding*2,
			    rad->itemHeight);
			AG_DrawRectBlended(rad, r,
			    WCOLOR_HOV(rad,0),
			    AG_ALPHA_SRC);
		}

		xc = rad->xPadding + rad->radius;
		yc = y + rad->itemHeight/2;

		AG_DrawCircleFilled(rad, xc, yc, rad->radius,
		    WCOLOR(rad,SHAPE_COLOR));
		AG_DrawCircle(rad, xc, yc, rad->radius,
		    WCOLOR(rad,LINE_COLOR));

		if (i == val) {
			AG_DrawCircleFilled(rad, xc, yc,
			    rad->radius/2,
			    WCOLOR_SEL(rad,SHAPE_COLOR));
		}
		if (i == rad->oversel) {
			AG_DrawCircle(rad, xc, yc,
			    rad->radius - 2,
			    WCOLOR_HOV(rad,LINE_COLOR));
		}

		if (ri->surface != -1) {
			AG_WidgetBlitSurface(rad, ri->surface,
			    x,
			    y + rad->ySpacing/2);
		}
		y += rad->itemHeight;
	}
	AG_PopClipRect(rad);
}

static void
Destroy(void *p)
{
	AG_Radio *rad = p;

	Free(rad->items);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Radio *rad = obj;

	if (rad->nItems == 0) {
		r->w = 0;
		r->h = 0;
	} else {
		r->w = rad->xPadding*2 + rad->radius*2 + rad->xSpacing*2 +
		       rad->max_w;
		r->h = rad->yPadding*2 + rad->nItems*rad->itemHeight;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Radio *rad = obj;
	
	if (a->w < rad->radius*2 ||
	    a->h < rad->radius*2) {
		return (-1);
	}
	rad->r.x = rad->xPadding;
	rad->r.y = rad->yPadding;
	rad->r.w = a->w - rad->xPadding;
	rad->r.h = a->h - rad->yPadding;
	return (0);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2) - rad->yPadding;
	int ns;

	if (x < 0 || x > WIDGET(rad)->w ||
	    y < 0 || y > WIDGET(rad)->h) {
		rad->oversel = -1;
		return;
	}
	ns = (y / rad->itemHeight);
	if (ns != rad->oversel) {
		rad->oversel = ns;
		AG_Redraw(rad);
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	AG_Variable *value;
	int button = AG_INT(1);
	int y = AG_INT(3);
	int *sel, selNew = -1;

	if (!AG_WidgetIsFocused(rad))
		AG_WidgetFocus(rad);

	value = AG_GetVariable(rad, "value", &sel);
	switch (button) {
	case AG_MOUSE_LEFT:
		selNew = ((y - rad->yPadding) / rad->itemHeight);
		if (selNew >= rad->nItems) {
			selNew = rad->nItems - 1;
		} else if (selNew < 0) {
			selNew = 0;
		}
		break;
	}
	if (selNew != -1 && selNew != *sel) {
		*sel = selNew;
		AG_PostEvent(NULL, rad, "radio-changed", "%i", *sel);
		AG_Redraw(rad);
	}
	AG_UnlockVariable(value);
}

static void
KeyDown(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	AG_Variable *value;
	int keysym = AG_INT(1);
	int *sel, selNew = -1;
	int i;

	value = AG_GetVariable(rad, "value", &sel);
	switch (keysym) {
	case AG_KEY_DOWN:
		selNew = *sel;
		if (++selNew >= rad->nItems)
			selNew = rad->nItems-1;
		break;
	case AG_KEY_UP:
		selNew = *sel;
		if (--selNew < 0)
			selNew = 0;
		break;
	default:
		for (i = 0; i < rad->nItems; i++) {
			if (rad->items[i].hotkey != 0 &&
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
		AG_Redraw(rad);
	}
	AG_UnlockVariable(value);
}

static void
OnFontChange(AG_Event *event)
{
	AG_Radio *rad = AG_SELF();
	AG_Font *font = WIDGET(rad)->font;
	int i, w;

	rad->itemHeight = font->height + rad->ySpacing*2;
	rad->radius = MAX(0, font->height/2 - 1);
	rad->max_w = 0;

	for (i = 0; i < rad->nItems; i++) {
		AG_RadioItem *ri = &rad->items[i];

		if (ri->surface != -1) {
			AG_WidgetUnmapSurface(rad, ri->surface);
			ri->surface = -1;
		}
		AG_TextSize(ri->text, &w, NULL);
		if (w > rad->max_w) { rad->max_w = w; }
	}
		if (w > rad->max_w) { rad->max_w = w; }
}

static void
Init(void *obj)
{
	AG_Radio *rad = obj;

	WIDGET(rad)->flags |= AG_WIDGET_FOCUSABLE|
	                      AG_WIDGET_UNFOCUSED_MOTION|
			      AG_WIDGET_TABLE_EMBEDDABLE|
			      AG_WIDGET_USE_TEXT;

	rad->flags = 0;
	rad->value = -1;
	rad->max_w = 0;
	rad->oversel = -1;
	rad->xPadding = 4;
	rad->yPadding = 4;
	rad->xSpacing = 4;
	rad->ySpacing = 1;
	rad->itemHeight = agTextFontHeight + rad->ySpacing*2;
	rad->radius = MAX(0, agTextFontHeight/2 - 1);
	rad->items = NULL;
	rad->nItems = 0;
	rad->r = AG_RECT(0,0,0,0);

	AG_AddEvent(rad, "font-changed", OnFontChange, NULL);
	AG_SetEvent(rad, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(rad, "key-down", KeyDown, NULL);
	AG_SetEvent(rad, "mouse-motion", MouseMotion, NULL);

	AG_BindInt(rad, "value", &rad->value);
	AG_RedrawOnChange(rad, 100, "value");
}

AG_WidgetClass agRadioClass = {
	{
		"Agar(Widget:Radio)",
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

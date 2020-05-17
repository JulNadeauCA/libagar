/*
 * Copyright (c) 2002-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Radio Group widget. It displays one or more options (text labels) from
 * a NULL-terminated array of strings. Items can be arranged vertically or
 * horizontally. The radio group can be connected to an integer representing
 * the index of the selected item.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/radio.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>

static void DrawVert(AG_Radio *_Nonnull);
static void DrawHoriz(AG_Radio *_Nonnull);

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
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	return (rad);
}

AG_Radio *
AG_RadioNew(void *parent, Uint flags, const char **itemText)
{
	AG_Radio *rad;

	rad = Malloc(sizeof(AG_Radio));
	AG_ObjectInit(rad, &agRadioClass);

	if (flags & AG_RADIO_HFILL) { WIDGET(rad)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_RADIO_VFILL) { WIDGET(rad)->flags |= AG_WIDGET_VFILL; }
	rad->flags |= flags;

	if (itemText != NULL) {
		AG_RadioItemsFromArray(rad, itemText);
	}
	AG_ObjectAttach(parent, rad);
	return (rad);
}

/*
 * Set the disposition of radio items to horizontal or vertical.
 * (default = Vertical).
 */
void
AG_RadioSetDisposition(AG_Radio *rad, AG_RadioType type)
{
	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	rad->type = type;
	AG_Redraw(rad);
}

/* Create a set of radio items from a NULL-terminated array of strings. */
void
AG_RadioItemsFromArray(AG_Radio *rad, const char **itemText)
{
	const char *s, **pItems;
	AG_RadioItem *ri;
	int i, w;
	
	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	AG_ObjectLock(rad);

	for (i=0, pItems=itemText; (s = *pItems++) != NULL; i++) {
		rad->items = Realloc(rad->items, (rad->nItems+1) *
		                                 sizeof(AG_RadioItem));
		ri = &rad->items[rad->nItems++];
		Strlcpy(ri->text, s, sizeof(ri->text));
		ri->surface = -1;
		ri->hotkey = 0;
		AG_TextSize(s, &w, NULL);
		if (w > rad->extent) { rad->extent = w; }
	}

	AG_Redraw(rad);
	AG_ObjectUnlock(rad);
}

/* Create a radio item and return its index (C string). */
int
AG_RadioAddItemS(AG_Radio *rad, const char *s)
{
	AG_RadioItem *ri;
	int w, rv;

	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	AG_ObjectLock(rad);

	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = 0;
	Strlcpy(ri->text, s, sizeof(ri->text));

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->extent) { rad->extent = w; }
	rv = rad->nItems++;

	AG_Redraw(rad);
	AG_ObjectUnlock(rad);

	return (rv);
}

/* Create a radio item and return its index (format string). */
int
AG_RadioAddItem(AG_Radio *rad, const char *fmt, ...)
{
	AG_RadioItem *ri;
	va_list ap;
	int w, rv;

	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	AG_ObjectLock(rad);

	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = 0;

	va_start(ap, fmt);
	Vsnprintf(ri->text, sizeof(ri->text), fmt, ap);
	va_end(ap);

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->extent) { rad->extent = w; }
	rv = rad->nItems++;

	AG_Redraw(rad);
	AG_ObjectUnlock(rad);

	return (rv);
}

/* Create a radio item and return its index (with hotkey; C string). */
int
AG_RadioAddItemHKS(AG_Radio *rad, AG_KeySym hotkey, const char *s)
{
	AG_RadioItem *ri;
	int w, rv;

	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	AG_ObjectLock(rad);

	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = hotkey;
	Strlcpy(ri->text, s, sizeof(ri->text));

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->extent) { rad->extent = w; }
	rv = rad->nItems++;

	AG_Redraw(rad);
	AG_ObjectUnlock(rad);

	return (rv);
}

/* Create a radio item and return its index (with hotkey; format string). */
int
AG_RadioAddItemHK(AG_Radio *rad, AG_KeySym hotkey, const char *fmt, ...)
{
	AG_RadioItem *ri;
	va_list ap;
	int w, rv;

	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	AG_ObjectLock(rad);

	rad->items = Realloc(rad->items, (rad->nItems+1)*sizeof(AG_RadioItem));
	ri = &rad->items[rad->nItems];
	ri->surface = -1;
	ri->hotkey = hotkey;
	va_start(ap, fmt);
	Vsnprintf(ri->text, sizeof(ri->text), fmt, ap);
	va_end(ap);

	AG_TextSize(ri->text, &w, NULL);
	if (w > rad->extent) { rad->extent = w; }
	rv = rad->nItems++;

	AG_Redraw(rad);
	AG_ObjectUnlock(rad);

	return (rv);
}

/* Remove all radio items */
void
AG_RadioClearItems(AG_Radio *rad)
{
	int i;

	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");
	AG_ObjectLock(rad);

	for (i = 0; i < rad->nItems; i++) {
		if (rad->items[i].surface != -1)
			AG_WidgetUnmapSurface(rad, rad->items[i].surface);
	}
	Free(rad->items);
	rad->items = Malloc(sizeof(AG_RadioItem));
	rad->nItems = 0;
	rad->extent = 0;

	AG_Redraw(rad);
	AG_ObjectUnlock(rad);
}

/* Specify an alternate initial size requisition. */
void
AG_RadioSizeHint(AG_Radio *rad, int nLines, const char *text)
{
	AG_OBJECT_ISA(rad, "AG_Widget:AG_Radio:*");

	AG_TextSize(text, &rad->wPre, NULL);
	rad->hPre = nLines;
}

static void
Draw(void *_Nonnull obj)
{
	static void (*pfDraw[])(AG_Radio *) = {
		DrawVert,            /* VERT */
		DrawHoriz            /* HORIZ */
	};
	AG_Radio *rad = obj;
	const AG_Color *cBg = &WCOLOR(rad, BG_COLOR);
	const int isUndersize = ( WIDTH(rad) <= rad->wReq ||
	                          HEIGHT(rad) <= rad->hReq );
	
	if (cBg->a > 0)
		AG_DrawRect(rad, &WIDGET(rad)->r, cBg);

	if (AG_WidgetIsFocused(rad))
		AG_DrawRectOutline(rad, &WIDGET(rad)->r, &WCOLOR(rad,LINE_COLOR));

	if (isUndersize)
		AG_PushClipRect(rad, &WIDGET(rad)->r);

	AG_PushBlendingMode(rad, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

#ifdef AG_DEBUG
	if (rad->type >= AG_RADIO_TYPE_LAST)
		AG_FatalError("type");
#endif
	pfDraw[rad->type](rad);

	AG_PopBlendingMode(rad);

	if (isUndersize)
		AG_PopClipRect(rad);
}

static void
DrawVert(AG_Radio *_Nonnull rad)
{
	const AG_Color *cFg = &WCOLOR(rad, FG_COLOR);
	const AG_Color *cLine = &WCOLOR(rad, LINE_COLOR);
	const AG_Color *cSel = &WCOLOR(rad, SELECTION_COLOR);
	const int radius = (WFONT(rad)->lineskip >> 1);
	const int x = WIDGET(rad)->paddingLeft + WFONT(rad)->lineskip;
	const int value = AG_GetInt(rad, "value");
	int i, y;

	for (i = 0, y = WIDGET(rad)->paddingTop;
	     i < rad->nItems;
	     i++) {
		AG_RadioItem *radi = &rad->items[i];
		AG_Surface *S;
		AG_Rect r;
		int xc, yc;
	
		if (radi->surface == -1) {
			radi->surface = AG_WidgetMapSurface(rad,
			    AG_TextRender(radi->text));
		}
		S = WSURFACE(rad, radi->surface);
		if (i == rad->hoverItem) {
			r.x = WIDGET(rad)->paddingLeft;
			r.y = y;
			r.w = WIDTH(rad) - (WIDGET(rad)->paddingLeft +
			                    WIDGET(rad)->paddingRight);
			r.h = S->h;
			AG_DrawRect(rad, &r, &WCOLOR_HOVER(rad,BG_COLOR));
		}
		xc = WIDGET(rad)->paddingLeft + radius;
		yc = y + (S->h >> 1);
		AG_DrawCircleFilled(rad, xc,yc, radius, cFg);
		AG_DrawCircle(rad, xc,yc, radius, cLine);
		if (i == value) {
			AG_DrawCircleFilled(rad, xc,yc, (radius >> 1), cSel);
		}
		if (i == rad->hoverItem) {
			AG_DrawCircle(rad, xc,yc, radius-2,
			    &WCOLOR_HOVER(rad, LINE_COLOR));
		}
		AG_WidgetBlitSurface(rad, radi->surface, x + WIDGET(rad)->spacingHoriz, y);
		y += S->h + WIDGET(rad)->spacingVert;
	}
}

static void
DrawHoriz(AG_Radio *_Nonnull rad)
{
	const AG_Color *cFg = &WCOLOR(rad, FG_COLOR);
	const AG_Color *cLine = &WCOLOR(rad, LINE_COLOR);
	const AG_Color *cSel = &WCOLOR(rad, SELECTION_COLOR);
	const int radius = (WFONT(rad)->lineskip >> 1);
	const int y = WIDGET(rad)->paddingTop + WFONT(rad)->lineskip;
	const int value = AG_GetInt(rad, "value");
	int i, x;

	for (i = 0, x = WIDGET(rad)->paddingLeft;
	     i < rad->nItems;
	     i++) {
		AG_RadioItem *radi = &rad->items[i];
		AG_Surface *S;
		AG_Rect r;
		int xc, yc;
	
		if (radi->surface == -1) {
			radi->surface = AG_WidgetMapSurface(rad,
			    AG_TextRender(radi->text));
		}
		S = WSURFACE(rad, radi->surface);
		if (i == rad->hoverItem) {
			r.x = x;
			r.y = WIDGET(rad)->paddingTop;
			r.w = S->w;
			r.h = HEIGHT(rad) - (WIDGET(rad)->paddingTop +
			                     WIDGET(rad)->paddingBottom);
			AG_DrawRect(rad, &r, &WCOLOR_HOVER(rad,BG_COLOR));
		}
		xc = x + (S->w >> 1);
		yc = WIDGET(rad)->paddingTop + radius;
		AG_DrawCircleFilled(rad, xc,yc, radius, cFg);
		AG_DrawCircle(rad, xc,yc, radius, cLine);
		if (i == value) {
			AG_DrawCircleFilled(rad, xc,yc, (radius >> 1), cSel);
		}
		if (i == rad->hoverItem) {
			AG_DrawCircle(rad, xc,yc, radius-2,
			    &WCOLOR_HOVER(rad, LINE_COLOR));
		}
		AG_WidgetBlitSurface(rad, radi->surface, x,
		    y + WIDGET(rad)->spacingVert);
		x += S->w + WIDGET(rad)->spacingHoriz;
	}
}

static void
Destroy(void *_Nonnull obj)
{
	AG_Radio *rad = obj;

	Free(rad->items);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Radio *rad = obj;
	const int paddingHoriz = WIDGET(rad)->paddingLeft +
	                         WIDGET(rad)->paddingRight;
	const int paddingVert = WIDGET(rad)->paddingTop +
	                        WIDGET(rad)->paddingBottom;
	const int dia = WFONT(rad)->lineskip;
	int i;

	r->w = paddingHoriz;
	r->h = paddingVert;

	if (rad->wPre != -1 &&
	    rad->hPre != -1) {
		r->w += rad->wPre;
		r->h += rad->hPre;
		goto out;
	}

	if (rad->nItems == 0)
		goto out;

	switch (rad->type) {
	case AG_RADIO_VERT:
		for (i=0; i < rad->nItems; i++) {
			const AG_RadioItem *radi = &rad->items[i];
			int w, h;

			AG_TextSize(radi->text, &w, &h);
			r->h += h + WIDGET(rad)->spacingVert;
			r->w = MAX(r->w, dia + WIDGET(rad)->spacingHoriz + w +
			                 paddingHoriz);
		}
		if (rad->nItems > 1)
			r->h -= WIDGET(rad)->spacingVert;
		break;
	case AG_RADIO_HORIZ:
		for (i=0; i < rad->nItems; i++) {
			const AG_RadioItem *radi = &rad->items[i];
			int w, h;

			AG_TextSize(radi->text, &w, &h);
			r->w += w + WIDGET(rad)->spacingHoriz;
			r->h = MAX(r->h, dia + WIDGET(rad)->spacingVert + h +
			                 paddingVert);
		}
		if (rad->nItems > 1)
			r->w -= WIDGET(rad)->spacingHoriz;
		break;
	default:
		break;
	}
out:
	rad->wReq = r->w;
	rad->hReq = r->h;
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	int i, cur, itemNew = -1;

	if (x < 0 || x > WIDTH(rad) ||
	    y < 0 || y > HEIGHT(rad))
		goto out;

	switch (rad->type) {
	case AG_RADIO_VERT:
		for (i=0, cur=WIDGET(rad)->paddingTop;
		     i < rad->nItems;
		     i++) {
			const AG_RadioItem *radi = &rad->items[i];
			int h;

			if (radi->surface != -1) {
				h = WSURFACE(rad,radi->surface)->h;
			} else {
				AG_TextSize(radi->text, NULL, &h);
			}
			if (y >= cur && y <= cur+h) {
				itemNew = i;
				break;
			}
			cur += h + WIDGET(rad)->spacingVert;
		}
		break;
	case AG_RADIO_HORIZ:
		for (i=0, cur=WIDGET(rad)->paddingLeft;
		     i < rad->nItems;
		     i++) {
			const AG_RadioItem *radi = &rad->items[i];
			int w;

			if (radi->surface != -1) {
				w = WSURFACE(rad,radi->surface)->w;
			} else {
				AG_TextSize(radi->text, &w, NULL);
			}
			if (x >= cur && x <= cur+w) {
				itemNew = i;
				break;
			}
			cur += w + WIDGET(rad)->spacingHoriz;
		}
		break;
	default:
		break;
	}
out:
	if (rad->hoverItem != itemNew) {
		rad->hoverItem = itemNew;
/*		AG_PostEvent(rad, "radio-hover", "%i", itemNew); */
		AG_Redraw(rad);
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();
	const int button = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);
	int i, cur, *sel, selNew = -1;
	AG_Variable *value;

	if (!AG_WidgetIsFocused(rad))
		AG_WidgetFocus(rad);

	if (button != AG_MOUSE_LEFT)
		return;

	value = AG_GetVariable(rad, "value", (void *)&sel);

	switch (rad->type) {
	case AG_RADIO_VERT:
		for (i=0, cur=WIDGET(rad)->paddingTop;
		     i < rad->nItems;
		     i++) {
			const AG_RadioItem *radi = &rad->items[i];
			int h;

			if (radi->surface != -1) {
				h = WSURFACE(rad,radi->surface)->h;
			} else {
				AG_TextSize(radi->text, NULL, &h);
			}
			if (y >= cur && y <= cur+h) {
				selNew = i;
				break;
			}
			cur += h + WIDGET(rad)->spacingVert;
		}
		break;
	case AG_RADIO_HORIZ:
		for (i=0, cur = WIDGET(rad)->paddingLeft;
		     i < rad->nItems;
		     i++) {
			const AG_RadioItem *radi = &rad->items[i];
			int w;

			if (radi->surface != -1) {
				w = WSURFACE(rad,radi->surface)->w;
			} else {
				AG_TextSize(radi->text, &w, NULL);
			}
			if (x >= cur && x <= cur+w) {
				selNew = i;
				break;
			}
			cur += w + WIDGET(rad)->spacingHoriz;
		}
		break;
	default:
		break;
	}

	if (selNew != -1 && selNew != *sel) {
		*sel = selNew;
		AG_PostEvent(rad, "radio-changed", "%i", *sel);
		AG_Redraw(rad);
	}
	AG_UnlockVariable(value);
}

static void
MoveSelection(AG_Radio *rad, int delta)
{
	AG_Variable *V;
	int *sel, selNew;

	V = AG_GetVariable(rad, "value", (void *)&sel);
	selNew = *sel + delta;
	if (selNew >= rad->nItems) {
		selNew = rad->nItems - 1;
	} else if (selNew < 0) {
		selNew = 0;
	}
	if (selNew != -1 && selNew != *sel) {
		*sel = selNew;
		AG_PostEvent(rad, "radio-changed", "%i", *sel);
		AG_Redraw(rad);
	}
	AG_UnlockVariable(V);
}

/* Timer for moving keyboard selection. */
static Uint32
MoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();
	const int delta = AG_INT(1);

	if (delta < 0) {
		MoveSelection(rad, delta);
	} else {
		MoveSelection(rad, delta);
	}
	return (agKbdRepeat);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();
	const int keysym = AG_INT(1);
	int i;

	switch (keysym) {
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		MoveSelection(rad, +1);
		AG_AddTimer(rad, &rad->moveTo, agKbdDelay, MoveTimeout, "%i", +1);
		break;
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		MoveSelection(rad, -1);
		AG_AddTimer(rad, &rad->moveTo, agKbdDelay, MoveTimeout, "%i", -1);
		break;
	default:
		{
			AG_Variable *V;
			int *sel, selNew = -1;

			V = AG_GetVariable(rad, "value", (void *)&sel);
			for (i = 0; i < rad->nItems; i++) {
				if (rad->items[i].hotkey != 0 &&
				    rad->items[i].hotkey == keysym) {
					selNew = i;
					break;
				}
			}
			if (selNew != -1 && selNew != *sel) {
				*sel = selNew;
				AG_PostEvent(rad, "radio-changed", "%i", *sel);
				AG_Redraw(rad);
			}
			AG_UnlockVariable(V);
		}
		break;
	}
	rad->lastKeyDown = keysym;
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();
	const int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_DOWN:
	case AG_KEY_LEFT:
	case AG_KEY_RIGHT:
		if (keysym == rad->lastKeyDown) {
			AG_DelTimer(rad, &rad->moveTo);
		}
		break;
	}
}

static void
StyleChanged(AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();
	int i, w;

/*	rad->itemHeight = WFONT(rad)->lineskip; */
	rad->extent = 0;

	for (i = 0; i < rad->nItems; i++) {
		AG_RadioItem *ri = &rad->items[i];

		if (ri->surface != -1) {
			AG_WidgetUnmapSurface(rad, ri->surface);
			ri->surface = -1;
		}
		AG_TextSize(ri->text, &w, NULL);
		if (w > rad->extent)
			rad->extent = w;
	}
}

static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Radio *rad = AG_RADIO_SELF();

	AG_DelTimer(rad, &rad->moveTo);
}

static void
Init(void *_Nonnull obj)
{
	AG_Radio *rad = obj;

	WIDGET(rad)->flags |= AG_WIDGET_FOCUSABLE |
	                      AG_WIDGET_UNFOCUSED_MOTION |
			      AG_WIDGET_USE_TEXT;

	rad->type = AG_RADIO_VERT;
	rad->flags = 0;
	rad->value = -1;
	rad->items = NULL;
	rad->nItems = 0;
	rad->hoverItem = -1;
	rad->extent = 0;
/*	rad->itemHeight = agTextFontLineSkip; */
	rad->wPre = -1;
	rad->hPre = -1;
	rad->lastKeyDown = AG_KEY_NONE;

	AG_InitTimer(&rad->moveTo, "move", 0);

	AG_AddEvent(rad, "font-changed", StyleChanged, NULL);
	AG_AddEvent(rad, "palette-changed", StyleChanged, NULL);
	AG_AddEvent(rad, "widget-hidden", OnHide, NULL);

	AG_SetEvent(rad, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(rad, "key-down", KeyDown, NULL);
	AG_SetEvent(rad, "key-up", KeyUp, NULL);
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
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	NULL,			/* size_allocate */
};

#endif /* AG_WIDGETS */

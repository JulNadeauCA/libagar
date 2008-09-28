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

#include "button.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

#include <stdarg.h>

static int GetState(AG_Button *, AG_WidgetBinding *, void *);
static void SetState(AG_WidgetBinding *, void *, int);
static void mousemotion(AG_Event *);
static void mousebuttonup(AG_Event *);
static void mousebuttondown(AG_Event *);
static void keyup(AG_Event *);
static void keydown(AG_Event *);

AG_Button *
AG_ButtonNew(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Button *bu;
	va_list args;
	
	bu = Malloc(sizeof(AG_Button));
	AG_ObjectInit(bu, &agButtonClass);

	if (fmt != NULL) {
		va_start(args, fmt);
		Vasprintf(&bu->text, fmt, args);
		va_end(args);
	} else {
		bu->text = NULL;
	}
	
	bu->flags |= flags;

	if (flags & AG_BUTTON_HFILL) { AG_ExpandHoriz(bu); }
	if (flags & AG_BUTTON_VFILL) { AG_ExpandVert(bu); }

	AG_ObjectAttach(parent, bu);
	return (bu);
}

AG_Button *
AG_ButtonNewFn(void *parent, Uint flags, const char *caption, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;

	bu = AG_ButtonNew(parent, flags, caption);
	ev = AG_SetEvent(bu, "button-pushed", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	return (bu);
}

AG_Button *
AG_ButtonNewBool(void *parent, Uint flags, const char *caption,
    AG_WidgetBindingType type, void *p)
{
	AG_Button *bu = AG_ButtonNew(parent, flags, caption);
	AG_WidgetBind(bu, "state", type, p);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag(void *parent, Uint flags, const char *caption,
    Uint *p, Uint bitmask)
{
	AG_Button *bu = AG_ButtonNew(parent, flags, caption);
	AG_WidgetBindFlag(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag8(void *parent, Uint flags, const char *caption,
   Uint8 *p, Uint8 bitmask)
{
	AG_Button *bu = AG_ButtonNew(parent, flags, caption);
	AG_WidgetBindFlag8(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag16(void *parent, Uint flags, const char *caption,
    Uint16 *p, Uint16 bitmask)
{
	AG_Button *bu = AG_ButtonNew(parent, flags, caption);
	AG_WidgetBindFlag16(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag32(void *parent, Uint flags, const char *caption,
    Uint32 *p, Uint32 bitmask)
{
	AG_Button *bu = AG_ButtonNew(parent, flags, caption);
	AG_WidgetBindFlag32(bu, "state", p, bitmask);
	return (bu);
}

static Uint32
ExpireRepeat(void *obj, Uint32 ival, void *arg)
{
	AG_PostEvent(NULL, obj, "button-pushed", "%i", 1);
	return (agMouseSpinIval);
}

static Uint32
ExpireDelay(void *obj, Uint32 ival, void *arg)
{
	AG_Button *bu = obj;

	AG_ReplaceTimeout(bu, &bu->repeat_to, agMouseSpinIval);
	return (0);
}

static void
Init(void *obj)
{
	AG_Button *bu = obj;

	/* TODO replace the unfocused motion flag with a timer */
	WIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_UNFOCUSED_BUTTONUP;

	AG_WidgetBindBool(bu, "state", &bu->state);

	bu->flags = 0;
	bu->text = NULL;
	bu->surface = -1;
	bu->state = 0;
	bu->justify = AG_TEXT_CENTER;
	bu->valign = AG_TEXT_MIDDLE;
	bu->lPad = 4;
	bu->rPad = 4;
	bu->tPad = 3;
	bu->bPad = 3;

	AG_SetTimeout(&bu->repeat_to, ExpireRepeat, NULL, 0);
	AG_SetTimeout(&bu->delay_to, ExpireDelay, NULL, 0);

	AG_SetEvent(bu, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(bu, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(bu, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(bu, "window-keyup", keyup, NULL);
	AG_SetEvent(bu, "window-keydown", keydown, NULL);
}

static void
Destroy(void *p)
{
	AG_Button *bu = p;

	if ((bu->flags & AG_BUTTON_TEXT_NODUP) == 0)
		Free(bu->text);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Button *bu = p;

	if (bu->surface != -1) {
		r->w = WSURFACE(bu,bu->surface)->w;
		r->h = WSURFACE(bu,bu->surface)->h;
	} else {
		if (bu->text != NULL && bu->text[0] != '\0') {
			AG_TextSize(bu->text, &r->w, &r->h);
		} else {
			r->w = 0;
			r->h = agTextFontHeight;
		}
	}
	r->w += bu->lPad + bu->rPad;
	r->h += bu->tPad + bu->bPad;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Button *bu = p;

	if (a->w < (bu->lPad + bu->rPad) ||
	    a->h < (bu->tPad + bu->bPad)) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *p)
{
	AG_Button *bu = p;
	AG_WidgetBinding *binding;
	void *pState;
	int x = 0, y = 0;
	int pressed, wLbl, hLbl;
	
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	pressed = GetState(bu, binding, pState);
	AG_WidgetUnlockBinding(binding);

	STYLE(bu)->ButtonBackground(bu, pressed);

	if (bu->text != NULL && bu->text[0] != '\0') {
		if (bu->surface == -1) {
			AG_PushTextState();
			AG_TextJustify(bu->justify);
			AG_TextColor(BUTTON_TXT_COLOR);
			bu->surface = AG_WidgetMapSurface(bu,
			    AG_TextRender(bu->text));
			AG_PopTextState();
		} else if (bu->flags & AG_BUTTON_REGEN) {
			AG_PushTextState();
			AG_TextJustify(bu->justify);
			AG_TextColor(BUTTON_TXT_COLOR);
			AG_WidgetReplaceSurface(bu, bu->surface,
			    AG_TextRender(bu->text));
			AG_PopTextState();
		}
	}
	if (bu->surface == -1)
		return;

	wLbl = WSURFACE(bu,bu->surface)->w;
	hLbl = WSURFACE(bu,bu->surface)->h;

	switch (bu->justify) {
	case AG_TEXT_LEFT:
		x = bu->lPad;
		break;
	case AG_TEXT_CENTER:
		x = WIDGET(bu)->w/2 - wLbl/2;
		break;
	case AG_TEXT_RIGHT:
		x = WIDGET(bu)->w - wLbl - bu->rPad;
		break;
	}
	y = WIDGET(bu)->h/2 - hLbl/2;
	STYLE(bu)->ButtonTextOffset(bu, pressed, &x, &y);
	AG_WidgetBlitSurface(bu, bu->surface, x, y);
}

static int
GetState(AG_Button *bu, AG_WidgetBinding *binding, void *p)
{
	int v = 0;

	switch (binding->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
		v = *(int *)p;
		break;
	case AG_WIDGET_UINT8:
		v = (int)(*(Uint8 *)p);
		break;
	case AG_WIDGET_UINT16:
		v = (int)(*(Uint16 *)p);
		break;
	case AG_WIDGET_UINT32:
		v = (int)(*(Uint32 *)p);
		break;
	case AG_WIDGET_FLAG:
		v = (*(int *)p & (int)binding->data.bitmask);
		break;
	case AG_WIDGET_FLAG8:
		v = (int)(*(Uint8 *)p & (Uint8)binding->data.bitmask);
		break;
	case AG_WIDGET_FLAG16:
		v = (int)(*(Uint16 *)p & (Uint16)binding->data.bitmask);
		break;
	case AG_WIDGET_FLAG32:
		v = (int)(*(Uint32 *)p & (Uint32)binding->data.bitmask);
		break;
	}
	if (bu->flags & AG_BUTTON_INVSTATE) {
		v = !v;
	}
	return (v);
}

static void
SetState(AG_WidgetBinding *binding, void *p, int v)
{
	switch (binding->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
		*(int *)p = v;
		break;
	case AG_WIDGET_UINT8:
		*(Uint8 *)p = v;
		break;
	case AG_WIDGET_UINT16:
		*(Uint16 *)p = v;
		break;
	case AG_WIDGET_UINT32:
		*(Uint32 *)p = v;
		break;
	case AG_WIDGET_FLAG:
		AG_SETFLAGS(*(int *)p, (int)binding->data.bitmask, v);
		break;
	case AG_WIDGET_FLAG8:
		AG_SETFLAGS(*(Uint8 *)p, (Uint8)binding->data.bitmask, v);
		break;
	case AG_WIDGET_FLAG16:
		AG_SETFLAGS(*(Uint16 *)p, (Uint16)binding->data.bitmask, v);
		break;
	case AG_WIDGET_FLAG32:
		AG_SETFLAGS(*(Uint32 *)p, (Uint32)binding->data.bitmask, v);
		break;
	}
}

static void
mousemotion(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_WidgetBinding *binding;
	int x = AG_INT(1);
	int y = AG_INT(2);
	void *pState;

	if (AG_WidgetDisabled(bu))
		return;

	binding = AG_WidgetGetBinding(bu, "state", &pState);
	if (!AG_WidgetRelativeArea(bu, x, y)) {
		if ((bu->flags & AG_BUTTON_STICKY) == 0 &&
		    GetState(bu, binding, pState) == 1) {
			SetState(binding, pState, 0);
			AG_WidgetBindingChanged(binding);
		}
		if (bu->flags & AG_BUTTON_MOUSEOVER) {
			bu->flags &= ~(AG_BUTTON_MOUSEOVER);
			AG_PostEvent(NULL, bu, "button-mouseoverlap", "%i", 0);
		}
	} else {
		bu->flags |= AG_BUTTON_MOUSEOVER;
		AG_PostEvent(NULL, bu, "button-mouseoverlap", "%i", 1);
	}
	AG_WidgetUnlockBinding(binding);
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	int button = AG_INT(1);
	AG_WidgetBinding *binding;
	void *pState;
	int newState;
	
	if (AG_WidgetDisabled(bu))
		return;

	AG_WidgetFocus(bu);

	if (button != SDL_BUTTON_LEFT)
		return;
	
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	if (!(bu->flags & AG_BUTTON_STICKY)) {
		SetState(binding, pState, 1);
	} else {
		newState = !GetState(bu, binding, pState);
		SetState(binding, pState, newState);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", newState);
	}
	AG_WidgetBindingChanged(binding);
	AG_WidgetUnlockBinding(binding);

	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_ReplaceTimeout(bu, &bu->delay_to, agMouseSpinDelay);
	}
}

static void
mousebuttonup(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	int button = AG_INT(1);
	AG_WidgetBinding *binding;
	void *pState;
	int x = AG_INT(2);
	int y = AG_INT(3);
		
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_DelTimeout(bu, &bu->delay_to);
	}
	
	if (AG_WidgetDisabled(bu) ||
	    x < 0 || y < 0 ||
	    x > WIDGET(bu)->w || y > WIDGET(bu)->h) {
		return;
	}
	
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	if (GetState(bu, binding, pState) && button == SDL_BUTTON_LEFT &&
	    !(bu->flags & AG_BUTTON_STICKY)) {
	    	SetState(binding, pState, 0);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", 0);
		AG_WidgetBindingChanged(binding);
	}
	AG_WidgetUnlockBinding(binding);
}

static void
keydown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_WidgetBinding *binding;
	void *pState;
	int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu))
		return;
	if (keysym != SDLK_RETURN &&		/* TODO configurable */
	    keysym != SDLK_SPACE) {
		return;
	}
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	SetState(binding, pState, 1);
	AG_PostEvent(NULL, bu, "button-pushed", "%i", 1);

	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_ReplaceTimeout(bu, &bu->delay_to, 800);
	}
	AG_WidgetUnlockBinding(binding);
}

static void
keyup(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_WidgetBinding *binding;
	void *pState;
	int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu)) {
		return;
	}
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->delay_to);
		AG_DelTimeout(bu, &bu->repeat_to);
	}
	if (keysym != SDLK_RETURN &&		/* TODO configurable */
	    keysym != SDLK_SPACE) {
		return;
	}
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	SetState(binding, pState, 0);
	AG_WidgetUnlockBinding(binding);
	AG_PostEvent(NULL, bu, "button-pushed", "%i", 0);
}

void
AG_ButtonSetPadding(AG_Button *bu, int lPad, int rPad, int tPad, int bPad)
{
	AG_ObjectLock(bu);
	if (lPad != -1) { bu->lPad = lPad; }
	if (rPad != -1) { bu->rPad = rPad; }
	if (tPad != -1) { bu->tPad = tPad; }
	if (bPad != -1) { bu->bPad = bPad; }
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetFocusable(AG_Button *bu, int focusable)
{
	AG_ObjectLock(bu);
	if (focusable) {
		WIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE;
		WIDGET(bu)->flags &= ~(AG_WIDGET_UNFOCUSED_BUTTONUP);
	} else {
		WIDGET(bu)->flags &= ~(AG_WIDGET_FOCUSABLE);
		WIDGET(bu)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetSticky(AG_Button *bu, int flag)
{
	AG_ObjectLock(bu);
	AG_SETFLAGS(bu->flags, AG_BUTTON_STICKY, flag);
	AG_ObjectUnlock(bu);
}

void
AG_ButtonInvertState(AG_Button *bu, int flag)
{
	AG_ObjectLock(bu);
	AG_SETFLAGS(bu->flags, AG_BUTTON_INVSTATE, flag);
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetJustification(AG_Button *bu, enum ag_text_justify jus)
{
	AG_ObjectLock(bu);
	bu->justify = jus;
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetValign(AG_Button *bu, enum ag_text_valign va)
{
	AG_ObjectLock(bu);
	bu->valign = va;
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSurface(AG_Button *bu, AG_Surface *su)
{
	AG_Surface *suDup = (su != NULL) ? AG_DupSurface(su) : NULL;

	AG_ObjectLock(bu);
	if (bu->surface != -1) {
		AG_WidgetReplaceSurface(bu, bu->surface, suDup);
	} else {
		bu->surface = AG_WidgetMapSurface(bu, suDup);
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSurfaceNODUP(AG_Button *bu, AG_Surface *su)
{
	AG_ObjectLock(bu);
	if (bu->surface != -1) {
		AG_WidgetReplaceSurfaceNODUP(bu, bu->surface, su);
	} else {
		bu->surface = AG_WidgetMapSurfaceNODUP(bu, su);
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetRepeatMode(AG_Button *bu, int repeat)
{
	AG_ObjectLock(bu);
	if (repeat) {
		bu->flags |= (AG_BUTTON_REPEAT);
	} else {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_DelTimeout(bu, &bu->delay_to);
		bu->flags &= ~(AG_BUTTON_REPEAT);
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonTextNODUP(AG_Button *bu, char *text)
{
	AG_ObjectLock(bu);
	Free(bu->text);
	bu->text = text;
	bu->flags |= (AG_BUTTON_REGEN|AG_BUTTON_TEXT_NODUP);
	AG_ObjectUnlock(bu);
}

void
AG_ButtonText(AG_Button *bu, const char *fmt, ...)
{
	va_list args;

	AG_ObjectLock(bu);
	Free(bu->text);
	va_start(args, fmt);
	Vasprintf(&bu->text, fmt, args);
	va_end(args);
	bu->flags |=  AG_BUTTON_REGEN;
	bu->flags &= ~AG_BUTTON_TEXT_NODUP;
	AG_ObjectUnlock(bu);
}

AG_WidgetClass agButtonClass = {
	{
		"Agar(Widget:Button)",
		sizeof(AG_Button),
		{ 0,0 },
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

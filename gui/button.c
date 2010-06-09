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

#include <core/core.h>
#include <core/config.h>

#include "button.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

#include <stdarg.h>

static int  GetState(AG_Button *, AG_Variable *, void *);
static void SetState(AG_Button *, AG_Variable *, void *, int);

AG_Button *
AG_ButtonNew(void *parent, Uint flags, const char *fmt, ...)
{
	va_list ap;
	char *s;
	
	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
	} else {
		s = NULL;
	}
	return AG_ButtonNewS(parent, flags, s);
}

AG_Button *
AG_ButtonNewS(void *parent, Uint flags, const char *label)
{
	AG_Button *bu;
	
	bu = Malloc(sizeof(AG_Button));
	AG_ObjectInit(bu, &agButtonClass);

	if (label != NULL) {
		bu->lbl = AG_LabelNewS(bu, 0, label);
		AG_LabelJustify(bu->lbl, bu->justify);
		AG_LabelValign(bu->lbl, bu->valign);
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

	bu = AG_ButtonNewS(parent, flags, caption);
	ev = AG_SetEvent(bu, "button-pushed", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	return (bu);
}

AG_Button *
AG_ButtonNewInt(void *parent, Uint flags, const char *caption, int *v)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindInt(bu, "state", v);
	return (bu);
}

AG_Button *
AG_ButtonNewUint8(void *parent, Uint flags, const char *caption, Uint8 *v)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindUint8(bu, "state", v);
	return (bu);
}

AG_Button *
AG_ButtonNewUint16(void *parent, Uint flags, const char *caption, Uint16 *v)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindUint16(bu, "state", v);
	return (bu);
}

AG_Button *
AG_ButtonNewUint32(void *parent, Uint flags, const char *caption, Uint32 *v)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindUint32(bu, "state", v);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag(void *parent, Uint flags, const char *caption,
    Uint *p, Uint bitmask)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindFlag(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag8(void *parent, Uint flags, const char *caption,
   Uint8 *p, Uint8 bitmask)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindFlag8(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag16(void *parent, Uint flags, const char *caption,
    Uint16 *p, Uint16 bitmask)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindFlag16(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag32(void *parent, Uint flags, const char *caption,
    Uint32 *p, Uint32 bitmask)
{
	AG_Button *bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindFlag32(bu, "state", p, bitmask);
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

	AG_ScheduleTimeout(bu, &bu->repeat_to, agMouseSpinIval);
	return (0);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	int button = AG_INT(1);
	AG_Variable *binding;
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
	
	binding = AG_GetVariable(bu, "state", &pState);
	if (GetState(bu, binding, pState) && button == AG_MOUSE_LEFT &&
	    !(bu->flags & AG_BUTTON_STICKY)) {
	    	SetState(bu, binding, pState, 0);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", 0);
	}
	AG_UnlockVariable(binding);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	int button = AG_INT(1);
	AG_Variable *binding;
	void *pState;
	int newState;
	
	if (AG_WidgetDisabled(bu))
		return;

	AG_WidgetFocus(bu);

	if (button != AG_MOUSE_LEFT)
		return;
	
	binding = AG_GetVariable(bu, "state", &pState);
	if (!(bu->flags & AG_BUTTON_STICKY)) {
		SetState(bu, binding, pState, 1);
	} else {
		newState = !GetState(bu, binding, pState);
		SetState(bu, binding, pState, newState);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", newState);
	}
	AG_UnlockVariable(binding);

	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_ScheduleTimeout(bu, &bu->delay_to, agMouseSpinDelay);
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_Variable *binding;
	int x = AG_INT(1);
	int y = AG_INT(2);
	void *pState;

	if (AG_WidgetDisabled(bu))
		return;

	binding = AG_GetVariable(bu, "state", &pState);
	if (!AG_WidgetRelativeArea(bu, x, y)) {
		if ((bu->flags & AG_BUTTON_STICKY) == 0 &&
		    GetState(bu, binding, pState) == 1) {
			SetState(bu, binding, pState, 0);
		}
		if (bu->flags & AG_BUTTON_MOUSEOVER) {
			bu->flags &= ~(AG_BUTTON_MOUSEOVER);
			AG_PostEvent(NULL, bu, "button-mouseoverlap", "%i", 0);
		}
	} else {
		bu->flags |= AG_BUTTON_MOUSEOVER;
		AG_PostEvent(NULL, bu, "button-mouseoverlap", "%i", 1);
	}
	AG_UnlockVariable(binding);
}

static void
KeyUp(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_Variable *binding;
	void *pState;
	int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu)) {
		return;
	}
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->delay_to);
		AG_DelTimeout(bu, &bu->repeat_to);
	}
	if (keysym != AG_KEY_RETURN &&		/* TODO AG_Action */
	    keysym != AG_KEY_SPACE) {
		return;
	}
	binding = AG_GetVariable(bu, "state", &pState);
	SetState(bu, binding, pState, 0);
	AG_UnlockVariable(binding);

	if (bu->flags & AG_BUTTON_KEYDOWN) {
		bu->flags &= ~(AG_BUTTON_KEYDOWN);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", 0);
	}
}

static void
KeyDown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_Variable *binding;
	void *pState;
	int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu))
		return;
	if (keysym != AG_KEY_RETURN &&		/* TODO AG_Action */
	    keysym != AG_KEY_SPACE) {
		return;
	}
	binding = AG_GetVariable(bu, "state", &pState);
	SetState(bu, binding, pState, 1);
	AG_PostEvent(NULL, bu, "button-pushed", "%i", 1);
	bu->flags |= AG_BUTTON_KEYDOWN;

	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_ScheduleTimeout(bu, &bu->delay_to, 800);
	}
	AG_UnlockVariable(binding);
}

static void
Init(void *obj)
{
	AG_Button *bu = obj;

	/* TODO replace the unfocused motion flag with a timer */
	WIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_UNFOCUSED_BUTTONUP|
			     AG_WIDGET_TABLE_EMBEDDABLE;

	bu->flags = 0;
	bu->lbl = NULL;
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

	AG_SetEvent(bu, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(bu, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(bu, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(bu, "key-up", KeyUp, NULL);
	AG_SetEvent(bu, "key-down", KeyDown, NULL);

	AG_BindInt(bu, "state", &bu->state);
	AG_RedrawOnChange(bu, 100, "state");

#ifdef AG_DEBUG
	AG_BindInt(bu, "surface", &bu->surface);
	AG_BindUint(bu, "flags", &bu->flags);
	AG_BindInt(bu, "lPad", &bu->lPad);
	AG_BindInt(bu, "rPad", &bu->rPad);
	AG_BindInt(bu, "tPad", &bu->tPad);
	AG_BindInt(bu, "bPad", &bu->bPad);
#endif
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Button *bu = p;
	AG_SizeReq rLbl;
	
	r->w = bu->lPad + bu->rPad;
	r->h = bu->tPad + bu->bPad;

	if (bu->surface != -1) {
		r->w += WSURFACE(bu,bu->surface)->w;
		r->h += WSURFACE(bu,bu->surface)->h;
	} else {
		if (bu->lbl != NULL) {
			AG_WidgetSizeReq(bu->lbl, &rLbl);
			r->w += rLbl.w;
		}
		r->h += agTextFontHeight;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Button *bu = p;
	AG_SizeAlloc aLbl;

	if (a->w < 2 || a->h < 2) {
		return (-1);
	}
	if (bu->lbl != NULL) {
		aLbl.x = bu->lPad;
		aLbl.y = bu->tPad;
		aLbl.w = a->w - (bu->lPad+bu->rPad);
		aLbl.h = a->h - (bu->tPad+bu->bPad);
		AG_WidgetSizeAlloc(bu->lbl, &aLbl);
	}
	return (0);
}

static int
GetState(AG_Button *bu, AG_Variable *binding, void *p)
{
	int v = 0;

	switch (AG_VARIABLE_TYPE(binding)) {
	case AG_VARIABLE_INT:
		v = *(int *)p;
		break;
	case AG_VARIABLE_UINT8:
		v = (int)(*(Uint8 *)p);
		break;
	case AG_VARIABLE_UINT16:
		v = (int)(*(Uint16 *)p);
		break;
	case AG_VARIABLE_UINT32:
		v = (int)(*(Uint32 *)p);
		break;
	case AG_VARIABLE_P_FLAG:
		v = (*(int *)p & (int)binding->info.bitmask);
		break;
	case AG_VARIABLE_P_FLAG8:
		v = (int)(*(Uint8 *)p & (Uint8)binding->info.bitmask);
		break;
	case AG_VARIABLE_P_FLAG16:
		v = (int)(*(Uint16 *)p & (Uint16)binding->info.bitmask);
		break;
	case AG_VARIABLE_P_FLAG32:
		v = (int)(*(Uint32 *)p & (Uint32)binding->info.bitmask);
		break;
	default:
		break;
	}
	if (bu->flags & AG_BUTTON_INVSTATE) {
		v = !v;
	}
	return (v);
}

static void
SetState(AG_Button *bu, AG_Variable *binding, void *p, int v)
{
	switch (AG_VARIABLE_TYPE(binding)) {
	case AG_VARIABLE_INT:
		*(int *)p = v;
		break;
	case AG_VARIABLE_UINT8:
		*(Uint8 *)p = v;
		break;
	case AG_VARIABLE_UINT16:
		*(Uint16 *)p = v;
		break;
	case AG_VARIABLE_UINT32:
		*(Uint32 *)p = v;
		break;
	case AG_VARIABLE_P_FLAG:
		AG_SETFLAGS(*(int *)p, (int)binding->info.bitmask, v);
		break;
	case AG_VARIABLE_P_FLAG8:
		AG_SETFLAGS(*(Uint8 *)p, (Uint8)binding->info.bitmask, v);
		break;
	case AG_VARIABLE_P_FLAG16:
		AG_SETFLAGS(*(Uint16 *)p, (Uint16)binding->info.bitmask, v);
		break;
	case AG_VARIABLE_P_FLAG32:
		AG_SETFLAGS(*(Uint32 *)p, (Uint32)binding->info.bitmask, v);
		break;
	default:
		break;
	}
	AG_Redraw(bu);
}

static void
Draw(void *p)
{
	AG_Button *bu = p;
	AG_Variable *binding;
	void *pState;
	int pressed;
	
	binding = AG_GetVariable(bu, "state", &pState);
	pressed = GetState(bu, binding, pState);
	AG_UnlockVariable(binding);

	STYLE(bu)->ButtonBackground(bu, pressed);

	if (bu->lbl != NULL) {
		AG_WidgetDraw(bu->lbl);
		return;
	}

	if (bu->surface != -1) {
		int x = 0, y = 0, w, h;
		
		w = WSURFACE(bu,bu->surface)->w;
		h = WSURFACE(bu,bu->surface)->h;

		switch (bu->justify) {
		case AG_TEXT_LEFT:	x = bu->lPad;			break;
		case AG_TEXT_CENTER:	x = WIDTH(bu)/2 - w/2;		break;
		case AG_TEXT_RIGHT:	x = WIDTH(bu) - w - bu->rPad;	break;
		}
		switch (bu->valign) {
		case AG_TEXT_TOP:	y = bu->tPad;			break;
		case AG_TEXT_MIDDLE:	y = HEIGHT(bu)/2 - h/2;		break;
		case AG_TEXT_BOTTOM:	y = HEIGHT(bu) - h - bu->bPad;	break;
		}
		STYLE(bu)->ButtonTextOffset(bu, pressed, &x, &y);
		AG_WidgetBlitSurface(bu, bu->surface, x, y);
	}
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
	AG_Redraw(bu);
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
AG_ButtonJustify(AG_Button *bu, enum ag_text_justify jus)
{
	AG_ObjectLock(bu);
	bu->justify = jus;
	if (bu->lbl != NULL) {
		AG_LabelJustify(bu->lbl, jus);
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonValign(AG_Button *bu, enum ag_text_valign va)
{
	AG_ObjectLock(bu);
	bu->valign = va;
	if (bu->lbl != NULL) {
		AG_LabelValign(bu->lbl, va);
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSurface(AG_Button *bu, AG_Surface *su)
{
	AG_Surface *suDup = (su != NULL) ? AG_SurfaceDup(su) : NULL;

	AG_ObjectLock(bu);
	if (bu->lbl != NULL) {
		AG_ObjectDetach(bu->lbl);
		AG_ObjectDestroy(bu->lbl);
		bu->lbl = NULL;
	}
	if (bu->surface != -1) {
		AG_WidgetReplaceSurface(bu, bu->surface, suDup);
	} else {
		bu->surface = AG_WidgetMapSurface(bu, suDup);
	}
	AG_ObjectUnlock(bu);
	AG_Redraw(bu);
}

void
AG_ButtonSurfaceNODUP(AG_Button *bu, AG_Surface *su)
{
	AG_ObjectLock(bu);
	if (bu->lbl != NULL) {
		AG_ObjectDetach(bu->lbl);
		AG_ObjectDestroy(bu->lbl);
		bu->lbl = NULL;
	}
	if (bu->surface != -1) {
		AG_WidgetReplaceSurfaceNODUP(bu, bu->surface, su);
	} else {
		bu->surface = AG_WidgetMapSurfaceNODUP(bu, su);
	}
	AG_ObjectUnlock(bu);
	AG_Redraw(bu);
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

/* Set the label text (C string). */
void
AG_ButtonTextS(AG_Button *bu, const char *label)
{
	AG_ObjectLock(bu);
	if (bu->surface != -1) {
		AG_ButtonSurface(bu, NULL);
	}
	if (bu->lbl == NULL) {
		bu->lbl = AG_LabelNewS(bu, 0, label);
		AG_LabelJustify(bu->lbl, bu->justify);
		AG_LabelValign(bu->lbl, bu->valign);
	} else {
		AG_LabelTextS(bu->lbl, label);
	}
	AG_ObjectUnlock(bu);
	AG_Redraw(bu);
}

/* Set the label text (format string). */
void
AG_ButtonText(AG_Button *bu, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);
	AG_ButtonTextS(bu, s);
}

AG_WidgetClass agButtonClass = {
	{
		"Agar(Widget:Button)",
		sizeof(AG_Button),
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

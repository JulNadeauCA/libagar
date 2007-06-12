/*	$Csoft: button.c,v 1.95 2005/10/01 14:15:38 vedge Exp $	*/

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

#include <core/core.h>
#include <core/view.h>

#include <stdarg.h>

#include "button.h"

#include <gui/window.h>
#include <gui/primitive.h>
#include <gui/label.h>

const AG_WidgetOps agButtonOps = {
	{
		"AG_Widget:AG_Button",
		sizeof(AG_Button),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy XXX */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_ButtonDraw,
	AG_ButtonScale
};

static int GetState(AG_WidgetBinding *, void *);
static void SetState(AG_WidgetBinding *, void *, int);
static void MouseMotion(AG_Event *);
static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);
static void KeyUp(AG_Event *);
static void KeyDown(AG_Event *);

AG_Button *
AG_ButtonNew(void *parent, Uint flags, const char *caption)
{
	AG_Button *btn;

	btn = Malloc(sizeof(AG_Button), M_OBJECT);
	AG_ButtonInit(btn, flags, caption);
	AG_ObjectAttach(parent, btn);

	if (flags & AG_BUTTON_FOCUS) {
		AG_WidgetFocus(btn);
	}
	return (btn);
}

AG_Button *
AG_ButtonAct(void *parent, Uint flags, const char *caption,
    void (*fn)(AG_Event *), const char *fmt, ...)
{
	AG_Button *btn;
	AG_Event *ev;

	btn = Malloc(sizeof(AG_Button), M_OBJECT);
	AG_ButtonInit(btn, flags, caption);
	AG_ObjectAttach(parent, btn);

	ev = AG_SetEvent(btn, "button-pushed", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);

	if (flags & AG_BUTTON_FOCUS) {
		AG_WidgetFocus(btn);
	}
	return (btn);
}

static Uint32
repeat_expire(void *obj, Uint32 ival, void *arg)
{
	AG_PostEvent(NULL, obj, "button-pushed", "%i", 1);
	return (agMouseSpinIval);
}

static Uint32
delay_expire(void *obj, Uint32 ival, void *arg)
{
	AG_Button *bu = obj;

	AG_ReplaceTimeout(bu, &bu->repeat_to, agMouseSpinIval);
	return (0);
}

void
AG_ButtonInit(AG_Button *bu, Uint flags, const char *caption)
{
	SDL_Surface *label;

	/* XXX replace the unfocused motion flag with a timer */
	AG_WidgetInit(bu, "button", &agButtonOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION);
	AG_WidgetBindBool(bu, "state", &bu->state);

	label = (caption == NULL) ? NULL :
	    AG_TextRender(NULL, -1, AG_COLOR(BUTTON_TXT_COLOR), caption);
	AG_WidgetMapSurface(bu, label);

	bu->flags = flags;
	bu->state = 0;
	bu->justify = AG_BUTTON_CENTER;
	bu->lPad = 4;
	bu->rPad = 4;
	bu->tPad = 3;
	bu->bPad = 3;

	AG_SetTimeout(&bu->repeat_to, repeat_expire, NULL, 0);
	AG_SetTimeout(&bu->delay_to, delay_expire, NULL, 0);

	AG_SetEvent(bu, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(bu, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(bu, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(bu, "window-keyup", KeyUp, NULL);
	AG_SetEvent(bu, "window-keydown", KeyDown, NULL);

	if (flags & AG_BUTTON_HFILL) { AGWIDGET(bu)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_BUTTON_VFILL) { AGWIDGET(bu)->flags |= AG_WIDGET_VFILL; }
}

void
AG_ButtonScale(void *p, int w, int h)
{
	AG_Button *bu = p;
	SDL_Surface *label = AGWIDGET_SURFACE(bu,0);

	if (w == -1 && h == -1) {
		if (label != NULL) {
			AGWIDGET(bu)->w = label->w + bu->lPad + bu->rPad;
			AGWIDGET(bu)->h = label->h + bu->tPad + bu->bPad;
		} else {
			AGWIDGET(bu)->w = 1;
			AGWIDGET(bu)->h = 1;
		}
	}
}

void
AG_ButtonDraw(void *p)
{
	AG_Button *bu = p;
	AG_WidgetBinding *binding;
	void *pState;
	SDL_Surface *label = AGWIDGET_SURFACE(bu,0);
	int x = 0, y = 0;
	int pressed;
	
	if (AGWIDGET(bu)->w < 8 || AGWIDGET(bu)->h < 8)
		return;

	binding = AG_WidgetGetBinding(bu, "state", &pState);
	pressed = GetState(binding, pState);
	AG_WidgetUnlockBinding(binding);

	if (bu->flags & AG_BUTTON_DISABLED) {
		agPrim.box_dithered(bu,
		    0, 0,
		    AGWIDGET(bu)->w, AGWIDGET(bu)->h,
		    pressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR),
		    AG_COLOR(DISABLED_COLOR));
	} else {
		agPrim.box(bu,
		    0, 0,
		    AGWIDGET(bu)->w, AGWIDGET(bu)->h,
		    pressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR));
	}

	if (label != NULL) {
		switch (bu->justify) {
		case AG_BUTTON_LEFT:
			x = bu->lPad;
			break;
		case AG_BUTTON_CENTER:
			x = AGWIDGET(bu)->w/2 - label->w/2;
			break;
		case AG_BUTTON_RIGHT:
			x = AGWIDGET(bu)->w - label->w - bu->rPad;
			break;
		}
		y = ((AGWIDGET(bu)->h - label->h)/2) - 1;	/* Middle */

		if (pressed) {
			x++;
			y++;
		}
		AG_WidgetBlitSurface(bu, 0, x, y);
	}
}

static int
GetState(AG_WidgetBinding *binding, void *p)
{
	switch (binding->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
		return *(int *)p;
	case AG_WIDGET_UINT8:
		return (int)(*(Uint8 *)p);
	case AG_WIDGET_UINT16:
		return (int)(*(Uint16 *)p);
	case AG_WIDGET_UINT32:
		return (int)(*(Uint32 *)p);
	case AG_WIDGET_FLAG:
		return (*(int *)p & (int)binding->bitmask);
	case AG_WIDGET_FLAG8:
		return (int)(*(Uint8 *)p & (Uint8)binding->bitmask);
	case AG_WIDGET_FLAG16:
		return (int)(*(Uint16 *)p & (Uint16)binding->bitmask);
	case AG_WIDGET_FLAG32:
		return (int)(*(Uint32 *)p & (Uint32)binding->bitmask);
	}
	return (-1);
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
		{
			int *state = (int *)p;
			if (*state & (int)binding->bitmask) {
				*state &= ~(int)binding->bitmask;
			} else {
				*state |= (int)binding->bitmask;
			}
		}
		break;
	case AG_WIDGET_FLAG8:
		{
			Uint8 *state = (Uint8 *)p;
			if (*state & (Uint8)binding->bitmask) {
				*state &= ~(Uint8)binding->bitmask;
			} else {
				*state |= (Uint8)binding->bitmask;
			}
		}
		break;
	case AG_WIDGET_FLAG16:
		{
			Uint16 *state = (Uint16 *)p;
			if (*state & (Uint16)binding->bitmask) {
				*state &= ~(Uint16)binding->bitmask;
			} else {
				*state |= (Uint16)binding->bitmask;
			}
		}
		break;
	case AG_WIDGET_FLAG32:
		{
			Uint32 *state = (Uint32 *)p;
			if (*state & (Uint32)binding->bitmask) {
				*state &= ~(Uint32)binding->bitmask;
			} else {
				*state |= (Uint32)binding->bitmask;
			}
		}
		break;
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_WidgetBinding *binding;
	int x = AG_INT(1);
	int y = AG_INT(2);
	void *pState;

	if (bu->flags & AG_BUTTON_DISABLED)
		return;

	binding = AG_WidgetGetBinding(bu, "state", &pState);
	if (!AG_WidgetRelativeArea(bu, x, y)) {
		if ((bu->flags & AG_BUTTON_STICKY) == 0 &&
		    GetState(binding, pState) == 1) {
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
	AG_WidgetRedraw(bu);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	int button = AG_INT(1);
	AG_WidgetBinding *binding;
	void *pState;
	int newState;
	
	if (bu->flags & AG_BUTTON_DISABLED)
		return;

	AG_WidgetFocus(bu);
	AG_WidgetRedraw(bu);

	if (button != SDL_BUTTON_LEFT)
		return;
	
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	if (!(bu->flags & AG_BUTTON_STICKY)) {
		SetState(binding, pState, 1);
	} else {
		newState = !GetState(binding, pState);
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
MouseButtonUp(AG_Event *event)
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
	
	if ((bu->flags & AG_BUTTON_DISABLED) ||
	    x < 0 || y < 0 ||
	    x > AGWIDGET(bu)->w || y > AGWIDGET(bu)->h) {
		return;
	}
	
	binding = AG_WidgetGetBinding(bu, "state", &pState);
	if (GetState(binding, pState) && button == SDL_BUTTON_LEFT &&
	    !(bu->flags & AG_BUTTON_STICKY)) {
	    	SetState(binding, pState, 0);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", 0);
		AG_WidgetBindingChanged(binding);
	}
	AG_WidgetUnlockBinding(binding);
	AG_WidgetRedraw(bu);
}

static void
KeyDown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_WidgetBinding *binding;
	void *pState;
	int keysym = AG_INT(1);
	
	if (bu->flags & AG_BUTTON_DISABLED)
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
	AG_WidgetRedraw(bu);
}

static void
KeyUp(AG_Event *event)
{
	AG_Button *bu = AG_SELF();
	AG_WidgetBinding *binding;
	void *pState;
	int keysym = AG_INT(1);
	
	if (bu->flags & AG_BUTTON_DISABLED) {
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
	AG_WidgetRedraw(bu);
}

void
AG_ButtonEnable(AG_Button *bu)
{
	bu->flags &= ~(AG_BUTTON_DISABLED);
	AG_WidgetRedraw(bu);
}

void
AG_ButtonDisable(AG_Button *bu)
{
	bu->flags |= (AG_BUTTON_DISABLED);
	AG_WidgetRedraw(bu);
}

void
AG_ButtonSetPadding(AG_Button *bu, int lPad, int rPad, int tPad, int bPad)
{
	if (lPad != -1) { bu->lPad = lPad; }
	if (rPad != -1) { bu->rPad = rPad; }
	if (tPad != -1) { bu->tPad = tPad; }
	if (bPad != -1) { bu->bPad = bPad; }
	AG_WidgetRedraw(bu);
}

void
AG_ButtonSetFocusable(AG_Button *bu, int focusable)
{
	if (focusable) {
		AGWIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE;
		AGWIDGET(bu)->flags &= ~(AG_WIDGET_UNFOCUSED_BUTTONUP);
	} else {
		AGWIDGET(bu)->flags &= ~(AG_WIDGET_FOCUSABLE);
		AGWIDGET(bu)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;
	}
}

void
AG_ButtonSetSticky(AG_Button *bu, int sticky)
{
	if (sticky) {
		bu->flags |= (AG_BUTTON_STICKY);
	} else {
		bu->flags &= ~(AG_BUTTON_STICKY);
	}
}

void
AG_ButtonSetJustification(AG_Button *bu, enum ag_button_justify jus)
{
	bu->justify = jus;
	AG_WidgetRedraw(bu);
}

void
AG_ButtonSetSurface(AG_Button *bu, SDL_Surface *su)
{
	AG_WidgetReplaceSurface(bu, 0, su);
	AG_WidgetRedraw(bu);
}

void
AG_ButtonSetRepeatMode(AG_Button *bu, int repeat)
{
	if (repeat) {
		bu->flags |= (AG_BUTTON_REPEAT);
	} else {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_DelTimeout(bu, &bu->delay_to);
		bu->flags &= ~(AG_BUTTON_REPEAT);
	}
}

void
AG_ButtonPrintf(AG_Button *bu, const char *fmt, ...)
{
	char buf[AG_LABEL_MAX];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	AG_WidgetReplaceSurface(bu, 0,
	    AG_TextRender(NULL, -1, AG_COLOR(BUTTON_TXT_COLOR), buf));
	
	AG_WidgetRedraw(bu);
}


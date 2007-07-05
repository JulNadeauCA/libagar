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

#include "button.h"

#include "window.h"
#include "primitive.h"
#include "label.h"

#include <stdarg.h>

const AG_WidgetOps agButtonOps = {
	{
		"AG_Widget:AG_Button",
		sizeof(AG_Button),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_ButtonDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_ButtonDraw,
	AG_ButtonScale
};

static int GetState(AG_WidgetBinding *, void *);
static void SetState(AG_WidgetBinding *, void *, int);
static void mousemotion(AG_Event *);
static void mousebuttonup(AG_Event *);
static void mousebuttondown(AG_Event *);
static void keyup(AG_Event *);
static void keydown(AG_Event *);

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
	/* XXX replace the unfocused motion flag with a timer */
	AG_WidgetInit(bu, &agButtonOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION);
	AG_WidgetBindBool(bu, "state", &bu->state);

	bu->text = (caption != NULL) ? Strdup(caption) : NULL;
	bu->surface = -1;

	bu->flags = flags;
	bu->state = 0;
	bu->justify = AG_BUTTON_CENTER;
	bu->lPad = 4;
	bu->rPad = 4;
	bu->tPad = 3;
	bu->bPad = 3;

	AG_SetTimeout(&bu->repeat_to, repeat_expire, NULL, 0);
	AG_SetTimeout(&bu->delay_to, delay_expire, NULL, 0);

	AG_SetEvent(bu, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(bu, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(bu, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(bu, "window-keyup", keyup, NULL);
	AG_SetEvent(bu, "window-keydown", keydown, NULL);

	if (flags & AG_BUTTON_HFILL) { AGWIDGET(bu)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_BUTTON_VFILL) { AGWIDGET(bu)->flags |= AG_WIDGET_VFILL; }
}

void
AG_ButtonDestroy(void *p)
{
	AG_Button *bu = p;
	SDL_Surface *su;

	if ((bu->flags & AG_BUTTON_TEXT_NODUP) == 0) {
		Free(bu->text,0);
	}
	AG_WidgetDestroy(bu);
}

void
AG_ButtonScale(void *p, int w, int h)
{
	AG_Button *bu = p;
	int wLbl, hLbl;

	if (w == -1 && h == -1) {
		if (bu->surface != -1) {
			wLbl = AGWIDGET_SURFACE(bu,bu->surface)->w;
			hLbl = AGWIDGET_SURFACE(bu,bu->surface)->h;
		} else {
			if (bu->text != NULL && bu->text[0] != '\0') {
				AG_TextPrescale(bu->text, &wLbl, &hLbl);
			} else {
				wLbl = 0;
				hLbl = agTextFontHeight;
			}
		}
		AGWIDGET(bu)->w = wLbl + bu->lPad + bu->rPad;
		AGWIDGET(bu)->h = hLbl + bu->tPad + bu->bPad;
	}
}

void
AG_ButtonDraw(void *p)
{
	AG_Button *bu = p;
	AG_WidgetBinding *binding;
	void *pState;
	int x = 0, y = 0;
	int pressed;
	int wLbl;
	
	if (AGWIDGET(bu)->w < 8 || AGWIDGET(bu)->h < 8)
		return;

	binding = AG_WidgetGetBinding(bu, "state", &pState);
	pressed = GetState(binding, pState);
	AG_WidgetUnlockBinding(binding);

	if (AG_WidgetEnabled(bu)) {
		agPrim.box(bu,
		    0, 0,
		    AGWIDGET(bu)->w, AGWIDGET(bu)->h,
		    pressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR));
	} else {
		agPrim.box_dithered(bu,
		    0, 0,
		    AGWIDGET(bu)->w, AGWIDGET(bu)->h,
		    pressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR),
		    AG_COLOR(DISABLED_COLOR));
	}

	if (bu->text != NULL && bu->text[0] != '\0') {
		if (bu->surface == -1) {
			bu->surface = AG_WidgetMapSurface(bu,
			    AG_TextRender(NULL, -1, AG_COLOR(BUTTON_TXT_COLOR),
			    bu->text));
		} else if (bu->flags & AG_BUTTON_REGEN) {
			AG_WidgetReplaceSurface(bu, bu->surface,
			    AG_TextRender(NULL, -1, AG_COLOR(BUTTON_TXT_COLOR),
			    bu->text));
		}
	}
	wLbl = AGWIDGET_SURFACE(bu,bu->surface)->w;

	/* XXX TODO move this code someplace else */
	switch (bu->justify) {
	case AG_BUTTON_LEFT:
		x = bu->lPad;
		break;
	case AG_BUTTON_CENTER:
		x = AGWIDGET(bu)->w/2 - wLbl/2;
		break;
	case AG_BUTTON_RIGHT:
		x = AGWIDGET(bu)->w - wLbl - bu->rPad;
		break;
	}
	y = ((AGWIDGET(bu)->h - AGWIDGET_SURFACE(bu,bu->surface)->h)/2) - 1;

	if (pressed) {
		x++;
		y++;
	}
	AG_WidgetBlitSurface(bu, bu->surface, x, y);
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
	if (lPad != -1) { bu->lPad = lPad; }
	if (rPad != -1) { bu->rPad = rPad; }
	if (tPad != -1) { bu->tPad = tPad; }
	if (bPad != -1) { bu->bPad = bPad; }
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
}

void
AG_ButtonSurface(AG_Button *bu, SDL_Surface *su)
{
	SDL_Surface *suDup = (su != NULL) ? AG_DupSurface(su) : NULL;

	if (bu->surface != -1) {
		AG_WidgetReplaceSurface(bu, bu->surface, suDup);
	} else {
		bu->surface = AG_WidgetMapSurface(bu, suDup);
	}
}

void
AG_ButtonSurfaceNODUP(AG_Button *bu, SDL_Surface *su)
{
	if (bu->surface != -1) {
		AG_WidgetReplaceSurfaceNODUP(bu, bu->surface, su);
	} else {
		bu->surface = AG_WidgetMapSurfaceNODUP(bu, su);
	}
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
AG_ButtonTextNODUP(AG_Button *bu, char *text)
{
	Free(bu->text,0);
	bu->text = text;
	bu->flags |= (AG_BUTTON_REGEN|AG_BUTTON_TEXT_NODUP);
}

void
AG_ButtonText(AG_Button *bu, const char *fmt, ...)
{
	va_list args;
	
	Free(bu->text,0);
	va_start(args, fmt);
	Vasprintf(&bu->text, fmt, args);
	va_end(args);
	bu->flags |=  AG_BUTTON_REGEN;
	bu->flags &= ~AG_BUTTON_TEXT_NODUP;
}

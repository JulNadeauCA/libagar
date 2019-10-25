/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#ifdef AG_WIDGETS

#include <agar/gui/button.h>
#include <agar/gui/primitive.h>
#include <agar/gui/window.h>

#include <stdarg.h>

static void SetState(AG_Button *_Nonnull, AG_Variable *_Nonnull, void *_Nonnull, int);
static int  GetState(AG_Button *_Nonnull, AG_Variable *_Nonnull, void *_Nonnull);
static int  GetStateGeneral(AG_Button *_Nonnull, AG_Variable *_Nonnull, void *_Nonnull);
static void SetStateGeneral(AG_Button *_Nonnull, AG_Variable *_Nonnull, void *_Nonnull, int);

AG_Button *
AG_ButtonNewInt(void *parent, Uint flags, const char *caption, int *v)
{
	AG_Button *bu;

	bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindInt(bu, "state", v);
	return (bu);
}

AG_Button *
AG_ButtonNewFlag(void *parent, Uint flags, const char *caption,
    Uint *p, Uint bitmask)
{
	AG_Button *bu;

	bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindFlag(bu, "state", p, bitmask);
	return (bu);
}

AG_Button *
AG_ButtonNew(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Button *bu;
	va_list ap;
	char *s;
	
	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
	} else {
		s = NULL;
	}
	bu = AG_ButtonNewS(parent, flags, s);
	free(s);
	return (bu);
}

AG_Button *
AG_ButtonNewFn(void *parent, Uint flags, const char *caption, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;
	va_list ap;

	if (!(flags & AG_BUTTON_NOEXCL)) { flags |= AG_BUTTON_EXCL;  }

	bu = AG_ButtonNewS(parent, flags, caption);
	ev = AG_SetEvent(bu, "button-pushed", fn, NULL);

	va_start(ap, fmt);
	AG_EventGetArgs(ev, fmt, ap);
	va_end(ap);

	return (bu);
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

void
AG_ButtonSetPadding(AG_Button *bu, int lPad, int rPad, int tPad, int bPad)
{
	if (lPad != -1) { bu->lPad = lPad; }
	if (rPad != -1) { bu->rPad = rPad; }
	if (tPad != -1) { bu->tPad = tPad; }
	if (bPad != -1) { bu->bPad = bPad; }

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


#ifdef AG_TIMERS
/* Delay/repeat timer callbacks for AG_BUTTON_REPEAT */
static Uint32
ExpireRepeat(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();

	AG_PostEvent(bu, "button-pushed", "%i", 1);
	return (to->ival);
}
static Uint32
ExpireDelay(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	const int repeatIval = AG_INT(1);

	AG_AddTimer(bu, &bu->repeatTo, repeatIval, ExpireRepeat, NULL);
	return (0);
}
#endif /* AG_TIMERS */

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Variable *bState;
	void *pState;
	const int button = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);

#ifdef AG_TIMERS
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimer(bu, &bu->repeatTo);
		AG_DelTimer(bu, &bu->delayTo);
		return;
	}
#endif
	if (AG_WidgetDisabled(bu) ||
	   !AG_WidgetRelativeArea(bu, x,y))
		return;
	
	bState = AG_GetVariable(bu, "state", &pState);
	if (GetState(bu, bState, pState) &&
	    button == AG_MOUSE_LEFT &&
	    !(bu->flags & AG_BUTTON_STICKY)) {
	    	SetState(bu, bState, pState, 0);
		AG_PostEvent(bu, "button-pushed", "%i", 0);
	}
	AG_UnlockVariable(bState);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	const int button = AG_INT(1);
	AG_Variable *bState;
	void *pState;
	int newState;
	
	if (AG_WidgetDisabled(bu))
		return;

	if (!AG_WidgetIsFocused(bu))
		AG_WidgetFocus(bu);

	if (button != AG_MOUSE_LEFT)
		return;
	
	bState = AG_GetVariable(bu, "state", &pState);
	if (!(bu->flags & AG_BUTTON_STICKY)) {
		SetState(bu, bState, pState, 1);
	} else {
		newState = !GetState(bu, bState, pState);
		SetState(bu, bState, pState, newState);
		AG_PostEvent(bu, "button-pushed", "%i", newState);
	}
	AG_UnlockVariable(bState);

#ifdef AG_TIMERS
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimer(bu, &bu->repeatTo);
		AG_PostEvent(bu, "button-pushed", "%i", 1);
		AG_AddTimer(bu, &bu->delayTo, agMouseSpinDelay,
		    ExpireDelay, "%i", agMouseSpinIval);
	}
#endif
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Variable *bState;
	void *pState;
	const int x = AG_INT(1);
	const int y = AG_INT(2);

	if (AG_WidgetDisabled(bu))
		return;

	bState = AG_GetVariable(bu, "state", &pState);
	if (!AG_WidgetRelativeArea(bu, x,y)) {
		if ((bu->flags & AG_BUTTON_STICKY) == 0 &&
		    GetState(bu, bState, pState) == 1) {
			SetState(bu, bState, pState, 0);
		}
	}
	AG_UnlockVariable(bState);
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Variable *bState;
	void *pState;
	const int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu))
		return;
#ifdef AG_TIMERS
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimer(bu, &bu->delayTo);
		AG_DelTimer(bu, &bu->repeatTo);
	}
#endif
	if (keysym != AG_KEY_RETURN &&		/* TODO AG_Action */
	    keysym != AG_KEY_KP_ENTER &&
	    keysym != AG_KEY_SPACE) {
		return;
	}
	bState = AG_GetVariable(bu, "state", &pState);
	SetState(bu, bState, pState, 0);
	AG_UnlockVariable(bState);

	if (bu->flags & AG_BUTTON_KEYDOWN) {
		bu->flags &= ~(AG_BUTTON_KEYDOWN);
		AG_PostEvent(bu, "button-pushed", "%i", 0);
	}
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Variable *bState;
	void *pState;
	const int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu)) {
		return;
	}
	if (keysym != AG_KEY_RETURN &&		/* TODO AG_Action */
	    keysym != AG_KEY_KP_ENTER &&
	    keysym != AG_KEY_SPACE) {
		return;
	}
	bState = AG_GetVariable(bu, "state", &pState);
	SetState(bu, bState, pState, 1);
	AG_PostEvent(bu, "button-pushed", "%i", 1);
	bu->flags |= AG_BUTTON_KEYDOWN;
#ifdef AG_TIMERS
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimer(bu, &bu->repeatTo);
		AG_AddTimer(bu, &bu->delayTo, agKbdDelay,
		    ExpireDelay, "%i", agKbdRepeat);
	}
#endif
	AG_UnlockVariable(bState);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();

	if (bu->flags & AG_BUTTON_SET) {
		AG_Variable *bState;
		void *pState;

		bState = AG_GetVariable(bu, "state", &pState);
		SetState(bu, bState, pState, 1);
		AG_UnlockVariable(bState);
	}
	if ((bu->flags & AG_BUTTON_EXCL) == 0)
		AG_RedrawOnChange(bu, 100, "state");
}

static void
Init(void *_Nonnull obj)
{
	AG_Button *bu = obj;

	WIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE |
	                     AG_WIDGET_UNFOCUSED_MOTION |
			     AG_WIDGET_UNFOCUSED_BUTTONUP |
			     AG_WIDGET_TABLE_EMBEDDABLE |
			     AG_WIDGET_USE_TEXT |
			     AG_WIDGET_USE_MOUSEOVER;
	bu->state = 0;
	bu->surface = -1;
	bu->lbl = NULL;
	bu->justify = AG_TEXT_CENTER;
	bu->valign = AG_TEXT_MIDDLE;
	bu->flags = 0;
	bu->lPad = 4;
	bu->rPad = 4;
	bu->tPad = 3;
	bu->bPad = 3;
#ifdef AG_TIMERS
	AG_InitTimer(&bu->delayTo, "delay", 0);
	AG_InitTimer(&bu->repeatTo, "repeat", 0);
#endif
	AG_AddEvent(bu, "widget-shown", OnShow, NULL);
	AG_SetEvent(bu, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(bu, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(bu, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(bu, "key-up", KeyUp, NULL);
	AG_SetEvent(bu, "key-down", KeyDown, NULL);

	AG_BindInt(bu, "state", &bu->state);
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
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
		r->h += WFONT(bu)->height;
	}
}

static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
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

static void
SetState(AG_Button *_Nonnull bu, AG_Variable *_Nonnull bState, void *_Nonnull p,
    int v)
{
	switch (AG_VARIABLE_TYPE(bState)) {
	case AG_VARIABLE_INT:
		*(int *)p = v;
		break;
	case AG_VARIABLE_UINT:
		*(Uint *)p = v;
		break;
	case AG_VARIABLE_P_FLAG:
		AG_SETFLAGS(*(Uint *)p, bState->info.bitmask.u, v);
		break;
	default:
		SetStateGeneral(bu, bState, p, v);
		break;
	}
	AG_Redraw(bu);
}

static void
Draw(void *_Nonnull p)
{
	AG_Button *bu = p;
	AG_Variable *bState;
	void *pState;
	AG_Label *lbl;
	AG_Rect rd;
	int pressed, surface;
	
	bState = AG_GetVariable(bu, "state", &pState);
	pressed = GetState(bu, bState, pState);
	AG_UnlockVariable(bState);

	rd.x = 0;
	rd.y = 0;
	rd.w = WIDTH(bu);
	rd.h = HEIGHT(bu);

	if (AG_WidgetEnabled(bu)) {
		AG_DrawBox(bu, &rd, pressed ? -1 : 1,
		    &WCOLOR(bu,0));
	} else {
		AG_DrawBoxDisabled(bu, &rd, pressed ? -1 : 1,
		    &WCOLOR_DEF(bu,0),
		    &WCOLOR_DIS(bu,0));
	}

	if ((lbl = bu->lbl) != NULL) {
		if (pressed) {
			WIDGET(lbl)->rView.x1++;
			WIDGET(lbl)->rView.y1++;
		}

		AG_WidgetDraw(lbl);

		if (pressed) {
			WIDGET(lbl)->rView.x1--;
			WIDGET(lbl)->rView.y1--;
		}
	} else if ((surface = bu->surface) != -1) {
		const int w = WSURFACE(bu,surface)->w;
		const int h = WSURFACE(bu,surface)->h;
		int x=0, y=0;

		switch (bu->justify) {
		case AG_TEXT_LEFT:    x = bu->lPad;                    break;
		case AG_TEXT_CENTER:  x = (WIDTH(bu) >> 1)-(w >> 1);   break;
		case AG_TEXT_RIGHT:   x = WIDTH(bu) - w - bu->rPad;    break;
		}
		switch (bu->valign) {
		case AG_TEXT_TOP:     y = bu->tPad;                    break;
		case AG_TEXT_MIDDLE:  y = (HEIGHT(bu) >> 1)-(h >> 1);  break;
		case AG_TEXT_BOTTOM:  y = HEIGHT(bu) - h - bu->bPad;   break;
		}
		if (pressed) {
			x++;
			y++;
		}
		AG_WidgetBlitSurface(bu, surface, x,y);
	}
}

static int
GetState(AG_Button *_Nonnull bu, AG_Variable *_Nonnull bState, void *_Nonnull p)
{
	int v;

	switch (AG_VARIABLE_TYPE(bState)) {
	case AG_VARIABLE_INT:
		v = *(int *)p;
		break;
	case AG_VARIABLE_UINT:
		v = *(Uint *)p;
		break;
	case AG_VARIABLE_P_FLAG:
		v = (int)(*(Uint *)p & bState->info.bitmask.u);
		break;
	default:
		v = GetStateGeneral(bu, bState, p);
		break;
	}
	if (bu->flags & AG_BUTTON_INVSTATE) {
		v = !v;
	}
	return (v);
}
static int
GetStateGeneral(AG_Button *_Nonnull bu, AG_Variable *_Nonnull bState,
    void *_Nonnull p)
{
	switch (AG_VARIABLE_TYPE(bState)) {
	case AG_VARIABLE_UINT8:
		return (int)(*(Uint8 *)p);
	case AG_VARIABLE_UINT16:
		return (int)(*(Uint16 *)p);
	case AG_VARIABLE_UINT32:
		return (int)(*(Uint32 *)p);
	case AG_VARIABLE_P_FLAG8:
		return (int)(*(Uint8 *)p & bState->info.bitmask.u8);
	case AG_VARIABLE_P_FLAG16:
		return (int)(*(Uint16 *)p & bState->info.bitmask.u16);
	case AG_VARIABLE_P_FLAG32:
		return (int)(*(Uint32 *)p & bState->info.bitmask.u32);
	default:
		break;
	}
	return (0);
}

static void
SetStateGeneral(AG_Button *_Nonnull bu, AG_Variable *_Nonnull bState,
    void *_Nonnull p, int v)
{
	switch (AG_VARIABLE_TYPE(bState)) {
	case AG_VARIABLE_UINT8:
		*(Uint8 *)p = v;
		break;
	case AG_VARIABLE_UINT16:
		*(Uint16 *)p = v;
		break;
	case AG_VARIABLE_UINT32:
		*(Uint32 *)p = v;
		break;
	case AG_VARIABLE_P_FLAG8:
		AG_SETFLAGS(*(Uint8 *)p, bState->info.bitmask.u8, v);
		break;
	case AG_VARIABLE_P_FLAG16:
		AG_SETFLAGS(*(Uint16 *)p, bState->info.bitmask.u16, v);
		break;
	case AG_VARIABLE_P_FLAG32:
		AG_SETFLAGS(*(Uint32 *)p, bState->info.bitmask.u32, v);
		break;
	}
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
	if (bu->lbl) {
		AG_LabelValign(bu->lbl, va);
	}
	AG_ObjectUnlock(bu);
}

void
AG_ButtonSurface(AG_Button *bu, const AG_Surface *su)
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
#ifdef AG_TIMERS
		AG_DelTimer(bu, &bu->repeatTo);
		AG_DelTimer(bu, &bu->delayTo);
#endif
		bu->flags &= ~(AG_BUTTON_REPEAT);
	}
	AG_ObjectUnlock(bu);
}

/* Set the label text (format string). */
void
AG_ButtonText(AG_Button *bu, const char *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_ButtonTextS(bu, s);

	free(s);
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

AG_WidgetClass agButtonClass = {
	{
		"Agar(Widget:Button)",
		sizeof(AG_Button),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */

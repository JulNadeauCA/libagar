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
 * Push-button widget. It can display a text label or an image.
 * It can trigger events or be connected to a boolean variable.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/button.h>
#include <agar/gui/primitive.h>
#include <agar/gui/window.h>

#include <stdarg.h>

static void SetState(AG_Button *_Nonnull, AG_Variable *_Nonnull, void *_Nonnull, int);
static int  GetState(AG_Button *_Nonnull, const AG_Variable *_Nonnull, void *_Nonnull);
static int  GetStateGeneral(AG_Button *_Nonnull, const AG_Variable *_Nonnull, void *_Nonnull);
static void SetStateGeneral(AG_Button *_Nonnull, AG_Variable *_Nonnull, void *_Nonnull, int);

/* Create a new button bound to an integer (representing a boolean). */
AG_Button *
AG_ButtonNewInt(void *parent, Uint flags, const char *caption, int *v)
{
	AG_Button *bu;

	bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindInt(bu, "state", v);
	return (bu);
}

/* Create a new button bound to one or more bits in a natural integer. */
AG_Button *
AG_ButtonNewFlag(void *parent, Uint flags, const char *caption,
    Uint *p, Uint bitmask)
{
	AG_Button *bu;

	bu = AG_ButtonNewS(parent, flags, caption);
	AG_BindFlag(bu, "state", p, bitmask);
	return (bu);
}

/* Create a new button with the given text label. */
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

/* Create a new button and connect "button-pushed" to the given routine. */
AG_Button *
AG_ButtonNewFn(void *parent, Uint flags, const char *caption, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;
	va_list ap;

	/*
	 * Make EXCL the default behavior in this constructor since it's less
	 * common for function-calling buttons to also have a boolean binding.
	 */
	if (!(flags & AG_BUTTON_NOEXCL)) { flags |= AG_BUTTON_EXCL;  }

	bu = AG_ButtonNewS(parent, flags, caption);
	ev = AG_SetEvent(bu, "button-pushed", fn, NULL);

	va_start(ap, fmt);
	AG_EventGetArgs(ev, fmt, ap);
	va_end(ap);

	return (bu);
}

static __inline__ void
SetFocusable(AG_Button *_Nonnull bu, int focusable)
{
	if (focusable) {
		WIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE;
		WIDGET(bu)->flags &= ~(AG_WIDGET_UNFOCUSED_BUTTONUP);
	} else {
		WIDGET(bu)->flags &= ~(AG_WIDGET_FOCUSABLE);
		WIDGET(bu)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;
	}
}

/* Create a new button with the given text label. */
AG_Button *
AG_ButtonNewS(void *parent, Uint flags, const char *label)
{
	AG_Button *bu;
	
	bu = Malloc(sizeof(AG_Button));
	AG_ObjectInit(bu, &agButtonClass);

	if (label != NULL) {
		bu->label = TryStrdup(label);
	}
	bu->flags |= flags;

	if (flags & AG_BUTTON_HFILL) { WIDGET(bu)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_BUTTON_VFILL) { WIDGET(bu)->flags |= AG_WIDGET_VFILL; }
	if (flags & AG_BUTTON_NO_FOCUS) { SetFocusable(bu, 0); }
	if (flags & AG_BUTTON_REPEAT) { AG_ButtonSetRepeatMode(bu, 1); }

	AG_ObjectAttach(parent, bu);
	return (bu);
}

void
AG_ButtonSetFocusable(AG_Button *bu, int enable)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	SetFocusable(bu, enable);

	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetSticky(AG_Button *bu, int flag)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	AG_SETFLAGS(bu->flags, AG_BUTTON_STICKY, flag);

	AG_ObjectUnlock(bu);
}

void
AG_ButtonSetInverted(AG_Button *bu, int flag)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	AG_SETFLAGS(bu->flags, AG_BUTTON_INVERTED, flag);

	AG_ObjectUnlock(bu);
}


/* Delay/repeat timer callbacks for REPEAT option. */
static Uint32
MouseSpinRepeat(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();

	AG_PostEvent(bu, "button-pushed", "%i", 1);
	return (to->ival);
}
static Uint32
MouseSpinDelay(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();

	if (bu->repeat != NULL) {
		AG_AddTimer(bu, &bu->repeat->ivalTo, agMouseSpinIval,
		    MouseSpinRepeat, NULL);
	}
	return (0);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	void *pState;
	const int button = AG_INT(1);

	if (button != AG_MOUSE_LEFT) {
		return;
	}

	bu->flags &= ~(AG_BUTTON_PRESSING);

	if (bu->repeat) {
		AG_DelTimer(bu, &bu->repeat->ivalTo);
		AG_DelTimer(bu, &bu->repeat->delayTo);
	}
	if (AG_WidgetDisabled(bu))
		return;

	if ((bu->flags & AG_BUTTON_STICKY) == 0) {
		AG_Variable *V;

		V = AG_GetVariable(bu, "state", &pState);
		if (GetState(bu, V, pState)) {
		    	SetState(bu, V, pState, 0);
			AG_PostEvent(bu, "button-pushed", "%i", 0);
		}
		AG_UnlockVariable(V);
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	const int button = AG_INT(1);
	AG_Variable *V;
	void *pState;
	int newState;
	
	if (AG_WidgetDisabled(bu))
		return;

	if (!AG_WidgetIsFocused(bu))
		AG_WidgetFocus(bu);

	if (button != AG_MOUSE_LEFT)
		return;

	bu->flags |= AG_BUTTON_PRESSING;

	V = AG_GetVariable(bu, "state", &pState);
	if (bu->flags & AG_BUTTON_STICKY) {
		newState = !GetState(bu, V, pState);
		SetState(bu, V, pState, newState);
		AG_PostEvent(bu, "button-pushed", "%i", newState);
	} else {
		SetState(bu, V, pState, 1);
	}
	AG_UnlockVariable(V);

	if (bu->repeat) {
		AG_DelTimer(bu, &bu->repeat->ivalTo);
		AG_AddTimer(bu, &bu->repeat->delayTo, agMouseSpinDelay,
		    MouseSpinDelay, NULL);
	}
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Variable *V;
	void *pState;
	const int keysym = AG_INT(1);
	
	if (AG_WidgetDisabled(bu))
		return;

	if (bu->repeat) {
		AG_DelTimer(bu, &bu->repeat->delayTo);
		AG_DelTimer(bu, &bu->repeat->ivalTo);
	}
	if (keysym != AG_KEY_RETURN &&		/* TODO AG_Action */
	    keysym != AG_KEY_KP_ENTER &&
	    keysym != AG_KEY_SPACE) {
		return;
	}
	V = AG_GetVariable(bu, "state", &pState);
	SetState(bu, V, pState, 0);
	AG_UnlockVariable(V);

	if (bu->flags & AG_BUTTON_KEYDOWN) {
		bu->flags &= ~(AG_BUTTON_KEYDOWN);
		AG_PostEvent(bu, "button-pushed", "%i", 0);
	}
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();
	AG_Variable *V;
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
	V = AG_GetVariable(bu, "state", &pState);
	SetState(bu, V, pState, 1);
	AG_PostEvent(bu, "button-pushed", "%i", 1);
	AG_UnlockVariable(V);

	bu->flags |= AG_BUTTON_KEYDOWN;

	if (bu->repeat) {
		AG_DelTimer(bu, &bu->repeat->ivalTo);
		AG_AddTimer(bu, &bu->repeat->delayTo, agMouseSpinDelay,
		    MouseSpinDelay, NULL);
	}
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();

	if (bu->flags & AG_BUTTON_SET) {
		AG_Variable *V;
		void *pState;

		V = AG_GetVariable(bu, "state", &pState);
		SetState(bu, V, pState, 1);
		AG_UnlockVariable(V);
	}
	if ((bu->flags & AG_BUTTON_EXCL) == 0)
		AG_RedrawOnChange(bu, 250, "state");
}

static void
StyleChanged(AG_Event *_Nonnull event)
{
	AG_Button *bu = AG_BUTTON_SELF();

	if (bu->surfaceLbl != -1) {
		AG_WidgetUnmapSurface(bu, bu->surfaceLbl);
		bu->surfaceLbl = -1;
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Button *bu = obj;

	WIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE |
	                     AG_WIDGET_UNFOCUSED_MOTION |
			     AG_WIDGET_UNFOCUSED_BUTTONUP |
			     AG_WIDGET_USE_TEXT |
			     AG_WIDGET_USE_MOUSEOVER;

	bu->flags = 0;
	bu->label = NULL;
	bu->state = 0;
	bu->surfaceLbl = -1;
	bu->surfaceSrc = -1;
	bu->justify = AG_TEXT_CENTER;
	bu->valign = AG_TEXT_MIDDLE;
	bu->repeat = NULL;

	AG_AddEvent(bu, "widget-shown", OnShow, NULL);
	AG_AddEvent(bu, "font-changed", StyleChanged, NULL);
	AG_AddEvent(bu, "palette-changed", StyleChanged, NULL);

	AG_SetEvent(bu, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(bu, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(bu, "key-up", KeyUp, NULL);
	AG_SetEvent(bu, "key-down", KeyDown, NULL);

	AG_BindInt(bu, "state", &bu->state);
}

static __inline__ void
RenderLabel(AG_Button *_Nonnull bu)
{
	AG_TextColor(&WCOLOR(bu, TEXT_COLOR));
/*	AG_TextBGColor(&WCOLOR(bu, FG_COLOR)); */

	bu->surfaceLbl = AG_WidgetMapSurface(bu, AG_TextRender(bu->label));
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	AG_Button *bu = p;
	
	r->w = WIDGET(bu)->paddingLeft + WIDGET(bu)->paddingRight;
	r->h = WIDGET(bu)->paddingTop + WIDGET(bu)->paddingBottom;

	if (bu->label && bu->label[0] != '\0') {
		if (bu->surfaceLbl == -1) {
			RenderLabel(bu);
		}
		r->w += WSURFACE(bu,bu->surfaceLbl)->w;
		r->h += WSURFACE(bu,bu->surfaceLbl)->h;
	} else if (bu->surfaceSrc != -1) {
		r->w += WSURFACE(bu,bu->surfaceSrc)->w;
		r->h += WSURFACE(bu,bu->surfaceSrc)->h;
	}

	bu->wReq = r->w;
	bu->hReq = r->h;
}

static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
{
	if (a->w < 2 || a->h < 2) {
		return (-1);
	}
	return (0);
}

static void
SetState(AG_Button *bu, AG_Variable *V, void *p, int v)
{
	switch (V->type) {
	case AG_VARIABLE_INT:
	case AG_VARIABLE_P_INT:
		*(int *)p = v;
		break;
	case AG_VARIABLE_UINT:
	case AG_VARIABLE_P_UINT:
		*(Uint *)p = v;
		break;
	case AG_VARIABLE_P_FLAG:
		AG_SETFLAGS(*(Uint *)p, V->info.bitmask.u, v);
		break;
	default:
		SetStateGeneral(bu, V, p, v);
		break;
	}
	AG_Redraw(bu);
}

static void
Draw(void *_Nonnull p)
{
	AG_Button *bu = p;
	AG_Variable *V;
	void *pState;
	int surface, pressed, x=0, y=0;
	
	V = AG_GetVariable(bu, "state", &pState);
	pressed = GetState(bu, V, pState);
	AG_UnlockVariable(V);

	if (pressed) {
		AG_DrawBoxSunk(bu, &WIDGET(bu)->r, &WCOLOR(bu, FG_COLOR));
	} else {
		AG_DrawBoxRaised(bu, &WIDGET(bu)->r, &WCOLOR(bu, FG_COLOR));
	}

	if (bu->surfaceSrc != -1) {
		surface = bu->surfaceSrc;
	} else {                           
		if (bu->surfaceLbl == -1) {
			if (bu->label == NULL) {                /* No label */
				return;
			}
			RenderLabel(bu);
		}
		surface = bu->surfaceLbl;
	}

	switch (bu->justify) {
	case AG_TEXT_LEFT:
		x = WIDGET(bu)->paddingLeft;    
		break;
	case AG_TEXT_CENTER:
		x = (WIDTH(bu) >> 1) - (WSURFACE(bu,surface)->w >> 1);
		break;
	case AG_TEXT_RIGHT:
		x = WIDTH(bu) - WSURFACE(bu,surface)->w -
		    WIDGET(bu)->paddingRight;
		break;
	}
	switch (bu->valign) {
	case AG_TEXT_TOP:
		y = WIDGET(bu)->paddingTop;
		break;
	case AG_TEXT_MIDDLE:
		y = (HEIGHT(bu) >> 1) - (WSURFACE(bu,surface)->h >> 1);
		break;
	case AG_TEXT_BOTTOM:
		y = HEIGHT(bu) - WSURFACE(bu,surface)->h -
		    WIDGET(bu)->paddingBottom;
		break;
	}
	if (pressed) {
		x++;
		y++;
	}

	AG_PushBlendingMode(bu, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	if (AG_WidgetIsFocused(bu)) {
		AG_DrawLineH(bu, 1, WIDTH(bu) - 3 + pressed,     /* x1,x2,y */
		                   HEIGHT(bu) - 3 + pressed,
		             &WCOLOR(bu, SELECTION_COLOR));

		AG_DrawLineV(bu, WIDTH(bu) - 3 + pressed,        /* x,y1,y2 */
		             1, HEIGHT(bu) - 3 + pressed,
		             &WCOLOR(bu, SELECTION_COLOR));
	}

	if (WIDTH(bu) < bu->wReq || HEIGHT(bu) < bu->hReq) {
		AG_PushClipRect(bu, &WIDGET(bu)->r);
		AG_WidgetBlitSurface(bu, surface, x,y);
		AG_PopClipRect(bu);
	} else {
		AG_WidgetBlitSurface(bu, surface, x,y);
	}

	AG_PopBlendingMode(bu);
}

/* Return the current boolean state of the button. */
int
AG_ButtonGetState(AG_Button *bu)
{
	AG_Variable *V;
	void *pState;
	int stateCur;

	V = AG_GetVariable(bu, "state", &pState);
	stateCur = GetState(bu, V, pState);
	AG_UnlockVariable(V);
	return (stateCur);
}

/* Set the boolean state of the button and return the previous state. */
int
AG_ButtonSetState(AG_Button *bu, int stateNew)
{
	AG_Variable *V;
	void *pState;
	int statePrev;

	V = AG_GetVariable(bu, "state", &pState);
	statePrev = GetState(bu, V, pState);
	SetState(bu, V, pState, 1);
	AG_UnlockVariable(V);
	return (statePrev);
}

/* Atomically toggle the boolean state of the button and return the new state. */
int
AG_ButtonToggle(AG_Button *bu)
{
	AG_Variable *V;
	void *pState;
	int statePrev, stateNew;

	V = AG_GetVariable(bu, "state", &pState);
	statePrev = GetState(bu, V, pState);
	stateNew = !statePrev;
	SetState(bu, V, pState, stateNew);
	AG_UnlockVariable(V);
	return (stateNew);
}

static int
GetState(AG_Button *bu, const AG_Variable *V, void *p)
{
	int v;

	switch (V->type) {
	case AG_VARIABLE_INT:
	case AG_VARIABLE_P_INT:
	case AG_VARIABLE_UINT:
	case AG_VARIABLE_P_UINT:
		v = (*(Uint *)p) ? 1 : 0;
		break;
	case AG_VARIABLE_P_FLAG:
		v = (*(Uint *)p & V->info.bitmask.u) ? 1 : 0;
		break;
	default:
		v = GetStateGeneral(bu, V, p) ? 1 : 0;
		break;
	}
	if (bu->flags & AG_BUTTON_INVERTED) {
		v = !v;
	}
	return (v);
}

static int
GetStateGeneral(AG_Button *bu, const AG_Variable *V, void *p)
{
	switch (V->type) {
	case AG_VARIABLE_UINT8:
		return (int)(*(Uint8 *)p);
	case AG_VARIABLE_UINT16:
		return (int)(*(Uint16 *)p);
	case AG_VARIABLE_UINT32:
		return (int)(*(Uint32 *)p);
	case AG_VARIABLE_P_FLAG8:
		return (int)(*(Uint8 *)p & V->info.bitmask.u8);
	case AG_VARIABLE_P_FLAG16:
		return (int)(*(Uint16 *)p & V->info.bitmask.u16);
	case AG_VARIABLE_P_FLAG32:
		return (int)(*(Uint32 *)p & V->info.bitmask.u32);
	default:
		break;
	}
	return (0);
}

static void
SetStateGeneral(AG_Button *bu, AG_Variable *V, void *p, int v)
{
	switch (V->type) {
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
		AG_SETFLAGS(*(Uint8 *)p, V->info.bitmask.u8, v);
		break;
	case AG_VARIABLE_P_FLAG16:
		AG_SETFLAGS(*(Uint16 *)p, V->info.bitmask.u16, v);
		break;
	case AG_VARIABLE_P_FLAG32:
		AG_SETFLAGS(*(Uint32 *)p, V->info.bitmask.u32, v);
		break;
	default:
		break;
	}
}

/* Set horizontal alignment mode. */
void
AG_ButtonJustify(AG_Button *bu, enum ag_text_justify jus)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	bu->justify = jus;
}

/* Set vertical alignment mode. */
void
AG_ButtonValign(AG_Button *bu, enum ag_text_valign va)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	bu->valign = va;
}

/*
 * Set the button label to (a copy of) the given graphics surface.
 */
void
AG_ButtonSurface(AG_Button *bu, const AG_Surface *S)
{
	AG_Surface *Sdup = (S != NULL) ? AG_SurfaceDup(S) : NULL;

	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	if (bu->surfaceSrc != -1) {
		AG_WidgetReplaceSurface(bu, bu->surfaceSrc, Sdup);
	} else {
		bu->surfaceSrc = AG_WidgetMapSurface(bu, Sdup);
	}

	AG_Redraw(bu);
	AG_ObjectUnlock(bu);
}

/*
 * Set the button label to the given graphics surface (potentially unsafe).
 */
void
AG_ButtonSurfaceNODUP(AG_Button *bu, AG_Surface *S)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	if (bu->surfaceSrc != -1) {
		AG_WidgetReplaceSurfaceNODUP(bu, bu->surfaceSrc, S);
	} else {
		bu->surfaceSrc = AG_WidgetMapSurfaceNODUP(bu, S);
	}

	AG_Redraw(bu);
	AG_ObjectUnlock(bu);
}

/* Enable or Disable repeat mode */
void
AG_ButtonSetRepeatMode(AG_Button *bu, int enable)
{
	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	if (enable) {
		if (bu->repeat == NULL) {
			bu->repeat = Malloc(sizeof(AG_ButtonRepeat));
			AG_InitTimer(&bu->repeat->delayTo, "delay", 0);
			AG_InitTimer(&bu->repeat->ivalTo, "ival", 0);
		}
		bu->flags |= AG_BUTTON_REPEAT;
	} else {
		if (bu->repeat != NULL) {
			AG_DelTimer(bu, &bu->repeat->ivalTo);
			AG_DelTimer(bu, &bu->repeat->delayTo);
			free(bu->repeat);
			bu->repeat = NULL;
		}
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
	char *labelDup = TryStrdup(label);

	AG_OBJECT_ISA(bu, "AG_Widget:AG_Button:*");
	AG_ObjectLock(bu);

	if (bu->surfaceLbl != -1) {
		AG_WidgetUnmapSurface(bu, bu->surfaceLbl);
		bu->surfaceLbl = -1;
	}
	Free(bu->label);
	bu->label = labelDup;
	
	AG_Redraw(bu);
	AG_ObjectUnlock(bu);
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

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

#include <agar/gui/checkbox.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>

#include <stdarg.h>
#include <string.h>

static void DrawMouseOver(AG_Checkbox *_Nonnull);

#if AG_MODEL != AG_SMALL
AG_Checkbox *
AG_CheckboxNewFn(void *parent, Uint flags, const char *label, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Checkbox *cb;
	AG_Event *ev;

	cb = AG_CheckboxNewS(parent, flags, label);
	ev = AG_SetEvent(cb, "checkbox-changed", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewInt(void *parent, Uint flags, const char *label, int *pVal)
{
	AG_Checkbox *cb;

	cb = AG_CheckboxNewS(parent, flags, label);
	AG_BindInt(cb, "state", pVal);
	return (cb);
}

/* Create a set of checkboxes for the given set of flags. */
void
AG_CheckboxSetFromFlags(void *parent, Uint flags, Uint *pFlags,
    const AG_FlagDescr *fdSet)
{
	int i;

	for (i = 0; fdSet[i].bitmask != 0; i++) {
		const AG_FlagDescr *fd = &fdSet[i];
		AG_Checkbox *cb;

		cb = AG_CheckboxNewFlag(parent, flags, fd->descr, pFlags,
		    fd->bitmask);
		if (!fd->writeable)
			AG_WidgetDisable(cb);
	}
}

AG_Checkbox *
AG_CheckboxNewFlag(void *parent, Uint flags, const char *label, Uint *pFlags,
    Uint bitmask)
{
	AG_Checkbox *cb;

	cb = AG_CheckboxNewS(parent, flags, label);
	AG_BindFlag(cb, "state", pFlags, bitmask);
	return (cb);
}

AG_Checkbox *
AG_CheckboxNew(void *parent, Uint flags, const char *fmt, ...)
{
	char *s;
	va_list ap;
	AG_Checkbox *cb;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
		cb = AG_CheckboxNewS(parent, flags, s);
		free(s);
	} else {
		cb = AG_CheckboxNewS(parent, flags, NULL);
	}
	return (cb);
}
#endif /* !AG_SMALL */

AG_Checkbox *
AG_CheckboxNewS(void *parent, Uint flags, const char *label)
{
	AG_Checkbox *cb;

	cb = Malloc(sizeof(AG_Checkbox));
	AG_ObjectInit(cb, &agCheckboxClass);
	cb->flags |= flags;

	if (label != NULL) {
		cb->lbl = AG_LabelNewS(cb, 0, label);
		AG_LabelValign(cb->lbl, AG_TEXT_MIDDLE);
	}
	if (flags & AG_CHECKBOX_SET)
		cb->state = 1;
	if (flags & AG_CHECKBOX_INVERT)
		cb->invert = 1;

	AG_ObjectAttach(parent, cb);
	return (cb);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Checkbox *cb = AG_CHECKBOX_SELF();
	const int button = AG_INT(1);

	if (!AG_WidgetEnabled(cb))
		return;

	if (button == AG_MOUSE_LEFT) {
		if (!AG_WidgetIsFocused(cb)) {
			AG_WidgetFocus(cb);
		}
		AG_CheckboxToggle(cb);
	}
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Checkbox *cb = AG_CHECKBOX_SELF();
	const int key = AG_INT(1);
	
	if (!AG_WidgetEnabled(cb))
		return;

	switch (key) {
	case AG_KEY_RETURN:
	case AG_KEY_KP_ENTER:
	case AG_KEY_SPACE:
		AG_CheckboxToggle(cb);
		break;
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Checkbox *cb = obj;

	WIDGET(cb)->flags |= AG_WIDGET_FOCUSABLE |
	                     AG_WIDGET_TABLE_EMBEDDABLE |
			     AG_WIDGET_USE_TEXT |
			     AG_WIDGET_USE_MOUSEOVER;

	cb->flags = 0;
	cb->state = 0;
	cb->spacing = 4;
	cb->invert = 0;
	cb->lbl = NULL;
	
	AG_BindInt(cb, "state", &cb->state);
	AG_RedrawOnChange(cb, 100, "state");
	
	AG_SetEvent(cb, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(cb, "key-down", KeyDown, NULL);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Checkbox *cb = obj;
	AG_Variable *V;
	void *p;
	AG_Rect r;
	int state;

	r.x = 0;
	r.y = 0;
	r.w = WIDTH(cb);
	r.h = HEIGHT(cb);
	AG_PushClipRect(cb, &r); /* XXX */

	V = AG_GetVariable(cb, "state", &p);
	switch (V->type) {
	case AG_VARIABLE_INT:				/* Natural integer */
	case AG_VARIABLE_UINT:
		state = V->data.i;
		AG_UnlockVariable(V);
		break;
	case AG_VARIABLE_P_INT:
	case AG_VARIABLE_P_UINT:
		state = *(int *)p;
		AG_UnlockVariable(V);
		break;
	case AG_VARIABLE_P_FLAG:
		state = *(Uint *)p & V->info.bitmask.u;
		AG_UnlockVariable(V);
		break;
	default:					/* General case */
		AG_UnlockVariable(V);
		state = AG_CheckboxGetState(cb);
		break;
	}

	if (cb->invert)
		state = !state;

	if (WIDGET(cb)->flags & AG_WIDGET_MOUSEOVER)
		DrawMouseOver(cb);

	r.x = 0;
	r.y = 0;
	r.w = WFONT(cb)->height;
	r.h = r.w;

	if (AG_WidgetEnabled(cb)) {
		AG_DrawBox(cb, &r, state ? -1 : 1, &WCOLOR(cb,0));
	} else {
		AG_DrawBoxDisabled(cb, &r, state ? -1 : 1, &WCOLOR(cb,0),
		                                           &WCOLOR_DIS(cb,0));
	}
	if (cb->lbl != NULL) {
		AG_WidgetDraw(cb->lbl);
	}
	AG_PopClipRect(cb);
}

/* Return the checkbox state. */
int
AG_CheckboxGetState(AG_Checkbox *cb)
{
	AG_Variable *bState;
	void *p;
	int rv;

	AG_ObjectLock(cb);
	bState = AG_GetVariable(cb, "state", &p);
	switch (bState->type) {
	case AG_VARIABLE_P_FLAG8:
		rv = *(Uint8 *)p & bState->info.bitmask.u8;
		break;
	case AG_VARIABLE_P_FLAG16:
		rv = *(Uint16 *)p & bState->info.bitmask.u16;
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG32:
		rv = *(Uint32 *)p & bState->info.bitmask.u32;
		break;
#endif
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
		rv = bState->data.u8;
		break;
	case AG_VARIABLE_P_UINT8:
	case AG_VARIABLE_P_SINT8:
		rv = *(Uint8 *)p;
		break;
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
		rv = bState->data.u16;
		break;
	case AG_VARIABLE_P_UINT16:
	case AG_VARIABLE_P_SINT16:
		rv = *(Uint16 *)p;
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		rv = bState->data.u32;
		break;
	case AG_VARIABLE_P_UINT32:
	case AG_VARIABLE_P_SINT32:
		rv = *(Uint32 *)p;
		break;
#endif
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
		rv = bState->data.i;
		break;
	case AG_VARIABLE_P_INT:
	case AG_VARIABLE_P_UINT:
		rv = *(int *)p;
		break;
	case AG_VARIABLE_P_FLAG:
		rv = *(Uint *)p & bState->info.bitmask.u;
		break;
	default:
		rv = 0;
	}
	AG_UnlockVariable(bState);
	AG_ObjectUnlock(cb);
	return (rv);
}

static void
DrawMouseOver(AG_Checkbox *_Nonnull cb)
{
	AG_Rect r;
	AG_Color c;

	r.x = 0;
	r.y = 0;
	r.w = WIDTH(cb);
	r.h = HEIGHT(cb)-1;

	AG_ColorRGBA_8(&c, 255,255,255, 25);
	AG_DrawRectBlended(cb, &r, &c, AG_ALPHA_SRC,
	                               AG_ALPHA_ONE_MINUS_SRC);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Checkbox *cb = obj;
	const int hFont = WFONT(cb)->height;
	AG_SizeReq rLbl;

	r->h = hFont;
	r->w = hFont;
	
	if (cb->lbl != NULL) {
		AG_WidgetSizeReq(cb->lbl, &rLbl);
		r->w += cb->spacing + rLbl.w;
		r->h = MAX(r->h, rLbl.h);
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Checkbox *cb = obj;
	const int hFont = WFONT(cb)->height;
	AG_SizeAlloc aLbl;

	if (a->w < hFont) {
		return (-1);
	}
	if (cb->lbl != NULL) {
		aLbl.x = hFont + cb->spacing;
		aLbl.y = 0;
		aLbl.w = a->w - aLbl.x;
		aLbl.h = a->h;
		AG_WidgetSizeAlloc(cb->lbl, &aLbl);
	}
	return (0);
}

/* Set the checkbox state. */
void
AG_CheckboxSetState(AG_Checkbox *cb, int newState)
{
	AG_Variable *V;
	void *p;

	AG_ObjectLock(cb);
	if (cb->invert) {
		newState = !newState;
	}
	V = AG_GetVariable(cb, "state", &p);
	switch (V->type) {
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
		V->data.i = newState;
		break;
	case AG_VARIABLE_P_INT:
	case AG_VARIABLE_P_UINT:
		*(int *)p = newState;
		break;
	case AG_VARIABLE_P_FLAG:
		AG_SETFLAGS(*(Uint *)p, V->info.bitmask.u, newState);
		break;
	case AG_VARIABLE_P_FLAG8:
		AG_SETFLAGS(*(Uint8 *)p, V->info.bitmask.u8, newState);
		break;
	case AG_VARIABLE_P_FLAG16:
		AG_SETFLAGS(*(Uint16 *)p, V->info.bitmask.u16, newState);
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG32:
		AG_SETFLAGS(*(Uint32 *)p, V->info.bitmask.u32, newState);
		break;
#endif
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
		V->data.u8 = (Uint8)newState;
		break;
	case AG_VARIABLE_P_UINT8:
	case AG_VARIABLE_P_SINT8:
		*(Uint8 *)p = newState;
		break;
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
		V->data.u16 = (Uint16)newState;
		break;
	case AG_VARIABLE_P_UINT16:
	case AG_VARIABLE_P_SINT16:
		*(Uint16 *)p = newState;
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		V->data.u32 = (Uint32)newState;
		break;
	case AG_VARIABLE_P_UINT32:
	case AG_VARIABLE_P_SINT32:
		*(Uint32 *)p = newState;
		break;
#endif
	default:
		break;
	}
	AG_PostEvent(cb, "checkbox-changed", "%i", newState);
	AG_UnlockVariable(V);
	AG_ObjectUnlock(cb);
	AG_Redraw(cb);
}

/* Toggle the checkbox state. */
void
AG_CheckboxToggle(AG_Checkbox *cb)
{
	AG_Variable *V;
	void *p;

	AG_ObjectLock(cb);
	V = AG_GetVariable(cb, "state", &p);
	switch (V->type) {
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
		{
			int old_state = V->data.i;
			V->data.i = !old_state;
			AG_PostEvent(cb, "checkbox-changed", "%i", V->data.i);
		}
		break;
	case AG_VARIABLE_P_INT:
	case AG_VARIABLE_P_UINT:
		{
			int *state = (int *)p;
			*state = !(*state);
			AG_PostEvent(cb, "checkbox-changed", "%i", *state);
		}
		break;
	case AG_VARIABLE_P_FLAG:
		{
			Uint *state = (Uint *)p;
			AG_INVFLAGS(*state, V->info.bitmask.u);
			AG_PostEvent(cb, "checkbox-changed", "%i", (Uint)*state);
		}
		break;
	case AG_VARIABLE_P_FLAG8:
		{
			Uint8 *state = (Uint8 *)p;
			AG_INVFLAGS(*state, V->info.bitmask.u8);
			AG_PostEvent(cb, "checkbox-changed", "%i", (Uint8)*state);
		}
		break;
	case AG_VARIABLE_P_FLAG16:
		{
			Uint16 *state = (Uint16 *)p;
			AG_INVFLAGS(*state, V->info.bitmask.u16);
			AG_PostEvent(cb, "checkbox-changed", "%i", (Uint16)*state);
		}
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG32:
		{
			Uint32 *state = (Uint32 *)p;
			AG_INVFLAGS(*state, V->info.bitmask.u32);
			AG_PostEvent(cb, "checkbox-changed", "%i", (Uint32)*state);
		}
		break;
#endif
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
		{
			Uint8 old_state = V->data.u8;
			V->data.u8 = !old_state;
			AG_PostEvent(cb, "checkbox-changed", "%i", V->data.u8);
		}
		break;
	case AG_VARIABLE_P_UINT8:
	case AG_VARIABLE_P_SINT8:
		{
			Uint8 *state = (Uint8 *)p;
			*state = !(*state);
			AG_PostEvent(cb, "checkbox-changed", "%i", (int)*state);
		}
		break;
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
		{
			Uint16 old_state = V->data.u16;
			V->data.u16 = !old_state;
			AG_PostEvent(cb, "checkbox-changed", "%i", V->data.u16);
		}
		break;
	case AG_VARIABLE_P_UINT16:
	case AG_VARIABLE_P_SINT16:
		{
			Uint16 *state = (Uint16 *)p;
			*state = !(*state);
			AG_PostEvent(cb, "checkbox-changed", "%i", (int)*state);
		}
		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		{
			Uint32 old_state = V->data.u32;
			V->data.u32 = !old_state;
			AG_PostEvent(cb, "checkbox-changed", "%i", (int)V->data.u32);
		}
		break;
	case AG_VARIABLE_P_UINT32:
	case AG_VARIABLE_P_SINT32:
		{
			Uint32 *state = (Uint32 *)p;
			*state = !(*state);
			AG_PostEvent(cb, "checkbox-changed", "%i", (int)*state);
		}
		break;
#endif /* !AG_SMALL */
	default:
		break;
	}
	AG_UnlockVariable(V);
	AG_ObjectUnlock(cb);
	AG_Redraw(cb);
}

AG_WidgetClass agCheckboxClass = {
	{
		"Agar(Widget:Checkbox)",
		sizeof(AG_Checkbox),
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

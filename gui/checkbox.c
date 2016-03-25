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

#include <agar/core/core.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>

#include <stdarg.h>
#include <string.h>

AG_Checkbox *
AG_CheckboxNew(void *parent, Uint flags, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(s, sizeof(s), fmt, ap);
		va_end(ap);
		return AG_CheckboxNewS(parent, flags, s);
	} else {
		return AG_CheckboxNewS(parent, flags, NULL);
	}
}

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
	if (flags & AG_CHECKBOX_SET) {
		cb->state = 1;
	}
	AG_ObjectAttach(parent, cb);
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewFn(void *parent, Uint flags, const char *label, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Checkbox *cb;
	AG_Event *ev;

	cb = AG_CheckboxNewS(parent, flags, label);
	ev = AG_SetEvent(cb, "checkbox-changed", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
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
AG_CheckboxNewFlag32(void *parent, Uint flags, const char *label,
    Uint32 *pFlags, Uint32 bitmask)
{
	AG_Checkbox *cb;
	cb = AG_CheckboxNewS(parent, flags, label);
	AG_BindFlag32(cb, "state", pFlags, bitmask);
	return (cb);
}

/* Create a set of checkboxes for the given set of flags. */
void
AG_CheckboxSetFromFlags(void *parent, Uint flags, Uint *pFlags,
    const AG_FlagDescr *fdSet)
{
	const AG_FlagDescr *fd;
	AG_Checkbox *cb;
	int i;

	for (i = 0; fdSet[i].bitmask != 0; i++) {
		fd = &fdSet[i];
		cb = AG_CheckboxNewFlag(parent, flags, fd->descr,
		    pFlags, fd->bitmask);
		if (!fd->writeable)
			AG_WidgetDisable(cb);
	}
}

/* Create a set of checkboxes for the given set of flags. */
void
AG_CheckboxSetFromFlags32(void *parent, Uint flags, Uint32 *pFlags,
    const AG_FlagDescr *fdSet)
{
	const AG_FlagDescr *fd;
	AG_Checkbox *cb;
	int i;

	for (i = 0; fdSet[i].bitmask != 0; i++) {
		fd = &fdSet[i];
		cb = AG_CheckboxNewFlag32(parent, flags, fd->descr,
		    pFlags, (Uint32)fd->bitmask);
		if (!fd->writeable)
			AG_WidgetDisable(cb);
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Checkbox *cb = AG_SELF();
	int button = AG_INT(1);

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
KeyDown(AG_Event *event)
{
	AG_Checkbox *cb = AG_SELF();
	int key = AG_INT(1);
	
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
Init(void *obj)
{
	AG_Checkbox *cb = obj;

	WIDGET(cb)->flags |= AG_WIDGET_FOCUSABLE|
	                     AG_WIDGET_UNFOCUSED_MOTION|
	                     AG_WIDGET_TABLE_EMBEDDABLE|
			     AG_WIDGET_USE_TEXT|
			     AG_WIDGET_USE_MOUSEOVER;

	AG_BindInt(cb, "state", &cb->state);
	AG_RedrawOnChange(cb, 100, "state");

	cb->flags = 0;
	cb->state = 0;
	cb->lbl = NULL;
	cb->spacing = 4;
	
	AG_SetEvent(cb, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(cb, "key-down", KeyDown, NULL);
}

static void
Draw(void *obj)
{
	AG_Checkbox *cb = obj;
	AG_Font *font = WIDGET(cb)->font;
	AG_Variable *stateb;
	void *p;
	int state;
	
	stateb = AG_GetVariable(cb, "state", &p);
	switch (AG_VARIABLE_TYPE(stateb)) {
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
		state = *(int *)p;
		break;
	case AG_VARIABLE_P_FLAG:
		state = *(int *)p & (int)stateb->info.bitmask;
		break;
	case AG_VARIABLE_P_FLAG8:
		state = *(Uint8 *)p & (Uint8)stateb->info.bitmask;
		break;
	case AG_VARIABLE_P_FLAG16:
		state = *(Uint16 *)p & (Uint16)stateb->info.bitmask;
		break;
	case AG_VARIABLE_P_FLAG32:
		state = *(Uint32 *)p & (Uint32)stateb->info.bitmask;
		break;
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
		state = *(Uint8 *)p;
		break;
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
		state = *(Uint16 *)p;
		break;
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		state = *(Uint32 *)p;
		break;
	default:
		state = 0;
		break;
	}

	if (WIDGET(cb)->flags & AG_WIDGET_MOUSEOVER) {
		AG_Rect r = AG_RECT(0,0,WIDGET(cb)->w,WIDGET(cb)->h-1);
		AG_DrawRectBlended(cb, r,
		    AG_ColorRGBA(255,255,255,25),
		    AG_ALPHA_SRC);
	}

	if (AG_WidgetEnabled(cb)) {
		AG_DrawBox(cb,
		    AG_RECT(0, 0, font->height, font->height),
		    state ? -1 : 1,
		    WCOLOR(cb,0));
	} else {
		AG_DrawBoxDisabled(cb,
		    AG_RECT(0, 0, font->height, font->height),
		    state ? -1 : 1,
		    WCOLOR(cb,0),
		    WCOLOR_DIS(cb,0));
	}

	if (cb->lbl != NULL) {
		AG_WidgetDraw(cb->lbl);
	}
	AG_UnlockVariable(stateb);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Checkbox *cb = obj;
	AG_Font *font = WIDGET(cb)->font;
	AG_SizeReq rLbl;

	r->h = font->height;
	r->w = font->height;
	
	if (cb->lbl != NULL) {
		AG_WidgetSizeReq(cb->lbl, &rLbl);
		r->w += cb->spacing + rLbl.w;
		r->h = MAX(r->h, rLbl.h);
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Checkbox *cb = obj;
	AG_Font *font = WIDGET(cb)->font;
	AG_SizeAlloc aLbl;

	if (a->w < font->height) {
		return (-1);
	}
	if (cb->lbl != NULL) {
		aLbl.x = font->height + cb->spacing;
		aLbl.y = 0;
		aLbl.w = a->w - aLbl.x;
		aLbl.h = a->h;
		AG_WidgetSizeAlloc(cb->lbl, &aLbl);
	}
	return (0);
}

/* Toggle the checkbox state. */
void
AG_CheckboxToggle(AG_Checkbox *cb)
{
	AG_Variable *stateb;
	void *p;

	AG_ObjectLock(cb);
	stateb = AG_GetVariable(cb, "state", &p);
	switch (AG_VARIABLE_TYPE(stateb)) {
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
		{
			int *state = (int *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    *state);
		}
		break;
	case AG_VARIABLE_P_FLAG:
		{
			int *state = (int *)p;
			AG_INVFLAGS(*state, (int)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_VARIABLE_P_FLAG8:
		{
			Uint8 *state = (Uint8 *)p;
			AG_INVFLAGS(*state, (Uint8)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (Uint8)*state);
		}
		break;
	case AG_VARIABLE_P_FLAG16:
		{
			Uint16 *state = (Uint16 *)p;
			AG_INVFLAGS(*state, (Uint16)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (Uint16)*state);
		}
		break;
	case AG_VARIABLE_P_FLAG32:
		{
			Uint32 *state = (Uint32 *)p;
			AG_INVFLAGS(*state, (Uint32)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (Uint32)*state);
		}
		break;
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
		{
			Uint8 *state = (Uint8 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
		{
			Uint16 *state = (Uint16 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		{
			Uint32 *state = (Uint32 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	default:
		break;
	}
	AG_UnlockVariable(stateb);
	AG_ObjectUnlock(cb);
	AG_Redraw(cb);
}

AG_WidgetClass agCheckboxClass = {
	{
		"Agar(Widget:Checkbox)",
		sizeof(AG_Checkbox),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
